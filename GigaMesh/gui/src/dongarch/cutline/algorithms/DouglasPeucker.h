/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 2: Douglas-Peucker Line Simplification Algorithm
 *
 * 참조: "Algorithms for the Reduction of the Number of Points Required
 *        to Represent a Digitized Line or its Caricature"
 *       Douglas & Peucker, 1973 (Classic Algorithm)
 *       Cited: 268 times (research-papers.md)
 *       Wikipedia: https://en.wikipedia.org/wiki/Ramer-Douglas-Peucker_algorithm
 *
 * This file is part of DongArch3D (based on GigaMesh).
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_DOUGLAS_PEUCKER_H
#define DONGARCH_DOUGLAS_PEUCKER_H

#include "dongarch/common/DongArchTypes.h"
#include <GigaMesh/mesh/vector3d.h>
#include <vector>
#include <span>
#include <ranges>
#include <algorithm>
#include <cmath>

namespace DongArch {
namespace Cutline {
namespace Algorithms {

// Point2D 타입을 DongArchTypes.h에서 가져옴
using Point2D = DongArch::Point2D;

//! \class DouglasPeucker
//! \brief Douglas-Peucker 라인 단순화 알고리즘 (1973)
//!
//! 폴리라인의 점 개수를 줄이면서 원본 형태를 최대한 유지하는 알고리즘
//!
//! 알고리즘:
//! 1. 시작점과 끝점을 연결하는 직선 생성
//! 2. 직선으로부터 가장 먼 점 찾기
//! 3. 거리 > tolerance이면 재귀적으로 분할
//! 4. 거리 <= tolerance이면 직선으로 근사
//!
//! 사용법:
//! ```cpp
//! std::vector<Vector3D> points = ...; // Original polyline
//! DouglasPeucker dp;
//! auto simplified = dp.simplify(points, 0.5); // tolerance = 0.5mm
//! ```
//!
//! 성능: O(n log n) average, O(n²) worst case
class DouglasPeucker
{
public:
    //! Detail Level (UI 슬라이더용)
    enum class DetailLevel {
        VeryHigh,  //!< 0.1mm tolerance (거의 원본)
        High,      //!< 0.3mm tolerance
        Medium,    //!< 0.5mm tolerance (기본값)
        Low,       //!< 1.0mm tolerance
        VeryLow    //!< 2.0mm tolerance (매우 단순화)
    };

    DouglasPeucker() = default;
    ~DouglasPeucker() = default;

    //! 폴리라인 단순화 (tolerance 직접 지정)
    //! \param points 원본 폴리라인 점들 (3D)
    //! \param tolerance 허용 오차 (mm)
    //! \return 단순화된 폴리라인
    std::vector<Vector3D> simplify(std::span<const Vector3D> points, double tolerance) const;

    //! 폴리라인 단순화 (DetailLevel 사용)
    //! \param points 원본 폴리라인 점들 (3D)
    //! \param level Detail level
    //! \return 단순화된 폴리라인
    std::vector<Vector3D> simplify(std::span<const Vector3D> points, DetailLevel level) const;

    //! 2D 폴리라인 단순화 (tolerance 직접 지정)
    //! \param points 원본 폴리라인 점들 (2D)
    //! \param tolerance 허용 오차 (mm)
    //! \return 단순화된 폴리라인
    std::vector<Point2D> simplify2D(std::span<const Point2D> points, double tolerance) const;

    //! DetailLevel을 tolerance 값으로 변환
    //! \param level Detail level
    //! \return Tolerance (mm)
    static constexpr double levelToTolerance(DetailLevel level);

    //! 단순화 통계
    struct Statistics {
        size_t originalPointCount;   //!< 원본 점 개수
        size_t simplifiedPointCount; //!< 단순화 후 점 개수
        double reductionRatio;       //!< 감소 비율 (%)
        double maxDeviation;         //!< 최대 편차 (mm)
        double avgDeviation;         //!< 평균 편차 (mm)
    };

    //! 마지막 단순화 통계 조회
    //! \return 통계 정보
    const Statistics& getLastStatistics() const { return mLastStats; }

private:
    //! Douglas-Peucker 재귀 구현 (3D)
    //! \param points 점들
    //! \param start 시작 인덱스
    //! \param end 끝 인덱스
    //! \param tolerance 허용 오차
    //! \param keep 유지할 점들의 인덱스 (output)
    void douglasPeuckerRecursive(
        std::span<const Vector3D> points,
        size_t start,
        size_t end,
        double tolerance,
        std::vector<bool>& keep
    ) const;

    //! Douglas-Peucker 재귀 구현 (2D)
    void douglasPeuckerRecursive2D(
        std::span<const Point2D> points,
        size_t start,
        size_t end,
        double tolerance,
        std::vector<bool>& keep
    ) const;

    //! 점에서 직선까지의 수직 거리 계산 (3D)
    //! \param point 점
    //! \param lineStart 직선 시작점
    //! \param lineEnd 직선 끝점
    //! \return 수직 거리
    double perpendicularDistance(
        const Vector3D& point,
        const Vector3D& lineStart,
        const Vector3D& lineEnd
    ) const;

    //! 점에서 직선까지의 수직 거리 계산 (2D)
    double perpendicularDistance2D(
        const Point2D& point,
        const Point2D& lineStart,
        const Point2D& lineEnd
    ) const;

    //! 통계 계산
    //! \param original 원본 점들
    //! \param simplified 단순화된 점들
    //! \param tolerance 허용 오차
    void calculateStatistics(
        std::span<const Vector3D> original,
        std::span<const Vector3D> simplified,
        double tolerance
    ) const;

    mutable Statistics mLastStats;  //!< 마지막 단순화 통계
};

//==============================================================================
// Inline Implementations
//==============================================================================

constexpr double DouglasPeucker::levelToTolerance(DetailLevel level)
{
    switch (level) {
        case DetailLevel::VeryHigh: return 0.1;
        case DetailLevel::High:     return 0.3;
        case DetailLevel::Medium:   return 0.5;
        case DetailLevel::Low:      return 1.0;
        case DetailLevel::VeryLow:  return 2.0;
        default:                    return 0.5;
    }
}

} // namespace Algorithms
} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_DOUGLAS_PEUCKER_H
