#include <QApplication>
#include <QGroupBox>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QFile>
#include <QTextStream>
#include "glwidget.h"
#include "EXAMPLEcamera_preprocessor.h"
#include "real_estate_camera_preprocessor.h" //real estate camera data
#include <QFileDialog>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("OpenGL with Qt5 Controls: kinda looks like the rebel plans of the Death Star");

    // Create OpenGL widget
    GLWidget *glWidget = new GLWidget;

    // --- Camera arrangement selector ---
    QComboBox *arrangementBox = new QComboBox;
    arrangementBox->addItem("Circle");
    arrangementBox->addItem("Tower");
    arrangementBox->addItem("Grid");
    arrangementBox->addItem("Spiral");
    arrangementBox->addItem("Default");
    arrangementBox->addItem("Real Estate");
    arrangementBox->addItem("Custom");  // Unused    
    arrangementBox->addItem("Loaded File"); // !for user-loaded poses

    // --- Camera index selector ---
    QComboBox *cameraIndexBox = new QComboBox;

    // Store loaded extrinsics globally in main
    std::vector<QMatrix4x4> loadedFileExtrinsics;

    // Helpr to reload extrinsics and update camera index box
    // Could add syncExtrinsicWidgets() call  
    auto reloadExtrinsics = [&](){
        std::vector<QMatrix4x4> extrinsics;
        if (arrangementBox->currentText() == "Circle") {
            extrinsics = exampleCameraExtrinsics_Circle();
        } else if (arrangementBox->currentText() == "Tower") {
            extrinsics = exampleCameraExtrinsics_Tower();
        } else if (arrangementBox->currentText() == "Grid") {
            extrinsics = exampleCameraExtrinsics_Grid();
        } else if (arrangementBox->currentText() == "Spiral") {
            extrinsics = exampleCameraExtrinsics_Spiral();
        } else if (arrangementBox->currentText() == "Real Estate") {
            extrinsics = realEstateCameraExtrinsics();
        } else if (arrangementBox->currentText() == "Loaded File") {
            extrinsics = loadedFileExtrinsics;
        } else if (arrangementBox->currentText() == "Custom") {
            extrinsics.clear(); // No cameras by default
        } else {
            extrinsics = exampleCameraExtrinsics();
        }
        glWidget->setCameraExtrinsics(extrinsics);

        cameraIndexBox->clear();
        for (size_t i = 0; i < extrinsics.size(); ++i) {
            cameraIndexBox->addItem(QString("Camera %1").arg(i+1));
        }
        cameraIndexBox->setCurrentIndex(0);
        glWidget->setCurrentExtrinsicIndex(0);

        glWidget->drawRays();
    };

    // Load and set initial example camera extrinsics
    reloadExtrinsics();

    QObject::connect(arrangementBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int){
        reloadExtrinsics();
    });

    // --- Intrinsic matrix controls ---
    QGroupBox *intrinsicBox = new QGroupBox("Intrinsic (3x3)");
    QGridLayout *intrinsicGrid = new QGridLayout;
    QDoubleSpinBox *intrinsicSpin[3][3];
    for (int r=0; r<3; ++r) {
        for (int c=0; c<3; ++c) {
            intrinsicSpin[r][c] = new QDoubleSpinBox;
            intrinsicSpin[r][c]->setRange(-100000, 100000);
            intrinsicSpin[r][c]->setDecimals(6);
            intrinsicSpin[r][c]->setValue(glWidget->getIntrinsic(r, c));
            intrinsicGrid->addWidget(intrinsicSpin[r][c], r, c);
            QObject::connect(intrinsicSpin[r][c], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [glWidget, r, c](double val){ glWidget->setIntrinsic(r, c, val); });
        }
    }
    QPushButton *intrinsicTranspose = new QPushButton("Transpose");
    intrinsicGrid->addWidget(intrinsicTranspose, 3, 0, 1, 3);
    QObject::connect(intrinsicTranspose, &QPushButton::clicked, glWidget, &GLWidget::transposeIntrinsic);

    // Row/col swap for intrinsic
    QPushButton *swapRowBtn = new QPushButton("Swap Rows");
    QPushButton *swapColBtn = new QPushButton("Swap Cols");
    QComboBox *row1Box = new QComboBox, *row2Box = new QComboBox, *col1Box = new QComboBox, *col2Box = new QComboBox;
    for (int i = 1; i < 4; ++i) {
        row1Box->addItem(QString::number(i));
        row2Box->addItem(QString::number(i));
        col1Box->addItem(QString::number(i));
        col2Box->addItem(QString::number(i));
    }
    intrinsicGrid->addWidget(row1Box, 4, 0);
    intrinsicGrid->addWidget(row2Box, 4, 1);
    intrinsicGrid->addWidget(swapRowBtn, 4, 2);
    intrinsicGrid->addWidget(col1Box, 5, 0);
    intrinsicGrid->addWidget(col2Box, 5, 1);
    intrinsicGrid->addWidget(swapColBtn, 5, 2);
    QObject::connect(swapRowBtn, &QPushButton::clicked, [=](){
        glWidget->swapIntrinsicRows(row1Box->currentIndex(), row2Box->currentIndex());
    });
    QObject::connect(swapColBtn, &QPushButton::clicked, [=](){
        glWidget->swapIntrinsicCols(col1Box->currentIndex(), col2Box->currentIndex());
    });

    intrinsicBox->setLayout(intrinsicGrid);

    // --- Extrinsic controls (rotation and translation) with camera selector ---
    QGroupBox *extrinsicBox = new QGroupBox("Extrinsic (Rotation 3x3 and Translation x,y,z)");
    QVBoxLayout *extrinsicMainLayout = new QVBoxLayout;

    // Camera selector for extrinsic
    QHBoxLayout *extrinsicSelectorLayout = new QHBoxLayout;
    extrinsicSelectorLayout->addWidget(new QLabel("Edit Camera:"));
    extrinsicSelectorLayout->addWidget(cameraIndexBox);
    extrinsicMainLayout->addLayout(extrinsicSelectorLayout);

    // --- Extrinsic Rotation 3x3 ---
    QGroupBox *extrinsicRotBox = new QGroupBox("Rotation (3x3)");
    QGridLayout *extrinsicRotGrid = new QGridLayout;
    QDoubleSpinBox *extrinsicRotSpin[3][3];
    double previousRotValue[3][3];
    for (int r=0; r<3; ++r) {
        for (int c=0; c<3; ++c) {
            extrinsicRotSpin[r][c] = new QDoubleSpinBox;
            extrinsicRotSpin[r][c]->setRange(-10000000, 10000000);
            extrinsicRotSpin[r][c]->setDecimals(9);
            double initialValue = glWidget->getExtrinsicForCamera(0, r, c); 
            extrinsicRotSpin[r][c]->setValue(glWidget->getExtrinsicForCamera(0, r, c));
            previousRotValue[r][c] = initialValue;
            extrinsicRotGrid->addWidget(extrinsicRotSpin[r][c], r, c);
        }
    }

    QPushButton *extrinsicRotTranspose = new QPushButton("Transpose");
    extrinsicRotGrid->addWidget(extrinsicRotTranspose, 3, 0, 1, 3);

    // Row/col swap for extrinsic rotation
    QPushButton *swapRowBtnE = new QPushButton("Swap Rows");
    QPushButton *swapColBtnE = new QPushButton("Swap Cols");
    QComboBox *row1BoxE = new QComboBox, *row2BoxE = new QComboBox, *col1BoxE = new QComboBox, *col2BoxE = new QComboBox;
    for (int i = 1; i < 4; ++i) {
        row1BoxE->addItem(QString::number(i));
        row2BoxE->addItem(QString::number(i));
        col1BoxE->addItem(QString::number(i));
        col2BoxE->addItem(QString::number(i));
    }
    extrinsicRotGrid->addWidget(row1BoxE, 4, 0);
    extrinsicRotGrid->addWidget(row2BoxE, 4, 1);
    extrinsicRotGrid->addWidget(swapRowBtnE, 4, 2);
    extrinsicRotGrid->addWidget(col1BoxE, 5, 0);
    extrinsicRotGrid->addWidget(col2BoxE, 5, 1);
    extrinsicRotGrid->addWidget(swapColBtnE, 5, 2);

    extrinsicRotBox->setLayout(extrinsicRotGrid);
    extrinsicMainLayout->addWidget(extrinsicRotBox);
    
    

    // --- Extrinsic Translation 3x1 ---
    /*
    QGroupBox *extrinsicTransBox = new QGroupBox("Translation (x, y, z)");
    QHBoxLayout *extrinsicTransLayout = new QHBoxLayout;
    QDoubleSpinBox *extrinsicTransSpin[3];
    const char* labels[3] = {"X", "Y", "Z"};
    for (int i=0; i<3; ++i) {
        QLabel *lbl = new QLabel(labels[i]);
        extrinsicTransSpin[i] = new QDoubleSpinBox;
        extrinsicTransSpin[i]->setRange(-10000, 10000);
        extrinsicTransSpin[i]->setDecimals(6);
        extrinsicTransSpin[i]->setValue(glWidget->getExtrinsicForCamera(0, i, 3));
        extrinsicTransLayout->addWidget(lbl);
        extrinsicTransLayout->addWidget(extrinsicTransSpin[i]);
    }
    extrinsicTransBox->setLayout(extrinsicTransLayout);
    extrinsicMainLayout->addWidget(extrinsicTransBox);

    extrinsicBox->setLayout(extrinsicMainLayout);*/

    // --- Extrinsic Translation 3x1 ---
    QGroupBox *extrinsicTransBox = new QGroupBox("Translation (x, y, z)");
    QHBoxLayout *extrinsicTransLayout = new QHBoxLayout;
    QDoubleSpinBox *extrinsicTransSpin[3];
    double previousTransValue[3]; // Track previous values for deltas
    const char* labels[3] = {"X", "Y", "Z"};

    for (int i = 0; i < 3; ++i) {
        QLabel *lbl = new QLabel(labels[i]);
        extrinsicTransSpin[i] = new QDoubleSpinBox;
        extrinsicTransSpin[i]->setRange(-10000, 10000);
        extrinsicTransSpin[i]->setDecimals(6);

        double initialValue = glWidget->getExtrinsicForCamera(0, i, 3);
        extrinsicTransSpin[i]->setValue(initialValue);
        previousTransValue[i] = initialValue;

        extrinsicTransLayout->addWidget(lbl);
        extrinsicTransLayout->addWidget(extrinsicTransSpin[i]);
    }

    extrinsicTransBox->setLayout(extrinsicTransLayout);
    extrinsicMainLayout->addWidget(extrinsicTransBox);
    extrinsicBox->setLayout(extrinsicMainLayout);

    // --- Extrinsic rotation row/col swap and transpose connections ---
    //ADD Ray call
    QObject::connect(swapRowBtnE, &QPushButton::clicked, [=]() {
        int camIdx = cameraIndexBox->currentIndex();
        int row1 = row1BoxE->currentIndex();
        int row2 = row2BoxE->currentIndex();
        if (camIdx >= 0) {
            for (int c = 0; c < 3; ++c) {
                double old1 = glWidget->getExtrinsicForCamera(camIdx, row1, c);
                double old2 = glWidget->getExtrinsicForCamera(camIdx, row2, c);
                glWidget->setExtrinsicForCamera(camIdx, row1, c, old2);
                glWidget->setExtrinsicForCamera(camIdx, row2, c, old1);
            }
        }

        if (glWidget->isAllCamerasEditing()) {
            for (size_t i = 0; i < glWidget->getCameraCount(); ++i) {
                if (i == camIdx) {
                    continue; //Need to check on 0 indexing and whatnot -------------------------------------------------------------------------
                }
                for (int c = 0; c < 3; ++c) {
                double old1 = glWidget->getExtrinsicForCamera(i, row1, c);
                double old2 = glWidget->getExtrinsicForCamera(i, row2, c);
                glWidget->setExtrinsicForCamera(i, row1, c, old2);
                glWidget->setExtrinsicForCamera(i, row2, c, old1);
                }
            }
        }
        glWidget->drawRays(); // TA-da
    });
    //ADD ray call
    QObject::connect(swapColBtnE, &QPushButton::clicked, [=]() {
        int camIdx = cameraIndexBox->currentIndex();
        int col1 = col1BoxE->currentIndex();
        int col2 = col2BoxE->currentIndex();
        if (camIdx >= 0) {
            for (int r = 0; r < 3; ++r) {
                double old1 = glWidget->getExtrinsicForCamera(camIdx, r, col1);
                double old2 = glWidget->getExtrinsicForCamera(camIdx, r, col2);
                glWidget->setExtrinsicForCamera(camIdx, r, col1, old2);
                glWidget->setExtrinsicForCamera(camIdx, r, col2, old1);
            }
        }
        if (glWidget->isAllCamerasEditing()) {
            for(size_t i = 0; i < glWidget->getCameraCount(); ++i) {
                int cameraIndex = i;
                if (cameraIndex == camIdx) {
                    continue; //Need to check on 0 indexing and whatnot -------------------------------------------------------------------------
                }
                for (int r = 0; r < 3; ++r) {
                    double old1 = glWidget->getExtrinsicForCamera(cameraIndex, r, col1);
                    double old2 = glWidget->getExtrinsicForCamera(cameraIndex, r, col2);
                    glWidget->setExtrinsicForCamera(cameraIndex, r, col1, old2);
                    glWidget->setExtrinsicForCamera(cameraIndex, r, col2, old1);
                }
            }
        }
        glWidget->drawRays(); // TA-da
    });
    //ADD Ray call
    QObject::connect(extrinsicRotTranspose, &QPushButton::clicked, [glWidget, cameraIndexBox, extrinsicRotSpin]() {
        int camIdx = cameraIndexBox->currentIndex();
        /*if (camIdx >= 0) {
            double rot[3][3];
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c)
                    rot[r][c] = glWidget->getExtrinsicForCamera(camIdx, r, c);
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c)
                    glWidget->setExtrinsicForCamera(camIdx, r, c, rot[c][r]);
            int idx = cameraIndexBox->currentIndex();
            for (int r=0; r<3; ++r) {
                for (int c=0; c<3; ++c) {
                    extrinsicRotSpin[r][c]->blockSignals(true);
                    extrinsicRotSpin[r][c]->setValue(glWidget->getExtrinsicForCamera(idx, r, c));
                    extrinsicRotSpin[r][c]->blockSignals(false);
                }
            }
        }*/
        if (glWidget->isAllCamerasEditing()) {
            for (size_t i = 0; i < glWidget->getCameraCount(); ++i) {
                int cameraIndex = i; // Just for understanding
                double rot[3][3];
                for (int r = 0; r < 3; ++r) {
                    for (int c = 0; c < 3; ++c) {
                        rot[r][c] = glWidget->getExtrinsicForCamera(cameraIndex, r, c);
                    }
                }
                for (int r = 0; r < 3; ++r) {
                    for (int c = 0; c < 3; ++c) {
                        glWidget->setExtrinsicForCamera(cameraIndex, r, c, rot[c][r]);
                    }
                }
                //experimental if block
                if (cameraIndex == camIdx) {
                    for (int r=0; r<3; ++r) {
                        for (int c=0; c<3; ++c) {
                            extrinsicRotSpin[r][c]->blockSignals(true);
                            extrinsicRotSpin[r][c]->setValue(glWidget->getExtrinsicForCamera(cameraIndex, r, c));
                            extrinsicRotSpin[r][c]->blockSignals(false);
                        }
                    }
                }
            }
        } else if (camIdx >= 0) {
            double rot[3][3];
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; ++c) {
                    rot[r][c] = glWidget->getExtrinsicForCamera(camIdx, r, c);
                }
            }
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; ++c) {
                    glWidget->setExtrinsicForCamera(camIdx, r, c, rot[c][r]);
                }
            }
            int idx = cameraIndexBox->currentIndex();
            for (int r=0; r<3; ++r) {
                for (int c=0; c<3; ++c) {
                    extrinsicRotSpin[r][c]->blockSignals(true);
                    extrinsicRotSpin[r][c]->setValue(glWidget->getExtrinsicForCamera(idx, r, c));
                    extrinsicRotSpin[r][c]->blockSignals(false);
                }
            }     
        }
        glWidget->drawRays(); // TA-da
    });

    // --- Write Matrix Change Log Checkbox ---
    QCheckBox *writeLogCheck = new QCheckBox("Write Matrix Change Log");
    QObject::connect(writeLogCheck, &QCheckBox::stateChanged, [=](int state){
        if (state == Qt::Checked) {
            QFile file("matrix_change_log.txt");
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                const auto& history = glWidget->getMatrixChangeHistory();
                for (const auto& rec : history) {
                    out << rec.type << ", camIdx=" << rec.camIdx
                        << ", row=" << rec.row << ", col=" << rec.col
                        << ", old=" << rec.oldValue << ", new=" << rec.newValue << "\n";
                }
                file.close();
            }
            writeLogCheck->setChecked(false);
        }
    });

    // --- Edit All Extrinsics Uniformly ---
    //probably got to edit all the bunch of stuff with other versions ------------------------------------- 
    QCheckBox *selectAllCameras = new QCheckBox("Select All Sensors");
    QObject::connect(selectAllCameras, &QCheckBox::stateChanged,
        [glWidget](int state) {
            glWidget->setAllCamerasEditing(state == Qt::Checked);
        });

    // --- Reset view Button ---
    QPushButton *resetViewBtn = new QPushButton("Reset View");
    QObject::connect(resetViewBtn, &QPushButton::clicked, glWidget, &GLWidget::resetViewToOrigin);

    // --- Rays draw option for quick visual evaluation ---
    QPushButton *drawRaysBtn = new QPushButton("Draw Rays");
    QObject::connect(drawRaysBtn, &QPushButton::clicked, glWidget, &GLWidget::toggleRays);
    //QObject::connect(drawRaysBtn, &QPushButton::clicked, glWidget, &GLWidget::drawRays); // Want to fix so that rays are updated by frustum changes

    // --- Layout ---
    QVBoxLayout *controls = new QVBoxLayout;
    controls->addWidget(new QLabel("Camera Arrangement:"));
    controls->addWidget(arrangementBox);
    controls->addWidget(intrinsicBox);
    controls->addWidget(extrinsicBox);
    controls->addWidget(selectAllCameras);
    controls->addWidget(writeLogCheck);
    controls->addWidget(resetViewBtn);
    controls->addWidget(drawRaysBtn);

    QPushButton *loadPosesBtn = new QPushButton("Load Camera Poses...");
    controls->addWidget(loadPosesBtn);

    // --- image texturing additions ---
    QPushButton *loadImagesBtn = new QPushButton("Load Camera Images...");
    controls->addWidget(loadImagesBtn);

    QPushButton *clearImagesBtn = new QPushButton("Clear Images");
    controls->addWidget(clearImagesBtn);

    QPushButton *loadSIFTBtn = new QPushButton("Load SIFT Dataset...");
    controls->addWidget(loadSIFTBtn);

    QPushButton *loadSIFTCloudBtn = new QPushButton("Load SIFT 3D Pointcloud...");
    controls->addWidget(loadSIFTCloudBtn);

    QPushButton *loadBundleAdjustmentBtn = new QPushButton("Load Bundle Adjustment NPZ...");
    controls->addWidget(loadBundleAdjustmentBtn);

    QPushButton *clearBundleAdjustmentBtn = new QPushButton("Clear Bundle Adjustment Data");
    controls->addWidget(clearBundleAdjustmentBtn);

    QPushButton *exportKRTBtn = new QPushButton("Export KRTs...");
    controls->addWidget(exportKRTBtn);
    // --- end image texturing additions ---

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(glWidget, 1);
    mainLayout->addLayout(controls);

    window.setLayout(mainLayout);
    window.resize(1200, 700);
    window.show();

    // --- Matrix UI sync: update spinboxes if matrix changes programmatically ---
    QObject::connect(glWidget, &GLWidget::intrinsicMatrixChanged, [&](){
        for (int r=0; r<3; r++)
            for (int c=0; c<3; c++)
                intrinsicSpin[r][c]->blockSignals(true),
                intrinsicSpin[r][c]->setValue(glWidget->getIntrinsic(r, c)),
                intrinsicSpin[r][c]->blockSignals(false);
    });

    // --- Extrinsic rotation/translation widget sync ---
    auto syncExtrinsicWidgets = [&](){
        int idx = cameraIndexBox->currentIndex();
        // Rotation
        for (int r=0; r<3; ++r)
            for (int c=0; c<3; ++c) {
                extrinsicRotSpin[r][c]->blockSignals(true);
                extrinsicRotSpin[r][c]->setValue(glWidget->getExtrinsicForCamera(idx, r, c));
                extrinsicRotSpin[r][c]->blockSignals(false);
            }
        // Translation
        for (int i=0; i<3; ++i) {
            extrinsicTransSpin[i]->blockSignals(true);
            extrinsicTransSpin[i]->setValue(glWidget->getExtrinsicForCamera(idx, i, 3));
            extrinsicTransSpin[i]->blockSignals(false);
        }
        // Also doing the previous
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                previousRotValue[r][c] = glWidget->getExtrinsicForCamera(idx, r, c);
            }
        } // Keep track of the current previous
        for (int i = 0; i < 3; ++i) {
            previousTransValue[i] = glWidget->getExtrinsicForCamera(idx, i, 3);
        }
    };

    QObject::connect(glWidget, &GLWidget::extrinsicMatrixChanged, syncExtrinsicWidgets);

    // When camera index changes, update extrinsic widgets
    QObject::connect(cameraIndexBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int idx){
        syncExtrinsicWidgets();
        glWidget->setCurrentExtrinsicIndex(idx);
    });

    
    
    /*// When user edits rotation spinboxes, update selected camera's extrinsic
    for (int r=0; r<3; ++r) {
        for (int c=0; c<3; ++c) {
            QObject::connect(extrinsicRotSpin[r][c], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [glWidget, cameraIndexBox, r, c](double val){
                    glWidget->setExtrinsicForCamera(cameraIndexBox->currentIndex(), r, c, val);
                });
        }
    }*/

    // This is the extrinsic rotation spiboxes connection
    for (int r=0; r<3; ++r) {
        for (int c = 0; c<3; ++c) {
            /* QObject::connect(extrinsicRotSpin[r][c], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [glWidget, cameraIndexBox, r, c, &previousRotValue](double newval) {
                    double delta = newval - previousRotValue[r][c];
                    previousRotValue[r][c] = newval;

                    int selectedCam = cameraIndexBox->currentIndex();
                    glWidget->setExtrinsicForCamera(cameraIndexBox->currentIndex(), r, c, newval);

                    if (glWidget->isAllCamerasEditing()) {
                        int numCameras = glWidget->getCameraCount();
                        for (int camIdx = 0; camIdx < numCameras; ++camIdx) {
                            if (camIdx == selectedCam) continue;
                            double current = glWidget->getExtrinsicForCamera(camIdx, r, c);
                            glWidget->setExtrinsicForCamera(camIdx, r, c, current + delta);
                        }
                    }
                glWidget->drawRays(); //pls work
                }
            );*/
            QObject::connect(extrinsicRotSpin[r][c], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    [glWidget, cameraIndexBox, r, c, &previousRotValue](double newval) {
                    int selectedCam = cameraIndexBox->currentIndex();
                    double delta = newval - previousRotValue[r][c];

                    // Update selected camera FIRST
                    glWidget->setExtrinsicForCamera(selectedCam, r, c, newval);
                    previousRotValue[r][c] = newval;

                    // Update others if needed
                    if (glWidget->isAllCamerasEditing()) {
                        int numCameras = glWidget->getCameraCount();
                        for (int camIdx = 0; camIdx < numCameras; ++camIdx) {
                            if (camIdx == selectedCam) continue;
                            double current = glWidget->getExtrinsicForCamera(camIdx, r, c);
                            glWidget->setExtrinsicForCamera(camIdx, r, c, current + delta);
                        }
                    }

                    glWidget->drawRays();
                }
            );
        }
    }


    // When user edits translation spinboxes, update selected camera's extrinsic
    /*for (int i=0; i<3; ++i) {
        QObject::connect(extrinsicTransSpin[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [glWidget, cameraIndexBox, i](double val){
                glWidget->setExtrinsicForCamera(cameraIndexBox->currentIndex(), i, 3, val);
            });
    }*/
   
    //ADDed Ray call
    // !! known problem with the computation of delta. editing translation values does not affect cameras uniformly and correctly
    for (int i = 0; i < 3; ++i) {
        /*QObject::connect(extrinsicTransSpin[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=, &previousTransValue](double newVal) {
                // measure distance
                double delta = newVal - previousTransValue[i];
                previousTransValue[i] = newVal;

                if (glWidget->isAllCamerasEditing()) {
                    int numCameras = glWidget->getCameraCount();
                    for (int camIdx = 0; camIdx < numCameras; ++camIdx) {
                        //if (camIdx == selectedCam) continue;
                        double current = glWidget->getExtrinsicForCamera(camIdx, i, 3);
                        glWidget->setExtrinsicForCamera(camIdx, i, 3, current + delta);
                    }
                } else {
                    int selectedCam = cameraIndexBox->currentIndex();
                    glWidget->setExtrinsicForCamera(selectedCam, i, 3, newVal);
                }

                glWidget->drawRays(); //pls work                
            }
        ); */

        QObject::connect(extrinsicTransSpin[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [=, &previousTransValue](double newVal) {
                int selectedCam = cameraIndexBox->currentIndex();
                double delta = newVal - previousTransValue[i];

                // Update selected camera
                glWidget->setExtrinsicForCamera(selectedCam, i, 3, newVal);
                previousTransValue[i] = newVal;

                // Apply delta to others if editing all
                if (glWidget->isAllCamerasEditing()) {
                    int numCameras = glWidget->getCameraCount();
                    for (int camIdx = 0; camIdx < numCameras; ++camIdx) {
                        if (camIdx == selectedCam) continue;
                        double current = glWidget->getExtrinsicForCamera(camIdx, i, 3);
                        glWidget->setExtrinsicForCamera(camIdx, i, 3, current + delta);
                    }
                }

                glWidget->drawRays();
            }
        );        
    }

    /* QObject::connect(loadPosesBtn, &QPushButton::clicked, [&](){
        QString fileName = QFileDialog::getOpenFileName(&window, "Open Camera Pose File", "", "Text Files (*.txt)");
        if (fileName.isEmpty()) return;

        std::vector<QMatrix4x4> extrinsics;
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty() || line.startsWith("//")) continue;
                QStringList cols = line.split(" ", Qt::SkipEmptyParts);
                if (cols.size() == 16) {
                    QMatrix4x4 mat;
                    for (int i = 0; i < 16; ++i)
                        mat(i / 4, i % 4) = cols[i].toFloat();
                    extrinsics.push_back(mat);
                }
            }
            file.close();
        }
        if (!extrinsics.empty()) {
            loadedFileExtrinsics = extrinsics; // Save globally
            arrangementBox->setCurrentText("Loaded File"); // Switch to loaded file
            reloadExtrinsics();
        }
    }); */

    QObject::connect(loadPosesBtn, &QPushButton::clicked, [&]() {
        QString fileName = QFileDialog::getOpenFileName(&window, "Open Camera Pose File", "", "Text Files (*.txt)");
        if (fileName.isEmpty()) return;

        std::vector<QMatrix4x4> extrinsics;
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty() || line.startsWith("//")) continue;

                // Remove trailing comment after '#'
                int commentIndex = line.indexOf('#');
                if (commentIndex != -1)
                    line = line.left(commentIndex).trimmed();

                QStringList cols = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (cols.size() >= 16) {
                    QMatrix4x4 mat;
                    bool ok = true;
                    for (int i = 0; i < 16; ++i) {
                        bool valueOk;
                        float val = cols[i].toFloat(&valueOk);
                        if (!valueOk) {
                            ok = false;
                            break;
                        }
                        mat(i / 4, i % 4) = val;
                    }
                    if (ok) {
                        extrinsics.push_back(mat);
                    }
                }
            }
            file.close();
        }

        if (!extrinsics.empty()) {
            loadedFileExtrinsics = extrinsics; // Save globally
            arrangementBox->setCurrentText("Loaded File"); // Switch to loaded file
            reloadExtrinsics(); // Trigger view update
        }
    });


    // --- image texturing additions ---
    QObject::connect(loadImagesBtn, &QPushButton::clicked, [&]() {
        QStringList files = QFileDialog::getOpenFileNames(&window, "Select Camera Images (assigned to selected camera and onward)", "", "Images (*.png *.jpg *.jpeg *.bmp)");
        if (!files.isEmpty()) {
            std::vector<QString> imagePaths;
            for (const QString& f : files) imagePaths.push_back(f);
            glWidget->setCameraImages(imagePaths);
        }
    });

    QObject::connect(clearImagesBtn, &QPushButton::clicked, glWidget, &GLWidget::clearCameraImages);

    // --- SIFT dataset loading: update all relevant GUI state after loading ---
    QObject::connect(loadSIFTBtn, &QPushButton::clicked, [&]() {
        QString dir = QFileDialog::getExistingDirectory(&window, "Select SIFT Dataset Directory");
        if (!dir.isEmpty()) {
            glWidget->loadSIFTDataset(dir);

            // Get the new number of cameras after loading SIFT dataset
            int numCams = glWidget->getCameraCount();

            // Update cameraIndexBox to match the new number of cameras
            cameraIndexBox->blockSignals(true);
            cameraIndexBox->clear();
            for (int i = 0; i < numCams; ++i) {
                cameraIndexBox->addItem(QString("Camera %1").arg(i + 1));
            }
            cameraIndexBox->setCurrentIndex(0);
            cameraIndexBox->blockSignals(false);

            // Set the current extrinsic index to 0 (first camera)
            glWidget->setCurrentExtrinsicIndex(0);

            // Sync intrinsic spinboxes
            for (int r = 0; r < 3; r++)
                for (int c = 0; c < 3; c++)
                    intrinsicSpin[r][c]->blockSignals(true),
                    intrinsicSpin[r][c]->setValue(glWidget->getIntrinsic(r, c)),
                    intrinsicSpin[r][c]->blockSignals(false);

            // Sync extrinsic spinboxes for first camera
            syncExtrinsicWidgets();

            // Redraw rays/frustums for new cameras
            glWidget->drawRays();
        }
    });

    QObject::connect(loadSIFTCloudBtn, &QPushButton::clicked, [&]() {
        QString file = QFileDialog::getOpenFileName(&window, "Select SIFT 3D Pointcloud File", "", "SIFT 3D (*.sift3d.txt *.txt);;All Files (*)");
        if (!file.isEmpty()) {
            glWidget->loadSIFTPointCloudFile(file);
        }
    });

    QObject::connect(loadBundleAdjustmentBtn, &QPushButton::clicked, [&]() {
        QString file = QFileDialog::getOpenFileName(&window, "Select Bundle Adjustment NPZ File", "", "NPZ Files (*.npz);;Text Files (*.txt);;All Files (*)");
        if (!file.isEmpty()) {
            glWidget->loadBundleAdjustmentNPZ(file);
        }
    });

    QObject::connect(clearBundleAdjustmentBtn, &QPushButton::clicked, [&]() {
        glWidget->clearBundleAdjustmentData();
    });
    // --- end image texturing additions ---

    //  Export KRTs logic ---
    QObject::connect(exportKRTBtn, &QPushButton::clicked, [&]() {
        QString dir = QFileDialog::getExistingDirectory(&window, "Select Output Directory for KRTs");
        if (dir.isEmpty()) return;

        // Get current intrinsics
        QMatrix3x3 K;
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                K(r, c) = glWidget->getIntrinsic(r, c);

        int numCams = glWidget->getCameraCount();
        for (int camIdx = 0; camIdx < numCams; ++camIdx) {
            QMatrix4x4 ext;
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    ext(r, c) = glWidget->getExtrinsicForCamera(camIdx, r, c);

            QMatrix3x3 R;
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c)
                    R(r, c) = ext(r, c);
            QVector3D t(ext(0, 3), ext(1, 3), ext(2, 3));

            QString fname = QString("%1/camera_%2.txt").arg(dir).arg(camIdx, 3, 10, QChar('0'));
            QFile file(fname);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out.setRealNumberNotation(QTextStream::FixedNotation);
                out.setRealNumberPrecision(8);

                // Write K (3x3, row per line)
                for (int r = 0; r < 3; ++r) {
                    for (int c = 0; c < 3; ++c) {
                        out << QString::number(K(r, c), 'f', 8);
                        if (c < 2) out << " ";
                    }
                    out << "\n";
                }
                out << "\n";

                // Write R (3x3, row per line)
                for (int r = 0; r < 3; ++r) {
                    for (int c = 0; c < 3; ++c) {
                        out << QString::number(R(r, c), 'f', 8);
                        if (c < 2) out << " ";
                    }
                    out << "\n";
                }
                out << "\n";

                // Write t (3x1, one row)
                out << QString::number(t.x(), 'f', 8) << " "
                    << QString::number(t.y(), 'f', 8) << " "
                    << QString::number(t.z(), 'f', 8) << "\n";
                file.close();
            }
        }
    });
    // --- end Export KRTs logic ---

    return app.exec();
}