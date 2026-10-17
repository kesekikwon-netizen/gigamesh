// DongArch3D Studio — 시작(홈) 화면 (UI_RULES R7 + Artec Studio 직결 연동).
#pragma once
#include <QStringList>
#include <QWidget>

class QLabel;
class QTableWidget;
class QPushButton;

namespace studio {

struct RecentItem {
    QString path, name, kind, dims, when;
    int slotCount = 4;
};

QList<RecentItem> loadRecent();
void pushRecent(const RecentItem& it);

class StartPage : public QWidget {
    Q_OBJECT
public:
    explicit StartPage(QWidget* parent = nullptr);
    void reload();
    void setWatchFolderStatus(const QString& dir, bool active);

signals:
    void openFileRequested();
    void openPathRequested(const QString& path);
    void sampleRequested();
    void artecBlindSampleRequested();
    void watchFolderRequested();

protected:
    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;

private:
    QLabel* resumeName_;
    QLabel* resumePath_;
    QLabel* resumeWhen_;
    QLabel* resumeSlots_;
    QLabel* resumeDims_;
    QLabel* resumeAlign_;
    QPushButton* resumeBtn_;
    QPushButton* watchBtn_;
    QTableWidget* table_;
    QLabel* recentCount_;
    QString resumeTarget_;
};

}  // namespace studio
