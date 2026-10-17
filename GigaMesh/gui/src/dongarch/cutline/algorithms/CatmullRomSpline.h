/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 2: Catmull-Rom Spline Curve Fitting
 *
 * 참조: "A class of local interpolating splines"
 *       Catmull & Rom, 1974 (Classic Algorithm)
 *       Wikipedia: https://en.wikipedia.org/wiki/Centripetal_Catmull-Rom_spline
 *
 * Centripetal Catmull-Rom Spline:
 * - C1 연속성 보장 (smooth curve)
 * - 제어점을 통과하는 보간 곡선
 * - Overshoot/undershoot 최소화
 *
 * This file is part of DongArch3D (based on GigaMesh).
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_CATMULL_ROM_SPLINE_H
#define DONGARCH_CATMULL_ROM_SPLINE_H

#include "dongarch/common/DongArchTypes.h"
#include <GigaMesh/mesh/vector3d.h>
#include <vector>
#include <span>
#include <cmath>

namespace DongArch {
namespace Cutline {
namespace Algorithms {

// Point2D 타입을 DongArchTypes.h에서 가져옴
using Point2D = DongArch::Point2D;

//! \class CatmullRomSpline
//! \brief Catmull-Rom Spline 보간 (1974)
//!
//! Centripetal Catmull-Rom Spline을 사용하여 부드러운 곡선 생성
//!
//! 특징:
//! - 모든 제어점을 통과 (interpolation)
//! - C1 연속성 (1차 미분 연속)
//! - Self-intersection과 cusp 최소화
//!
//! 사용법:
//! ```cpp
//! std::vector<Vector3D> controlPoints = ...; // 제어점들
//! CatmullRomSpline spline;
//! auto curve = spline.interpolate(controlPoints, CurveLevelToSegments(CurveLevel::High));
//! ```
class CatmullRomSpline
{
public:
    //! Curve Level (UI 선택용)
    enum class CurveLevel {
        Low,    //!< 10 segments per span (빠름, 거친 곡선)
        Medium, //!< 20 segments per span (기본값)
        High    //!< 40 segments per span (느림, 부드러운 곡선)
    };

    //! Spline Type
    enum class SplineType {
        Uniform,      //!< Uniform Catmull-Rom (간단하지만 loop/cusp 발생 가능)
        Centripetal,  //!< Centripetal (권장, overshoot 최소화)
        Chordal       //!< Chordal (매우 부드러움, 약간 느림)
    };

    CatmullRomSpline() = default;
    ~CatmullRomSpline() = default;

    //! Spline 보간 (3D)
    //! \param controlPoints 제어점들 (최소 4개 필요)
    //! \param segmentsPerSpan 각 스팬당 세그먼트 수
    //! \param type Spline 타입
    //! \return 보간된 곡선 점들
    std::vector<Vector3D> interpolate(
        std::span<const Vector3D> controlPoints,
        int segmentsPerSpan = 20,
        SplineType type = SplineType::Centripetal
    ) const;

    //! Spline 보간 (CurveLevel 사용)
    //! \param controlPoints 제어점들
    //! \param level Curve level
    //! \param type Spline 타입
    //! \return 보간된 곡선 점들
    std::vector<Vector3D> interpolate(
        std::span<const Vector3D> controlPoints,
        CurveLevel level,
        SplineType type = SplineType::Centripetal
    ) const;

    //! Spline 보간 (2D)
    //! \param controlPoints 제어점들 (최소 4개 필요)
    //! \param segmentsPerSpan 각 스팬당 세그먼트 수
    //! \param type Spline 타입
    //! \return 보간된 곡선 점들
    std::vector<Point2D> interpolate2D(
        std::span<const Point2D> controlPoints,
        int segmentsPerSpan = 20,
        SplineType type = SplineType::Centripetal
    ) const;

    //! CurveLevel을 segmentsPerSpan으로 변환
    //! \param level Curve level
    //! \return Segments per span
    static constexpr int curveLevelToSegments(CurveLevel level);

private:
    //! 단일 스팬 보간 (p0-p1-p2-p3 중 p1-p2 구간)
    //! \param p0 이전 제어점
    //! \param p1 현재 스팬 시작점
    //! \param p2 현재 스팬 끝점
    //! \param p3 다음 제어점
    //! \param segments 세그먼트 수
    //! \param type Spline 타입
    //! \param output 출력 벡터 (append)
    void interpolateSpan(
        const Vector3D& p0,
        const Vector3D& p1,
        const Vector3D& p2,
        const Vector3D& p3,
        int segments,
        SplineType type,
        std::vector<Vector3D>& output
    ) const;

    //! 단일 스팬 보간 (2D)
    void interpolateSpan2D(
        const Point2D& p0,
        const Point2D& p1,
        const Point2D& p2,
        const Point2D& p3,
        int segments,
        SplineType type,
        std::vector<Point2D>& output
    ) const;

    //! 파라미터 t 계산 (Centripetal/Chordal용)
    //! \param t0 이전 t
    //! \param p0 이전 점
    //! \param p1 현재 점
    //! \param alpha 0: uniform, 0.5: centripetal, 1: chordal
    //! \return 다음 t
    double calculateT(
        double t0,
        const Vector3D& p0,
        const Vector3D& p1,
        double alpha
    ) const;

    //! 파라미터 t 계산 (2D)
    double calculateT2D(
        double t0,
        const Point2D& p0,
        const Point2D& p1,
        double alpha
    ) const;
};

//==============================================================================
// Inline Implementations
//==============================================================================

constexpr int CatmullRomSpline::curveLevelToSegments(CurveLevel level)
{
    switch (level) {
        case CurveLevel::Low:    return 10;
        case CurveLevel::Medium: return 20;
        case CurveLevel::High:   return 40;
        default:                 return 20;
    }
}

} // namespace Algorithms
} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CATMULL_ROM_SPLINE_H
