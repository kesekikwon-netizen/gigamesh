/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * This file is part of DongArch3D.
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_VECTOR_TRACER_H
#define DONGARCH_VECTOR_TRACER_H

#include "../SilhouetteDetector.h"
#include <GigaMesh/mesh/vector3d.h>
#include <QPointF>
#include <QMatrix4x4>
#include <vector>
#include <span>
#include <unordered_set>
#include <unordered_map>

//! \file VectorTracer.h
//! \brief Phase 3: Vector Tracing for Outline Edges
//!
//! DongArch3D v4.0 Phase 3 구현
//! - Silhouette Edge → Vector Polylines 변환
//! - Contour Following 알고리즘
//! - Douglas-Peucker 단순화 지원
//! - 3D → 2D 투영

namespace DongArch {
namespace Outline {
namespace Algorithms {

//! \struct TracingParams
//! \brief Vector Tracing 파라미터
struct TracingParams {
    double simplificationTolerance;  //!< Douglas-Peucker 단순화 허용 오차 (mm)
    bool applySimplifcation;         //!< 단순화 적용 여부
    int minPolylineLength;           //!< 최소 폴리라인 길이 (점 개수)

    TracingParams()
        : simplificationTolerance(0.1)
        , applySimplifcation(true)
        , minPolylineLength(2)
    {}
};

//! \struct TracingResult
//! \brief Vector Tracing 결과
struct TracingResult {
    std::vector<std::vector<QPointF>> polylines2D;  //!< 2D 폴리라인 목록
    std::vector<std::vector<Vector3D>> polylines3D; //!< 3D 폴리라인 목록

    size_t totalEdges;                              //!< 원본 엣지 개수
    size_t polylineCount;                           //!< 추출된 폴리라인 개수
    size_t totalPoints;                             //!< 총 점 개수

    double processingTime;                          //!< 처리 시간 (ms)
    double simplificationRatio;                     //!< 단순화 비율 (%)
};

//! \class VectorTracer
//! \brief Silhouette Edge를 Vector Polylines로 변환
//!
//! 알고리즘:
//! 1. Edge Graph 구성 (인접 관계)
//! 2. Contour Following (DFS 방식)
//! 3. Douglas-Peucker 단순화 (옵션)
//! 4. 3D → 2D 투영
//!
//! 사용법:
//! ```cpp
//! VectorTracer tracer;
//! TracingParams params;
//! params.simplificationTolerance = 0.1;  // 0.1mm
//!
//! TracingResult result;
//! tracer.traceEdges(silhouetteResult.edges, params, result);
//! ```
class VectorTracer {
public:
    //! 생성자
    VectorTracer() = default;

    //! 소멸자
    ~VectorTracer() = default;

    //! Edges → Polylines 변환 (3D)
    //! \param edges 실루엣 엣지 목록
    //! \param params Tracing 파라미터
    //! \param result 결과 (출력)
    //! \return 성공 여부
    bool traceEdges(std::span<const EdgeRef> edges,
                    const TracingParams& params,
                    TracingResult& result);

    //! Edges → Polylines 변환 with 3D → 2D 투영
    //! \param edges 실루엣 엣지 목록
    //! \param params Tracing 파라미터
    //! \param viewMatrix 뷰 행렬 (3D → 2D 투영)
    //! \param result 결과 (출력)
    //! \return 성공 여부
    bool traceEdgesWithProjection(std::span<const EdgeRef> edges,
                                   const TracingParams& params,
                                   const QMatrix4x4& viewMatrix,
                                   TracingResult& result);

    //! 3D 폴리라인 → 2D 투영
    //! \param polylines3D 3D 폴리라인 목록
    //! \param viewMatrix 뷰 행렬
    //! \return 2D 폴리라인 목록
    static std::vector<std::vector<QPointF>> project3DTo2D(
        std::span<const std::vector<Vector3D>> polylines3D,
        const QMatrix4x4& viewMatrix);

    //! ViewDirection → View Matrix 변환
    //! \param viewDir 뷰 방향
    //! \return 뷰 행렬 (정사영)
    static QMatrix4x4 createOrthographicViewMatrix(ViewDirection viewDir);

private:
    //! Edge Graph 구성 (Vertex → connected Edges)
    //! \param edges 엣지 목록
    //! \return Edge Graph
    std::unordered_map<Vertex*, std::vector<EdgeRef>> buildEdgeGraph(
        std::span<const EdgeRef> edges);

    //! Contour Following (DFS)
    //! \param startEdge 시작 엣지
    //! \param edgeGraph 엣지 그래프
    //! \param visited 방문 여부
    //! \return 폴리라인 (3D)
    std::vector<Vector3D> followContour(
        const EdgeRef& startEdge,
        const std::unordered_map<Vertex*, std::vector<EdgeRef>>& edgeGraph,
        std::unordered_set<const Face*>& visited);

    //! Douglas-Peucker 단순화
    //! \param points 점 목록
    //! \param tolerance 허용 오차
    //! \return 단순화된 점 목록
    std::vector<Vector3D> simplifyPolyline(
        std::span<const Vector3D> points,
        double tolerance);

    //! Douglas-Peucker 재귀 함수
    //! \param points 점 목록
    //! \param startIdx 시작 인덱스
    //! \param endIdx 끝 인덱스
    //! \param tolerance 허용 오차
    //! \param result 결과 (출력)
    void douglasPeuckerRecursive(
        std::span<const Vector3D> points,
        size_t startIdx,
        size_t endIdx,
        double tolerance,
        std::vector<bool>& keep);

    //! 점 → 선분 거리 계산
    //! \param point 점
    //! \param lineStart 선분 시작점
    //! \param lineEnd 선분 끝점
    //! \return 거리
    static double pointToLineDistance(
        const Vector3D& point,
        const Vector3D& lineStart,
        const Vector3D& lineEnd);
};

} // namespace Algorithms
} // namespace Outline
} // namespace DongArch

#endif // DONGARCH_VECTOR_TRACER_H
