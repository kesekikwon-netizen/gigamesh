// DongArch3D Studio — 메인 유물 실측 도판 (UI_RULES R1.1, R1.3, R1.4, R5).
#pragma once
#include "Plate.h"

#include <QLabel>
#include <QPushButton>
#include <QWidget>

namespace studio {

class SheetCanvas : public QWidget {
    Q_OBJECT
public:
    explicit SheetCanvas(QWidget* parent = nullptr);
    void setDoc(const ArtifactDoc* d);
    void setOptions(const PlateOptions& o);
    const PlateOptions& options() const { return opts_; }
    void docChanged();  // 선/음영 다시 계산
    void fitView();

signals:
    void cursorMm(double r, double h, double wall, bool inside);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    QTransform xf() const;
    double outerRadiusAt(double h) const;

    const ArtifactDoc* doc_ = nullptr;
    PlateOptions opts_;
    ShadeImage shade_;
    bool shadeDirty_ = true;
    double ppm_ = 3.0;
    QPointF centerMm_{0, 90};
    bool fitted_ = false;
    bool panning_ = false;
    QPoint lastPos_;
    QPointF hoverMm_;
    bool hover_ = false;
};

class SheetPane : public QWidget {
    Q_OBJECT
public:
    explicit SheetPane(QWidget* parent = nullptr);
    SheetCanvas* canvas() const { return canvas_; }
    void refresh(const ArtifactDoc& doc, int slot);
    void setMaximized(bool on);

signals:
    void maximizeToggled();

private:
    QLabel* title_;
    QLabel* sub_;
    QLabel* badge_;
    QLabel* scale_;
    QPushButton* maxBtn_;
    QLabel* dims_;
    QLabel* order_;
    SheetCanvas* canvas_;
};

}  // namespace studio
