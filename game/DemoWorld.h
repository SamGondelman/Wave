#ifndef DEMOWORLD_H
#define DEMOWORLD_H

#include "World.h"

#include <winsock2.h>

#include "WaveDetector.h"

enum Mode {
    SERVER = 0,
    NONE,
    LIGHT_ON,
    BRIGHTNESS,
    SCROLL_UP,
    SCROLL_DOWN,
    SET_PLANE,
    CLICK,
    NUM_MODES
};

class DemoWorld : public World {
public:
    DemoWorld();
    ~DemoWorld();

    void makeCurrent() override;
    void update(float dt) override;
    void drawGeometry() override;
    void reset() override;

    WaveDetector &getWaveDetector() { return m_waveDetector; }
    void saveInstantGesture() { m_waveDetector.saveInstantGesture(m_handData); }
    void recordMotionGesture();

    void switchLight() { renderLightCube = !renderLightCube; }
    void setScreenPlane();
    void scroll(bool up);
    void checkClick();

    Mode m_mode;
private:
    // demo specific stuff
    bool renderLightCube { false };
    float lightCubeBrightness { 1 };
    glm::vec3 screenPlanePos { glm::vec3(NAN) };
    glm::mat4 screenPlaneRot;
    glm::vec3 m_clickPoint { glm::vec3(NAN) };

    // General stuff
    SOCKET clientSocket;
    bool serverStarted;

    WaveDetector m_waveDetector;
    bool m_isRecordingMotion { false };
    std::vector<glm::vec3> m_recordedMotion;

    std::vector<glm::vec3> m_handData;
    std::vector<glm::vec3> m_handPos;
    std::vector<glm::vec3> m_offsets;
    std::vector<float> m_segmentLengths;

    void startServer();
    void stopServer();

    void drawHand(const std::vector<glm::vec3> &data, float scale, bool savePos = false);
};

#endif // DEMOWORLD_H
