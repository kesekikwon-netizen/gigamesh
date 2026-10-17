/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "SelfIntersectionRepairer.h"
#include <QElapsedTimer>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

namespace DongArch {
namespace Clip {
namespace Algorithms {

// ============================================================================
// Triangle 구조체 구현
// ============================================================================

Triangle Triangle::fromFace(const Face* face) {
    if (!face) {
        return Triangle();
    }

    Vertex* v0 = face->getVertA();
    Vertex* v1 = face->getVertB();
    Vertex* v2 = face->getVertC();

    if (!v0 || !v1 || !v2) {
        return Triangle();
    }

    return Triangle(
        DongArch::Math::Vec3{v0->getX(), v0->getY(), v0->getZ()},
        DongArch::Math::Vec3{v1->getX(), v1->getY(), v1->getZ()},
        DongArch::Math::Vec3{v2->getX(), v2->getY(), v2->getZ()}
    );
}

DongArch::Math::Vec3 Triangle::normal() const {
    // Cross product: (v1 - v0) × (v2 - v0)
    double edge1_x = v1.x - v0.x;
    double edge1_y = v1.y - v0.y;
    double edge1_z = v1.z - v0.z;

    double edge2_x = v2.x - v0.x;
    double edge2_y = v2.y - v0.y;
    double edge2_z = v2.z - v0.z;

    double nx = edge1_y * edge2_z - edge1_z * edge2_y;
    double ny = edge1_z * edge2_x - edge1_x * edge2_z;
    double nz = edge1_x * edge2_y - edge1_y * edge2_x;

    // Normalize
    double len = std::sqrt(nx*nx + ny*ny + nz*nz);
    if (len > 1e-10) {
        nx /= len;
        ny /= len;
        nz /= len;
    }

    return DongArch::Math::Vec3{nx, ny, nz};
}

double Triangle::area() const {
    // Half of cross product magnitude
    double edge1_x = v1.x - v0.x;
    double edge1_y = v1.y - v0.y;
    double edge1_z = v1.z - v0.z;

    double edge2_x = v2.x - v0.x;
    double edge2_y = v2.y - v0.y;
    double edge2_z = v2.z - v0.z;

    double cx = edge1_y * edge2_z - edge1_z * edge2_y;
    double cy = edge1_z * edge2_x - edge1_x * edge2_z;
    double cz = edge1_x * edge2_y - edge1_y * edge2_x;

    return 0.5 * std::sqrt(cx*cx + cy*cy + cz*cz);
}

void Triangle::computeAABB(DongArch::Math::Vec3& min, DongArch::Math::Vec3& max) const {
    min.x = std::min({v0.x, v1.x, v2.x});
    min.y = std::min({v0.y, v1.y, v2.y});
    min.z = std::min({v0.z, v1.z, v2.z});

    max.x = std::max({v0.x, v1.x, v2.x});
    max.y = std::max({v0.y, v1.y, v2.y});
    max.z = std::max({v0.z, v1.z, v2.z});
}

// ============================================================================
// SelfIntersectionRepairer 구현
// ============================================================================

SelfIntersectionRepairer::SelfIntersectionRepairer(Mesh* mesh)
    : mMesh(mesh)
    , mUseOctree(true)
    , mOctreeDepth(8)
{
}

bool SelfIntersectionRepairer::detectIntersections(RepairResult& result) {
    if (!mMesh) {
        result.success = false;
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    // Face → Triangle 변환
    std::vector<Face*> faces;
    faces.reserve(mMesh->getFaceNr());
    for (size_t i = 0; i < mMesh->getFaceNr(); i++) {
        Face* face = mMesh->getFacePos(i);
        if (face) {
            faces.push_back(face);
        }
    }

    std::vector<Triangle> triangles = facesToTriangles(faces);

    result.totalFaces = triangles.size();

    // 교차 감지
    size_t intersectionCount;
    if (mUseOctree) {
        intersectionCount = detectUsingOctree(triangles, result);
    } else {
        intersectionCount = detectBruteForce(triangles, result);
    }

    result.intersectionCount = intersectionCount;
    result.detectionTime = timer.elapsed();

    std::cout << "[SelfIntersectionRepairer] Intersection detection complete\n";
    std::cout << "  - Total faces: " << result.totalFaces << "\n";
    std::cout << "  - Intersections found: " << intersectionCount << "\n";
    std::cout << "  - Detection time: " << result.detectionTime << " ms\n";

    return intersectionCount > 0;
}

bool SelfIntersectionRepairer::detectIntersections(std::span<const Triangle> triangles, RepairResult& result) {
    QElapsedTimer timer;
    timer.start();

    result.totalFaces = triangles.size();

    // 교차 감지
    size_t intersectionCount;
    if (mUseOctree) {
        intersectionCount = detectUsingOctree(triangles, result);
    } else {
        intersectionCount = detectBruteForce(triangles, result);
    }

    result.intersectionCount = intersectionCount;
    result.detectionTime = timer.elapsed();

    return intersectionCount > 0;
}

bool SelfIntersectionRepairer::repairIntersections(RepairResult& result) {
    if (!mMesh) {
        result.success = false;
        return false;
    }

    if (result.intersections.empty()) {
        result.success = true;
        result.repairedCount = 0;
        return true;
    }

    QElapsedTimer timer;
    timer.start();

    size_t repairedCount = 0;
    size_t newFacesCreated = 0;
    size_t facesRemoved = 0;

    // 각 교차 쌍 수정
    for (auto& pair : result.intersections) {
        bool repaired = repairIntersectionPair(pair);
        if (repaired) {
            repairedCount++;
            // TODO: 실제 Face 생성/제거 통계 업데이트
        }
    }

    result.repairedCount = repairedCount;
    result.newFacesCreated = newFacesCreated;
    result.facesRemoved = facesRemoved;
    result.repairTime = timer.elapsed();
    result.success = (repairedCount == result.intersections.size());

    std::cout << "[SelfIntersectionRepairer] Intersection repair complete\n";
    std::cout << "  - Repaired: " << repairedCount << "/" << result.intersections.size() << "\n";
    std::cout << "  - New faces created: " << newFacesCreated << "\n";
    std::cout << "  - Faces removed: " << facesRemoved << "\n";
    std::cout << "  - Repair time: " << result.repairTime << " ms\n";

    return result.success;
}

bool SelfIntersectionRepairer::detectAndRepair(RepairResult& result) {
    QElapsedTimer totalTimer;
    totalTimer.start();

    // 1. 감지
    bool hasIntersections = detectIntersections(result);

    if (!hasIntersections) {
        result.success = true;
        result.totalTime = totalTimer.elapsed();
        std::cout << "[SelfIntersectionRepairer] No self-intersections found\n";
        return true;
    }

    // 2. 수정
    bool repaired = repairIntersections(result);

    result.totalTime = totalTimer.elapsed();

    std::cout << "[SelfIntersectionRepairer] Detect and repair complete\n";
    std::cout << "  - Total time: " << result.totalTime << " ms\n";

    return repaired;
}

// ============================================================================
// Triangle-Triangle Intersection (Möller 1997)
// ============================================================================

bool SelfIntersectionRepairer::triangleIntersectsTriangle(const Triangle& t1, const Triangle& t2) {
    // 1. AABB 테스트 (빠른 reject)
    DongArch::Math::Vec3 min1, max1, min2, max2;
    t1.computeAABB(min1, max1);
    t2.computeAABB(min2, max2);

    if (!aabbIntersects(min1, max1, min2, max2)) {
        return false;
    }

    // 2. Separating Axis Theorem (SAT) 기반 Triangle-Triangle intersection
    // Möller 1997: "A Fast Triangle-Triangle Intersection Test"

    // Plane equation for t1: n1 · (x - v0) = 0
    auto n1 = t1.normal();
    double d1 = -(n1.x * t1.v0.x + n1.y * t1.v0.y + n1.z * t1.v0.z);

    // Distance of t2 vertices from plane of t1
    double dist2_v0 = n1.x * t2.v0.x + n1.y * t2.v0.y + n1.z * t2.v0.z + d1;
    double dist2_v1 = n1.x * t2.v1.x + n1.y * t2.v1.y + n1.z * t2.v1.z + d1;
    double dist2_v2 = n1.x * t2.v2.x + n1.y * t2.v2.y + n1.z * t2.v2.z + d1;

    // Check if all vertices of t2 are on the same side of plane of t1
    if ((dist2_v0 > 0 && dist2_v1 > 0 && dist2_v2 > 0) ||
        (dist2_v0 < 0 && dist2_v1 < 0 && dist2_v2 < 0)) {
        return false;  // No intersection
    }

    // Plane equation for t2
    auto n2 = t2.normal();
    double d2 = -(n2.x * t2.v0.x + n2.y * t2.v0.y + n2.z * t2.v0.z);

    // Distance of t1 vertices from plane of t2
    double dist1_v0 = n2.x * t1.v0.x + n2.y * t1.v0.y + n2.z * t1.v0.z + d2;
    double dist1_v1 = n2.x * t1.v1.x + n2.y * t1.v1.y + n2.z * t1.v1.z + d2;
    double dist1_v2 = n2.x * t1.v2.x + n2.y * t1.v2.y + n2.z * t1.v2.z + d2;

    // Check if all vertices of t1 are on the same side of plane of t2
    if ((dist1_v0 > 0 && dist1_v1 > 0 && dist1_v2 > 0) ||
        (dist1_v0 < 0 && dist1_v1 < 0 && dist1_v2 < 0)) {
        return false;  // No intersection
    }

    // If we reach here, triangles likely intersect
    // (More detailed edge-edge tests would be needed for exact intersection)
    return true;
}

bool SelfIntersectionRepairer::triangleIntersectsTriangle(
    const Triangle& t1,
    const Triangle& t2,
    std::vector<DongArch::Math::Vec3>& intersectionLine)
{
    intersectionLine.clear();

    bool intersects = triangleIntersectsTriangle(t1, t2);
    if (!intersects) {
        return false;
    }

    // TODO: Compute exact intersection line
    // For now, just mark as intersecting
    return true;
}

bool SelfIntersectionRepairer::aabbIntersects(
    const DongArch::Math::Vec3& min1, const DongArch::Math::Vec3& max1,
    const DongArch::Math::Vec3& min2, const DongArch::Math::Vec3& max2)
{
    // AABB intersection test
    if (max1.x < min2.x || min1.x > max2.x) return false;
    if (max1.y < min2.y || min1.y > max2.y) return false;
    if (max1.z < min2.z || min1.z > max2.z) return false;
    return true;
}

// ============================================================================
// Private Methods: Intersection Detection
// ============================================================================

size_t SelfIntersectionRepairer::detectBruteForce(std::span<const Triangle> triangles, RepairResult& result) {
    result.intersections.clear();

    size_t count = 0;

    // Brute-force: O(n²)
    for (size_t i = 0; i < triangles.size(); i++) {
        for (size_t j = i + 1; j < triangles.size(); j++) {
            const Triangle& t1 = triangles[i];
            const Triangle& t2 = triangles[j];

            std::vector<DongArch::Math::Vec3> intersectionLine;
            bool intersects = triangleIntersectsTriangle(t1, t2, intersectionLine);

            if (intersects) {
                IntersectionPair pair;
                pair.faceIndex1 = i;
                pair.faceIndex2 = j;
                pair.face1 = nullptr;  // TODO: Get from mesh
                pair.face2 = nullptr;
                pair.triangle1 = t1;
                pair.triangle2 = t2;
                pair.intersectionLine = intersectionLine;

                result.intersections.push_back(pair);
                count++;
            }
        }
    }

    return count;
}

size_t SelfIntersectionRepairer::detectUsingOctree(std::span<const Triangle> triangles, RepairResult& result) {
    // TODO: Octree 기반 구현
    // 현재는 brute-force로 fallback
    std::cout << "[SelfIntersectionRepairer] Octree not implemented yet, using brute-force\n";
    return detectBruteForce(triangles, result);
}

bool SelfIntersectionRepairer::repairIntersectionPair(IntersectionPair& pair) {
    // 2025 논문: Instant Self-Intersection Repair
    // 1. 교차선 계산
    // 2. 삼각형 분할
    // 3. Topological correctness 유지

    if (pair.intersectionLine.empty()) {
        // 교차선 계산
        triangleIntersectsTriangle(pair.triangle1, pair.triangle2, pair.intersectionLine);
    }

    if (pair.intersectionLine.empty()) {
        // 교차선을 찾을 수 없음
        return false;
    }

    // TODO: 실제 삼각형 분할 및 메시 수정
    // 현재는 감지만 구현
    return true;
}

bool SelfIntersectionRepairer::splitTriangle(
    const Triangle& triangle,
    const std::vector<DongArch::Math::Vec3>& intersectionLine,
    std::vector<Triangle>& newTriangles)
{
    // TODO: 삼각형 분할 알고리즘 구현
    newTriangles.clear();
    return false;
}

// ============================================================================
// Helper Methods
// ============================================================================

std::vector<Triangle> SelfIntersectionRepairer::facesToTriangles(std::span<Face* const> faces) const {
    std::vector<Triangle> triangles;
    triangles.reserve(faces.size());

    for (Face* face : faces) {
        if (face) {
            Triangle tri = Triangle::fromFace(face);
            triangles.push_back(tri);
        }
    }

    return triangles;
}

Face* SelfIntersectionRepairer::triangleToFace(const Triangle& triangle) {
    if (!mMesh) {
        return nullptr;
    }

    // TODO: GigaMesh Mesh 클래스는 동적으로 Vertex/Face를 추가하는 간단한 API가 없음
    // 실제 구현을 위해서는:
    // 1. VertexOfFace* 3개 생성 (new VertexOfFace(x, y, z))
    // 2. std::vector<Vertex*> vertices에 추가
    // 3. mMesh->insertVertices(&vertices) 호출
    // 4. Face* face = new Face(index, vof0, vof1, vof2) 생성
    // 5. Mesh에 Face 추가 (API 확인 필요)
    // 6. mMesh->establishStructure() 호출 필요
    //
    // 현재는 컴파일 에러 방지를 위해 nullptr 반환
    // Phase 4 Self-Intersection Repair는 추후 구현

    (void)triangle; // Suppress unused parameter warning
    return nullptr;
}

} // namespace Algorithms
} // namespace Clip
} // namespace DongArch
