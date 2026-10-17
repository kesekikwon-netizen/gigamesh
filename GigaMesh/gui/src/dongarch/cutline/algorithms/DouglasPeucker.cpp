/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 2: Douglas-Peucker Line Simplification Implementation
 */

#include "DouglasPeucker.h"
#include <iostream>
#include <numeric>

namespace DongArch {
namespace Cutline {
namespace Algorithms {

//==============================================================================
// Public Methods - 3D Simplification
//==============================================================================

std::vector<Vector3D> DouglasPeucker::simplify(
    std::span<const Vector3D> points,
    double tolerance) const
{
    if (points.size() < 3) {
        // 점이 2개 이하면 단순화 불필요
        return std::vector<Vector3D>(points.begin(), points.end());
    }

    // Keep flags: true면 유지, false면 제거
    std::vector<bool> keep(points.size(), false);

    // 시작점과 끝점은 항상 유지
    keep[0] = true;
    keep[points.size() - 1] = true;

    // 재귀적 단순화
    douglasPeuckerRecursive(points, 0, points.size() - 1, tolerance, keep);

    // C++20 Ranges를 사용하여 유지할 점들만 필터링
    std::vector<Vector3D> simplified;
    simplified.reserve(points.size());

    for (size_t i = 0; i < points.size(); i++) {
        if (keep[i]) {
            simplified.push_back(points[i]);
        }
    }

    // 통계 계산
    calculateStatistics(points, std::span<const Vector3D>{simplified}, tolerance);

    return simplified;
}

std::vector<Vector3D> DouglasPeucker::simplify(
    std::span<const Vector3D> points,
    DetailLevel level) const
{
    double tolerance = levelToTolerance(level);
    return simplify(points, tolerance);
}

//==============================================================================
// Public Methods - 2D Simplification
//==============================================================================

std::vector<Point2D> DouglasPeucker::simplify2D(
    std::span<const Point2D> points,
    double tolerance) const
{
    if (points.size() < 3) {
        return std::vector<Point2D>(points.begin(), points.end());
    }

    std::vector<bool> keep(points.size(), false);
    keep[0] = true;
    keep[points.size() - 1] = true;

    douglasPeuckerRecursive2D(points, 0, points.size() - 1, tolerance, keep);

    // C++20 Ranges 필터링
    std::vector<Point2D> simplified;
    simplified.reserve(points.size());

    for (size_t i = 0; i < points.size(); i++) {
        if (keep[i]) {
            simplified.push_back(points[i]);
        }
    }

    return simplified;
}

//==============================================================================
// Private Methods - Recursive Algorithm (3D)
//==============================================================================

void DouglasPeucker::douglasPeuckerRecursive(
    std::span<const Vector3D> points,
    size_t start,
    size_t end,
    double tolerance,
    std::vector<bool>& keep) const
{
    if (end <= start + 1) {
        // 인접한 점들이면 재귀 종료
        return;
    }

    // 직선 세그먼트: points[start] -> points[end]
    const Vector3D& lineStart = points[start];
    const Vector3D& lineEnd = points[end];

    // 가장 먼 점 찾기
    double maxDistance = 0.0;
    size_t maxIndex = start;

    for (size_t i = start + 1; i < end; i++) {
        double distance = perpendicularDistance(points[i], lineStart, lineEnd);
        if (distance > maxDistance) {
            maxDistance = distance;
            maxIndex = i;
        }
    }

    // 최대 거리가 tolerance보다 크면 분할
    if (maxDistance > tolerance) {
        // 가장 먼 점을 유지
        keep[maxIndex] = true;

        // 재귀적으로 분할
        douglasPeuckerRecursive(points, start, maxIndex, tolerance, keep);
        douglasPeuckerRecursive(points, maxIndex, end, tolerance, keep);
    }
    // else: tolerance 이하면 직선으로 근사 (중간 점들 제거)
}

//==============================================================================
// Private Methods - Recursive Algorithm (2D)
//==============================================================================

void DouglasPeucker::douglasPeuckerRecursive2D(
    std::span<const Point2D> points,
    size_t start,
    size_t end,
    double tolerance,
    std::vector<bool>& keep) const
{
    if (end <= start + 1) {
        return;
    }

    const Point2D& lineStart = points[start];
    const Point2D& lineEnd = points[end];

    double maxDistance = 0.0;
    size_t maxIndex = start;

    for (size_t i = start + 1; i < end; i++) {
        double distance = perpendicularDistance2D(points[i], lineStart, lineEnd);
        if (distance > maxDistance) {
            maxDistance = distance;
            maxIndex = i;
        }
    }

    if (maxDistance > tolerance) {
        keep[maxIndex] = true;
        douglasPeuckerRecursive2D(points, start, maxIndex, tolerance, keep);
        douglasPeuckerRecursive2D(points, maxIndex, end, tolerance, keep);
    }
}

//==============================================================================
// Private Methods - Distance Calculation (3D)
//==============================================================================

double DouglasPeucker::perpendicularDistance(
    const Vector3D& point,
    const Vector3D& lineStart,
    const Vector3D& lineEnd) const
{
    // 점 P에서 직선 AB까지의 수직 거리
    // 공식: |AP × AB| / |AB|
    //
    // AP = P - A
    // AB = B - A
    // Cross product: AP × AB
    // Distance = |AP × AB| / |AB|

    Vector3D AP = point - lineStart;
    Vector3D AB = lineEnd - lineStart;

    // AB의 길이
    double AB_length = AB.getLength3();

    if (AB_length < 1e-10) {
        // A와 B가 거의 같은 점이면 AP의 길이 반환
        return AP.getLength3();
    }

    // Cross product: AP × AB
    Vector3D cross = AP % AB;  // GigaMesh Vector3D의 % 연산자는 외적

    // Distance = |AP × AB| / |AB|
    double distance = cross.getLength3() / AB_length;

    return distance;
}

//==============================================================================
// Private Methods - Distance Calculation (2D)
//==============================================================================

double DouglasPeucker::perpendicularDistance2D(
    const Point2D& point,
    const Point2D& lineStart,
    const Point2D& lineEnd) const
{
    // 2D 점에서 직선까지의 수직 거리
    // 공식: |ax + by + c| / sqrt(a² + b²)
    //
    // 직선 방정식: (y2-y1)x - (x2-x1)y + (x2-x1)y1 - (y2-y1)x1 = 0

    double x = point.x;
    double y = point.y;
    double x1 = lineStart.x;
    double y1 = lineStart.y;
    double x2 = lineEnd.x;
    double y2 = lineEnd.y;

    double dx = x2 - x1;
    double dy = y2 - y1;

    double line_length_sq = dx * dx + dy * dy;

    if (line_length_sq < 1e-10) {
        // 시작점과 끝점이 거의 같으면 점 사이 거리 반환
        double px = x - x1;
        double py = y - y1;
        return std::sqrt(px * px + py * py);
    }

    // 수직 거리 계산
    double numerator = std::abs(dy * x - dx * y + x2 * y1 - y2 * x1);
    double denominator = std::sqrt(line_length_sq);

    return numerator / denominator;
}

//==============================================================================
// Statistics
//==============================================================================

void DouglasPeucker::calculateStatistics(
    std::span<const Vector3D> original,
    std::span<const Vector3D> simplified,
    double tolerance) const
{
    mLastStats.originalPointCount = original.size();
    mLastStats.simplifiedPointCount = simplified.size();

    if (original.size() > 0) {
        mLastStats.reductionRatio = 100.0 * (1.0 - static_cast<double>(simplified.size()) / original.size());
    } else {
        mLastStats.reductionRatio = 0.0;
    }

    // 최대 편차 및 평균 편차 계산
    // (단순화된 폴리라인과 원본 점들 사이의 최소 거리)

    mLastStats.maxDeviation = 0.0;
    double totalDeviation = 0.0;

    for (const auto& origPoint : original) {
        // 단순화된 폴리라인에서 가장 가까운 세그먼트 찾기
        double minDist = std::numeric_limits<double>::max();

        for (size_t i = 0; i < simplified.size() - 1; i++) {
            double dist = perpendicularDistance(origPoint, simplified[i], simplified[i + 1]);
            minDist = std::min(minDist, dist);
        }

        mLastStats.maxDeviation = std::max(mLastStats.maxDeviation, minDist);
        totalDeviation += minDist;
    }

    if (original.size() > 0) {
        mLastStats.avgDeviation = totalDeviation / original.size();
    } else {
        mLastStats.avgDeviation = 0.0;
    }
}

} // namespace Algorithms
} // namespace Cutline
} // namespace DongArch
