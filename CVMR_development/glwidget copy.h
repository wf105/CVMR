#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QMatrix4x4>
#include <QMatrix3x3>
#include <vector>
#include <QString>
#include <QImage>
#include <QOpenGLTexture>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <map>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    // --- Matrix change tracking ---
    struct MatrixChange {
        QString type; // "intrinsic", "extrinsic_rotation", "extrinsic_translation", or "extrinsic"
        int camIdx;   // -1 for intrinsic, or camera index for extrinsic
        int row, col;
        double oldValue, newValue;
    };
    const std::vector<MatrixChange>& getMatrixChangeHistory() const;
    void clearMatrixChangeHistory();

    // --- Fixes for main.cpp compatibility ---
    double getIntrinsic(int row, int col) const;
    double getExtrinsicForCamera(int camIdx, int row, int col) const;
    int getCameraCount() const;
    bool isAllCamerasEditing() const;
    void setAllCamerasEditing(bool enabled);
    void toggleRays(); // Already implemented in .cpp

public slots:
    // Intrinsic matrix element set
    void setIntrinsic(int row, int col, double value);
    void transposeIntrinsic();

    // Extrinsic rotation (3x3) and translation (3x1)
    void setExtrinsicRotation(int row, int col, double value);
    void transposeExtrinsicRotation();
    void setExtrinsicTranslation(int idx, double value);

    // Per-camera extrinsic (4x4) editing for compatibility
    void setExtrinsicForCamera(int camIdx, int row, int col, double value);
    void transposeExtrinsicForCamera(int camIdx);
    void swapExtrinsicRowsForCamera(int camIdx, int row1, int row2);
    void swapExtrinsicColsForCamera(int camIdx, int col1, int col2);

    void setCameraExtrinsics(const std::vector<QMatrix4x4>& extrinsics);

    void setFx(int fx);
    void setFy(int fy);
    void setCx(double cx);
    void setCy(double cy);
    void setViewX(double x);
    void setViewY(double y);
    void setViewZ(double z);

    void swapIntrinsicRows(int row1, int row2);
    void swapIntrinsicCols(int col1, int col2);
    void drawRays();

    void setCurrentExtrinsicIndex(int idx);
    void resetViewToOrigin();

    // Add these for compatibility with main extrinsic as 4x4
    void setExtrinsic(int row, int col, double value);
    void transposeExtrinsic();
    void swapExtrinsicRows(int row1, int row2);
    void swapExtrinsicCols(int col1, int col2);

    // --- Image texturing additions ---
    void setCameraImages(const std::vector<QString>& imagePaths); // NEW
    void clearCameraImages(); // NEW
    void loadSIFTDataset(const QString& dirPath); // NEW

signals:
    void intrinsicMatrixChanged();
    void extrinsicMatrixChanged();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    bool m_glInitialized = false;

    // Camera intrinsics
    int m_fx = 500, m_fy = 500;
    double m_cx = 320, m_cy = 240;
    double m_viewX = 0, m_viewY = 2, m_viewZ = 8;
    double m_viewScale = 1.0; // Added scaling factor
    double m_viewYaw = 0.0, m_viewPitch = 0.0;

    // OpenGL handles
    QOpenGLShaderProgram m_shader;
    GLuint m_gridVao = 0, m_gridVbo = 0;
    GLuint m_frustumVao = 0, m_frustumVbo = 0;
    GLuint m_raysVao = 0, m_raysVbo = 0;

    std::vector<QVector3D> m_gridVertices;
    std::vector<QVector3D> m_frustumVertices;

    // VH & VB is a hmm
    GLuint m_axisVao = 0, m_axisVbo = 0;
    std::vector<QVector3D> m_axisVertices;
    void updateAxes();
    // Handle key press
    void keyPressEvent(QKeyEvent *event) override;

    QMatrix3x3 m_intrinsic = QMatrix3x3();
    QMatrix3x3 m_extrinsicRotation = QMatrix3x3();
    QVector3D m_extrinsicTranslation = QVector3D(0, 2, 8);

    void updateFrustum(); // Will use m_intrinsic and m_extrinsic

    void updateGrid();

    std::vector<QMatrix4x4> m_cameraExtrinsics; //
    std::vector<std::vector<QVector3D>> m_allFrustumVertices; // Frustum lines for each camera

    QPoint m_lastMousePos;
    bool m_mouseDragging = false;

    // --- Add these for ray drawing ---
    bool m_showRays = false;
    std::vector<QVector3D> m_raysVertices;

    // ---Added for general extrinsics
    bool allCamerasEditing = false; 

    int m_currentExtrinsicIndex = 0; // No longer used for per-camera editing

    std::vector<MatrixChange> m_matrixChangeHistory;

    QMatrix4x4 makeExtrinsicMatrix(const QMatrix3x3& R, const QVector3D& t) const;

    // --- Image texturing additions ---
    std::vector<QOpenGLTexture*> m_cameraTextures; // One per camera
    std::vector<QString> m_cameraImagePaths; // For reload
    GLuint m_groundPlaneVao = 0, m_groundPlaneVbo = 0; // For textured quads
    std::vector<QVector3D> m_groundPlaneVertices; // 4 per camera
    std::vector<QVector2D> m_groundPlaneTexCoords; // 4 per camera

    void updateGroundPlanes(); // NEW: update geometry for textured quads
    void drawGroundPlanes();   // NEW: render textured quads

    // SIFT feature storage
    struct SIFTResult {
        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        std::vector<cv::Point3f> points3d; // triangulated 3D points
    };
    std::map<QString, SIFTResult> m_siftResults; // image filename -> SIFTResult

    void runSIFTAndTriangulate(const std::vector<QString>& imagePaths, const std::vector<QMatrix4x4>& poses, const std::vector<QString>& poseNames);
    void writeSIFT3DResultsToFile(const QString& outPath);

    // Helper for KRT parsing
    static bool loadKRT(const QString& krtPath, QMatrix4x4& extrinsic, QMatrix3x3& intrinsic);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};