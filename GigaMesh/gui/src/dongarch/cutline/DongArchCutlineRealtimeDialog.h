/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#ifndef DONGARCH_CUTLINE_REALTIME_DIALOG_H
#define DONGARCH_CUTLINE_REALTIME_DIALOG_H

#include "CutlineController.h"
#include "CutlineOverlayWidget.h"
#include "CutlineSlotManager.h"
#include "CutlineSVGExporter.h"
#include "../../meshwidget.h"
#include <QDialog>
#include <QComboBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QGroupBox>

namespace DongArch {
namespace Cutline {

//! 실시간 Cutline 다이얼로그
//!
//! **기능**:
//! - Top/Front/Right/Custom 평면 모드
//! - 오프셋 슬라이더 + 마우스 휠 + 숫자 입력
//! - 실시간 미리보기 (Controller 연결)
//! - 스타일 조절 (색상/두께/불투명도)
//! - 바운딩 박스 스냅
//!
//! **UI 레이아웃**:
//! - 좌측: 3D 뷰포트 (오버레이)
//! - 우측: 컨트롤 패널 (평면/오프셋/스타일/슬롯)
class DongArchCutlineRealtimeDialog : public QDialog {
    Q_OBJECT

public:
    explicit DongArchCutlineRealtimeDialog(Mesh* mesh, QWidget* parent = nullptr);
    ~DongArchCutlineRealtimeDialog() override = default;

    //! 평면 모드를 프로그램적으로 설정 (메뉴 연동용)
    void setInitialPlaneMode(PlaneMode mode);

    //! MeshWidget 설정 (MeshWidget 오버레이 렌더링용)
    void setMeshWidget(MeshWidget* meshWidget);

protected:
    //! 마우스 휠 이벤트 (오프셋 조절)
    void wheelEvent(QWheelEvent* event) override;

private slots:
    // === 평면 모드 ===
    void onPlaneModeChanged(int index);
    void onOffsetSliderChanged(int value);
    void onOffsetSpinBoxChanged(double value);

    // === 알고리즘 파라미터 ===
    void onEpsilonChanged(double value);
    void onSplineTensionChanged(double value);

    // === 스타일 ===
    void onColorButtonClicked();
    void onWidthChanged(double value);
    void onOpacityChanged(int value);

    // === 스냅 옵션 ===
    void onSnapBBToggled(bool checked);
    void onSnapFeatureToggled(bool checked);
    void onBBDivisionsChanged(int value);

    // === 슬롯 관리 ===
    void onSaveSlotClicked(int slotId);
    void onLoadSlotClicked(int slotId);
    void onClearSlotClicked(int slotId);

    // === SVG Export ===
    void onExportCurrentClicked();
    void onExportAllSlotsClicked();

    // === Controller 이벤트 ===
    void onPolylineReady(const std::vector<Vector3D>& polyline, bool isRaw);
    void onProgressChanged(int percent);
    void onErrorOccurred(const QString& message);

private:
    //! UI 초기화
    void setupUI();
    QWidget* createControlPanel();
    QGroupBox* createPlaneModeGroup();
    QGroupBox* createAlgorithmGroup();
    QGroupBox* createStyleGroup();
    QGroupBox* createSnapGroup();
    QGroupBox* createSlotGroup();
    QGroupBox* createExportGroup();
    QWidget* createProgressWidget();

    //! 오프셋을 바운딩 박스 분할에 스냅
    double snapOffsetToBB(double offset);

    Mesh* mMesh;                                  //!< 메시 (non-owning)
    MeshWidget* mMeshWidget;                      //!< MeshWidget (non-owning, for overlay rendering)
    CutlineController* mController;               //!< Controller
    CutlineOverlayWidget* mOverlay;               //!< 오버레이 위젯
    CutlineSlotManager* mSlotManager;             //!< 슬롯 매니저
    CutlineSVGExporter* mSVGExporter;             //!< SVG Exporter

    // === 평면 모드 ===
    QComboBox* mPlaneModeCombo;                   //!< 평면 모드 선택
    QSlider* mOffsetSlider;                       //!< 오프셋 슬라이더 (0-1000)
    QDoubleSpinBox* mOffsetSpinBox;               //!< 오프셋 숫자 입력 (0.0-1.0)
    QLabel* mOffsetZLabel;                        //!< Z 좌표 표시

    // === 알고리즘 파라미터 ===
    QDoubleSpinBox* mEpsilonSpinBox;              //!< Douglas-Peucker epsilon
    QDoubleSpinBox* mSplineTensionSpinBox;        //!< Catmull-Rom tension

    // === 스타일 ===
    QPushButton* mColorButton;                    //!< 색상 선택 버튼
    QDoubleSpinBox* mWidthSpinBox;                //!< 라인 두께
    QSlider* mOpacitySlider;                      //!< 불투명도 (0-100)

    // === 스냅 옵션 ===
    QCheckBox* mSnapBBCheckBox;                   //!< 바운딩 박스 스냅
    QCheckBox* mSnapFeatureCheckBox;              //!< 피처 스냅
    QSpinBox* mBBDivisionsSpinBox;                //!< BB 분할 수 (10 or 20)

    // === 슬롯 UI ===
    QPushButton* mSlotSaveButtons[5];             //!< 저장 버튼 (5개)
    QPushButton* mSlotLoadButtons[5];             //!< 로드 버튼 (5개)
    QPushButton* mSlotClearButtons[5];            //!< 삭제 버튼 (5개)
    QLabel* mSlotInfoLabels[5];                   //!< 슬롯 정보 라벨

    // === Export UI ===
    QPushButton* mExportCurrentButton;            //!< 현재 Cutline Export
    QPushButton* mExportAllSlotsButton;           //!< 모든 슬롯 Export (레이어 분리)

    // === 진행률 ===
    QProgressBar* mProgressBar;                   //!< 진행률
    QLabel* mStatusLabel;                         //!< 상태 메시지

    // === 메시 바운딩 박스 ===
    double mMeshMinZ;                             //!< 메시 최소 Z
    double mMeshMaxZ;                             //!< 메시 최대 Z

    // === 현재 폴리라인 (저장용) ===
    std::vector<Vector3D> mCurrentPolyline;       //!< 마지막으로 계산된 폴리라인
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_REALTIME_DIALOG_H
