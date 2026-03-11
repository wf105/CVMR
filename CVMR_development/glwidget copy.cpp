#include "glwidget.h"
#include "EXAMPLEcamera_preprocessor.h"
#include <QMatrix4x4>
#include <cmath>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QtMath>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <QFile>
#include <QTextStream>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    qDebug() << "GLWidget constructor called";
    setFocusPolicy(Qt::StrongFocus);

    m_intrinsic = QMatrix3x3();
    m_intrinsic(0,0) = 500.0f;
    m_intrinsic(1,1) = 500.0f;
    m_intrinsic(0,2) = 320.0f;
    m_intrinsic(1,2) = 240.0f;
    m_intrinsic(2,2) = 1.0f;

    m_extrinsicRotation = QMatrix3x3();
    m_extrinsicRotation(0,0) = 1.0f; m_extrinsicRotation(0,1) = 0.0f; m_extrinsicRotation(0,2) = 0.0f;
    m_extrinsicRotation(1,0) = 0.0f; m_extrinsicRotation(1,1) = 1.0f; m_extrinsicRotation(1,2) = 0.0f;
    m_extrinsicRotation(2,0) = 0.0f; m_extrinsicRotation(2,1) = 0.0f; m_extrinsicRotation(2,2) = 1.0f;
    m_extrinsicTranslation = QVector3D(0.0f, 2.0f, 8.0f);

    // --- Image texturing additions ---
    m_groundPlaneVao = 0;
    m_groundPlaneVbo = 0;
    // --- end image texturing additions ---
}

GLWidget::~GLWidget() {
    qDebug() << "GLWidget destructor called";
    makeCurrent();
    glDeleteVertexArrays(1, &m_gridVao);
    glDeleteBuffers(1, &m_gridVbo);
    glDeleteVertexArrays(1, &m_frustumVao);
    glDeleteBuffers(1, &m_frustumVbo);
    glDeleteVertexArrays(1, &m_raysVao);
    glDeleteBuffers(1, &m_raysVbo);
    // --- Image texturing additions ---
    glDeleteVertexArrays(1, &m_groundPlaneVao);
    glDeleteBuffers(1, &m_groundPlaneVbo);
    for (auto tex : m_cameraTextures) {
        if (tex) delete tex;
    }
    m_cameraTextures.clear();
    // --- end image texturing additions ---
    doneCurrent();
}

void GLWidget::initializeGL() {
    qDebug() << "initializeGL called";
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glLineWidth(.05f); // Make lines thicker for visibility

    // Compile shader, openGL actual renderer code
    bool vs = m_shader.addShaderFromSourceFile(QOpenGLShader::Vertex, "shaders/basic.vs");
    bool fs = m_shader.addShaderFromSourceFile(QOpenGLShader::Fragment, "shaders/basic.fs");
    bool link = m_shader.link();
    qDebug() << "Shader vertex loaded:" << vs << "fragment loaded:" << fs << "linked:" << link;

    // grid VAO/VBO
    glGenVertexArrays(1, &m_gridVao);
    glGenBuffers(1, &m_gridVbo);

    // Frustum VAO/VBO
    glGenVertexArrays(1, &m_frustumVao);
    glGenBuffers(1, &m_frustumVbo);

    // Rays VAO/VBO
    glGenVertexArrays(1, &m_raysVao);
    glGenBuffers(1, &m_raysVbo);

    // --- Image texturing additions ---
    glGenVertexArrays(1, &m_groundPlaneVao);
    glGenBuffers(1, &m_groundPlaneVbo);
    // --- end image texturing additions ---

    updateGrid();
    updateFrustum();
    // Axis VAO/VBO
    glGenVertexArrays(1, &m_axisVao);
    glGenBuffers(1, &m_axisVbo);
    updateAxes();
    m_glInitialized = true;\
    updateGroundPlanes(); 
}

void GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void GLWidget::paintGL() {
    qDebug() << "paintGL called";
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.0f); // Make lines thicker for visibility
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 projection;
    // Increase near plane to avoid grid clipping, and use a larger far plane
    projection.perspective(45.0f, float(width())/height(), 0.01f, 200.0f);

    // View matrix (user-controlled)
    QMatrix4x4 view;
    // Move camera up and back for a better grid view
    view.translate(-m_viewX, -m_viewY - 2.0, -m_viewZ - 10.0);
    view.scale(m_viewScale);
    view.rotate(m_viewYaw, 0, 1, 0);
    view.rotate(m_viewPitch, 1, 0, 0);

    m_shader.bind();
    m_shader.setUniformValue("projection", projection);
    m_shader.setUniformValue("view", view);

    // Draw grid
    m_shader.setUniformValue("model", QMatrix4x4());
    m_shader.setUniformValue("color", QVector3D(0.5f, 0.5f, 0.5f)); // Even brighter grid
    glBindVertexArray(m_gridVao);
    glDrawArrays(GL_LINES, 0, m_gridVertices.size());

    // Draw global axes
    glLineWidth(4.0f); //for clearer axis
    glBindVertexArray(m_axisVao);
    m_shader.setUniformValue("color", QVector3D(1.0f, 0.0f, 0.0f));
    glDrawArrays(GL_LINES, 0, 2);
    m_shader.setUniformValue("color", QVector3D(0.0f, 1.0f, 0.0f));
    glDrawArrays(GL_LINES, 2, 2);
    m_shader.setUniformValue("color", QVector3D(0.0f, 0.0f, 1.0f));
    glDrawArrays(GL_LINES, 4, 2);
    glBindVertexArray(0);
    glLineWidth(2.0f);

    // --- Image texturing additions: draw ground plane images ---
    drawGroundPlanes();
    // --- end image texturing additions ---

    // Draw all frustums and their local axes
    for (size_t i = 0; i < m_cameraExtrinsics.size(); ++i) {
        QMatrix4x4 model = m_cameraExtrinsics[i];   // model is the "truly used" extrinsic
        m_shader.setUniformValue("model", model);  // THIS might be the most subtly important line! the open GL logic is using the extrinsic values through the shader 

        // Draw frustum
        if (int(i) == m_currentExtrinsicIndex) {
            m_shader.setUniformValue("color", QVector3D(1.0f, 0.5f, 0.0f)); // Orange for selected
        } else {
            m_shader.setUniformValue("color", QVector3D(1.0f, 1.0f, 1.0f)); // White for others
        }
        glBindVertexArray(m_frustumVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_frustumVbo);
        glBufferData(GL_ARRAY_BUFFER, m_allFrustumVertices[i].size() * sizeof(QVector3D), m_allFrustumVertices[i].data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINES, 0, m_allFrustumVertices[i].size());

        // Draw local axes at camera origin
        float axisLen = 0.3f;
        QVector3D axes[6] = {
            QVector3D(0,0,0), QVector3D(axisLen,0,0), // X
            QVector3D(0,0,0), QVector3D(0,axisLen,0), // Y
            QVector3D(0,0,0), QVector3D(0,0,axisLen)  // Z
        };
        GLuint axisVbo;
        glGenBuffers(1, &axisVbo);
        glBindBuffer(GL_ARRAY_BUFFER, axisVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        m_shader.setUniformValue("color", QVector3D(1,0,0));
        glDrawArrays(GL_LINES, 0, 2);
        m_shader.setUniformValue("color", QVector3D(0,1,0));
        glDrawArrays(GL_LINES, 2, 2);
        m_shader.setUniformValue("color", QVector3D(0,0,1));
        glDrawArrays(GL_LINES, 4, 2);

        glDeleteBuffers(1, &axisVbo);
    }

    // --- Draw camera rays if enabled ---
    if (m_showRays && !m_raysVertices.empty()) {
        m_shader.setUniformValue("model", QMatrix4x4());
        m_shader.setUniformValue("color", QVector3D(1.0f, 1.0f, 0.0f)); // Yellow rays
        glBindVertexArray(m_raysVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_raysVbo);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glDrawArrays(GL_LINES, 0, m_raysVertices.size());
        glBindVertexArray(0);
    }

    glBindVertexArray(0);
    m_shader.release();
}

// --- Image texturing additions ---

void GLWidget::setCameraImages(const std::vector<QString>& imagePaths) {
    makeCurrent();
    // Ensure m_cameraTextures has at least as many as m_cameraExtrinsics
    if (m_cameraTextures.size() < m_cameraExtrinsics.size()) {
        m_cameraTextures.resize(m_cameraExtrinsics.size(), nullptr);
    }
    // Assign each loaded image to the currently selected camera and onward
    int startIdx = m_currentExtrinsicIndex;
    for (size_t i = 0; i < imagePaths.size(); ++i) {
        int camIdx = startIdx + int(i);
        if (camIdx >= int(m_cameraExtrinsics.size())) break;
        // Delete old texture if present
        if (m_cameraTextures[camIdx]) {
            delete m_cameraTextures[camIdx];
            m_cameraTextures[camIdx] = nullptr;
        }
        QImage img(imagePaths[i]);
        if (!img.isNull()) {
            QOpenGLTexture* tex = new QOpenGLTexture(img.mirrored());
            tex->setMinificationFilter(QOpenGLTexture::Linear);
            tex->setMagnificationFilter(QOpenGLTexture::Linear);
            tex->setWrapMode(QOpenGLTexture::ClampToEdge);
            m_cameraTextures[camIdx] = tex;
        }
    }
    updateGroundPlanes();
    update();
}

void GLWidget::clearCameraImages() {
    makeCurrent();
    for (auto tex : m_cameraTextures) {
        if (tex) delete tex;
    }
    m_cameraTextures.clear();
    m_cameraImagePaths.clear();
    updateGroundPlanes();
    update();
}

// Compute ground plane quads for each camera
void GLWidget::updateGroundPlanes() {
    if (!m_glInitialized) {
        qDebug() << "updateGroundPlanes: OpenGL not initialized yet, skipping buffer upload";
        return;
    }

    qDebug() << "updateGroundPlanes called, m_cameraExtrinsics.size()=" << m_cameraExtrinsics.size();
    m_groundPlaneVertices.clear();
    m_groundPlaneTexCoords.clear();

    float w = 640.0f, h = 480.0f;
    float fx = (m_intrinsic(0, 0) != 0) ? m_intrinsic(0, 0) : 500.0f;
    float fy = (m_intrinsic(1, 1) != 0) ? m_intrinsic(1, 1) : 500.0f;
    float cx = (m_intrinsic(0, 2) != 0) ? m_intrinsic(0, 2) : w / 2.0f;
    float cy = (m_intrinsic(1, 2) != 0) ? m_intrinsic(1, 2) : h / 2.0f;

    qDebug() << "fx:" << fx << "fy:" << fy << "cx:" << cx << "cy:" << cy;

    float z_plane = 0.0f;

    for (size_t c = 0; c < m_cameraExtrinsics.size(); ++c) {
        qDebug() << "Processing camera" << c;
        std::vector<QVector3D> quad(4);
        std::vector<QVector2D> tex(4);

        for (int i = 0; i < 4; ++i) {
            float x = (i == 1 || i == 2) ? w : 0;
            float y = (i >= 2) ? h : 0;
            QVector3D camPt((x - cx) / fx, (y - cy) / fy, 1.0f);
            QMatrix4x4 ext = m_cameraExtrinsics[c];
            QVector3D origin = ext * QVector3D(0, 0, 0);
            QVector3D dir = ext.mapVector(camPt).normalized();
            float t = (z_plane - origin.z()) / dir.z();
            QVector3D worldPt = origin + dir * t;
            quad[i] = worldPt;
            tex[i] = QVector2D(i == 1 || i == 2 ? 1.0f : 0.0f, i >= 2 ? 1.0f : 0.0f);

            qDebug() << "  Corner" << i << ": x=" << x << "y=" << y << "camPt=" << camPt << "origin=" << origin << "dir=" << dir << "t=" << t << "worldPt=" << worldPt;
        }
        for (int i = 0; i < 4; ++i) {
            m_groundPlaneVertices.push_back(quad[i]);
            m_groundPlaneTexCoords.push_back(tex[i]);
        }
    }

    qDebug() << "m_groundPlaneVertices.size()=" << m_groundPlaneVertices.size();
    glBindVertexArray(m_groundPlaneVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_groundPlaneVbo);

    std::vector<float> data;
    for (size_t i = 0; i < m_groundPlaneVertices.size(); ++i) {
        data.push_back(m_groundPlaneVertices[i].x());
        data.push_back(m_groundPlaneVertices[i].y());
        data.push_back(m_groundPlaneVertices[i].z());
        data.push_back(m_groundPlaneTexCoords[i].x());
        data.push_back(m_groundPlaneTexCoords[i].y());
    }
    qDebug() << "Uploading" << data.size() << "floats to ground plane VBO";
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void GLWidget::drawGroundPlanes() {
    if (m_cameraTextures.empty() || m_groundPlaneVertices.empty())
        return;

    glBindVertexArray(m_groundPlaneVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_groundPlaneVbo);

    // Assume shader uses layout(location=0) for pos, location=1 for texcoord
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Draw all images that have textures, for all cameras with extrinsics and images
    for (size_t i = 0; i < m_cameraTextures.size(); ++i) {
        if (!m_cameraTextures[i]) continue;
        if (i >= m_cameraExtrinsics.size()) continue; // Only draw if camera exists
        m_cameraTextures[i]->bind(0);
        m_shader.setUniformValue("useTexture", 1);
        m_shader.setUniformValue("tex", 0);
        m_shader.setUniformValue("model", QMatrix4x4());
        m_shader.setUniformValue("color", QVector3D(1,1,1));
        glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
        m_cameraTextures[i]->release();
    }
    m_shader.setUniformValue("useTexture", 0); // Reset for other draws
    glBindVertexArray(0);
}
// --- end image texturing additions ---

void GLWidget::updateGrid() {
    m_gridVertices.clear();
    //grid size
    const int N = 1000;
    const float step = 100.0f;
    for (int i = -N; i <= N; ++i) {
        m_gridVertices.push_back(QVector3D(i * step, -N * step, 0));
        m_gridVertices.push_back(QVector3D(i * step, N * step, 0));
        m_gridVertices.push_back(QVector3D(-N * step, i * step, 0));
        m_gridVertices.push_back(QVector3D(N * step, i * step, 0));
    }
    glBindVertexArray(m_gridVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVbo);
    glBufferData(GL_ARRAY_BUFFER, m_gridVertices.size() * sizeof(QVector3D), m_gridVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void GLWidget::updateFrustum() {
    // This function is now only for the "main" frustum, but we want to update all
    // So, call setCameraExtrinsics again to regenerate all frustums
    setCameraExtrinsics(m_cameraExtrinsics);
}

//ALL righty. this is important below AlSO BAD NAMING. bc of haste, the below is called set Camera Extrinsics
// but it really should not be. This should be called setCameraFrustums or ...
// Note that it grabs extrinsic info from verts
void GLWidget::setCameraExtrinsics(const std::vector<QMatrix4x4>& extrinsics) {
    qDebug() << "setCameraExtrinsics called, extrinsics.size()=" << extrinsics.size();
    m_cameraExtrinsics = extrinsics;
    m_allFrustumVertices.clear();

    // "Image" size
    float w = 640.0f, h = 480.0f;

    // Intrinsics fallback logic
    float fx = (m_intrinsic(0, 0) != 0) ? m_intrinsic(0, 0) : 500.0f;
    float fy = (m_intrinsic(1, 1) != 0) ? m_intrinsic(1, 1) : 500.0f;
    float cx = (m_intrinsic(0, 2) != 0) ? m_intrinsic(0, 2) : w / 2.0f;
    float cy = (m_intrinsic(1, 2) != 0) ? m_intrinsic(1, 2) : h / 2.0f;

    float nearZ = 0.05f, farZ = 0.2f;  //Change here to make Camera frustums more visible

    auto getCorner = [&](float x, float y, float z) -> QVector3D {
        return QVector3D((x - cx) * z / fx, (y - cy) * z / fy, z);
    };

    for (size_t c = 0; c < m_cameraExtrinsics.size(); ++c) {
        std::vector<QVector3D> verts;
        QVector3D nc[4], fc[4];
        nc[0] = getCorner(0, 0, nearZ);
        nc[1] = getCorner(w, 0, nearZ);
        nc[2] = getCorner(w, h, nearZ);
        nc[3] = getCorner(0, h, nearZ);
        fc[0] = getCorner(0, 0, farZ);
        fc[1] = getCorner(w, 0, farZ);
        fc[2] = getCorner(w, h, farZ);
        fc[3] = getCorner(0, h, farZ);

        for (int i = 0; i < 4; ++i)
            verts.push_back(nc[i]), verts.push_back(nc[(i + 1) % 4]);
        for (int i = 0; i < 4; ++i)
            verts.push_back(fc[i]), verts.push_back(fc[(i + 1) % 4]);
        for (int i = 0; i < 4; ++i)
            verts.push_back(nc[i]), verts.push_back(fc[i]);

        m_allFrustumVertices.push_back(verts);
    }

    updateGroundPlanes(); // --- Image texturing additions: update ground quads when cameras change
    update();
}

void GLWidget::updateAxes() {
    m_axisVertices.clear();
    float len = 10.0f; // Make global axes much longer

    // X axis (red)
    m_axisVertices.push_back(QVector3D(0, 0, 0));
    m_axisVertices.push_back(QVector3D(len, 0, 0));

    // Y axis (green)
    m_axisVertices.push_back(QVector3D(0, 0, 0));
    m_axisVertices.push_back(QVector3D(0, len, 0));

    // Z axis (blue)
    m_axisVertices.push_back(QVector3D(0, 0, 0));
    m_axisVertices.push_back(QVector3D(0, 0, len));

    glBindVertexArray(m_axisVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_axisVbo);
    glBufferData(GL_ARRAY_BUFFER, m_axisVertices.size() * sizeof(QVector3D), m_axisVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void GLWidget::toggleRays() {
    m_showRays = !m_showRays;
    drawRays(); // This is for better logic, makes multipurpose
}

void GLWidget::drawRays() {
    qDebug() << "drawRays called, m_showRays=" << m_showRays;
    /* Toggle rendering state
    m_showRays = !m_showRays; //ha */
    if (!m_showRays) return; // Better for rays

    if (m_showRays) {
        m_raysVertices.clear();
        const float rayLength = 100000.0f;

        // For each camera extrinsic, compute the ray from camera center along its forward (Z) axis
        for (const QMatrix4x4& extr : m_cameraExtrinsics) {
            // Camera center in world coordinates
            QVector3D camOrigin = extr * QVector3D(0, 0, 0);
            // Camera forward direction (third column of rotation part)
            QVector3D forward = extr.mapVector(QVector3D(0, 0, 1)).normalized();
            QVector3D rayEnd = camOrigin + forward * rayLength;

            m_raysVertices.push_back(camOrigin);
            m_raysVertices.push_back(rayEnd);
        }

        glBindVertexArray(m_raysVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_raysVbo);
        glBufferData(GL_ARRAY_BUFFER, m_raysVertices.size() * sizeof(QVector3D), m_raysVertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
    } else {
        // Clear the rays VBO
        m_raysVertices.clear();
        glBindBuffer(GL_ARRAY_BUFFER, m_raysVbo);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    update();
}

// Slots for Qt controls
void GLWidget::setFx(int fx) { m_fx = fx; updateFrustum(); update(); }
void GLWidget::setFy(int fy) { m_fy = fy; updateFrustum(); update(); }
void GLWidget::setCx(double cx) { m_cx = cx; updateFrustum(); update(); }
void GLWidget::setCy(double cy) { m_cy = cy; updateFrustum(); update(); }
void GLWidget::setViewX(double x) { m_viewX = x; update(); }
void GLWidget::setViewY(double y) { m_viewY = y; update(); }
void GLWidget::setViewZ(double z) { m_viewZ = z; update(); }
void GLWidget::setIntrinsic(int row, int col, double value) {
    double oldValue = m_intrinsic(row, col);
    m_intrinsic(row, col) = value;
    // Track change
    m_matrixChangeHistory.push_back(MatrixChange{"intrinsic", -1, row, col, oldValue, value});
    updateFrustum();
    update();
    emit intrinsicMatrixChanged();
}

void GLWidget::transposeIntrinsic() {
    m_intrinsic = m_intrinsic.transposed();
    updateFrustum();
    update();
    emit intrinsicMatrixChanged();
}

void GLWidget::setExtrinsic(int row, int col, double value) {
    if (row < 3 && col < 3) {
        setExtrinsicRotation(row, col, value);
    } else if (row < 3 && col == 3) {
        setExtrinsicTranslation(row, value);
    }
    // No-op for row==3 (bottom row, always [0 0 0 1] in a rigid transform)
}

void GLWidget::transposeExtrinsic() {
    transposeExtrinsicRotation();
}

void GLWidget::swapExtrinsicRows(int row1, int row2) {
    // Only swap in rotation part (3x3) and translation part (3x1)
    for (int c = 0; c < 3; ++c) {
        double old1 = m_extrinsicRotation(row1, c), old2 = m_extrinsicRotation(row2, c);
        std::swap(m_extrinsicRotation(row1, c), m_extrinsicRotation(row2, c));
        m_matrixChangeHistory.push_back(MatrixChange{"extrinsic_rotation", -1, row1, c, old1, m_extrinsicRotation(row1, c)});
        m_matrixChangeHistory.push_back(MatrixChange{"extrinsic_rotation", -1, row2, c, old2, m_extrinsicRotation(row2, c)});
    }
    double oldT1 = m_extrinsicTranslation[row1], oldT2 = m_extrinsicTranslation[row2];
    std::swap(m_extrinsicTranslation[row1], m_extrinsicTranslation[row2]);
    m_matrixChangeHistory.push_back(MatrixChange{"extrinsic_translation", -1, row1, 3, oldT1, m_extrinsicTranslation[row1]});
    m_matrixChangeHistory.push_back(MatrixChange{"extrinsic_translation", -1, row2, 3, oldT2, m_extrinsicTranslation[row2]});
    updateFrustum();
    update();
    emit extrinsicMatrixChanged();
}

void GLWidget::swapExtrinsicCols(int col1, int col2) {
    // Only swap in rotation part (3x3)
    for (int r = 0; r < 3; ++r) {
        double old1 = m_extrinsicRotation(r, col1), old2 = m_extrinsicRotation(r, col2);
        std::swap(m_extrinsicRotation(r, col1), m_extrinsicRotation(r, col2));
        m_matrixChangeHistory.push_back(MatrixChange{"extrinsic_rotation", -1, r, col1, old1, m_extrinsicRotation(r, col1)});
        m_matrixChangeHistory.push_back(MatrixChange{"extrinsic_rotation", -1, r, col2, old2, m_extrinsicRotation(r, col2)});
    }
    updateFrustum();
    update();
    emit extrinsicMatrixChanged();
}

QMatrix4x4 GLWidget::makeExtrinsicMatrix(const QMatrix3x3& R, const QVector3D& t) const {
    QMatrix4x4 m;
    m.setToIdentity();
    for (int r=0; r<3; ++r)
        for (int c=0; c<3; ++c)
            m(r,c) = R(r,c);
    m(0,3) = t.x();
    m(1,3) = t.y();
    m(2,3) = t.z();
    return m;
}

void GLWidget::setExtrinsicRotation(int row, int col, double value) {
    double oldValue = m_extrinsicRotation(row, col);
    m_extrinsicRotation(row, col) = value;
    m_matrixChangeHistory.push_back(MatrixChange{"extrinsic_rotation", -1, row, col, oldValue, value});
    updateFrustum();
    update();
    emit extrinsicMatrixChanged();
}

void GLWidget::transposeExtrinsicRotation() {
    m_extrinsicRotation = m_extrinsicRotation.transposed();
    updateFrustum();
    update();
    emit extrinsicMatrixChanged();
}

void GLWidget::setExtrinsicTranslation(int idx, double value) {
    double oldValue = m_extrinsicTranslation[idx];
    m_extrinsicTranslation[idx] = value;
    m_matrixChangeHistory.push_back(MatrixChange{"extrinsic_translation", -1, idx, 3, oldValue, value});
    updateFrustum();
    update();
    emit extrinsicMatrixChanged();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    qDebug() << "keyPressEvent:" << event->key();
    float step = 0.3f;
    switch (event->key()) {
        case Qt::Key_Left:  m_viewX -= step; break;
        case Qt::Key_Right: m_viewX += step; break;
        case Qt::Key_Up:    m_viewZ -= step; break;
        case Qt::Key_Down:  m_viewZ += step; break;
        case Qt::Key_PageUp:   m_viewY += step; break;
        case Qt::Key_PageDown: m_viewY -= step; break;
        case Qt::Key_Plus:
        case Qt::Key_Equal: // For US keyboards, '+' is shift+'='
            m_viewScale *= 1.1; break;
        case Qt::Key_Minus:
        case Qt::Key_Underscore:
            m_viewScale /= 1.1; break;
        default: QOpenGLWidget::keyPressEvent(event); return;
    }
    update();
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    qDebug() << "mousePressEvent:" << event->button() << event->pos();
    if (event->button() == Qt::LeftButton) {
        m_mouseDragging = true;
        m_lastMousePos = event->pos();
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "mouseMoveEvent:" << event->pos();
    if (m_mouseDragging) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_viewYaw += delta.x() * 0.5;
        m_viewPitch += delta.y() * 0.5;
        //if (m_viewPitch > 89.0) m_viewPitch = 89.0;
        //if (m_viewPitch < -89.0) m_viewPitch = -89.0;
        m_lastMousePos = event->pos();
        update();
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << "mouseReleaseEvent:" << event->button() << event->pos();
    if (event->button() == Qt::LeftButton) {
        m_mouseDragging = false;
    }
}

void GLWidget::swapIntrinsicRows(int row1, int row2) {
    for (int c = 0; c < 3; ++c) {
        double old1 = m_intrinsic(row1, c), old2 = m_intrinsic(row2, c);
        std::swap(m_intrinsic(row1, c), m_intrinsic(row2, c));
        // Track both swaps
        m_matrixChangeHistory.push_back(MatrixChange{"intrinsic", -1, row1, c, old1, m_intrinsic(row1, c)});
        m_matrixChangeHistory.push_back(MatrixChange{"intrinsic", -1, row2, c, old2, m_intrinsic(row2, c)});
    }
    updateFrustum();
    update();
    emit intrinsicMatrixChanged();
}

void GLWidget::swapIntrinsicCols(int col1, int col2) {
    for (int r = 0; r < 3; ++r) {
        double old1 = m_intrinsic(r, col1), old2 = m_intrinsic(r, col2);
        std::swap(m_intrinsic(r, col1), m_intrinsic(r, col2));
        m_matrixChangeHistory.push_back(MatrixChange{"intrinsic", -1, r, col1, old1, m_intrinsic(r, col1)});
        m_matrixChangeHistory.push_back(MatrixChange{"intrinsic", -1, r, col2, old2, m_intrinsic(r, col2)});
    }
    updateFrustum();
    update();
    emit intrinsicMatrixChanged();
}

void GLWidget::resetViewToOrigin() {
    m_viewX = 0;
    m_viewY = 2;
    m_viewZ = 8;
    m_viewScale = 1.0;
    m_viewYaw = 0.0;
    m_viewPitch = 0.0;
    update();
}

void GLWidget::setCurrentExtrinsicIndex(int idx) {
    m_currentExtrinsicIndex = idx;
    emit extrinsicMatrixChanged();
    update();
}

// --- Move these from header to here ---
const std::vector<GLWidget::MatrixChange>& GLWidget::getMatrixChangeHistory() const {
    return m_matrixChangeHistory;
}
void GLWidget::clearMatrixChangeHistory() {
    m_matrixChangeHistory.clear();
}
double GLWidget::getIntrinsic(int row, int col) const {
    return m_intrinsic(row, col);
}

double GLWidget::getExtrinsicForCamera(int camIdx, int row, int col) const {
    qDebug() << "getExtrinsicForCamera camIdx=" << camIdx << "row=" << row << "col=" << col;
    if (camIdx >= 0 && camIdx < int(m_cameraExtrinsics.size())) {
        double val = m_cameraExtrinsics[camIdx](row, col);
        qDebug() << "Returning value:" << val;
        return val;
    }
    qDebug() << "Invalid camIdx in getExtrinsicForCamera!";
    return 0.0;
}

int GLWidget::getCameraCount() const {
    return static_cast<int>(m_cameraExtrinsics.size());
}

bool GLWidget::isAllCamerasEditing() const {
    return allCamerasEditing;
}

void GLWidget::setAllCamerasEditing(bool enabled) {
    allCamerasEditing = enabled;
}

void GLWidget::setExtrinsicForCamera(int camIdx, int row, int col, double value) {
    qDebug() << "setExtrinsicForCamera camIdx=" << camIdx << "row=" << row << "col=" << col << "value=" << value;
    if (camIdx >= 0 && camIdx < int(m_cameraExtrinsics.size())) {
        QMatrix4x4& mat = m_cameraExtrinsics[camIdx];
        qDebug() << "Before: mat(" << row << "," << col << ")=" << mat(row, col);
        mat(row, col) = value;
        qDebug() << "After: mat(" << row << "," << col << ")=" << mat(row, col);
        emit extrinsicMatrixChanged();
        updateGroundPlanes();  //OOF this might be bad. But to help the realtime aspect
        update();
    } else {
        qDebug() << "Invalid camIdx in setExtrinsicForCamera!";
    }
}

void GLWidget::transposeExtrinsicForCamera(int camIdx) {
    // TODO: Implement logic to transpose extrinsic rotation for a specific camera
    if (camIdx >= 0 && camIdx < int(m_cameraExtrinsics.size())) {
        QMatrix4x4& mat = m_cameraExtrinsics[camIdx];
        QMatrix3x3 rot;
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                rot(r, c) = mat(r, c);
        rot = rot.transposed();
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                mat(r, c) = rot(r, c);
        emit extrinsicMatrixChanged();
        updateGroundPlanes();  //OOF this might be bad. But to help the realtime aspect
        update();
    }
}

void GLWidget::swapExtrinsicRowsForCamera(int camIdx, int row1, int row2) {
    // TODO: Implement logic to swap extrinsic rotation rows for a specific camera
    if (camIdx >= 0 && camIdx < int(m_cameraExtrinsics.size())) {
        QMatrix4x4& mat = m_cameraExtrinsics[camIdx];
        for (int c = 0; c < 3; ++c) {
            std::swap(mat(row1, c), mat(row2, c));
        }
        std::swap(mat(row1, 3), mat(row2, 3)); // translation part
        emit extrinsicMatrixChanged();
        updateGroundPlanes();  //OOF this might be bad. But to help the realtime aspect
        update();
    }
}

void GLWidget::swapExtrinsicColsForCamera(int camIdx, int col1, int col2) {
    // TODO: Implement logic to swap extrinsic rotation cols for a specific camera
    if (camIdx >= 0 && camIdx < int(m_cameraExtrinsics.size())) {
        QMatrix4x4& mat = m_cameraExtrinsics[camIdx];
        for (int r = 0; r < 3; ++r) {
            std::swap(mat(r, col1), mat(r, col2));
        }
        emit extrinsicMatrixChanged();
        updateGroundPlanes();  //OOF this might be bad. But to help the realtime aspect
        update();
    }
}

bool GLWidget::loadKRT(const QString& krtPath, QMatrix4x4& extrinsic, QMatrix3x3& intrinsic) {
    QFile file(krtPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    QTextStream in(&file);
    // KRT: 3x3 K, 3x3 R, 3x1 t (12 floats)
    float K[9], R[9], t[3];
    for (int i = 0; i < 9; ++i) in >> K[i];
    for (int i = 0; i < 9; ++i) in >> R[i];
    for (int i = 0; i < 3; ++i) in >> t[i];
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            intrinsic(r, c) = K[r * 3 + c];
    QMatrix4x4 ext;
    ext.setToIdentity();
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            ext(r, c) = R[r * 3 + c];
    ext(0, 3) = t[0];
    ext(1, 3) = t[1];
    ext(2, 3) = t[2];
    extrinsic = ext;
    return true;
}

void GLWidget::loadSIFTDataset(const QString& dirPath) {
    // 1. Find images/ and poses/ subdirs
    QDir baseDir(dirPath);
    QDir imgDir(baseDir.filePath("images"));
    QDir poseDir(baseDir.filePath("poses"));
    if (!imgDir.exists() || !poseDir.exists()) {
        qWarning() << "Missing images/ or poses/ subdirectory!";
        return;
    }
    // 2. Match images and KRTs by filename (without extension)
    QFileInfoList imgFiles = imgDir.entryInfoList(QStringList() << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp", QDir::Files);
    std::vector<QString> imagePaths, poseNames;
    std::vector<QMatrix4x4> extrinsics;
    std::vector<QMatrix3x3> intrinsics;
    for (const QFileInfo& imgInfo : imgFiles) {
        QString base = imgInfo.completeBaseName();
        QString krtPath = poseDir.filePath(base + ".txt");
        if (!QFile::exists(krtPath)) continue;
        QMatrix4x4 ext;
        QMatrix3x3 intr;
        if (!loadKRT(krtPath, ext, intr)) continue;
        imagePaths.push_back(imgInfo.absoluteFilePath());
        poseNames.push_back(base);
        extrinsics.push_back(ext);
        intrinsics.push_back(intr);
    }
    if (imagePaths.empty()) {
        qWarning() << "No matching images/poses found!";
        return;
    }
    // 3. Load extrinsics into GUI
    setCameraExtrinsics(extrinsics);
    // 4. Load images into GUI
    setCameraImages(imagePaths);
    // 5. Run SIFT and triangulate
    runSIFTAndTriangulate(imagePaths, extrinsics, poseNames);
    // 6. Write results to file
    writeSIFT3DResultsToFile(dirPath + "/sift3d_results.txt");
}

void GLWidget::runSIFTAndTriangulate(const std::vector<QString>& imagePaths, const std::vector<QMatrix4x4>& poses, const std::vector<QString>& poseNames) {
    m_siftResults.clear();
    std::vector<cv::Mat> images;
    std::vector<std::vector<cv::KeyPoint>> allKeypoints;
    std::vector<cv::Mat> allDescriptors;
    // 1. Load images and extract SIFT features
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create(200); // lightweight
    for (size_t i = 0; i < imagePaths.size(); ++i) {
        cv::Mat img = cv::imread(imagePaths[i].toStdString(), cv::IMREAD_GRAYSCALE);
        if (img.empty()) continue;
        std::vector<cv::KeyPoint> kps;
        cv::Mat desc;
        sift->detectAndCompute(img, cv::noArray(), kps, desc);
        SIFTResult res;
        res.keypoints = kps;
        res.descriptors = desc;
        m_siftResults[poseNames[i]] = res;
        images.push_back(img);
        allKeypoints.push_back(kps);
        allDescriptors.push_back(desc);
    }
    // 2. Lightweight triangulation: match features between consecutive pairs, triangulate matches
    if (images.size() < 2) return;
    cv::BFMatcher matcher(cv::NORM_L2, true);
    for (size_t i = 0; i + 1 < images.size(); ++i) {
        std::vector<cv::DMatch> matches;
        if (allDescriptors[i].empty() || allDescriptors[i+1].empty()) continue;
        matcher.match(allDescriptors[i], allDescriptors[i+1], matches);
        // Camera matrices
        QMatrix3x3 K = m_intrinsic;
        cv::Matx33f Kcv;
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                Kcv(r, c) = K(r, c);
        QMatrix4x4 ext1 = poses[i], ext2 = poses[i+1];
        cv::Matx34f P1, P2;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 4; ++c) {
                P1(r, c) = ext1(r, c);
                P2(r, c) = ext2(r, c);
            }
        }
        cv::Matx34f P1full = Kcv * P1;
        cv::Matx34f P2full = Kcv * P2;
        std::vector<cv::Point2f> pts1, pts2;
        for (const auto& m : matches) {
            pts1.push_back(allKeypoints[i][m.queryIdx].pt);
            pts2.push_back(allKeypoints[i+1][m.trainIdx].pt);
        }
        if (pts1.size() < 8) continue;
        cv::Mat pts4d;
        cv::triangulatePoints(P1full, P2full, pts1, pts2, pts4d);
        // Store 3D points for each image
        for (int k = 0; k < pts4d.cols; ++k) {
            cv::Mat col = pts4d.col(k);
            cv::Point3f pt3d(col.at<float>(0)/col.at<float>(3), col.at<float>(1)/col.at<float>(3), col.at<float>(2)/col.at<float>(3));
            m_siftResults[poseNames[i]].points3d.push_back(pt3d);
            m_siftResults[poseNames[i+1]].points3d.push_back(pt3d);
        }
    }
}

void GLWidget::writeSIFT3DResultsToFile(const QString& outPath) {
    QFile file(outPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    for (const auto& kv : m_siftResults) {
        out << "Image: " << kv.first << "\n";
        const auto& res = kv.second;
        out << "Keypoints: " << res.keypoints.size() << "\n";
        for (size_t i = 0; i < res.points3d.size(); ++i) {
            const auto& pt = res.points3d[i];
            out << pt.x << " " << pt.y << " " << pt.z << "\n";
        }
        out << "\n";
    }
    file.close();
}