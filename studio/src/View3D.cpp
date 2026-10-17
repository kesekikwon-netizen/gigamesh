#include "View3D.h"

#include "Theme.h"

#include <QHBoxLayout>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <cmath>

namespace studio {

View3DCanvas::View3DCanvas(QWidget* parent) : QWidget(parent) {
    setMinimumSize(200, 200);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setCursor(Qt::OpenHandCursor);
}

void View3DCanvas::setDoc(const ArtifactDoc* d, int slot) {
    doc_ = d;
    slot_ = slot;
    cut_.clear();
    if (doc_ && doc_->isLoaded()) {
        const auto& sl = doc_->slotList();
        const auto& m = doc_->lodMesh().faces.empty() ? doc_->mesh() : doc_->lodMesh();
        if (slot >= 0 && slot < int(sl.size()) && sl[size_t(slot)].kind == SlotKind::Horizontal)
            cut_ = KA::Slice(m, KA::Plane{{0, 0, 1}, -sl[size_t(slot)].z});
        else
            cut_ = KA::Slice(m, KA::Plane{{0, 1, 0}, 0.0});
    }
    dirty_ = true;
    update();
}

void View3DCanvas::setView(double yaw, double pitch) {
    yaw_ = yaw;
    pitch_ = pitch;
    dirty_ = true;
    update();
}

RasterView View3DCanvas::view() const {
    RasterView v;
    v.yawDeg = yaw_;
    v.pitchDeg = pitch_;
    if (!doc_ || !doc_->isLoaded()) return v;
    const auto& m = doc_->mesh();
    const double r = std::max({std::fabs(m.bboxMin.x), std::fabs(m.bboxMax.x), std::fabs(m.bboxMin.y), std::fabs(m.bboxMax.y)});
    const double h = m.bboxMax.z - m.bboxMin.z;
    const double extent = std::sqrt(4 * r * r + h * h) * 1.12;
    v.pxPerMm = std::min(width(), height()) / std::max(1.0, extent);
    v.centerPx = QPointF(width() / 2.0, height() / 2.0);
    v.centerZ = m.bboxMin.z + h / 2.0;
    return v;
}

void View3DCanvas::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), theme::Ground);
    if (!doc_ || !doc_->isLoaded()) return;
    const RasterView v = view();
    if (dirty_ || img_.size() != size()) {
        const auto& lm = doc_->lodMesh().faces.empty() ? doc_->mesh() : doc_->lodMesh();
        const auto& lc = doc_->lodVertexColors();
        img_ = renderMesh(lm, size(), v, ShadeMode::Real, theme::ModelClay, nullptr,
                          lc.size() == lm.vertices.size() ? &lc : nullptr);
        dirty_ = false;
    }
    p.setRenderHint(QPainter::Antialiasing);
    const auto& m = doc_->mesh();
    const double R = std::max({std::fabs(m.bboxMin.x), std::fabs(m.bboxMax.x), std::fabs(m.bboxMin.y), std::fabs(m.bboxMax.y)}) + 8;
    const double z0 = m.bboxMin.z - 6, z1 = m.bboxMax.z + 8;
    const auto& sl = doc_->slotList();
    const bool horiz = slot_ >= 0 && slot_ < int(sl.size()) && sl[size_t(slot_)].kind == SlotKind::Horizontal;

    // 절단면(평면) — 뒤쪽 반투명
    QPolygonF quad;
    if (horiz) {
        const double z = sl[size_t(slot_)].z;
        for (auto [x, y] : {std::pair{-R, -R}, {R, -R}, {R, R}, {-R, R}}) quad << projectPoint({x, y, z}, v);
    } else {
        for (auto [x, z] : {std::pair{-R, z0}, {R, z0}, {R, z1}, {-R, z1}}) quad << projectPoint({x, 0, z}, v);
    }
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 0, 0, 22));
    p.drawPolygon(quad);
    p.drawImage(0, 0, img_);
    p.setPen(QPen(theme::ProfileRed, 1, Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    p.drawPolygon(quad);

    // 중심축
    p.setPen(QPen(theme::Ink, 1, Qt::DashDotLine));
    p.drawLine(projectPoint({0, 0, z0}, v), projectPoint({0, 0, z1}, v));

    // 절단 단면선: 흰 3 px 테 + 빨강
    for (int pass = 0; pass < 2; ++pass) {
        p.setPen(QPen(pass == 0 ? QColor(Qt::white) : theme::ProfileRed, pass == 0 ? 5.0 : 2.0, Qt::SolidLine, Qt::RoundCap,
                      Qt::RoundJoin));
        for (const auto& pl : cut_) {
            QPolygonF poly;
            for (const auto& q : pl.pts) poly << projectPoint(q, v);
            if (poly.size() > 1) p.drawPolyline(poly);
        }
    }
    // A / A′ 표지
    const QString a = horiz ? sl[size_t(slot_)].name.left(1) : QStringLiteral("A");
    auto tag = [&](QPointF c, const QString& s) {
        const QRectF r(c.x() - 13, c.y() - 10, 26, 20);
        p.setPen(QPen(theme::ProfileRed, 1));
        p.setBrush(theme::Surface);
        p.drawRoundedRect(r, 3, 3);
        p.setFont(theme::monoFont(12, true));
        p.setPen(theme::ProfileRed);
        p.drawText(r, Qt::AlignCenter, s);
    };
    tag((quad[2] + quad[3]) / 2, a);
    tag((quad[0] + quad[1]) / 2, a + QStringLiteral("′"));
}

void View3DCanvas::mousePressEvent(QMouseEvent* e) {
    drag_ = true;
    last_ = e->pos();
    setCursor(Qt::ClosedHandCursor);
}

void View3DCanvas::mouseMoveEvent(QMouseEvent* e) {
    if (!drag_) return;
    const QPoint d = e->pos() - last_;
    last_ = e->pos();
    if (cutTool_) {
        emit cutAngleDragged(d.x() * 0.5);
        return;
    }
    yaw_ += d.x() * 0.6;
    pitch_ = std::clamp(pitch_ + d.y() * 0.5, -89.0, 89.0);
    dirty_ = true;
    update();
}

void View3DCanvas::mouseReleaseEvent(QMouseEvent*) {
    drag_ = false;
    setCursor(Qt::OpenHandCursor);
}

// ---- View3DPane -----------------------------------------------------------
View3DPane::View3DPane(QWidget* parent) : QWidget(parent) {
    setObjectName("View3DPane");
    setAttribute(Qt::WA_StyledBackground, true);
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);

    auto* head = new QWidget(this);
    head->setObjectName("PaneHead");
    head->setAttribute(Qt::WA_StyledBackground, true);
    head->setFixedHeight(32);
    auto* h = new QHBoxLayout(head);
    h->setContentsMargins(12, 0, 6, 0);
    h->setSpacing(4);
    auto* t = new QLabel(QStringLiteral("<b>3D 유물</b> <span style='color:%1'>절단면 · 중심축</span>").arg(theme::hex(theme::Muted)), head);
    t->setProperty("ph", true);
    h->addWidget(t);
    h->addStretch();
    auto mk = [&](const QString& s, const QString& tip) {
        auto* b = new QPushButton(s, head);
        b->setProperty("icon-btn", true);
        b->setStyleSheet(QStringLiteral("min-width:34px; max-width:40px;"));
        b->setToolTip(tip);
        b->setFocusPolicy(Qt::NoFocus);
        h->addWidget(b);
        return b;
    };
    auto* front = mk(QStringLiteral("정면"), QStringLiteral("정면에서 보기"));
    auto* top = mk(QStringLiteral("위"), QStringLiteral("위에서 보기"));
    auto* coll = mk(QStringLiteral("◀"), QStringLiteral("3D 보조창 접기"));
    v->addWidget(head);

    auto* info = new QWidget(this);
    info->setObjectName("View3DInfo");
    info->setAttribute(Qt::WA_StyledBackground, true);
    info->setFixedHeight(28);
    auto* ih = new QHBoxLayout(info);
    ih->setContentsMargins(12, 0, 12, 0);
    info_ = new QLabel(info);
    info2_ = new QLabel(info);
    info_->setFont(theme::monoFont(12));
    info2_->setFont(theme::monoFont(12));
    ih->addWidget(info_);
    ih->addStretch();
    ih->addWidget(info2_);
    v->addWidget(info);

    canvas_ = new View3DCanvas(this);
    v->addWidget(canvas_, 1);
    connect(front, &QPushButton::clicked, this, [this] { canvas_->setView(0, 0); });
    connect(top, &QPushButton::clicked, this, [this] { canvas_->setView(0, 89); });
    connect(coll, &QPushButton::clicked, this, &View3DPane::collapseRequested);
}

void View3DPane::refresh(const ArtifactDoc& doc, int slot) {
    canvas_->setDoc(&doc, slot);
    const QString mu = theme::hex(theme::Muted);
    info_->setText(QStringLiteral("<span style='color:%1; font-family:Malgun Gothic'>메시</span> <b>%2</b><span style='color:%1; font-family:Malgun Gothic'>면</span>")
                       .arg(mu, QLocale(QLocale::English).toString(qlonglong(doc.mesh().faces.size()))));
    info_->setToolTip(doc.scannerProfile());
    info2_->setText(QStringLiteral("<span style='color:%1; font-family:Malgun Gothic'>중심축 오차</span> %2 mm")
                        .arg(mu).arg(doc.axisRmse(), 0, 'f', 2));
}

}  // namespace studio
