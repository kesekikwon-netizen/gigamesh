/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "CutlineOverlayWidget.h"
#include <QPainter>
#include <QPen>

namespace DongArch {
namespace Cutline {

CutlineOverlayWidget::CutlineOverlayWidget(QWidget* parent)
    : QWidget(parent)
    , mColor(Qt::red)
    , mWidth(2.0f)
    , mOpacity(0.8f)
    , mIsRaw(false)
{
    // 투명 배경
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents); // 마우스 이벤트 투과
}

void CutlineOverlayWidget::setPolyline(const std::vector<Vector3D>& polyline) {
    mPolyline = polyline;
    update(); // 재렌더링 트리거
}

void CutlineOverlayWidget::setColor(const QColor& color) {
    mColor = color;
    update();
}

void CutlineOverlayWidget::setWidth(float width) {
    mWidth = width;
    update();
}

void CutlineOverlayWidget::setOpacity(float opacity) {
    mOpacity = opacity;
    update();
}

void CutlineOverlayWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    if (mPolyline.size() < 2) {
        return; // 라인 그릴 수 없음
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 펜 설정
    QPen pen(mColor);
    pen.setWidthF(mWidth);
    if (mIsRaw) {
        pen.setStyle(Qt::DashLine); // Raw 결과는 점선
    }

    // 불투명도 적용
    QColor colorWithAlpha = mColor;
    colorWithAlpha.setAlphaF(mOpacity);
    pen.setColor(colorWithAlpha);

    painter.setPen(pen);

    // 폴리라인 그리기
    for (size_t i = 1; i < mPolyline.size(); ++i) {
        QPointF p1 = project3DTo2D(mPolyline[i - 1]);
        QPointF p2 = project3DTo2D(mPolyline[i]);

        painter.drawLine(p1, p2);
    }
}

QPointF CutlineOverlayWidget::project3DTo2D(const Vector3D& point) const {
    // TODO: 실제 카메라 투영 행렬 사용
    // 현재는 간단한 직교 투영 (X, Y만 사용)

    // 화면 크기
    int w = width();
    int h = height();

    // 정규화된 좌표 [-1, 1] → 화면 좌표 [0, w/h]
    // (임시: X, Y를 [-100, 100] 범위로 가정)
    double x = point.getX();
    double y = point.getY();

    double screenX = (x + 100.0) / 200.0 * w;
    double screenY = h - (y + 100.0) / 200.0 * h; // Y축 반전

    return QPointF(screenX, screenY);
}

} // namespace Cutline
} // namespace DongArch
