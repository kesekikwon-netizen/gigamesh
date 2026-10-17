/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 2: Catmull-Rom Spline Implementation
 */

#include "CatmullRomSpline.h"
#include <iostream>

namespace DongArch {
namespace Cutline {
namespace Algorithms {

//==============================================================================
// Public Methods - 3D Interpolation
//==============================================================================

std::vector<Vector3D> CatmullRomSpline::interpolate(
    std::span<const Vector3D> controlPoints,
    int segmentsPerSpan,
    SplineType type) const
{
    if (controlPoints.size() < 2) {
        // 점이 2개 미만이면 그대로 반환
        return std::vector<Vector3D>(controlPoints.begin(), controlPoints.end());
    }

    if (controlPoints.size() == 2) {
        // 점이 2개면 직선 보간
        std::vector<Vector3D> result;
        result.reserve(segmentsPerSpan + 1);

        for (int i = 0; i <= segmentsPerSpan; i++) {
            double t = static_cast<double>(i) / segmentsPerSpan;
            Vector3D point = controlPoints[0] * (1.0 - t) + controlPoints[1] * t;
            result.push_back(point);
        }

        return result;
    }

    if (controlPoints.size() == 3) {
        // 점이 3개면 중간 점에서 곡선 생성 (양 끝에 가상 점 추가)
        Vector3D p_before = controlPoints[0] * 2.0 - controlPoints[1];
        Vector3D p_after = controlPoints[2] * 2.0 - controlPoints[1];

        std::vector<Vector3D> result;
        result.reserve(segmentsPerSpan * 2 + 1);

        interpolateSpan(p_before, controlPoints[0], controlPoints[1], controlPoints[2], segmentsPerSpan, type, result);
        interpolateSpan(controlPoints[0], controlPoints[1], controlPoints[2], p_after, segmentsPerSpan, type, result);

        return result;
    }

    // 점이 4개 이상: Catmull-Rom Spline
    std::vector<Vector3D> result;
    result.reserve((controlPoints.size() - 1) * segmentsPerSpan + 1);

    // 첫 번째 점 추가
    result.push_back(controlPoints[0]);

    // 각 스팬 보간 (n개 점이면 n-1개 스팬)
    for (size_t i = 0; i < controlPoints.size() - 1; i++) {
        // 4개의 제어점 선택: p0, p1, p2, p3
        // 현재 스팬: p1 -> p2

        Vector3D p0, p1, p2, p3;

        if (i == 0) {
            // 첫 스팬: p0을 가상으로 생성
            p0 = controlPoints[0] * 2.0 - controlPoints[1];
            p1 = controlPoints[0];
            p2 = controlPoints[1];
            p3 = controlPoints[2];
        } else if (i == controlPoints.size() - 2) {
            // 마지막 스팬: p3을 가상으로 생성
            p0 = controlPoints[i - 1];
            p1 = controlPoints[i];
            p2 = controlPoints[i + 1];
            p3 = controlPoints[i + 1] * 2.0 - controlPoints[i];
        } else {
            // 중간 스팬
            p0 = controlPoints[i - 1];
            p1 = controlPoints[i];
            p2 = controlPoints[i + 1];
            p3 = controlPoints[i + 2];
        }

        // 스팬 보간 (p1 제외, 이미 추가됨)
        std::vector<Vector3D> spanPoints;
        interpolateSpan(p0, p1, p2, p3, segmentsPerSpan, type, spanPoints);

        // 첫 점 제외하고 추가 (p1은 이미 result에 있음)
        for (size_t j = 1; j < spanPoints.size(); j++) {
            result.push_back(spanPoints[j]);
        }
    }

    return result;
}

std::vector<Vector3D> CatmullRomSpline::interpolate(
    std::span<const Vector3D> controlPoints,
    CurveLevel level,
    SplineType type) const
{
    int segments = curveLevelToSegments(level);
    return interpolate(controlPoints, segments, type);
}

//==============================================================================
// Public Methods - 2D Interpolation
//==============================================================================

std::vector<Point2D> CatmullRomSpline::interpolate2D(
    std::span<const Point2D> controlPoints,
    int segmentsPerSpan,
    SplineType type) const
{
    if (controlPoints.size() < 2) {
        return std::vector<Point2D>(controlPoints.begin(), controlPoints.end());
    }

    if (controlPoints.size() == 2) {
        std::vector<Point2D> result;
        result.reserve(segmentsPerSpan + 1);

        for (int i = 0; i <= segmentsPerSpan; i++) {
            double t = static_cast<double>(i) / segmentsPerSpan;
            Point2D point;
            point.x = controlPoints[0].x * (1.0 - t) + controlPoints[1].x * t;
            point.y = controlPoints[0].y * (1.0 - t) + controlPoints[1].y * t;
            result.push_back(point);
        }

        return result;
    }

    std::vector<Point2D> result;
    result.reserve((controlPoints.size() - 1) * segmentsPerSpan + 1);
    result.push_back(controlPoints[0]);

    for (size_t i = 0; i < controlPoints.size() - 1; i++) {
        Point2D p0, p1, p2, p3;

        if (i == 0) {
            p0.x = controlPoints[0].x * 2.0 - controlPoints[1].x;
            p0.y = controlPoints[0].y * 2.0 - controlPoints[1].y;
            p1 = controlPoints[0];
            p2 = controlPoints[1];
            p3 = controlPoints[2];
        } else if (i == controlPoints.size() - 2) {
            p0 = controlPoints[i - 1];
            p1 = controlPoints[i];
            p2 = controlPoints[i + 1];
            p3.x = controlPoints[i + 1].x * 2.0 - controlPoints[i].x;
            p3.y = controlPoints[i + 1].y * 2.0 - controlPoints[i].y;
        } else {
            p0 = controlPoints[i - 1];
            p1 = controlPoints[i];
            p2 = controlPoints[i + 1];
            p3 = controlPoints[i + 2];
        }

        std::vector<Point2D> spanPoints;
        interpolateSpan2D(p0, p1, p2, p3, segmentsPerSpan, type, spanPoints);

        for (size_t j = 1; j < spanPoints.size(); j++) {
            result.push_back(spanPoints[j]);
        }
    }

    return result;
}

//==============================================================================
// Private Methods - Span Interpolation (3D)
//==============================================================================

void CatmullRomSpline::interpolateSpan(
    const Vector3D& p0,
    const Vector3D& p1,
    const Vector3D& p2,
    const Vector3D& p3,
    int segments,
    SplineType type,
    std::vector<Vector3D>& output) const
{
    // alpha 값: 0 = uniform, 0.5 = centripetal, 1 = chordal
    double alpha = 0.0;
    switch (type) {
        case SplineType::Uniform:     alpha = 0.0; break;
        case SplineType::Centripetal: alpha = 0.5; break;
        case SplineType::Chordal:     alpha = 1.0; break;
    }

    // 파라미터 t 계산
    double t0 = 0.0;
    double t1 = calculateT(t0, p0, p1, alpha);
    double t2 = calculateT(t1, p1, p2, alpha);
    double t3 = calculateT(t2, p2, p3, alpha);

    // p1에서 p2로 보간
    for (int i = 0; i <= segments; i++) {
        double t = t1 + (t2 - t1) * static_cast<double>(i) / segments;

        // Catmull-Rom 공식 (Barry-Goldman form)
        Vector3D A1 = p0 * ((t1 - t) / (t1 - t0)) + p1 * ((t - t0) / (t1 - t0));
        Vector3D A2 = p1 * ((t2 - t) / (t2 - t1)) + p2 * ((t - t1) / (t2 - t1));
        Vector3D A3 = p2 * ((t3 - t) / (t3 - t2)) + p3 * ((t - t2) / (t3 - t2));

        Vector3D B1 = A1 * ((t2 - t) / (t2 - t0)) + A2 * ((t - t0) / (t2 - t0));
        Vector3D B2 = A2 * ((t3 - t) / (t3 - t1)) + A3 * ((t - t1) / (t3 - t1));

        Vector3D C = B1 * ((t2 - t) / (t2 - t1)) + B2 * ((t - t1) / (t2 - t1));

        output.push_back(C);
    }
}

//==============================================================================
// Private Methods - Span Interpolation (2D)
//==============================================================================

void CatmullRomSpline::interpolateSpan2D(
    const Point2D& p0,
    const Point2D& p1,
    const Point2D& p2,
    const Point2D& p3,
    int segments,
    SplineType type,
    std::vector<Point2D>& output) const
{
    double alpha = 0.0;
    switch (type) {
        case SplineType::Uniform:     alpha = 0.0; break;
        case SplineType::Centripetal: alpha = 0.5; break;
        case SplineType::Chordal:     alpha = 1.0; break;
    }

    double t0 = 0.0;
    double t1 = calculateT2D(t0, p0, p1, alpha);
    double t2 = calculateT2D(t1, p1, p2, alpha);
    double t3 = calculateT2D(t2, p2, p3, alpha);

    for (int i = 0; i <= segments; i++) {
        double t = t1 + (t2 - t1) * static_cast<double>(i) / segments;

        // Barry-Goldman form (2D)
        auto lerp2D = [](const Point2D& a, const Point2D& b, double t) -> Point2D {
            Point2D result;
            result.x = a.x * (1.0 - t) + b.x * t;
            result.y = a.y * (1.0 - t) + b.y * t;
            return result;
        };

        Point2D A1 = lerp2D(p0, p1, (t - t0) / (t1 - t0));
        Point2D A2 = lerp2D(p1, p2, (t - t1) / (t2 - t1));
        Point2D A3 = lerp2D(p2, p3, (t - t2) / (t3 - t2));

        Point2D B1 = lerp2D(A1, A2, (t - t0) / (t2 - t0));
        Point2D B2 = lerp2D(A2, A3, (t - t1) / (t3 - t1));

        Point2D C = lerp2D(B1, B2, (t - t1) / (t2 - t1));

        output.push_back(C);
    }
}

//==============================================================================
// Private Methods - Parameter Calculation
//==============================================================================

double CatmullRomSpline::calculateT(
    double t0,
    const Vector3D& p0,
    const Vector3D& p1,
    double alpha) const
{
    Vector3D diff = p1 - p0;
    double distance = diff.getLength3();

    if (distance < 1e-10) {
        return t0; // 같은 점이면 t 증가 안 함
    }

    return t0 + std::pow(distance, alpha);
}

double CatmullRomSpline::calculateT2D(
    double t0,
    const Point2D& p0,
    const Point2D& p1,
    double alpha) const
{
    double dx = p1.x - p0.x;
    double dy = p1.y - p0.y;
    double distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 1e-10) {
        return t0;
    }

    return t0 + std::pow(distance, alpha);
}

} // namespace Algorithms
} // namespace Cutline
} // namespace DongArch
