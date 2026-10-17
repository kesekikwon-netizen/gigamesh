/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "CutlineController.h"
#include <QDebug>
#include <cmath>
#include <chrono>

namespace DongArch {
namespace Cutline {

CutlineController::CutlineController(Mesh* mesh, QObject* parent)
    : QObject(parent)
    , mMesh(mesh)
    , mService(std::make_unique<CutlineService>(mesh, this))
    , mDebounceTimer(new QTimer(this))
{
    if (!mMesh) {
        throw std::invalid_argument("[CutlineController] Mesh is null");
    }

    // 디바운스 타이머 설정 (16ms = ~60fps)
    mDebounceTimer->setSingleShot(true);
    mDebounceTimer->setInterval(16);
    connect(mDebounceTimer, &QTimer::timeout, this, &CutlineController::onDebounceTimeout);

    // Service 시그널 연결
    connect(mService.get(), &CutlineService::polylineReady,
            this, &CutlineController::onPolylineComputed);
    connect(mService.get(), &CutlineService::progressChanged,
            this, &CutlineController::progressChanged);
    connect(mService.get(), &CutlineService::errorOccurred,
            this, &CutlineController::errorOccurred);

    qDebug() << "[CutlineController] Initialized with debounce 16ms";
}

void CutlineController::setParams(const CutlineParams& params) {
    mParams = params;
    emit paramsChanged(mParams);
    triggerCompute();
}

void CutlineController::setPlaneMode(PlaneMode mode) {
    if (mParams.mode == mode) return;

    mParams.mode = mode;
    qDebug() << "[CutlineController] Plane mode changed:" << mParams.modeName();

    emit paramsChanged(mParams);
    triggerCompute();
}

void CutlineController::setOffset(float offset) {
    // Clamp to [0, 1]
    offset = std::clamp(offset, 0.0f, 1.0f);

    if (std::fabs(mParams.offset - offset) < 1e-6f) return;

    mParams.offset = offset;
    qDebug() << "[CutlineController] Offset changed:" << offset;

    emit paramsChanged(mParams);
    triggerCompute();
}

void CutlineController::setEpsilon(float epsilon) {
    // Clamp to [0.001, 0.1]
    epsilon = std::clamp(epsilon, 0.001f, 0.1f);

    if (std::fabs(mParams.epsilon - epsilon) < 1e-6f) return;

    mParams.epsilon = epsilon;
    qDebug() << "[CutlineController] Epsilon changed:" << epsilon;

    emit paramsChanged(mParams);
    triggerCompute();
}

void CutlineController::setSplineTension(float tension) {
    // Clamp to [0.0, 1.0]
    tension = std::clamp(tension, 0.0f, 1.0f);

    if (std::fabs(mParams.splineTension - tension) < 1e-6f) return;

    mParams.splineTension = tension;
    qDebug() << "[CutlineController] Spline tension changed:" << tension;

    emit paramsChanged(mParams);
    triggerCompute();
}

void CutlineController::setColor(const QColor& color) {
    if (mParams.color == color) return;

    mParams.color = color;
    qDebug() << "[CutlineController] Color changed:" << color.name();

    emit paramsChanged(mParams);
    // Note: 색상 변경은 재계산 필요 없음 (렌더링만)
}

void CutlineController::setWidth(float width) {
    width = std::clamp(width, 0.5f, 10.0f);

    if (std::fabs(mParams.width - width) < 1e-6f) return;

    mParams.width = width;
    qDebug() << "[CutlineController] Width changed:" << width;

    emit paramsChanged(mParams);
    // Note: 두께 변경은 재계산 필요 없음 (렌더링만)
}

void CutlineController::setOpacity(float opacity) {
    opacity = std::clamp(opacity, 0.0f, 1.0f);

    if (std::fabs(mParams.opacity - opacity) < 1e-6f) return;

    mParams.opacity = opacity;
    qDebug() << "[CutlineController] Opacity changed:" << opacity;

    emit paramsChanged(mParams);
    // Note: 불투명도 변경은 재계산 필요 없음 (렌더링만)
}

void CutlineController::setSnapBoundingBox(bool enabled) {
    if (mParams.snapBoundingBox == enabled) return;

    mParams.snapBoundingBox = enabled;
    qDebug() << "[CutlineController] Snap bounding box:" << enabled;

    emit paramsChanged(mParams);
    // Snap 옵션은 UI 동작만 영향 (재계산 불필요)
}

void CutlineController::setSnapFeature(bool enabled) {
    if (mParams.snapFeature == enabled) return;

    mParams.snapFeature = enabled;
    qDebug() << "[CutlineController] Snap feature:" << enabled;

    emit paramsChanged(mParams);
    // Snap 옵션은 UI 동작만 영향 (재계산 불필요)
}

void CutlineController::onCameraChanged() {
    qDebug() << "[CutlineController] Camera changed, triggering recompute";
    triggerCompute();
}

void CutlineController::onPlaneChanged() {
    qDebug() << "[CutlineController] Plane changed, triggering immediate compute";
    computeImmediate();
}

void CutlineController::triggerCompute() {
    // 디바운스 타이머 리셋 (16ms 대기)
    mDebounceTimer->stop();
    mDebounceTimer->start();
}

void CutlineController::computeImmediate() {
    // 디바운스 무시하고 즉시 계산
    mDebounceTimer->stop();
    onDebounceTimeout();
}

void CutlineController::onDebounceTimeout() {
    qDebug() << "[CutlineController] Debounce timeout, starting async compute";

    // 성능 측정 시작
    auto startTime = std::chrono::high_resolution_clock::now();

    // 비동기 계산 시작
    RequestId requestId = mService->computeAsync(mParams);
    mLastSentRequestId = requestId;

    // 성능 측정 종료 (트리거 시간만 측정, 실제 계산은 비동기)
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    qDebug() << "[CutlineController] Sent request:" << requestId
             << "| Trigger time:" << duration.count() << "μs";

    // 60fps 목표 = 16.67ms/frame
    // 트리거는 매우 빨라야 함 (< 1ms)
    if (duration.count() > 1000) {
        qWarning() << "[Performance] Trigger time too slow:" << duration.count() << "μs (> 1ms)";
    }
}

void CutlineController::onPolylineComputed(
    const std::vector<Vector3D>& polyline,
    RequestId requestId,
    bool isRaw) {

    // 최신성 검증: 더 오래된 결과는 폐기
    if (requestId < mLastReceivedRequestId) {
        qDebug() << "[CutlineController] Dropped stale result:"
                 << "received=" << requestId
                 << "last=" << mLastReceivedRequestId;
        return;
    }

    // 최신 결과 수용
    mLastReceivedRequestId = requestId;

    if (isRaw) {
        qDebug() << "[CutlineController] Received raw polyline:"
                 << polyline.size() << "points (requestId:" << requestId << ")";
    } else {
        qDebug() << "[CutlineController] Received final polyline:"
                 << polyline.size() << "points (requestId:" << requestId << ")";
    }

    // View에 전달
    emit polylineReady(polyline, isRaw);
}

} // namespace Cutline
} // namespace DongArch
