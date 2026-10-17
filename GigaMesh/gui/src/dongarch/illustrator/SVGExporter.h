/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 7: SVG Exporter for Illustrator
 *
 * SVG Export:
 * - Cutline 및 Outline을 SVG 파일로 내보내기
 * - 레이어 구조 (Cutline, Outline 별도)
 * - Illustrator 호환 (CS6+)
 * - 메타데이터 포함 (스케일, 단위)
 *
 * This file is part of DongArch3D (based on GigaMesh).
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_SVG_EXPORTER_H
#define DONGARCH_SVG_EXPORTER_H

#include <QString>
#include <QSizeF>
#include <QColor>
#include <QPointF>
#include <QVector3D>
#include <QMatrix4x4>
#include <vector>
#include <memory>

namespace DongArch {
namespace Illustrator {

//! \struct SVGLayer
//! \brief SVG 레이어 정보
struct SVGLayer
{
    QString name;                       //!< 레이어 이름
    QString id;                         //!< 레이어 ID
    std::vector<std::vector<QPointF>> polylines;  //!< 폴리라인들
    QColor strokeColor;                 //!< 선 색상
    float strokeWidth;                  //!< 선 두께 (mm)
    bool visible;                       //!< 가시성
};

//! \class SVGExporter
//! \brief SVG 파일 내보내기
//!
//! 기능:
//! 1. Cutline/Outline을 SVG로 변환
//! 2. 레이어별 그룹화
//! 3. 스케일/단위 메타데이터
//! 4. Illustrator 호환성
//!
//! 사용법:
//! ```cpp
//! SVGExporter exporter;
//! exporter.setCanvasSize(QSizeF(200, 200));  // mm
//! exporter.setUnit(SVGExporter::Unit::Millimeter);
//!
//! // Add Cutline layer
//! SVGLayer cutlineLayer;
//! cutlineLayer.name = "Cutline";
//! cutlineLayer.strokeColor = Qt::red;
//! cutlineLayer.strokeWidth = 0.5f;
//! cutlineLayer.polylines = { ... };
//! exporter.addLayer(cutlineLayer);
//!
//! // Export
//! exporter.exportToFile("/path/to/output.svg");
//! ```
class SVGExporter
{
public:
    //! SVG 단위
    enum class Unit {
        Millimeter,  //!< mm (기본값)
        Centimeter,  //!< cm
        Pixel        //!< px (96 DPI)
    };

    //! SVG 버전
    enum class Version {
        SVG_1_1,     //!< SVG 1.1 (Illustrator CS6+)
        SVG_2_0      //!< SVG 2.0 (최신)
    };

    SVGExporter();
    ~SVGExporter();

    //! 캔버스 크기 설정
    //! \param size 크기 (단위: mm)
    void setCanvasSize(const QSizeF& size);

    //! 단위 설정
    //! \param unit SVG 단위
    void setUnit(Unit unit);

    //! SVG 버전 설정
    //! \param version SVG 버전
    void setVersion(Version version);

    //! 배경색 설정
    //! \param color 배경색 (투명하려면 Qt::transparent)
    void setBackgroundColor(const QColor& color);

    //! 레이어 추가
    //! \param layer 레이어
    void addLayer(const SVGLayer& layer);

    //! 레이어 초기화
    void clearLayers();

    //! SVG 파일로 내보내기
    //! \param filePath 파일 경로
    //! \return 성공 여부
    bool exportToFile(const QString& filePath);

    //! SVG 문자열로 내보내기
    //! \return SVG XML 문자열
    QString exportToString();

    //! 2D 점 변환 (3D -> 2D 투영)
    //! \param point3D 3D 점 (X, Y, Z)
    //! \param viewMatrix 뷰 행렬 (3D -> 2D)
    //! \return 2D 점
    static QPointF project3DTo2D(const QVector3D& point3D, const QMatrix4x4& viewMatrix);

    //! 단위 문자열로 변환
    //! \param unit 단위
    //! \return 단위 문자열 (예: "mm", "cm", "px")
    static QString unitToString(Unit unit);

private:
    //! SVG 헤더 생성
    //! \return SVG 헤더 문자열
    QString generateSVGHeader();

    //! SVG 레이어 생성
    //! \param layer 레이어
    //! \return SVG 레이어 문자열
    QString generateSVGLayer(const SVGLayer& layer);

    //! SVG 폴리라인 생성
    //! \param polyline 폴리라인
    //! \param strokeColor 선 색상
    //! \param strokeWidth 선 두께
    //! \return SVG 폴리라인 문자열
    QString generateSVGPolyline(const std::vector<QPointF>& polyline,
                                const QColor& strokeColor,
                                float strokeWidth);

    //! SVG 푸터 생성
    //! \return SVG 푸터 문자열
    QString generateSVGFooter();

    QSizeF mCanvasSize;                     //!< 캔버스 크기 (mm)
    Unit mUnit;                             //!< SVG 단위
    Version mVersion;                       //!< SVG 버전
    QColor mBackgroundColor;                //!< 배경색
    std::vector<SVGLayer> mLayers;          //!< 레이어들
};

} // namespace Illustrator
} // namespace DongArch

#endif // DONGARCH_SVG_EXPORTER_H
