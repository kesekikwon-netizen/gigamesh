/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#ifndef DONGARCH_CUTLINE_SERVICE_H
#define DONGARCH_CUTLINE_SERVICE_H

#include "CutlineParams.h"
#include <GigaMesh/mesh/mesh.h>
#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <atomic>
#include <memory>
#include <mutex>

namespace DongArch {
namespace Cutline {

//! 취소 토큰 (비동기 작업 중단)
class CancellationToken {
public:
    void cancel() { mCancelled.store(true, std::memory_order_release); }
    bool isCancelled() const { return mCancelled.load(std::memory_order_acquire); }
    void reset() { mCancelled.store(false, std::memory_order_release); }

private:
    std::atomic<bool> mCancelled{false};
};

//! 비동기 Cutline 계산 서비스
//!
//! **기능**:
//! - QtConcurrent 기반 비동기 계산
//! - 취소 토큰으로 이전 작업 중단
//! - RequestId로 최신성 보장 (stale result 폐기)
//! - Graceful degradation (raw → simplified → smoothed)
//!
//! **성능 목표**: 60fps 유지 (16ms 이하)
class CutlineService : public QObject {
    Q_OBJECT

public:
    explicit CutlineService(Mesh* mesh, QObject* parent = nullptr);
    ~CutlineService() override;

    //! 비동기 계산 시작 (이전 작업 자동 취소)
    //! \param params 파라미터
    //! \return Request ID (최신성 검증용)
    RequestId computeAsync(const CutlineParams& params);

    //! 현재 작업 취소
    void cancel();

    //! 작업 진행 중 여부
    bool isComputing() const { return mComputing.load(std::memory_order_acquire); }

    //! 마지막 Request ID
    RequestId lastRequestId() const { return mRequestId.load(std::memory_order_acquire); }

signals:
    //! 폴리라인 준비 완료 (RequestId로 최신성 보장)
    //! \param polyline 계산된 폴리라인
    //! \param requestId 요청 ID
    //! \param isRaw true면 raw 결과 (fallback), false면 최종 결과
    void polylineReady(const std::vector<Vector3D>& polyline, RequestId requestId, bool isRaw);

    //! 진행률 (0-100)
    void progressChanged(int percent);

    //! 에러 발생
    void errorOccurred(const QString& message);

private:
    //! 워커 스레드 함수 (QtConcurrent에서 실행)
    //! \return 계산된 폴리라인 (실패 시 빈 벡터)
    std::vector<Vector3D> computeWorker(const CutlineParams& params,
                                        std::shared_ptr<CancellationToken> token,
                                        RequestId requestId);

    //! 평면 HNF 계산 (PlaneMode에 따라)
    Vector3D computePlaneHNF(const CutlineParams& params) const;

    Mesh* mMesh;                                      //!< 메시 (non-owning)
    std::shared_ptr<CancellationToken> mToken;        //!< 취소 토큰
    QFutureWatcher<std::vector<Vector3D>>* mWatcher;  //!< Future 감시자
    std::atomic<bool> mComputing{false};              //!< 계산 중 플래그
    std::atomic<RequestId> mRequestId{0};             //!< Request ID (최신성 보장)
    std::mutex mMutex;                                //!< 상태 보호 뮤텍스
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_SERVICE_H
