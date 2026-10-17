/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#ifndef DONGARCH_CUTLINE_CONTROLLER_H
#define DONGARCH_CUTLINE_CONTROLLER_H

#include "CutlineParams.h"
#include "CutlineService.h"
#include <QObject>
#include <QTimer>
#include <memory>

namespace DongArch {
namespace Cutline {

//! Cutline Controller (MVVM의 ViewModel)
//!
//! **역할**:
//! - 파라미터 변경 이벤트 수신 및 디바운스 (16ms)
//! - CutlineService 비동기 계산 트리거
//! - RequestId 검증으로 최신 결과만 수용
//! - 카메라/평면 변경 시 자동 재계산
//!
//! **연결 다이어그램**:
//! ```
//! UI (Dialog) → Controller → Service (Worker Thread)
//!                   ↓
//!              Overlay (Render)
//! ```
class CutlineController : public QObject {
    Q_OBJECT

public:
    explicit CutlineController(Mesh* mesh, QObject* parent = nullptr);
    ~CutlineController() override = default;

    // === 파라미터 접근 ===
    const CutlineParams& params() const { return mParams; }
    void setParams(const CutlineParams& params);

    // === 개별 파라미터 설정 (디바운스 적용) ===
    void setPlaneMode(PlaneMode mode);
    void setOffset(float offset);
    void setEpsilon(float epsilon);
    void setSplineTension(float tension);
    void setColor(const QColor& color);
    void setWidth(float width);
    void setOpacity(float opacity);
    void setSnapBoundingBox(bool enabled);
    void setSnapFeature(bool enabled);

    // === 이벤트 핸들러 ===
    void onCameraChanged();  //!< 카메라 변경 (회전/줌)
    void onPlaneChanged();   //!< 평면 변경 (수동)

    // === 상태 쿼리 ===
    bool isComputing() const { return mService && mService->isComputing(); }
    RequestId lastRequestId() const { return mLastSentRequestId; }

signals:
    //! 폴리라인 준비 완료 (최신 결과만)
    void polylineReady(const std::vector<Vector3D>& polyline, bool isRaw);

    //! 파라미터 변경 (UI 업데이트용)
    void paramsChanged(const CutlineParams& params);

    //! 진행률 (0-100)
    void progressChanged(int percent);

    //! 에러 발생
    void errorOccurred(const QString& message);

private slots:
    //! 디바운스 타이머 만료 → 계산 트리거
    void onDebounceTimeout();

    //! Service에서 폴리라인 수신 (RequestId 검증)
    void onPolylineComputed(const std::vector<Vector3D>& polyline,
                            RequestId requestId,
                            bool isRaw);

private:
    //! 계산 트리거 (디바운스 적용)
    void triggerCompute();

    //! 즉시 계산 (디바운스 무시)
    void computeImmediate();

    Mesh* mMesh;                         //!< 메시 (non-owning)
    CutlineParams mParams;               //!< 현재 파라미터
    std::unique_ptr<CutlineService> mService; //!< 비동기 서비스
    QTimer* mDebounceTimer;              //!< 디바운스 타이머 (16ms)
    RequestId mLastSentRequestId{0};     //!< 마지막 전송한 Request ID
    RequestId mLastReceivedRequestId{0}; //!< 마지막 수신한 Request ID
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_CONTROLLER_H
