/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "DongArchClipDialog.h"
#include "meshwidget.h"
#include <iostream>
#include <QWheelEvent>

namespace DongArch {
namespace Clip {

DongArchClipDialog::DongArchClipDialog(MeshWidget* meshWidget,
                                       DongArchClipManager* clipManager,
                                       QWidget* parent)
    : QDialog(parent)
    , mMeshWidget(meshWidget)
    , mClipManager(clipManager)
    , mCurrentHeight(0.5)
    , mPreviewEnabled(true)
    , mCurrentMode(ClipMode::KEEP_BOTH)
    , mMeshMinZ(0.0)
    , mMeshMaxZ(1.0)
    , mMeshCenterZ(0.5)
{
    setWindowTitle(tr("DongArch Clip - 메시 절단"));
    setModal(false);  // Non-modal so user can rotate view
    resize(400, 350);

    // Get mesh bounds for height normalization
    if (mMeshWidget && mMeshWidget->getMeshVisual()) {
        MeshQt* mesh = mMeshWidget->getMeshVisual();
        // Use getBoundingBoxA (min corner) and getBoundingBoxG (max corner)
        Vector3D bboxMin = mesh->getBoundingBoxA();
        Vector3D bboxMax = mesh->getBoundingBoxG();
        mMeshMinZ = bboxMin.getZ();
        mMeshMaxZ = bboxMax.getZ();
        mMeshCenterZ = (mMeshMinZ + mMeshMaxZ) / 2.0;
        mCurrentHeight = 0.5;  // Start at center
    }

    createUI();

    // Enable preview by default
    mPreviewCheckbox->setChecked(true);
    updatePreview();
}

DongArchClipDialog::~DongArchClipDialog() {
    // Disable preview when dialog closes
    if (mMeshWidget && mPreviewEnabled) {
        // Signal to disable clip preview
        emit previewStateChanged(false);
    }
}

void DongArchClipDialog::createUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title label
    QLabel* titleLabel = new QLabel(tr("평면으로 메시 절단 (실시간 미리보기)"));
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(10);

    // Height controls
    mainLayout->addWidget(createHeightControls());

    mainLayout->addSpacing(10);

    // Mode controls
    mainLayout->addWidget(createModeControls());

    mainLayout->addSpacing(10);

    // Preview checkbox
    QGroupBox* previewGroup = new QGroupBox(tr("미리보기"));
    QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
    mPreviewCheckbox = new QCheckBox(tr("실시간 미리보기 활성화"));
    mPreviewCheckbox->setChecked(true);
    connect(mPreviewCheckbox, &QCheckBox::stateChanged,
            this, &DongArchClipDialog::onPreviewCheckboxChanged);
    previewLayout->addWidget(mPreviewCheckbox);

    mAutoZoomButton = new QPushButton(tr("절단면으로 자동 줌"));
    connect(mAutoZoomButton, &QPushButton::clicked,
            this, &DongArchClipDialog::onAutoZoomClicked);
    previewLayout->addWidget(mAutoZoomButton);

    mainLayout->addWidget(previewGroup);

    mainLayout->addSpacing(10);

    // Action buttons
    mainLayout->addWidget(createActionButtons());

    mainLayout->addStretch();
}

QWidget* DongArchClipDialog::createHeightControls() {
    QGroupBox* heightGroup = new QGroupBox(tr("절단 높이"));
    QVBoxLayout* heightLayout = new QVBoxLayout(heightGroup);

    // Height info label
    QLabel* infoLabel = new QLabel(tr("슬라이더를 움직여 절단 높이를 조정하세요"));
    infoLabel->setWordWrap(true);
    heightLayout->addWidget(infoLabel);

    // Slider
    mHeightSlider = new QSlider(Qt::Horizontal);
    mHeightSlider->setRange(0, 1000);
    mHeightSlider->setValue(500);  // Start at center
    mHeightSlider->setTickPosition(QSlider::TicksBelow);
    mHeightSlider->setTickInterval(100);
    connect(mHeightSlider, &QSlider::valueChanged,
            this, &DongArchClipDialog::onHeightSliderChanged);
    heightLayout->addWidget(mHeightSlider);

    // Spin box for precise input
    QHBoxLayout* spinBoxLayout = new QHBoxLayout();
    QLabel* heightLabel = new QLabel(tr("높이 (정규화):"));
    mHeightSpinBox = new QDoubleSpinBox();
    mHeightSpinBox->setRange(0.0, 1.0);
    mHeightSpinBox->setSingleStep(0.01);
    mHeightSpinBox->setDecimals(3);
    mHeightSpinBox->setValue(0.5);
    connect(mHeightSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DongArchClipDialog::onHeightSpinBoxChanged);
    spinBoxLayout->addWidget(heightLabel);
    spinBoxLayout->addWidget(mHeightSpinBox);
    spinBoxLayout->addStretch();
    heightLayout->addLayout(spinBoxLayout);

    // Z-coordinate display
    double currentZ = mMeshMinZ + mCurrentHeight * (mMeshMaxZ - mMeshMinZ);
    QLabel* zLabel = new QLabel(tr("현재 Z 좌표: %1").arg(currentZ, 0, 'f', 3));
    zLabel->setObjectName("zCoordLabel");
    heightLayout->addWidget(zLabel);

    return heightGroup;
}

QWidget* DongArchClipDialog::createModeControls() {
    QGroupBox* modeGroup = new QGroupBox(tr("절단 모드"));
    QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);

    mModeButtonGroup = new QButtonGroup(this);

    mRadioKeepFront = new QRadioButton(tr("평면 앞쪽 유지 (위쪽 제거)"));
    mRadioKeepBack = new QRadioButton(tr("평면 뒤쪽 유지 (아래쪽 제거)"));
    mRadioKeepBoth = new QRadioButton(tr("양쪽 모두 유지 (메시 분할)"));

    mModeButtonGroup->addButton(mRadioKeepFront, static_cast<int>(ClipMode::KEEP_FRONT));
    mModeButtonGroup->addButton(mRadioKeepBack, static_cast<int>(ClipMode::KEEP_BACK));
    mModeButtonGroup->addButton(mRadioKeepBoth, static_cast<int>(ClipMode::KEEP_BOTH));

    modeLayout->addWidget(mRadioKeepFront);
    modeLayout->addWidget(mRadioKeepBack);
    modeLayout->addWidget(mRadioKeepBoth);

    // Default: Keep Both
    mRadioKeepBoth->setChecked(true);

    connect(mModeButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &DongArchClipDialog::onClipModeChanged);

    return modeGroup;
}

QWidget* DongArchClipDialog::createActionButtons() {
    QWidget* buttonWidget = new QWidget();
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);

    mExecuteButton = new QPushButton(tr("실행"));
    mExecuteButton->setMinimumHeight(35);
    QFont executeFont = mExecuteButton->font();
    executeFont.setBold(true);
    mExecuteButton->setFont(executeFont);
    connect(mExecuteButton, &QPushButton::clicked,
            this, &DongArchClipDialog::onExecuteClicked);

    mCancelButton = new QPushButton(tr("취소"));
    mCancelButton->setMinimumHeight(35);
    connect(mCancelButton, &QPushButton::clicked,
            this, &DongArchClipDialog::onCancelClicked);

    buttonLayout->addWidget(mExecuteButton);
    buttonLayout->addWidget(mCancelButton);

    return buttonWidget;
}

void DongArchClipDialog::onHeightSliderChanged(int value) {
    mCurrentHeight = value / 1000.0;

    // Update spin box (block signals to prevent loop)
    mHeightSpinBox->blockSignals(true);
    mHeightSpinBox->setValue(mCurrentHeight);
    mHeightSpinBox->blockSignals(false);

    // Update Z coordinate label
    double currentZ = mMeshMinZ + mCurrentHeight * (mMeshMaxZ - mMeshMinZ);
    QLabel* zLabel = findChild<QLabel*>("zCoordLabel");
    if (zLabel) {
        zLabel->setText(tr("현재 Z 좌표: %1").arg(currentZ, 0, 'f', 3));
    }

    emit clipHeightChanged(mCurrentHeight);

    if (mPreviewEnabled) {
        updatePreview();
    }
}

void DongArchClipDialog::onHeightSpinBoxChanged(double value) {
    mCurrentHeight = value;

    // Update slider (block signals to prevent loop)
    mHeightSlider->blockSignals(true);
    mHeightSlider->setValue(static_cast<int>(value * 1000));
    mHeightSlider->blockSignals(false);

    // Update Z coordinate label
    double currentZ = mMeshMinZ + mCurrentHeight * (mMeshMaxZ - mMeshMinZ);
    QLabel* zLabel = findChild<QLabel*>("zCoordLabel");
    if (zLabel) {
        zLabel->setText(tr("현재 Z 좌표: %1").arg(currentZ, 0, 'f', 3));
    }

    emit clipHeightChanged(mCurrentHeight);

    if (mPreviewEnabled) {
        updatePreview();
    }
}

void DongArchClipDialog::onClipModeChanged() {
    int modeId = mModeButtonGroup->checkedId();
    mCurrentMode = static_cast<ClipMode>(modeId);
    emit clipModeChanged(mCurrentMode);

    std::cout << "[DongArchClipDialog] Mode changed: " << clipModeName(mCurrentMode) << "\n";
}

void DongArchClipDialog::onPreviewCheckboxChanged(int state) {
    mPreviewEnabled = (state == Qt::Checked);
    emit previewStateChanged(mPreviewEnabled);

    if (mPreviewEnabled) {
        updatePreview();
    } else {
        // Disable preview in OpenGL
        if (mMeshWidget) {
            MeshQt* mesh = mMeshWidget->getMeshVisual();
            if (mesh) {
                mesh->setParamFlagMeshGL(MeshGLParams::DONGARCH_CLIP_PREVIEW_ENABLED, false);
            }
            mMeshWidget->update();
        }
    }
}

void DongArchClipDialog::onExecuteClicked() {
    if (!mClipManager) {
        std::cerr << "[DongArchClipDialog] Error: ClipManager is null\n";
        return;
    }

    // Disable preview before executing
    if (mMeshWidget && mPreviewEnabled) {
        MeshQt* mesh = mMeshWidget->getMeshVisual();
        if (mesh) {
            mesh->setParamFlagMeshGL(MeshGLParams::DONGARCH_CLIP_PREVIEW_ENABLED, false);
        }
    }

    // Compute clip plane
    Vector3D clipPlane = computeClipPlane();

    // Execute clipping
    ClipParams params;
    params.planeHNF = clipPlane;
    params.mode = mCurrentMode;
    params.duplicateVertices = false;
    params.noRedraw = false;  // Enable redraw after clipping
    params.epsilon = 1e-10;

    ClipResult result;
    bool success = mClipManager->clipByPlane(params, result);

    if (success) {
        std::cout << "[DongArchClipDialog] Clipping executed successfully\n";
        std::cout << "  - Original faces: " << result.originalFaceCount << "\n";
        std::cout << "  - Final faces: " << result.frontFaceCount << "\n";
        std::cout << "  - Time: " << result.processingTime << " ms\n";

        emit executeClip();
        accept();  // Close dialog
    } else {
        std::cerr << "[DongArchClipDialog] Clipping failed\n";
    }
}

void DongArchClipDialog::onCancelClicked() {
    // Disable preview
    if (mMeshWidget && mPreviewEnabled) {
        MeshQt* mesh = mMeshWidget->getMeshVisual();
        if (mesh) {
            mesh->setParamFlagMeshGL(MeshGLParams::DONGARCH_CLIP_PREVIEW_ENABLED, false);
        }
        mMeshWidget->update();
    }

    reject();  // Close dialog
}

void DongArchClipDialog::onAutoZoomClicked() {
    // TODO: Implement auto-zoom to clip plane
    // This would require access to camera controls
    std::cout << "[DongArchClipDialog] Auto-zoom to clip plane\n";
}

void DongArchClipDialog::updatePreview() {
    if (!mMeshWidget) {
        return;
    }

    MeshQt* mesh = mMeshWidget->getMeshVisual();
    if (!mesh) {
        return;
    }

    // Compute clip plane in Hesse Normal Form
    Vector3D clipPlane = computeClipPlane();

    // Enable clip preview in MeshGL layer via MeshQt
    mesh->setParamFlagMeshGL(MeshGLParams::DONGARCH_CLIP_PREVIEW_ENABLED, true);

    // Set clip plane parameters in MeshGL float parameters
    // These will be read by meshGLShader.cpp and applied via glClipPlane()
    mesh->setParamFloatMeshGL(MeshGLParams::DONGARCH_CLIP_PLANE_X, clipPlane.getX());
    mesh->setParamFloatMeshGL(MeshGLParams::DONGARCH_CLIP_PLANE_Y, clipPlane.getY());
    mesh->setParamFloatMeshGL(MeshGLParams::DONGARCH_CLIP_PLANE_Z, clipPlane.getZ());
    mesh->setParamFloatMeshGL(MeshGLParams::DONGARCH_CLIP_PLANE_W, clipPlane.getH());

    std::cout << "[DongArchClipDialog::updatePreview] Clip plane: ("
              << clipPlane.getX() << ", " << clipPlane.getY() << ", "
              << clipPlane.getZ() << ", " << clipPlane.getH() << ")\n";

    // Trigger OpenGL update
    mMeshWidget->update();
}

Vector3D DongArchClipDialog::computeClipPlane() const {
    // Compute Z coordinate from normalized height
    double z = mMeshMinZ + mCurrentHeight * (mMeshMaxZ - mMeshMinZ);

    // Create horizontal plane at height z
    // Plane equation: 0*x + 0*y + 1*z - z = 0
    // Normal: (0, 0, 1), pointing up
    // HNF: (nx, ny, nz, -d) where d = distance from origin
    Vector3D planeHNF(0.0, 0.0, 1.0, -z);

    return planeHNF;
}

double DongArchClipDialog::getClipHeight() const {
    return mCurrentHeight;
}

ClipMode DongArchClipDialog::getClipMode() const {
    return mCurrentMode;
}

bool DongArchClipDialog::isPreviewEnabled() const {
    return mPreviewEnabled;
}

void DongArchClipDialog::wheelEvent(QWheelEvent* event) {
    // Use mouse wheel to adjust clip height
    // Positive delta = scroll up = increase height (move plane up)
    // Negative delta = scroll down = decrease height (move plane down)

    if (!event) {
        return;
    }

    // Get wheel delta (typically ±120 for one notch)
    int delta = event->angleDelta().y();

    // Calculate height change (0.01 per wheel notch, ~1% of range)
    double heightChange = (delta / 120.0) * 0.01;

    // Update height with bounds checking
    double newHeight = mCurrentHeight + heightChange;
    newHeight = std::max(0.0, std::min(1.0, newHeight));

    // Update slider (which will trigger height update)
    mHeightSlider->setValue(static_cast<int>(newHeight * 1000));

    // Accept event
    event->accept();

    std::cout << "[DongArchClipDialog] Mouse wheel: delta=" << delta
              << ", new height=" << newHeight << "\n";
}

} // namespace Clip
} // namespace DongArch
