// DongArch3D Studio — 진입점.
#include "MainWindow.h"
#include "Theme.h"

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
    const QByteArray geo = QSettings().value("window/geometry").toByteArray();
    if (!geo.isEmpty()) w.restoreGeometry(geo);
    w.show();

    const QStringList args = app.arguments();
    // 검수용: --shot <png> [--max] [--export] — 창을 그려 저장하고 끝낸다
    const int shotIdx = int(args.indexOf("--shot"));
    if (args.size() > 1 && !args[1].startsWith("--shot")) {
        if (args[1] == "--sample") w.openSample();
        else if (args[1] == "--blind") w.openArtecBlindSample();
        else if (args[1] != "--home") w.openPath(args[1]);
    }
    if (shotIdx > 0 && shotIdx + 1 < args.size()) {
        const QString out = args[shotIdx + 1];
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
            if (target) target->grab().save(out);
            QApplication::exit(0);
        });
    }
    return app.exec();
}
