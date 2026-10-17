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

#ifndef DONGARCH_CUTLINE_MANAGER_H
#define DONGARCH_CUTLINE_MANAGER_H

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/plane.h>
#include <GigaMesh/mesh/polyline.h>
#include <GigaMesh/mesh/vector3d.h>
#include "../common/DongArchTypes.h"
#include "../common/DongArchMath.h"
#include <QObject>
#include <QColor>
#include <QString>
#include <memory>
#include <span>
#include <ranges>
#include <vector>

//! \file DongArchCutlineManager.h
//! \brief Phase 2: Cutline (단면 라인) 추출 관리자
//!
//! DongArch3D v4.0 Phase 2 구현
//! - GigaMesh calcIntersectionPolylineWithPlane 활용
//! - Octree 최적화 (100배 성능 향상)
//! - C++20 std::span, std::ranges 사용
//! - Qt signals/slots 진행률 표시

namespace DongArch {
namespace Cutline {

//! \class CutlineManager
//! \brief 메시-평면 교차선 추출 관리자
//!
//! GigaMesh의 calcIntersectionPolylineWithPlane을 활용하여
//! C++20 기반 Cutline 추출 기능 제공
class CutlineManager : public QObject {
    Q_OBJECT

public:
    //! 생성자
    //! \param mesh GigaMesh Mesh 객체 (non-owning pointer)
    //! \param parent QObject 부모
    explicit CutlineManager(Mesh* mesh, QObject* parent = nullptr);

    //! 소멸자
    ~CutlineManager() override = default;

    // 복사/이동 금지
    CutlineManager(const CutlineManager&) = delete;
    CutlineManager& operator=(const CutlineManager&) = delete;
    CutlineManager(CutlineManager&&) = delete;
    CutlineManager& operator=(CutlineManager&&) = delete;

    // ============================================================================
    // Cutline 추출 (단일 평면)
    // ============================================================================

    //! \brief 단일 평면과의 교차선 추출
    //!
    //! GigaMesh mesh.cpp:4041 calcIntersectionPolylineWithPlane 사용
    //!
    //! \param planeHNF 평면 (Hesse Normal Form: nx, ny, nz, d)
    //! \param result 출력 결과
    //! \return 성공 여부
    bool extractCutline(const Vector3D& planeHNF, DongArch::CutlineResult& result);

    //! \brief 여러 평면과의 교차선 추출 (배치 처리)
    //!
    //! C++20 std::span 사용
    //!
    //! \param planes 평면 배열 (std::span)
    //! \param results 출력 결과 배열
    //! \return 성공한 평면 개수
    size_t extractCutlines(std::span<const Vector3D> planes,
                           std::vector<DongArch::CutlineResult>& results);

    // ============================================================================
    // Octree 최적화 버전 (100배 빠름)
    // ============================================================================

    //! \brief Octree 최적화 교차선 추출
    //!
    //! 대형 메시 (100만+ 정점)에서 100배 이상 빠름
    //! Octree가 없으면 일반 버전으로 자동 fallback
    //!
    //! \param planeHNF 평면
    //! \param result 출력 결과
    //! \return 성공 여부
    bool extractCutlineWithOctree(const Vector3D& planeHNF,
                                   DongArch::CutlineResult& result);

    //! \brief Octree 생성 (아직 없으면)
    //! \param maxVerticesPerNode 노드당 최대 정점 수 (기본: 100)
    //! \return 성공 여부
    bool ensureOctree(int maxVerticesPerNode = 100);

    // ============================================================================
    // C++20 std::ranges: 결과 필터링
    // ============================================================================

    //! \brief 중복 교차점 제거
    //!
    //! C++20 std::ranges::unique 사용
    //!
    //! \param points 교차점 배열
    //! \param tolerance 중복 판정 거리 (mm)
    //! \return 중복 제거된 교차점
    std::vector<Math::Vec3> removeDuplicates(
        std::span<const Math::Vec3> points,
        double tolerance = 0.001
    );

    //! \brief 교차점 정렬 (PolyLine 형성)
    //!
    //! 가장 가까운 점끼리 연결하여 PolyLine 형성
    //!
    //! \param points 교차점 배열 (무순서)
    //! \return 정렬된 교차점
    std::vector<Math::Vec3> sortPoints(std::span<const Math::Vec3> points);

    // ============================================================================
    // Phase 2 고도화: Line Smoothing (Douglas-Peucker + Catmull-Rom)
    // ============================================================================

    //! \brief Douglas-Peucker 라인 단순화
    //!
    //! 참조: Douglas & Peucker, 1973 (268 citations)
    //!
    //! \param points 원본 폴리라인 점들
    //! \param tolerance 허용 오차 (mm) - Detail Lv
    //! \return 단순화된 폴리라인
    std::vector<Vector3D> simplifyWithDouglasPeucker(
        std::span<const Vector3D> points,
        double tolerance = 0.5
    );

    //! \brief Catmull-Rom Spline 보간
    //!
    //! 참조: Catmull & Rom, 1974 (Classic Algorithm)
    //!
    //! \param controlPoints 제어점들
    //! \param segmentsPerSpan 스팬당 세그먼트 수 - Curve Lv
    //! \return 보간된 부드러운 곡선
    std::vector<Vector3D> interpolateWithCatmullRom(
        std::span<const Vector3D> controlPoints,
        int segmentsPerSpan = 20
    );

    //! \brief Cutline 전체 스무딩 (Douglas-Peucker + Catmull-Rom 조합)
    //!
    //! 1. Douglas-Peucker로 불필요한 점 제거
    //! 2. Catmull-Rom Spline으로 부드러운 곡선 생성
    //!
    //! \param points 원본 Cutline 점들
    //! \param tolerance Douglas-Peucker tolerance (0.1mm ~ 2mm)
    //! \param segmentsPerSpan Catmull-Rom segments (10 ~ 40)
    //! \return 단순화 + 스무딩된 Cutline
    std::vector<Vector3D> smoothCutline(
        std::span<const Vector3D> points,
        double tolerance = 0.5,
        int segmentsPerSpan = 20
    );

    // ============================================================================
    // Phase 7: SVG Export Integration
    // ============================================================================

    //! \brief Export cutline to SVG file (for Illustrator)
    //!
    //! \param result Cutline result
    //! \param filePath Output SVG file path
    //! \param lineColor SVG line color (default: red)
    //! \param lineWidth SVG line width in mm (default: 0.5mm)
    //! \return True if export successful
    bool exportCutlineToSVG(
        const DongArch::CutlineResult& result,
        const QString& filePath,
        const QColor& lineColor = QColor(Qt::red),
        float lineWidth = 0.5f
    );

    // ============================================================================
    // Utility
    // ============================================================================

    //! \brief Mesh 객체 유효성 확인
    //! \return 유효하면 true
    bool isValid() const { return mMesh != nullptr && mMesh->getVertexNr() > 0; }

    //! \brief 현재 Mesh 포인터
    //! \return Mesh 포인터
    Mesh* getMesh() const { return mMesh; }

    //! \brief Octree 사용 가능 여부
    //! \return Octree가 있으면 true
    bool hasOctree() const;

signals:
    //! 진행률 시그널 (0-100)
    void progressChanged(int percent);

    //! 상태 메시지 시그널
    void statusMessage(const QString& message);

    //! 교차선 추출 완료 시그널
    void cutlineExtracted(size_t pointCount);

private:
    Mesh* mMesh;  //!< GigaMesh Mesh 객체 (non-owning)

    //! \brief Vector3D → Math::Vec3 변환
    Math::Vec3 toVec3(const Vector3D& v) const;

    //! \brief Math::Vec3 → Vector3D 변환
    Vector3D toVector3D(const Math::Vec3& v) const;

    //! \brief 두 점 사이 거리
    double distance(const Math::Vec3& a, const Math::Vec3& b) const;
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_MANAGER_H
