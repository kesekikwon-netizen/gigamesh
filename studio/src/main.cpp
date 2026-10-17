// DongArch3D Studio — 진입점.
#include "MainWindow.h"
#include "Theme.h"

#include <QAbstractButton>
#include <QApplication>
#include <QKeyEvent>
#include <QSettings>
#include <QTimer>

int main(int argc, char** argv) {
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Dongguk");
    QApplication::setApplicationName("DongArch3D");
    app.setFont(theme::uiFont(13));
    app.setStyleSheet(theme::styleSheet());

    studio::MainWindow w;
    const QStringList args = app.arguments();
    const QByteArray geo = QSettings().value("window/geometry").toByteArray();
    if (!geo.isEmpty()) w.restoreGeometry(geo);
    // 검수 캡처는 창을 1845×920(JOB-004·005 캡처 크기)으로 고정 — 창 최소 폭은 가장 넓은 리본 페이지를 따라
    // 단추 하나만 빼도 바뀌므로, 고정해야 시작 화면·작업 화면을 이전 캡처와 바이트·화소로 비교할 수 있다 (최소 폭이 더 크면 그 폭)
    if (args.contains("--shot")) w.resize(1845, 920);
    w.show();

    // 검수용: --shot <png> [--max] [--export] [--flip] — 창을 그려 저장하고 끝낸다
    //         --sample|--blind [--base] — 예제를 연다. --base 면 확정 전(들어온 자세) ① 바닥 정하기 페이지
    //         예: DongArch3DStudio.exe --sample --base --shot shot_base.png (파일 경로 인자는 원래 확정 전으로 열림)
    //         --flip — R 키를 보내 도판 좌/우를 반전(D5: 단면 왼쪽 · 외면 오른쪽)한 뒤 찍는다
    //         예: DongArch3DStudio.exe --sample --flip --shot shot_flip.png
    //         --cut — S 로 ② 막으로 자르기에 가서 → 를 12번(디자인 시안의 12°) 누른 뒤 찍는다
    //         예: DongArch3DStudio.exe --sample --cut --shot shot_cut.png
    //         --edit — 300 ms 에 「도면 편집으로」(toEdit)를 눌러 ③ 도면 편집에서 찍는다(--cut 과 함께면 12° 고정)
    //         예: DongArch3DStudio.exe --sample --cut --edit --shot shot_edit.png
    const int shotIdx = int(args.indexOf("--shot"));
    const bool baseStage = args.contains("--base");
    if (args.size() > 1 && !args[1].startsWith("--shot")) {
        if (args[1] == "--sample") w.openSample(baseStage);
        else if (args[1] == "--blind") w.openArtecBlindSample(baseStage);
        else if (args[1] != "--home") w.openPath(args[1]);
    }
    if (args.contains("--cut")) QTimer::singleShot(200, &w, [&w] {  // 실제 키 경로(합치기 → 300 ms 뒤 원본 확정)
        QKeyEvent s(QEvent::KeyPress, Qt::Key_S, Qt::NoModifier);
        QApplication::sendEvent(&w, &s);
        for (int i = 0; i < 12; ++i) {
            QKeyEvent down(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier), up(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier);
            QApplication::sendEvent(&w, &down);
            QApplication::sendEvent(&w, &up);
        }
    });
    if (args.contains("--edit")) QTimer::singleShot(300, &w, [&w] {  // 실제 단추 경로 (먼저 확정 → 자동 고정 → ③)
        if (auto* b = w.findChild<QAbstractButton*>("toEdit")) b->click();
    });
    if (shotIdx > 0 && shotIdx + 1 < args.size()) {
        const QString out = args[shotIdx + 1];
        if (args.contains("--flip")) QTimer::singleShot(200, &w, [&w] {
            QKeyEvent k(QEvent::KeyPress, Qt::Key_R, Qt::NoModifier);
            QApplication::sendEvent(&w, &k);
        });
        if (args.contains("--max")) QTimer::singleShot(300, &w, [&w] {
            QKeyEvent k(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
            QApplication::sendEvent(&w, &k);
        });
        if (args.contains("--export")) QTimer::singleShot(400, &w, [&w] {
            QKeyEvent k(QEvent::KeyPress, Qt::Key_P, Qt::ControlModifier);
            QApplication::sendEvent(&w, &k);
        });
        QTimer::singleShot(1800, &app, [out] {
            QWidget* top = QApplication::activeModalWidget();
            QWidget* target = top ? top : QApplication::activeWindow();
            if (!target) target = QApplication::topLevelWidgets().value(0);
            // 원본 계산(UI 스레드 ≈ 1.4 s) 중에 이 타이머가 밀리면 계산 직후 글자만 바뀌고 다시 배치는 아직이다 → 먼저 배치
            QApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);
            if (target) target->grab().save(out);
            QApplication::exit(0);
        });
    }
    return app.exec();
}
