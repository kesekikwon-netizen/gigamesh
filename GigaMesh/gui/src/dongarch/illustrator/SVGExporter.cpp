/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 7: SVG Exporter Implementation
 */

#include "SVGExporter.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector4D>

namespace DongArch {
namespace Illustrator {

//==============================================================================
// Constructor / Destructor
//==============================================================================

SVGExporter::SVGExporter()
    : mCanvasSize(200.0, 200.0)  // Default: 200x200mm
    , mUnit(Unit::Millimeter)
    , mVersion(Version::SVG_1_1)
    , mBackgroundColor(Qt::white)
{
}

SVGExporter::~SVGExporter()
{
}

//==============================================================================
// Settings
//==============================================================================

void SVGExporter::setCanvasSize(const QSizeF& size)
{
    mCanvasSize = size;
}

void SVGExporter::setUnit(Unit unit)
{
    mUnit = unit;
}

void SVGExporter::setVersion(Version version)
{
    mVersion = version;
}

void SVGExporter::setBackgroundColor(const QColor& color)
{
    mBackgroundColor = color;
}

//==============================================================================
// Layer Management
//==============================================================================

void SVGExporter::addLayer(const SVGLayer& layer)
{
    mLayers.push_back(layer);
}

void SVGExporter::clearLayers()
{
    mLayers.clear();
}

//==============================================================================
// Export
//==============================================================================

bool SVGExporter::exportToFile(const QString& filePath)
{
    QString svgContent = exportToString();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Failed to open file for writing:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream << svgContent;

    file.close();

    qDebug() << "SVG exported successfully to" << filePath;
    return true;
}

QString SVGExporter::exportToString()
{
    QString svg;

    svg += generateSVGHeader();

    for (const auto& layer : mLayers) {
        svg += generateSVGLayer(layer);
    }

    svg += generateSVGFooter();

    return svg;
}

//==============================================================================
// SVG Generation
//==============================================================================

QString SVGExporter::generateSVGHeader()
{
    QString header;

    // XML declaration
    header += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";

    // SVG version
    if (mVersion == Version::SVG_1_1) {
        header += "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
                  "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
    }

    // SVG element
    QString unitStr = unitToString(mUnit);

    header += QString("<svg width=\"%1%2\" height=\"%3%4\" "
                      "viewBox=\"0 0 %1 %3\" "
                      "xmlns=\"http://www.w3.org/2000/svg\" "
                      "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                      "version=\"%5\">\n")
                  .arg(mCanvasSize.width())
                  .arg(unitStr)
                  .arg(mCanvasSize.height())
                  .arg(unitStr)
                  .arg(mVersion == Version::SVG_1_1 ? "1.1" : "2.0");

    // Metadata
    header += "  <metadata>\n";
    header += "    <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" "
              "xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n";
    header += "      <rdf:Description>\n";
    header += "        <dc:creator>DongArch3D v4.0</dc:creator>\n";
    header += "        <dc:title>Archaeological 3D Measurement Export</dc:title>\n";
    header += "        <dc:format>image/svg+xml</dc:format>\n";
    header += "      </rdf:Description>\n";
    header += "    </rdf:RDF>\n";
    header += "  </metadata>\n";

    // Background
    if (mBackgroundColor.alpha() > 0) {
        header += QString("  <rect width=\"%1\" height=\"%2\" fill=\"%3\"/>\n")
                      .arg(mCanvasSize.width())
                      .arg(mCanvasSize.height())
                      .arg(mBackgroundColor.name());
    }

    return header;
}

QString SVGExporter::generateSVGLayer(const SVGLayer& layer)
{
    QString layerSVG;

    // Layer group
    QString visibilityAttr = layer.visible ? "" : " style=\"display:none\"";

    layerSVG += QString("  <g id=\"%1\"%2>\n")
                    .arg(layer.id)
                    .arg(visibilityAttr);

    layerSVG += QString("    <!-- Layer: %1 -->\n").arg(layer.name);

    // Polylines
    for (const auto& polyline : layer.polylines) {
        layerSVG += "    ";
        layerSVG += generateSVGPolyline(polyline, layer.strokeColor, layer.strokeWidth);
    }

    layerSVG += "  </g>\n";

    return layerSVG;
}

QString SVGExporter::generateSVGPolyline(const std::vector<QPointF>& polyline,
                                         const QColor& strokeColor,
                                         float strokeWidth)
{
    if (polyline.empty()) {
        return "";
    }

    QString points;
    for (size_t i = 0; i < polyline.size(); i++) {
        if (i > 0) points += " ";
        points += QString("%1,%2").arg(polyline[i].x()).arg(polyline[i].y());
    }

    QString svg = QString("<polyline points=\"%1\" "
                          "stroke=\"%2\" stroke-width=\"%3\" "
                          "fill=\"none\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n")
                      .arg(points)
                      .arg(strokeColor.name())
                      .arg(strokeWidth);

    return svg;
}

QString SVGExporter::generateSVGFooter()
{
    return "</svg>\n";
}

//==============================================================================
// Utilities
//==============================================================================

QPointF SVGExporter::project3DTo2D(const QVector3D& point3D, const QMatrix4x4& viewMatrix)
{
    QVector4D homogeneous(point3D.x(), point3D.y(), point3D.z(), 1.0f);
    QVector4D projected = viewMatrix * homogeneous;

    // Perspective division
    if (projected.w() != 0.0f) {
        projected /= projected.w();
    }

    return QPointF(projected.x(), projected.y());
}

QString SVGExporter::unitToString(Unit unit)
{
    switch (unit) {
        case Unit::Millimeter: return "mm";
        case Unit::Centimeter: return "cm";
        case Unit::Pixel: return "px";
        default: return "mm";
    }
}

} // namespace Illustrator
} // namespace DongArch
