#ifndef WAVEDETECTOR_H
#define WAVEDETECTOR_H

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>

#include <QString>

#include "glm.hpp"

enum HandData {
    PALM_ROT = 0,
    FINGER1_1_ROT,
    FINGER1_2_ROT,
    FINGER2_1_ROT,
    FINGER2_2_ROT,
    FINGER3_1_ROT,
    FINGER3_2_ROT,
    FINGER4_1_ROT,
    FINGER4_2_ROT,
    THUMB_1_ROT,
    FOREARM_ROT,
    UPPERARM_ROT,
    NUM_HAND_DATA,
    FIRST_FINGER = FINGER1_1_ROT
};

enum HandPositions {
    PALM_POS = 0,
    ELBOW_POS,
    KNUCKLE_1_POS,
    JOINT1_1_POS,
    JOINT1_2_POS,
    KNUCKLE_2_POS,
    JOINT2_1_POS,
    JOINT2_2_POS,
    KNUCKLE_3_POS,
    JOINT3_1_POS,
    JOINT3_2_POS,
    KNUCKLE_4_POS,
    JOINT4_1_POS,
    JOINT4_2_POS,
    THUMB_KNUCKLE_POS,
    THUMB1_POS,
    THUMB2_POS,  // fake news
    WRIST_POS,
    NUM_HAND_POS,
    FIRST_KNUCKLE_POS = KNUCKLE_1_POS
};

enum Fingers {
    FINGER1 = 0,
    FINGER2,
    FINGER3,
    FINGER4,
    THUMB,
    WRIST,
    NUM_FINGERS
};

class Gesture {
public:
    Gesture() {}
    Gesture(int id) : m_id(id) {}
    virtual bool loadFromFile(QString path) = 0;
    virtual void saveToFile(QString path) = 0;
    int getID() { return m_id; }
    static const int NUM_DATA { 36 };
private:
    int m_id { -1 };
};

class InstantGesture : public Gesture {
public:
    InstantGesture(int id) : Gesture(id) {}
    InstantGesture(const QStringList &sp, float margin); // only for loading parts of motion gestures (no id)
    InstantGesture(int id, const std::vector<glm::vec3> &data, float margin,
                   bool usePosition, bool usePitch, bool useYaw, bool useRoll);
    InstantGesture(const std::vector<glm::vec3> &data, int index, float margin,
                   bool usePosition, bool usePitch, bool useYaw, bool useRoll); // only for loading parts of motion gestures when recording
    bool loadFromFile(QString path) override;
    void saveToFile(QString path) override;
    bool checkGesture(const std::vector<glm::vec3> &data);
    const float &getMargin() { return m_margin; }
    const std::vector<glm::vec3> &getData() { return m_data; }
private:
    float m_margin { 0.1 };
    std::vector<glm::vec3> m_data;
};

class MotionGesture : public Gesture {
public:
    MotionGesture(int id) : Gesture(id) {}
    MotionGesture(int id, const std::vector<glm::vec3> &data, float margin, float duration,
                   bool usePosition, bool usePitch, bool useYaw, bool useRoll);
    bool loadFromFile(QString path) override;
    void saveToFile(QString path) override;
    int getNumFrames() { return m_frames.size(); }
    const std::vector<std::pair<std::shared_ptr<InstantGesture>, float>> &getFrames() { return m_frames; }
private:
    std::vector<std::pair<std::shared_ptr<InstantGesture>, float>> m_frames;
};

class WaveDetector {
public:
    WaveDetector(bool loadGestures);
    void update(float dt, const std::vector<glm::vec3> &data);

    void saveInstantGesture(std::vector<glm::vec3> data);
    void saveMotionGesture(std::vector<glm::vec3> &data);
    void changeVisualizedID(int change);
    int getVisID() { return m_visID; }
    int getVisFrame() { return m_visFrame; }
    std::vector<std::shared_ptr<InstantGesture>> &getInstantGestures() { return m_instantGestures; }
    std::vector<std::shared_ptr<MotionGesture>> &getMotionGestures() { return m_motionGestures; }

    std::unordered_map<int, QString> &getGestureNames() { return m_gestureNames; }
    std::unordered_set<int> &getCurrentGestures() { return m_currentGestures; }

private:
    int m_gestureID { 0 };
    int m_visID { 0 };
    int m_visFrame { 0 };
    float m_visFrameTime { 0.0f };

    // Gesture data
    std::unordered_map<int, QString> m_gestureNames;
    std::vector<std::shared_ptr<InstantGesture>> m_instantGestures;
    std::vector<std::shared_ptr<MotionGesture>> m_motionGestures;

    // Detection data
    std::vector<std::pair<int, float>> m_currentFramesDurations;
    std::unordered_set<int> m_currentGestures;

};

#endif // WAVEDETECTOR_H
