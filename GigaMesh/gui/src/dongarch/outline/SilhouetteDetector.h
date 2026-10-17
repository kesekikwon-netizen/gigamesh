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

#ifndef DONGARCH_SILHOUETTE_DETECTOR_H
#define DONGARCH_SILHOUETTE_DETECTOR_H

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/face.h>
#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/vector3d.h>
#include "dongarch/common/DongArchTypes.h"
#include "dongarch/common/DongArchMath.h"
#include <vector>
#include <span>
#include <ranges>

//! \file SilhouetteDetector.h
//! \brief Phase 3-A: CPU-based Silhouette Edge Detection
//!
//! DongArch3D v4.0 Phase 3 구현 (ADR-011)
//! - View-Dependent Silhouette 계산
//! - 6방향 정사영 (TOP, FRONT, RIGHT, LEFT, BACK, BOTTOM)
//! - C++20 std::ranges 활용
//! - Qt Concurrent 병렬 처리

namespace DongArch {
namespace Outline {

//! \enum ViewDirection
//! \brief 6방향 정사영 뷰 방향
enum class ViewDirection {
    TOP,     //!< 위에서 본 뷰 (Y축 음의 방향)
    BOTTOM,  //!< 아래에서 본 뷰 (Y축 양의 방향)
    FRONT,   //!< 앞에서 본 뷰 (Z축 음의 방향)
    BACK,    //!< 뒤에서 본 뷰 (Z축 양의 방향)
    RIGHT,   //!< 오른쪽에서 본 뷰 (X축 양의 방향)
    LEFT     //!< 왼쪽에서 본 뷰 (X축 음의 방향)
};

//! \struct EdgeRef
//! \brief Face 기반 엣지 참조
//!
//! GigaMesh는 Edge 클래스를 public API로 노출하지 않으므로
//! Face + eEdgeNames로 엣지를 표현합니다.
struct EdgeRef {
    Face* face;                         //!< 엣지를 소유한 Face
    Face::eEdgeNames edgeIdx;           //!< 엣지 인덱스 (EDGE_AB, EDGE_BC, EDGE_CA)

    EdgeRef() : face(nullptr), edgeIdx(Face::EDGE_NONE) {}
    EdgeRef(Face* f, Face::eEdgeNames idx) : face(f), edgeIdx(idx) {}

    //! 두 정점 조회
    bool getVertices(Vertex*& v1, Vertex*& v2) const;

    //! 인접 Face 조회
    Face* getNeighbourFace() const {
        return face ? face->getNeighbourFace(edgeIdx) : nullptr;
    }

    //! 비교 연산자 (중복 제거용)
    bool operator<(const EdgeRef& other) const {
        if (face != other.face) {
            return face < other.face;
        }
        return edgeIdx < other.edgeIdx;
    }

    bool operator==(const EdgeRef& other) const {
        return face == other.face && edgeIdx == other.edgeIdx;
    }
};

//! \struct SilhouetteResult
//! \brief Silhouette 검출 결과
struct SilhouetteResult {
    std::vector<EdgeRef> edges;         //!< 실루엣 엣지 목록
    ViewDirection viewDirection;        //!< 사용된 뷰 방향
    Vector3D viewVector;                //!< 뷰 벡터 (정규화됨)
    size_t totalEdges;                  //!< 전체 엣지 수
    size_t silhouetteEdges;             //!< 실루엣 엣지 수
    double processingTime;              //!< 처리 시간 (ms)
};

//! \class SilhouetteDetector
//! \brief CPU 기반 View-Dependent Silhouette Edge 검출기
//!
//! GigaMesh Mesh의 Silhouette Edge를 검출합니다.
//! - Dot product 기반: (n1 · view) * (n2 · view) < 0
//! - std::ranges::filter 사용
//! - Qt Concurrent 병렬 처리 (100개 이상 edge)
class SilhouetteDetector {
public:
    //! 생성자
    //! \param mesh 대상 메시 (non-owning)
    explicit SilhouetteDetector(Mesh* mesh);

    //! 소멸자
    ~SilhouetteDetector() = default;

    // Copy/Move 금지 (non-owning pointer)
    SilhouetteDetector(const SilhouetteDetector&) = delete;
    SilhouetteDetector& operator=(const SilhouetteDetector&) = delete;
    SilhouetteDetector(SilhouetteDetector&&) = delete;
    SilhouetteDetector& operator=(SilhouetteDetector&&) = delete;

    //! 메시 유효성 확인
    //! \return 메시가 유효하면 true
    bool isValid() const { return mMesh != nullptr; }

    //! Silhouette Edge 검출 (단일 뷰 방향)
    //! \param viewDir 뷰 방향 (6방향 중 하나)
    //! \param result 검출 결과 (출력)
    //! \return 성공 여부
    bool detectSilhouette(ViewDirection viewDir, SilhouetteResult& result);

    //! Silhouette Edge 검출 (커스텀 뷰 벡터)
    //! \param viewVector 뷰 벡터 (정규화되지 않아도 됨)
    //! \param result 검출 결과 (출력)
    //! \return 성공 여부
    bool detectSilhouette(const Vector3D& viewVector, SilhouetteResult& result);

    //! 6방향 모두 Silhouette 검출
    //! \param results 각 방향별 결과 (6개)
    //! \return 성공한 방향 개수
    size_t detectAllDirections(std::vector<SilhouetteResult>& results);

    //! 단일 Edge가 Silhouette인지 판정
    //! \param edgeRef 판정할 엣지 (Face + eEdgeNames)
    //! \param viewVector 뷰 벡터 (정규화됨)
    //! \return Silhouette이면 true
    static bool isSilhouetteEdge(const EdgeRef& edgeRef, const Vector3D& viewVector);

    //! ViewDirection → 뷰 벡터 변환
    //! \param viewDir 뷰 방향
    //! \return 정규화된 뷰 벡터
    static Vector3D getViewVector(ViewDirection viewDir);

    //! ViewDirection → 문자열 변환
    //! \param viewDir 뷰 방향
    //! \return 방향 이름 (한글)
    static const char* viewDirectionName(ViewDirection viewDir);

    //! 병렬 처리 임계값 설정
    //! \param threshold 엣지 개수 임계값 (기본: 100)
    void setParallelThreshold(size_t threshold) { mParallelThreshold = threshold; }

    //! 병렬 처리 임계값 조회
    //! \return 현재 임계값
    size_t getParallelThreshold() const { return mParallelThreshold; }

private:
    //! 모든 엣지 조회
    //! \return 엣지 참조 벡터
    std::vector<EdgeRef> getAllEdges() const;

    //! Silhouette 검출 (내부 구현)
    //! \param viewVector 정규화된 뷰 벡터
    //! \param result 검출 결과 (출력)
    //! \return 성공 여부
    bool detectSilhouetteInternal(const Vector3D& viewVector, SilhouetteResult& result);

    //! Silhouette 검출 (병렬 처리 버전)
    //! \param edges 전체 엣지 목록
    //! \param viewVector 정규화된 뷰 벡터
    //! \return Silhouette 엣지 목록
    std::vector<EdgeRef> detectParallel(const std::vector<EdgeRef>& edges,
                                        const Vector3D& viewVector);

    //! Silhouette 검출 (순차 처리 버전 - std::ranges)
    //! \param edges 전체 엣지 목록
    //! \param viewVector 정규화된 뷰 벡터
    //! \return Silhouette 엣지 목록
    std::vector<EdgeRef> detectSequential(const std::vector<EdgeRef>& edges,
                                          const Vector3D& viewVector);

    Mesh* mMesh;                    //!< 대상 메시 (non-owning)
    size_t mParallelThreshold;      //!< 병렬 처리 임계값 (기본: 100)
};

} // namespace Outline
} // namespace DongArch

#endif // DONGARCH_SILHOUETTE_DETECTOR_H
