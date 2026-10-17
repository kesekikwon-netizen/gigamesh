/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "CutlineSVGExporter.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QFileInfo>
#include <cmath>
#include <limits>

namespace DongArch {
namespace Cutline {

// ============================================================================
// Public API
// ============================================================================

SVGExportResult CutlineSVGExporter::exportSingleSlot(
    const CutlineSlot& slot,
    const QString& filePath
) {
    SVGExportResult result;
    result.filePath = filePath;

    // 유효성 검사
    if (!slot.isValid()) {
        result.success = false;
        result.errorMessage = "Invalid slot: empty polyline";
        return result;
    }

    if (slot.polyline.empty()) {
        result.success = false;
        result.errorMessage = "Polyline is empty";
        return result;
    }

    // Bounding Box 계산
    double minX, minY, maxX, maxY;
    calculateBoundingBox2D(slot.polyline, slot.params.mode, minX, minY, maxX, maxY);

    double width = maxX - minX;
    double height = maxY - minY;
    double margin = 10.0; // 10mm 여백

    // SVG 문서 생성
    QString svg;
    svg += generateSVGHeader(width + 2 * margin, height + 2 * margin);
    svg += generateMetadata(slot);

    // Transform: (minX, minY)를 원점으로 이동 + 여백 추가
    svg += QString("  <g id=\"layer-%1\" transform=\"translate(%2, %3)\">\n")
        .arg(slot.id)
        .arg(margin - minX)
        .arg(margin - minY);

    // Path 생성
    svg += polylineToSVGPath(
        slot.polyline,
        slot.params.mode,
        slot.params.color,
        slot.params.width,
        slot.params.opacity
    );

    svg += "  </g>\n";
    svg += generateSVGFooter();

    // 파일 저장
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot open file: %1").arg(filePath);
        return result;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << svg;
    file.close();

    result.success = true;
    result.layersExported = 1;
    return result;
}

SVGExportResult CutlineSVGExporter::exportMultipleSlots(
    const std::vector<CutlineSlot>& slotList,
    const QString& filePath
) {
    SVGExportResult result;
    result.filePath = filePath;

    // 유효한 슬롯만 필터링
    std::vector<CutlineSlot> validSlots;
    for (const auto& slot : slotList) {
        if (slot.isValid() && !slot.polyline.empty()) {
            validSlots.push_back(slot);
        }
    }

    if (validSlots.empty()) {
        result.success = false;
        result.errorMessage = "No valid slots to export";
        return result;
    }

    // 전체 Bounding Box 계산
    double minX, minY, maxX, maxY;
    calculateCombinedBoundingBox(validSlots, minX, minY, maxX, maxY);

    double width = maxX - minX;
    double height = maxY - minY;
    double margin = 10.0; // 10mm 여백

    // SVG 문서 생성
    QString svg;
    svg += generateSVGHeader(width + 2 * margin, height + 2 * margin);

    // 메타데이터 (멀티 레이어)
    svg += "  <desc>\n";
    svg += "    DongArch3D Cutline Export (Multi-layer)\n";
    svg += QString("    Exported: %1\n").arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    svg += QString("    Layers: %1\n").arg(validSlots.size());
    svg += "  </desc>\n";

    // 각 슬롯을 레이어로 분리
    for (const auto& slot : validSlots) {
        svg += QString("  <g id=\"layer-%1\" inkscape:label=\"%2\">\n")
            .arg(slot.id)
            .arg(slot.name.isEmpty() ? QString("Slot %1").arg(slot.id) : slot.name);

        // Transform
        svg += QString("    <g transform=\"translate(%1, %2)\">\n")
            .arg(margin - minX)
            .arg(margin - minY);

        // Path 생성
        svg += polylineToSVGPath(
            slot.polyline,
            slot.params.mode,
            slot.params.color,
            slot.params.width,
            slot.params.opacity
        );

        svg += "    </g>\n";
        svg += "  </g>\n";
    }

    svg += generateSVGFooter();

    // 파일 저장
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.success = false;
        result.errorMessage = QString("Cannot open file: %1").arg(filePath);
        return result;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << svg;
    file.close();

    result.success = true;
    result.layersExported = static_cast<int>(validSlots.size());
    return result;
}

// ============================================================================
// 3D → 2D Projection
// ============================================================================

QPointF CutlineSVGExporter::projectTo2D(const Vector3D& point, PlaneMode mode) const {
    double x, y;

    switch (mode) {
        case PlaneMode::TOP:
            // XY 평면 (Z축 무시) - 평면도
            x = point.getX();
            y = point.getY();
            break;

        case PlaneMode::FRONT:
            // XZ 평면 (Y축 무시) - 정면도
            x = point.getX();
            y = point.getZ();
            break;

        case PlaneMode::RIGHT:
            // YZ 평면 (X축 무시) - 측면도
            x = point.getY();
            y = point.getZ();
            break;

        case PlaneMode::CUSTOM:
        default:
            // Custom 모드는 TOP과 동일하게 처리 (폴백)
            x = point.getX();
            y = point.getY();
            break;
    }

    // 스케일 적용 (1.0 = 1mm/unit)
    return QPointF(x * mScaleFactor, y * mScaleFactor);
}

// ============================================================================
// SVG 생성
// ============================================================================

QString CutlineSVGExporter::polylineToSVGPath(
    const std::vector<Vector3D>& polyline,
    PlaneMode mode,
    const QColor& color,
    float width,
    float opacity
) const {
    if (polyline.empty()) {
        return QString();
    }

    // Path data 생성
    QString pathData;
    bool first = true;

    for (const auto& point3d : polyline) {
        QPointF point2d = projectTo2D(point3d, mode);

        if (first) {
            pathData += QString("M %1 %2 ").arg(point2d.x(), 0, 'f', 3).arg(point2d.y(), 0, 'f', 3);
            first = false;
        } else {
            pathData += QString("L %1 %2 ").arg(point2d.x(), 0, 'f', 3).arg(point2d.y(), 0, 'f', 3);
        }
    }

    // SVG path 요소
    QString path = QString("    <path d=\"%1\" "
                          "stroke=\"%2\" "
                          "stroke-width=\"%3\" "
                          "stroke-opacity=\"%4\" "
                          "fill=\"none\" "
                          "stroke-linecap=\"round\" "
                          "stroke-linejoin=\"round\" />\n")
        .arg(pathData.trimmed())
        .arg(colorToSVGString(color))
        .arg(width, 0, 'f', 2)
        .arg(opacity, 0, 'f', 2);

    return path;
}

QString CutlineSVGExporter::generateSVGHeader(double width, double height) const {
    QString header;
    header += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
    header += QString("<svg width=\"%1mm\" height=\"%2mm\" "
                     "viewBox=\"0 0 %1 %2\" "
                     "xmlns=\"http://www.w3.org/2000/svg\" "
                     "xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" "
                     "version=\"1.1\">\n")
        .arg(width, 0, 'f', 3)
        .arg(height, 0, 'f', 3);

    return header;
}

QString CutlineSVGExporter::generateSVGFooter() const {
    return "</svg>\n";
}

QString CutlineSVGExporter::generateMetadata(const CutlineSlot& slot) const {
    QString meta;
    meta += "  <desc>\n";
    meta += "    DongArch3D Cutline Export\n";
    meta += QString("    Slot: %1\n").arg(slot.id);
    meta += QString("    Name: %1\n").arg(slot.name.isEmpty() ? "Unnamed" : slot.name);
    meta += QString("    Mode: %1\n").arg(slot.params.modeName());
    meta += QString("    Offset: %1\n").arg(slot.params.offset, 0, 'f', 3);
    meta += QString("    Epsilon: %1\n").arg(slot.params.epsilon, 0, 'f', 4);
    meta += QString("    Spline Tension: %1\n").arg(slot.params.splineTension, 0, 'f', 2);
    meta += QString("    Points: %1\n").arg(slot.polyline.size());
    meta += QString("    Timestamp: %1\n").arg(slot.timestamp.toString(Qt::ISODate));
    meta += QString("    Exported: %1\n").arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    meta += "    Software: DongArch3D v4.0\n";
    meta += "  </desc>\n";

    return meta;
}

// ============================================================================
// Bounding Box 계산
// ============================================================================

void CutlineSVGExporter::calculateBoundingBox2D(
    const std::vector<Vector3D>& polyline,
    PlaneMode mode,
    double& minX, double& minY,
    double& maxX, double& maxY
) const {
    minX = minY = std::numeric_limits<double>::max();
    maxX = maxY = std::numeric_limits<double>::lowest();

    for (const auto& point3d : polyline) {
        QPointF point2d = projectTo2D(point3d, mode);
        minX = std::min(minX, point2d.x());
        minY = std::min(minY, point2d.y());
        maxX = std::max(maxX, point2d.x());
        maxY = std::max(maxY, point2d.y());
    }
}

void CutlineSVGExporter::calculateCombinedBoundingBox(
    const std::vector<CutlineSlot>& slotList,
    double& minX, double& minY,
    double& maxX, double& maxY
) const {
    minX = minY = std::numeric_limits<double>::max();
    maxX = maxY = std::numeric_limits<double>::lowest();

    for (const auto& slot : slotList) {
        if (!slot.isValid() || slot.polyline.empty()) {
            continue;
        }

        double slotMinX, slotMinY, slotMaxX, slotMaxY;
        calculateBoundingBox2D(slot.polyline, slot.params.mode, slotMinX, slotMinY, slotMaxX, slotMaxY);

        minX = std::min(minX, slotMinX);
        minY = std::min(minY, slotMinY);
        maxX = std::max(maxX, slotMaxX);
        maxY = std::max(maxY, slotMaxY);
    }
}

// ============================================================================
// Utility
// ============================================================================

QString CutlineSVGExporter::colorToSVGString(const QColor& color) const {
    return QString("rgb(%1,%2,%3)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue());
}

} // namespace Cutline
} // namespace DongArch
