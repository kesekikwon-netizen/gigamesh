/* * DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * This file is part of DongArch3D.
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_QUADRIC_OPTIMIZER_H
#define DONGARCH_QUADRIC_OPTIMIZER_H

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/face.h>
#include <GigaMesh/mesh/vector3d.h>
#include "dongarch/common/DongArchTypes.h"
#include "dongarch/common/DongArchMath.h"
#include <vector>
#include <span>
#include <array>

//! \file QuadricOptimizer.h
//! \brief Phase 3: Quadric-based Silhouette Optimization (2025)
//!
//! DongArch3D v4.0 Phase 3 구현
//! - Quadric Error Metric (Garland & Heckbert 1997)
//! - 2025년 최신 논문: "Quadric-Based Silhouette Sampling for Differentiable Rendering"
//! - Douglas-Peucker + Quadric 하이브리드
//! - 고곡률 영역 보존

namespace DongArch {
namespace Outline {
namespace Algorithms {

//! \class QuadricMatrix
//! \brief 4x4 Quadric Error Matrix
//!
//! Quadric Error Metric을 위한 대칭 행렬 Q
//! Q = Σ K_p (인접 평면들의 quadric 합)
//! Error(v) = v^T * Q * v
class QuadricMatrix {
public:
    //! 기본 생성자 (영행렬)
    QuadricMatrix();

    //! 평면으로부터 Quadric Matrix 생성
    //! \param plane 평면 방정식 (a, b, c, d): ax + by + cz + d = 0
    explicit QuadricMatrix(const Vector3D& plane);

    //! Quadric Matrix 요소 설정
    //! \param row 행 인덱스 (0-3)
    //! \param col 열 인덱스 (0-3)
    //! \param value 값
    void set(int row, int col, double value);

    //! Quadric Matrix 요소 조회
    //! \param row 행 인덱스 (0-3)
    //! \param col 열 인덱스 (0-3)
    //! \return 값
    double get(int row, int col) const;

    //! Quadric Error 계산
    //! \param v 정점 위치
    //! \return Error(v) = v^T * Q * v
    double computeError(const Vector3D& v) const;

    //! Quadric Matrix 덧셈
    //! \param other 더할 Quadric Matrix
    //! \return 합
    QuadricMatrix operator+(const QuadricMatrix& other) const;

    //! Quadric Matrix 누적 덧셈
    //! \param other 더할 Quadric Matrix
    //! \return this
    QuadricMatrix& operator+=(const QuadricMatrix& other);

    //! 스칼라 곱셈
    //! \param scalar 스칼라 값
    //! \return 곱
    QuadricMatrix operator*(double scalar) const;

    //! 영행렬 초기화
    void clear();

private:
    std::array<double, 16> mData;  //!< 4x4 대칭 행렬 (row-major)
};

//! \struct SimplificationResult
//! \brief Polyline 간략화 결과
struct SimplificationResult {
    std::vector<DongArch::Math::Vec3> points;  //!< 간략화된 점들
    size_t originalCount;                       //!< 원본 점 개수
    size_t simplifiedCount;                     //!< 간략화 후 점 개수
    double averageError;                        //!< 평균 Quadric Error
    double maxError;                            //!< 최대 Quadric Error
    double compressionRatio;                    //!< 압축률 (%)
};

//! \class QuadricOptimizer
//! \brief Quadric Error Metric 기반 Silhouette 최적화
//!
//! 2025년 논문 기반:
//! - Quadric-Based Silhouette Sampling for Differentiable Rendering
//! - Mariia Soroka, Christoph Peters, Steve Marschner
//! - DOI: 10.1145/3731146
//!
//! 기능:
//! - Quadric Error 계산
//! - Douglas-Peucker + Quadric 하이브리드
//! - 고곡률 영역 보존
class QuadricOptimizer {
public:
    //! 생성자
    //! \param mesh 대상 메시 (non-owning)
    explicit QuadricOptimizer(Mesh* mesh);

    //! 소멸자
    ~QuadricOptimizer() = default;

    // Copy/Move 금지 (non-owning pointer)
    QuadricOptimizer(const QuadricOptimizer&) = delete;
    QuadricOptimizer& operator=(const QuadricOptimizer&) = delete;
    QuadricOptimizer(QuadricOptimizer&&) = delete;
    QuadricOptimizer& operator=(QuadricOptimizer&&) = delete;

    //! 메시 유효성 확인
    //! \return 메시가 유효하면 true
    bool isValid() const { return mMesh != nullptr; }

    //! 정점의 Quadric Error 계산
    //! \param vertex 대상 정점
    //! \return Quadric Error 값
    double quadricError(const Vertex* vertex) const;

    //! 정점의 Quadric Matrix 계산
    //! \param vertex 대상 정점
    //! \return Quadric Matrix (인접 평면들의 합)
    QuadricMatrix computeQuadricMatrix(const Vertex* vertex) const;

    //! Polyline 간략화 (Douglas-Peucker)
    //! \param points 원본 점들
    //! \param epsilon 거리 임계값
    //! \param result 간략화 결과
    //! \return 성공 여부
    bool simplifyDouglasPeucker(std::span<const DongArch::Math::Vec3> points,
                                double epsilon,
                                SimplificationResult& result);

    //! Polyline 간략화 (Quadric 기반)
    //! \param points 원본 점들
    //! \param errorThreshold Quadric Error 임계값
    //! \param result 간략화 결과
    //! \return 성공 여부
    bool simplifyQuadric(std::span<const DongArch::Math::Vec3> points,
                         double errorThreshold,
                         SimplificationResult& result);

    //! Polyline 간략화 (하이브리드: Douglas-Peucker + Quadric)
    //! \param points 원본 점들
    //! \param epsilon Douglas-Peucker 거리 임계값
    //! \param errorThreshold Quadric Error 임계값
    //! \param result 간략화 결과
    //! \return 성공 여부
    bool simplifyHybrid(std::span<const DongArch::Math::Vec3> points,
                        double epsilon,
                        double errorThreshold,
                        SimplificationResult& result);

    //! 고곡률 영역 검출
    //! \param points Polyline 점들
    //! \param curvatureThreshold 곡률 임계값
    //! \return 고곡률 영역 인덱스 목록
    std::vector<size_t> detectHighCurvatureRegions(
        std::span<const DongArch::Math::Vec3> points,
        double curvatureThreshold) const;

    //! 점의 곡률 계산
    //! \param prev 이전 점
    //! \param current 현재 점
    //! \param next 다음 점
    //! \return 곡률 값 (각도 기반)
    static double computeCurvature(const DongArch::Math::Vec3& prev,
                                   const DongArch::Math::Vec3& current,
                                   const DongArch::Math::Vec3& next);

private:
    //! Douglas-Peucker 재귀 함수
    //! \param points 점들
    //! \param start 시작 인덱스
    //! \param end 종료 인덱스
    //! \param epsilon 거리 임계값
    //! \param keep 보존할 점 인덱스 (출력)
    void douglasPeuckerRecursive(std::span<const DongArch::Math::Vec3> points,
                                 size_t start,
                                 size_t end,
                                 double epsilon,
                                 std::vector<bool>& keep);

    //! 점과 선분 사이의 수직 거리
    //! \param point 점
    //! \param lineStart 선분 시작점
    //! \param lineEnd 선분 끝점
    //! \return 수직 거리
    static double perpendicularDistance(const DongArch::Math::Vec3& point,
                                        const DongArch::Math::Vec3& lineStart,
                                        const DongArch::Math::Vec3& lineEnd);

    //! 정점 찾기 (Vec3 → Vertex*)
    //! \param pos 위치
    //! \param tolerance 허용 오차
    //! \return 정점 포인터 (없으면 nullptr)
    Vertex* findVertex(const DongArch::Math::Vec3& pos, double tolerance = 0.001) const;

    Mesh* mMesh;  //!< 대상 메시 (non-owning)
};

} // namespace Algorithms
} // namespace Outline
} // namespace DongArch

#endif // DONGARCH_QUADRIC_OPTIMIZER_H
