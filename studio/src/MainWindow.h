// DongArch3D Studio — 메인 창 (UI_RULES R1 고정 구조 + Artec Studio 실시간 연동).
#pragma once
#include "ArtifactDoc.h"
#include "Plate.h"

#include <QMainWindow>
#include <QVector>

class QFileSystemWatcher;
class QLabel;
class QListWidget;
class QStackedWidget;
class QTimer;
class QWidget;

namespace studio {

class Ribbon;
class SheetPane;
class View3DPane;
class StartPage;
enum class ExportFormat;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
    bool openPath(const QString& path);  // 들어온 자세 그대로 ① 바닥 정하기 단계로 연다
    // poseStage=false: 예전처럼 자동 세우기 → 확정 후 홈, true: 들어온 자세 그대로 ① 바닥 정하기
    void openSample(bool poseStage = false);
    void openArtecBlindSample(bool poseStage = false);
    void showHome();

protected:
    void keyPressEvent(QKeyEvent*) override;
    bool eventFilter(QObject*, QEvent*) override;
    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;
    void closeEvent(QCloseEvent*) override;

private:
    struct ViewState {
        double angle;
        HeightRef ref;
        double simplify;
        bool smooth;
    };
    void buildRibbon();
    QWidget* buildSlotPanel();
    QWidget* buildCtxBar();
    QWidget* buildStatusBar();
    void onAction(const QString& id);
    void onChip(const QString& group, int idx);
    void setHandTool(const QString& id);  // "" = 내려놓기
    void toggleMaximize();
    void refreshAll(bool geometryChanged);
    void refreshSlots();
    void selectSlot(int i);
    void applyOptions();
    void pushUndo();
    void restore(const ViewState& s);
    void setFrontAngle(double deg);
    void saveSlotAngle(int k);
    void openExport(ExportFormat f);
    void openFileDialog();
    void toggleWatchFolder();
    void applyWatchFolder(const QString& dir);
    void checkWatchFolder();
    void setHint(const QString& s);
    bool requirePose();  // 확정 전이면 안내하고 ① 페이지로 → false
    void confirmPose();  // 이 자세로 확정 (Enter)
    void recordRecent();  // 최근 목록에 현재 유물 기록 (경로가 있을 때)

    ArtifactDoc doc_;
    PlateOptions opts_;
    int slot_ = 0;
    QString hand_;
    bool maximized_ = false;
    bool dirtyExport_ = false;
    double lastHoverH_ = -1;
    QVector<ViewState> undo_, redo_;

    // Artec Studio 실시간 감시 폴더
    QFileSystemWatcher* watcher_ = nullptr;
    QTimer* watchTimer_ = nullptr;
    QString watchDir_;
    QString pendingWatchFile_;
    qint64 pendingWatchSize_ = -1;

    QStackedWidget* pages_ = nullptr;
    StartPage* start_ = nullptr;
    Ribbon* ribbon_ = nullptr;
    QWidget* ctx_ = nullptr;
    QLabel* ctxBadge_ = nullptr;
    QLabel* ctxHint_ = nullptr;
    QLabel* ctxRight_ = nullptr;
    QVector<QWidget*> ctxCutKeys_;   // 단면 도구 단축키 묶음 ([ ] · W · - + · 1~5 · Esc)
    QVector<QWidget*> ctxBaseKeys_;  // ① 바닥 정하기 단축키 묶음 (Enter 확정 · Shift −90°)
    int pHome_ = -1, pBase_ = -1;     // 리본 페이지: 홈, ① 바닥 정하기
    QWidget* slotPanel_ = nullptr;
    QListWidget* slotList_ = nullptr;
    QLabel* slotCount_ = nullptr;
    View3DPane* view3d_ = nullptr;
    SheetPane* sheet_ = nullptr;
    QLabel* stHint_ = nullptr;
    QLabel* stCursor_ = nullptr;
    QLabel* stScale_ = nullptr;
    QLabel* stRef_ = nullptr;
    QLabel* stDirty_ = nullptr;
};

}  // namespace studio
