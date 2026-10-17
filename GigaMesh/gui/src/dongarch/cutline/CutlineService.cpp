/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "CutlineService.h"
#include "algorithms/DouglasPeucker.h"
#include "algorithms/CatmullRomSpline.h"
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <stdexcept>
#include <chrono>

namespace DongArch {
namespace Cutline {

CutlineService::CutlineService(Mesh* mesh, QObject* parent)
    : QObject(parent)
    , mMesh(mesh)
    , mToken(std::make_shared<CancellationToken>())
    , mWatcher(new QFutureWatcher<std::vector<Vector3D>>(this))
{
    if (!mMesh) {
        throw std::invalid_argument("[CutlineService] Mesh is null");
    }

    // Future 완료 시 플래그 해제
    connect(mWatcher, &QFutureWatcherBase::finished, this, [this]() {
        mComputing.store(false, std::memory_order_release);
        qDebug() << "[CutlineService] Computation finished";
    });
}

CutlineService::~CutlineService() {
    cancel();
    mWatcher->waitForFinished();
}

RequestId CutlineService::computeAsync(const CutlineParams& params) {
    std::lock_guard<std::mutex> lock(mMutex);

    // 1. 이전 작업 취소
    if (mComputing.load(std::memory_order_acquire)) {
        qDebug() << "[CutlineService] Cancelling previous computation";
        cancel();
        mWatcher->waitForFinished();
    }

    // 2. 새로운 Request ID 생성
    RequestId requestId = mRequestId.fetch_add(1, std::memory_order_acq_rel) + 1;
    qDebug() << "[CutlineService] Starting async computation, requestId:" << requestId;

    // 3. 취소 토큰 리셋
    mToken = std::make_shared<CancellationToken>();
    mComputing.store(true, std::memory_order_release);

    // 4. QtConcurrent로 워커 실행
    auto token = mToken; // 복사 (람다 캡처용)
    QFuture<std::vector<Vector3D>> future = QtConcurrent::run(
        [this, params, token, requestId]() -> std::vector<Vector3D> {
            return computeWorker(params, token, requestId);
        });

    // 5. Watcher 연결
    mWatcher->setFuture(future);

    return requestId;
}

void CutlineService::cancel() {
    if (mToken) {
        mToken->cancel();
        qDebug() << "[CutlineService] Cancellation requested";
    }
}

std::vector<Vector3D> CutlineService::computeWorker(
    const CutlineParams& params,
    std::shared_ptr<CancellationToken> token,
    RequestId requestId) {

    auto totalStartTime = std::chrono::high_resolution_clock::now();

    try {
        // === 단계 1: Raw 교차선 계산 (GigaMesh API) ===
        auto stage1Start = std::chrono::high_resolution_clock::now();
        emit progressChanged(10);
        if (token->isCancelled()) {
            qDebug() << "[CutlineService] Cancelled at intersection stage";
            return {};
        }

        Vector3D planeHNF = computePlaneHNF(params);
        std::vector<Vector3D> rawPoints;

        bool success = mMesh->calcIntersectionPolylineWithPlane(planeHNF, &rawPoints);
        if (!success || rawPoints.empty()) {
            emit errorOccurred(QStringLiteral("Failed to compute mesh-plane intersection"));
            return {};
        }

        auto stage1End = std::chrono::high_resolution_clock::now();
        auto stage1Duration = std::chrono::duration_cast<std::chrono::milliseconds>(stage1End - stage1Start);

        qDebug() << "[CutlineService] Raw intersection points:" << rawPoints.size()
                 << "| Time:" << stage1Duration.count() << "ms";

        // Fallback Level 1: 즉시 Raw 결과 표시
        emit polylineReady(rawPoints, requestId, true);

        emit progressChanged(40);
        if (token->isCancelled()) {
            qDebug() << "[CutlineService] Cancelled after raw intersection";
            return rawPoints;
        }

        // === 단계 2: Douglas-Peucker 간략화 ===
        auto stage2Start = std::chrono::high_resolution_clock::now();
        DongArch::Cutline::Algorithms::DouglasPeucker dp;
        auto simplified = dp.simplify(rawPoints, params.epsilon);
        auto stage2End = std::chrono::high_resolution_clock::now();
        auto stage2Duration = std::chrono::duration_cast<std::chrono::milliseconds>(stage2End - stage2Start);

        qDebug() << "[CutlineService] Simplified points:" << simplified.size()
                 << "(epsilon:" << params.epsilon << ")"
                 << "| Time:" << stage2Duration.count() << "ms";

        // Fallback Level 2: 간략화 결과 표시
        emit polylineReady(simplified, requestId, false);

        emit progressChanged(70);
        if (token->isCancelled()) {
            qDebug() << "[CutlineService] Cancelled after simplification";
            return simplified;
        }

        // === 단계 3: Catmull-Rom Spline 피팅 ===
        auto stage3Start = std::chrono::high_resolution_clock::now();
        DongArch::Cutline::Algorithms::CatmullRomSpline spline;
        int segmentsPerSpan = static_cast<int>(10 * params.splineTension);
        if (segmentsPerSpan < 2) segmentsPerSpan = 2;

        auto smoothed = spline.interpolate(simplified, segmentsPerSpan);
        auto stage3End = std::chrono::high_resolution_clock::now();
        auto stage3Duration = std::chrono::duration_cast<std::chrono::milliseconds>(stage3End - stage3Start);

        qDebug() << "[CutlineService] Smoothed points:" << smoothed.size()
                 << "(tension:" << params.splineTension << ")"
                 << "| Time:" << stage3Duration.count() << "ms";

        emit progressChanged(100);

        // 전체 성능 요약
        auto totalEndTime = std::chrono::high_resolution_clock::now();
        auto totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(totalEndTime - totalStartTime);

        qInfo() << "[Performance] Total compute time:" << totalDuration.count() << "ms"
                << "| Stage1:" << stage1Duration.count() << "ms"
                << "| Stage2:" << stage2Duration.count() << "ms"
                << "| Stage3:" << stage3Duration.count() << "ms";

        // 60fps 목표 검증 (16.67ms/frame)
        if (totalDuration.count() > 16) {
            qWarning() << "[Performance] Compute time exceeds 60fps budget:"
                       << totalDuration.count() << "ms (> 16.67ms)";
        }

        // 최종 결과 반환
        return smoothed;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Cutline computation failed: %1").arg(e.what()));
        qCritical() << "[CutlineService] Exception:" << e.what();
        return {};
    }
}

Vector3D CutlineService::computePlaneHNF(const CutlineParams& params) const {
    // 메시 바운딩 박스 가져오기
    Vector3D bboxMin = mMesh->getBoundingBoxA(); // Min corner
    Vector3D bboxMax = mMesh->getBoundingBoxG(); // Max corner
    Vector3D bboxCenter = (bboxMin + bboxMax) * 0.5;

    switch (params.mode) {
        case PlaneMode::TOP: {
            // XY 평면 (Z축 고정)
            double zMin = bboxMin.getZ();
            double zMax = bboxMax.getZ();
            double z = zMin + params.offset * (zMax - zMin);
            // HNF: nx=0, ny=0, nz=1, d=-z
            return Vector3D(0.0, 0.0, 1.0, -z);
        }
        case PlaneMode::FRONT: {
            // XZ 평면 (Y축 고정)
            double yMin = bboxMin.getY();
            double yMax = bboxMax.getY();
            double y = yMin + params.offset * (yMax - yMin);
            // HNF: nx=0, ny=1, nz=0, d=-y
            return Vector3D(0.0, 1.0, 0.0, -y);
        }
        case PlaneMode::RIGHT: {
            // YZ 평면 (X축 고정)
            double xMin = bboxMin.getX();
            double xMax = bboxMax.getX();
            double x = xMin + params.offset * (xMax - xMin);
            // HNF: nx=1, ny=0, nz=0, d=-x
            return Vector3D(1.0, 0.0, 0.0, -x);
        }
        case PlaneMode::CUSTOM: {
            // 사용자 정의 평면 (params.plane 사용)
            // TODO: Plane → Vector3D HNF 변환 구현
            qWarning() << "[CutlineService] Custom plane mode not fully implemented yet";
            return Vector3D(0.0, 0.0, 1.0, -bboxCenter.getZ());
        }
        default:
            qWarning() << "[CutlineService] Unknown plane mode, using TOP";
            return Vector3D(0.0, 0.0, 1.0, -bboxCenter.getZ());
    }
}

} // namespace Cutline
} // namespace DongArch
