/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "DongArchClipManager.h"
#include <QElapsedTimer>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <set>

namespace DongArch {
namespace Clip {

DongArchClipManager::DongArchClipManager(Mesh* mesh)
    : mMesh(mesh)
{
}

bool DongArchClipManager::getMeshBounds(double& minZ, double& maxZ) const {
    if (!mMesh) {
        return false;
    }

    Vector3D bboxMin = mMesh->getBoundingBoxA();
    Vector3D bboxMax = mMesh->getBoundingBoxG();

    minZ = bboxMin.getZ();
    maxZ = bboxMax.getZ();
    return true;
}

Vector3D DongArchClipManager::createHorizontalPlane(double normalizedHeight) const {
    if (!mMesh) {
        return Vector3D(0.0, 0.0, 1.0, 0.0);
    }

    // Get mesh bounds
    double minZ, maxZ;
    if (!getMeshBounds(minZ, maxZ)) {
        return Vector3D(0.0, 0.0, 1.0, 0.0);
    }

    // Compute Z coordinate from normalized height
    double z = minZ + normalizedHeight * (maxZ - minZ);

    // Create horizontal plane at height z
    // Plane equation: 0*x + 0*y + 1*z - z = 0
    // Normal: (0, 0, 1), pointing up
    // HNF: (nx, ny, nz, -d) where d = distance from origin
    Vector3D planeHNF(0.0, 0.0, 1.0, -z);

    return planeHNF;
}

bool DongArchClipManager::clipByPlane(const ClipParams& params, ClipResult& result) {
    if (!mMesh) {
        result.success = false;
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    // 원본 통계
    result.originalFaceCount = mMesh->getFaceNr();

    // 3개 함수 생성
    auto intersectFun = createIntersectFunction(params.planeHNF);
    auto distFun = createDistanceFunction(params.planeHNF);
    auto getIntersectionVectorFun = createIntersectionVectorFunction(params.planeHNF);

    // 오프셋 벡터 계산
    Vector3D offset = params.uniformOffset;
    if (params.duplicateVertices && offset.getLength3() < 1e-10) {
        // 기본 오프셋: 평면 법선 방향으로 epsilon
        offset = Vector3D(
            params.planeHNF.getX() * params.epsilon,
            params.planeHNF.getY() * params.epsilon,
            params.planeHNF.getZ() * params.epsilon
        );
        offset.setH(1.0);
    }

    // GigaMesh splitMesh 호출
    bool success = splitMeshDirect(
        intersectFun,
        distFun,
        getIntersectionVectorFun,
        params.duplicateVertices,
        params.noRedraw,
        offset
    );

    if (!success) {
        result.success = false;
        return false;
    }

    // 후처리: 모드에 따라 면 제거
    size_t removedCount = postProcessClip(params.planeHNF, params.mode);

    // 절단선 추출
    extractIntersectionLine(params.planeHNF, result.intersectionPoints);
    result.intersectionEdgeCount = result.intersectionPoints.size() / 2;

    // 결과 통계
    result.frontFaceCount = mMesh->getFaceNr();  // 후처리 후 남은 면
    result.backFaceCount = removedCount;
    result.newVertexCount = mMesh->getVertexNr() - result.originalFaceCount * 3;  // 근사치
    result.processingTime = timer.elapsed();
    result.success = true;

    std::cout << "[DongArchClipManager::clipByPlane] 완료\n";
    std::cout << "  - 원본 면: " << result.originalFaceCount << "\n";
    std::cout << "  - 최종 면: " << result.frontFaceCount << "\n";
    std::cout << "  - 제거된 면: " << removedCount << "\n";
    std::cout << "  - 교차점: " << result.intersectionPoints.size() << "\n";
    std::cout << "  - 처리 시간: " << result.processingTime << " ms\n";

    return true;
}

bool DongArchClipManager::clipByPlane(const Vector3D& planeHNF, ClipMode mode) {
    ClipParams params;
    params.planeHNF = planeHNF;
    params.mode = mode;
    params.duplicateVertices = false;
    params.noRedraw = true;
    params.epsilon = 1e-10;

    ClipResult result;
    return clipByPlane(params, result);
}

bool DongArchClipManager::extractIntersectionLine(
    const Vector3D& planeHNF,
    std::vector<Vector3D>& intersectionPoints)
{
    if (!mMesh) {
        return false;
    }

    // GigaMesh의 calcIntersectionPolylineWithPlane 호출
    return mMesh->calcIntersectionPolylineWithPlane(planeHNF, &intersectionPoints);
}

bool DongArchClipManager::splitMeshDirect(
    const std::function<bool(Face*)>& intersectFun,
    const std::function<double(VertexOfFace*)>& distFun,
    const std::function<void(VertexOfFace*, VertexOfFace*, Vector3D&)>& getIntersectionVectorFun,
    bool duplicateVertices,
    bool noRedraw,
    const Vector3D& uniformOffset)
{
    if (!mMesh) {
        return false;
    }

    // GigaMesh mesh.cpp:4149 splitMesh() 직접 호출
    return mMesh->splitMesh(
        intersectFun,
        distFun,
        getIntersectionVectorFun,
        duplicateVertices,
        noRedraw,
        uniformOffset
    );
}

// ============================================================================
// Static Factory Functions (C++20 std::function)
// ============================================================================

std::function<bool(Face*)>
DongArchClipManager::createIntersectFunction(const Vector3D& planeHNF) {
    // Lambda capture by value (C++20)
    return [planeHNF](Face* face) -> bool {
        if (!face) return false;
        return face->intersectsPlane(&planeHNF);
    };
}

std::function<double(VertexOfFace*)>
DongArchClipManager::createDistanceFunction(const Vector3D& planeHNF) {
    // Lambda capture by value (C++20)
    return [planeHNF](VertexOfFace* vertex) -> double {
        if (!vertex) return 0.0;
        return vertex->estDistanceToPlane(planeHNF);
    };
}

std::function<void(VertexOfFace*, VertexOfFace*, Vector3D&)>
DongArchClipManager::createIntersectionVectorFunction(const Vector3D& planeHNF) {
    // Lambda capture by value (C++20)
    // Plane 객체를 lambda 내부에서 생성
    return [planeHNF](VertexOfFace* vertX, VertexOfFace* vertY, Vector3D& rVecIntersection) {
        if (!vertX || !vertY) return;

        // Plane 객체 생성 (GigaMesh API)
        // const_cast 필요 (GigaMesh API 제약)
        Vector3D planeHNF_copy = planeHNF;
        Plane cutPlane(&planeHNF_copy);

        // 교차점 계산
        cutPlane.getIntersectionFacePlaneLinePos(
            vertX->getPositionVector(),
            vertY->getPositionVector(),
            rVecIntersection
        );
    };
}

// ============================================================================
// Private Methods: Post-Processing
// ============================================================================

size_t DongArchClipManager::postProcessClip(const Vector3D& planeHNF, ClipMode mode) {
    switch (mode) {
        case ClipMode::KEEP_FRONT:
            return removeBackFaces(planeHNF);

        case ClipMode::KEEP_BACK:
            return removeFrontFaces(planeHNF);

        case ClipMode::KEEP_BOTH:
        case ClipMode::SPLIT_ONLY:
            return 0;  // 아무것도 제거하지 않음

        default:
            return 0;
    }
}

size_t DongArchClipManager::removeFrontFaces(const Vector3D& planeHNF) {
    if (!mMesh) return 0;

    std::vector<Face*> facesToRemove;

    for (size_t i = 0; i < mMesh->getFaceNr(); i++) {
        Face* face = mMesh->getFacePos(i);
        if (face && isFaceFront(face, planeHNF)) {
            facesToRemove.push_back(face);
        }
    }

    // 면 제거
    std::set<Face*> faceSet(facesToRemove.begin(), facesToRemove.end());
    mMesh->removeFaces(&faceSet);

    std::cout << "[DongArchClipManager] 앞쪽 면 " << facesToRemove.size() << "개 제거\n";
    return facesToRemove.size();
}

size_t DongArchClipManager::removeBackFaces(const Vector3D& planeHNF) {
    if (!mMesh) return 0;

    std::vector<Face*> facesToRemove;

    for (size_t i = 0; i < mMesh->getFaceNr(); i++) {
        Face* face = mMesh->getFacePos(i);
        if (face && isFaceBack(face, planeHNF)) {
            facesToRemove.push_back(face);
        }
    }

    // 면 제거
    std::set<Face*> faceSet(facesToRemove.begin(), facesToRemove.end());
    mMesh->removeFaces(&faceSet);

    std::cout << "[DongArchClipManager] 뒤쪽 면 " << facesToRemove.size() << "개 제거\n";
    return facesToRemove.size();
}

bool DongArchClipManager::isFaceFront(Face* face, const Vector3D& planeHNF) const {
    if (!face) return false;

    // 면의 3개 정점의 평균 거리로 판단
    Vertex* v0 = face->getVertA();
    Vertex* v1 = face->getVertB();
    Vertex* v2 = face->getVertC();

    if (!v0 || !v1 || !v2) return false;

    double dist0 = v0->estDistanceToPlane(planeHNF);
    double dist1 = v1->estDistanceToPlane(planeHNF);
    double dist2 = v2->estDistanceToPlane(planeHNF);

    double avgDist = (dist0 + dist1 + dist2) / 3.0;

    return avgDist > 0.0;  // 양수면 앞쪽
}

bool DongArchClipManager::isFaceBack(Face* face, const Vector3D& planeHNF) const {
    if (!face) return false;

    // 면의 3개 정점의 평균 거리로 판단
    Vertex* v0 = face->getVertA();
    Vertex* v1 = face->getVertB();
    Vertex* v2 = face->getVertC();

    if (!v0 || !v1 || !v2) return false;

    double dist0 = v0->estDistanceToPlane(planeHNF);
    double dist1 = v1->estDistanceToPlane(planeHNF);
    double dist2 = v2->estDistanceToPlane(planeHNF);

    double avgDist = (dist0 + dist1 + dist2) / 3.0;

    return avgDist < 0.0;  // 음수면 뒤쪽
}

} // namespace Clip
} // namespace DongArch
