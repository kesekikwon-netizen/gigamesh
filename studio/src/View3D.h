// DongArch3D Studio — 3D 유물·절단면 보조창 (UI_RULES R1: 약 27 %, 접기 가능).
#pragma once
#include "ArtifactDoc.h"
#include "MeshRaster.h"

#include <QLabel>
#include <QWidget>

namespace studio {

class View3DCanvas : public QWidget {
    Q_OBJECT
public:
    explicit View3DCanvas(QWidget* parent = nullptr);
    void setDoc(const ArtifactDoc* d, int slot);
    void setView(double yaw, double pitch);
    void setCutTool(bool on) { cutTool_ = on; }

signals:
    void cutAngleDragged(double deltaDeg);
    void cutDragReleased();  // 단면 도구로 끌다가 놓음 → 원본으로 확정

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent*) override { dirty_ = true; }

private:
    RasterView view() const;
    const ArtifactDoc* doc_ = nullptr;
    int slot_ = 0;
    double yaw_ = -32.0, pitch_ = 14.0;
    QImage img_;
    bool dirty_ = true;
    bool drag_ = false, cutTool_ = false;
    bool cutDrag_ = false;  // 단면 도구 + 왼쪽 단추 끌기 = 회전판 (오른쪽·가운데 = 보는 방향)
    QPoint last_;
    std::vector<KA::Polyline3> cut_;
};

class View3DPane : public QWidget {
    Q_OBJECT
public:
    explicit View3DPane(QWidget* parent = nullptr);
    View3DCanvas* canvas() const { return canvas_; }
    void refresh(const ArtifactDoc& doc, int slot);
    void setCutPage(bool on);  // ② 막으로 자르기: 머리 제목·3D 끌기 툴팁

signals:
    void collapseRequested();

private:
    bool cutPage_ = false;
    View3DCanvas* canvas_;
    QLabel* title_;
    QLabel* info_;
    QLabel* info2_;
};

}  // namespace studio
