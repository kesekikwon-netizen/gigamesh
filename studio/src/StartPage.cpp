#include "StartPage.h"

#include "Theme.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMimeData>
#include <QPushButton>
#include <QSettings>
#include <QTableWidget>
#include <QVBoxLayout>

namespace studio {

QList<RecentItem> loadRecent() {
    QSettings s;
    QList<RecentItem> out;
    const int n = s.beginReadArray("recent");
    for (int i = 0; i < n; ++i) {
        s.setArrayIndex(i);
        RecentItem it{s.value("path").toString(), s.value("name").toString(), s.value("kind").toString(),
                      s.value("dims").toString(), s.value("when").toString(), s.value("slots", 4).toInt()};
        if (!it.path.isEmpty()) out << it;
    }
    s.endArray();
    return out;
}

void pushRecent(const RecentItem& it) {
    auto list = loadRecent();
    for (int i = list.size() - 1; i >= 0; --i)
        if (list[i].path == it.path) list.removeAt(i);
    list.prepend(it);
    while (list.size() > 8) list.removeLast();
    QSettings s;
    s.beginWriteArray("recent", int(list.size()));
    for (int i = 0; i < list.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue("path", list[i].path);
        s.setValue("name", list[i].name);
        s.setValue("kind", list[i].kind);
        s.setValue("dims", list[i].dims);
        s.setValue("when", list[i].when);
        s.setValue("slots", list[i].slotCount);
    }
    s.endArray();
}

namespace {
QLabel* lab(const QString& t, QWidget* p, const QFont& f, const QColor& c) {
    auto* l = new QLabel(t, p);
    l->setFont(f);
    l->setStyleSheet(QStringLiteral("color:%1; background:transparent;").arg(theme::hex(c)));
    return l;
}
QFrame* card(QWidget* p) {
    auto* f = new QFrame(p);
    f->setStyleSheet(QStringLiteral("QFrame#Card{background:%1; border:1px solid %2; border-radius:8px;}")
                         .arg(theme::hex(theme::Surface), theme::hex(theme::Edge)));
    f->setObjectName("Card");
    return f;
}
QLabel* kbd(const QString& t, QWidget* p) {
    auto* l = new QLabel(t, p);
    l->setProperty("kbd", true);
    return l;
}
}  // namespace

StartPage::StartPage(QWidget* parent) : QWidget(parent) {
    setAcceptDrops(true);
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(48, 32, 48, 32);
    root->setSpacing(32);

    // ---- 왼쪽 ----
    auto* left = new QVBoxLayout;
    left->setSpacing(10);
    auto* titleRow = new QHBoxLayout;
    titleRow->addWidget(lab(QStringLiteral("동국유물실측 (DongArch3D)"), this, theme::serifFont(30), theme::Ink));
    auto* tsub = lab(QStringLiteral("Artec 3D 스캐너(Space Spider · Eva · Leo)에서 1:1 보고서 실측 도판·일러스트(SVG)까지"),
                     this, theme::uiFont(12), theme::Muted);
    titleRow->addWidget(tsub, 0, Qt::AlignBottom);
    titleRow->addStretch();
    left->addLayout(titleRow);
    left->addWidget(lab(QStringLiteral("Artec Studio에서 내보낸 유물(PLY · OBJ+JPG · WRL · STL)을 끌어 놓거나 감시 폴더로 자동 연동하세요."),
                        this, theme::uiFont(13), theme::Muted));
    left->addSpacing(8);

    // 이어서 작업 카드
    auto* rc = card(this);
    auto* rh = new QHBoxLayout(rc);
    rh->setContentsMargins(24, 18, 0, 18);
    auto* info = new QVBoxLayout;
    info->setSpacing(2);
    info->addWidget(lab(QStringLiteral("이어서 작업"), rc, theme::uiFont(12), theme::Muted));
    resumeName_ = lab(QString(), rc, theme::serifFont(30), theme::Ink);
    resumePath_ = lab(QString(), rc, theme::monoFont(12), theme::Muted);
    info->addWidget(resumeName_);
    info->addWidget(resumePath_);
    auto* hr = new QFrame(rc);
    hr->setFixedHeight(1);
    hr->setStyleSheet(QStringLiteral("background:%1;").arg(theme::hex(theme::Line)));
    info->addSpacing(8);
    info->addWidget(hr);
    info->addSpacing(8);
    auto* g = new QGridLayout;
    g->setHorizontalSpacing(36);
    g->setVerticalSpacing(2);
    const QStringList heads{QStringLiteral("마지막 작업"), QStringLiteral("실측 슬롯"), QStringLiteral("유물 치수"),
                            QStringLiteral("정위치 기준")};
    for (int i = 0; i < heads.size(); ++i) g->addWidget(lab(heads[i], rc, theme::uiFont(12), theme::Muted), 0, i);
    resumeWhen_ = lab(QString(), rc, theme::uiFont(13, true), theme::Ink);
    resumeSlots_ = lab(QString(), rc, theme::uiFont(13, true), theme::Ink);
    resumeDims_ = lab(QString(), rc, theme::monoFont(12, true), theme::Ink);
    resumeAlign_ = lab(QString(), rc, theme::uiFont(13, true), theme::Ink);
    g->addWidget(resumeWhen_, 1, 0);
    g->addWidget(resumeSlots_, 1, 1);
    g->addWidget(resumeDims_, 1, 2);
    g->addWidget(resumeAlign_, 1, 3);
    g->setColumnStretch(4, 1);
    info->addLayout(g);
    rh->addLayout(info, 1);
    auto* vsep = new QFrame(rc);
    vsep->setFixedWidth(1);
    vsep->setStyleSheet(QStringLiteral("background:%1;").arg(theme::hex(theme::Line)));
    rh->addWidget(vsep);
    auto* act = new QVBoxLayout;
    act->setContentsMargins(24, 0, 24, 0);
    act->addStretch();
    resumeBtn_ = new QPushButton(QStringLiteral("이어서 열기 →"), rc);
    resumeBtn_->setProperty("primary", true);  // R3: 이 화면의 유일한 Action
    resumeBtn_->setMinimumWidth(180);
    resumeBtn_->setFixedHeight(40);
    resumeBtn_->setCursor(Qt::PointingHandCursor);
    act->addWidget(resumeBtn_, 0, Qt::AlignHCenter);
    act->addWidget(lab(QStringLiteral("중심축·단면 슬롯을 그대로 되살립니다"), rc, theme::uiFont(11), theme::Muted), 0, Qt::AlignHCenter);
    act->addStretch();
    rh->addLayout(act);
    left->addWidget(rc);
    connect(resumeBtn_, &QPushButton::clicked, this, [this] {
        if (resumeTarget_.isEmpty()) emit sampleRequested();
        else emit openPathRequested(resumeTarget_);
    });

    // 최근 실측 유물 (삼선표)
    left->addSpacing(14);
    auto* rt = new QHBoxLayout;
    rt->addWidget(lab(QStringLiteral("최근 실측 유물"), this, theme::uiFont(13, true), theme::Ink));
    recentCount_ = lab(QString(), this, theme::monoFont(12), theme::Muted);
    rt->addWidget(recentCount_);
    rt->addStretch();
    left->addLayout(rt);
    table_ = new QTableWidget(0, 5, this);
    table_->setHorizontalHeaderLabels({QStringLiteral("유물 모델"), QStringLiteral("정위치 · 스캐너 프로파일"), QStringLiteral("실측 슬롯"),
                                       QStringLiteral("크기"), QStringLiteral("마지막 작업")});
    table_->verticalHeader()->hide();
    table_->setShowGrid(false);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setFocusPolicy(Qt::NoFocus);
    table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table_->verticalHeader()->setDefaultSectionSize(58);
    table_->setStyleSheet(QStringLiteral(
        "QTableWidget{background:transparent; border:0; border-top:2px solid %1; border-bottom:2px solid %1;}"
        "QHeaderView::section{background:transparent; border:0; border-bottom:1px solid %2; color:%3; font-size:11px; padding:6px 10px;}"
        "QTableWidget::item{border-bottom:1px solid %2; padding:0 10px;}"
        "QTableWidget::item:selected{background:%4; color:%1;}")
                              .arg(theme::hex(theme::Ink), theme::hex(theme::Line), theme::hex(theme::Muted), theme::hex(theme::Wash)));
    table_->setMaximumHeight(36 + 58 * 4);
    connect(table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        const QString p = table_->item(row, 0)->data(Qt::UserRole).toString();
        if (p.isEmpty()) emit sampleRequested();
        else if (p == QLatin1String("@blind")) emit artecBlindSampleRequested();
        else emit openPathRequested(p);
    });
    left->addWidget(table_);

    // 끌어놓기 영역
    left->addSpacing(14);
    auto* drop = new QFrame(this);
    drop->setObjectName("Drop");
    drop->setStyleSheet(QStringLiteral("QFrame#Drop{background:%1; border:1px dashed %2; border-radius:8px;}")
                            .arg(theme::hex(theme::Wash), theme::hex(theme::Ring)));
    drop->setFixedHeight(72);
    auto* dh = new QHBoxLayout(drop);
    dh->addStretch();
    dh->addWidget(lab(QStringLiteral("Artec 3D 파일(PLY · OBJ+JPG · WRL · STL)을 여기에 끌어 놓으세요   또는"), drop,
                      theme::uiFont(13), theme::Ink2));
    auto* openBtn = new QPushButton(QStringLiteral("파일 열기  Ctrl+O"), drop);
    openBtn->setProperty("secondary", true);
    openBtn->setCursor(Qt::PointingHandCursor);
    dh->addWidget(openBtn);
    watchBtn_ = new QPushButton(QStringLiteral("Artec 연동 폴더"), drop);
    watchBtn_->setProperty("secondary", true);
    watchBtn_->setCursor(Qt::PointingHandCursor);
    watchBtn_->setToolTip(QStringLiteral("Artec Studio 내보내기 폴더를 지정하면 파일 저장 즉시 자동으로 열립니다"));
    dh->addWidget(watchBtn_);
    auto* sample = new QPushButton(QStringLiteral("예제 유물"), drop);
    sample->setProperty("secondary", true);
    sample->setCursor(Qt::PointingHandCursor);
    sample->setToolTip(QStringLiteral("기울어진 합성 토기로 정위치·단면·내보내기를 바로 써봅니다"));
    dh->addWidget(sample);
    dh->addStretch();
    connect(openBtn, &QPushButton::clicked, this, &StartPage::openFileRequested);
    connect(watchBtn_, &QPushButton::clicked, this, &StartPage::watchFolderRequested);
    connect(sample, &QPushButton::clicked, this, &StartPage::sampleRequested);
    left->addWidget(drop);
    left->addStretch();
    root->addLayout(left, 2);

    // ---- 오른쪽 안내 카드 ----
    auto* guide = card(this);
    guide->setMaximumWidth(470);
    auto* gv = new QVBoxLayout(guide);
    gv->setContentsMargins(18, 18, 18, 18);
    gv->setSpacing(4);
    gv->addWidget(lab(QStringLiteral("Artec 스캐너 → 세 걸음으로 실측도 완성"), guide, theme::uiFont(14, true), theme::Ink));
    gv->addSpacing(10);
    const struct { const char* t; const char* k; const char* d; } steps[] = {
        {"① Artec 파일 열기 · 자동 전처리", "B",
         "PLY·OBJ(컬러)·WRL·STL을 던지면 턴테이블 부유 노이즈 제거, 단위(m/cm→mm) 환산, 회전 중심축 정렬이 한 번에 끝납니다."},
        {"② 내벽 미스캔 파선 보정 & 디지털 탁본", "W",
         "좁은 아가리 토기 내부 미스캔 구간은 구연부 두께 기준 파선(추정선)으로 자동 연결되고, 외면 쪽(왼쪽)에 D-Tak 탁본이 결합됩니다."},
        {"③ 보고서 도판 · 일러스트(SVG) 저장", "Ctrl+P",
         "1:1 실제 크기(mm) 4레이어 분리 SVG · 벡터 PDF · R12 DXF로 내보내 일러스트레이터와 CAD에서 바로 마감합니다."},
    };
    for (const auto& s : steps) {
        auto* row = new QHBoxLayout;
        row->addWidget(lab(QString::fromUtf8(s.t), guide, theme::uiFont(13, true), theme::Ink));
        row->addWidget(kbd(QString::fromUtf8(s.k), guide));
        row->addStretch();
        gv->addLayout(row);
        auto* d = lab(QString::fromUtf8(s.d), guide, theme::uiFont(12), theme::Muted);
        d->setWordWrap(true);
        gv->addWidget(d);
        gv->addSpacing(10);
    }
    auto* rv = new QVBoxLayout;
    rv->addSpacing(70);
    rv->addWidget(guide);
    rv->addStretch();
    root->addLayout(rv, 1);
    reload();
}

void StartPage::setWatchFolderStatus(const QString& dir, bool active) {
    if (active && !dir.isEmpty()) {
        watchBtn_->setText(QStringLiteral("● Artec 감시 중 (%1)").arg(QFileInfo(dir).fileName()));
        watchBtn_->setToolTip(QStringLiteral("감시 폴더: %1\nArtec Studio에서 내보내면 자동으로 열립니다 (클릭하여 변경/해제)")
                                  .arg(QDir::toNativeSeparators(dir)));
    } else {
        watchBtn_->setText(QStringLiteral("Artec 연동 폴더"));
        watchBtn_->setToolTip(QStringLiteral("Artec Studio 내보내기 폴더를 지정하면 파일 저장 즉시 자동으로 열립니다"));
    }
}

void StartPage::reload() {
    auto list = loadRecent();
    if (!list.isEmpty()) {
        const auto& r = list.first();
        resumeTarget_ = r.path;
        resumeName_->setText(r.name);
        resumePath_->setText(QDir::toNativeSeparators(r.path));
        resumeWhen_->setText(r.when);
        resumeSlots_->setText(QStringLiteral("A–A′ 정면도 외 %1개").arg(std::max(0, r.slotCount - 1)));
        resumeDims_->setText(r.dims);
        resumeAlign_->setText(QStringLiteral("✓ 바닥수평 · 중심축 완료"));
    } else {
        resumeTarget_.clear();
        resumeName_->setText(QStringLiteral("예제_합성토기_01호"));
        resumePath_->setText(QStringLiteral("(내장 예제 — 7° 기울어진 항아리, 기벽 6.8 mm, Artec 컬러)"));
        resumeWhen_->setText(QStringLiteral("오늘 · 내장 예제"));
        resumeSlots_->setText(QStringLiteral("A–A′ 정면도 외 3개"));
        resumeDims_->setText(QStringLiteral("기고 184.8 · 구경 142.0 mm"));
        resumeAlign_->setText(QStringLiteral("✓ 바닥수평 · 중심축 완료"));
        list = {
            {QString(), QStringLiteral("예제_합성토기_01호.ply"), QStringLiteral("✓ 중심축 완료 · Artec Space Spider 급"),
             QStringLiteral("기고 184.8 · 구경 142.0 mm"), QStringLiteral("내장 예제"), 4},
            {QStringLiteral("@blind"), QStringLiteral("Artec_내벽미스캔_단경호_02호.ply"),
             QStringLiteral("✓ 내벽 미스캔 파선 자동보정 · 단경호"), QStringLiteral("기고 190.0 · 구경 124.0 mm"),
             QStringLiteral("Artec 보정 예제"), 4},
        };
    }
    table_->setRowCount(0);
    for (const auto& r : list) {
        const int row = table_->rowCount();
        table_->insertRow(row);
        const bool isBuiltin = r.path.isEmpty() || r.path == QLatin1String("@blind");
        const QString sub = isBuiltin ? QStringLiteral("(더블클릭하여 바로 열기)")
                                      : (QStringLiteral("…\\") + QFileInfo(r.path).fileName());
        auto* a = new QTableWidgetItem(r.name + "\n" + sub);
        a->setData(Qt::UserRole, r.path);
        a->setFont(theme::uiFont(13, true));
        table_->setItem(row, 0, a);
        table_->setItem(row, 1, new QTableWidgetItem(r.kind));
        auto* s = new QTableWidgetItem(QStringLiteral("%1개").arg(r.slotCount));
        s->setFont(theme::monoFont(12));
        table_->setItem(row, 2, s);
        const double mb = isBuiltin ? 3.8 : (QFileInfo(r.path).size() / 1048576.0);
        auto* z = new QTableWidgetItem(QStringLiteral("%1 MB").arg(mb, 0, 'f', 1));
        z->setFont(theme::monoFont(12));
        table_->setItem(row, 3, z);
        auto* w = new QTableWidgetItem(r.when);
        w->setFont(theme::monoFont(12));
        w->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table_->setItem(row, 4, w);
    }
    recentCount_->setText(QString::number(list.size()));
    table_->setVisible(true);
}

void StartPage::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasUrls()) e->acceptProposedAction();
}

void StartPage::dropEvent(QDropEvent* e) {
    const auto urls = e->mimeData()->urls();
    if (!urls.isEmpty()) emit openPathRequested(urls.first().toLocalFile());
}

}  // namespace studio
