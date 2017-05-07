#ifndef VIEW_H
#define VIEW_H

#include "GL/glew.h"
#include <qgl.h>
#include <QTime>
#include <QTimer>
#include <memory>
#include <set>

#include "glm/glm.hpp"

class CS123Shader;
class FBO;
class SphereMesh;
class CubeMesh;
class ConeMesh;
class CylinderMesh;
class FullScreenQuad;
class Light;
class Player;
class ParticleSystem;
class Texture2D;
class DemoWorld;
class Entity;

enum DrawMode {
    POSITION = 0,
    NORMAL,
    AMBIENT,
    DIFFUSE,
    SPECULAR,
    LIGHTS,
    NO_HDR,
    BRIGHT,
    BRIGHT_BLUR,
    NO_DISTORTION,
    DEFAULT
};

class View : public QGLWidget {
    Q_OBJECT

public:
    View(QWidget *parent);
    ~View();

    static float m_globalTime;
    static std::unique_ptr<Player> m_player;

    // Shapes
    static std::unique_ptr<SphereMesh> m_sphere;
    static std::unique_ptr<CubeMesh> m_cube;
    static std::unique_ptr<ConeMesh> m_cone;
    static std::unique_ptr<CylinderMesh> m_cylinder;

    static std::set<int> m_pressedKeys;

private:
    int m_width;
    int m_height;

    QTime m_time;
    QTimer m_timer;

    float m_fps;
    QTimer m_FPStimer;

    std::shared_ptr<DemoWorld> m_world;

    // Game state
    DrawMode m_drawMode;

    // FBOs and shaders
    std::unique_ptr<FBO> m_deferredBuffer;
    std::unique_ptr<CS123Shader> m_rockProgram;

    std::unique_ptr<FBO> m_lightingBuffer;
    std::unique_ptr<CS123Shader> m_lightingProgram;

    std::unique_ptr<FBO> m_distortionBuffer;
    std::unique_ptr<CS123Shader> m_distortionStencilProgram;
    std::unique_ptr<CS123Shader> m_distortionProgram;

    GLuint m_fullscreenQuadVAO;
    std::unique_ptr<CS123Shader> m_textureProgram;

    std::unique_ptr<CS123Shader> m_brightProgram;

    // https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
    std::shared_ptr<FBO> m_vblurBuffer;
    std::shared_ptr<FBO> m_hblurBuffer;
    std::shared_ptr<CS123Shader> m_horizontalBlurProgram;
    std::shared_ptr<CS123Shader> m_verticalBlurProgram;

    std::unique_ptr<CS123Shader> m_bloomProgram;
    float m_exposure;
    bool m_useAdaptiveExposure;

    // Lights
    std::unique_ptr<SphereMesh> m_lightSphere;
    std::unique_ptr<FullScreenQuad> m_fullscreenQuad;

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void tick();
    void printFPS();
};

#endif // VIEW_H
