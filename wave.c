#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <math.h>
#include "linux/i2c-dev.h"

#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

// Loop interval in milliseconds
#define DT 20
#define DT_S 0.02

// Calibration time in milliseconds
#define CALIB_TIME 5000

// Returns time in milliseconds
int getTime() {
        struct timeval t;
        gettimeofday(&t, NULL);
        return (t.tv_sec)*1000 + (t.tv_usec)/1000;
}

#define MULTIPLEXER_ADDRESS 0x70

#define GYRO_ADDRESS 0x6B
#define GYRO_ADDRESS_2 0x6A
#define CTRL_REG1_G 0x20
#define CTRL_REG4_G 0x23
#define OUT_X_L_G 0x28
#define G_GAIN 0.0175

#define ACC_ADDRESS 0x19
#define CTRL_REG1_A 0x20
#define CTRL_REG2_A 0x21
#define CTRL_REG4_A 0x23
#define OUT_X_L_A 0x28
// 9.81 * 4 / ((2^16) - 1)
#define A_GAIN 0.00059876401

#define RAD_TO_DEG 57.29578
#define DEG_TO_RAD 0.0174533

int file;
int chan;

void selectDevice(int file, int addr) {
        if (ioctl(file, I2C_SLAVE, addr) < 0) {
                printf("Failed to select I2C device: %d on channel %d.\n", addr, chan);
                exit(1);
        }
}

void readBlock(uint8_t command, uint8_t size, uint8_t *data) {
        int result = i2c_smbus_read_i2c_block_data(file, command, size, data);
        if (result != size) {
                printf("Failed to read block from I2C on channel: %d\n", chan);
                exit(1);
        }
}

void readDevice(int *b, int deviceAddress, int dataAddress) {
        uint8_t block[6];
        selectDevice(file, deviceAddress);
        //printf("addr: %d", deviceAddress);
        readBlock(0x80 | dataAddress, sizeof(block), block);

        *b = (int16_t)(block[0] | block[1] << 8);
        *(b+1) = (int16_t)(block[2] | block[3] << 8);
        *(b+2) = (int16_t)(block[4] | block[5] << 8);
}

void writeReg(uint8_t reg, uint8_t value, int address) {
        selectDevice(file, address);
        int result = i2c_smbus_write_byte_data(file, reg, value);
        if (result == -1) {
                printf("Failed to write byte to I2C address %d on channel %d.\n", address, chan);
                exit(1);
        }
}

void switchChannel(int channel) {
        chan = channel;
        selectDevice(file, MULTIPLEXER_ADDRESS);
        int result = i2c_smbus_write_byte(file, 1 << channel);
        if (result == -1) {
                printf("Failed to switch to channel %d on I2C multiplexer.\n", channel);
                exit(1);
        }

}

void enableGyroscope(int address) {
        writeReg(CTRL_REG1_G, 0b00001111, address);
        writeReg(CTRL_REG4_G, 0b00011000, address);
}

void enableSensors() {
        __u16 block[I2C_SMBUS_BLOCK_MAX];

        int res, bus, size;

        char filename[20];
        sprintf(filename, "/dev/i2c-%d", 1);
        file = open(filename, O_RDWR);
        if (file < 0) {
                printf("Unable to open I2C bus.\n");
                exit(1);
        }

        // IMU
        switchChannel(0);
        // Gyroscope
        enableGyroscope(GYRO_ADDRESS);
        // Accelerometer
        //writeReg(CTRL_REG1_A, 0b01010111, ACC_ADDRESS);
        //writeReg(CTRL_REG2_A, 0b00001000, ACC_ADDRESS); // switch 4th to last bit to disable/enable filtering
        //writeReg(CTRL_REG4_A, 0b00010000, ACC_ADDRESS); // don't need this
        // Magnetometer?

        // Thumb
        switchChannel(1);
        enableGyroscope(GYRO_ADDRESS);

        // Finger 1
        switchChannel(7);
        enableGyroscope(GYRO_ADDRESS);
        enableGyroscope(GYRO_ADDRESS_2);

        // Finger 2
        switchChannel(6);
        enableGyroscope(GYRO_ADDRESS);
        enableGyroscope(GYRO_ADDRESS_2);

        // Finger 3
        switchChannel(4);
        enableGyroscope(GYRO_ADDRESS);
        enableGyroscope(GYRO_ADDRESS_2);

        // Finger 4
        switchChannel(2);
        enableGyroscope(GYRO_ADDRESS);
        enableGyroscope(GYRO_ADDRESS_2);

        // Arm
        switchChannel(3);
        enableGyroscope(GYRO_ADDRESS);
        enableGyroscope(GYRO_ADDRESS_2);
}

int startClient() {
        // Start Bluetooth client
        struct sockaddr_rc addr = { 0 };
        int s;
        char dest[18] = "34:02:86:92:81:36"; // hardcoded bluetooth MAC address of server

        while (1) {
                s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
                if (s == -1) {
                    perror("Socket creation failed: ");
                    return 1;
                }
                addr.rc_family = AF_BLUETOOTH;
                addr.rc_channel = 2;
                str2ba(dest, &addr.rc_bdaddr);
                if (connect(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
                        close(s);
                        sleep(5);
                        continue;
                }
                break;
        }
        return s;
}

int main(int argc, char *argv) {
        int s = startClient();

        const int DATA_SIZE = 3 * 12;
        int dataRaw[DATA_SIZE];
        float dataRate[DATA_SIZE];
        //float vel[3] = { 0.0, 0.0, 0.0 };

        int calibrating = 1;
        int calibrationSamples = 0;
        int dataSum[DATA_SIZE];
        memset(dataSum, 0.0, sizeof(dataSum));
        float dataBias[DATA_SIZE];

        enableSensors();

        float data[DATA_SIZE];

        int time;
        int startTime = getTime();
        int i;
        printf("Calibrating...\n");
        while (1) {
                time = getTime();

                switchChannel(0);
                //readDevice(dataRaw, ACC_ADDRESS, OUT_X_L_A);
                readDevice(dataRaw, GYRO_ADDRESS, OUT_X_L_G);
                switchChannel(7);
                readDevice(dataRaw + 3, GYRO_ADDRESS, OUT_X_L_G);
                readDevice(dataRaw + 6, GYRO_ADDRESS_2, OUT_X_L_G);
                switchChannel(4);
                readDevice(dataRaw + 9, GYRO_ADDRESS, OUT_X_L_G);
                readDevice(dataRaw + 12, GYRO_ADDRESS_2, OUT_X_L_G);
                switchChannel(6);
                readDevice(dataRaw + 15, GYRO_ADDRESS, OUT_X_L_G);
                readDevice(dataRaw + 18, GYRO_ADDRESS_2, OUT_X_L_G);
                switchChannel(2);
                readDevice(dataRaw + 21, GYRO_ADDRESS, OUT_X_L_G);
                readDevice(dataRaw + 24, GYRO_ADDRESS_2, OUT_X_L_G);
                switchChannel(1);
                readDevice(dataRaw + 27, GYRO_ADDRESS, OUT_X_L_G);
                switchChannel(3);
                readDevice(dataRaw + 30, GYRO_ADDRESS, OUT_X_L_G);
                readDevice(dataRaw + 33, GYRO_ADDRESS_2, OUT_X_L_G);

                if (time - startTime < CALIB_TIME) {
                        for (i = 0; i < DATA_SIZE; i++) {
                                dataSum[i] += dataRaw[i];
                        }
                        calibrationSamples++;
                } else {
                        if (calibrating) {
                                printf("Calibration finished.\n");
                                for (i = 0; i < DATA_SIZE; i++) {
                                        dataBias[i] = ((float)(dataSum[i]))/calibrationSamples;
                                        printf("Gyroscope Bias: (%7.3f, %7.3f, %7.3f)\n", dataBias[i-2], dataBias[i-1], dataBias[i]);
                                }
                                calibrating = 0;
                        }

                        //float prevVel[3];
                        for (i = 0; i < DATA_SIZE; i++) {
                                //if (i < 3) {
                                        //prevVel[i] = vel[i];
                                        //vel[i] += dataRate[i] * DT_S;
                                //}
                                float gain = G_GAIN;
                                dataRate[i] = ((float)(dataRaw[i]) - dataBias[i]) * gain;
                        }

                        // Coordinate space needs to be corrected for renderer
                        data[0] = dataRate[0] * DT_S * DEG_TO_RAD;
                        data[1] = dataRate[2] * DT_S * DEG_TO_RAD;
                        data[2] = -dataRate[1] * DT_S * DEG_TO_RAD;
                        for (i = 3; i < DATA_SIZE - 6; i += 3) {
                                data[i] = dataRate[i+1] * DT_S * DEG_TO_RAD;
                                data[i+1] = dataRate[i+2] * DT_S * DEG_TO_RAD;
                                data[i+2] = dataRate[i] * DT_S * DEG_TO_RAD;
                        }
                        // forearm and upper arm
                        data[DATA_SIZE - 6] = -dataRate[DATA_SIZE - 5] * DT_S * DEG_TO_RAD;
                        data[DATA_SIZE - 5] = dataRate[DATA_SIZE - 4] * DT_S * DEG_TO_RAD;
                        data[DATA_SIZE - 4] = -dataRate[DATA_SIZE - 6] * DT_S * DEG_TO_RAD;
                        data[DATA_SIZE - 3] = -dataRate[DATA_SIZE - 2] * DT_S * DEG_TO_RAD;
                        data[DATA_SIZE - 2] = dataRate[DATA_SIZE - 1] * DT_S * DEG_TO_RAD;
                        data[DATA_SIZE - 1] = -dataRate[DATA_SIZE - 3] * DT_S * DEG_TO_RAD;

                        if (write(s, &data, DATA_SIZE*sizeof(float)) < 0) {
                                perror("Write failed: ");
                                close(s);
                                s = startClient();
                                calibrating = 1;
                                calibrationSamples = 0;
                                memset(dataSum, 0.0, sizeof(dataSum));
                                //memset(vel, 0.0, sizeof(vel));
                                startTime = getTime();
                        }
                }

                // Make sure loop intervals are even
                while (getTime() - time < DT) {
                        //if (calibrating == 0) printf("sleeping");
                        usleep(100);
                }
        }

        close(s);
        return 0;
}