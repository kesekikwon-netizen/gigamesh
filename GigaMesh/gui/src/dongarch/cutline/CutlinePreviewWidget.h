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

#ifndef DONGARCH_CUTLINE_PREVIEW_WIDGET_H
#define DONGARCH_CUTLINE_PREVIEW_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <vector>
#include "../common/DongArchTypes.h"

//! \file CutlinePreviewWidget.h
//! \brief Phase 2: Cutline 2D Preview Widget (v4.0)
//!
//! 2D SVG-style preview of cutline before export
//! - Real-time rendering with QPainter
//! - 1:1 scale display (mm units)
//! - Interactive pan and zoom
//! - Grid and axis display

namespace DongArch {
namespace Cutline {

//! \struct PreviewSettings
//! \brief 2D Preview display settings
struct PreviewSettings {
    // Display options
    bool showGrid = true;           //!< Show background grid
    bool showAxes = true;           //!< Show X/Y axes
    bool showDimensions = true;     //!< Show dimensions
    bool showScale = true;          //!< Show scale bar

    // Visual style
    QColor lineColor = Qt::red;     //!< Cutline color
    float lineWidth = 2.0f;         //!< Line width (pixels)
    QColor gridColor = QColor(200, 200, 200);  //!< Grid color
    QColor bgColor = Qt::white;     //!< Background color

    // Units
    QString unit = "mm";            //!< Display unit (mm/inch)
    float gridSpacing = 10.0f;      //!< Grid spacing (mm)

    // View transform
    float zoom = 1.0f;              //!< Zoom level (1.0 = 1:1)
    QPointF panOffset;              //!< Pan offset (pixels)
};

//! \class CutlinePreviewWidget
//! \brief 2D preview widget for cutline visualization
//!
//! Renders cutline in 2D view before SVG export
//! Uses QPainter for real-time rendering
class CutlinePreviewWidget : public QWidget {
    Q_OBJECT

public:
    //! Constructor
    //! \param parent Parent widget
    explicit CutlinePreviewWidget(QWidget* parent = nullptr);

    //! Destructor
    ~CutlinePreviewWidget() override = default;

    //! Set cutline data to display
    //! \param cutline Cutline result from CutlineManager
    void setCutline(const DongArch::CutlineResult& cutline);

    //! Clear cutline data
    void clearCutline();

    //! Get preview settings
    //! \return Current settings
    const PreviewSettings& getSettings() const { return mSettings; }

    //! Set preview settings
    //! \param settings New settings
    void setSettings(const PreviewSettings& settings);

    //! Update single setting
    void setShowGrid(bool show);
    void setShowAxes(bool show);
    void setShowDimensions(bool show);
    void setShowScale(bool show);
    void setLineColor(const QColor& color);
    void setLineWidth(float width);
    void setZoom(float zoom);

    //! Fit view to cutline bounds
    void fitToView();

    //! Reset zoom and pan to default
    void resetView();

    //! Export current view as image
    //! \param path Output file path (PNG)
    //! \return True if successful
    bool exportImage(const QString& path);

signals:
    //! Emitted when user clicks on cutline
    //! \param point Click position in cutline coordinates (mm)
    void pointClicked(const QPointF& point);

    //! Emitted when view changes (zoom/pan)
    void viewChanged();

protected:
    //! Paint event - render cutline
    void paintEvent(QPaintEvent* event) override;

    //! Mouse press event
    void mousePressEvent(QMouseEvent* event) override;

    //! Mouse move event
    void mouseMoveEvent(QMouseEvent* event) override;

    //! Mouse release event
    void mouseReleaseEvent(QMouseEvent* event) override;

    //! Wheel event (zoom)
    void wheelEvent(QWheelEvent* event) override;

    //! Resize event
    void resizeEvent(QResizeEvent* event) override;

private:
    //! Render background (grid, axes)
    void renderBackground(QPainter& painter);

    //! Render cutline polyline
    void renderCutline(QPainter& painter);

    //! Render dimensions (width, height)
    void renderDimensions(QPainter& painter);

    //! Render scale bar
    void renderScaleBar(QPainter& painter);

    //! Convert cutline coordinates (mm) to widget coordinates (pixels)
    //! \param pt Cutline point (x, y in mm)
    //! \return Widget pixel coordinates
    QPointF toWidgetCoords(const QPointF& pt) const;

    //! Convert widget coordinates (pixels) to cutline coordinates (mm)
    //! \param pt Widget pixel point
    //! \return Cutline point (x, y in mm)
    QPointF toCutlineCoords(const QPointF& pt) const;

    //! Calculate bounding box of cutline
    //! \return Bounding rect in cutline coordinates (mm)
    QRectF calculateBounds() const;

    //! Convert flat float array to QPointF vector
    //! \param points Flat array (x, y, z, x, y, z, ...)
    //! \return 2D points (x, y)
    std::vector<QPointF> toPointVector(const std::vector<float>& points) const;

    // Data
    DongArch::CutlineResult mCutline;   //!< Current cutline data
    std::vector<QPointF> mPoints2D;     //!< 2D points (x, y) for rendering
    PreviewSettings mSettings;          //!< Display settings
    QRectF mBounds;                     //!< Cutline bounding box (mm)

    // Interaction state
    bool mPanning;                      //!< Currently panning
    QPoint mLastMousePos;               //!< Last mouse position
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_PREVIEW_WIDGET_H
