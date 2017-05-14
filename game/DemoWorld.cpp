#include "DemoWorld.h"

#include "glm.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"
#include "SphereMesh.h"
#include "CubeMesh.h"
#include "CylinderMesh.h"

#include <ws2bth.h>
#include <iostream>

#include <winuser.h>

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
}

void DemoWorld::makeCurrent() {
    m_lights.clear();
    m_lights.push_back(Light(glm::vec3(-1.0f), glm::vec3(0.7f)));
}

void DemoWorld::update(float dt) {
//    View::m_player->setEye(glm::vec3(0.8f*glm::sin(View::m_globalTime/5.0f), 0.8f, 0.8f*glm::cos(View::m_globalTime/5.0f)));
    View::m_player->setEye(glm::vec3(0.8f, 0.8f, 0.8f));
    View::m_player->setCenter(glm::vec3(0, 0.15, 0));

    if (serverStarted) {
        const int MAX_PACKETS = 150;
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
        glm::vec3 *res = (glm::vec3 *) recvbuf;
//        float *data = (float *) m_handData.data();
        int numVecs = numBytes / sizeof(float) / 3;
        for (int i = 0; i < numVecs; i++) {
            int j = i % (Gesture::NUM_DATA / 3);
            // 0 < angle < 2*PI
            m_handData[j] = glm::mod(glm::eulerAngles(glm::quat(m_handData[j]) * glm::quat(res[i])), M_2PI);
        }
    }

    if (m_isRecordingMotion) m_recordedMotion.insert(m_recordedMotion.end(), m_handData.begin(), m_handData.end());

    m_waveDetector.update(dt, m_handData);
    for (auto &g : m_waveDetector.getCurrentGestures()) {
        if (m_waveDetector.getGestureNames()[g] == "light switch") {
            switchLight();
        } else if (m_waveDetector.getGestureNames()[g] == "brightness") {
            lightCubeBrightness = glm::clamp((m_handPos[PALM_POS].y - 0.1f) / 0.432f, 0.0f, 1.0f);
        } else if (m_waveDetector.getGestureNames()[g] == "scroll up") {
            scroll(true);
            std::cout << "scroll up" << std::endl;
        } else if (m_waveDetector.getGestureNames()[g] == "scroll down") {
            scroll(false);
            std::cout << "scroll down" << std::endl;
        } else if (m_waveDetector.getGestureNames()[g] == "set screen") {
            setScreenPlane();
        } else if (m_waveDetector.getGestureNames()[g] == "click") {
            checkClick();
        }
    }

    if (m_mode == NONE) {
        m_handData[PALM_ROT] = glm::radians(glm::vec3(0, 270, 270));
        m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(0, 290, 270));
        m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(0, 290, 270));
        m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(0, 270, 270));
        m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(0, 270, 270));
        m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(0, 240, 270));
        m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(0, 240, 270));
        m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(0, 230, 270));
        m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(0, 230, 270));
        m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(300, 0, 0));
        m_handData[FOREARM_ROT] = glm::radians(glm::vec3(90, 0, 180));
        m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(90, 0, 180));
    } else if (m_mode == LIGHT_ON) {
        m_handData[PALM_ROT] = glm::radians(glm::vec3(0, 0, 180));
        m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(-90 * (1.0f - glm::mod(View::m_globalTime/2.0f, 1.0f)), 20, 180));
        m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(-180 * (1.0f - glm::mod(View::m_globalTime/2.0f, 1.0f)), 20, 180));
        m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(-90 * (1.0f - glm::mod(View::m_globalTime/2.0f, 1.0f)), 0, 180));
        m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(-180 * (1.0f - glm::mod(View::m_globalTime/2.0f, 1.0f)), 0, 180));
        m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(-90 * (1.0f - glm::mod(View::m_globalTime/2.0f, 1.0f)), 330, 180));
        m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(-180 * (1.0f - glm::mod(View::m_globalTime/2.0f, 1.0f)), 330, 180));
        m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(-90 * (1.0f - glm::mod(View::m_globalTime/2.0f, 1.0f)), 320, 180));
        m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(-180 * (1.0f - glm::mod(View::m_globalTime/2.0f, 1.0f)), 320, 180));
        m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(-20 + 90 * (glm::mod(View::m_globalTime/2.0f, 1.0f)), 45, 270));
        m_handData[FOREARM_ROT] = glm::radians(glm::vec3(30 * glm::sin(View::m_globalTime*1.2f), -30 * (glm::sin(View::m_globalTime/2.0f)+1), 90));
        m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(30 * glm::sin(View::m_globalTime/0.8f), 45 + 30 * (glm::sin(View::m_globalTime*1.5f)+1), 90));
    } else if (m_mode == BRIGHTNESS) {
        m_handData[PALM_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(0, 10, 270));
        m_handData[FOREARM_ROT] = glm::radians(glm::vec3(0, -30 * (glm::sin(View::m_globalTime*3.0f)+1), 90));
        m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(0, 45 + 30 * (glm::sin(View::m_globalTime*1.5f)+1), 90));
    } else if (m_mode == SCROLL_UP) {
        float pitch = 45 * glm::sin(View::m_globalTime*3.0f);
        m_handData[PALM_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(270 + pitch, 340, 0));
        m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(180 + pitch, 340, 0));
        m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(270 + pitch, 330, 0));
        m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(180 + pitch, 330, 0));
        m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(330 + pitch, 330, 0));
        m_handData[FOREARM_ROT] = glm::radians(glm::vec3(0, -45, 90));
        m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(0, 45, 90));
    } else if (m_mode == SCROLL_DOWN) {
        float pitch = 45 * glm::sin(View::m_globalTime*3.0f);
        m_handData[PALM_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(270 + pitch, 0, 0));
        m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(180 + pitch, 0, 0));
        m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(270 + pitch, 340, 0));
        m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(180 + pitch, 340, 0));
        m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(270 + pitch, 330, 0));
        m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(180 + pitch, 330, 0));
        m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(330 + pitch, 330, 0));
        m_handData[FOREARM_ROT] = glm::radians(glm::vec3(0, -45, 90));
        m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(0, 45, 90));
    } else if (m_mode == SET_PLANE) {
        m_handData[PALM_ROT] = glm::radians(glm::vec3(90, 0, 0));
        m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(0 + 45 * (glm::sin(View::m_globalTime/2.0f)+1), 0, 0));
        m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(270 + 90 * (glm::sin(View::m_globalTime/2.0f)+1), 0, 0));
        m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(0, 0, 0));
        m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(270, 0, 0));
        m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(0, 340, 0));
        m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(270, 340, 0));
        m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(0, 330, 0));
        m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(270, 330, 0));
        m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(0, 300 + 75 * (glm::sin(View::m_globalTime/2.0f)+1), 0));
        m_handData[FOREARM_ROT] = glm::radians(glm::vec3(-20 * (glm::sin(View::m_globalTime/2.0f)+1), -60 + 25 * (glm::sin(View::m_globalTime/2.0f)+1), 90));
        m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(0, 90 - 22.5 * (glm::sin(View::m_globalTime/2.0f)+1), 90));
    } else if (m_mode == CLICK) {
        float pitch = 15;
        m_handData[PALM_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(0 + pitch, 0, 0));
        m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(270 + pitch, 0, 0));
        m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(180 + pitch, 0, 0));
        m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(270 + pitch, 340, 0));
        m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(180 + pitch, 340, 0));
        m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(270 + pitch, 330, 0));
        m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(180 + pitch, 330, 0));
        m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(330 + pitch, 330, 0));
        m_handData[FOREARM_ROT] = glm::radians(glm::vec3(0, -45 - 20 * glm::sin(View::m_globalTime/1.7f), 90));
        m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(30 * glm::sin(View::m_globalTime/1.5f), 75, 90));
    }
}

void DemoWorld::reset() {
    // Calibrated hand orientation
    m_handData.resize(NUM_HAND_DATA);
    // rotations are pitch, yaw, roll
    m_handData[PALM_ROT] = glm::radians(glm::vec3(0, 270, 270));
    m_handData[FINGER1_1_ROT] = glm::radians(glm::vec3(0, 290, 270));
    m_handData[FINGER1_2_ROT] = glm::radians(glm::vec3(0, 290, 270));
    m_handData[FINGER2_1_ROT] = glm::radians(glm::vec3(0, 270, 270));
    m_handData[FINGER2_2_ROT] = glm::radians(glm::vec3(0, 270, 270));
    m_handData[FINGER3_1_ROT] = glm::radians(glm::vec3(0, 240, 270));
    m_handData[FINGER3_2_ROT] = glm::radians(glm::vec3(0, 240, 270));
    m_handData[FINGER4_1_ROT] = glm::radians(glm::vec3(0, 230, 270));
    m_handData[FINGER4_2_ROT] = glm::radians(glm::vec3(0, 230, 270));
    m_handData[THUMB_1_ROT] = glm::radians(glm::vec3(300, 0, 0));
    m_handData[FOREARM_ROT] = glm::radians(glm::vec3(90, 0, 180));
    m_handData[UPPERARM_ROT] = glm::radians(glm::vec3(90, 0, 180));

    m_handPos.resize(NUM_HAND_POS);
}

void DemoWorld::drawGeometry() {
    drawHand(m_handData, 0.01f, true);

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

    // demo light cube
    if (renderLightCube) {
        M = glm::translate(glm::vec3(0.0f, 0.6f, 0.0f)) * glm::scale(glm::vec3(0.1));
        m_program->setUniform("M", M);
        CS123SceneMaterial mat;
        mat.cAmbient = glm::vec4(0, 0, 0, 1);
        mat.cDiffuse = glm::vec4(lightCubeBrightness * glm::vec3(1, 1, 0), 1);
        mat.cSpecular = glm::vec4(0.7, 0.7, 0.7, 1);
        mat.shininess = 10.0f;
        m_program->applyMaterial(mat);
        View::m_cube->draw();
    }

    if (!glm::isnan(screenPlanePos.x)) {
        M = glm::translate(screenPlanePos) * screenPlaneRot * glm::scale(glm::vec3(0.345f, 0.001f, 0.195f));
        m_program->setUniform("M", M);
        CS123SceneMaterial mat;
        mat.cAmbient = glm::vec4(0.1, 0.1, 0.1, 1);
        mat.cDiffuse = glm::vec4(0.7, 0.7, 0.7, 1);
        mat.cSpecular = glm::vec4(0.3, 0.3, 0.3, 1);
        mat.shininess = 10.0f;
        m_program->applyMaterial(mat);
        View::m_cube->draw();
    }

    if (!glm::isnan(m_clickPoint.x)) {
        M = glm::translate(m_clickPoint) * glm::scale(glm::vec3(0.05f));
        m_program->setUniform("M", M);
        CS123SceneMaterial mat;
        mat.cAmbient = glm::vec4(0.1, 0.1, 0.1, 1);
        mat.cDiffuse = glm::vec4(1, 0, 0, 1);
        mat.cSpecular = glm::vec4(0.3, 0.3, 0.3, 1);
        mat.shininess = 10.0f;
        m_program->applyMaterial(mat);
        View::m_sphere->draw();
    }
}

void DemoWorld::drawHand(const std::vector<glm::vec3> &data, float scale, bool savePos) {
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
    glm::quat handRotQuat = glm::quat(palmRot);
    glm::mat4 handRot = glm::toMat4(handRotQuat);
    glm::mat4 handScale = glm::scale(glm::vec3(0.01, 0.01, 2.0*scale));

    const glm::vec3 shoulder = glm::vec3(0.22, 0.47, 0);
    glm::vec3 elbow = shoulder + m_segmentLengths[2 * WRIST] * (glm::quat(data[UPPERARM_ROT]) * glm::vec3(0, 0, -1));
    if (glm::isnan(elbow.x)) elbow.x = 0;
    if (glm::isnan(elbow.y)) elbow.y = 0;
    if (glm::isnan(elbow.z)) elbow.z = 0;
    glm::vec3 wrist = elbow + m_segmentLengths[2 * WRIST - 1] * (glm::quat(data[FOREARM_ROT]) * glm::vec3(0, 0, -1));

    if (glm::isnan(wrist.x)) wrist.x = 0;
    if (glm::isnan(wrist.y)) wrist.y = 0;
    if (glm::isnan(wrist.z)) wrist.z = 0;
    glm::vec3 palmPos = wrist - handRotQuat * m_offsets[WRIST];
//    palmPos = glm::vec3(0);

    if (savePos) {
        m_handPos[PALM_POS] = palmPos;
        m_handPos[ELBOW_POS] = elbow;
    }

    glm::mat4 handPos = glm::translate(palmPos);

    M = handPos * handRot * handScale;
    m_program->setUniform("M", M);
    m_program->applyMaterial(mat);
    View::m_sphere->draw();

    const float NUM_POINTS_PER_FINGER = 3;
    for (int i = 0; i < NUM_FINGERS; i++) {
        // Knuckles/wrist
        M = handPos * handRot * glm::translate(m_offsets[i]) * handScale;
        m_handPos[FIRST_KNUCKLE_POS + i * NUM_POINTS_PER_FINGER] = glm::vec3(M[3][0], M[3][1], M[3][2]);
        m_program->setUniform("M", M);
        mat.cDiffuse = glm::vec4(0, 1, 0, 1);
        m_program->applyMaterial(mat);
        View::m_sphere->draw();

        // Finger joints
        if (i < WRIST) {
            int firstIndex = FIRST_FINGER + 2 * i;
            glm::vec3 firstJoint = palmPos + handRotQuat * m_offsets[i] +
                    m_segmentLengths[2 * i] * (glm::quat(data[firstIndex]) * glm::vec3(0, 0, -1));
            if (savePos) m_handPos[FIRST_KNUCKLE_POS + i * NUM_POINTS_PER_FINGER + 1] = firstJoint;
            M = glm::translate(firstJoint) * glm::mat4_cast(glm::quat(data[firstIndex])) * handScale;
            m_program->setUniform("M", M);
            mat.cDiffuse = glm::vec4(1, 0, 0, 1);
            m_program->applyMaterial(mat);
            View::m_cylinder->draw();

            if (i < THUMB) {
                int secondIndex = FIRST_FINGER + 2 * i + 1;
                glm::vec3 secondJoint = firstJoint +
                        m_segmentLengths[2 * i + 1] * (glm::quat(data[secondIndex]) * glm::vec3(0, 0, -1));
                if (savePos) m_handPos[FIRST_KNUCKLE_POS + i * NUM_POINTS_PER_FINGER + 2] = secondJoint;
                M = glm::translate(secondJoint) * glm::mat4_cast(glm::quat(data[secondIndex])) * handScale;
                m_program->setUniform("M", M);
                m_program->applyMaterial(mat);
                View::m_cylinder->draw();
            }
        }
    }

    // Shoulder
    mat.cDiffuse = glm::vec4(1, 1, 0, 1);
    M = glm::translate(shoulder) * glm::mat4_cast(glm::quat(data[UPPERARM_ROT])) *
            glm::scale(glm::vec3(0.01, 0.01, m_segmentLengths[2 * WRIST])) * glm::translate(glm::vec3(0, 0, -0.5f));
    m_program->applyMaterial(mat);
    m_program->setUniform("M", M);
    View::m_cube->draw();

    // Elbow
    M = glm::translate(elbow) * glm::mat4_cast(glm::quat(data[FOREARM_ROT])) *
            glm::scale(glm::vec3(0.01, 0.01, m_segmentLengths[2 * WRIST - 1])) * glm::translate(glm::vec3(0, 0, -0.5f));
    m_program->setUniform("M", M);
    View::m_cube->draw();

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

void DemoWorld::setScreenPlane() {
    glm::vec3 v1 = glm::normalize(m_handPos[THUMB1_POS] - m_handPos[PALM_POS]);
    glm::vec3 v2 = glm::normalize(m_handPos[KNUCKLE_2_POS] - m_handPos[PALM_POS]);
    screenPlaneRot = glm::mat4_cast(glm::quat(m_handData[FINGER1_1_ROT]));
    screenPlanePos = m_handPos[PALM_POS] + v1 * 0.345f / 2.0f + v2 * 0.195f / 2.0f;
}

void DemoWorld::scroll(bool up) {
    const int minWheelMovement = 120;
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, (up ? 1 : -1) * 4 * minWheelMovement, 0);
}

void DemoWorld::checkClick() {
    if (!glm::isnan(screenPlanePos.x)) {
        glm::vec4 o = glm::vec4(m_handPos[JOINT1_1_POS], 1);
        glm::vec4 r = glm::normalize(glm::vec4(m_handPos[JOINT1_2_POS], 0) - o);
        o -= glm::vec4(screenPlanePos, 0);
        o = glm::inverse(screenPlaneRot) * o;
        r = glm::normalize(glm::inverse(screenPlaneRot) * r);
        const glm::vec4 nor = glm::vec4(0, 1, 0, 0);
        if (glm::dot(nor, r) >= 0.0f) return;
        float x = -glm::dot(nor, o) / glm::dot(nor, r);
        if (x > 0.0f/* && x < 1.0f*/) {
            m_clickPoint = glm::vec3(screenPlaneRot * (o + r * x)) + screenPlanePos;
            glm::vec2 contact = glm::vec2(o.x + r.x*x, o.z + r.z*x) / glm::vec2(0.195f, 0.345f);
            if (contact.x > 0 && contact.x < 1 && contact.y > 0 && contact.y < 1) {
                RECT desktop;
                const HWND hDesktop = GetDesktopWindow();
                GetWindowRect(hDesktop, &desktop);
                SetCursorPos(desktop.right * contact.x, desktop.bottom * contact.y);
//                mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, desktop.right * contact.x, desktop.bottom * contact.y, 0, 0);
//                mouse_event(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE, desktop.right * contact.x, desktop.bottom * contact.y, 0, 0);
            }
        } else {
            m_clickPoint = glm::vec3(NAN);
        }
    }
}
