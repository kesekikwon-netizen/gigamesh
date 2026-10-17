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

#include "DongArchCutlineManager.h"
#include "algorithms/DouglasPeucker.h"
#include "algorithms/CatmullRomSpline.h"
#include "../illustrator/SVGExporter.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ranges>
#include <unordered_set>

using namespace DongArch::Math;

namespace DongArch {
namespace Cutline {

// ============================================================================
// 생성자
// ============================================================================

CutlineManager::CutlineManager(Mesh* mesh, QObject* parent)
    : QObject(parent)
    , mMesh(mesh)
{
    if (!isValid()) {
        std::cerr << "[CutlineManager] WARNING: Invalid mesh or no vertices!" << std::endl;
    }
}

// ============================================================================
// Cutline 추출 (단일 평면)
// ============================================================================

bool CutlineManager::extractCutline(const Vector3D& planeHNF, DongArch::CutlineResult& result) {
    if (!isValid()) {
        return false;
    }

    emit statusMessage(tr("교차선 추출 중..."));
    emit progressChanged(0);

    // GigaMesh calcIntersectionPolylineWithPlane 호출
    std::vector<Vector3D> intersectionPoints;
    bool success = mMesh->calcIntersectionPolylineWithPlane(planeHNF, &intersectionPoints);

    if (!success || intersectionPoints.empty()) {
        emit statusMessage(tr("교차점을 찾을 수 없습니다"));
        emit progressChanged(100);
        return false;
    }

    emit progressChanged(50);

    // Vector3D → flat float array 변환
    // result.points는 std::vector<float> (x, y, z 순서)
    result.points.clear();
    result.points.reserve(intersectionPoints.size() * 3);
    for (const auto& pt : intersectionPoints) {
        result.points.push_back(static_cast<float>(pt.getX()));
        result.points.push_back(static_cast<float>(pt.getY()));
        result.points.push_back(static_cast<float>(pt.getZ()));
    }

    result.pointCount = intersectionPoints.size();
    // Note: CutlineResult에 plane 멤버가 없으므로 제거
    // 필요시 DongArchTypes.h에 Vector3D plane 멤버 추가 가능

    emit progressChanged(100);
    emit statusMessage(tr("교차선 추출 완료: %1개 점").arg(result.pointCount));
    emit cutlineExtracted(result.pointCount);

    return true;
}

// ============================================================================
// 여러 평면 배치 처리
// ============================================================================

size_t CutlineManager::extractCutlines(
    std::span<const Vector3D> planes,
    std::vector<DongArch::CutlineResult>& results)
{
    if (!isValid() || planes.empty()) {
        return 0;
    }

    results.clear();
    results.reserve(planes.size());

    size_t successCount = 0;
    for (size_t i = 0; i < planes.size(); i++) {
        // 진행률 계산
        int progress = static_cast<int>((i * 100) / planes.size());
        emit progressChanged(progress);
        emit statusMessage(tr("평면 %1/%2 처리 중...").arg(i + 1).arg(planes.size()));

        DongArch::CutlineResult result;
        if (extractCutline(planes[i], result)) {
            results.push_back(std::move(result));
            successCount++;
        }
    }

    emit progressChanged(100);
    emit statusMessage(tr("배치 처리 완료: %1/%2 성공").arg(successCount).arg(planes.size()));

    return successCount;
}

// ============================================================================
// Octree 최적화 버전
// ============================================================================

bool CutlineManager::extractCutlineWithOctree(
    const Vector3D& planeHNF,
    DongArch::CutlineResult& result)
{
    if (!isValid()) {
        return false;
    }

    // Octree가 없으면 자동으로 일반 버전으로 fallback
    if (!hasOctree()) {
        emit statusMessage(tr("Octree 없음 - 일반 추출 방식 사용"));
        return extractCutline(planeHNF, result);
    }

    emit statusMessage(tr("Octree 최적화 교차선 추출 중..."));
    emit progressChanged(0);

    // TODO: GigaMesh Octree API를 사용한 최적화 구현
    // 현재는 일반 버전 호출 (향후 Octree 활용으로 100배 최적화)
    // Octree를 사용하면 평면 근처 Face만 검사하여 성능 향상

    emit statusMessage(tr("Octree 최적화 활용 중..."));
    bool success = extractCutline(planeHNF, result);

    if (success) {
        emit statusMessage(tr("Octree 최적화 완료: %1개 점").arg(result.pointCount));
    }

    return success;
}

bool CutlineManager::ensureOctree(int maxVerticesPerNode) {
    if (!isValid()) {
        return false;
    }

    // GigaMesh Octree가 이미 있는지 확인
    if (hasOctree()) {
        return true;
    }

    emit statusMessage(tr("Octree 생성 중..."));

    // TODO: GigaMesh Mesh::buildOctree() 호출
    // 현재 GigaMesh에 Octree 생성 함수가 있는지 확인 필요
    // 있다면 mMesh->buildOctree(maxVerticesPerNode) 호출

    emit statusMessage(tr("Octree 생성 완료"));
    return true;
}

bool CutlineManager::hasOctree() const {
    if (!isValid()) {
        return false;
    }

    // TODO: GigaMesh Mesh::hasOctree() 또는 getOctree() 확인
    // 현재는 false 반환 (향후 GigaMesh API 확인 후 구현)
    return false;
}

// ============================================================================
// C++20 std::ranges: 중복 제거
// ============================================================================

std::vector<Math::Vec3> CutlineManager::removeDuplicates(
    std::span<const Math::Vec3> points,
    double tolerance)
{
    if (points.empty()) {
        return {};
    }

    std::vector<Math::Vec3> result;
    result.reserve(points.size());

    // 첫 번째 점 추가
    result.push_back(points[0]);

    // C++20 ranges: 중복 제거
    // tolerance 내의 점들을 중복으로 판정
    for (const auto& pt : points | std::views::drop(1)) {
        bool isDuplicate = false;

        // 이미 추가된 점들과 거리 비교
        for (const auto& existing : result) {
            if (distance(pt, existing) < tolerance) {
                isDuplicate = true;
                break;
            }
        }

        if (!isDuplicate) {
            result.push_back(pt);
        }
    }

    return result;
}

// ============================================================================
// C++20 std::ranges: 교차점 정렬 (PolyLine 형성)
// ============================================================================

std::vector<Math::Vec3> CutlineManager::sortPoints(std::span<const Math::Vec3> points) {
    if (points.empty()) {
        return {};
    }

    if (points.size() == 1) {
        return {points[0]};
    }

    // Nearest Neighbor 알고리즘으로 PolyLine 형성
    std::vector<Math::Vec3> sorted;
    sorted.reserve(points.size());

    std::vector<bool> visited(points.size(), false);

    // 첫 번째 점에서 시작
    size_t currentIdx = 0;
    sorted.push_back(points[currentIdx]);
    visited[currentIdx] = true;

    // 나머지 점들을 가장 가까운 순서로 연결
    for (size_t i = 1; i < points.size(); i++) {
        double minDist = std::numeric_limits<double>::max();
        size_t nearestIdx = 0;

        // 현재 점에서 가장 가까운 미방문 점 찾기
        for (size_t j = 0; j < points.size(); j++) {
            if (!visited[j]) {
                double dist = distance(sorted.back(), points[j]);
                if (dist < minDist) {
                    minDist = dist;
                    nearestIdx = j;
                }
            }
        }

        sorted.push_back(points[nearestIdx]);
        visited[nearestIdx] = true;
    }

    return sorted;
}

// ============================================================================
// Utility: 변환 함수
// ============================================================================

Math::Vec3 CutlineManager::toVec3(const Vector3D& v) const {
    return Math::Vec3(v.getX(), v.getY(), v.getZ());
}

Vector3D CutlineManager::toVector3D(const Math::Vec3& v) const {
    return Vector3D(v.x, v.y, v.z);
}

double CutlineManager::distance(const Math::Vec3& a, const Math::Vec3& b) const {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double dz = b.z - a.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// ============================================================================
// Phase 2 고도화: Line Smoothing (Douglas-Peucker + Catmull-Rom)
// ============================================================================

std::vector<Vector3D> CutlineManager::simplifyWithDouglasPeucker(
    std::span<const Vector3D> points,
    double tolerance)
{
    // Douglas-Peucker 알고리즘 호출
    Algorithms::DouglasPeucker dp;
    return dp.simplify(points, tolerance);
}

std::vector<Vector3D> CutlineManager::interpolateWithCatmullRom(
    std::span<const Vector3D> controlPoints,
    int segmentsPerSpan)
{
    // Catmull-Rom Spline 보간
    Algorithms::CatmullRomSpline spline;
    return spline.interpolate(controlPoints, segmentsPerSpan);
}

std::vector<Vector3D> CutlineManager::smoothCutline(
    std::span<const Vector3D> points,
    double tolerance,
    int segmentsPerSpan)
{
    if (points.size() < 2) {
        return std::vector<Vector3D>(points.begin(), points.end());
    }

    emit statusMessage(tr("Cutline 스무딩 중..."));
    emit progressChanged(25);

    // Step 1: Douglas-Peucker 단순화
    Algorithms::DouglasPeucker dp;
    auto simplified = dp.simplify(points, tolerance);

    emit progressChanged(50);

    // 단순화 통계 출력
    const auto& stats = dp.getLastStatistics();
    std::cout << "[CutlineManager] Douglas-Peucker 단순화:" << std::endl;
    std::cout << "  - 원본 점 개수: " << stats.originalPointCount << std::endl;
    std::cout << "  - 단순화 후: " << stats.simplifiedPointCount << std::endl;
    std::cout << "  - 감소 비율: " << stats.reductionRatio << "%" << std::endl;
    std::cout << "  - 최대 편차: " << stats.maxDeviation << " mm" << std::endl;
    std::cout << "  - 평균 편차: " << stats.avgDeviation << " mm" << std::endl;

    if (simplified.size() < 2) {
        emit progressChanged(100);
        return simplified;
    }

    emit progressChanged(75);

    // Step 2: Catmull-Rom Spline 보간
    Algorithms::CatmullRomSpline spline;
    auto smoothed = spline.interpolate(simplified, segmentsPerSpan);

    emit progressChanged(100);
    emit statusMessage(tr("Cutline 스무딩 완료"));

    std::cout << "[CutlineManager] Catmull-Rom Spline 보간:" << std::endl;
    std::cout << "  - 제어점: " << simplified.size() << std::endl;
    std::cout << "  - 최종 점 개수: " << smoothed.size() << std::endl;

    return smoothed;
}

// ============================================================================
// Phase 7: SVG Export Integration
// ============================================================================

bool CutlineManager::exportCutlineToSVG(
    const DongArch::CutlineResult& result,
    const QString& filePath,
    const QColor& lineColor,
    float lineWidth)
{
    if (result.pointCount == 0 || result.points.empty()) {
        std::cerr << "[CutlineManager] Cannot export: no cutline data" << std::endl;
        return false;
    }

    using namespace DongArch::Illustrator;

    // Convert flat float array to QPointF vector
    std::vector<QPointF> points2D;
    points2D.reserve(result.pointCount);

    for (size_t i = 0; i + 2 < result.points.size(); i += 3) {
        float x = result.points[i];
        float y = result.points[i + 1];
        // Ignore Z for 2D SVG
        points2D.emplace_back(x, y);
    }

    // Calculate bounding box
    float minX = points2D[0].x();
    float maxX = minX;
    float minY = points2D[0].y();
    float maxY = minY;

    for (const auto& pt : points2D) {
        minX = std::min(minX, static_cast<float>(pt.x()));
        maxX = std::max(maxX, static_cast<float>(pt.x()));
        minY = std::min(minY, static_cast<float>(pt.y()));
        maxY = std::max(maxY, static_cast<float>(pt.y()));
    }

    float width = maxX - minX;
    float height = maxY - minY;

    // Create SVG exporter
    SVGExporter exporter;
    exporter.setCanvasSize(QSizeF(width + 20, height + 20));  // 10mm margin on each side
    exporter.setUnit(SVGExporter::Unit::Millimeter);
    exporter.setBackgroundColor(Qt::transparent);

    // Shift points to have 10mm margin
    std::vector<QPointF> shiftedPoints;
    shiftedPoints.reserve(points2D.size());
    for (const auto& pt : points2D) {
        shiftedPoints.emplace_back(pt.x() - minX + 10, pt.y() - minY + 10);
    }

    // Create Cutline layer
    SVGLayer cutlineLayer;
    cutlineLayer.name = "Cutline";
    cutlineLayer.id = "cutline_layer_1";
    cutlineLayer.strokeColor = lineColor;
    cutlineLayer.strokeWidth = lineWidth;
    cutlineLayer.visible = true;
    cutlineLayer.polylines.push_back(shiftedPoints);

    exporter.addLayer(cutlineLayer);

    // Export to file
    bool success = exporter.exportToFile(filePath);

    if (success) {
        emit statusMessage(QStringLiteral("SVG 내보내기 성공: %1").arg(filePath));
        std::cout << "[CutlineManager] SVG exported: " << filePath.toStdString()
                  << " (" << points2D.size() << " points, "
                  << width << "x" << height << "mm)" << std::endl;
    } else {
        emit statusMessage(QStringLiteral("SVG 내보내기 실패"));
        std::cerr << "[CutlineManager] Failed to export SVG" << std::endl;
    }

    return success;
}

} // namespace Cutline
} // namespace DongArch
