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

#ifndef DONGARCH_ALIGN_MANAGER_H
#define DONGARCH_ALIGN_MANAGER_H

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/matrix4d.h>
#include "../common/DongArchTypes.h"
#include "../common/DongArchMath.h"
#include <memory>
#include <ranges>

//! \file DongArchAlignManager.h
//! \brief Phase 1: Mesh Alignment (메시 정렬) 관리자
//!
//! DongArch3D v4.0 Phase 1 구현
//! - GigaMesh Mesh 회전 기능 래핑
//! - C++20 std::ranges 기반 정점 처리
//! - Ground Plane 정렬
//! - 6방향 표준 뷰 정렬

namespace DongArch {
namespace Align {

//! \class AlignManager
//! \brief 메시 정렬 관리자
//!
//! GigaMesh의 applyTransformationToWholeMesh를 활용하여
//! C++20 기반 정렬 기능 제공
class AlignManager {
public:
    //! 생성자
    //! \param mesh GigaMesh Mesh 객체 (non-owning pointer)
    explicit AlignManager(Mesh* mesh);

    //! 소멸자
    ~AlignManager() = default;

    // 복사/이동 금지 (Mesh 포인터 관리)
    AlignManager(const AlignManager&) = delete;
    AlignManager& operator=(const AlignManager&) = delete;
    AlignManager(AlignManager&&) = delete;
    AlignManager& operator=(AlignManager&&) = delete;

    // ============================================================================
    // Rotation: 회전 변환
    // ============================================================================

    //! \brief X축 회전
    //! \param degrees 회전 각도 (도)
    //! \param resetNormals 법선 벡터 재계산 여부
    //! \return 성공 여부
    bool rotateX(double degrees, bool resetNormals = true);

    //! \brief Y축 회전
    //! \param degrees 회전 각도 (도)
    //! \param resetNormals 법선 벡터 재계산 여부
    //! \return 성공 여부
    bool rotateY(double degrees, bool resetNormals = true);

    //! \brief Z축 회전
    //! \param degrees 회전 각도 (도)
    //! \param resetNormals 법선 벡터 재계산 여부
    //! \return 성공 여부
    bool rotateZ(double degrees, bool resetNormals = true);

    //! \brief 임의의 축에 대한 회전
    //! \param axis 회전축 (단위 벡터)
    //! \param degrees 회전 각도 (도)
    //! \param resetNormals 법선 벡터 재계산 여부
    //! \return 성공 여부
    bool rotateAroundAxis(const Math::Vec3& axis, double degrees, bool resetNormals = true);

    //! \brief Matrix4D를 사용한 일반 변환
    //! \param matrix 4x4 변환 행렬
    //! \param resetNormals 법선 벡터 재계산 여부
    //! \return 성공 여부
    bool applyTransformation(const Matrix4D& matrix, bool resetNormals = true);

    // ============================================================================
    // Alignment: 정렬
    // ============================================================================

    //! \brief Ground Plane 정렬 (Z=0)
    //!
    //! 메시의 바닥을 Z=0 평면에 정렬
    //! 1. 메시의 최소 Z값 찾기
    //! 2. Z축 방향으로 평행이동
    //!
    //! \return 성공 여부
    bool alignToGroundPlane();

    //! \brief 메시 중심을 원점으로 이동
    //! \return 성공 여부
    bool centerMesh();

    //! \brief 표준 뷰 정렬
    //!
    //! 6방향 표준 뷰 중 하나로 정렬
    //! (Top, Bottom, Front, Back, Left, Right)
    enum class StandardView {
        TOP,      //!< 상단 뷰 (Z축 상향)
        BOTTOM,   //!< 하단 뷰 (Z축 하향)
        FRONT,    //!< 전면 뷰 (Y축 전방)
        BACK,     //!< 후면 뷰 (Y축 후방)
        LEFT,     //!< 좌측 뷰 (X축 좌측)
        RIGHT     //!< 우측 뷰 (X축 우측)
    };

    //! \brief 표준 뷰로 정렬
    //! \param view 표준 뷰
    //! \return 성공 여부
    bool alignToStandardView(StandardView view);

    // ============================================================================
    // C++20 std::ranges: 정점 데이터 접근
    // ============================================================================

    //! \brief 모든 정점의 좌표 가져오기 (std::ranges::view)
    //!
    //! C++20 std::ranges를 사용한 효율적인 정점 순회
    //! \code
    //! auto vertices = manager.getVertexPositions();
    //! for (const auto& v : vertices) {
    //!     std::cout << v.x << ", " << v.y << ", " << v.z << std::endl;
    //! }
    //! \endcode
    //!
    //! \return 정점 좌표 range view
    auto getVertexPositions() const;

    //! \brief 정점 개수
    //! \return 정점 개수
    size_t getVertexCount() const;

    //! \brief 메시 바운딩 박스 가져오기
    //! \param[out] min 최소 좌표
    //! \param[out] max 최대 좌표
    //! \return 성공 여부
    bool getBoundingBox(Math::Vec3& min, Math::Vec3& max) const;

    //! \brief 메시 중심 좌표
    //! \return 중심 좌표
    Math::Vec3 getMeshCenter() const;

    // ============================================================================
    // Utility
    // ============================================================================

    //! \brief Mesh 객체 유효성 확인
    //! \return 유효하면 true
    bool isValid() const { return mMesh != nullptr && mMesh->getVertexNr() > 0; }

    //! \brief 현재 Mesh 포인터
    //! \return Mesh 포인터
    Mesh* getMesh() const { return mMesh; }

private:
    Mesh* mMesh;  //!< GigaMesh Mesh 객체 (non-owning)

    //! \brief Matrix4D → Math::Mat3 변환 (회전 부분만)
    Math::Mat3 toMat3(const Matrix4D& mat4) const;

    //! \brief Math::Mat3 → Matrix4D 변환 (회전 행렬)
    Matrix4D toMatrix4D(const Math::Mat3& mat3) const;
};

// ============================================================================
// Inline 구현: std::ranges getVertexPositions
// ============================================================================

inline auto AlignManager::getVertexPositions() const {
    // C++20 ranges: 정점 벡터 → 좌표 변환 view
    const std::vector<Vertex*>* vertices = mMesh->getPrimitiveListVertices();
    return (*vertices)
        | std::views::transform([](Vertex* v) -> Math::Vec3 {
              return Math::Vec3(v->getX(), v->getY(), v->getZ());
          });
}

inline size_t AlignManager::getVertexCount() const {
    return mMesh ? mMesh->getVertexNr() : 0;
}

} // namespace Align
} // namespace DongArch

#endif // DONGARCH_ALIGN_MANAGER_H
