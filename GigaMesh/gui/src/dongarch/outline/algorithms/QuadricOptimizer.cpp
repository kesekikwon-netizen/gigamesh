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

#include "QuadricOptimizer.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

namespace DongArch {
namespace Outline {
namespace Algorithms {

// ============================================================================
// QuadricMatrix 구현
// ============================================================================

QuadricMatrix::QuadricMatrix() {
    clear();
}

QuadricMatrix::QuadricMatrix(const Vector3D& plane) {
    clear();

    // 평면 방정식: ax + by + cz + d = 0
    // Quadric K_p = p * p^T (p = [a, b, c, d]^T)
    double a = plane.getX();
    double b = plane.getY();
    double c = plane.getZ();
    double d = plane.getH();

    // 대칭 행렬이므로 상삼각 행렬만 계산
    mData[0] = a * a;  // Q[0][0]
    mData[1] = a * b;  // Q[0][1]
    mData[2] = a * c;  // Q[0][2]
    mData[3] = a * d;  // Q[0][3]

    mData[4] = a * b;  // Q[1][0] (symmetric)
    mData[5] = b * b;  // Q[1][1]
    mData[6] = b * c;  // Q[1][2]
    mData[7] = b * d;  // Q[1][3]

    mData[8] = a * c;   // Q[2][0] (symmetric)
    mData[9] = b * c;   // Q[2][1] (symmetric)
    mData[10] = c * c;  // Q[2][2]
    mData[11] = c * d;  // Q[2][3]

    mData[12] = a * d;  // Q[3][0] (symmetric)
    mData[13] = b * d;  // Q[3][1] (symmetric)
    mData[14] = c * d;  // Q[3][2] (symmetric)
    mData[15] = d * d;  // Q[3][3]
}

void QuadricMatrix::set(int row, int col, double value) {
    mData[row * 4 + col] = value;
}

double QuadricMatrix::get(int row, int col) const {
    return mData[row * 4 + col];
}

double QuadricMatrix::computeError(const Vector3D& v) const {
    // Error(v) = v^T * Q * v
    // v = [x, y, z, 1]^T (homogeneous coordinates)
    double x = v.getX();
    double y = v.getY();
    double z = v.getZ();
    double w = 1.0;

    // Q * v
    double qv0 = mData[0] * x + mData[1] * y + mData[2] * z + mData[3] * w;
    double qv1 = mData[4] * x + mData[5] * y + mData[6] * z + mData[7] * w;
    double qv2 = mData[8] * x + mData[9] * y + mData[10] * z + mData[11] * w;
    double qv3 = mData[12] * x + mData[13] * y + mData[14] * z + mData[15] * w;

    // v^T * (Q * v)
    double error = x * qv0 + y * qv1 + z * qv2 + w * qv3;
    return error;
}

QuadricMatrix QuadricMatrix::operator+(const QuadricMatrix& other) const {
    QuadricMatrix result;
    for (int i = 0; i < 16; i++) {
        result.mData[i] = mData[i] + other.mData[i];
    }
    return result;
}

QuadricMatrix& QuadricMatrix::operator+=(const QuadricMatrix& other) {
    for (int i = 0; i < 16; i++) {
        mData[i] += other.mData[i];
    }
    return *this;
}

QuadricMatrix QuadricMatrix::operator*(double scalar) const {
    QuadricMatrix result;
    for (int i = 0; i < 16; i++) {
        result.mData[i] = mData[i] * scalar;
    }
    return result;
}

void QuadricMatrix::clear() {
    mData.fill(0.0);
}

// ============================================================================
// QuadricOptimizer 구현
// ============================================================================

QuadricOptimizer::QuadricOptimizer(Mesh* mesh)
    : mMesh(mesh)
{
    if (!isValid()) {
        std::cerr << "[QuadricOptimizer] WARNING: Invalid mesh!" << std::endl;
    }
}

// ============================================================================
// Quadric Matrix 계산
// ============================================================================

QuadricMatrix QuadricOptimizer::computeQuadricMatrix(const Vertex* vertex) const {
    QuadricMatrix Q;

    if (!vertex) {
        return Q;
    }

    // 정점의 인접 Face들 조회
    // Note: GigaMesh API는 const-correctness를 완전히 지원하지 않음
    // const_cast 사용 (읽기 전용이므로 안전)
    std::set<Face*> adjacentFaces;
    const_cast<Vertex*>(vertex)->getFaces(&adjacentFaces);

    // 각 Face의 평면 방정식에서 Quadric 계산 후 합산
    for (Face* face : adjacentFaces) {
        if (!face) continue;

        // Face의 평면 방정식 (Hesse Normal Form)
        Vector3D normal = face->getNormal(true);  // 정규화된 법선
        // HNF: ax + by + cz + d = 0, d는 원점에서 평면까지 거리
        // 정점 중 하나를 사용해 d 계산
        Vertex* v0 = face->getVertA();
        double d = -(normal.getX() * v0->getX() +
                     normal.getY() * v0->getY() +
                     normal.getZ() * v0->getZ());
        Vector3D plane(normal.getX(), normal.getY(), normal.getZ(), d);

        // Quadric Matrix 생성 및 누적
        QuadricMatrix K_p(plane);
        Q += K_p;
    }

    return Q;
}

// ============================================================================
// Quadric Error 계산
// ============================================================================

double QuadricOptimizer::quadricError(const Vertex* vertex) const {
    if (!vertex) {
        return std::numeric_limits<double>::max();
    }

    // Quadric Matrix 계산
    QuadricMatrix Q = computeQuadricMatrix(vertex);

    // 정점 위치
    Vector3D pos(vertex->getX(), vertex->getY(), vertex->getZ());

    // Error 계산
    return Q.computeError(pos);
}

// ============================================================================
// Douglas-Peucker 알고리즘
// ============================================================================

double QuadricOptimizer::perpendicularDistance(
    const DongArch::Math::Vec3& point,
    const DongArch::Math::Vec3& lineStart,
    const DongArch::Math::Vec3& lineEnd)
{
    // 선분 벡터
    double dx = lineEnd.x - lineStart.x;
    double dy = lineEnd.y - lineStart.y;
    double dz = lineEnd.z - lineStart.z;

    double lineLength = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (lineLength < 1e-10) {
        // 선분 길이가 거의 0 (시작점 = 끝점)
        double pdx = point.x - lineStart.x;
        double pdy = point.y - lineStart.y;
        double pdz = point.z - lineStart.z;
        return std::sqrt(pdx * pdx + pdy * pdy + pdz * pdz);
    }

    // 선분 방향 단위 벡터
    double ux = dx / lineLength;
    double uy = dy / lineLength;
    double uz = dz / lineLength;

    // 점에서 선분 시작점으로의 벡터
    double px = point.x - lineStart.x;
    double py = point.y - lineStart.y;
    double pz = point.z - lineStart.z;

    // 투영 길이 (dot product)
    double t = px * ux + py * uy + pz * uz;

    // 투영점
    double projX, projY, projZ;
    if (t < 0.0) {
        // 선분 밖 (시작점 쪽)
        projX = lineStart.x;
        projY = lineStart.y;
        projZ = lineStart.z;
    } else if (t > lineLength) {
        // 선분 밖 (끝점 쪽)
        projX = lineEnd.x;
        projY = lineEnd.y;
        projZ = lineEnd.z;
    } else {
        // 선분 위
        projX = lineStart.x + t * ux;
        projY = lineStart.y + t * uy;
        projZ = lineStart.z + t * uz;
    }

    // 수직 거리
    double distX = point.x - projX;
    double distY = point.y - projY;
    double distZ = point.z - projZ;
    return std::sqrt(distX * distX + distY * distY + distZ * distZ);
}

void QuadricOptimizer::douglasPeuckerRecursive(
    std::span<const DongArch::Math::Vec3> points,
    size_t start,
    size_t end,
    double epsilon,
    std::vector<bool>& keep)
{
    if (end <= start + 1) {
        return;  // 점이 2개 이하
    }

    // 가장 먼 점 찾기
    double maxDist = 0.0;
    size_t maxIndex = start;

    for (size_t i = start + 1; i < end; i++) {
        double dist = perpendicularDistance(points[i], points[start], points[end]);
        if (dist > maxDist) {
            maxDist = dist;
            maxIndex = i;
        }
    }

    // 거리가 임계값보다 크면 분할
    if (maxDist > epsilon) {
        keep[maxIndex] = true;

        // 재귀
        douglasPeuckerRecursive(points, start, maxIndex, epsilon, keep);
        douglasPeuckerRecursive(points, maxIndex, end, epsilon, keep);
    }
    // 아니면 중간 점들 모두 제거 (keep = false 유지)
}

bool QuadricOptimizer::simplifyDouglasPeucker(
    std::span<const DongArch::Math::Vec3> points,
    double epsilon,
    SimplificationResult& result)
{
    if (points.size() < 3) {
        result.originalCount = points.size();
        result.simplifiedCount = points.size();
        result.points.assign(points.begin(), points.end());
        return true;
    }

    result.originalCount = points.size();

    // 보존할 점 마킹
    std::vector<bool> keep(points.size(), false);
    keep[0] = true;  // 시작점
    keep[points.size() - 1] = true;  // 끝점

    // Douglas-Peucker 재귀
    douglasPeuckerRecursive(points, 0, points.size() - 1, epsilon, keep);

    // 보존할 점만 추출
    result.points.clear();
    for (size_t i = 0; i < points.size(); i++) {
        if (keep[i]) {
            result.points.push_back(points[i]);
        }
    }

    result.simplifiedCount = result.points.size();
    result.compressionRatio = 100.0 * (1.0 - static_cast<double>(result.simplifiedCount) /
                                       static_cast<double>(result.originalCount));
    result.averageError = 0.0;  // Douglas-Peucker는 Quadric Error 없음
    result.maxError = 0.0;

    return true;
}

// ============================================================================
// Quadric 기반 간략화
// ============================================================================

Vertex* QuadricOptimizer::findVertex(const DongArch::Math::Vec3& pos, double tolerance) const {
    if (!isValid()) {
        return nullptr;
    }

    const std::vector<Vertex*>* vertices = mMesh->getPrimitiveListVertices();
    if (!vertices) {
        return nullptr;
    }

    // 가장 가까운 정점 찾기
    Vertex* closest = nullptr;
    double minDist = tolerance;

    for (Vertex* v : *vertices) {
        if (!v) continue;

        double dx = v->getX() - pos.x;
        double dy = v->getY() - pos.y;
        double dz = v->getZ() - pos.z;
        double dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist < minDist) {
            minDist = dist;
            closest = v;
        }
    }

    return closest;
}

bool QuadricOptimizer::simplifyQuadric(
    std::span<const DongArch::Math::Vec3> points,
    double errorThreshold,
    SimplificationResult& result)
{
    if (points.size() < 3) {
        result.originalCount = points.size();
        result.simplifiedCount = points.size();
        result.points.assign(points.begin(), points.end());
        return true;
    }

    result.originalCount = points.size();

    // Quadric Error 기반 점 선택
    std::vector<bool> keep(points.size(), false);
    keep[0] = true;  // 시작점
    keep[points.size() - 1] = true;  // 끝점

    std::vector<double> errors(points.size(), 0.0);

    // 각 점의 Quadric Error 계산
    for (size_t i = 1; i < points.size() - 1; i++) {
        Vertex* v = findVertex(points[i]);
        if (v) {
            errors[i] = quadricError(v);
        } else {
            errors[i] = 0.0;  // 정점 없으면 에러 0
        }
    }

    // Error가 임계값보다 큰 점 보존
    for (size_t i = 1; i < points.size() - 1; i++) {
        if (errors[i] > errorThreshold) {
            keep[i] = true;
        }
    }

    // 보존할 점만 추출
    result.points.clear();
    double totalError = 0.0;
    double maxErr = 0.0;

    for (size_t i = 0; i < points.size(); i++) {
        if (keep[i]) {
            result.points.push_back(points[i]);
            totalError += errors[i];
            maxErr = std::max(maxErr, errors[i]);
        }
    }

    result.simplifiedCount = result.points.size();
    result.compressionRatio = 100.0 * (1.0 - static_cast<double>(result.simplifiedCount) /
                                       static_cast<double>(result.originalCount));
    result.averageError = (result.simplifiedCount > 0) ? totalError / result.simplifiedCount : 0.0;
    result.maxError = maxErr;

    return true;
}

// ============================================================================
// 하이브리드: Douglas-Peucker + Quadric
// ============================================================================

bool QuadricOptimizer::simplifyHybrid(
    std::span<const DongArch::Math::Vec3> points,
    double epsilon,
    double errorThreshold,
    SimplificationResult& result)
{
    if (points.size() < 3) {
        result.originalCount = points.size();
        result.simplifiedCount = points.size();
        result.points.assign(points.begin(), points.end());
        return true;
    }

    result.originalCount = points.size();

    // 1단계: Douglas-Peucker로 기본 간략화
    SimplificationResult dpResult;
    if (!simplifyDouglasPeucker(points, epsilon, dpResult)) {
        return false;
    }

    // 2단계: 고곡률 영역 검출
    std::vector<size_t> highCurvature = detectHighCurvatureRegions(points, 30.0);  // 30도 이상

    // 3단계: Quadric Error 계산 및 추가
    std::vector<bool> keep(points.size(), false);

    // Douglas-Peucker 결과를 keep에 마킹
    for (const auto& p : dpResult.points) {
        for (size_t i = 0; i < points.size(); i++) {
            if (std::abs(p.x - points[i].x) < 0.001 &&
                std::abs(p.y - points[i].y) < 0.001 &&
                std::abs(p.z - points[i].z) < 0.001) {
                keep[i] = true;
                break;
            }
        }
    }

    // 고곡률 영역 추가
    for (size_t idx : highCurvature) {
        keep[idx] = true;
    }

    // Quadric Error 높은 점 추가
    for (size_t i = 1; i < points.size() - 1; i++) {
        if (!keep[i]) {
            Vertex* v = findVertex(points[i]);
            if (v && quadricError(v) > errorThreshold) {
                keep[i] = true;
            }
        }
    }

    // 결과 추출
    result.points.clear();
    for (size_t i = 0; i < points.size(); i++) {
        if (keep[i]) {
            result.points.push_back(points[i]);
        }
    }

    result.simplifiedCount = result.points.size();
    result.compressionRatio = 100.0 * (1.0 - static_cast<double>(result.simplifiedCount) /
                                       static_cast<double>(result.originalCount));
    result.averageError = dpResult.averageError;  // Douglas-Peucker 에러 사용
    result.maxError = dpResult.maxError;

    return true;
}

// ============================================================================
// 고곡률 영역 검출
// ============================================================================

double QuadricOptimizer::computeCurvature(
    const DongArch::Math::Vec3& prev,
    const DongArch::Math::Vec3& current,
    const DongArch::Math::Vec3& next)
{
    // 벡터 v1 = current - prev
    double v1x = current.x - prev.x;
    double v1y = current.y - prev.y;
    double v1z = current.z - prev.z;
    double len1 = std::sqrt(v1x * v1x + v1y * v1y + v1z * v1z);

    // 벡터 v2 = next - current
    double v2x = next.x - current.x;
    double v2y = next.y - current.y;
    double v2z = next.z - current.z;
    double len2 = std::sqrt(v2x * v2x + v2y * v2y + v2z * v2z);

    if (len1 < 1e-10 || len2 < 1e-10) {
        return 0.0;  // 벡터 길이가 0
    }

    // 정규화
    v1x /= len1; v1y /= len1; v1z /= len1;
    v2x /= len2; v2y /= len2; v2z /= len2;

    // Dot product
    double dotProduct = v1x * v2x + v1y * v2y + v1z * v2z;

    // 각도 계산 (라디안 → 도)
    double angle = std::acos(std::max(-1.0, std::min(1.0, dotProduct)));
    return angle * 180.0 / M_PI;  // 도 단위 변환
}

std::vector<size_t> QuadricOptimizer::detectHighCurvatureRegions(
    std::span<const DongArch::Math::Vec3> points,
    double curvatureThreshold) const
{
    std::vector<size_t> highCurvatureIndices;

    if (points.size() < 3) {
        return highCurvatureIndices;
    }

    // 각 점의 곡률 계산
    for (size_t i = 1; i < points.size() - 1; i++) {
        double curvature = computeCurvature(points[i - 1], points[i], points[i + 1]);

        if (curvature > curvatureThreshold) {
            highCurvatureIndices.push_back(i);
        }
    }

    return highCurvatureIndices;
}

} // namespace Algorithms
} // namespace Outline
} // namespace DongArch
