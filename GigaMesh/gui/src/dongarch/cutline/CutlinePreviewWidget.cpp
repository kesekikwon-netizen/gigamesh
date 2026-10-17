/* * DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "CutlinePreviewWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPen>
#include <QBrush>
#include <QFontMetrics>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace DongArch {
namespace Cutline {

CutlinePreviewWidget::CutlinePreviewWidget(QWidget* parent)
    : QWidget(parent)
    , mPanning(false)
{
    // Enable mouse tracking for hover effects
    setMouseTracking(true);

    // Set minimum size
    setMinimumSize(400, 300);

    // Initialize settings
    mSettings.zoom = 1.0f;
    mSettings.panOffset = QPointF(0, 0);

    std::cout << "[CutlinePreviewWidget] Widget created" << std::endl;
}

void CutlinePreviewWidget::setCutline(const DongArch::CutlineResult& cutline) {
    mCutline = cutline;

    // Convert flat float array to 2D point vector
    mPoints2D = toPointVector(cutline.points);

    // Calculate bounding box
    mBounds = calculateBounds();

    // Fit view to new cutline
    fitToView();

    std::cout << "[CutlinePreviewWidget] Cutline set: " << mPoints2D.size()
              << " points, bounds=" << mBounds.width() << "x" << mBounds.height() << "mm" << std::endl;

    update();  // Trigger repaint
}

void CutlinePreviewWidget::clearCutline() {
    mCutline = DongArch::CutlineResult();
    mPoints2D.clear();
    mBounds = QRectF();
    update();

    std::cout << "[CutlinePreviewWidget] Cutline cleared" << std::endl;
}

void CutlinePreviewWidget::setSettings(const PreviewSettings& settings) {
    mSettings = settings;
    update();
}

void CutlinePreviewWidget::setShowGrid(bool show) {
    mSettings.showGrid = show;
    update();
}

void CutlinePreviewWidget::setShowAxes(bool show) {
    mSettings.showAxes = show;
    update();
}

void CutlinePreviewWidget::setShowDimensions(bool show) {
    mSettings.showDimensions = show;
    update();
}

void CutlinePreviewWidget::setShowScale(bool show) {
    mSettings.showScale = show;
    update();
}

void CutlinePreviewWidget::setLineColor(const QColor& color) {
    mSettings.lineColor = color;
    update();
}

void CutlinePreviewWidget::setLineWidth(float width) {
    mSettings.lineWidth = width;
    update();
}

void CutlinePreviewWidget::setZoom(float zoom) {
    // Clamp zoom to reasonable range
    mSettings.zoom = std::max(0.1f, std::min(10.0f, zoom));
    update();
    emit viewChanged();
}

void CutlinePreviewWidget::fitToView() {
    if (mPoints2D.empty() || mBounds.isEmpty()) {
        return;
    }

    // Calculate zoom to fit bounds in widget
    float widgetWidth = width() * 0.9f;   // 90% of widget width (leave margin)
    float widgetHeight = height() * 0.9f;

    float zoomX = widgetWidth / mBounds.width();
    float zoomY = widgetHeight / mBounds.height();

    // Use smaller zoom to fit both dimensions
    mSettings.zoom = std::min(zoomX, zoomY);

    // Center the cutline
    mSettings.panOffset = QPointF(width() / 2.0f, height() / 2.0f);

    std::cout << "[CutlinePreviewWidget] Fit to view: zoom=" << mSettings.zoom << std::endl;

    update();
    emit viewChanged();
}

void CutlinePreviewWidget::resetView() {
    mSettings.zoom = 1.0f;
    mSettings.panOffset = QPointF(0, 0);
    update();
    emit viewChanged();
}

bool CutlinePreviewWidget::exportImage(const QString& path) {
    if (mPoints2D.empty()) {
        std::cerr << "[CutlinePreviewWidget] Cannot export: no cutline data" << std::endl;
        return false;
    }

    // Create image with current widget size
    QImage image(size(), QImage::Format_ARGB32);
    image.fill(mSettings.bgColor);

    // Render to image
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    renderBackground(painter);
    renderCutline(painter);
    if (mSettings.showDimensions) renderDimensions(painter);
    if (mSettings.showScale) renderScaleBar(painter);

    // Save to file
    bool success = image.save(path);

    if (success) {
        std::cout << "[CutlinePreviewWidget] Image exported: " << path.toStdString() << std::endl;
    } else {
        std::cerr << "[CutlinePreviewWidget] Failed to export image" << std::endl;
    }

    return success;
}

// ============================================================================
// Protected: Event Handlers
// ============================================================================

void CutlinePreviewWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Fill background
    painter.fillRect(rect(), mSettings.bgColor);

    if (mPoints2D.empty()) {
        // No cutline - show message
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, tr("No cutline data\n(Load mesh and extract cutline)"));
        return;
    }

    // Render components
    renderBackground(painter);
    renderCutline(painter);
    if (mSettings.showDimensions) renderDimensions(painter);
    if (mSettings.showScale) renderScaleBar(painter);
}

void CutlinePreviewWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mPanning = true;
        mLastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    } else if (event->button() == Qt::RightButton) {
        // Convert to cutline coords and emit signal
        QPointF cutlinePos = toCutlineCoords(event->pos());
        emit pointClicked(cutlinePos);
    }
}

void CutlinePreviewWidget::mouseMoveEvent(QMouseEvent* event) {
    if (mPanning) {
        // Calculate delta
        QPoint delta = event->pos() - mLastMousePos;
        mLastMousePos = event->pos();

        // Update pan offset
        mSettings.panOffset += delta;

        update();
        emit viewChanged();
    }
}

void CutlinePreviewWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mPanning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void CutlinePreviewWidget::wheelEvent(QWheelEvent* event) {
    // Zoom with mouse wheel
    float delta = event->angleDelta().y() / 120.0f;  // Standard wheel step = 15 degrees = 120 units
    float zoomFactor = std::pow(1.1f, delta);

    float newZoom = mSettings.zoom * zoomFactor;

    // Clamp zoom
    newZoom = std::max(0.1f, std::min(10.0f, newZoom));

    // Zoom towards mouse position
    QPointF mousePos = event->position();
    QPointF cutlinePos = toCutlineCoords(mousePos);

    mSettings.zoom = newZoom;

    // Adjust pan to keep mouse position fixed
    QPointF newWidgetPos = toWidgetCoords(cutlinePos);
    mSettings.panOffset += (mousePos - newWidgetPos);

    update();
    emit viewChanged();
}

void CutlinePreviewWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    // Could adjust view on resize
    // For now, just repaint
    update();
}

// ============================================================================
// Private: Rendering
// ============================================================================

void CutlinePreviewWidget::renderBackground(QPainter& painter) {
    // Draw grid
    if (mSettings.showGrid && mSettings.zoom > 0.3f) {  // Only show grid if zoomed in enough
        painter.setPen(QPen(mSettings.gridColor, 1));

        // Grid spacing in mm
        float gridStep = mSettings.gridSpacing;

        // Convert grid bounds to widget coords
        QPointF topLeft = toWidgetCoords(mBounds.topLeft());
        QPointF bottomRight = toWidgetCoords(mBounds.bottomRight());

        // Draw vertical grid lines
        for (float x = 0; x < mBounds.right(); x += gridStep) {
            QPointF p1 = toWidgetCoords(QPointF(x, mBounds.top()));
            QPointF p2 = toWidgetCoords(QPointF(x, mBounds.bottom()));
            painter.drawLine(p1, p2);
        }

        // Draw horizontal grid lines
        for (float y = 0; y < mBounds.bottom(); y += gridStep) {
            QPointF p1 = toWidgetCoords(QPointF(mBounds.left(), y));
            QPointF p2 = toWidgetCoords(QPointF(mBounds.right(), y));
            painter.drawLine(p1, p2);
        }
    }

    // Draw axes
    if (mSettings.showAxes) {
        painter.setPen(QPen(Qt::black, 2));

        // X axis (horizontal)
        QPointF xStart = toWidgetCoords(QPointF(mBounds.left(), 0));
        QPointF xEnd = toWidgetCoords(QPointF(mBounds.right(), 0));
        painter.drawLine(xStart, xEnd);

        // Y axis (vertical)
        QPointF yStart = toWidgetCoords(QPointF(0, mBounds.top()));
        QPointF yEnd = toWidgetCoords(QPointF(0, mBounds.bottom()));
        painter.drawLine(yStart, yEnd);

        // Labels
        painter.setFont(QFont("Arial", 10));
        painter.drawText(xEnd + QPointF(5, 5), "X");
        painter.drawText(yStart + QPointF(5, -5), "Y");
    }
}

void CutlinePreviewWidget::renderCutline(QPainter& painter) {
    if (mPoints2D.size() < 2) {
        return;
    }

    // Setup pen
    QPen pen(mSettings.lineColor, mSettings.lineWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    // Convert points to widget coords and draw polyline
    QPolygonF polygon;
    for (const auto& pt : mPoints2D) {
        polygon << toWidgetCoords(pt);
    }

    painter.drawPolyline(polygon);

    // Draw points (if zoomed in enough)
    if (mSettings.zoom > 2.0f) {
        painter.setBrush(mSettings.lineColor);
        for (const auto& widgetPt : polygon) {
            painter.drawEllipse(widgetPt, 3, 3);
        }
    }
}

void CutlinePreviewWidget::renderDimensions(QPainter& painter) {
    if (mBounds.isEmpty()) {
        return;
    }

    painter.setPen(QPen(Qt::blue, 1));
    painter.setFont(QFont("Arial", 10));

    // Width dimension (bottom)
    QString widthText = QString("%1 %2").arg(mBounds.width(), 0, 'f', 1).arg(mSettings.unit);
    QPointF widthPos = toWidgetCoords(QPointF(mBounds.center().x(), mBounds.bottom()));
    painter.drawText(widthPos + QPointF(-30, 20), widthText);

    // Height dimension (right)
    QString heightText = QString("%1 %2").arg(mBounds.height(), 0, 'f', 1).arg(mSettings.unit);
    QPointF heightPos = toWidgetCoords(QPointF(mBounds.right(), mBounds.center().y()));
    painter.drawText(heightPos + QPointF(10, 5), heightText);
}

void CutlinePreviewWidget::renderScaleBar(QPainter& painter) {
    // Scale bar in bottom-left corner
    int barLength = 100;  // pixels
    float realLength = barLength / mSettings.zoom;  // mm

    // Round to nice value
    float niceLength = std::pow(10, std::floor(std::log10(realLength)));
    if (realLength / niceLength > 5) niceLength *= 5;
    else if (realLength / niceLength > 2) niceLength *= 2;

    int niceBarLength = static_cast<int>(niceLength * mSettings.zoom);

    // Draw scale bar
    int margin = 20;
    QPoint barStart(margin, height() - margin);
    QPoint barEnd(margin + niceBarLength, height() - margin);

    painter.setPen(QPen(Qt::black, 3));
    painter.drawLine(barStart, barEnd);

    // Draw ticks
    painter.drawLine(barStart, barStart + QPoint(0, -10));
    painter.drawLine(barEnd, barEnd + QPoint(0, -10));

    // Draw label
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    QString label = QString("%1 %2").arg(niceLength, 0, 'f', 0).arg(mSettings.unit);
    painter.drawText(barStart + QPoint(0, -15), label);
}

// ============================================================================
// Private: Coordinate Conversion
// ============================================================================

QPointF CutlinePreviewWidget::toWidgetCoords(const QPointF& pt) const {
    // Convert cutline coords (mm) to widget pixels
    // Apply zoom and pan

    float x = pt.x() * mSettings.zoom + mSettings.panOffset.x();
    float y = -pt.y() * mSettings.zoom + mSettings.panOffset.y();  // Flip Y axis (screen Y goes down)

    return QPointF(x, y);
}

QPointF CutlinePreviewWidget::toCutlineCoords(const QPointF& pt) const {
    // Convert widget pixels to cutline coords (mm)
    // Reverse zoom and pan

    float x = (pt.x() - mSettings.panOffset.x()) / mSettings.zoom;
    float y = -(pt.y() - mSettings.panOffset.y()) / mSettings.zoom;  // Flip Y axis

    return QPointF(x, y);
}

QRectF CutlinePreviewWidget::calculateBounds() const {
    if (mPoints2D.empty()) {
        return QRectF();
    }

    float minX = mPoints2D[0].x();
    float maxX = minX;
    float minY = mPoints2D[0].y();
    float maxY = minY;

    for (const auto& pt : mPoints2D) {
        minX = std::min(minX, static_cast<float>(pt.x()));
        maxX = std::max(maxX, static_cast<float>(pt.x()));
        minY = std::min(minY, static_cast<float>(pt.y()));
        maxY = std::max(maxY, static_cast<float>(pt.y()));
    }

    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

std::vector<QPointF> CutlinePreviewWidget::toPointVector(const std::vector<float>& points) const {
    std::vector<QPointF> result;
    result.reserve(points.size() / 3);

    // Convert flat array (x, y, z, x, y, z, ...) to QPointF (x, y)
    for (size_t i = 0; i + 2 < points.size(); i += 3) {
        result.emplace_back(points[i], points[i + 1]);
        // Ignore Z coordinate for 2D preview
    }

    return result;
}

} // namespace Cutline
} // namespace DongArch
