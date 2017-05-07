#ifndef DEMOWORLD_H
#define DEMOWORLD_H

#include "World.h"

#include <winsock2.h>

#include "WaveDetector.h"

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

private:
    SOCKET clientSocket;
    bool serverStarted;

    WaveDetector m_waveDetector;
    bool m_isRecordingMotion { false };
    std::vector<glm::vec3> m_recordedMotion;

    std::vector<glm::vec3> m_handData;
    std::vector<glm::vec3> m_offsets;
    std::vector<float> m_segmentLengths;

    void startServer();
    void stopServer();

    void drawHand(const std::vector<glm::vec3> &data, float scale = 0.01f);
};

#endif // DEMOWORLD_H
