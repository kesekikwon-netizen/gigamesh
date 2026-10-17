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

#include "DongArchAlignManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ranges>

using namespace DongArch::Math;

namespace DongArch {
namespace Align {

// ============================================================================
// 생성자
// ============================================================================

AlignManager::AlignManager(Mesh* mesh)
    : mMesh(mesh)
{
    if (!isValid()) {
        std::cerr << "[AlignManager] WARNING: Invalid mesh or no vertices!" << std::endl;
    }
}

// ============================================================================
// Rotation: 회전 변환
// ============================================================================

bool AlignManager::rotateX(double degrees, bool resetNormals) {
    if (!isValid()) {
        return false;
    }

    // DongArchMath 회전 행렬 생성
    double radians = degreesToRadians(degrees);
    Mat3 rot3 = rotationMatrixX(radians);

    // Mat3 → Matrix4D 변환
    Matrix4D rot4 = toMatrix4D(rot3);

    // GigaMesh 함수 호출
    return mMesh->applyTransformationToWholeMesh(rot4, resetNormals);
}

bool AlignManager::rotateY(double degrees, bool resetNormals) {
    if (!isValid()) {
        return false;
    }

    double radians = degreesToRadians(degrees);
    Mat3 rot3 = rotationMatrixY(radians);
    Matrix4D rot4 = toMatrix4D(rot3);

    return mMesh->applyTransformationToWholeMesh(rot4, resetNormals);
}

bool AlignManager::rotateZ(double degrees, bool resetNormals) {
    if (!isValid()) {
        return false;
    }

    double radians = degreesToRadians(degrees);
    Mat3 rot3 = rotationMatrixZ(radians);
    Matrix4D rot4 = toMatrix4D(rot3);

    return mMesh->applyTransformationToWholeMesh(rot4, resetNormals);
}

bool AlignManager::rotateAroundAxis(const Vec3& axis, double degrees, bool resetNormals) {
    if (!isValid()) {
        return false;
    }

    double radians = degreesToRadians(degrees);
    Mat3 rot3 = rotationMatrixAxis(axis, radians);
    Matrix4D rot4 = toMatrix4D(rot3);

    return mMesh->applyTransformationToWholeMesh(rot4, resetNormals);
}

bool AlignManager::applyTransformation(const Matrix4D& matrix, bool resetNormals) {
    if (!isValid()) {
        return false;
    }

    return mMesh->applyTransformationToWholeMesh(matrix, resetNormals);
}

// ============================================================================
// Alignment: 정렬
// ============================================================================

bool AlignManager::alignToGroundPlane() {
    if (!isValid()) {
        return false;
    }

    // C++20 ranges: 모든 정점의 Z 좌표 찾기
    auto vertexPositions = getVertexPositions();

    // std::ranges::min_element로 최소 Z값 찾기
    auto minZVertex = std::ranges::min(
        vertexPositions,
        {},
        [](const Vec3& v) { return v.z; }
    );

    double minZ = minZVertex.z;

    // Z축으로 평행이동: -minZ만큼 이동하여 바닥을 Z=0에 맞춤
    std::vector<double> transValues = {0.0, 0.0, -minZ};
    Matrix4D translation(Matrix4D::INIT_TRANSLATE, &transValues);

    return applyTransformation(translation, false);  // 평행이동은 법선 재계산 불필요
}

bool AlignManager::centerMesh() {
    if (!isValid()) {
        return false;
    }

    // 메시 중심 계산
    Vec3 center = getMeshCenter();

    // 원점으로 이동
    std::vector<double> transValues = {-center.x, -center.y, -center.z};
    Matrix4D translation(Matrix4D::INIT_TRANSLATE, &transValues);

    return applyTransformation(translation, false);
}

bool AlignManager::alignToStandardView(StandardView view) {
    if (!isValid()) {
        return false;
    }

    // 표준 뷰에 따라 회전 각도 결정
    switch (view) {
        case StandardView::TOP:
            // 이미 상단 뷰 (Z축 상향) - 변환 불필요
            return true;

        case StandardView::BOTTOM:
            // X축 180도 회전 -> Z축 하향
            return rotateX(180.0);

        case StandardView::FRONT:
            // X축 -90도 회전 -> Y축 전방
            return rotateX(-90.0);

        case StandardView::BACK:
            // X축 90도 회전 -> Y축 후방
            return rotateX(90.0);

        case StandardView::LEFT:
            // Z축 90도 회전 + X축 -90도 회전 -> X축 좌측
            return rotateZ(90.0) && rotateX(-90.0);

        case StandardView::RIGHT:
            // Z축 -90도 회전 + X축 -90도 회전 -> X축 우측
            return rotateZ(-90.0) && rotateX(-90.0);

        default:
            return false;
    }
}

// ============================================================================
// C++20 std::ranges: 정점 데이터 접근
// ============================================================================

bool AlignManager::getBoundingBox(Vec3& min, Vec3& max) const {
    if (!isValid()) {
        return false;
    }

    // C++20 ranges로 바운딩 박스 계산
    auto positions = getVertexPositions();

    // 첫 정점으로 초기화
    auto firstVertex = *std::ranges::begin(positions);
    min = firstVertex;
    max = firstVertex;

    // 나머지 정점 순회
    for (const auto& v : positions) {
        min.x = std::min(min.x, v.x);
        min.y = std::min(min.y, v.y);
        min.z = std::min(min.z, v.z);

        max.x = std::max(max.x, v.x);
        max.y = std::max(max.y, v.y);
        max.z = std::max(max.z, v.z);
    }

    return true;
}

Vec3 AlignManager::getMeshCenter() const {
    if (!isValid()) {
        return Vec3(0, 0, 0);
    }

    Vec3 min, max;
    getBoundingBox(min, max);

    // 바운딩 박스 중심
    return Vec3(
        (min.x + max.x) / 2.0,
        (min.y + max.y) / 2.0,
        (min.z + max.z) / 2.0
    );
}

// ============================================================================
// Utility: Matrix 변환
// ============================================================================

Mat3 AlignManager::toMat3(const Matrix4D& mat4) const {
    Mat3 result;

    // 4x4 행렬의 좌상단 3x3 추출 (회전 부분)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result.m[i][j] = mat4.get(i, j);
        }
    }

    return result;
}

Matrix4D AlignManager::toMatrix4D(const Mat3& mat3) const {
    Matrix4D result(Matrix4D::INIT_IDENTITY);  // 단위 행렬로 초기화

    // 3x3 회전 행렬을 4x4 좌상단에 복사
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result.set(i, j, mat3.m[i][j]);
        }
    }

    return result;
}

} // namespace Align
} // namespace DongArch
