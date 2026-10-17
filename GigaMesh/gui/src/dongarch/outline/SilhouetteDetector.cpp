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

#include "SilhouetteDetector.h"
#include <QtConcurrent/QtConcurrent>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace DongArch {
namespace Outline {

// ============================================================================
// EdgeRef 구현
// ============================================================================

bool EdgeRef::getVertices(Vertex*& v1, Vertex*& v2) const {
    if (!face || edgeIdx == Face::EDGE_NONE) {
        return false;
    }

    switch (edgeIdx) {
        case Face::EDGE_AB:
            v1 = face->getVertA();
            v2 = face->getVertB();
            return true;
        case Face::EDGE_BC:
            v1 = face->getVertB();
            v2 = face->getVertC();
            return true;
        case Face::EDGE_CA:
            v1 = face->getVertC();
            v2 = face->getVertA();
            return true;
        default:
            return false;
    }
}

// ============================================================================
// 생성자
// ============================================================================

SilhouetteDetector::SilhouetteDetector(Mesh* mesh)
    : mMesh(mesh)
    , mParallelThreshold(100)  // 100개 이상 엣지 시 병렬 처리
{
    if (!isValid()) {
        std::cerr << "[SilhouetteDetector] WARNING: Invalid mesh!" << std::endl;
    }
}

// ============================================================================
// ViewDirection → 뷰 벡터 변환
// ============================================================================

Vector3D SilhouetteDetector::getViewVector(ViewDirection viewDir) {
    switch (viewDir) {
        case ViewDirection::TOP:
            return Vector3D(0.0, -1.0, 0.0);  // Y축 음의 방향 (위에서 아래로)
        case ViewDirection::BOTTOM:
            return Vector3D(0.0, 1.0, 0.0);   // Y축 양의 방향 (아래에서 위로)
        case ViewDirection::FRONT:
            return Vector3D(0.0, 0.0, -1.0);  // Z축 음의 방향 (앞에서 뒤로)
        case ViewDirection::BACK:
            return Vector3D(0.0, 0.0, 1.0);   // Z축 양의 방향 (뒤에서 앞으로)
        case ViewDirection::RIGHT:
            return Vector3D(1.0, 0.0, 0.0);   // X축 양의 방향 (오른쪽에서 왼쪽으로)
        case ViewDirection::LEFT:
            return Vector3D(-1.0, 0.0, 0.0);  // X축 음의 방향 (왼쪽에서 오른쪽으로)
        default:
            return Vector3D(0.0, 0.0, -1.0);  // 기본값: FRONT
    }
}

const char* SilhouetteDetector::viewDirectionName(ViewDirection viewDir) {
    switch (viewDir) {
        case ViewDirection::TOP:    return "위 (Top)";
        case ViewDirection::BOTTOM: return "아래 (Bottom)";
        case ViewDirection::FRONT:  return "앞 (Front)";
        case ViewDirection::BACK:   return "뒤 (Back)";
        case ViewDirection::RIGHT:  return "오른쪽 (Right)";
        case ViewDirection::LEFT:   return "왼쪽 (Left)";
        default:                    return "알 수 없음";
    }
}

// ============================================================================
// Silhouette Edge 판정 (핵심 알고리즘)
// ============================================================================

bool SilhouetteDetector::isSilhouetteEdge(const EdgeRef& edgeRef, const Vector3D& viewVector) {
    if (!edgeRef.face || edgeRef.edgeIdx == Face::EDGE_NONE) {
        return false;
    }

    // Boundary edge (한쪽 면만 있는 경우)는 항상 silhouette
    Face* face1 = edgeRef.face;
    Face* face2 = edgeRef.getNeighbourFace();

    if (!face1 || !face2) {
        return true;  // Boundary edge
    }

    // 두 면의 법선 벡터 조회
    Vector3D normal1 = face1->getNormal(false);  // 정규화되지 않은 법선
    Vector3D normal2 = face2->getNormal(false);

    // 법선 벡터 정규화
    normal1.normalize3();
    normal2.normalize3();

    // Dot product 계산: (n1 · view) * (n2 · view)
    double dot1 = normal1.getX() * viewVector.getX() +
                  normal1.getY() * viewVector.getY() +
                  normal1.getZ() * viewVector.getZ();

    double dot2 = normal2.getX() * viewVector.getX() +
                  normal2.getY() * viewVector.getY() +
                  normal2.getZ() * viewVector.getZ();

    // Silhouette 조건: 한 면은 앞을 향하고, 다른 면은 뒤를 향함
    // (n1 · view) * (n2 · view) < 0
    return (dot1 * dot2) < 0.0;
}

// ============================================================================
// 모든 엣지 조회
// ============================================================================

std::vector<EdgeRef> SilhouetteDetector::getAllEdges() const {
    std::vector<EdgeRef> edges;

    if (!isValid()) {
        return edges;
    }

    // Mesh에서 모든 Face 조회
    const std::vector<Face*>* faces = mMesh->getPrimitiveListFaces();
    if (!faces) {
        return edges;
    }

    // Face의 모든 Edge 수집 (중복 제거 필요)
    // 각 엣지는 최대 2개 Face에 속하므로, 정점 쌍으로 중복 제거
    std::set<std::pair<Vertex*, Vertex*>> edgeSet;
    std::vector<EdgeRef> allEdgeRefs;

    for (Face* face : *faces) {
        if (!face) continue;

        // Face의 3개 Edge 추가
        const Face::eEdgeNames edgeIndices[] = {
            Face::EDGE_AB,
            Face::EDGE_BC,
            Face::EDGE_CA
        };

        for (Face::eEdgeNames idx : edgeIndices) {
            EdgeRef edgeRef(face, idx);

            // 정점 조회
            Vertex* v1 = nullptr;
            Vertex* v2 = nullptr;
            if (!edgeRef.getVertices(v1, v2)) {
                continue;
            }

            // 정규화된 키 생성 (v1 < v2 순서)
            auto edgeKey = (v1 < v2) ? std::make_pair(v1, v2) : std::make_pair(v2, v1);

            // 중복 체크
            if (edgeSet.find(edgeKey) == edgeSet.end()) {
                edgeSet.insert(edgeKey);
                allEdgeRefs.push_back(edgeRef);
            }
        }
    }

    return allEdgeRefs;
}

// ============================================================================
// Silhouette 검출 (순차 처리 - std::ranges)
// ============================================================================

std::vector<EdgeRef> SilhouetteDetector::detectSequential(
    const std::vector<EdgeRef>& edges,
    const Vector3D& viewVector)
{
    // C++20 std::ranges::filter 사용
    auto silhouetteView = edges
        | std::views::filter([&viewVector](const EdgeRef& edgeRef) {
              return isSilhouetteEdge(edgeRef, viewVector);
          });

    // View → Vector 변환
    std::vector<EdgeRef> result;
    result.reserve(edges.size() / 10);  // 추정: 10% 정도가 silhouette

    for (const EdgeRef& edgeRef : silhouetteView) {
        result.push_back(edgeRef);
    }

    return result;
}

// ============================================================================
// Silhouette 검출 (병렬 처리 - Qt Concurrent)
// ============================================================================

std::vector<EdgeRef> SilhouetteDetector::detectParallel(
    const std::vector<EdgeRef>& edges,
    const Vector3D& viewVector)
{
    // Qt Concurrent::blockingFiltered 사용
    auto silhouetteEdges = QtConcurrent::blockingFiltered(
        edges,
        [&viewVector](const EdgeRef& edgeRef) {
            return isSilhouetteEdge(edgeRef, viewVector);
        }
    );

    // QList → std::vector 변환 (Qt 5)
    return std::vector<EdgeRef>(silhouetteEdges.begin(), silhouetteEdges.end());
}

// ============================================================================
// Silhouette 검출 (내부 구현)
// ============================================================================

bool SilhouetteDetector::detectSilhouetteInternal(
    const Vector3D& viewVector,
    SilhouetteResult& result)
{
    if (!isValid()) {
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    // 모든 엣지 조회
    std::vector<EdgeRef> allEdges = getAllEdges();
    result.totalEdges = allEdges.size();

    if (allEdges.empty()) {
        result.silhouetteEdges = 0;
        result.processingTime = 0.0;
        return false;
    }

    // 뷰 벡터 정규화 (이미 정규화되어 있을 수도 있지만 확실히)
    Vector3D normalizedView = viewVector;
    normalizedView.normalize3();
    result.viewVector = normalizedView;

    // 병렬 처리 여부 결정
    if (allEdges.size() >= mParallelThreshold) {
        result.edges = detectParallel(allEdges, normalizedView);
    } else {
        result.edges = detectSequential(allEdges, normalizedView);
    }

    result.silhouetteEdges = result.edges.size();
    result.processingTime = timer.elapsed();

    return true;
}

// ============================================================================
// Silhouette 검출 (공개 인터페이스)
// ============================================================================

bool SilhouetteDetector::detectSilhouette(
    ViewDirection viewDir,
    SilhouetteResult& result)
{
    result.viewDirection = viewDir;
    Vector3D viewVector = getViewVector(viewDir);
    return detectSilhouetteInternal(viewVector, result);
}

bool SilhouetteDetector::detectSilhouette(
    const Vector3D& viewVector,
    SilhouetteResult& result)
{
    // viewDirection은 CUSTOM으로 설정할 수 없으므로 FRONT로 기본값
    result.viewDirection = ViewDirection::FRONT;
    return detectSilhouetteInternal(viewVector, result);
}

// ============================================================================
// 6방향 모두 검출
// ============================================================================

size_t SilhouetteDetector::detectAllDirections(std::vector<SilhouetteResult>& results) {
    if (!isValid()) {
        return 0;
    }

    results.clear();
    results.reserve(6);

    size_t successCount = 0;

    // 6방향 순회
    const ViewDirection directions[] = {
        ViewDirection::TOP,
        ViewDirection::BOTTOM,
        ViewDirection::FRONT,
        ViewDirection::BACK,
        ViewDirection::RIGHT,
        ViewDirection::LEFT
    };

    for (ViewDirection dir : directions) {
        SilhouetteResult result;
        if (detectSilhouette(dir, result)) {
            results.push_back(result);
            successCount++;

            std::cout << "[SilhouetteDetector] "
                      << viewDirectionName(dir) << ": "
                      << result.silhouetteEdges << "/" << result.totalEdges
                      << " edges (" << result.processingTime << " ms)"
                      << std::endl;
        }
    }

    return successCount;
}

} // namespace Outline
} // namespace DongArch
