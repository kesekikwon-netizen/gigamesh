// 화면 검증: ② 막으로 자르기 페이지 (JOB-006 수용 기준 1-18)
// 표기 함수(CutPageText.h) → Ribbon 단독 → 실제 MainWindow(오프스크린, 예제 메시)를 한 실행 안에서 차례로 확인한다.
// 허용오차: 계산을 바꾸지 않으므로 화면 문자열은 정확히 같음(== · startsWith · endsWith), 각도 판정 경계 1°·표기 0.1° 는
// 원문(디자인 v2 1460, 표기 정수 12°). 기다림 상한 15 s 는 판정값이 아닌 실패 감지용(예제 원본 계산 1.36 s 의 10배+).
// 시간에 따라 결과가 갈리는 판정은 없다 — "끄는 중" 은 왼쪽 단추를 누른 채(놓을 때까지 확정 없음)로 본다.
#include "CutPageText.h"
#include "MainWindow.h"
#include "Ribbon.h"
#include "Theme.h"
#include "View3D.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QPushButton>
#include <QStackedWidget>
#include <cstdio>
#include <functional>
#include <utility>

using namespace studio;
namespace cp = studio::cutpage;

static int fails = 0;
#define CHECK(c, ...)                         \
    do {                                      \
        if (!(c)) {                           \
            std::printf("FAIL %s: ", #c);     \
            std::printf(__VA_ARGS__);         \
            std::printf("\n");                \
            ++fails;                          \
        }                                     \
    } while (0)

static void eq(const QString& got, const QString& want, const char* what) {
    CHECK(got == want, "%s: '%s' != '%s'", what, qUtf8Printable(got), qUtf8Printable(want));
}

// ---- 기다림 · 입력 -------------------------------------------------------------
// 조건이 참이 될 때까지 이벤트를 돌린다 (상한 15 s — 넘으면 false)
static bool wait(const std::function<bool()>& cond) {
    QElapsedTimer t;
    t.start();
    while (!cond()) {
        if (t.elapsed() > 15000) return false;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    return true;
}
// 조건 없이 100 ms 동안 이벤트를 돌린다 — 위치·폭을 읽기 전에 (다시 배치는 LayoutRequest 로 미뤄진다)
static void settle() {
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < 100) QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}
static void key(QWidget& w, QEvent::Type type, int k, Qt::KeyboardModifiers m = Qt::NoModifier) {
    QKeyEvent e(type, k, m);
    QApplication::sendEvent(&w, &e);
}
static void tap(QWidget& w, int k, Qt::KeyboardModifiers m = Qt::NoModifier) {  // 누름·뗌 (자동 반복 아님)
    key(w, QEvent::KeyPress, k, m);
    key(w, QEvent::KeyRelease, k, m);
}
static void mouse(QWidget* c, QEvent::Type type, QPoint p, Qt::MouseButton b, Qt::MouseButtons bs) {
    QMouseEvent e(type, QPointF(p), QPointF(c->mapToGlobal(p)), b, bs, Qt::NoModifier);
    QApplication::sendEvent(c, &e);
}

// ---- 위젯 찾기 -----------------------------------------------------------------
static QList<QPushButton*> tabButtons(Ribbon* rb) {
    QList<QPushButton*> out;
    for (auto* b : rb->tabsBar()->findChildren<QPushButton*>(QString(), Qt::FindDirectChildrenOnly))
        if (b->property("tab").toBool()) out << b;
    return out;
}
static QLabel* groupName(QWidget* rb, const QString& prefix) {
    for (auto* l : rb->findChildren<QLabel*>())
        if (l->property("gname").toBool() && l->text().startsWith(prefix)) return l;
    return nullptr;
}
static QLabel* labelWithText(QWidget* parent, const QString& t) {
    for (auto* l : parent->findChildren<QLabel*>())
        if (l->text() == t) return l;
    return nullptr;
}
static QString text(QWidget& w, const char* name) {
    auto* l = w.findChild<QLabel*>(QString::fromLatin1(name));
    return l ? l->text() : QStringLiteral("<%1 없음>").arg(QString::fromLatin1(name));
}
static QColor textColor(QPushButton* b) { return b->palette().color(QPalette::ButtonText); }
// 페이지 안 그룹 이름 (찾은 순서 = 만든 순서 = 왼쪽→오른쪽, JOB-007 기준 13·15)
static QStringList groupNames(QWidget* page) {
    QStringList out;
    for (auto* l : page->findChildren<QLabel*>())
        if (l->property("gname").toBool()) out << l->text();
    return out;
}
// 페이지 안 주 단추(Primary) 타일의 objectName 들
static QStringList primaryTiles(QWidget* page) {
    QStringList out;
    for (auto* t : page->findChildren<Tile*>())
        if (t->toolState() == ToolState::Primary) out << t->objectName();
    return out;
}
// 슬롯 목록의 지금 항목 위젯 안에 글자가 t 인 QLabel 이 있는지 (지운 항목 위젯은 보지 않는다)
static bool slotHas(QListWidget* l, const QString& t) {
    for (int i = 0; i < l->count(); ++i)
        if (QWidget* w = l->itemWidget(l->item(i)); w && labelWithText(w, t)) return true;
    return false;
}

// ---- A-1 표기 함수 (기준 1-4) ---------------------------------------------------
static void testText() {
    const std::pair<double, const char*> angles[] = {{0, "0°"},     {12, "12°"},     {37.5, "37.5°"},
                                                     {-20, "−20°"}, {-0.04, "0°"},   {180, "180°"},
                                                     {12.04, "12°"}, {12.06, "12.1°"}, {-179.5, "−179.5°"}};
    for (const auto& [d, s] : angles) eq(cp::angleText(d), QString::fromUtf8(s), "[1] angleText");

    eq(cp::plateTitle(QChar('A'), 12), QStringLiteral("A–A′ 실시간 단면 — 12°"), "[2] plateTitle");
    eq(cp::plateSub(SectionSide::Right), QStringLiteral("— 3D에서 막을 움직이는 동안 바로 다시 그림 · 외면 왼쪽 · 단면 오른쪽"), "[2] plateSub(Right)");
    eq(cp::plateSub(SectionSide::Left), QStringLiteral("— 3D에서 막을 움직이는 동안 바로 다시 그림 · 단면 왼쪽 · 외면 오른쪽"), "[2] plateSub(Left)");

    for (double d : {0.0, 1.0, -1.0, 179.0, 179.5, 180.0, -180.0}) CHECK(cp::atAxis(d, CutAxis::Y), "[3] Y 막 %g", d);
    for (double d : {1.5, 12.0, 90.0, -20.0}) CHECK(!cp::atAxis(d, CutAxis::Y), "[3] Y 막 아님 %g", d);
    for (double d : {90.0, 89.0, 91.0, -90.0, -89.0}) CHECK(cp::atAxis(d, CutAxis::X), "[3] X 막 %g", d);
    for (double d : {88.5, 92.0, 0.0, 105.0}) CHECK(!cp::atAxis(d, CutAxis::X), "[3] X 막 아님 %g", d);
    const QString snap = QStringLiteral("막 방향 (누르면 바로 맞춤)");
    eq(cp::axisGroupName(0), snap, "[3] axisGroupName(0)");
    eq(cp::axisGroupName(90.5), snap, "[3] axisGroupName(90.5)");
    eq(cp::axisGroupName(12), QStringLiteral("막 방향 · 지금 12° (누르면 맞춤)"), "[3] axisGroupName(12)");
    eq(cp::axisGroupName(-20), QStringLiteral("막 방향 · 지금 −20° (누르면 맞춤)"), "[3] axisGroupName(-20)");

    const std::pair<size_t, const char*> faces[] = {{26180, "2.6만 면"}, {77000, "7.7만 면"}, {35000, "3.5만 면"},
                                                    {250000, "25만 면"}, {10000, "1만 면"},   {9800, "9,800 면"}};
    for (const auto& [n, s] : faces) eq(cp::faceText(n), QString::fromUtf8(s), "[4] faceText");
    eq(cp::ctxRight(12, 26180, 77000, false), QStringLiteral("막 12° · 끄는 동안 2.6만 면 미리보기 · 놓으면 원본 7.7만 면 다시 계산"),
       "[4] ctxRight(놓음)");
    eq(cp::ctxRight(12, 26180, 77000, true), QStringLiteral("막 12° · 끄는 중 2.6만 면 미리보기 — 음영·외곽·치수·두께는 놓은 뒤"),
       "[4] ctxRight(끄는 중)");
}

// ---- A-2 Ribbon 단독 (기준 5-7) -------------------------------------------------
static void testRibbon() {
    Ribbon rb;
    rb.addPage(QStringLiteral("가"));
    rb.addPage(QStringLiteral("나"));
    QList<int> got;
    QObject::connect(&rb, &Ribbon::pageChanged, &rb, [&got](int p) { got << p; });
    rb.setCurrentPage(1);
    CHECK(got == QList<int>{1} && rb.currentPage() == 1, "[5] setCurrentPage(1) → 받은 값 %d개", int(got.size()));
    const auto tb = tabButtons(&rb);
    CHECK(tb.size() == 2, "[5] 탭 단추 %d개", int(tb.size()));
    if (tb.size() == 2) {
        tb[0]->click();
        CHECK(!got.isEmpty() && got.last() == 0 && rb.currentPage() == 0, "[5] 탭 가 누름 → 0");
        rb.setPageLocked(1, true, QStringLiteral("이유"));
        CHECK(tb[1]->property("locked").toBool() && tb[1]->toolTip() == QStringLiteral("이유"), "[6] 잠금");
        rb.setPageLocked(1, false, QStringLiteral("이유"));
        CHECK(!tb[1]->property("locked").toBool() && tb[1]->toolTip().isEmpty(), "[6] 잠금 풀기");
    }
    RibbonGroup* g = rb.addGroup(0, QStringLiteral("옛 이름"));
    g->setTitle(QStringLiteral("새 이름"));
    QLabel* name = groupName(g, QString());
    CHECK(name && name->text() == QStringLiteral("새 이름"), "[7] setTitle");
    SmallButton* b = g->button("bt", QStringLiteral("+"));
    g->rows({{b}});
    int hits = 0;
    QObject::connect(&rb, &Ribbon::triggered, &rb, [&hits](const QString& id) { if (id == "bt") ++hits; });
    b->click();
    CHECK(hits == 1, "[7] triggered(bt) %d번", hits);
    rb.setState("bt", ToolState::On);
    CHECK(b->property("state").toString() == "on", "[7] state=%s", qUtf8Printable(b->property("state").toString()));
}

// ---- A-3 화면 통합 (기준 8-17) --------------------------------------------------
static void testWindow() {
    MainWindow w;
    w.resize(1560, 920);
    w.show();
    w.openSample(true);  // 확정 전 (① 바닥 정하기)
    auto* rb = w.findChild<Ribbon*>();
    auto* ctxBar = w.findChild<QWidget*>(QStringLiteral("CtxBar"));
    auto* ctxHint = w.findChild<QLabel*>(QStringLiteral("CtxHint"));
    auto* canvas = w.findChild<View3DCanvas*>();
    auto* slotList = w.findChild<QListWidget*>(QStringLiteral("SlotList"));
    if (!rb || !ctxBar || !ctxHint || !canvas || !slotList) {
        CHECK(false, "화면 위젯을 찾지 못함");
        return;
    }
    const int pHome = 1, pBase = 2, pCut = 3;
    auto title = [&w] { return text(w, "SheetTitle"); };
    auto ctxRight = [&w] { return text(w, "CtxRight"); };
    const QString t0 = QStringLiteral("A–A′ 실시간 단면 — 0°"), t90 = QStringLiteral("A–A′ 실시간 단면 — 90°"),
                  t92 = QStringLiteral("A–A′ 실시간 단면 — 92°"), t105 = QStringLiteral("A–A′ 실시간 단면 — 105°");

    // [8] 탭 순서·잠김
    const auto tb = tabButtons(rb);
    QStringList names;
    for (auto* b : tb) names << b->text();
    const QStringList want = {QStringLiteral("파일"), QStringLiteral("홈"), QStringLiteral("① 바닥 정하기"),
                              QStringLiteral("② 막으로 자르기"), QStringLiteral("③ 도면 편집"), QStringLiteral("내보내기")};
    CHECK(names == want, "[8] 탭: %s", qUtf8Printable(names.join(QStringLiteral(" · "))));
    if (tb.size() != 6) return;
    QPushButton* cutTab = tb[pCut];
    CHECK(rb->currentPage() == pBase, "[8] 현재 페이지 %d", rb->currentPage());
    CHECK(cutTab->property("locked").toBool(), "[8] ② 잠김");
    CHECK(cutTab->toolTip().contains(QStringLiteral("바닥 확정 후 사용")), "[8] 툴팁 '%s'", qUtf8Printable(cutTab->toolTip()));
    CHECK(textColor(cutTab) == theme::Faint, "[8] ② 글자색 %s", qUtf8Printable(textColor(cutTab).name()));
    cutTab->click();
    CHECK(rb->currentPage() == pBase, "[8] 잠긴 ② 누름 → 페이지 %d", rb->currentPage());
    CHECK(text(w, "StHint").contains(QStringLiteral("먼저 ① 바닥 정하기에서")), "[8] 상태줄 '%s'", qUtf8Printable(text(w, "StHint")));
    eq(text(w, "ToolBadge"), QStringLiteral("▣ 바닥 정하기"), "[8] ToolBadge");
    CHECK(ctxBar->isVisible(), "[8] CtxBar 보임");
    CHECK(textColor(cutTab) == theme::Faint, "[8] 누른 뒤 ② 글자색 %s", qUtf8Printable(textColor(cutTab).name()));
    // [J7-12] ③ 도면 편집도 확정 전에는 ② 와 같이 잠김 (누르면 ① 로 안내)
    const int pEdit = 4;
    QPushButton* editTab = tb[pEdit];
    CHECK(editTab->property("locked").toBool(), "[J7-12] ③ 잠김");
    CHECK(editTab->toolTip().contains(QStringLiteral("바닥 확정 후 사용")), "[J7-12] ③ 툴팁 '%s'", qUtf8Printable(editTab->toolTip()));
    CHECK(textColor(editTab) == theme::Faint, "[J7-12] ③ 글자색 %s", qUtf8Printable(textColor(editTab).name()));
    editTab->click();
    CHECK(rb->currentPage() == pBase, "[J7-12] 잠긴 ③ 누름 → 페이지 %d", rb->currentPage());
    CHECK(text(w, "StHint").contains(QStringLiteral("먼저 ① 바닥 정하기에서")), "[J7-12] 상태줄 '%s'", qUtf8Printable(text(w, "StHint")));

    // [9] 확정 → ②
    key(w, QEvent::KeyPress, Qt::Key_Return);
    CHECK(rb->currentPage() == pCut, "[9] 확정 뒤 페이지 %d", rb->currentPage());
    CHECK(!cutTab->property("locked").toBool() && cutTab->toolTip().isEmpty(), "[9] 잠김 풀림");
    CHECK(!editTab->property("locked").toBool() && editTab->toolTip().isEmpty(), "[J7-12] 확정 뒤 ③ 잠김 풀림");
    CHECK(textColor(cutTab) == theme::Ink, "[9] ② 글자색 %s", qUtf8Printable(textColor(cutTab).name()));
    CHECK(ctxBar->isVisible(), "[9] CtxBar 보임");
    eq(text(w, "ToolBadge"), QStringLiteral("✂ 막으로 자르기"), "[9] ToolBadge");
    eq(ctxHint->text(), cp::kCtxHint, "[9] CtxHint");
    eq(ctxHint->toolTip(), cp::kCtxHint, "[9] CtxHint 툴팁");
    eq(text(w, "StHint"), cp::kStatus, "[9] StHint");
    eq(title(), t0, "[9] SheetTitle");
    CHECK(text(w, "SheetSub").startsWith(cp::plateSub(SectionSide::Right)), "[9] SheetSub '%s'", qUtf8Printable(text(w, "SheetSub")));
    CHECK(text(w, "View3DTitle").contains(QStringLiteral("3D — 막으로 자르기")), "[9] View3DTitle '%s'", qUtf8Printable(text(w, "View3DTitle")));
    eq(text(w, "CutAngle"), QStringLiteral("0°"), "[9] CutAngle");
    QLabel* axisName = groupName(rb, QStringLiteral("막 방향"));
    CHECK(axisName && axisName->text() == QStringLiteral("막 방향 (누르면 바로 맞춤)"), "[9] 막 방향 그룹 이름");
    auto* ax = w.findChild<Tile*>(QStringLiteral("axX"));
    auto* ay = w.findChild<Tile*>(QStringLiteral("axY"));
    auto* az = w.findChild<Tile*>(QStringLiteral("axZ"));
    auto* cutTile = w.findChild<Tile*>(QStringLiteral("cut"));
    auto* plus = w.findChild<QPushButton*>(QStringLiteral("cutPlus"));
    auto* minus = w.findChild<QPushButton*>(QStringLiteral("cutMinus"));
    if (!axisName || !ax || !ay || !az || !cutTile || !plus || !minus) {
        CHECK(false, "[9] ② 리본 위젯을 찾지 못함");
        return;
    }
    CHECK(ay->toolState() == ToolState::On && ax->toolState() == ToolState::Idle, "[9] Y 막 켜짐 · X 막 대기");
    CHECK(!az->isEnabled() && az->toolTip().startsWith(QStringLiteral("🔒")) && az->toolTip().contains(QStringLiteral("다음 판")),
          "[9] Z 막 사용 불가 '%s'", qUtf8Printable(az->toolTip()));
    CHECK(ctxRight().startsWith(QStringLiteral("막 0° · 끄는 동안 ")) && ctxRight().endsWith(QStringLiteral(" 다시 계산")),
          "[9] CtxRight '%s'", qUtf8Printable(ctxRight()));
    eq(ax->text(), QStringLiteral("X 막 옆에서"), "[9] axX 글자");
    eq(ay->text(), QStringLiteral("Y 막 정면에서"), "[9] axY 글자");
    eq(az->text(), QStringLiteral("Z 막 높이로"), "[9] axZ 글자");
    QLabel* sideName = groupName(rb, QStringLiteral("단면 위치"));
    CHECK(groupName(rb, QStringLiteral("막 움직이기 (회전판)")) && sideName, "[9] 그룹 이름 막 움직이기·단면 위치");
    for (const QString& s : {QStringLiteral("☑ 중심선에 고정 (돌리기만)"), QStringLiteral("각도"), QStringLiteral("끄는 동안 실시간"),
                             QStringLiteral("국내 보고서 표준"), QStringLiteral("사용자 도판으로 확인")})
        CHECK(labelWithText(rb, s), "[9] 리본 글 '%s' 없음", qUtf8Printable(s));
    const int tileH = cutTile->sizeHint().height();
    CHECK(ax->sizeHint().height() == tileH && ay->sizeHint().height() == tileH && az->sizeHint().height() == tileH,
          "[9] 막 방향 타일 높이 %d · 홈 타일 %d", ax->sizeHint().height(), tileH);
    auto* stack = rb->findChild<QStackedWidget*>(QString(), Qt::FindDirectChildrenOnly);
    CHECK(stack && stack->widget(pCut)->sizeHint().height() <= stack->widget(pBase)->sizeHint().height(), "[9] ② 페이지 높이 %d > ① %d",
          stack ? stack->widget(pCut)->sizeHint().height() : -1, stack ? stack->widget(pBase)->sizeHint().height() : -1);
    settle();
    auto* angle = w.findChild<QLabel*>(QStringLiteral("CutAngle"));
    CHECK(axisName->width() >= axisName->fontMetrics().horizontalAdvance(cp::axisGroupName(-179.5)), "[9] 막 방향 이름 폭 %d", axisName->width());
    CHECK(angle && angle->width() >= angle->fontMetrics().horizontalAdvance(QStringLiteral("−179.5°")), "[9] 각도 칸 폭");
    std::printf("[9] ribbon height=%d px (JOB-005 계산값 107)\n", rb->height());
    std::printf("[9] ctxHint width=%d / sizeHint=%d px · ctx minimumSizeHint width=%d px (offscreen 글꼴)\n", ctxHint->width(),
                ctxHint->sizeHint().width(), ctxBar->minimumSizeHint().width());
    const int sideX = sideName ? sideName->mapTo(rb, QPoint()).x() : -1;
    std::printf("[9] 단면 위치 x=%d\n", sideX);

    // [10] X 막 (진행 중 제스처 취소 포함)
    plus->click();
    ax->click();  // 이벤트 처리 없이 바로
    eq(title(), t90, "[10] X 막");
    eq(text(w, "CutAngle"), QStringLiteral("90°"), "[10] CutAngle");
    CHECK(ax->toolState() == ToolState::On && ay->toolState() == ToolState::Idle, "[10] X 막 켜짐");
    eq(text(w, "StHint"), cp::kStatus, "[10] StHint");
    for (int i = 0; i < 5; ++i) settle();
    eq(title(), t90, "[10] 취소된 [+] 가 확정되지 않음");
    key(w, QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    eq(title(), t0, "[10] Ctrl+Z → 0°");
    CHECK(ctxRight().startsWith(QStringLiteral("막 0° · 끄는 동안 ")), "[10] CtxRight '%s'", qUtf8Printable(ctxRight()));
    ax->click();
    eq(title(), t90, "[10] 다시 X 막");

    // [11] [+] 두 번 = 합치기 한 번 / 끄는 중 표시
    plus->click();
    plus->click();
    eq(title(), t90, "[11] 이벤트 처리 전");
    CHECK(wait([&] { return ctxRight().startsWith(QStringLiteral("막 92° · 끄는 동안 ")); }), "[11] 92° 확정 '%s'", qUtf8Printable(ctxRight()));
    eq(title(), t92, "[11] SheetTitle");
    eq(text(w, "CutAngle"), QStringLiteral("92°"), "[11] CutAngle");
    eq(axisName->text(), QStringLiteral("막 방향 · 지금 92° (누르면 맞춤)"), "[11] 그룹 이름");
    CHECK(ax->toolState() == ToolState::Idle && ay->toolState() == ToolState::Idle, "[11] 막 방향 둘 다 대기");
    key(w, QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    eq(title(), t90, "[11] Ctrl+Z (제스처 하나 = 한 칸)");
    mouse(canvas, QEvent::MouseButtonPress, {100, 100}, Qt::LeftButton, Qt::LeftButton);
    mouse(canvas, QEvent::MouseMove, {104, 100}, Qt::NoButton, Qt::LeftButton);
    CHECK(wait([&] {
              return ctxRight().startsWith(QStringLiteral("막 92° · 끄는 중 ")) &&
                     ctxRight().endsWith(QStringLiteral("— 음영·외곽·치수·두께는 놓은 뒤"));
          }),
          "[11] 끄는 중 '%s'", qUtf8Printable(ctxRight()));
    eq(title(), t92, "[11] 끄는 중 SheetTitle");
    for (int i = 0; i < 5; ++i) settle();
    CHECK(ctxRight().startsWith(QStringLiteral("막 92° · 끄는 중 ")), "[11] 누르고 있는 동안 확정 없음 '%s'", qUtf8Printable(ctxRight()));
    mouse(canvas, QEvent::MouseButtonRelease, {104, 100}, Qt::LeftButton, Qt::NoButton);
    CHECK(wait([&] { return ctxRight().startsWith(QStringLiteral("막 92° · 끄는 동안 ")); }), "[11] 놓음 '%s'", qUtf8Printable(ctxRight()));
    key(w, QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    eq(title(), t90, "[11] 끌기 Ctrl+Z");
    settle();
    const int x11 = sideName ? sideName->mapTo(rb, QPoint()).x() : -1;
    std::printf("[11] 단면 위치 x=%d\n", x11);
    CHECK(x11 == sideX, "[11] 리본이 옆으로 밀림 %d → %d", sideX, x11);

    // [12] [−]
    minus->click();
    CHECK(wait([&] { return ctxRight().startsWith(QStringLiteral("막 89° · 끄는 동안 ")); }), "[12] 89° '%s'", qUtf8Printable(ctxRight()));
    eq(axisName->text(), QStringLiteral("막 방향 (누르면 바로 맞춤)"), "[12] 그룹 이름");
    CHECK(ax->toolState() == ToolState::On, "[12] X 막 켜짐");

    // [13] 키 · 오른쪽 끌기
    tap(w, Qt::Key_Right);
    CHECK(wait([&] { return ctxRight().startsWith(QStringLiteral("막 90° · 끄는 동안 ")); }), "[13] → '%s'", qUtf8Printable(ctxRight()));
    tap(w, Qt::Key_Right, Qt::ShiftModifier);
    CHECK(wait([&] { return ctxRight().startsWith(QStringLiteral("막 105° · 끄는 동안 ")); }), "[13] Shift+→ '%s'", qUtf8Printable(ctxRight()));
    eq(title(), t105, "[13] SheetTitle");
    eq(axisName->text(), QStringLiteral("막 방향 · 지금 105° (누르면 맞춤)"), "[13] 그룹 이름");
    settle();
    const int x13 = sideName ? sideName->mapTo(rb, QPoint()).x() : -1;
    std::printf("[13] 단면 위치 x=%d\n", x13);
    CHECK(x13 == sideX, "[13] 리본이 옆으로 밀림 %d → %d", sideX, x13);
    eq(canvas->toolTip(), QStringLiteral("왼쪽 끌기 = 회전판 돌리기 · 오른쪽 끌기 = 보는 방향만"), "[13] 3D 툴팁");
    const QImage img0 = canvas->grab().toImage();
    mouse(canvas, QEvent::MouseButtonPress, {100, 100}, Qt::RightButton, Qt::RightButton);
    mouse(canvas, QEvent::MouseMove, {140, 120}, Qt::NoButton, Qt::RightButton);
    mouse(canvas, QEvent::MouseButtonRelease, {140, 120}, Qt::RightButton, Qt::NoButton);
    settle();
    CHECK(ctxRight().startsWith(QStringLiteral("막 105° · 끄는 동안 ")), "[13] 오른쪽 끌기는 막을 돌리지 않음 '%s'", qUtf8Printable(ctxRight()));
    eq(title(), t105, "[13] 오른쪽 끌기 뒤 SheetTitle");
    CHECK(canvas->grab().toImage() != img0, "[13] 오른쪽 끌기 = 보는 방향이 돌아야 함");

    // [14] R 반전
    key(w, QEvent::KeyPress, Qt::Key_R);
    auto combined = w.findChildren<QPushButton*>(QStringLiteral("combined"));
    CHECK(combined.size() == 2, "[14] combined 단추 %d개", int(combined.size()));
    for (auto* b : combined) eq(b->text(), QStringLiteral("단면 왼쪽·외면 오른쪽 (반전)"), "[14] 반전 글자");
    CHECK(text(w, "SheetSub").startsWith(cp::plateSub(SectionSide::Left)), "[14] SheetSub '%s'", qUtf8Printable(text(w, "SheetSub")));
    key(w, QEvent::KeyPress, Qt::Key_R);
    for (auto* b : combined) eq(b->text(), QStringLiteral("외면 왼쪽·단면 오른쪽"), "[14] 되돌린 글자");
    CHECK(text(w, "SheetSub").startsWith(cp::plateSub(SectionSide::Right)), "[14] SheetSub '%s'", qUtf8Printable(text(w, "SheetSub")));

    // [15] 페이지 = 도구 · ctx 겹침 숨김
    QLabel* dupMove = labelWithText(ctxBar, QStringLiteral("막 1° · Shift 15°"));
    QLabel* dupSide = labelWithText(ctxBar, QStringLiteral("좌/우 반전"));
    CHECK(dupMove && dupSide && !dupMove->isVisible() && !dupSide->isVisible(), "[15] ② 에서 겹치는 ctx 묶음 숨김");
    key(w, QEvent::KeyPress, Qt::Key_Escape);
    CHECK(rb->currentPage() == pHome, "[15] Esc → 홈 (%d)", rb->currentPage());
    CHECK(!ctxBar->isVisible(), "[15] Esc → CtxBar 숨김");
    eq(title(), QStringLiteral("A–A′ 정면 실측도"), "[15] 홈 SheetTitle");
    CHECK(!text(w, "View3DTitle").contains(QStringLiteral("막으로 자르기")), "[15] View3DTitle '%s'", qUtf8Printable(text(w, "View3DTitle")));
    CHECK(canvas->toolTip().isEmpty(), "[15] 3D 툴팁 비어야");
    CHECK(textColor(cutTab) == theme::Ink2, "[15] ② 글자색 %s", qUtf8Printable(textColor(cutTab).name()));
    key(w, QEvent::KeyPress, Qt::Key_S);
    CHECK(rb->currentPage() == pCut && ctxBar->isVisible(), "[15] S → ②");
    eq(text(w, "ToolBadge"), QStringLiteral("✂ 막으로 자르기"), "[15] S → ToolBadge");
    tb[5]->click();
    CHECK(!ctxBar->isVisible(), "[15] 내보내기 → CtxBar 숨김");
    key(w, QEvent::KeyPress, Qt::Key_S);
    CHECK(rb->currentPage() == pCut, "[15] S → ② (%d)", rb->currentPage());
    key(w, QEvent::KeyPress, Qt::Key_O);
    CHECK(rb->currentPage() == pHome, "[15] O → 홈 (%d)", rb->currentPage());
    eq(text(w, "ToolBadge"), QStringLiteral("◠ 외곽선 편집 중"), "[15] O → ToolBadge");
    CHECK(ctxBar->isVisible(), "[15] O → CtxBar 보임");
    CHECK(dupMove && dupSide && dupMove->isVisible() && dupSide->isVisible(), "[15] 홈 외곽선 도구에서 ctx 묶음 다시 보임");

    // [16] 옛 단추 없음
    CHECK(w.findChildren<QWidget*>(QStringLiteral("rotate")).isEmpty() && w.findChildren<QWidget*>(QStringLiteral("cutMove")).isEmpty(),
          "[16] 옛 단추 rotate·cutMove 가 남아 있음");

    // [17] 가로 슬롯에서 ② 로 (각은 그대로)
    slotList->setCurrentRow(1);
    eq(title(), QStringLiteral("B–B′ 구연부 횡단면"), "[17] 가로 슬롯");
    key(w, QEvent::KeyPress, Qt::Key_S);
    CHECK(slotList->currentRow() == 0, "[17] 슬롯 %d", slotList->currentRow());
    eq(title(), t105, "[17] ② 종단면");

    // ---- JOB-007: ② 고정 · 넘기기 · ③ 도면 편집 (수용 기준 13-19, [17] 끝 = ② · 105° · 고정 전) ----
    auto stFix = [&w] { return text(w, "StFix"); };
    auto* stFixLbl = w.findChild<QLabel*>(QStringLiteral("StFix"));
    auto* fixTile = w.findChild<Tile*>(QStringLiteral("fixAngle"));
    auto* editTile = w.findChild<Tile*>(QStringLiteral("toEdit"));
    auto* pane3d = w.findChild<View3DPane*>();
    if (!stack || !stFixLbl || !fixTile || !editTile || !pane3d) {
        CHECK(false, "[J7-13] 고정·넘기기 위젯을 찾지 못함");
        w.close();
        return;
    }
    QWidget* cutPage = stack->widget(pCut);
    QWidget* editPage = stack->widget(pEdit);
    // [J7-13] ② 고정 · 넘기기
    eq(cp::fixText(std::nullopt), QStringLiteral("막 고정 전"), "[J7-13] fixText(없음)");
    eq(cp::fixText(12.0), QStringLiteral("막 12° 고정"), "[J7-13] fixText(12)");
    eq(cp::fixText(-20.0), QStringLiteral("막 −20° 고정"), "[J7-13] fixText(-20)");
    eq(cp::slotMemo(12.0), QStringLiteral("막 12°"), "[J7-13] slotMemo(12)");
    eq(cp::kStatus, QStringLiteral("⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고 「지금 각도 고정」 → 「도면 편집으로」"), "[J7-13] kStatus");
    const QStringList cutGroups = groupNames(cutPage);
    CHECK(cutGroups.size() == 5 && cutGroups.last() == cp::kGrpFix, "[J7-13] ② 그룹 %s", qUtf8Printable(cutGroups.join(QStringLiteral(" | "))));
    eq(fixTile->text(), QStringLiteral("지금 각도 고정"), "[J7-13] fixAngle 글자");
    eq(editTile->text(), QStringLiteral("도면 편집으로"), "[J7-13] toEdit 글자");
    CHECK(fixTile->toolState() == ToolState::Idle && editTile->toolState() == ToolState::Primary, "[J7-13] fixAngle 대기 · toEdit 주 단추");
    CHECK(fixTile->sizeHint().height() == tileH && editTile->sizeHint().height() == tileH, "[J7-13] 타일 높이 %d·%d · 홈 %d",
          fixTile->sizeHint().height(), editTile->sizeHint().height(), tileH);
    CHECK(primaryTiles(cutPage) == QStringList{QStringLiteral("toEdit")}, "[J7-13] ② 주 단추 %s", qUtf8Printable(primaryTiles(cutPage).join(',')));
    CHECK(cutPage->sizeHint().height() <= stack->widget(pBase)->sizeHint().height(), "[J7-13] ② 페이지 높이 %d > ① %d",
          cutPage->sizeHint().height(), stack->widget(pBase)->sizeHint().height());
    CHECK(stFixLbl->isVisible(), "[J7-13] StFix 보임");
    eq(stFix(), QStringLiteral("막 고정 전"), "[J7-13] StFix");
    eq(text(w, "StHint"), cp::kStatus, "[J7-13] StHint");
    QLabel* ctxFix = labelWithText(ctxBar, cp::kTileFix);
    CHECK(ctxFix && ctxFix->isVisible(), "[J7-13] ctx 「지금 각도 고정」 보임");
    std::printf("[J7-13] window minimumSizeHint width=%d\n", w.minimumSizeHint().width());
    CHECK(w.minimumSizeHint().width() <= 1845, "[J7-13] 창 최소 폭 %d > 1845", w.minimumSizeHint().width());

    // [J7-14] 고정 · 풀림
    tap(w, Qt::Key_Return);
    eq(stFix(), QStringLiteral("막 105° 고정"), "[J7-14] Enter → StFix");
    CHECK(fixTile->toolState() == ToolState::On, "[J7-14] fixAngle 켜짐");
    CHECK(slotHas(slotList, QStringLiteral("막 105° 고정")), "[J7-14] 슬롯 「막 105° 고정」");
    tap(w, Qt::Key_Right);
    CHECK(wait([&] { return ctxRight().startsWith(QStringLiteral("막 106° · 끄는 동안 ")); }), "[J7-14] → '%s'", qUtf8Printable(ctxRight()));
    eq(stFix(), QStringLiteral("막 고정 전"), "[J7-14] 막을 움직이면 풀림");
    CHECK(fixTile->toolState() == ToolState::Idle, "[J7-14] fixAngle 대기");
    CHECK(!slotHas(slotList, QStringLiteral("막 105° 고정")) && slotHas(slotList, QStringLiteral("막 105°")),
          "[J7-14] 슬롯 고정 표시만 풀림 (메모 막 105°)");
    key(w, QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    eq(title(), t105, "[J7-14] Ctrl+Z SheetTitle");
    eq(stFix(), QStringLiteral("막 고정 전"), "[J7-14] Ctrl+Z 뒤 StFix");
    fixTile->click();
    eq(stFix(), QStringLiteral("막 105° 고정"), "[J7-14] 타일 → StFix");

    // [J7-15] ③ 화면
    editTile->click();
    CHECK(rb->currentPage() == pEdit, "[J7-15] toEdit → 페이지 %d", rb->currentPage());
    eq(text(w, "ToolBadge"), cp::kEditBadge, "[J7-15] ToolBadge");
    eq(ctxHint->text(), cp::kEditHint, "[J7-15] CtxHint");
    eq(ctxHint->toolTip(), cp::kEditHint, "[J7-15] CtxHint 툴팁");
    CHECK(ctxBar->isVisible(), "[J7-15] CtxBar 보임");
    eq(text(w, "StHint"), cp::kEditStatus, "[J7-15] StHint");
    eq(stFix(), QStringLiteral("막 105° 고정"), "[J7-15] StFix");
    CHECK(text(w, "View3DTitle").contains(QStringLiteral("3D — 고정된 막")), "[J7-15] View3DTitle '%s'", qUtf8Printable(text(w, "View3DTitle")));
    eq(title(), QStringLiteral("A–A′ 정면 실측도"), "[J7-15] SheetTitle");
    CHECK(ctxRight().startsWith(QStringLiteral("점 ")), "[J7-15] CtxRight '%s'", qUtf8Printable(ctxRight()));
    CHECK(dupMove && !dupMove->isVisible() && ctxFix && !ctxFix->isVisible() && dupSide && dupSide->isVisible(),
          "[J7-15] ③ ctx: ← → · 지금 각도 고정 숨김, 좌/우 반전 보임");
    CHECK(canvas->toolTip().isEmpty(), "[J7-15] 3D 툴팁 비어야");
    settle();
    CHECK(pane3d->width() == 330, "[J7-15] 3D 보조창 폭 %d", pane3d->width());
    const QStringList editGroups = groupNames(editPage);
    const QStringList wantGroups = {QStringLiteral("파일"), QStringLiteral("고정된 막"), QStringLiteral("선 다듬기 · 간략화"),
                                    QStringLiteral("도판 표시 (겹침)"), QStringLiteral("내보내기")};
    CHECK(editGroups == wantGroups, "[J7-15] ③ 그룹 %s", qUtf8Printable(editGroups.join(QStringLiteral(" | "))));
    auto* fixedLbl = editPage->findChild<QLabel*>(QStringLiteral("FixedAngle"));
    eq(fixedLbl ? fixedLbl->text() : QString(), QStringLiteral("막 105° 고정"), "[J7-15] FixedAngle");
    auto* backBtn = editPage->findChild<QPushButton*>(QStringLiteral("backToCut"));
    eq(backBtn ? backBtn->text() : QString(), QStringLiteral("↩ 3D로 돌아가 다시 자르기"), "[J7-15] backToCut 글자");
    CHECK(primaryTiles(editPage) == QStringList{QStringLiteral("export")}, "[J7-15] ③ 주 단추 %s", qUtf8Printable(primaryTiles(editPage).join(',')));
    CHECK(editPage->sizeHint().height() <= stack->widget(pBase)->sizeHint().height(), "[J7-15] ③ 페이지 높이 %d > ① %d",
          editPage->sizeHint().height(), stack->widget(pBase)->sizeHint().height());
    std::printf("[J7-15] window minimumSizeHint width=%d\n", w.minimumSizeHint().width());
    CHECK(w.minimumSizeHint().width() <= 1845, "[J7-15] 창 최소 폭 %d > 1845", w.minimumSizeHint().width());

    // [J7-16] ③ 에서는 막이 돌지 않음 (키 무시 · 왼쪽 끌기 = 보는 방향)
    tap(w, Qt::Key_Right);
    tap(w, Qt::Key_Period);
    tap(w, Qt::Key_BracketRight);
    for (int i = 0; i < 5; ++i) settle();
    eq(stFix(), QStringLiteral("막 105° 고정"), "[J7-16] 키 뒤 StFix");
    CHECK(ctxRight().startsWith(QStringLiteral("점 ")), "[J7-16] CtxRight '%s'", qUtf8Printable(ctxRight()));
    const QImage img1 = canvas->grab().toImage();
    mouse(canvas, QEvent::MouseButtonPress, {100, 100}, Qt::LeftButton, Qt::LeftButton);
    mouse(canvas, QEvent::MouseMove, {140, 120}, Qt::NoButton, Qt::LeftButton);
    mouse(canvas, QEvent::MouseButtonRelease, {140, 120}, Qt::LeftButton, Qt::NoButton);
    settle();
    CHECK(canvas->grab().toImage() != img1, "[J7-16] ③ 왼쪽 끌기 = 보는 방향이 돌아야 함");
    eq(stFix(), QStringLiteral("막 105° 고정"), "[J7-16] 끌기 뒤 StFix");

    // [J7-17] 다시 자르기
    if (backBtn) backBtn->click();
    CHECK(rb->currentPage() == pCut, "[J7-17] 다시 자르기 → 페이지 %d", rb->currentPage());
    eq(text(w, "ToolBadge"), QStringLiteral("✂ 막으로 자르기"), "[J7-17] ToolBadge");
    eq(title(), t105, "[J7-17] SheetTitle");
    eq(stFix(), QStringLiteral("막 105° 고정"), "[J7-17] StFix (막을 움직이기 전)");
    settle();
    CHECK(pane3d->width() > 330, "[J7-17] ② 3D 보조창 폭 %d", pane3d->width());
    editTab->click();
    CHECK(rb->currentPage() == pEdit, "[J7-17] ③ 탭 → 페이지 %d", rb->currentPage());
    key(w, QEvent::KeyPress, Qt::Key_Escape);
    CHECK(rb->currentPage() == pHome && !ctxBar->isVisible() && !stFixLbl->isVisible(), "[J7-17] Esc → 홈 %d · CtxBar %d · StFix %d",
          rb->currentPage(), int(ctxBar->isVisible()), int(stFixLbl->isVisible()));

    // [J7-18] 들어가면 자동 고정 · 슬롯이 고정 각을 기억
    key(w, QEvent::KeyPress, Qt::Key_S);
    CHECK(rb->currentPage() == pCut, "[J7-18] S → ② (%d)", rb->currentPage());
    tap(w, Qt::Key_Left);
    CHECK(wait([&] { return ctxRight().startsWith(QStringLiteral("막 104° · 끄는 동안 ")); }), "[J7-18] ← '%s'", qUtf8Printable(ctxRight()));
    eq(stFix(), QStringLiteral("막 고정 전"), "[J7-18] ← 뒤 StFix");
    editTab->click();
    eq(stFix(), QStringLiteral("막 104° 고정"), "[J7-18] ③ 들어가면 자동 고정");
    CHECK(slotHas(slotList, QStringLiteral("막 104° 고정")), "[J7-18] 슬롯 「막 104° 고정」");
    slotList->setCurrentRow(1);
    eq(title(), QStringLiteral("B–B′ 구연부 횡단면"), "[J7-18] ③ 가로 슬롯");
    slotList->setCurrentRow(0);
    eq(title(), QStringLiteral("A–A′ 정면 실측도"), "[J7-18] ③ A–A′");
    eq(stFix(), QStringLiteral("막 104° 고정"), "[J7-18] 슬롯을 다시 골라도 고정 각");
    key(w, QEvent::KeyPress, Qt::Key_S);
    eq(title(), QStringLiteral("A–A′ 실시간 단면 — 104°"), "[J7-18] S → ②");
    tap(w, Qt::Key_Left);  // ② 에서 가로 슬롯을 보며 고정 → 첫 종단면 A–A′ 에 남는다
    CHECK(wait([&] { return ctxRight().startsWith(QStringLiteral("막 103° · 끄는 동안 ")); }), "[J7-18] ← '%s'", qUtf8Printable(ctxRight()));
    slotList->setCurrentRow(1);
    eq(title(), QStringLiteral("B–B′ 구연부 횡단면"), "[J7-18] ② 가로 슬롯");
    tap(w, Qt::Key_Return);
    eq(stFix(), QStringLiteral("막 103° 고정"), "[J7-18] 가로 슬롯에서 Enter");
    editTile->click();
    CHECK(rb->currentPage() == pEdit, "[J7-18] toEdit → 페이지 %d", rb->currentPage());
    eq(title(), QStringLiteral("A–A′ 정면 실측도"), "[J7-18] ③ 첫 종단면");
    slotList->setCurrentRow(1);
    slotList->setCurrentRow(0);
    eq(stFix(), QStringLiteral("막 103° 고정"), "[J7-18] A–A′ 가 고정 각 103°");
    CHECK(slotHas(slotList, QStringLiteral("막 103° 고정")), "[J7-18] 슬롯 「막 103° 고정」");
    key(w, QEvent::KeyPress, Qt::Key_S);
    eq(title(), QStringLiteral("A–A′ 실시간 단면 — 103°"), "[J7-18] S → ② 103°");

    // [J7-19] 새 문서 = 고정 풀림
    w.openSample(false);
    key(w, QEvent::KeyPress, Qt::Key_S);
    tap(w, Qt::Key_Return);
    eq(stFix(), QStringLiteral("막 0° 고정"), "[J7-19] 새 예제 ② Enter");
    w.openSample(false);
    key(w, QEvent::KeyPress, Qt::Key_S);
    eq(stFix(), QStringLiteral("막 고정 전"), "[J7-19] 다시 연 예제");
    w.close();
}

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    QApplication::setOrganizationName("DongArch3DTest");  // 사용자 QSettings·최근 목록·감시 폴더와 분리
    QApplication::setApplicationName("DongArch3DTest");
    app.setFont(theme::uiFont(13));
    app.setStyleSheet(theme::styleSheet());  // 탭 글자색 판정에 필요
    testText();
    testRibbon();
    testWindow();
    std::puts(fails ? "FAILED" : "PASS");  // [18]
    return fails ? 1 : 0;
}
