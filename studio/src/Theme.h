// DongArch3D Studio — 「조용한 유물 도판」 디자인 토큰.
// 규칙 원본: docs/ui/UI_RULES.md (R2 색상, R3 도구 상태, R4 글꼴). 여기 없는
// 색은 쓰지 않는다.
#pragma once
#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>

namespace theme {

// ---- R2 색상 토큰 --------------------------------------------------------
inline bool gHighContrast = false;
inline void setHighContrast(bool on) { gHighContrast = on; }
inline bool isHighContrast() { return gHighContrast; }

inline const QColor Ground{0xFA, 0xF9, 0xF5};
inline const QColor Surface{0xFF, 0xFF, 0xFF};
inline const QColor Wash{0xF5, 0xF4, 0xED};
inline const QColor Desk{0xF0, 0xEE, 0xE6};
inline const QColor Press{0xE8, 0xE6, 0xDC};
inline const QColor Oat{0xE3, 0xDA, 0xCC};
inline const QColor Ring{0xB0, 0xAE, 0xA5};
inline const QColor Hand{0x30, 0x30, 0x2E};
inline const QColor Action{0xB5, 0x57, 0x3A};
inline const QColor ActionHover{0xA4, 0x4D, 0x32};
inline const QColor Ink{0x14, 0x14, 0x13};
inline const QColor Ink2{0x3D, 0x3D, 0x3A};
inline const QColor Muted{0x5E, 0x5D, 0x59};
inline const QColor Faint{0x73, 0x72, 0x6C};
inline const QColor Line{0xDE, 0xDC, 0xD1};
inline const QColor Edge{0xC2, 0xC0, 0xB6};
inline const QColor Caution{0x7A, 0x5A, 0x00};
inline const QColor Done{0x3F, 0x6B, 0x31};

// ---- 실측 데이터 색 (도판·3D·내보내기) ------------------------------------
inline const QColor ProfileRed{0xFF, 0x00, 0x00}; // 단면선 전용
inline const QColor ProfileFill{0xFF, 0x00, 0x00,
                                30};            // 단면 기벽 채움(연한 빨강)
inline const QColor OuterInk{0x14, 0x14, 0x13}; // 외곽선·중심축
inline const QColor RidgeInk{0x3D, 0x3D, 0x3A}; // 박리 능선·문양선 (R2 Ink2)
inline const QColor LithicFill{0xE3, 0xDA, 0xCC,
                               110}; // 석기 4면 전개 바탕 음영 (R2 Oat 반투명)
inline const QColor Grid1mm{0xED, 0xEB, 0xE2};
inline const QColor Grid10mm{0xD5, 0xD2, 0xC6};
inline const QColor ModelClay{0xB0, 0x8B, 0x63}; // 3D 보조창 유물 기본 음영색

// 햇빛 고대비 모드(야외 발굴 현장용 라이트 테마 변형 — R2 토큰 내 고대비 매핑)
inline QColor grid1mmColor() { return gHighContrast ? Grid10mm : Grid1mm; }
inline QColor grid10mmColor() { return gHighContrast ? Ring : Grid10mm; }
inline QColor edgeColor() { return gHighContrast ? Ink2 : Edge; }
inline double strokeScale() { return gHighContrast ? 1.35 : 1.0; }

// ---- R4 글꼴 -------------------------------------------------------------
inline QFont uiFont(int px = 13, bool bold = false) {
  QFont f;
  f.setFamilies(
      {QStringLiteral("Malgun Gothic"), QStringLiteral("Noto Sans KR")});
  f.setPixelSize(px);
  f.setBold(bold);
  return f;
}
inline QFont monoFont(int px = 12, bool bold = false) {
  QFont f;
  f.setFamilies({QStringLiteral("Consolas"), QStringLiteral("JetBrains Mono")});
  f.setStyleHint(QFont::Monospace);
  f.setPixelSize(px);
  f.setBold(bold);
  return f;
}
inline QFont serifFont(int px = 20) {
  QFont f;
  f.setFamilies({QStringLiteral("Noto Serif KR"), QStringLiteral("Batang"),
                 QStringLiteral("Georgia")});
  f.setPixelSize(px);
  f.setWeight(QFont::DemiBold);
  return f;
}

inline QString hex(const QColor &c) { return c.name(QColor::HexRgb).toUpper(); }

// ---- 전역 QSS (R1/R3) ----------------------------------------------------
// 위젯 objectName / dynamic property("state") 로 3단계 도구 상태를 표현한다.
inline QString styleSheet() {
  QString s = QStringLiteral(R"(
QWidget { background: %Ground%; color: %Ink%; font-family: "Malgun Gothic"; font-size: 13px; }
QToolTip { background: %Surface%; color: %Ink%; border: 1px solid %Edge%; padding: 4px 6px; }
#Tabs, #Ribbon, #Titlebar { background: %Surface%; border-bottom: 1px solid %Line%; }
#Tabs QPushButton[tab="true"] { background: transparent; border: 0; padding: 0 14px; height: 34px; color: %Ink2%; }
#Tabs QPushButton[tab="true"][on="true"] { color: %Ink%; font-weight: 700; border-bottom: 2px solid %Ink%; }
#Tabs QPushButton[tab="true"][locked="true"] { color: %Faint%; }
QPushButton#UndoBtn { height: 24px; padding: 0 8px; border: 1px solid %Edge%; border-radius: 6px; background: %Surface%; color: %Ink2%; font-size: 12px; }
#Ribbon QWidget { background: %Surface%; }
#Ribbon QLabel[gname="true"] { font-size: 11px; font-weight: 700; color: %Muted%; }
#Ribbon QFrame[vsep="true"] { background: %Line%; min-width: 1px; max-width: 1px; }
QToolButton[tile="true"] { background: transparent; border: 0; font-size: 12px; color: %Ink%; }
QPushButton[sbtn="true"] { height: 23px; padding: 0 8px; border: 1px solid %Edge%; border-radius: 6px; background: %Surface%; font-size: 12px; text-align: left; }
QPushButton[sbtn="true"]:hover { background: %Wash%; }
QPushButton[sbtn="true"][state="on"] { background: %Oat%; border-color: %Ring%; font-weight: 600; }
QPushButton[chip="true"] { height: 23px; padding: 0 8px; border: 1px solid %Edge%; background: %Surface%; font-family: Consolas; font-size: 12px; border-radius: 0; }
QPushButton[chip="true"][first="true"] { border-top-left-radius: 6px; border-bottom-left-radius: 6px; }
QPushButton[chip="true"][last="true"] { border-top-right-radius: 6px; border-bottom-right-radius: 6px; }
QPushButton[chip="true"][state="on"] { background: %Oat%; border-color: %Ink%; font-weight: 700; }
#CtxBar { background: %Wash%; border-bottom: 1px solid %Line%; }
#CtxBar QLabel { background: transparent; font-size: 12px; }
#CtxBar QLabel#ToolBadge { background: %Hand%; color: %Ground%; font-weight: 700; border-radius: 6px; padding: 0 10px; }
QLabel[kbd="true"] { font-family: Consolas; font-size: 11px; color: %Ink2%; background: %Surface%; border: 1px solid %Edge%; border-bottom-width: 2px; border-radius: 4px; padding: 0 4px; }
#SlotPanel, #View3DPane, #SheetPane { background: %Surface%; }
#SlotPanel { border-right: 1px solid %Line%; }
#View3DPane { border-right: 1px solid %Line%; }
QLabel[ph="true"] { font-weight: 700; font-size: 12px; background: %Surface%; }
#PaneHead { background: %Surface%; border-bottom: 1px solid %Line%; }
#SheetHead { background: %Surface%; border-bottom: 2px solid %Ink%; }
#InfoStrip, #View3DInfo { background: %Wash%; border-bottom: 1px solid %Line%; }
#InfoStrip QLabel, #View3DInfo QLabel { background: transparent; font-size: 12px; color: %Ink2%; }
QLabel#BadgeOk { background: %Wash%; border: 1px solid #87867F; border-radius: 4px; padding: 0 8px; font-size: 12px; }
QLabel#BadgeWarn { background: %Surface%; border: 1px solid %Caution%; color: %Caution%; border-radius: 4px; padding: 0 8px; font-size: 12px; font-weight: 600; }
QPushButton[icon-btn="true"] { min-width: 24px; max-width: 24px; height: 24px; border: 1px solid transparent; border-radius: 4px; background: transparent; color: %Ink2%; font-size: 12px; }
QPushButton[icon-btn="true"]:hover { background: %Wash%; border-color: %Edge%; }
QPushButton[icon-btn="true"][state="on"] { background: %Oat%; border-color: %Ring%; }
QListWidget#SlotList { background: %Surface%; border: 0; outline: 0; }
QListWidget#SlotList::item { border-bottom: 1px solid %Line%; padding: 0; }
QListWidget#SlotList::item:selected { background: %Oat%; border-left: 3px solid %Ink%; color: %Ink%; }
QListWidget#SlotList::item:hover:!selected { background: %Wash%; }
#StatusBar { background: %Ground%; border-top: 1px solid %Line%; }
#StatusBar QLabel { background: transparent; font-size: 12px; color: %Ink2%; }
QPushButton[primary="true"] { background: %Action%; color: #FFFFFF; border: 1px solid %Action%; border-radius: 6px; padding: 0 18px; height: 34px; font-weight: 700; }
QPushButton[primary="true"]:hover { background: %ActionHover%; }
QPushButton[secondary="true"] { background: %Surface%; color: %Ink%; border: 1px solid %Edge%; border-radius: 6px; padding: 0 14px; height: 32px; }
QPushButton[secondary="true"]:hover { background: %Wash%; }
QPushButton[link="true"] { background: transparent; border: 0; color: %Muted%; }
QLineEdit, QComboBox { background: %Surface%; border: 1px solid %Edge%; border-radius: 4px; padding: 3px 6px; min-height: 22px; }
QLineEdit:focus, QComboBox:focus { border-color: %Ink%; }
QCheckBox { background: transparent; spacing: 6px; }
QScrollArea { border: 0; }
QSplitter::handle { background: %Line%; }
)");
  const std::pair<const char *, QColor> toks[] = {
      {"%Ground%", Ground},
      {"%Surface%", Surface},
      {"%Wash%", Wash},
      {"%Desk%", Desk},
      {"%Press%", Press},
      {"%Oat%", Oat},
      {"%Ring%", Ring},
      {"%Hand%", Hand},
      {"%ActionHover%", ActionHover},
      {"%Action%", Action},
      {"%Ink2%", Ink2},
      {"%Ink%", Ink},
      {"%Muted%", Muted},
      {"%Faint%", Faint},
      {"%Line%", Line},
      {"%Edge%", Edge},
      {"%Caution%", Caution},
      {"%Done%", Done},
  };
  for (const auto &[k, c] : toks)
    s.replace(QLatin1String(k), hex(c));
  return s;
}

} // namespace theme
