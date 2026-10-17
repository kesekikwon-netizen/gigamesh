// DongArch3D Studio — 탭 + 타일형 리본 (UI_RULES R1, R3).
// 같은 id 의 타일/단추가 여러 탭에 있어도 상태(대기/켜짐/손에 든 도구/주 단추)는 한 번에 바뀐다.
#pragma once
#include <QHash>
#include <QPushButton>
#include <QStackedWidget>
#include <QToolButton>
#include <QVector>
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;

namespace studio {

enum class ToolState { Idle, On, Hand, Primary };

class Tile : public QToolButton {
    Q_OBJECT
public:
    Tile(const QString& id, const QString& text, const QString& icon, QWidget* parent = nullptr);
    void setToolState(ToolState s);
    ToolState toolState() const { return state_; }
    QSize sizeHint() const override;
    static void drawIcon(QPainter& p, const QString& kind, const QRectF& r, const QColor& c);

protected:
    void paintEvent(QPaintEvent*) override;
    void enterEvent(QEnterEvent*) override { hover_ = true; update(); }
    void leaveEvent(QEvent*) override { hover_ = false; update(); }

private:
    QString icon_;
    ToolState state_ = ToolState::Idle;
    bool hover_ = false;
};

// 작은 단추 + 오른쪽 단축키 배지
class SmallButton : public QPushButton {
    Q_OBJECT
public:
    SmallButton(const QString& text, const QString& kbd, QWidget* parent = nullptr);
    void setToolState(ToolState s);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QString kbd_;
};

class RibbonGroup : public QWidget {
    Q_OBJECT
public:
    RibbonGroup(class Ribbon* rb, const QString& name, QWidget* parent);
    Tile* tile(const QString& id, const QString& text, const QString& icon, const QString& tip = {});
    void column(const QList<QPair<QString, QPair<QString, QString>>>& items);  // id, (text, kbd)
    // 칩 묶음: 앞에 작은 라벨. 선택은 Ribbon::chipChosen(group, idx)
    void chips(const QString& groupId, const QString& label, const QStringList& items, int sel);
    void chipRows(const QList<std::tuple<QString, QString, QStringList, int>>& rows);

private:
    QWidget* chipRow(const QString& groupId, const QString& label, const QStringList& items, int sel);
    class Ribbon* rb_;
    QHBoxLayout* row_;
};

class Ribbon : public QWidget {
    Q_OBJECT
public:
    explicit Ribbon(QWidget* parent = nullptr);
    QWidget* tabsBar() const { return tabs_; }
    int addPage(const QString& name);  // 반환: 페이지 번호
    RibbonGroup* addGroup(int page, const QString& name);
    void setCurrentPage(int i);

    void setState(const QString& id, ToolState s);
    ToolState state(const QString& id) const { return states_.value(id, ToolState::Idle); }
    void setChip(const QString& groupId, int idx);
    void setText(const QString& id, const QString& text);

    // 내부 등록
    void registerWidget(const QString& id, QWidget* w);
    void registerChip(const QString& groupId, QPushButton* b);

signals:
    void triggered(const QString& id);
    void chipChosen(const QString& groupId, int idx);
    void undoRequested();
    void redoRequested();

private:
    QWidget* tabs_;
    QHBoxLayout* tabsLay_;
    QStackedWidget* stack_;
    QVector<QPushButton*> tabBtns_;
    QVector<QHBoxLayout*> pageLays_;
    QHash<QString, QVector<QWidget*>> widgets_;
    QHash<QString, ToolState> states_;
    QHash<QString, QVector<QPushButton*>> chips_;
};

}  // namespace studio
