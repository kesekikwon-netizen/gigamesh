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

#ifndef DONGARCH_TYPES_H
#define DONGARCH_TYPES_H

#include <concepts>
#include <ranges>
#include <span>
#include <vector>
#include <cstdint>
#include <cmath>

//! \file DongArchTypes.h
//! \brief C++20 타입 시스템 및 Concepts 정의
//!
//! DongArch3D v4.0의 핵심 타입 시스템
//! - C++20 Concepts 기반 타입 제약
//! - Designated Initializers 구조체
//! - std::span 기반 안전한 배열 전달

namespace DongArch {

// ============================================================================
// C++20 Concepts: 타입 제약
// ============================================================================

//! \concept VertexLike
//! \brief 3D 정점 타입 제약
//!
//! x, y, z 좌표를 가진 모든 타입에 적용 가능
//! 예: Vector3D, Vertex, glm::vec3
template<typename T>
concept VertexLike = requires(T v) {
    { v.x } -> std::convertible_to<double>;
    { v.y } -> std::convertible_to<double>;
    { v.z } -> std::convertible_to<double>;
};

//! \concept MeshContainer
//! \brief 메시 데이터 컨테이너 제약
//!
//! std::ranges::range이며 VertexLike 요소를 가진 컨테이너
//! 예: std::vector<Vector3D>, std::span<Vertex>
template<typename R>
concept MeshContainer = std::ranges::range<R> &&
                        VertexLike<std::ranges::range_value_t<R>>;

//! \concept FloatingPoint
//! \brief 부동소수점 타입 제약
template<typename T>
concept FloatingPoint = std::floating_point<T>;

// ============================================================================
// Designated Initializers 구조체: 설정 파라미터
// ============================================================================

//! \struct CutlineParams
//! \brief Cutline (단면 라인) 추출 파라미터
//!
//! Phase 2: Cutline에서 사용
//! Designated Initializers로 초기화:
//! \code
//! CutlineParams params {
//!     .planeHeight = 10.0f,
//!     .smoothingLevel = 3,
//!     .useSpline = true
//! };
//! \endcode
struct CutlineParams {
    float planeHeight = 0.0f;           //!< 평면 높이 (Z축 기준, mm)
    int smoothingLevel = 2;             //!< Douglas-Peucker 간략화 레벨 (0-5)
    bool useSpline = true;              //!< Catmull-Rom Spline 사용 여부
    int splineSegments = 20;            //!< Spline 세그먼트 수 (10-40)
    float tolerance = 0.5f;             //!< Douglas-Peucker tolerance (mm)
    bool showPreview = true;            //!< Fragment Shader 프리뷰 활성화

    // 평면 방향
    enum class Direction {
        TOP,      //!< 상단 (Z축)
        FRONT,    //!< 전면 (Y축)
        RIGHT,    //!< 우측 (X축)
        CUSTOM    //!< 사용자 정의
    };
    Direction direction = Direction::TOP;
};

//! \struct OutlineParams
//! \brief Outline (외곽선) 추출 파라미터
//!
//! Phase 3: Outline에서 사용
struct OutlineParams {
    bool useGeometryShader = true;      //!< Geometry Shader Silhouette 사용
    bool useSobelFilter = false;        //!< Sobel Edge Detection 후처리
    float edgeThreshold = 0.1f;         //!< Silhouette 검출 임계값
    int qualityLevel = 3;               //!< Quadric Error Metric 품질 (1-5)
    bool dualPassRendering = false;     //!< Dual-pass Rendering 활성화

    // 외곽선 방향
    enum class ViewDirection {
        TOP, BOTTOM,
        FRONT, BACK,
        LEFT, RIGHT,
        CURRENT                         //!< 현재 카메라 방향
    };
    ViewDirection viewDir = ViewDirection::CURRENT;
};

//! \struct ClipParams
//! \brief Clip (3D 절단) 파라미터
//!
//! Phase 4: Clip에서 사용
struct ClipParams {
    float planeDistance = 0.0f;         //!< 절단 평면 거리 (mm)
    bool keepFront = true;              //!< 앞면 유지 여부
    bool keepBack = false;              //!< 뒷면 유지 여부
    bool repairSelfIntersection = true; //!< Self-Intersection 자동 수리
    bool showClipPreview = true;        //!< Fragment Shader 프리뷰

    // 절단 평면 방향
    enum class ClipDirection {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        CUSTOM
    };
    ClipDirection clipDir = ClipDirection::Z_AXIS;
};

//! \struct VisParams
//! \brief Visualization (시각화) 파라미터
//!
//! Phase 5: Vis에서 사용
struct VisParams {
    bool useHeatMethodGPU = true;       //!< Heat Method GPU 가속
    bool useXRayRendering = false;      //!< X-Ray Depth Peeling
    int depthPeelingLayers = 3;         //!< Depth Peeling 레이어 수 (1-8)
    float xrayOpacity = 0.3f;           //!< X-Ray 투명도 (0.0-1.0)

    // 곡률 시각화 타입
    enum class CurvatureType {
        GAUSSIAN,
        MEAN,
        PRINCIPAL_MAX,
        PRINCIPAL_MIN,
        GEODESIC_DISTANCE                //!< D-Tak
    };
    CurvatureType curvType = CurvatureType::GEODESIC_DISTANCE;

    // 컬러맵
    enum class ColorMap {
        JET,
        VIRIDIS,
        PLASMA,
        GRAYSCALE
    };
    ColorMap colorMap = ColorMap::JET;
};

// ============================================================================
// 기본 타입 정의 (Basic Types)
// ============================================================================

//! \struct Point2D
//! \brief 2D 좌표 타입
//!
//! Douglas-Peucker, Catmull-Rom Spline 등에서 사용
//! GigaMesh에는 Point2D가 없으므로 직접 정의
struct Point2D {
    double x = 0.0;
    double y = 0.0;

    Point2D() = default;
    Point2D(double x_, double y_) : x(x_), y(y_) {}

    // 연산자
    Point2D operator+(const Point2D& other) const {
        return Point2D{x + other.x, y + other.y};
    }

    Point2D operator-(const Point2D& other) const {
        return Point2D{x - other.x, y - other.y};
    }

    Point2D operator*(double scalar) const {
        return Point2D{x * scalar, y * scalar};
    }

    Point2D operator/(double scalar) const {
        return Point2D{x / scalar, y / scalar};
    }

    // 거리 계산
    double distanceTo(const Point2D& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

// ============================================================================
// 공통 타입 별칭 (Type Aliases)
// ============================================================================

//! \typedef VertexSpan
//! \brief 정점 데이터를 위한 std::span (읽기 전용)
//!
//! 포인터 + 크기 대신 std::span 사용 (C++20)
//! \code
//! void processMesh(VertexSpan vertices) {
//!     for (const auto& v : vertices) {
//!         // 안전한 순회
//!     }
//! }
//! \endcode
template<VertexLike T>
using VertexSpan = std::span<const T>;

//! \typedef VertexVector
//! \brief 정점 데이터를 위한 std::vector
template<VertexLike T>
using VertexVector = std::vector<T>;

//! \typedef IndexSpan
//! \brief 인덱스 데이터를 위한 std::span (읽기 전용)
using IndexSpan = std::span<const uint32_t>;

//! \typedef IndexVector
//! \brief 인덱스 데이터를 위한 std::vector
using IndexVector = std::vector<uint32_t>;

// ============================================================================
// 결과 타입 (Result Types)
// ============================================================================

//! \struct CutlineResult
//! \brief Cutline 추출 결과
struct CutlineResult {
    std::vector<float> points;          //!< 교차점 좌표 (x, y, z 순서)
    std::vector<float> simplified;      //!< Douglas-Peucker 간략화 결과
    std::vector<float> spline;          //!< Catmull-Rom Spline 결과
    size_t pointCount = 0;              //!< 원본 교차점 개수
    size_t simplifiedCount = 0;         //!< 간략화 후 점 개수
    size_t splineSegments = 0;          //!< Spline 세그먼트 수
    bool success = false;               //!< 성공 여부
};

//! \struct OutlineResult
//! \brief Outline 추출 결과
struct OutlineResult {
    std::vector<float> silhouetteEdges; //!< Silhouette edge 좌표
    std::vector<uint32_t> edgeIndices;  //!< Edge 인덱스
    size_t edgeCount = 0;               //!< Edge 개수
    float qualityScore = 0.0f;          //!< Quadric 품질 점수
    bool success = false;               //!< 성공 여부
};

//! \struct ClipResult
//! \brief Clip 절단 결과
struct ClipResult {
    size_t frontFaceCount = 0;          //!< 앞면 Face 개수
    size_t backFaceCount = 0;           //!< 뒷면 Face 개수
    size_t repairedIntersections = 0;   //!< 수리된 Self-Intersection 개수
    bool success = false;               //!< 성공 여부
};

} // namespace DongArch

#endif // DONGARCH_TYPES_H
