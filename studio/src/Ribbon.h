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
class QLabel;
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
    void setTitle(const QString& t);
    QLabel* titleLabel() const { return name_; }
    Tile* tile(const QString& id, const QString& text, const QString& icon, const QString& tip = {});
    // 작은 단추를 만들어 id 로 등록·연결만 한다 (줄에 넣는 것은 column/rows)
    SmallButton* button(const QString& id, const QString& text, const QString& kbd = {});
    void column(const QList<QPair<QString, QPair<QString, QString>>>& items);  // id, (text, kbd)
    // 위젯 나열 줄 여러 개를 한 칸에 세로로 (줄 높이 23 px 고정 — 3줄 = 77 px, 리본 높이 불변)
    void rows(const QList<QList<QWidget*>>& rows);
    // 칩 묶음: 앞에 작은 라벨. 선택은 Ribbon::chipChosen(group, idx)
    void chips(const QString& groupId, const QString& label, const QStringList& items, int sel);
    void chipRows(const QList<std::tuple<QString, QString, QStringList, int>>& rows);

private:
    QWidget* chipRow(const QString& groupId, const QString& label, const QStringList& items, int sel);
    class Ribbon* rb_;
    QLabel* name_;
    QHBoxLayout* row_;
};

class Ribbon : public QWidget {
    Q_OBJECT
public:
    explicit Ribbon(QWidget* parent = nullptr);
    QWidget* tabsBar() const { return tabs_; }
    int addPage(const QString& name);  // 반환: 페이지 번호
    RibbonGroup* addGroup(int page, const QString& name);
    void setCurrentPage(int i);  // 늘 pageChanged 를 내보낸다 (탭 누름 포함)
    int currentPage() const { return stack_->currentIndex(); }
    // 잠긴 탭: Faint 글자 + 툴팁(이유). 누르는 것은 막지 않는다 (안내는 받는 쪽)
    void setPageLocked(int page, bool locked, const QString& tip);

    void setState(const QString& id, ToolState s);
    ToolState state(const QString& id) const { return states_.value(id, ToolState::Idle); }
    void setChip(const QString& groupId, int idx);
    void setText(const QString& id, const QString& text);

    // 내부 등록
    void registerWidget(const QString& id, QWidget* w);
    void registerChip(const QString& groupId, QPushButton* b);

signals:
    void triggered(const QString& id);
    void pageChanged(int page);
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
