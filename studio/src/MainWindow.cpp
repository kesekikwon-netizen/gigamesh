#include "MainWindow.h"

#include "ExportDialog.h"
#include "Ribbon.h"
#include "SheetView.h"
#include "StartPage.h"
#include "Theme.h"
#include "View3D.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDir>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <cmath>

namespace studio {

namespace {
QLabel* kbdLabel(const QString& t, QWidget* p) {
    auto* l = new QLabel(t, p);
    l->setProperty("kbd", true);
    return l;
}
QFrame* vsep(QWidget* p) {
    auto* f = new QFrame(p);
    f->setFixedSize(1, 18);
    f->setStyleSheet(QStringLiteral("background:%1;").arg(theme::hex(theme::Line)));
    return f;
}
const char* kShadeNames[] = {"음영: 실사", "음영: 깊이 음영", "음영: 디지털 탁본 (D-Tak)"};
// 리본 「단면 위치」 단추 글자 (D5 기본 / R 반전)
QString sideText(SectionSide s) {
    return s == SectionSide::Right ? QStringLiteral("외면 왼쪽·단면 오른쪽") : QStringLiteral("단면 왼쪽·외면 오른쪽 (반전)");
}
}  // namespace

MainWindow::MainWindow() {
    setWindowTitle(QStringLiteral("동국유물실측 (DongArch3D) 1.0"));
    resize(1560, 920);
    setAcceptDrops(true);  // 작업 화면 어디에나 Artec 파일을 끌어다 놓으면 즉시 열림
    pages_ = new QStackedWidget(this);
    setCentralWidget(pages_);

    start_ = new StartPage(pages_);
    pages_->addWidget(start_);
    connect(start_, &StartPage::openFileRequested, this, &MainWindow::openFileDialog);
    connect(start_, &StartPage::openPathRequested, this, [this](const QString& p) { openPath(p); });
    connect(start_, &StartPage::sampleRequested, this, [this] { openSample(); });
    connect(start_, &StartPage::artecBlindSampleRequested, this, [this] { openArtecBlindSample(); });
    connect(start_, &StartPage::watchFolderRequested, this, &MainWindow::toggleWatchFolder);

    watcher_ = new QFileSystemWatcher(this);
    watchTimer_ = new QTimer(this);
    watchTimer_->setSingleShot(true);
    connect(watcher_, &QFileSystemWatcher::directoryChanged, this, [this](const QString&) {
        pendingWatchSize_ = -1;
        watchTimer_->start(450);
    });
    connect(watchTimer_, &QTimer::timeout, this, &MainWindow::checkWatchFolder);

    auto* work = new QWidget(pages_);
    auto* v = new QVBoxLayout(work);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    ribbon_ = new Ribbon(work);
    v->addWidget(ribbon_->tabsBar());
    v->addWidget(ribbon_);
    ctx_ = buildCtxBar();
    v->addWidget(ctx_);
    auto* ws = new QWidget(work);
    auto* wh = new QHBoxLayout(ws);
    wh->setContentsMargins(0, 0, 0, 0);
    wh->setSpacing(0);
    slotPanel_ = buildSlotPanel();
    view3d_ = new View3DPane(ws);
    view3d_->setMinimumWidth(240);
    sheet_ = new SheetPane(ws);
    wh->addWidget(slotPanel_);
    wh->addWidget(view3d_, 27);
    wh->addWidget(sheet_, 73);
    v->addWidget(ws, 1);
    v->addWidget(buildStatusBar());
    pages_->addWidget(work);

    buildRibbon();
    sheet_->canvas()->setDoc(&doc_);
    connect(sheet_, &SheetPane::maximizeToggled, this, &MainWindow::toggleMaximize);
    connect(view3d_, &View3DPane::collapseRequested, this, &MainWindow::toggleMaximize);
    connect(view3d_->canvas(), &View3DCanvas::cutAngleDragged, this, [this](double d) { setFrontAngle(doc_.frontAngle() + d); });
    connect(sheet_->canvas(), &SheetCanvas::cursorMm, this, [this](double r, double h, double wall, bool inside) {
        if (!inside || !doc_.poseConfirmed()) {  // 확정 전에는 도판 좌표가 없다
            stCursor_->clear();
            return;
        }
        lastHoverH_ = h;
        const QString fa = theme::hex(theme::Faint);
        stCursor_->setText(QStringLiteral("반경 <b>R %1</b><span style='color:%4'> mm</span>&nbsp;&nbsp;높이 <b>H %2</b><span style='color:%4'> mm</span>&nbsp;&nbsp;두께 <b>%3</b><span style='color:%4'> mm</span>")
                               .arg(r, 0, 'f', 1)
                               .arg(doc_.heightRef() == HeightRef::Rim ? h - doc_.dims().height : h, 0, 'f', 1)
                               .arg(wall > 0 ? QString::number(wall, 'f', 1) : QStringLiteral("—"), fa));
    });
    const QString savedWatch = QSettings().value("artec/watchDir").toString();
    if (!savedWatch.isEmpty() && QDir(savedWatch).exists()) applyWatchFolder(savedWatch);
    qApp->installEventFilter(this);
    showHome();
}

// ---- 리본 구성 (목업 ① 그대로) ------------------------------------------------
void MainWindow::buildRibbon() {
    auto fileGroup = [this](int pg) {
        auto* g = ribbon_->addGroup(pg, QStringLiteral("파일"));
        g->tile("open", QStringLiteral("열기"), "open",
                QStringLiteral("Artec 3D 유물 열기 — PLY · OBJ(+JPG) · WRL · STL (Ctrl+O)"));
        g->tile("recent", QStringLiteral("최근 ▾"), "recent", QStringLiteral("최근 실측 유물 및 Artec 감시 폴더 설정"));
    };
    auto alignGroup = [this](int pg) {
        auto* g = ribbon_->addGroup(pg, QStringLiteral("유물 정위치 · 중심축"));
        g->tile("axis", QStringLiteral("중심축 맞춤"), "axis",
                QStringLiteral("구연부 PCA + 회전 중심축 자동 정렬 · 부유 노이즈 제거"));
        g->column({{"refBase", {QStringLiteral("바닥 수평 맞춤"), "B"}},
                   {"refRim", {QStringLiteral("구연부 수평 맞춤"), "T"}},
                   {"rotate", {QStringLiteral("정면 회전 ±1°"), ", ."}}});
    };
    auto lineGroup = [this](int pg) {
        auto* g = ribbon_->addGroup(pg, QStringLiteral("실측선 추출"));
        g->tile("cut", QStringLiteral("단면선"), "cut",
                QStringLiteral("단면선 도구 (S) · Artec 미스캔 내벽 파선 자동 보정(W)"));
        g->tile("outline", QStringLiteral("외곽선"), "outline", QStringLiteral("외곽선 도구 (O)"));
        g->column({{"combined", {sideText(doc_.sectionSide()), "R"}},
                   {"cutMove", {QStringLiteral("절단 위치 이동 (1°)"), "[ ]"}},
                   {"addSlot", {QStringLiteral("단면 추가 (횡단면)"), "N"}}});
    };
    auto simplifyGroup = [this](int pg) {
        auto* g = ribbon_->addGroup(pg, QStringLiteral("선 다듬기 · 간략화"));
        g->chipRows({{"simplify", QStringLiteral("간격"), {"0.1", "0.3", "0.5 mm", "1.0"}, 2},
                     {"curve", QStringLiteral("곡선"), {QStringLiteral("원본"), QStringLiteral("매끄럽게 (Catmull)")}, 1}});
    };
    auto showGroup = [this](int pg) {
        auto* g = ribbon_->addGroup(pg, QStringLiteral("도판 표시 (겹침)"));
        g->tile("shade", QStringLiteral("유물 음영"), "shade", QStringLiteral("유물 음영 · Artec 컬러 / 디지털 탁본 (I)"));
        g->tile("lines", QStringLiteral("단면·외곽"), "profile", QStringLiteral("단면·외곽선 표시 (L)"));
        g->tile("grid", QStringLiteral("방안 눈금"), "grid", QStringLiteral("방안 눈금 (V)"));
        g->column({{"shadeMode", {QString::fromUtf8(kShadeNames[2]), ""}}, {"gridInfo", {QStringLiteral("눈금 1 mm / 숫자 50 mm"), ""}}});
    };
    auto exportGroup = [this](int pg) {
        auto* g = ribbon_->addGroup(pg, QStringLiteral("내보내기"));
        g->tile("export", QStringLiteral("도면"), "doc", QStringLiteral("보고서 도면 (Ctrl+P)"));
        g->tile("svg", QStringLiteral("SVG 일러스트"), "pen", QStringLiteral("레이어 분리 SVG (1:1 mm)"));
        g->tile("dxf", QStringLiteral("DXF"), "dxf", QStringLiteral("CAD DXF R12"));
    };
    const int pFile = ribbon_->addPage(QStringLiteral("파일"));
    pHome_ = ribbon_->addPage(QStringLiteral("홈"));
    pBase_ = ribbon_->addPage(QStringLiteral("① 바닥 정하기"));
    const int pMeasure = ribbon_->addPage(QStringLiteral("측정·탁본"));
    const int pExport = ribbon_->addPage(QStringLiteral("내보내기"));
    {
        auto* g = ribbon_->addGroup(pFile, QStringLiteral("파일 · Artec 연동"));
        g->tile("open", QStringLiteral("열기"), "open", QStringLiteral("유물 열기 (Ctrl+O)"));
        g->tile("watch", QStringLiteral("Artec 연동"), "recent",
                QStringLiteral("Artec Studio 내보내기 폴더 실시간 자동 감지"));
        g->tile("sample", QStringLiteral("예제 유물"), "lithic", QStringLiteral("내장 합성 토기 (Artec 컬러)"));
        g->tile("blindSample", QStringLiteral("미스캔 예제"), "cut",
                QStringLiteral("좁은 아가리 토기 내벽 미스캔 파선 자동보정 예제"));
        g->tile("home", QStringLiteral("시작 화면"), "doc");
    }
    fileGroup(pHome_);
    alignGroup(pHome_);
    lineGroup(pHome_);
    simplifyGroup(pHome_);
    showGroup(pHome_);
    exportGroup(pHome_);
    // ① 바닥 정하기: 파일 · 바닥판에 올려놓기 · 확정 (디자인 v2, 손 실측: 실측판 위에 올려놓기)
    fileGroup(pBase_);
    {
        auto* g = ribbon_->addGroup(pBase_, QStringLiteral("바닥판에 올려놓기"));
        g->tile("poseAuto", QStringLiteral("자동 세우기"), "axis",
                QStringLiteral("자동 세우기 — 구연부를 찾아 회전 중심축을 수직으로 세우고 바닥판에 놓습니다 (입이 대략 위로 오게 돌린 뒤 누르세요)"));
        g->column({{"poseX", {QStringLiteral("X 90°"), ""}},
                   {"poseY", {QStringLiteral("Y 90°"), ""}},
                   {"poseZ", {QStringLiteral("Z 90°"), ""}}});
        g->column({{"poseFlip", {QStringLiteral("뒤집기"), ""}},
                   {"poseReset", {QStringLiteral("처음 자세로"), ""}},
                   {"poseDrop", {QStringLiteral("바닥에 내려놓기"), ""}}});
    }
    {
        auto* g = ribbon_->addGroup(pBase_, QStringLiteral("확정"));
        g->tile("poseConfirm", QStringLiteral("이 자세로 확정"), "check", QStringLiteral("이 자세로 확정 (Enter)"));
    }
    const QList<QPair<QString, QString>> poseTips = {
        {"poseX", QStringLiteral("X 축(바닥판 가로, 왼→오른쪽)을 축으로 +90° 돌립니다 (오른손 법칙) · Shift 를 누른 채 누르면 −90°")},
        {"poseY", QStringLiteral("Y 축(바닥판 세로, 앞→뒤)을 축으로 +90° 돌립니다 (오른손 법칙) · Shift 를 누른 채 누르면 −90°")},
        {"poseZ", QStringLiteral("Z 축(바닥판에 수직, 아래→위)을 축으로 +90° 돌립니다 (위에서 보면 반시계) · Shift 를 누른 채 누르면 −90°")},
        {"poseFlip", QStringLiteral("위아래 뒤집기 — X 축으로 180° (정면에서 보면 좌우는 그대로)")},
        {"poseReset", QStringLiteral("처음 자세로 — 스캔이 들어온 자세 그대로 되돌립니다")},
        {"poseDrop", QStringLiteral("바닥에 내려놓기 — 가장 낮은 점이 바닥판(Z 0 mm)에 닿게 내립니다")}};
    for (const auto& [id, tip] : poseTips)
        for (auto* b : ribbon_->findChildren<QPushButton*>(id)) b->setToolTip(tip);
    for (auto* b : ribbon_->findChildren<QPushButton*>("combined"))
        b->setToolTip(QStringLiteral("국내 보고서 표준 (D5) · R 로 좌/우 반전 — 같은 단면을 거울상으로 그립니다"));
    simplifyGroup(pMeasure);
    showGroup(pMeasure);
    exportGroup(pExport);
    ribbon_->setCurrentPage(pHome_);

    for (const char* id : {"export"}) ribbon_->setState(id, ToolState::Primary);  // R3: 화면당 하나
    ribbon_->setState("poseConfirm", ToolState::Primary);  // R3: ① 바닥 정하기 페이지의 주 단추 (확정 뒤 Idle)
    for (const char* id : {"shade", "lines", "grid", "combined", "refBase"}) ribbon_->setState(id, ToolState::On);
    connect(ribbon_, &Ribbon::triggered, this, &MainWindow::onAction);
    connect(ribbon_, &Ribbon::chipChosen, this, &MainWindow::onChip);
    connect(ribbon_, &Ribbon::undoRequested, this, [this] {
        if (!requirePose() || undo_.isEmpty()) return;
        redo_.push_back({doc_.frontAngle(), doc_.heightRef(), doc_.simplifyTol(), doc_.smooth()});
        restore(undo_.takeLast());
    });
    connect(ribbon_, &Ribbon::redoRequested, this, [this] {
        if (!requirePose() || redo_.isEmpty()) return;
        undo_.push_back({doc_.frontAngle(), doc_.heightRef(), doc_.simplifyTol(), doc_.smooth()});
        restore(redo_.takeLast());
    });
}

QWidget* MainWindow::buildCtxBar() {
    auto* w = new QWidget(this);
    w->setObjectName("CtxBar");
    w->setAttribute(Qt::WA_StyledBackground, true);
    w->setFixedHeight(36);
    auto* h = new QHBoxLayout(w);
    h->setContentsMargins(12, 0, 12, 0);
    h->setSpacing(10);
    ctxBadge_ = new QLabel(w);
    ctxBadge_->setObjectName("ToolBadge");
    ctxBadge_->setFixedHeight(24);
    ctxHint_ = new QLabel(w);
    ctxRight_ = new QLabel(w);
    ctxRight_->setFont(theme::monoFont(12));
    h->addWidget(ctxBadge_);
    h->addWidget(ctxHint_);
    h->addWidget(vsep(w));
    ctxCutKeys_ = {kbdLabel("[ ]", w), new QLabel(QStringLiteral("절단면 1° 이동"), w),
                   kbdLabel("W", w),   new QLabel(QStringLiteral("미스캔 내벽 파선보정"), w),
                   kbdLabel("- +", w), new QLabel(QStringLiteral("추정 두께 ±0.5mm"), w),
                   kbdLabel("1~5", w), new QLabel(QStringLiteral("슬롯 저장"), w),
                   kbdLabel("R", w),   new QLabel(QStringLiteral("좌/우 반전"), w),
                   kbdLabel("Esc", w), new QLabel(QStringLiteral("내려놓기"), w)};
    ctxBaseKeys_ = {kbdLabel("Enter", w), new QLabel(QStringLiteral("확정"), w),  // R8: ① 바닥 정하기
                    kbdLabel("Shift", w), new QLabel(QStringLiteral("−90°"), w)};
    for (auto* x : ctxCutKeys_) h->addWidget(x);
    for (auto* x : ctxBaseKeys_) {
        h->addWidget(x);
        x->hide();
    }
    h->addStretch();
    h->addWidget(ctxRight_);
    w->hide();  // R1: 도구를 손에 들었을 때만
    return w;
}

QWidget* MainWindow::buildSlotPanel() {
    auto* w = new QWidget(this);
    w->setObjectName("SlotPanel");
    w->setAttribute(Qt::WA_StyledBackground, true);
    w->setFixedWidth(176);
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(0);
    auto* head = new QWidget(w);
    head->setObjectName("PaneHead");
    head->setAttribute(Qt::WA_StyledBackground, true);
    head->setFixedHeight(32);
    auto* h = new QHBoxLayout(head);
    h->setContentsMargins(10, 0, 8, 0);
    auto* t = new QLabel(QStringLiteral("실측 도면 목록"), head);
    t->setProperty("ph", true);
    slotCount_ = new QLabel(head);
    slotCount_->setFont(theme::monoFont(12));
    slotCount_->setStyleSheet(QStringLiteral("color:%1;").arg(theme::hex(theme::Muted)));
    auto* add = new QPushButton(QStringLiteral("+ 추가"), head);
    add->setProperty("sbtn", true);
    add->setFocusPolicy(Qt::NoFocus);
    add->setToolTip(QStringLiteral("커서 높이에 횡단면 슬롯 추가 (N)"));
    connect(add, &QPushButton::clicked, this, [this] { onAction("addSlot"); });
    h->addWidget(t);
    h->addWidget(slotCount_);
    h->addStretch();
    h->addWidget(add);
    v->addWidget(head);
    slotList_ = new QListWidget(w);
    slotList_->setObjectName("SlotList");
    slotList_->setDragDropMode(QAbstractItemView::InternalMove);
    slotList_->setFocusPolicy(Qt::NoFocus);
    slotList_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    v->addWidget(slotList_, 1);
    auto* foot = new QLabel(QStringLiteral("누르면 해당 도판으로 전환 · 끌어서 보고서 도판 순서 변경"), w);
    foot->setWordWrap(true);
    foot->setStyleSheet(QStringLiteral("color:%1; font-size:11px; padding:8px 10px; border-top:1px solid %2;")
                            .arg(theme::hex(theme::Muted), theme::hex(theme::Line)));
    v->addWidget(foot);
    connect(slotList_, &QListWidget::currentRowChanged, this, [this](int r) {
        if (r >= 0 && r != slot_) selectSlot(r);
    });
    connect(slotList_->model(), &QAbstractItemModel::rowsMoved, this, [this] {
        std::vector<SectionSlot> reordered;
        for (int i = 0; i < slotList_->count(); ++i) {
            const int src = slotList_->item(i)->data(Qt::UserRole).toInt();
            reordered.push_back(doc_.slotList()[size_t(src)]);
        }
        doc_.slotList() = std::move(reordered);
        refreshSlots();
    });
    return w;
}

QWidget* MainWindow::buildStatusBar() {
    auto* w = new QWidget(this);
    w->setObjectName("StatusBar");
    w->setAttribute(Qt::WA_StyledBackground, true);
    w->setFixedHeight(30);
    auto* h = new QHBoxLayout(w);
    h->setContentsMargins(12, 0, 12, 0);
    h->setSpacing(14);
    stHint_ = new QLabel(w);
    stCursor_ = new QLabel(w);
    stCursor_->setFont(theme::monoFont(12));
    stScale_ = new QLabel(QStringLiteral("축척 <span style='font-family:Consolas'>1:1 (실제크기)</span>"), w);
    stRef_ = new QLabel(w);
    stDirty_ = new QLabel(w);
    h->addWidget(stHint_, 1);
    h->addWidget(stCursor_);
    h->addWidget(vsep(w));
    h->addWidget(stScale_);
    h->addWidget(vsep(w));
    h->addWidget(stRef_);
    h->addWidget(stDirty_);
    return w;
}

// ---- 열기 & Artec Studio 감시 폴더 --------------------------------------------
void MainWindow::openFileDialog() {
    QSettings s;
    const QString p = QFileDialog::getOpenFileName(
        this, QStringLiteral("Artec 3D 유물 열기"), s.value("open/dir").toString(),
        QStringLiteral("Artec / 3D 유물 (*.ply *.obj *.wrl *.stl);;모든 파일 (*.*)"));
    if (p.isEmpty()) return;
    s.setValue("open/dir", QFileInfo(p).absolutePath());
    openPath(p);
}

void MainWindow::toggleWatchFolder() {
    if (!watchDir_.isEmpty()) {
        QMenu m(this);
        m.addAction(QStringLiteral("Artec 감시 폴더 변경…"), this, [this] {
            const QString d = QFileDialog::getExistingDirectory(this, QStringLiteral("Artec Studio 내보내기 폴더 선택"), watchDir_);
            if (!d.isEmpty()) applyWatchFolder(d);
        });
        m.addAction(QStringLiteral("감시 중지 (%1)").arg(QDir::toNativeSeparators(watchDir_)), this, [this] {
            applyWatchFolder(QString());
        });
        m.exec(QCursor::pos());
        return;
    }
    QSettings s;
    const QString initDir = s.value("open/dir").toString();
    const QString d = QFileDialog::getExistingDirectory(
        this, QStringLiteral("Artec Studio 내보내기(Export) 폴더 선택 — 파일 저장 시 자동으로 열립니다"), initDir);
    if (!d.isEmpty()) applyWatchFolder(d);
}

void MainWindow::applyWatchFolder(const QString& dir) {
    if (!watcher_->directories().isEmpty()) watcher_->removePaths(watcher_->directories());
    watchDir_ = dir;
    QSettings().setValue("artec/watchDir", dir);
    if (!dir.isEmpty() && QDir(dir).exists()) {
        watcher_->addPath(dir);
        ribbon_->setState("watch", ToolState::On);
        start_->setWatchFolderStatus(dir, true);
        setHint(QStringLiteral("● Artec Studio 감시 폴더 연결됨: <span style='font-family:Consolas'>%1</span> — Artec에서 내보내면 자동으로 열립니다.")
                    .arg(QDir::toNativeSeparators(dir)));
    } else {
        ribbon_->setState("watch", ToolState::Idle);
        start_->setWatchFolderStatus(QString(), false);
        setHint(QStringLiteral("Artec Studio 감시 폴더를 해제했습니다."));
    }
}

void MainWindow::checkWatchFolder() {
    if (watchDir_.isEmpty()) return;
    QDir d(watchDir_);
    const QFileInfoList files = d.entryInfoList({"*.ply", "*.obj", "*.wrl", "*.stl"}, QDir::Files, QDir::Time);
    if (files.isEmpty()) return;
    const QFileInfo newest = files.first();
    if (newest.lastModified().secsTo(QDateTime::currentDateTime()) > 20) return;
    if (newest.absoluteFilePath() == doc_.path() && pendingWatchFile_.isEmpty()) return;
    // 파일 쓰기가 진행 중이면 크기가 멈출 때까지 대기
    if (pendingWatchFile_ != newest.absoluteFilePath() || pendingWatchSize_ != newest.size() || newest.size() == 0) {
        pendingWatchFile_ = newest.absoluteFilePath();
        pendingWatchSize_ = newest.size();
        watchTimer_->start(450);
        return;
    }
    const QString target = pendingWatchFile_;
    pendingWatchFile_.clear();
    pendingWatchSize_ = -1;
    if (openPath(target)) {
        setHint(QStringLiteral("✓ Artec Studio 연동: <b>%1</b> 자동 로드 — ① 바닥 정하기에서 자세를 확정하세요")
                    .arg(QFileInfo(target).fileName()));
    }
}

bool MainWindow::openPath(const QString& path) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString err;
    const bool ok = doc_.loadFile(path, &err);  // 들어온 자세 그대로 (확정 전)
    QApplication::restoreOverrideCursor();
    if (!ok) {
        QMessageBox::warning(this, QStringLiteral("유물을 열 수 없습니다"), err);
        return false;
    }
    recordRecent();
    setWindowTitle(QStringLiteral("%1 — 동국유물실측 (DongArch3D) 1.0").arg(QFileInfo(path).fileName()));
    undo_.clear();
    redo_.clear();
    slot_ = 0;
    pages_->setCurrentIndex(1);
    ribbon_->setCurrentPage(pBase_);  // ① 바닥 정하기
    sheet_->canvas()->setDoc(&doc_);
    refreshAll(true);
    return true;
}

void MainWindow::recordRecent() {
    if (doc_.path().isEmpty()) return;
    const bool done = doc_.poseConfirmed();
    const auto& d = doc_.dims();
    const QString state = !done ? QStringLiteral("바닥 미정")
                                : (doc_.aligned() ? QStringLiteral("✓ 중심축 완료") : QStringLiteral("정위치 필요"));
    pushRecent({doc_.path(), doc_.name(), QStringLiteral("%1 · %2").arg(state, doc_.scannerProfile()),
                done ? QStringLiteral("기고 %1 · 구경 %2 mm").arg(d.height, 0, 'f', 1).arg(d.rim, 0, 'f', 1)
                     : QStringLiteral("치수 — (바닥 확정 전)"),
                QDateTime::currentDateTime().toString("MM-dd HH:mm"), int(doc_.slotList().size())});
}

void MainWindow::openSample(bool poseStage) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    doc_.loadSample(!poseStage);
    QApplication::restoreOverrideCursor();
    setWindowTitle(QStringLiteral("%1 — 동국유물실측 (DongArch3D) 1.0").arg(doc_.name()));
    undo_.clear();
    redo_.clear();
    slot_ = 0;
    pages_->setCurrentIndex(1);
    ribbon_->setCurrentPage(poseStage ? pBase_ : pHome_);
    sheet_->canvas()->setDoc(&doc_);
    refreshAll(true);
}

void MainWindow::openArtecBlindSample(bool poseStage) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    doc_.loadArtecBlindSample(!poseStage);
    QApplication::restoreOverrideCursor();
    setWindowTitle(QStringLiteral("%1 — 동국유물실측 (DongArch3D) 1.0").arg(doc_.name()));
    undo_.clear();
    redo_.clear();
    slot_ = 0;
    pages_->setCurrentIndex(1);
    ribbon_->setCurrentPage(poseStage ? pBase_ : pHome_);
    sheet_->canvas()->setDoc(&doc_);
    refreshAll(true);
    if (poseStage) return;
    setHint(QStringLiteral("Artec 내벽 미스캔 자동 보정: 구연부 실측 두께(%1 mm)를 기준으로 미스캔 동체부·저부 내벽을 빨강 파선으로 복원했습니다 (W: 켜기/끄기, -/+: 두께 조절).")
                .arg(doc_.estimatedWallMm(), 0, 'f', 1));
}

void MainWindow::showHome() {
    start_->reload();
    pages_->setCurrentIndex(0);
}

// ---- 상태 갱신 --------------------------------------------------------------
void MainWindow::refreshAll(bool geometryChanged) {
    if (geometryChanged) dirtyExport_ = true;
    applyOptions();
    refreshSlots();
    sheet_->refresh(doc_, slot_);
    view3d_->refresh(doc_, slot_);
    if (geometryChanged) sheet_->canvas()->docChanged();
    ribbon_->setState("refBase", doc_.heightRef() == HeightRef::Base ? ToolState::On : ToolState::Idle);
    ribbon_->setState("refRim", doc_.heightRef() == HeightRef::Rim ? ToolState::On : ToolState::Idle);
    const bool base = doc_.isLoaded() && !doc_.poseConfirmed();
    ribbon_->setState("poseConfirm", base ? ToolState::Primary : ToolState::Idle);  // R3: 확정 전 주 단추 = poseConfirm
    for (auto* w : ctxCutKeys_) w->setVisible(!base);
    for (auto* w : ctxBaseKeys_) w->setVisible(base);
    if (base) {
        // ① 바닥 정하기 = 「▣ 바닥 정하기」를 손에 든 상태 (R1: ctx 줄 보임)
        if (!hand_.isEmpty()) setHandTool(QString());
        const bool grounded = doc_.isGrounded();
        stRef_->setText(grounded ? QStringLiteral("단위 mm · 놓음 (확정 전)") : QStringLiteral("단위 mm · 바닥 미정"));
        stDirty_->clear();
        setHint(grounded ? QStringLiteral("④ 수평·기준선 — 3D 바닥판에서 자세를 보고 「이 자세로 확정」 (Enter)")
                         : QStringLiteral("③ 바닥 놓기 — 입이 위로 오게 X·Y·Z 90°·뒤집기로 돌리거나 「자동 세우기」를 누르세요"));
        ctxBadge_->setText(QStringLiteral("▣ 바닥 정하기"));
        ctxHint_->setText(QStringLiteral("스캔은 위아래 없이 들어옵니다 · X·Y·Z 90° 단추나 「자동 세우기」로 세우세요 · Enter 확정"));
        ctxRight_->setText(doc_.groundNote());
        ctx_->show();
        return;
    }
    if (hand_.isEmpty()) ctx_->hide();  // 확정하면 바닥 정하기 ctx 를 내려놓는다
    stRef_->setText(QStringLiteral("단위 mm · 정위치 %1").arg(doc_.aligned() ? (doc_.heightRef() == HeightRef::Base ? QStringLiteral("바닥수평·중심축")
                                                                                                             : QStringLiteral("구연부 0 mm·중심축"))
                                                                         : QStringLiteral("미정렬")));
    stDirty_->setText(dirtyExport_ ? QStringLiteral("<span style='color:%1'>●</span> 도면 저장 전").arg(theme::hex(theme::Caution)) : QString());
    if (!doc_.aligned()) setHint(QStringLiteral("다음: [중심축 맞춤] 으로 유물을 바로 세우세요 — 단면·치수는 정위치 후에 정확합니다."));
    else if (dirtyExport_)
        setHint(QStringLiteral("실측선이 완성되었습니다 — 다음: <span style='font-family:Consolas'>Ctrl+P</span> 「도면」으로 보고서 도판을 만들거나 「SVG 일러스트」로 내보내세요."));
    else setHint(QStringLiteral("저장했습니다. 다른 슬롯을 고르거나 [단면 추가] 로 횡단면을 더하세요."));
    ctxRight_->setText(QStringLiteral("절단 각도 <b>%1°</b> (정면 중심축) · 점 %2 → <b>%3개</b> (%4 mm 간략화)")
                           .arg(doc_.frontAngle(), 0, 'f', 1)
                           .arg(QLocale(QLocale::English).toString(doc_.rawPointCount()))
                           .arg(doc_.pointCount())
                           .arg(doc_.simplifyTol(), 0, 'f', 1));
}

void MainWindow::refreshSlots() {
    QSignalBlocker b(slotList_);
    slotList_->clear();
    const auto& sl = doc_.slotList();
    for (int i = 0; i < int(sl.size()); ++i) {
        const auto& s = sl[size_t(i)];
        auto* it = new QListWidgetItem(slotList_);
        it->setData(Qt::UserRole, i);
        auto* w = new QWidget;
        w->setAttribute(Qt::WA_TranslucentBackground);
        auto* v = new QVBoxLayout(w);
        v->setContentsMargins(10, 8, 10, 8);
        v->setSpacing(2);
        auto* n = new QLabel(s.name, w);
        n->setStyleSheet(QStringLiteral("font-weight:700; font-size:12px; background:transparent;"));
        const QString meta = s.kind == SlotKind::Vertical
                                 ? QStringLiteral("%1 · %2 mm").arg(s.meta).arg(doc_.dims().height, 0, 'f', 1)
                                 : QStringLiteral("높이 H %1 mm %2").arg(s.z, 0, 'f', 1).arg(s.meta);
        auto* m = new QLabel(meta, w);
        m->setFont(theme::monoFont(11));
        m->setStyleSheet(QStringLiteral("color:%1; background:transparent;").arg(theme::hex(theme::Muted)));
        auto* nt = new QLabel(s.note, w);
        nt->setStyleSheet(QStringLiteral("color:%1; font-size:11px; background:transparent;").arg(theme::hex(theme::Muted)));
        v->addWidget(n);
        v->addWidget(m);
        v->addWidget(nt);
        it->setSizeHint(QSize(170, 74));
        slotList_->setItemWidget(it, w);
    }
    slotList_->setCurrentRow(slot_);
    slotCount_->setText(QString::number(sl.size()));
}

void MainWindow::selectSlot(int i) {
    if (i < 0 || i >= int(doc_.slotList().size())) return;
    slot_ = i;
    const auto& s = doc_.slotList()[size_t(i)];
    if (s.kind == SlotKind::Vertical && std::fabs(s.angleDeg - doc_.frontAngle()) > 1e-6) {
        doc_.setFrontAngle(s.angleDeg);
        refreshAll(true);
    } else {
        applyOptions();
        sheet_->refresh(doc_, slot_);
        view3d_->refresh(doc_, slot_);
        slotList_->setCurrentRow(slot_);
    }
}

void MainWindow::applyOptions() {
    opts_.slot = slot_;
    opts_.shade = ribbon_->state("shade") == ToolState::On;
    opts_.grid = ribbon_->state("grid") == ToolState::On;
    const bool lines = ribbon_->state("lines") == ToolState::On;
    opts_.section = lines;
    opts_.outline = lines;
    sheet_->canvas()->setOptions(opts_);
}

void MainWindow::setHint(const QString& s) { stHint_->setText(s); }

bool MainWindow::requirePose() {
    if (!doc_.isLoaded() || doc_.poseConfirmed()) return true;
    setHint(QStringLiteral("먼저 ① 바닥 정하기에서 「이 자세로 확정」 (Enter)"));
    ribbon_->setCurrentPage(pBase_);
    return false;
}

void MainWindow::confirmPose() {
    if (!doc_.isLoaded()) return;
    if (doc_.poseConfirmed()) {
        setHint(QStringLiteral("이미 확정했습니다 — 자세를 바꾸면 다시 확정하세요."));
        return;
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);
    doc_.confirmPose();
    QApplication::restoreOverrideCursor();
    slot_ = 0;
    ribbon_->setCurrentPage(pHome_);
    refreshAll(true);
    sheet_->canvas()->fitView();
    recordRecent();
}

// ---- 도구·동작 ----------------------------------------------------------------
void MainWindow::setHandTool(const QString& id) {
    for (const char* t : {"cut", "outline"}) ribbon_->setState(t, ToolState::Idle);
    hand_ = id;
    if (id.isEmpty()) {
        ctx_->hide();
        view3d_->canvas()->setCutTool(false);
        return;
    }
    ribbon_->setState(id, ToolState::Hand);  // R3: 손에 든 도구는 하나
    ctxBadge_->setText(id == "cut" ? QStringLiteral("✂ 단면·외곽선 편집 중") : QStringLiteral("◠ 외곽선 편집 중"));
    ctxHint_->setText(id == "cut" ? QStringLiteral("3D 뷰를 좌우로 끌어 절단면 A–A′ 각도를 돌리거나 1~5 로 슬롯에 저장합니다.")
                                  : QStringLiteral("간격 칩으로 외곽선 점 수를, 곡선 칩으로 매끄러움을 고릅니다."));
    view3d_->canvas()->setCutTool(id == "cut");
    ctx_->show();
}

void MainWindow::pushUndo() {
    undo_.push_back({doc_.frontAngle(), doc_.heightRef(), doc_.simplifyTol(), doc_.smooth()});
    if (undo_.size() > 100) undo_.removeFirst();
    redo_.clear();
}

void MainWindow::restore(const ViewState& s) {
    doc_.setHeightRef(s.ref);
    doc_.setSimplifyTol(s.simplify);
    doc_.setSmooth(s.smooth);
    doc_.setFrontAngle(s.angle);
    static const double tol[] = {0.1, 0.3, 0.5, 1.0};
    for (int i = 0; i < 4; ++i)
        if (std::fabs(tol[i] - s.simplify) < 1e-9) ribbon_->setChip("simplify", i);
    ribbon_->setChip("curve", s.smooth ? 1 : 0);
    refreshAll(true);
}

void MainWindow::setFrontAngle(double deg) {
    if (!doc_.isLoaded() || !doc_.poseConfirmed()) return;
    doc_.setFrontAngle(deg);
    refreshAll(true);
}

void MainWindow::saveSlotAngle(int k) {
    auto& sl = doc_.slotList();
    int seen = 0;
    for (int i = 0; i < int(sl.size()); ++i) {
        if (sl[size_t(i)].kind != SlotKind::Vertical) continue;
        if (++seen == k) {
            sl[size_t(i)].angleDeg = doc_.frontAngle();
            sl[size_t(i)].note = QStringLiteral("절단 각도 %1°").arg(doc_.frontAngle(), 0, 'f', 1);
            slot_ = i;
            refreshAll(false);
            setHint(QStringLiteral("슬롯 %1 에 절단 각도 %2° 를 저장했습니다.").arg(k).arg(doc_.frontAngle(), 0, 'f', 1));
            return;
        }
    }
    const QChar c = QChar('A' + std::min(25, seen));
    sl.push_back({QStringLiteral("%1–%1′ 정면 실측도").arg(c), QStringLiteral("종단면"),
                  QStringLiteral("절단 각도 %1°").arg(doc_.frontAngle(), 0, 'f', 1), SlotKind::Vertical, doc_.frontAngle(), 0.0});
    slot_ = int(sl.size()) - 1;
    refreshAll(false);
}

void MainWindow::openExport(ExportFormat f) {
    if (!doc_.isLoaded()) return;
    ExportDialog dlg(doc_, slot_, opts_, f, this);
    if (dlg.exec() == QDialog::Accepted && !dlg.savedPath().isEmpty()) {
        dirtyExport_ = false;
        refreshAll(false);
        setHint(QStringLiteral("저장했습니다: <span style='font-family:Consolas'>%1</span>").arg(QDir::toNativeSeparators(dlg.savedPath())));
    }
}

void MainWindow::onAction(const QString& id) {
    if (id == "open") return openFileDialog();
    if (id == "watch") return toggleWatchFolder();
    if (id == "sample") return openSample();
    if (id == "blindSample") return openArtecBlindSample();
    if (id == "home") return showHome();
    if (id == "recent") {
        QMenu m(this);
        for (const auto& r : loadRecent()) m.addAction(r.name, this, [this, p = r.path] { openPath(p); });
        m.addSeparator();
        m.addAction(QStringLiteral("내장 예제 1: 합성 토기 (Artec 컬러)"), this, [this] { openSample(); });
        m.addAction(QStringLiteral("내장 예제 2: Artec 내벽 미스캔 단경호 (파선 보정)"), this, [this] { openArtecBlindSample(); });
        m.addSeparator();
        m.addAction(watchDir_.isEmpty() ? QStringLiteral("Artec Studio 연동 감시 폴더 지정…")
                                        : QStringLiteral("● Artec 감시 중 (%1)…").arg(QFileInfo(watchDir_).fileName()),
                    this, &MainWindow::toggleWatchFolder);
        m.exec(QCursor::pos());
        return;
    }
    if (!doc_.isLoaded()) return;
    // 확정 전에는 단면·치수·내보내기 동작을 하지 않는다 (표시 토글 shade·lines·grid·shadeMode 는 허용)
    static const QStringList needPose = {"cut", "outline", "cutMove", "addSlot", "rotate",
                                         "refBase", "refRim", "export", "svg", "dxf", "combined"};
    if (needPose.contains(id) && !requirePose()) return;
    if (id == "poseX" || id == "poseY" || id == "poseZ") {
        // 고정 월드축 +90° (오른손 법칙), Shift 를 누른 채면 −90°. 자세 조작은 되돌리기에 쌓지 않는다.
        const bool neg = QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier);
        doc_.rotatePose(id == "poseX" ? PoseAxis::X : id == "poseY" ? PoseAxis::Y : PoseAxis::Z, neg ? -90.0 : 90.0);
        slot_ = 0;
        refreshAll(true);
    } else if (id == "poseFlip" || id == "poseReset" || id == "poseDrop") {
        if (id == "poseFlip") doc_.flipPose();
        else if (id == "poseReset") doc_.resetPose();
        else doc_.dropToFloor();
        slot_ = 0;
        refreshAll(true);
    } else if (id == "poseAuto") {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        doc_.autoAlign();  // 자동 세우기 — 확정은 사람이 (Enter)
        QApplication::restoreOverrideCursor();
        slot_ = 0;
        refreshAll(true);
    } else if (id == "poseConfirm") {
        confirmPose();
    } else if (id == "axis") {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        pushUndo();
        doc_.autoAlign();  // 중심축 맞춤 = 자동 세우기 → 확정 (한 번 누르기)
        doc_.confirmPose();
        QApplication::restoreOverrideCursor();
        slot_ = 0;
        refreshAll(true);
    } else if (id == "refBase" || id == "refRim") {
        pushUndo();
        doc_.setHeightRef(id == "refBase" ? HeightRef::Base : HeightRef::Rim);
        refreshAll(false);
        sheet_->canvas()->update();
    } else if (id == "rotate") {
        pushUndo();
        setFrontAngle(doc_.frontAngle() + 1.0);
    } else if (id == "cut" || id == "outline") {
        setHandTool(hand_ == id ? QString() : id);
    } else if (id == "cutMove") {
        pushUndo();
        setFrontAngle(doc_.frontAngle() + 1.0);
    } else if (id == "combined") {
        // D5 좌/우 반전 (R): 같은 단면의 거울상. 다시 계산하지 않고, 되돌리기에 쌓지 않는다.
        doc_.toggleSectionSide();
        ribbon_->setText("combined", sideText(doc_.sectionSide()));
        refreshAll(true);  // 도판·3D·음영(보는 방향)을 다시 그린다
        setHint(doc_.sectionSide() == SectionSide::Right
                    ? QStringLiteral("단면 위치: 외면 왼쪽 · 단면 오른쪽 (국내 보고서 표준)")
                    : QStringLiteral("단면 위치: 단면 왼쪽 · 외면 오른쪽 (R 반전) — R 을 다시 누르면 되돌립니다"));
    } else if (id == "addSlot") {
        const double h = doc_.dims().height;
        const double z = (lastHoverH_ > 0.5 && lastHoverH_ < h - 0.5) ? lastHoverH_ : doc_.dims().bellyZ;
        auto& sl = doc_.slotList();
        int nh = 0;
        for (const auto& s : sl) nh += s.kind == SlotKind::Horizontal;
        const QChar c = QChar('B' + std::min(24, nh));
        sl.push_back({QStringLiteral("%1–%1′ 횡단면").arg(c), QStringLiteral("수평절단"), QStringLiteral("커서 높이에서 추가"),
                      SlotKind::Horizontal, 0.0, z});
        slot_ = int(sl.size()) - 1;
        refreshAll(false);
    } else if (id == "shade" || id == "lines" || id == "grid") {
        ribbon_->setState(id, ribbon_->state(id) == ToolState::On ? ToolState::Idle : ToolState::On);
        applyOptions();
    } else if (id == "shadeMode") {
        opts_.shadeMode = ShadeMode((int(opts_.shadeMode) + 1) % 3);
        ribbon_->setText("shadeMode", QString::fromUtf8(kShadeNames[int(opts_.shadeMode)]));
        ribbon_->setState("shade", ToolState::On);
        applyOptions();
    } else if (id == "export") {
        openExport(ExportFormat::Pdf);
    } else if (id == "svg") {
        openExport(ExportFormat::Svg);
    } else if (id == "dxf") {
        openExport(ExportFormat::Dxf);
    }
}

void MainWindow::onChip(const QString& group, int idx) {
    if (!doc_.isLoaded()) return;
    pushUndo();
    if (group == "simplify") {
        static const double tol[] = {0.1, 0.3, 0.5, 1.0};
        doc_.setSimplifyTol(tol[idx]);
    } else if (group == "curve") {
        doc_.setSmooth(idx == 1);
    }
    refreshAll(true);
}

void MainWindow::toggleMaximize() {
    maximized_ = !maximized_;
    slotPanel_->setVisible(!maximized_);
    view3d_->setVisible(!maximized_);
    sheet_->setMaximized(maximized_);
    sheet_->canvas()->fitView();
}

bool MainWindow::eventFilter(QObject* o, QEvent* e) {
    if (e->type() == QEvent::KeyPress && pages_->currentIndex() == 1 && isActiveWindow()) {
        auto* k = static_cast<QKeyEvent*>(e);
        if (k->key() == Qt::Key_Tab && k->modifiers() == Qt::NoModifier && !qobject_cast<QLineEdit*>(QApplication::focusWidget())) {
            toggleMaximize();
            return true;
        }
    }
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::keyPressEvent(QKeyEvent* e) {
    const auto mod = e->modifiers();
    if (mod & Qt::ControlModifier) {
        switch (e->key()) {
            case Qt::Key_O: return openFileDialog();
            case Qt::Key_P: return onAction("export");
            case Qt::Key_Z: return emit ribbon_->undoRequested();
            case Qt::Key_Y: return emit ribbon_->redoRequested();
            default: break;
        }
    }
    if (pages_->currentIndex() != 1 || !doc_.isLoaded()) return QMainWindow::keyPressEvent(e);
    if (!doc_.poseConfirmed()) {  // ① 바닥 정하기
        switch (e->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter: return confirmPose();
            case Qt::Key_Escape: return;  // 바닥 정하기 ctx 유지
            case Qt::Key_S: case Qt::Key_O: case Qt::Key_B: case Qt::Key_T: case Qt::Key_N: case Qt::Key_R:
            case Qt::Key_Comma: case Qt::Key_Period: case Qt::Key_BracketLeft: case Qt::Key_BracketRight:
            case Qt::Key_1: case Qt::Key_2: case Qt::Key_3: case Qt::Key_4: case Qt::Key_5:
            case Qt::Key_W: case Qt::Key_Minus: case Qt::Key_Underscore: case Qt::Key_Equal: case Qt::Key_Plus:
                requirePose();
                return;
            default: break;  // I · L · V 표시 토글 등은 그대로
        }
    }
    switch (e->key()) {
        case Qt::Key_Escape: setHandTool(QString()); break;
        case Qt::Key_S: onAction("cut"); break;
        case Qt::Key_O: onAction("outline"); break;
        case Qt::Key_B: onAction("refBase"); break;
        case Qt::Key_T: onAction("refRim"); break;
        case Qt::Key_N: onAction("addSlot"); break;
        case Qt::Key_I: onAction("shade"); break;
        case Qt::Key_L: onAction("lines"); break;
        case Qt::Key_V: onAction("grid"); break;
        case Qt::Key_W:
            doc_.setInferInnerWall(!doc_.inferInnerWall());
            refreshAll(true);
            setHint(doc_.inferInnerWall()
                        ? QStringLiteral("Artec 내벽 미스캔 파선 자동 보정을 켰습니다 (추정 두께 %1 mm · -/+ 로 조절).")
                              .arg(doc_.estimatedWallMm(), 0, 'f', 1)
                        : QStringLiteral("Artec 내벽 미스캔 자동 보정을 껐습니다 (스캔된 원본 단면만 표시)."));
            break;
        case Qt::Key_Minus:
        case Qt::Key_Underscore:
            doc_.setEstimatedWallMm(doc_.estimatedWallMm() - 0.5);
            refreshAll(true);
            setHint(QStringLiteral("미스캔 추정 기벽 두께: %1 mm").arg(doc_.estimatedWallMm(), 0, 'f', 1));
            break;
        case Qt::Key_Equal:
        case Qt::Key_Plus:
            doc_.setEstimatedWallMm(doc_.estimatedWallMm() + 0.5);
            refreshAll(true);
            setHint(QStringLiteral("미스캔 추정 기벽 두께: %1 mm").arg(doc_.estimatedWallMm(), 0, 'f', 1));
            break;
        case Qt::Key_Comma: pushUndo(); setFrontAngle(doc_.frontAngle() - 1.0); break;
        case Qt::Key_Period: pushUndo(); setFrontAngle(doc_.frontAngle() + 1.0); break;
        case Qt::Key_BracketLeft: pushUndo(); setFrontAngle(doc_.frontAngle() - 1.0); break;
        case Qt::Key_BracketRight: pushUndo(); setFrontAngle(doc_.frontAngle() + 1.0); break;
        case Qt::Key_R: onAction("combined"); break;  // D5 좌/우 반전
        case Qt::Key_1: case Qt::Key_2: case Qt::Key_3: case Qt::Key_4: case Qt::Key_5:
            saveSlotAngle(e->key() - Qt::Key_0);
            break;
        default: QMainWindow::keyPressEvent(e);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* e) {
    if (e->mimeData()->hasUrls()) e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* e) {
    const auto urls = e->mimeData()->urls();
    if (!urls.isEmpty()) openPath(urls.first().toLocalFile());
}

void MainWindow::closeEvent(QCloseEvent* e) {
    QSettings().setValue("window/geometry", saveGeometry());
    e->accept();
}

}  // namespace studio
