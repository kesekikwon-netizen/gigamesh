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

#ifndef DONGARCH_SELF_INTERSECTION_REPAIRER_H
#define DONGARCH_SELF_INTERSECTION_REPAIRER_H

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/face.h>
#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/vector3d.h>
#include "dongarch/common/DongArchTypes.h"
#include "dongarch/common/DongArchMath.h"
#include <vector>
#include <span>
#include <set>
#include <memory>

//! \file SelfIntersectionRepairer.h
//! \brief Phase 4: Self-Intersection Repair (2025 Paper)
//!
//! DongArch3D v4.0 Phase 4 고급 기능
//! - 2025년 최신 논문: "Instant Self-Intersection Repair for 3D Meshes"
//! - 저자: W. Jang, Yucheol Jung, Gyeongmin Lee, Seungyong Lee
//! - DOI: 10.1145/3731427
//!
//! 기능:
//! - Mesh Split 후 자동 Self-Intersection 감지
//! - 교차하는 Face 분할
//! - Topological Correctness 보장
//! - C++20 std::span 사용

namespace DongArch {
namespace Clip {
namespace Algorithms {

//! \struct Triangle
//! \brief 삼각형 (3개 정점)
struct Triangle {
    DongArch::Math::Vec3 v0;  //!< 정점 0
    DongArch::Math::Vec3 v1;  //!< 정점 1
    DongArch::Math::Vec3 v2;  //!< 정점 2

    //! 기본 생성자
    Triangle() = default;

    //! 생성자
    Triangle(const DongArch::Math::Vec3& _v0,
             const DongArch::Math::Vec3& _v1,
             const DongArch::Math::Vec3& _v2)
        : v0(_v0), v1(_v1), v2(_v2) {}

    //! Face로부터 Triangle 생성
    static Triangle fromFace(const Face* face);

    //! 법선 벡터 계산
    DongArch::Math::Vec3 normal() const;

    //! 면적 계산
    double area() const;

    //! AABB (Axis-Aligned Bounding Box) 계산
    void computeAABB(DongArch::Math::Vec3& min, DongArch::Math::Vec3& max) const;
};

//! \struct IntersectionPair
//! \brief 교차하는 삼각형 쌍
struct IntersectionPair {
    size_t faceIndex1;  //!< Face 1 인덱스
    size_t faceIndex2;  //!< Face 2 인덱스
    Face* face1;        //!< Face 1 포인터
    Face* face2;        //!< Face 2 포인터
    Triangle triangle1; //!< Triangle 1
    Triangle triangle2; //!< Triangle 2

    //! 교차선 (있을 경우)
    std::vector<DongArch::Math::Vec3> intersectionLine;
};

//! \struct RepairResult
//! \brief Self-Intersection 수정 결과
struct RepairResult {
    bool success;                           //!< 수정 성공 여부
    size_t totalFaces;                      //!< 전체 면 개수
    size_t intersectionCount;               //!< 교차 쌍 개수
    size_t repairedCount;                   //!< 수정된 교차 개수
    size_t newFacesCreated;                 //!< 새로 생성된 면 개수
    size_t facesRemoved;                    //!< 제거된 면 개수

    double detectionTime;                   //!< 감지 시간 (ms)
    double repairTime;                      //!< 수정 시간 (ms)
    double totalTime;                       //!< 전체 시간 (ms)

    std::vector<IntersectionPair> intersections;  //!< 감지된 교차 목록
};

//! \class SelfIntersectionRepairer
//! \brief Self-Intersection 자동 감지 및 수정
//!
//! 2025년 논문 "Instant Self-Intersection Repair for 3D Meshes" 기반
//! - W. Jang, Yucheol Jung, Gyeongmin Lee, Seungyong Lee
//! - DOI: 10.1145/3731427
//!
//! 알고리즘:
//! 1. Triangle-Triangle Intersection 감지 (Möller 1997)
//! 2. 교차하는 삼각형 분할
//! 3. Topological Correctness 유지
//! 4. Octree 기반 공간 분할 (효율성)
//!
//! 성능:
//! - O(n log n) 평균 (Octree 사용)
//! - O(n²) 최악 (brute force)
class SelfIntersectionRepairer {
public:
    //! 생성자
    //! \param mesh 대상 메시 (non-owning)
    explicit SelfIntersectionRepairer(Mesh* mesh);

    //! 소멸자
    ~SelfIntersectionRepairer() = default;

    // Copy/Move 금지 (non-owning pointer)
    SelfIntersectionRepairer(const SelfIntersectionRepairer&) = delete;
    SelfIntersectionRepairer& operator=(const SelfIntersectionRepairer&) = delete;
    SelfIntersectionRepairer(SelfIntersectionRepairer&&) = delete;
    SelfIntersectionRepairer& operator=(SelfIntersectionRepairer&&) = delete;

    //! 메시 유효성 확인
    //! \return 메시가 유효하면 true
    bool isValid() const { return mMesh != nullptr; }

    //! Self-Intersection 감지
    //! \param result 결과 (출력)
    //! \return 교차가 발견되면 true
    bool detectIntersections(RepairResult& result);

    //! Self-Intersection 감지 (std::span 사용)
    //! \param triangles 삼각형 배열
    //! \param result 결과 (출력)
    //! \return 교차가 발견되면 true
    bool detectIntersections(std::span<const Triangle> triangles, RepairResult& result);

    //! Self-Intersection 수정
    //! \param result 수정 결과 (출력)
    //! \return 수정 성공 여부
    bool repairIntersections(RepairResult& result);

    //! Self-Intersection 감지 및 수정 (통합)
    //! \param result 결과 (출력)
    //! \return 수정 성공 여부
    bool detectAndRepair(RepairResult& result);

    //! Triangle-Triangle Intersection 테스트
    //! \param t1 삼각형 1
    //! \param t2 삼각형 2
    //! \return 교차하면 true
    static bool triangleIntersectsTriangle(const Triangle& t1, const Triangle& t2);

    //! Triangle-Triangle Intersection 테스트 (교차선 계산)
    //! \param t1 삼각형 1
    //! \param t2 삼각형 2
    //! \param intersectionLine 교차선 (출력)
    //! \return 교차하면 true
    static bool triangleIntersectsTriangle(const Triangle& t1,
                                            const Triangle& t2,
                                            std::vector<DongArch::Math::Vec3>& intersectionLine);

    //! AABB (Axis-Aligned Bounding Box) 교차 테스트
    //! \param min1 AABB 1 최소 좌표
    //! \param max1 AABB 1 최대 좌표
    //! \param min2 AABB 2 최소 좌표
    //! \param max2 AABB 2 최대 좌표
    //! \return 교차하면 true
    static bool aabbIntersects(const DongArch::Math::Vec3& min1,
                               const DongArch::Math::Vec3& max1,
                               const DongArch::Math::Vec3& min2,
                               const DongArch::Math::Vec3& max2);

    //! Octree 사용 여부 설정
    //! \param use Octree 사용
    void setUseOctree(bool use) { mUseOctree = use; }

    //! Octree 사용 여부 조회
    //! \return Octree 사용 여부
    bool useOctree() const { return mUseOctree; }

    //! Octree 깊이 설정
    //! \param depth Octree 최대 깊이
    void setOctreeDepth(int depth) { mOctreeDepth = depth; }

    //! Octree 깊이 조회
    //! \return Octree 최대 깊이
    int octreeDepth() const { return mOctreeDepth; }

private:
    //! Brute-force 교차 감지
    //! \param triangles 삼각형 배열
    //! \param result 결과 (출력)
    //! \return 교차 개수
    size_t detectBruteForce(std::span<const Triangle> triangles, RepairResult& result);

    //! Octree 기반 교차 감지
    //! \param triangles 삼각형 배열
    //! \param result 결과 (출력)
    //! \return 교차 개수
    size_t detectUsingOctree(std::span<const Triangle> triangles, RepairResult& result);

    //! 교차하는 삼각형 쌍 수정
    //! \param pair 교차 쌍
    //! \return 수정 성공 여부
    bool repairIntersectionPair(IntersectionPair& pair);

    //! 삼각형 분할 (교차선 기준)
    //! \param triangle 원본 삼각형
    //! \param intersectionLine 교차선
    //! \param newTriangles 분할된 삼각형들 (출력)
    //! \return 분할 성공 여부
    bool splitTriangle(const Triangle& triangle,
                       const std::vector<DongArch::Math::Vec3>& intersectionLine,
                       std::vector<Triangle>& newTriangles);

    //! Face를 Triangle로 변환
    //! \param faces Face 배열
    //! \return Triangle 배열
    std::vector<Triangle> facesToTriangles(std::span<Face* const> faces) const;

    //! Triangle을 Face로 변환 (메시에 추가)
    //! \param triangle Triangle
    //! \return 생성된 Face 포인터
    Face* triangleToFace(const Triangle& triangle);

    Mesh* mMesh;           //!< 대상 메시 (non-owning)
    bool mUseOctree;       //!< Octree 사용 여부
    int mOctreeDepth;      //!< Octree 최대 깊이
};

} // namespace Algorithms
} // namespace Clip
} // namespace DongArch

#endif // DONGARCH_SELF_INTERSECTION_REPAIRER_H
