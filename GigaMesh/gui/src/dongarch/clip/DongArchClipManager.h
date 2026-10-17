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

#ifndef DONGARCH_CLIP_MANAGER_H
#define DONGARCH_CLIP_MANAGER_H

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/face.h>
#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/vector3d.h>
#include <GigaMesh/mesh/plane.h>
#include "dongarch/common/DongArchTypes.h"
#include <functional>
#include <vector>
#include <memory>

//! \file DongArchClipManager.h
//! \brief Phase 4: Clip (3D 메시 절단) (v4.0)
//!
//! DongArch3D v4.0 Phase 4 구현
//! - GigaMesh splitMesh 활용 (70% 재사용)
//! - C++20 std::function 사용
//! - 3개 함수 파라미터: intersectFun, distFun, getIntersectionVectorFun
//!
//! 기반:
//! - GigaMesh mesh.cpp:4149 splitMesh() 함수
//! - GigaMesh mesh.cpp:4015 splitByPlane() 함수
//! - C++20 Lambda, std::function

namespace DongArch {
namespace Clip {

//! \enum ClipMode
//! \brief 절단 모드
enum class ClipMode {
    KEEP_FRONT,    //!< 평면 앞쪽(양수 쪽) 유지
    KEEP_BACK,     //!< 평면 뒤쪽(음수 쪽) 유지
    KEEP_BOTH,     //!< 양쪽 모두 유지 (메시 분할)
    SPLIT_ONLY     //!< 절단만 수행 (삭제하지 않음)
};

//! \struct ClipParams
//! \brief 절단 파라미터
struct ClipParams {
    Vector3D planeHNF;                  //!< 절단 평면 (Hesse Normal Form)
    ClipMode mode = ClipMode::KEEP_BOTH; //!< 절단 모드
    bool duplicateVertices = false;     //!< 정점 중복 생성 (간격 유지)
    bool noRedraw = true;               //!< 재그리기 생략 (성능 향상)
    double epsilon = 1e-10;             //!< 부동소수점 오차 허용치
    Vector3D uniformOffset;             //!< 정점 중복 시 오프셋 벡터
};

//! \struct ClipResult
//! \brief 절단 결과
struct ClipResult {
    bool success;                       //!< 성공 여부
    size_t originalFaceCount;           //!< 원본 면 개수
    size_t frontFaceCount;              //!< 앞쪽 면 개수
    size_t backFaceCount;               //!< 뒤쪽 면 개수
    size_t newVertexCount;              //!< 새로 생성된 정점 개수
    double processingTime;              //!< 처리 시간 (ms)

    // 절단선 (교차선)
    std::vector<Vector3D> intersectionPoints;  //!< 교차점 목록
    size_t intersectionEdgeCount;              //!< 교차 엣지 개수
};

//! \class DongArchClipManager
//! \brief 3D 메시 절단 관리자
//!
//! GigaMesh splitMesh 기반:
//! - mesh.cpp:4149의 splitMesh() 함수 활용 (70%)
//! - 3개 std::function 파라미터 사용 (C++20)
//! - Triangle-Plane intersection 알고리즘
//!
//! 기능:
//! - 평면에 의한 메시 절단
//! - 앞쪽/뒤쪽 선택적 유지
//! - 절단선 추출
//! - 정점 중복 옵션
class DongArchClipManager {
public:
    //! 생성자
    //! \param mesh 대상 메시 (non-owning)
    explicit DongArchClipManager(Mesh* mesh);

    //! 소멸자
    ~DongArchClipManager() = default;

    // Copy/Move 금지 (non-owning pointer)
    DongArchClipManager(const DongArchClipManager&) = delete;
    DongArchClipManager& operator=(const DongArchClipManager&) = delete;
    DongArchClipManager(DongArchClipManager&&) = delete;
    DongArchClipManager& operator=(DongArchClipManager&&) = delete;

    //! 메시 유효성 확인
    //! \return 메시가 유효하면 true
    bool isValid() const { return mMesh != nullptr; }

    //! 메시 바운딩 박스 가져오기
    //! \param minZ 최소 Z 좌표 (출력)
    //! \param maxZ 최대 Z 좌표 (출력)
    //! \return 성공 여부
    bool getMeshBounds(double& minZ, double& maxZ) const;

    //! 정규화된 높이에서 절단 평면 생성
    //! \param normalizedHeight 정규화된 높이 (0.0-1.0)
    //! \return 절단 평면 (Hesse Normal Form)
    Vector3D createHorizontalPlane(double normalizedHeight) const;

    //! 평면으로 메시 절단
    //! \param params 절단 파라미터
    //! \param result 절단 결과 (출력)
    //! \return 성공 여부
    bool clipByPlane(const ClipParams& params, ClipResult& result);

    //! 평면으로 메시 절단 (간단한 버전)
    //! \param planeHNF 절단 평면 (Hesse Normal Form)
    //! \param mode 절단 모드
    //! \return 성공 여부
    bool clipByPlane(const Vector3D& planeHNF, ClipMode mode = ClipMode::KEEP_BOTH);

    //! 절단선 추출 (교차선)
    //! \param planeHNF 절단 평면
    //! \param intersectionPoints 교차점 목록 (출력)
    //! \return 성공 여부
    bool extractIntersectionLine(const Vector3D& planeHNF,
                                  std::vector<Vector3D>& intersectionPoints);

    //! GigaMesh splitMesh 직접 호출
    //! \param intersectFun Triangle-Plane intersection 함수
    //! \param distFun 거리 계산 함수
    //! \param getIntersectionVectorFun 교차점 벡터 계산 함수
    //! \param duplicateVertices 정점 중복 생성
    //! \param noRedraw 재그리기 생략
    //! \param uniformOffset 오프셋 벡터
    //! \return 성공 여부
    bool splitMeshDirect(
        const std::function<bool(Face*)>& intersectFun,
        const std::function<double(VertexOfFace*)>& distFun,
        const std::function<void(VertexOfFace*, VertexOfFace*, Vector3D&)>& getIntersectionVectorFun,
        bool duplicateVertices = false,
        bool noRedraw = true,
        const Vector3D& uniformOffset = Vector3D(0.0, 0.0, 0.0)
    );

    //! 평면과 삼각형 교차 테스트 함수 생성
    //! \param planeHNF 평면 (Hesse Normal Form)
    //! \return std::function<bool(Face*)>
    static std::function<bool(Face*)>
    createIntersectFunction(const Vector3D& planeHNF);

    //! 평면까지의 거리 계산 함수 생성
    //! \param planeHNF 평면 (Hesse Normal Form)
    //! \return std::function<double(VertexOfFace*)>
    static std::function<double(VertexOfFace*)>
    createDistanceFunction(const Vector3D& planeHNF);

    //! 교차점 벡터 계산 함수 생성
    //! \param planeHNF 평면 (Hesse Normal Form)
    //! \return std::function<void(VertexOfFace*, VertexOfFace*, Vector3D&)>
    static std::function<void(VertexOfFace*, VertexOfFace*, Vector3D&)>
    createIntersectionVectorFunction(const Vector3D& planeHNF);

private:
    //! 절단 후 후처리 (앞쪽/뒤쪽 제거)
    //! \param planeHNF 절단 평면
    //! \param mode 절단 모드
    //! \return 제거된 면 개수
    size_t postProcessClip(const Vector3D& planeHNF, ClipMode mode);

    //! 평면 앞쪽 면 제거
    //! \param planeHNF 절단 평면
    //! \return 제거된 면 개수
    size_t removeFrontFaces(const Vector3D& planeHNF);

    //! 평면 뒤쪽 면 제거
    //! \param planeHNF 절단 평면
    //! \return 제거된 면 개수
    size_t removeBackFaces(const Vector3D& planeHNF);

    //! 면이 평면 앞쪽에 있는지 확인
    //! \param face 면
    //! \param planeHNF 평면
    //! \return 앞쪽이면 true
    bool isFaceFront(Face* face, const Vector3D& planeHNF) const;

    //! 면이 평면 뒤쪽에 있는지 확인
    //! \param face 면
    //! \param planeHNF 평면
    //! \return 뒤쪽이면 true
    bool isFaceBack(Face* face, const Vector3D& planeHNF) const;

    Mesh* mMesh;  //!< 대상 메시 (non-owning)
};

//! ClipMode 이름 조회
//! \param mode 절단 모드
//! \return 모드 이름 (한국어)
inline const char* clipModeName(ClipMode mode) {
    switch (mode) {
        case ClipMode::KEEP_FRONT: return "앞쪽 유지";
        case ClipMode::KEEP_BACK:  return "뒤쪽 유지";
        case ClipMode::KEEP_BOTH:  return "양쪽 유지 (분할)";
        case ClipMode::SPLIT_ONLY: return "절단만 수행";
        default:                   return "알 수 없음";
    }
}

} // namespace Clip
} // namespace DongArch

#endif // DONGARCH_CLIP_MANAGER_H
