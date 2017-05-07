#include "WaveDetector.h"

#include <iostream>

#include <QDirIterator>
#include <QTextStream>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>

#include "InstantGestureDialog.h"
#include "MotionGestureDialog.h"

#define M_2PI 6.28318530718f

WaveDetector::WaveDetector(bool loadGestures)
{
    if (loadGestures) {
        QString gestureDir = QDir::currentPath() + "/gestures";
        {
            QDirIterator instant(gestureDir + "/instant", QStringList() << "*.txt", QDir::Files);
            while (instant.hasNext()) {
                QString path = instant.next();
                int id = m_gestureID++;
                std::shared_ptr<InstantGesture> g = std::make_shared<InstantGesture>(id);
                if (g->loadFromFile(path)) {
                    // TODO: reserve expected size first to prevent copies
                    m_instantGestures.push_back(g);
                    QStringList sp = path.split("/");
                    m_gestureNames[id] = sp[sp.length() - 1].split(".")[0];
                }
            }
        }
        {
            QDirIterator motion(gestureDir + "/motion", QStringList() << "*.txt", QDir::Files);
            while (motion.hasNext()) {
                QString path = motion.next();
                int id = m_gestureID++;
                std::shared_ptr<MotionGesture> g = std::make_shared<MotionGesture>(id);
                if (g->loadFromFile(path)) {
                    // TODO: reserve expected size first to prevent copies
                    m_motionGestures.push_back(g);
                    QStringList sp = path.split("/");
                    m_gestureNames[id] = sp[sp.length() - 1].split(".")[0];
                    m_currentFramesDurations.emplace_back(-1, 0.0f);
                }
            }
        }
    }
}

InstantGesture::InstantGesture(const QStringList &sp, float margin) :
    m_margin(margin)
{
    m_data.resize(NUM_DATA / 3);
    float *data = (float *) m_data.data();
    for (int i = 0; i < NUM_DATA; i++) {
        data[i] = std::stof(sp[i].toStdString().c_str());
    }
}

InstantGesture::InstantGesture(int id, const std::vector<glm::vec3> &data, float margin,
               bool usePosition, bool usePitch, bool useYaw, bool useRoll) :
    Gesture(id), m_margin(margin), m_data(data)
{
    glm::vec3 offset(0.0f);
    if (!usePitch) {
        offset.x = m_data[PALM_ROT].x;
        m_data[PALM_ROT].x = NAN;
    }
    if (!useYaw) {
        offset.y = m_data[PALM_ROT].y;
        m_data[PALM_ROT].y = NAN;
    }
    if (!useRoll) {
        offset.z = m_data[PALM_ROT].z;
        m_data[PALM_ROT].z = NAN;
    }
    for (int i = FINGER1_1_ROT; i <= THUMB_1_ROT; i++) {
        m_data[i] =  glm::mod(m_data[i] - offset, M_2PI);
    }
    if (!usePosition) {
        m_data[FOREARM_ROT] = glm::vec3(NAN);
        m_data[UPPERARM_ROT] = glm::vec3(NAN);
    }
}

InstantGesture::InstantGesture(const std::vector<glm::vec3> &data, int index, float margin,
               bool usePosition, bool usePitch, bool useYaw, bool useRoll) :
    m_margin(margin)
{
    m_data.assign(data.begin() + index, data.begin() + index + NUM_DATA / 3);
    glm::vec3 offset(0.0f);
    if (!usePitch) {
        offset.x = m_data[PALM_ROT].x;
        m_data[PALM_ROT].x = NAN;
    }
    if (!useYaw) {
        offset.y = m_data[PALM_ROT].y;
        m_data[PALM_ROT].y = NAN;
    }
    if (!useRoll) {
        offset.z = m_data[PALM_ROT].z;
        m_data[PALM_ROT].z = NAN;
    }
    for (int i = FINGER1_1_ROT; i <= THUMB_1_ROT; i++) {
        m_data[i] =  glm::mod(m_data[i] - offset, M_2PI);
    }
    if (!usePosition) {
        m_data[FOREARM_ROT] = glm::vec3(NAN);
        m_data[UPPERARM_ROT] = glm::vec3(NAN);
    }
}

bool InstantGesture::loadFromFile(QString path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Can't open " << path.toStdString() << " for reading." << std::endl;
        return false;
    }

    QTextStream in(&file);
    int l = 0;
    while(!in.atEnd() && l < 2) {
        QString line = in.readLine();
        QStringList sp = line.split(" ");
        if (l == 0 && sp.length() == 1) {
            float margin = std::stof(line.toStdString().c_str());
            if (margin > 0.0f && margin < 1.0f) {
                m_margin = margin;
            }
        } else if (sp.length() == NUM_DATA) {
            m_data.resize(NUM_DATA / 3);
            float *data = (float *) m_data.data();
            for (int i = 0; i < NUM_DATA; i++) {
                data[i] = std::stof(sp[i].toStdString().c_str());
            }
        } else {
            std::cout << "Improperly formatted Instant Gesture: " << path.toStdString() << std::endl;
            return false;
        }
        l++;
    }

    file.close();
    return true;
}

void InstantGesture::saveToFile(QString path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        std::cout << "Can't open " << path.toStdString() << " for writing." << std::endl;
        return;
    }

    QTextStream out(&file);
    out << m_margin << "\n";
    for (size_t i = 0; i < m_data.size(); i++) {
        out << m_data[i].x << " " << m_data[i].y << " " << m_data[i].z;
        if (i != m_data.size() - 1) out << " ";
    }
    file.close();
}

bool InstantGesture::checkGesture(const std::vector<glm::vec3> &data) {
    float *dataF = (float *) data.data();
    float *m_dataF = (float *) m_data.data();
    glm::vec3 offset(0.0f);
    if (glm::isnan(m_dataF[3 * PALM_ROT])) offset.x = dataF[3 * PALM_ROT];
    if (glm::isnan(m_dataF[3 * PALM_ROT + 1])) offset.y = dataF[3 * PALM_ROT + 1];
    if (glm::isnan(m_dataF[3 * PALM_ROT + 2])) offset.z = dataF[3 * PALM_ROT + 2];
    for (size_t i = 0; i < 3 * data.size(); i++) {
        if (glm::isnan(m_dataF[i])) continue;
        float d = glm::mod(dataF[i] - (i < 3 * FOREARM_ROT ? offset[i % 3] : 0.0f), M_2PI);
        if (1.0f - glm::abs(glm::abs(m_dataF[i] - d) / M_PI - 1.0f) >= m_margin) {
            return false;
        }
    }
    return true;
}

MotionGesture::MotionGesture(int id, const std::vector<glm::vec3> &data, float margin, float duration,
               bool usePosition, bool usePitch, bool useYaw, bool useRoll) :
    Gesture(id)
{
    // might sometimes not include last frame because data.size() % NUM_DATA != 0?
    int frames = data.size() / NUM_DATA;
    m_frames.reserve(frames);
    for (int i = 0; i < frames; i++) {
        // TODO: make InstantGesture constructor that doesn't use id
        std::shared_ptr<InstantGesture> g =
                std::make_shared<InstantGesture>(data, i * NUM_DATA, margin, usePosition,
                                                 usePitch, useYaw, useRoll);
        m_frames.emplace_back(g, duration);
    }
}

bool MotionGesture::loadFromFile(QString path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        std::cout << "Can't open " << path.toStdString() << " for reading." << std::endl;
        return false;
    }

    QTextStream in(&file);
    int l = 0;
    float margin = -1.0f, duration = -1.0f;
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList sp = line.split(" ");
        if (l % 2 == 0 && sp.length() == 2) {
            margin = std::stof(sp[0].toStdString().c_str());
            if (margin <= 0.0f && margin >= 1.0f) margin = 0.1f;
            duration = std::stof(sp[1].toStdString().c_str());
            if (duration <= 0.0f) duration = 0.5f;
        } else if (l % 2 == 1 && sp.length() == NUM_DATA) {
            std::shared_ptr<InstantGesture> g = std::make_shared<InstantGesture>(sp, margin);
            m_frames.emplace_back(g, duration);
        } else {
            std::cout << "Improperly formatted Motion Gesture: " << path.toStdString() << std::endl;
            return false;
        }
        l++;
    }

    file.close();
    if (l > 2 && l % 2 == 0) {
        return true;
    } else {
        std::cout << "Improperly formatted Motion Gesture: " << path.toStdString() << std::endl;
        return false;
    }
}

void MotionGesture::saveToFile(QString path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        std::cout << "Can't open " << path.toStdString() << " for writing." << std::endl;
        return;
    }

    QTextStream out(&file);
    for (size_t i = 0; i < m_frames.size(); i++) {
        out << m_frames[i].first->getMargin() << " " << m_frames[i].second << "\n";
        for (size_t j = 0; j < m_frames[i].first->getData().size(); j++) {
            out << m_frames[i].first->getData()[j].x << " " << m_frames[i].first->getData()[j].y <<
                   " " << m_frames[i].first->getData()[j].z;
            if (j != m_frames[i].first->getData().size() - 1) out << " ";
        }
        if (i != m_frames.size() - 1) out << "\n";
    }
    file.close();
}

void WaveDetector::changeVisualizedID(int change) {
    m_visID = glm::mod((float)(m_visID + change),
                       (float)(m_instantGestures.size() + m_motionGestures.size() + 1));
    if (m_visID != 0) {
        int id = (m_visID <= (int) m_instantGestures.size()) ? m_instantGestures[m_visID - 1]->getID() :
                m_motionGestures[m_visID - 1 - m_instantGestures.size()]->getID();
        std::cout << "Visualizing " << m_gestureNames[id].toStdString() << std::endl;
    } else {
        std::cout << "Visualizer off" << std::endl;
    }
    m_visFrame = 0;
    m_visFrameTime = 0.0f;
}

void WaveDetector::saveInstantGesture(std::vector<glm::vec3> data) {
    InstantGestureDialog d;
    int result = d.exec();
    if (result == QDialog::Accepted) {
        // if visualizing a motion gesture, increment visID
        if (m_visID > (int) m_instantGestures.size()) {
            m_visID++;
        }

        // Add to current gestures
        int id = m_gestureID++;
        std::shared_ptr<InstantGesture> g =
                std::make_shared<InstantGesture>(id, data, d.getMargin(), d.getUsePosition(),
                                                 d.getUsePitch(), d.getUseYaw(), d.getUseRoll());
        m_instantGestures.push_back(g);
        m_gestureNames[id] = d.getName();

        // Give option to save
        QString fileName = QFileDialog::getSaveFileName(QApplication::activeWindow(),
                QString("Save Instant Gesture"), QString(QDir::currentPath() + "/gestures/instant/" + d.getName() + ".txt"),
                QString("Gesture File (*.txt);;All Files (*)"));
        if (!fileName.isEmpty()) g->saveToFile(fileName);
    }
}

void WaveDetector::saveMotionGesture(std::vector<glm::vec3> &data) {
    MotionGestureDialog d;
    int result = d.exec();
    if (result == QDialog::Accepted) {
        // Add to current gestures
        int id = m_gestureID++;
        std::shared_ptr<MotionGesture> g =
                std::make_shared<MotionGesture>(id, data, d.getMargin(), d.getDuration(), d.getUsePosition(),
                                                 d.getUsePitch(), d.getUseYaw(), d.getUseRoll());
        m_motionGestures.push_back(g);
        m_gestureNames[id] = d.getName();
        m_currentFramesDurations.emplace_back(-1, 0.0f);

        // Give option to save
        QString fileName = QFileDialog::getSaveFileName(QApplication::activeWindow(),
                QString("Save Motion Gesture"), QString(QDir::currentPath() + "/gestures/motion/" + d.getName() + ".txt"),
                QString("Gesture File (*.txt);;All Files (*)"));
        if (!fileName.isEmpty()) g->saveToFile(fileName);
    }
}

void WaveDetector::update(float dt, const std::vector<glm::vec3> &data) {
    m_currentGestures.clear();

    // Update visualizer frame if motion gesture selected
    if (m_visID > (int) m_instantGestures.size()) {
        if (m_visFrameTime > m_motionGestures[m_visID - m_instantGestures.size() - 1]->getFrames()[m_visFrame].second) {
            m_visFrameTime = 0.0f;
            m_visFrame = (m_visFrame + 1) % m_motionGestures[m_visID - m_instantGestures.size() - 1]->getNumFrames();
        }
        m_visFrameTime += dt;
    }

    // Instant gestures
    for (auto &i : m_instantGestures) {
        if (i->checkGesture(data)) {
            m_currentGestures.insert(i->getID());
        }
    }

    // Motion gestures
    for (size_t i = 0; i < m_motionGestures.size(); i++) {
        int currFrame = m_currentFramesDurations[i].first;
        int nextFrame = currFrame + 1;
        // if next frame detected
        //   if last frame, reset timer, frame, add to currentGestures
        //   if not last frame, reset timer, increment frame
        // else if current frame != -1, check current frame
        //   if detected, increment timer, check timer
        //   if not detected reset timer and frame
        if (m_motionGestures[i]->getFrames()[nextFrame].first->checkGesture(data)) {
            if (nextFrame == m_motionGestures[i]->getNumFrames() - 1) {
                m_currentFramesDurations[i] = std::pair<int, float>(-1, 0.0f);
                m_currentGestures.insert(m_motionGestures[i]->getID());
            } else {
                m_currentFramesDurations[i] = std::pair<int, float>(nextFrame, 0.0f);
            }
        } else if (currFrame != -1) {
            if (m_motionGestures[i]->getFrames()[currFrame].first->checkGesture(data)) {
                m_currentFramesDurations[i].second += dt;
                if (m_currentFramesDurations[i].second > m_motionGestures[i]->getFrames()[currFrame].second) {
                    m_currentFramesDurations[i] = std::pair<int, float>(-1, 0.0f);
                }
            } else {
                m_currentFramesDurations[i] = std::pair<int, float>(-1, 0.0f);
            }
        }
    }
//    std::cout << m_currentGestures.size() << std::endl;
}
