/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "DongArchCutlineRealtimeDialog.h"
#include "meshwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QColorDialog>
#include <QWheelEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

namespace DongArch {
namespace Cutline {

DongArchCutlineRealtimeDialog::DongArchCutlineRealtimeDialog(Mesh* mesh, QWidget* parent)
    : QDialog(parent)
    , mMesh(mesh)
    , mMeshWidget(nullptr)
    , mController(nullptr)
    , mOverlay(nullptr)
    , mSlotManager(nullptr)
    , mSVGExporter(nullptr)
    , mPlaneModeCombo(nullptr)
    , mOffsetSlider(nullptr)
    , mOffsetSpinBox(nullptr)
    , mOffsetZLabel(nullptr)
    , mEpsilonSpinBox(nullptr)
    , mSplineTensionSpinBox(nullptr)
    , mColorButton(nullptr)
    , mWidthSpinBox(nullptr)
    , mOpacitySlider(nullptr)
    , mSnapBBCheckBox(nullptr)
    , mSnapFeatureCheckBox(nullptr)
    , mBBDivisionsSpinBox(nullptr)
    , mProgressBar(nullptr)
    , mStatusLabel(nullptr)
    , mMeshMinZ(0.0)
    , mMeshMaxZ(0.0)
{
    if (!mMesh) {
        qCritical() << "[DongArchCutlineRealtimeDialog] Mesh is null";
        return;
    }

    // 메시 바운딩 박스 가져오기
    Vector3D bboxMin = mMesh->getBoundingBoxA();
    Vector3D bboxMax = mMesh->getBoundingBoxG();
    mMeshMinZ = bboxMin.getZ();
    mMeshMaxZ = bboxMax.getZ();

    qDebug() << "[DongArchCutlineRealtimeDialog] Mesh Z range:" << mMeshMinZ << "~" << mMeshMaxZ;

    // Controller 생성
    mController = new CutlineController(mMesh, this);

    // SlotManager 생성
    mSlotManager = new CutlineSlotManager(this);

    // SVGExporter 생성
    mSVGExporter = new CutlineSVGExporter();

    // UI 초기화
    setupUI();

    // Controller 시그널 연결
    connect(mController, &CutlineController::polylineReady,
            this, &DongArchCutlineRealtimeDialog::onPolylineReady);
    connect(mController, &CutlineController::progressChanged,
            this, &DongArchCutlineRealtimeDialog::onProgressChanged);
    connect(mController, &CutlineController::errorOccurred,
            this, &DongArchCutlineRealtimeDialog::onErrorOccurred);

    // SlotManager 시그널 연결 (UI 업데이트)
    connect(mSlotManager, &CutlineSlotManager::slotSaved, [this](int slotId) {
        qInfo() << "[RealtimeDialog] SlotManager signal: slotSaved" << slotId;
    });
    connect(mSlotManager, &CutlineSlotManager::slotCleared, [this](int slotId) {
        qInfo() << "[RealtimeDialog] SlotManager signal: slotCleared" << slotId;
    });

    // 초기 계산 트리거
    mController->setOffset(0.5f);  // 중앙값으로 시작
}

void DongArchCutlineRealtimeDialog::setupUI() {
    setWindowTitle("DongArch Cutline - Realtime Preview");
    setMinimumSize(1200, 800);

    // 메인 레이아웃: 좌측 오버레이 + 우측 컨트롤
    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    // === 좌측: 오버레이 위젯 ===
    mOverlay = new CutlineOverlayWidget(this);
    mOverlay->setMinimumSize(800, 600);
    mainLayout->addWidget(mOverlay, 1);

    // === 우측: 컨트롤 패널 ===
    QWidget* controlPanel = createControlPanel();
    mainLayout->addWidget(controlPanel);

    setLayout(mainLayout);
}

QWidget* DongArchCutlineRealtimeDialog::createControlPanel() {
    QWidget* panel = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(panel);

    // 평면 모드 그룹
    layout->addWidget(createPlaneModeGroup());

    // 알고리즘 파라미터 그룹
    layout->addWidget(createAlgorithmGroup());

    // 스타일 그룹
    layout->addWidget(createStyleGroup());

    // 스냅 옵션 그룹
    layout->addWidget(createSnapGroup());

    // 슬롯 관리 그룹
    layout->addWidget(createSlotGroup());

    // SVG Export 그룹
    layout->addWidget(createExportGroup());

    // 진행률 위젯
    layout->addWidget(createProgressWidget());

    layout->addStretch();

    return panel;
}

QGroupBox* DongArchCutlineRealtimeDialog::createPlaneModeGroup() {
    QGroupBox* group = new QGroupBox("Plane Mode", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    // 평면 모드 콤보박스
    QHBoxLayout* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Mode:", this));
    mPlaneModeCombo = new QComboBox(this);
    mPlaneModeCombo->addItem("Top (XY Plane)");
    mPlaneModeCombo->addItem("Front (XZ Plane)");
    mPlaneModeCombo->addItem("Right (YZ Plane)");
    mPlaneModeCombo->addItem("Custom");
    modeLayout->addWidget(mPlaneModeCombo);
    layout->addLayout(modeLayout);

    // 오프셋 슬라이더
    QFormLayout* offsetLayout = new QFormLayout();
    mOffsetSlider = new QSlider(Qt::Horizontal, this);
    mOffsetSlider->setRange(0, 1000);
    mOffsetSlider->setValue(500);  // 중앙값
    offsetLayout->addRow("Offset (slider):", mOffsetSlider);

    // 오프셋 숫자 입력
    mOffsetSpinBox = new QDoubleSpinBox(this);
    mOffsetSpinBox->setRange(0.0, 1.0);
    mOffsetSpinBox->setValue(0.5);
    mOffsetSpinBox->setSingleStep(0.01);
    mOffsetSpinBox->setDecimals(3);
    offsetLayout->addRow("Offset (value):", mOffsetSpinBox);

    // Z 좌표 표시
    mOffsetZLabel = new QLabel(QString::number((mMeshMinZ + mMeshMaxZ) / 2.0, 'f', 2), this);
    offsetLayout->addRow("Z coordinate:", mOffsetZLabel);

    layout->addLayout(offsetLayout);

    // 시그널 연결
    connect(mPlaneModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DongArchCutlineRealtimeDialog::onPlaneModeChanged);
    connect(mOffsetSlider, &QSlider::valueChanged,
            this, &DongArchCutlineRealtimeDialog::onOffsetSliderChanged);
    connect(mOffsetSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DongArchCutlineRealtimeDialog::onOffsetSpinBoxChanged);

    return group;
}

QGroupBox* DongArchCutlineRealtimeDialog::createAlgorithmGroup() {
    QGroupBox* group = new QGroupBox("Algorithm Parameters", this);
    QFormLayout* layout = new QFormLayout(group);

    // Douglas-Peucker epsilon
    mEpsilonSpinBox = new QDoubleSpinBox(this);
    mEpsilonSpinBox->setRange(0.001, 1.0);
    mEpsilonSpinBox->setValue(0.01);
    mEpsilonSpinBox->setSingleStep(0.001);
    mEpsilonSpinBox->setDecimals(3);
    layout->addRow("Epsilon (DP):", mEpsilonSpinBox);

    // Catmull-Rom tension
    mSplineTensionSpinBox = new QDoubleSpinBox(this);
    mSplineTensionSpinBox->setRange(0.1, 2.0);
    mSplineTensionSpinBox->setValue(0.5);
    mSplineTensionSpinBox->setSingleStep(0.1);
    mSplineTensionSpinBox->setDecimals(1);
    layout->addRow("Spline Tension:", mSplineTensionSpinBox);

    // 시그널 연결
    connect(mEpsilonSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DongArchCutlineRealtimeDialog::onEpsilonChanged);
    connect(mSplineTensionSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DongArchCutlineRealtimeDialog::onSplineTensionChanged);

    return group;
}

QGroupBox* DongArchCutlineRealtimeDialog::createStyleGroup() {
    QGroupBox* group = new QGroupBox("Style", this);
    QFormLayout* layout = new QFormLayout(group);

    // 색상 선택 버튼
    mColorButton = new QPushButton(this);
    mColorButton->setStyleSheet("background-color: rgb(255, 0, 0);");
    mColorButton->setFixedHeight(30);
    layout->addRow("Color:", mColorButton);

    // 라인 두께
    mWidthSpinBox = new QDoubleSpinBox(this);
    mWidthSpinBox->setRange(0.5, 10.0);
    mWidthSpinBox->setValue(2.0);
    mWidthSpinBox->setSingleStep(0.5);
    mWidthSpinBox->setDecimals(1);
    layout->addRow("Width:", mWidthSpinBox);

    // 불투명도
    mOpacitySlider = new QSlider(Qt::Horizontal, this);
    mOpacitySlider->setRange(0, 100);
    mOpacitySlider->setValue(80);
    layout->addRow("Opacity (%):", mOpacitySlider);

    // 시그널 연결
    connect(mColorButton, &QPushButton::clicked,
            this, &DongArchCutlineRealtimeDialog::onColorButtonClicked);
    connect(mWidthSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DongArchCutlineRealtimeDialog::onWidthChanged);
    connect(mOpacitySlider, &QSlider::valueChanged,
            this, &DongArchCutlineRealtimeDialog::onOpacityChanged);

    return group;
}

QGroupBox* DongArchCutlineRealtimeDialog::createSnapGroup() {
    QGroupBox* group = new QGroupBox("Snap Options", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    // 바운딩 박스 스냅
    mSnapBBCheckBox = new QCheckBox("Snap to Bounding Box", this);
    layout->addWidget(mSnapBBCheckBox);

    // BB 분할 수
    QHBoxLayout* divisionsLayout = new QHBoxLayout();
    divisionsLayout->addWidget(new QLabel("Divisions:", this));
    mBBDivisionsSpinBox = new QSpinBox(this);
    mBBDivisionsSpinBox->setRange(5, 50);
    mBBDivisionsSpinBox->setValue(10);
    mBBDivisionsSpinBox->setEnabled(false);
    divisionsLayout->addWidget(mBBDivisionsSpinBox);
    layout->addLayout(divisionsLayout);

    // 피처 스냅
    mSnapFeatureCheckBox = new QCheckBox("Snap to Feature", this);
    layout->addWidget(mSnapFeatureCheckBox);

    // 시그널 연결
    connect(mSnapBBCheckBox, &QCheckBox::toggled,
            this, &DongArchCutlineRealtimeDialog::onSnapBBToggled);
    connect(mSnapFeatureCheckBox, &QCheckBox::toggled,
            this, &DongArchCutlineRealtimeDialog::onSnapFeatureToggled);
    connect(mBBDivisionsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DongArchCutlineRealtimeDialog::onBBDivisionsChanged);

    return group;
}

QGroupBox* DongArchCutlineRealtimeDialog::createSlotGroup() {
    QGroupBox* group = new QGroupBox("Slot Management (Save/Load)", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    // 5개 슬롯 UI 생성
    for (int i = 0; i < 5; ++i) {
        int slotId = i + 1;

        // 슬롯 프레임
        QGroupBox* slotFrame = new QGroupBox(QString("Slot %1").arg(slotId), group);
        QVBoxLayout* slotLayout = new QVBoxLayout(slotFrame);

        // 정보 라벨
        mSlotInfoLabels[i] = new QLabel("Empty", slotFrame);
        mSlotInfoLabels[i]->setWordWrap(true);
        mSlotInfoLabels[i]->setStyleSheet("color: gray; font-size: 9pt;");
        slotLayout->addWidget(mSlotInfoLabels[i]);

        // 버튼 레이아웃
        QHBoxLayout* buttonLayout = new QHBoxLayout();

        // 저장 버튼
        mSlotSaveButtons[i] = new QPushButton("Save", slotFrame);
        connect(mSlotSaveButtons[i], &QPushButton::clicked, [this, slotId]() {
            onSaveSlotClicked(slotId);
        });
        buttonLayout->addWidget(mSlotSaveButtons[i]);

        // 로드 버튼
        mSlotLoadButtons[i] = new QPushButton("Load", slotFrame);
        connect(mSlotLoadButtons[i], &QPushButton::clicked, [this, slotId]() {
            onLoadSlotClicked(slotId);
        });
        buttonLayout->addWidget(mSlotLoadButtons[i]);

        // 삭제 버튼
        mSlotClearButtons[i] = new QPushButton("Clear", slotFrame);
        connect(mSlotClearButtons[i], &QPushButton::clicked, [this, slotId]() {
            onClearSlotClicked(slotId);
        });
        buttonLayout->addWidget(mSlotClearButtons[i]);

        slotLayout->addLayout(buttonLayout);
        layout->addWidget(slotFrame);

        // 초기 상태 업데이트 (세션 복구)
        bool isEmpty = mSlotManager->isSlotEmpty(slotId);
        if (!isEmpty) {
            CutlineSlot slot = mSlotManager->loadSlot(slotId);
            mSlotInfoLabels[i]->setText(slot.infoString());
            mSlotInfoLabels[i]->setStyleSheet("color: blue; font-size: 9pt;");
            qDebug() << "[RealtimeDialog] Restored slot" << slotId << "from session:" << slot.polyline.size() << "points";
        }
        mSlotLoadButtons[i]->setEnabled(!isEmpty);
        mSlotClearButtons[i]->setEnabled(!isEmpty);
    }

    return group;
}

QGroupBox* DongArchCutlineRealtimeDialog::createExportGroup() {
    QGroupBox* group = new QGroupBox("SVG Export", this);
    QVBoxLayout* layout = new QVBoxLayout(group);

    // Export Current 버튼
    mExportCurrentButton = new QPushButton("Export Current Cutline", this);
    mExportCurrentButton->setToolTip("Export current cutline to SVG (1:1 scale)");
    connect(mExportCurrentButton, &QPushButton::clicked,
            this, &DongArchCutlineRealtimeDialog::onExportCurrentClicked);
    layout->addWidget(mExportCurrentButton);

    // Export All Slots 버튼
    mExportAllSlotsButton = new QPushButton("Export All Slots (Multi-layer)", this);
    mExportAllSlotsButton->setToolTip("Export all saved slots as separate layers");
    connect(mExportAllSlotsButton, &QPushButton::clicked,
            this, &DongArchCutlineRealtimeDialog::onExportAllSlotsClicked);
    layout->addWidget(mExportAllSlotsButton);

    // 설명 라벨
    QLabel* infoLabel = new QLabel("SVG files are compatible with Adobe Illustrator", this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: gray; font-size: 9pt;");
    layout->addWidget(infoLabel);

    return group;
}

QWidget* DongArchCutlineRealtimeDialog::createProgressWidget() {
    QWidget* widget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widget);

    mProgressBar = new QProgressBar(this);
    mProgressBar->setRange(0, 100);
    mProgressBar->setValue(0);
    layout->addWidget(mProgressBar);

    mStatusLabel = new QLabel("Ready", this);
    mStatusLabel->setWordWrap(true);
    layout->addWidget(mStatusLabel);

    return widget;
}

// ============================================================================
// Slot 구현
// ============================================================================

void DongArchCutlineRealtimeDialog::onPlaneModeChanged(int index) {
    PlaneMode mode = static_cast<PlaneMode>(index);
    mController->setPlaneMode(mode);
    qDebug() << "[RealtimeDialog] Plane mode changed:" << index;
}

void DongArchCutlineRealtimeDialog::onOffsetSliderChanged(int value) {
    float offset = value / 1000.0f;

    // 스냅 처리
    if (mSnapBBCheckBox->isChecked()) {
        offset = static_cast<float>(snapOffsetToBB(offset));
    }

    // SpinBox 업데이트 (시그널 차단하여 무한 루프 방지)
    mOffsetSpinBox->blockSignals(true);
    mOffsetSpinBox->setValue(offset);
    mOffsetSpinBox->blockSignals(false);

    // Z 좌표 업데이트
    double z = mMeshMinZ + offset * (mMeshMaxZ - mMeshMinZ);
    mOffsetZLabel->setText(QString::number(z, 'f', 2));

    // Controller 업데이트
    mController->setOffset(offset);
}

void DongArchCutlineRealtimeDialog::onOffsetSpinBoxChanged(double value) {
    float offset = static_cast<float>(value);

    // 스냅 처리
    if (mSnapBBCheckBox->isChecked()) {
        offset = static_cast<float>(snapOffsetToBB(offset));
    }

    // Slider 업데이트
    mOffsetSlider->blockSignals(true);
    mOffsetSlider->setValue(static_cast<int>(offset * 1000));
    mOffsetSlider->blockSignals(false);

    // Z 좌표 업데이트
    double z = mMeshMinZ + offset * (mMeshMaxZ - mMeshMinZ);
    mOffsetZLabel->setText(QString::number(z, 'f', 2));

    // Controller 업데이트
    mController->setOffset(offset);
}

void DongArchCutlineRealtimeDialog::onEpsilonChanged(double value) {
    mController->setEpsilon(static_cast<float>(value));
}

void DongArchCutlineRealtimeDialog::onSplineTensionChanged(double value) {
    mController->setSplineTension(static_cast<float>(value));
}

void DongArchCutlineRealtimeDialog::onColorButtonClicked() {
    QColor currentColor = mOverlay->getColor();
    QColor newColor = QColorDialog::getColor(currentColor, this, "Select Line Color");

    if (newColor.isValid()) {
        mOverlay->setColor(newColor);
        mColorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
                                    .arg(newColor.red())
                                    .arg(newColor.green())
                                    .arg(newColor.blue()));
    }
}

void DongArchCutlineRealtimeDialog::onWidthChanged(double value) {
    mOverlay->setWidth(static_cast<float>(value));
}

void DongArchCutlineRealtimeDialog::onOpacityChanged(int value) {
    mOverlay->setOpacity(value / 100.0f);
}

void DongArchCutlineRealtimeDialog::onSnapBBToggled(bool checked) {
    mBBDivisionsSpinBox->setEnabled(checked);
    if (checked) {
        // 현재 offset을 스냅
        float currentOffset = static_cast<float>(mOffsetSpinBox->value());
        float snappedOffset = static_cast<float>(snapOffsetToBB(currentOffset));
        mOffsetSpinBox->setValue(snappedOffset);
    }
}

void DongArchCutlineRealtimeDialog::onSnapFeatureToggled(bool checked) {
    // TODO: 피처 스냅 구현 (Phase 2 고도화)
    qDebug() << "[RealtimeDialog] Feature snap:" << checked;
}

void DongArchCutlineRealtimeDialog::onBBDivisionsChanged(int value) {
    // 분할 수 변경 시 현재 offset을 다시 스냅
    if (mSnapBBCheckBox->isChecked()) {
        float currentOffset = static_cast<float>(mOffsetSpinBox->value());
        float snappedOffset = static_cast<float>(snapOffsetToBB(currentOffset));
        mOffsetSpinBox->setValue(snappedOffset);
    }
}

void DongArchCutlineRealtimeDialog::onPolylineReady(
    const std::vector<Vector3D>& polyline, bool isRaw) {

    qDebug() << "[RealtimeDialog] Polyline ready:" << polyline.size()
             << "points, raw:" << isRaw;

    // 현재 폴리라인 저장 (슬롯 저장용)
    mCurrentPolyline = polyline;

    // Dialog 내부 오버레이 업데이트
    mOverlay->setPolyline(polyline);
    mOverlay->setIsRaw(isRaw);
    mOverlay->update();

    // MeshWidget 3D 뷰 오버레이 업데이트 (메인 3D 윈도우에 붉은 라인 표시)
    if (mMeshWidget && !polyline.empty()) {
        mMeshWidget->setSectionPolyline3D(polyline);
        mMeshWidget->setSectionStyle(
            mOverlay->getColor(),
            mOverlay->getWidth(),
            mOverlay->getOpacity()
        );
        qDebug() << "[RealtimeDialog] Updated MeshWidget overlay with" << polyline.size() << "points";
    }

    mStatusLabel->setText(QString("Polyline: %1 points (%2)")
                         .arg(polyline.size())
                         .arg(isRaw ? "raw" : "smoothed"));
}

void DongArchCutlineRealtimeDialog::onProgressChanged(int percent) {
    mProgressBar->setValue(percent);
}

void DongArchCutlineRealtimeDialog::onErrorOccurred(const QString& message) {
    mStatusLabel->setText(QString("Error: %1").arg(message));
    qCritical() << "[RealtimeDialog] Error:" << message;
}

// ============================================================================
// 슬롯 관리 함수
// ============================================================================

void DongArchCutlineRealtimeDialog::onSaveSlotClicked(int slotId) {
    if (mCurrentPolyline.empty()) {
        qWarning() << "[RealtimeDialog] Cannot save: no polyline computed yet";
        mStatusLabel->setText("Error: No polyline to save");
        return;
    }

    // 현재 파라미터 가져오기
    CutlineParams params;
    params.mode = static_cast<PlaneMode>(mPlaneModeCombo->currentIndex());
    params.offset = static_cast<float>(mOffsetSpinBox->value());
    params.epsilon = static_cast<float>(mEpsilonSpinBox->value());
    params.splineTension = static_cast<float>(mSplineTensionSpinBox->value());
    params.color = mOverlay->getColor();
    params.width = mOverlay->getWidth();
    params.opacity = mOverlay->getOpacity();
    params.snapBoundingBox = mSnapBBCheckBox->isChecked();
    params.snapFeature = mSnapFeatureCheckBox->isChecked();
    params.bbDivisions = mBBDivisionsSpinBox->value();

    // SlotManager에 저장
    bool success = mSlotManager->saveSlot(slotId, params, mCurrentPolyline);
    if (success) {
        qInfo() << "[RealtimeDialog] Saved to slot" << slotId;
        mStatusLabel->setText(QString("Saved to Slot %1").arg(slotId));

        // UI 업데이트
        int index = slotId - 1;
        CutlineSlot slot = mSlotManager->loadSlot(slotId);
        mSlotInfoLabels[index]->setText(slot.infoString());
        mSlotInfoLabels[index]->setStyleSheet("color: green; font-size: 9pt;");
        mSlotLoadButtons[index]->setEnabled(true);
        mSlotClearButtons[index]->setEnabled(true);
    } else {
        qCritical() << "[RealtimeDialog] Failed to save slot" << slotId;
        mStatusLabel->setText(QString("Error: Failed to save Slot %1").arg(slotId));
    }
}

void DongArchCutlineRealtimeDialog::onLoadSlotClicked(int slotId) {
    CutlineSlot slot = mSlotManager->loadSlot(slotId);

    if (!slot.isValid()) {
        qWarning() << "[RealtimeDialog] Slot" << slotId << "is empty";
        mStatusLabel->setText(QString("Error: Slot %1 is empty").arg(slotId));
        return;
    }

    // UI에 파라미터 복원
    mPlaneModeCombo->setCurrentIndex(static_cast<int>(slot.params.mode));
    mOffsetSpinBox->setValue(slot.params.offset);
    mEpsilonSpinBox->setValue(slot.params.epsilon);
    mSplineTensionSpinBox->setValue(slot.params.splineTension);
    mWidthSpinBox->setValue(slot.params.width);
    mOpacitySlider->setValue(static_cast<int>(slot.params.opacity * 100));
    mSnapBBCheckBox->setChecked(slot.params.snapBoundingBox);
    mSnapFeatureCheckBox->setChecked(slot.params.snapFeature);
    mBBDivisionsSpinBox->setValue(slot.params.bbDivisions);

    // 색상 복원
    mOverlay->setColor(slot.params.color);
    mColorButton->setStyleSheet(QString("background-color: %1").arg(slot.params.color.name()));

    // 폴리라인 복원
    mCurrentPolyline = slot.polyline;
    mOverlay->setPolyline(slot.polyline);
    mOverlay->update();

    qInfo() << "[RealtimeDialog] Loaded from slot" << slotId;
    mStatusLabel->setText(QString("Loaded from Slot %1").arg(slotId));
}

void DongArchCutlineRealtimeDialog::onClearSlotClicked(int slotId) {
    mSlotManager->clearSlot(slotId);

    // UI 업데이트
    int index = slotId - 1;
    mSlotInfoLabels[index]->setText("Empty");
    mSlotInfoLabels[index]->setStyleSheet("color: gray; font-size: 9pt;");
    mSlotLoadButtons[index]->setEnabled(false);
    mSlotClearButtons[index]->setEnabled(false);

    qInfo() << "[RealtimeDialog] Cleared slot" << slotId;
    mStatusLabel->setText(QString("Cleared Slot %1").arg(slotId));
}

// ============================================================================
// SVG Export
// ============================================================================

void DongArchCutlineRealtimeDialog::onExportCurrentClicked() {
    if (mCurrentPolyline.empty()) {
        QMessageBox::warning(this, "Export Error", "No cutline to export. Please compute a cutline first.");
        return;
    }

    // 파일 저장 다이얼로그
    QString defaultFileName = QString("cutline_export_%1.svg")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Current Cutline to SVG",
        defaultFileName,
        "SVG Files (*.svg)"
    );

    if (filePath.isEmpty()) {
        return;  // 사용자가 취소
    }

    // 현재 파라미터로 임시 슬롯 생성
    CutlineSlot tempSlot;
    tempSlot.id = 0;  // 임시 슬롯
    tempSlot.name = "Current Cutline";
    tempSlot.timestamp = QDateTime::currentDateTime();
    tempSlot.params.mode = static_cast<PlaneMode>(mPlaneModeCombo->currentIndex());
    tempSlot.params.offset = static_cast<float>(mOffsetSpinBox->value());
    tempSlot.params.epsilon = static_cast<float>(mEpsilonSpinBox->value());
    tempSlot.params.splineTension = static_cast<float>(mSplineTensionSpinBox->value());
    tempSlot.params.color = mOverlay->getColor();
    tempSlot.params.width = static_cast<float>(mWidthSpinBox->value());
    tempSlot.params.opacity = static_cast<float>(mOpacitySlider->value() / 100.0);
    tempSlot.polyline = mCurrentPolyline;

    // Export 실행
    SVGExportResult result = mSVGExporter->exportSingleSlot(tempSlot, filePath);

    if (result.success) {
        QMessageBox::information(
            this,
            "Export Success",
            QString("Cutline exported successfully to:\n%1\n\nPoints: %2")
                .arg(result.filePath)
                .arg(mCurrentPolyline.size())
        );
        mStatusLabel->setText("Export successful");
        qInfo() << "[RealtimeDialog] Exported current cutline to" << result.filePath;
    } else {
        QMessageBox::critical(
            this,
            "Export Error",
            QString("Failed to export cutline:\n%1").arg(result.errorMessage)
        );
        mStatusLabel->setText("Export failed");
        qCritical() << "[RealtimeDialog] Export failed:" << result.errorMessage;
    }
}

void DongArchCutlineRealtimeDialog::onExportAllSlotsClicked() {
    // 유효한 슬롯 수집
    std::vector<CutlineSlot> validSlots;
    for (int i = 1; i <= 5; ++i) {
        if (!mSlotManager->isSlotEmpty(i)) {
            CutlineSlot slot = mSlotManager->loadSlot(i);
            if (slot.isValid()) {
                validSlots.push_back(slot);
            }
        }
    }

    if (validSlots.empty()) {
        QMessageBox::warning(this, "Export Error", "No saved slots to export. Please save some slots first.");
        return;
    }

    // 파일 저장 다이얼로그
    QString defaultFileName = QString("cutline_multilayer_%1.svg")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export All Slots to Multi-layer SVG",
        defaultFileName,
        "SVG Files (*.svg)"
    );

    if (filePath.isEmpty()) {
        return;  // 사용자가 취소
    }

    // Export 실행
    SVGExportResult result = mSVGExporter->exportMultipleSlots(validSlots, filePath);

    if (result.success) {
        QMessageBox::information(
            this,
            "Export Success",
            QString("Multi-layer SVG exported successfully to:\n%1\n\nLayers: %2")
                .arg(result.filePath)
                .arg(result.layersExported)
        );
        mStatusLabel->setText(QString("Exported %1 layers").arg(result.layersExported));
        qInfo() << "[RealtimeDialog] Exported" << result.layersExported << "slots to" << result.filePath;
    } else {
        QMessageBox::critical(
            this,
            "Export Error",
            QString("Failed to export slots:\n%1").arg(result.errorMessage)
        );
        mStatusLabel->setText("Export failed");
        qCritical() << "[RealtimeDialog] Export failed:" << result.errorMessage;
    }
}

// ============================================================================
// 마우스 휠 이벤트
// ============================================================================

void DongArchCutlineRealtimeDialog::wheelEvent(QWheelEvent* event) {
    // 마우스 휠로 오프셋 조절
    int delta = event->angleDelta().y();
    int step = delta > 0 ? 10 : -10;  // 1% 단위

    int newValue = mOffsetSlider->value() + step;
    newValue = qBound(0, newValue, 1000);

    mOffsetSlider->setValue(newValue);

    event->accept();
}

// ============================================================================
// 유틸리티
// ============================================================================

double DongArchCutlineRealtimeDialog::snapOffsetToBB(double offset) {
    int divisions = mBBDivisionsSpinBox->value();
    double step = 1.0 / divisions;

    // 가장 가까운 분할점으로 스냅
    int nearestStep = static_cast<int>(std::round(offset / step));
    return nearestStep * step;
}

// ============================================================================
// Public API - 프로그램적 제어
// ============================================================================

void DongArchCutlineRealtimeDialog::setInitialPlaneMode(PlaneMode mode) {
    int index = static_cast<int>(mode);
    mPlaneModeCombo->setCurrentIndex(index);
    // ComboBox 변경 시그널이 자동으로 onPlaneModeChanged()를 호출함
    qDebug() << "[RealtimeDialog] Initial plane mode set to:" << index;
}

void DongArchCutlineRealtimeDialog::setMeshWidget(MeshWidget* meshWidget) {
    mMeshWidget = meshWidget;
    qDebug() << "[RealtimeDialog] MeshWidget connected:" << (meshWidget != nullptr);
}

} // namespace Cutline
} // namespace DongArch
