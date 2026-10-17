#include "Ribbon.h"

#include "Theme.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QVBoxLayout>

namespace studio {

// ---- Tile -----------------------------------------------------------------
Tile::Tile(const QString& id, const QString& text, const QString& icon, QWidget* parent)
    : QToolButton(parent), icon_(icon) {
    setObjectName(id);
    setText(text);
    setProperty("tile", true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

QSize Tile::sizeHint() const {
    const QFontMetrics fm(theme::uiFont(12, true));
    const int lines = text().contains('\n') ? 2 : 1;
    return {std::max(56, fm.horizontalAdvance(text().section('\n', 0, 0)) + 8), 48 + 4 + lines * 16};
}

void Tile::setToolState(ToolState s) {
    state_ = s;
    update();
}

void Tile::drawIcon(QPainter& p, const QString& k, const QRectF& r, const QColor& c) {
    p.save();
    p.setPen(QPen(c, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    const double x = r.left(), y = r.top(), w = r.width();
    auto P = [&](double u, double v) { return QPointF(x + u * w / 24.0, y + v * w / 24.0); };
    if (k == "open") {
        QPainterPath a;
        a.moveTo(P(3, 7)); a.lineTo(P(3, 19)); a.lineTo(P(21, 19)); a.lineTo(P(21, 9)); a.lineTo(P(11, 9));
        a.lineTo(P(9, 6)); a.lineTo(P(3, 6)); a.closeSubpath();
        p.drawPath(a);
    } else if (k == "recent") {
        p.drawEllipse(QRectF(P(3, 3), P(21, 21)));
        p.drawLine(P(12, 7), P(12, 12)); p.drawLine(P(12, 12), P(15.5, 14));
    } else if (k == "axis") {
        p.drawLine(P(12, 2), P(12, 22)); p.drawLine(P(6, 8), P(18, 8)); p.drawLine(P(8, 16), P(16, 16));
    } else if (k == "cut") {
        p.drawLine(P(5, 19), P(19, 5)); p.drawEllipse(QRectF(P(3, 17), P(7, 21))); p.drawEllipse(QRectF(P(17, 3), P(21, 7)));
    } else if (k == "outline") {
        QPainterPath a;
        a.moveTo(P(8, 3)); a.lineTo(P(16, 3)); a.cubicTo(P(15, 7), P(21, 10), P(19, 16)); a.lineTo(P(15, 21));
        a.lineTo(P(9, 21)); a.lineTo(P(5, 16)); a.cubicTo(P(3, 10), P(9, 7), P(8, 3));
        p.drawPath(a);
    } else if (k == "shade") {
        p.drawRoundedRect(QRectF(P(3, 3), P(21, 21)), 2, 2);
        p.drawEllipse(QRectF(P(7, 7), P(11, 11)));
        p.drawPolyline(QPolygonF{P(3, 18), P(9, 13), P(14, 17), P(21, 11)});
    } else if (k == "profile") {
        QPainterPath a;
        a.moveTo(P(5, 16)); a.cubicTo(P(8, 9), P(16, 9), P(19, 16));
        p.setPen(QPen(theme::ProfileRed, 2, Qt::SolidLine, Qt::RoundCap));
        p.drawPath(a);
    } else if (k == "grid") {
        for (int i : {6, 12, 18}) p.drawLine(P(4, i), P(20, i));
    } else if (k == "doc") {
        QPainterPath a;
        a.moveTo(P(6, 3)); a.lineTo(P(14, 3)); a.lineTo(P(19, 8)); a.lineTo(P(19, 21)); a.lineTo(P(6, 21)); a.closeSubpath();
        p.drawPath(a); p.drawLine(P(14, 3), P(14, 8)); p.drawLine(P(14, 8), P(19, 8));
    } else if (k == "pen") {
        QPainterPath a;
        a.moveTo(P(4, 4)); a.lineTo(P(14, 7)); a.lineTo(P(20, 17)); a.lineTo(P(17, 20)); a.lineTo(P(7, 14)); a.closeSubpath();
        p.drawPath(a); p.drawLine(P(4, 4), P(10, 10));
    } else if (k == "dxf") {
        p.drawRect(QRectF(P(4, 4), P(20, 20))); p.drawLine(P(8, 12), P(16, 12));
    } else if (k == "lithic") {
        QPainterPath a;
        a.moveTo(P(12, 2)); a.lineTo(P(19, 10)); a.lineTo(P(16, 21)); a.lineTo(P(8, 21)); a.lineTo(P(5, 10)); a.closeSubpath();
        p.drawPath(a); p.drawLine(P(12, 2), P(12, 21));
    } else if (k == "check") {  // 확정 (이 자세로 확정)
        p.drawPolyline(QPolygonF{P(4, 13), P(10, 19), P(20, 6)});
    } else if (k == "lock") {  // ② 지금 각도 고정 (원문 i-lock: 둥근 몸통 + 고리)
        p.drawRoundedRect(QRectF(P(5, 11), P(19, 21)), 2, 2);
        p.drawArc(QRectF(P(8, 4), P(16, 14)), 0, 180 * 16);
        p.drawLine(P(8, 9), P(8, 11)); p.drawLine(P(16, 9), P(16, 11));
    } else if (k == "next") {  // ② 도면 편집으로 (원문 i-next: → 화살표)
        p.drawLine(P(4, 12), P(20, 12)); p.drawPolyline(QPolygonF{P(14, 6), P(20, 12), P(14, 18)});
    } else if (k == "axX" || k == "axY" || k == "axZ") {  // ② 막 방향: 글자 아이콘 (축 색은 R2 승인 전 — 먹색)
        p.setFont(theme::uiFont(16, true));
        p.drawText(r, Qt::AlignCenter, k.right(1));
    }
    p.restore();
}

void Tile::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF sq((width() - 48) / 2.0, 0.5, 48, 48);
    QColor bg = theme::Desk, border = Qt::transparent, fg = isEnabled() ? theme::Ink : theme::Faint;
    switch (state_) {
        case ToolState::On: bg = theme::Oat; border = theme::Ring; break;
        case ToolState::Hand: bg = theme::Hand; border = theme::Hand; fg = theme::Ground; break;
        case ToolState::Primary: bg = theme::Action; border = theme::Action; fg = Qt::white; break;
        default:
            if (hover_ && isEnabled()) bg = theme::Wash, border = theme::Edge;
    }
    if (state_ == ToolState::Primary && hover_) bg = theme::ActionHover;
    if (isDown()) bg = state_ == ToolState::Idle ? theme::Press : bg.darker(108);
    p.setPen(border == Qt::transparent ? Qt::NoPen : QPen(border, 1));
    p.setBrush(bg);
    p.drawRoundedRect(sq.adjusted(0.5, 0.5, -0.5, -0.5), 8, 8);
    drawIcon(p, icon_, QRectF(sq.center() - QPointF(12, 12), QSizeF(24, 24)), fg);
    const bool bold = state_ == ToolState::Hand || state_ == ToolState::Primary;
    p.setFont(theme::uiFont(12, bold));
    p.setPen(isEnabled() ? theme::Ink : theme::Faint);
    p.drawText(QRectF(0, 51, width(), height() - 51), Qt::AlignHCenter | Qt::AlignTop, text());
}

// ---- SmallButton ----------------------------------------------------------
SmallButton::SmallButton(const QString& text, const QString& kbd, QWidget* parent) : QPushButton(text, parent), kbd_(kbd) {
    setProperty("sbtn", true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

QSize SmallButton::sizeHint() const {
    const QFontMetrics fm(theme::uiFont(12, true)), fk(theme::monoFont(11));
    const int k = kbd_.isEmpty() ? 0 : fk.horizontalAdvance(kbd_) + 18;
    return {fm.horizontalAdvance(text()) + 20 + k, 23};
}

void SmallButton::setToolState(ToolState s) {
    setProperty("state", s == ToolState::On || s == ToolState::Hand ? "on" : "idle");
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void SmallButton::paintEvent(QPaintEvent* e) {
    QPushButton::paintEvent(e);
    if (kbd_.isEmpty()) return;
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QFont f = theme::monoFont(11);
    const int w = QFontMetrics(f).horizontalAdvance(kbd_) + 8;
    const QRectF r(width() - w - 6, 4, w, 15);
    p.setPen(QPen(theme::Edge, 1));
    p.setBrush(theme::Surface);
    p.drawRoundedRect(r, 3, 3);
    p.drawLine(r.bottomLeft() + QPointF(2, 0.5), r.bottomRight() + QPointF(-2, 0.5));
    p.setFont(f);
    p.setPen(theme::Ink2);
    p.drawText(r, Qt::AlignCenter, kbd_);
}

// ---- RibbonGroup ----------------------------------------------------------
RibbonGroup::RibbonGroup(Ribbon* rb, const QString& name, QWidget* parent) : QWidget(parent), rb_(rb) {
    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(10, 0, 10, 0);
    v->setSpacing(2);
    name_ = new QLabel(name, this);
    name_->setProperty("gname", true);
    name_->setFixedHeight(16);
    v->addWidget(name_);
    row_ = new QHBoxLayout;
    row_->setSpacing(6);
    row_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    v->addLayout(row_, 1);
}

void RibbonGroup::setTitle(const QString& t) { name_->setText(t); }

Tile* RibbonGroup::tile(const QString& id, const QString& text, const QString& icon, const QString& tip) {
    auto* t = new Tile(id, text, icon, this);
    if (!tip.isEmpty()) t->setToolTip(tip);
    row_->addWidget(t, 0, Qt::AlignTop);
    rb_->registerWidget(id, t);
    QObject::connect(t, &QToolButton::clicked, rb_, [this, id] { emit rb_->triggered(id); });
    return t;
}

SmallButton* RibbonGroup::button(const QString& id, const QString& text, const QString& kbd) {
    auto* b = new SmallButton(text, kbd, this);
    b->setObjectName(id);  // 타일처럼 id 로 찾을 수 있게 (툴팁 등)
    rb_->registerWidget(id, b);
    QObject::connect(b, &QPushButton::clicked, rb_, [this, id] { emit rb_->triggered(id); });
    return b;
}

void RibbonGroup::column(const QList<QPair<QString, QPair<QString, QString>>>& items) {
    auto* col = new QWidget(this);
    auto* v = new QVBoxLayout(col);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(4);
    for (const auto& it : items) v->addWidget(button(it.first, it.second.first, it.second.second));
    v->addStretch();
    row_->addWidget(col, 0, Qt::AlignTop);
}

void RibbonGroup::rows(const QList<QList<QWidget*>>& rows) {
    auto* col = new QWidget(this);
    auto* v = new QVBoxLayout(col);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(4);
    for (const auto& r : rows) {
        auto* line = new QWidget(col);
        line->setFixedHeight(23);  // SmallButton 높이 — 3줄 = 77 px = ① 3단 열 (리본 높이 불변)
        auto* h = new QHBoxLayout(line);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(4);
        for (auto* w : r) h->addWidget(w, 0, Qt::AlignVCenter);
        h->addStretch();
        v->addWidget(line);
    }
    v->addStretch();
    row_->addWidget(col, 0, Qt::AlignTop);
}

QWidget* RibbonGroup::chipRow(const QString& groupId, const QString& label, const QStringList& items, int sel) {
    auto* w = new QWidget(this);
    auto* h = new QHBoxLayout(w);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    if (!label.isEmpty()) {
        auto* l = new QLabel(label, w);
        l->setStyleSheet(QStringLiteral("color:%1; font-size:12px;").arg(theme::hex(theme::Muted)));
        l->setFixedWidth(32);
        h->addWidget(l);
    }
    for (int i = 0; i < items.size(); ++i) {
        auto* b = new QPushButton(items[i], w);
        b->setProperty("chip", true);
        b->setProperty("first", i == 0);
        b->setProperty("last", i == items.size() - 1);
        b->setProperty("state", i == sel ? "on" : "idle");
        b->setCursor(Qt::PointingHandCursor);
        b->setFocusPolicy(Qt::NoFocus);
        h->addWidget(b);
        rb_->registerChip(groupId, b);
        QObject::connect(b, &QPushButton::clicked, rb_, [this, groupId, i] {
            rb_->setChip(groupId, i);
            emit rb_->chipChosen(groupId, i);
        });
    }
    h->addStretch();
    return w;
}

void RibbonGroup::chips(const QString& groupId, const QString& label, const QStringList& items, int sel) {
    row_->addWidget(chipRow(groupId, label, items, sel), 0, Qt::AlignTop);
}

void RibbonGroup::chipRows(const QList<std::tuple<QString, QString, QStringList, int>>& rows) {
    auto* col = new QWidget(this);
    auto* v = new QVBoxLayout(col);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(4);
    for (const auto& [gid, label, items, sel] : rows) v->addWidget(chipRow(gid, label, items, sel));
    v->addStretch();
    row_->addWidget(col, 0, Qt::AlignTop);
}

// ---- Ribbon ---------------------------------------------------------------
Ribbon::Ribbon(QWidget* parent) : QWidget(parent) {
    setObjectName("Ribbon");
    setAttribute(Qt::WA_StyledBackground, true);
    tabs_ = new QWidget(parent);
    tabs_->setObjectName("Tabs");
    tabs_->setAttribute(Qt::WA_StyledBackground, true);
    tabs_->setFixedHeight(34);
    tabsLay_ = new QHBoxLayout(tabs_);
    tabsLay_->setContentsMargins(12, 0, 12, 0);
    tabsLay_->setSpacing(2);
    tabsLay_->addStretch();
    auto* undo = new QPushButton(QStringLiteral("↶ 되돌리기"), tabs_);
    auto* redo = new QPushButton(QStringLiteral("↷ 다시"), tabs_);
    for (auto* b : {undo, redo}) {
        b->setObjectName("UndoBtn");
        b->setFocusPolicy(Qt::NoFocus);
        tabsLay_->addWidget(b);
    }
    connect(undo, &QPushButton::clicked, this, &Ribbon::undoRequested);
    connect(redo, &QPushButton::clicked, this, &Ribbon::redoRequested);

    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(0, 6, 0, 6);
    stack_ = new QStackedWidget(this);
    v->addWidget(stack_);
    setMinimumHeight(98);
}

int Ribbon::addPage(const QString& name) {
    const int idx = int(tabBtns_.size());
    auto* b = new QPushButton(name, tabs_);
    b->setProperty("tab", true);
    b->setProperty("on", false);
    b->setFocusPolicy(Qt::NoFocus);
    b->setCursor(Qt::PointingHandCursor);
    b->setFixedHeight(34);
    tabsLay_->insertWidget(idx, b);
    tabBtns_.push_back(b);
    connect(b, &QPushButton::clicked, this, [this, idx] { setCurrentPage(idx); });
    auto* page = new QWidget(stack_);
    auto* h = new QHBoxLayout(page);
    h->setContentsMargins(8, 0, 8, 0);
    h->setSpacing(0);
    h->addStretch();
    pageLays_.push_back(h);
    stack_->addWidget(page);
    return idx;
}

RibbonGroup* Ribbon::addGroup(int page, const QString& name) {
    auto* lay = pageLays_[page];
    auto* parent = stack_->widget(page);
    if (lay->count() > 1) {
        auto* sep = new QFrame(parent);
        sep->setProperty("vsep", true);
        sep->setFixedWidth(1);
        lay->insertWidget(lay->count() - 1, sep);
    }
    auto* g = new RibbonGroup(this, name, parent);
    lay->insertWidget(lay->count() - 1, g);
    return g;
}

void Ribbon::setCurrentPage(int i) {
    stack_->setCurrentIndex(i);
    for (int k = 0; k < tabBtns_.size(); ++k) {
        tabBtns_[k]->setProperty("on", k == i);
        tabBtns_[k]->style()->unpolish(tabBtns_[k]);
        tabBtns_[k]->style()->polish(tabBtns_[k]);
    }
    emit pageChanged(i);
}

void Ribbon::setPageLocked(int page, bool locked, const QString& tip) {
    QPushButton* b = tabBtns_.value(page);
    if (!b) return;
    b->setProperty("locked", locked);
    b->setToolTip(locked ? tip : QString());
    b->style()->unpolish(b);
    b->style()->polish(b);
}

void Ribbon::registerWidget(const QString& id, QWidget* w) {
    widgets_[id].push_back(w);
    if (states_.contains(id)) {
        if (auto* t = qobject_cast<Tile*>(w)) t->setToolState(states_[id]);
        if (auto* s = qobject_cast<SmallButton*>(w)) s->setToolState(states_[id]);
    }
}

void Ribbon::registerChip(const QString& groupId, QPushButton* b) { chips_[groupId].push_back(b); }

void Ribbon::setState(const QString& id, ToolState s) {
    states_[id] = s;
    for (QWidget* w : widgets_.value(id)) {
        if (auto* t = qobject_cast<Tile*>(w)) t->setToolState(s);
        if (auto* sb = qobject_cast<SmallButton*>(w)) sb->setToolState(s);
    }
}

void Ribbon::setText(const QString& id, const QString& text) {
    for (QWidget* w : widgets_.value(id)) {
        if (auto* t = qobject_cast<QAbstractButton*>(w)) t->setText(text);
        w->updateGeometry();
    }
}

void Ribbon::setChip(const QString& groupId, int idx) {
    // 같은 groupId 칩 묶음이 여러 탭에 있을 수 있다 → 묶음 크기로 나눠 인덱스 비교
    const auto& list = chips_.value(groupId);
    if (list.isEmpty()) return;
    int perGroup = 0;
    for (auto* b : list) {
        ++perGroup;
        if (b->property("last").toBool()) break;
    }
    for (int k = 0; k < list.size(); ++k) {
        auto* b = list[k];
        b->setProperty("state", (k % perGroup) == idx ? "on" : "idle");
        b->style()->unpolish(b);
        b->style()->polish(b);
    }
}

}  // namespace studio
