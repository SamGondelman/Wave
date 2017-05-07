#include "DemoWorld.h"

#include "glm.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"
#include "SphereMesh.h"
#include "CubeMesh.h"

#include <ws2bth.h>
#include <iostream>

#define M_2PI 6.28318530718f

DemoWorld::DemoWorld() : World(":/shaders/shader.vert", ":/shaders/shader.frag"),
  serverStarted(false), m_waveDetector(true)
{
    // Resets hand data
    reset();

    // Hand offsets
    m_offsets.resize(NUM_FINGERS);
    m_offsets[FINGER1] = glm::vec3(-0.0375, 0, -0.0525);
    m_offsets[FINGER2] = glm::vec3(-0.01, 0, -0.0575);
    m_offsets[FINGER3] = glm::vec3(0.01, 0, -0.055);
    m_offsets[FINGER4] = glm::vec3(0.0375, 0, -0.042);
    m_offsets[THUMB] = glm::vec3(-0.035, 0, 0.0475);
    m_offsets[WRIST] = glm::vec3(0, 0, 0.0635);

    // Segment lengths
    m_segmentLengths.resize(2 * NUM_FINGERS - 3);
    m_segmentLengths[2 * FINGER1] = 0.02f;
    m_segmentLengths[2 * FINGER1 + 1] = 0.03f;
    m_segmentLengths[2 * FINGER2] = 0.025f;
    m_segmentLengths[2 * FINGER2 + 1] = 0.035f;
    m_segmentLengths[2 * FINGER3] = 0.025f;
    m_segmentLengths[2 * FINGER3 + 1] = 0.03f;
    m_segmentLengths[2 * FINGER4] = 0.02f;
    m_segmentLengths[2 * FINGER4 + 1] = 0.025f;
    m_segmentLengths[2 * THUMB] = 0.075f;
    // forearm and upper arm (-1 because only one point for thumb)
    m_segmentLengths[2 * WRIST - 1] = 0.254f;
    m_segmentLengths[2 * WRIST] = 0.3302f;

//    startServer();
}

DemoWorld::~DemoWorld() {
    stopServer();
}

void DemoWorld::stopServer() {
    if (serverStarted) {
        if (shutdown(clientSocket, SD_SEND) == SOCKET_ERROR) {
            std::cout << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            exit(1);
        }

        closesocket(clientSocket);
        WSACleanup();
    }
}

void DemoWorld::startServer() {
    WSADATA wsa;
    SOCKET listenSocket;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cout << "Initializing Winsock failed. Error Code: " << WSAGetLastError() << std::endl;
        exit(1);
    }

    serverStarted = true;

    if((listenSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM)) == INVALID_SOCKET) {
        std::cout << "Could not create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(1);
    }

    char optval = 1;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == SOCKET_ERROR) {
        std::cout << "Could not setsockopt: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(1);
    }

    SOCKADDR_BTH sab;
    memset (&sab, 0, sizeof(sab));
    sab.addressFamily  = AF_BTH;
    sab.port = 2 & 0xff; // must match client rc_channel

    if(bind(listenSocket, (SOCKADDR *) &sab, sizeof(sab)) == SOCKET_ERROR) {
        std::cout << "Bind failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        exit(1);
    }

    if (listen(listenSocket, 1) == SOCKET_ERROR) {
        std::cout << "Listen failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        exit(1);

    }

    SOCKADDR_BTH sab2;
    int c = sizeof(sab2);
    clientSocket = accept(listenSocket, (struct sockaddr *) &sab2, &c);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "Accept failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        exit(1);
    }

    closesocket(listenSocket);

//    const int DEFAULT_BUFLEN = 512;
//    char recvbuf[DEFAULT_BUFLEN];
//    int recvbuflen = DEFAULT_BUFLEN;
//    int numBytes = recv(clientSocket, recvbuf, recvbuflen, 0);
//    while (numBytes > 0) {
//        std::cout << numBytes << std::endl;
//        float *res = (float *) recvbuf;
//        for (int i = 0; i < numBytes; i += sizeof(float)) {
//            std::cout << *((float *)((char *)res + i)) << " ";
//        }
//        std::cout << std::endl;
//        numBytes = recv(clientSocket, recvbuf, recvbuflen, 0);
//    }

//    if (numBytes < 0) {
//        std::cout << "recv failed with error code: " << WSAGetLastError() << std::endl;
//        closesocket(clientSocket);
//        WSACleanup();
//        exit(1);
//    }
}

void DemoWorld::makeCurrent() {
    m_lights.clear();
    m_lights.push_back(Light(glm::vec3(-1.0f), glm::vec3(0.7f)));
}

void DemoWorld::update(float dt) {
//    View::m_player->setEye(glm::vec3(0.5f*glm::sin(View::m_globalTime/5.0f), 0.5f, 0.5f*glm::cos(View::m_globalTime/5.0f)));
    View::m_player->setEye(glm::vec3(0.8f, 0.8f, 0.8f));
    View::m_player->setCenter(glm::vec3(0));

    if (serverStarted) {
        const int MAX_PACKETS = 40;
        const int DEFAULT_BUFLEN = Gesture::NUM_DATA * MAX_PACKETS;
        char recvbuf[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;
        int numBytes = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (numBytes == 0) {
            // Reset server and data
            stopServer();
            startServer();
            reset();
        } else if (numBytes < 0) {
            std::cout << "recv failed with error code: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            exit(1);
        } else if (numBytes == DEFAULT_BUFLEN) {
            std::cout << "Received max numBytes." << std::endl;
        }
        float *res = (float *) recvbuf;
        float *data = (float *) m_handData.data();
        int numFloats = numBytes / sizeof(float);
        for (int i = 0; i < numFloats; i++) {
            int j = i % Gesture::NUM_DATA;
            data[j] += res[j];
            // 0 < angle < 2*PI
            data[j] = glm::mod(data[j], M_2PI);
        }
    }

    // arm swinging
    m_handData[PALM_ROT].y = M_PI_2 * glm::sin(View::m_globalTime * 1.5) - M_PI_2;
    m_handData[FOREARM_ROT].y = M_PI_2 * glm::sin(View::m_globalTime * 1.5) - M_PI_2;
    m_handData[UPPERARM_ROT].y = 0.66 * M_PI_2 * glm::sin(View::m_globalTime * 1.5) - M_PI_2;

    if (m_isRecordingMotion) m_recordedMotion.insert(m_recordedMotion.end(), m_handData.begin(), m_handData.end());

    m_waveDetector.update(dt, m_handData);
}

void DemoWorld::reset() {
    // Calibrated hand orientation
    m_handData.resize(NUM_HAND_DATA);
    // rotations are pitch, yaw, roll
    m_handData[PALM_ROT] = glm::radians(glm::vec3(0, 270, 270));
    m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(0, 310, 270));
    m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(0, 310, 270));
    m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(0, 270, 270));
    m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(0, 270, 270));
    m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(0, 240, 270));
    m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(0, 240, 270));
    m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(0, 230, 270));
    m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(0, 230, 270));
    m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(0, 320, 270));
    m_handData[FOREARM_ROT] = glm::radians(glm::vec3(0, 270, 270));
    m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(0, 270, 270));
}

void DemoWorld::drawGeometry() {
    drawHand(m_handData);

    if (m_waveDetector.getVisID() > 0) {
        if (m_waveDetector.getVisID() <= (int) m_waveDetector.getInstantGestures().size()) {
            drawHand(m_waveDetector.getInstantGestures()[m_waveDetector.getVisID() - 1]->getData(), 0.005f);
        } else {
            auto mg = m_waveDetector.getMotionGestures()[m_waveDetector.getVisID() -
                    m_waveDetector.getInstantGestures().size() - 1];
            drawHand(mg->getFrames()[m_waveDetector.getVisFrame()].first->getData(), 0.005f);
        }
    }

    // floor
    glm::mat4 M = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::vec3(5.0f, 1.0f, 5.0f));
    m_program->setUniform("M", M);
    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0.25, 0.25, 0.25, 1);
    mat.cSpecular = glm::vec4(1, 0, 1, 1);
    mat.shininess = 10.0f;
    m_program->applyMaterial(mat);
    View::m_cube->draw();
}

void DemoWorld::drawHand(const std::vector<glm::vec3> &data, float scale) {
    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0.1, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0, 1, 0, 1);
    mat.cSpecular = glm::vec4(0, 0, 1, 1);
    mat.shininess = 20.0f;
    glm::mat4 M;

    glm::vec3 palmRot = data[PALM_ROT];
    if (glm::isnan(palmRot.x)) palmRot.x = 0;
    if (glm::isnan(palmRot.y)) palmRot.y = 0;
    if (glm::isnan(palmRot.z)) palmRot.z = 0;
    glm::quat handRotQuat = glm::normalize(glm::quat(palmRot));
    glm::mat4 handRot = glm::toMat4(handRotQuat);
    glm::mat4 handScale = glm::scale(glm::vec3(scale));

    const glm::vec3 shoulder = glm::vec3(0.25, 0.5, 0);
    glm::vec3 elbow = shoulder + m_segmentLengths[2 * WRIST] * glm::normalize(glm::quat(data[UPPERARM_ROT]) * glm::vec3(0, 0, -1));
    if (glm::isnan(elbow.x)) elbow.x = 0;
    if (glm::isnan(elbow.y)) elbow.y = 0;
    if (glm::isnan(elbow.z)) elbow.z = 0;
    glm::vec3 wrist = elbow + m_segmentLengths[2 * WRIST - 1] * glm::normalize(glm::quat(data[FOREARM_ROT]) * glm::vec3(0, 0, -1));
    if (glm::isnan(wrist.x)) wrist.x = 0;
    if (glm::isnan(wrist.y)) wrist.y = 0;
    if (glm::isnan(wrist.z)) wrist.z = 0;
    glm::vec3 palmPos = wrist + handRotQuat * m_offsets[WRIST];

    glm::mat4 handPos = glm::translate(palmPos);

    M = handPos * handRot * handScale;
    m_program->setUniform("M", M);
    m_program->applyMaterial(mat);
    View::m_sphere->draw();

    for (int i = 0; i < NUM_FINGERS; i++) {
        // Knuckles/wrist
        M = handPos * handRot * glm::translate(m_offsets[i]) * handScale;
        m_program->setUniform("M", M);
        mat.cDiffuse = glm::vec4(0, 1, 0, 1);
        m_program->applyMaterial(mat);
        View::m_sphere->draw();

        // Finger joints
        if (i < WRIST) {
            int firstIndex = FIRST_FINGER + 2 * i;
            glm::vec3 firstJoint = palmPos + handRotQuat * m_offsets[i] +
                    m_segmentLengths[2 * i] * glm::normalize(glm::quat(data[firstIndex]) * glm::vec3(0, 0, -1));
            M = glm::translate(firstJoint) * glm::toMat4(glm::quat(data[firstIndex])) * handScale;
            m_program->setUniform("M", M);
            mat.cDiffuse = glm::vec4(0, 0, 1, 1);
            m_program->applyMaterial(mat);
            View::m_sphere->draw();

            if (i < THUMB) {
                int secondIndex = FIRST_FINGER + 2 * i + 1;
                glm::vec3 secondJoint = firstJoint +
                        m_segmentLengths[2 * i + 1] * glm::normalize(glm::quat(data[secondIndex]) * glm::vec3(0, 0, -1));
                M = glm::translate(secondJoint) * glm::toMat4(glm::quat(data[secondIndex])) * handScale;
                m_program->setUniform("M", M);
                m_program->applyMaterial(mat);
                View::m_sphere->draw();
            }
        }
    }

    // Shoulder
    mat.cDiffuse = glm::vec4(1, 1, 0, 1);
    M = glm::translate(shoulder) * handScale;
    m_program->applyMaterial(mat);
    m_program->setUniform("M", M);
    View::m_sphere->draw();

    // Elbow
    M = glm::translate(elbow) * handScale;
    m_program->setUniform("M", M);
    View::m_sphere->draw();

    // Body center and neck
    M = handScale;
    mat.cDiffuse = glm::vec4(1, 0, 1, 1);
    m_program->applyMaterial(mat);
    m_program->setUniform("M", M);
    View::m_sphere->draw();

    M = glm::translate(glm::vec3(0, shoulder.y, 0)) * handScale;
    m_program->setUniform("M", M);
    View::m_sphere->draw();
}

void DemoWorld::recordMotionGesture() {
    if (!m_isRecordingMotion) {
        std::cout << "Started recording" << std::endl;
        m_isRecordingMotion = true;
        // preallocate space for m_recordedMotion?
    } else {
        std::cout << "Stopped recording" << std::endl;
        m_isRecordingMotion = false;
        m_waveDetector.saveMotionGesture(m_recordedMotion);
        m_recordedMotion.clear();
    }
}
