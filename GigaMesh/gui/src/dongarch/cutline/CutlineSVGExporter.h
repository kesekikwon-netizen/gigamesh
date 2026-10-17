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

#ifndef DONGARCH_CUTLINE_SVG_EXPORTER_H
#define DONGARCH_CUTLINE_SVG_EXPORTER_H

#include "CutlineParams.h"
#include <GigaMesh/mesh/vector3d.h>
#include <QString>
#include <QPointF>
#include <vector>

namespace DongArch {
namespace Cutline {

//! SVG Export 결과
struct SVGExportResult {
    bool success = false;              //!< 성공 여부
    QString filePath;                  //!< 저장된 파일 경로
    QString errorMessage;              //!< 실패 시 에러 메시지
    int layersExported = 0;            //!< 내보낸 레이어 수
};

//! Cutline SVG Exporter
//!
//! **기능**:
//! - 3D polyline → 2D SVG 변환
//! - 1:1 스케일 유지 (mm 단위)
//! - 멀티 슬롯 → 레이어 분리
//! - Adobe Illustrator 호환 (SVG 1.1)
//! - 메타데이터 포함 (날짜, 파라미터, DongArch3D 버전)
//!
//! **사용 예**:
//! ```cpp
//! CutlineSVGExporter exporter;
//!
//! // 단일 슬롯 export
//! SVGExportResult result = exporter.exportSingleSlot(
//!     slot,
//!     "/path/to/output.svg"
//! );
//!
//! // 멀티 슬롯 export (레이어 분리)
//! std::vector<CutlineSlot> slots = {...};
//! SVGExportResult result = exporter.exportMultipleSlots(
//!     slots,
//!     "/path/to/output.svg"
//! );
//! ```
class CutlineSVGExporter {
public:
    CutlineSVGExporter() = default;
    ~CutlineSVGExporter() = default;

    //! 단일 슬롯을 SVG로 내보내기
    //! \param slot Cutline 슬롯
    //! \param filePath 저장 경로 (*.svg)
    //! \return Export 결과
    SVGExportResult exportSingleSlot(
        const CutlineSlot& slot,
        const QString& filePath
    );

    //! 여러 슬롯을 레이어로 분리하여 SVG로 내보내기
    //! \param slotList Cutline 슬롯 리스트
    //! \param filePath 저장 경로 (*.svg)
    //! \return Export 결과
    SVGExportResult exportMultipleSlots(
        const std::vector<CutlineSlot>& slotList,
        const QString& filePath
    );

    //! 스케일 인자 설정 (기본: 1.0 = 1mm/unit)
    void setScaleFactor(double scale) { mScaleFactor = scale; }

    //! 스케일 인자 가져오기
    double getScaleFactor() const { return mScaleFactor; }

private:
    //! 3D Vector3D → 2D QPointF 투영
    //! \param point 3D 좌표
    //! \param mode 평면 모드
    //! \return 2D 좌표 (mm 단위)
    QPointF projectTo2D(const Vector3D& point, PlaneMode mode) const;

    //! Polyline → SVG <path> 요소
    //! \param polyline 3D polyline
    //! \param mode 평면 모드
    //! \param color 라인 색상
    //! \param width 라인 두께 (mm)
    //! \param opacity 불투명도 (0.0 ~ 1.0)
    //! \return SVG path 문자열
    QString polylineToSVGPath(
        const std::vector<Vector3D>& polyline,
        PlaneMode mode,
        const QColor& color,
        float width,
        float opacity
    ) const;

    //! SVG 헤더 생성
    //! \param width SVG 너비 (mm)
    //! \param height SVG 높이 (mm)
    //! \return SVG 헤더 문자열
    QString generateSVGHeader(double width, double height) const;

    //! SVG 푸터 생성
    QString generateSVGFooter() const;

    //! 메타데이터 생성 (SVG <desc> 요소)
    //! \param slot Cutline 슬롯
    //! \return SVG desc 문자열
    QString generateMetadata(const CutlineSlot& slot) const;

    //! Bounding Box 계산 (2D)
    //! \param polyline 3D polyline
    //! \param mode 평면 모드
    //! \param[out] minX 최소 X
    //! \param[out] minY 최소 Y
    //! \param[out] maxX 최대 X
    //! \param[out] maxY 최대 Y
    void calculateBoundingBox2D(
        const std::vector<Vector3D>& polyline,
        PlaneMode mode,
        double& minX, double& minY,
        double& maxX, double& maxY
    ) const;

    //! 여러 슬롯의 전체 Bounding Box 계산
    void calculateCombinedBoundingBox(
        const std::vector<CutlineSlot>& slotList,
        double& minX, double& minY,
        double& maxX, double& maxY
    ) const;

    //! QColor → SVG 색상 문자열 변환
    QString colorToSVGString(const QColor& color) const;

    double mScaleFactor = 1.0;  //!< 스케일 인자 (1.0 = 1mm/unit)
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_SVG_EXPORTER_H
