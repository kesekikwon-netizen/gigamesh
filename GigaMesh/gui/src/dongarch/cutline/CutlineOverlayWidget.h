/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#ifndef DONGARCH_CUTLINE_OVERLAY_WIDGET_H
#define DONGARCH_CUTLINE_OVERLAY_WIDGET_H

#include "CutlineParams.h"
#include <GigaMesh/mesh/vector3d.h>
#include <QWidget>
#include <QPainter>
#include <QPen>
#include <vector>

namespace DongArch {
namespace Cutline {

//! Cutline 오버레이 위젯 (2D 렌더링)
//!
//! **기능**:
//! - 3D 폴리라인을 2D 화면 좌표로 투영
//! - 실시간 스타일 업데이트 (색상/두께/불투명도)
//! - 60fps 목표 (경량 QPainter 사용)
//!
//! **TODO**: OpenGL 3D 오버레이 버전 (Phase 2)
class CutlineOverlayWidget : public QWidget {
    Q_OBJECT

public:
    explicit CutlineOverlayWidget(QWidget* parent = nullptr);
    ~CutlineOverlayWidget() override = default;

    //! 폴리라인 설정
    void setPolyline(const std::vector<Vector3D>& polyline);

    //! 스타일 설정
    void setColor(const QColor& color);
    void setWidth(float width);
    void setOpacity(float opacity);

    //! 스타일 가져오기
    QColor getColor() const { return mColor; }
    float getWidth() const { return mWidth; }
    float getOpacity() const { return mOpacity; }

    //! Raw 결과 여부 (점선으로 표시)
    void setIsRaw(bool isRaw) { mIsRaw = isRaw; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    //! 3D → 2D 투영 (간단한 직교 투영)
    QPointF project3DTo2D(const Vector3D& point) const;

    std::vector<Vector3D> mPolyline;  //!< 3D 폴리라인
    QColor mColor;                    //!< 라인 색상
    float mWidth;                     //!< 라인 두께
    float mOpacity;                   //!< 불투명도
    bool mIsRaw;                      //!< Raw 결과 (점선)
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_OVERLAY_WIDGET_H
