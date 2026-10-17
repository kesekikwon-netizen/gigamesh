/**
 * QGMDarkModeManager.cpp
 *
 * Implementation of Dark Mode Manager for DongArch3D
 * Task 108: Dark Mode Support
 *
 * @file QGMDarkModeManager.cpp
 * @author DongArch3D Team
 * @date 2025-11-08
 * @version 1.0.0
 */

#include "QGMDarkModeManager.h"
#include <QSettings>
#include <QDebug>

// Initialize static singleton instance
QGMDarkModeManager* QGMDarkModeManager::mInstance = nullptr;

// ============================================================================
// Singleton Management
// ============================================================================

QGMDarkModeManager* QGMDarkModeManager::instance() {
    if (mInstance == nullptr) {
        mInstance = new QGMDarkModeManager();
    }
    return mInstance;
}

QGMDarkModeManager::QGMDarkModeManager(QObject *parent)
    : QObject(parent)
    , mDarkModeEnabled(false)
{
    loadSettings();
    qDebug() << "[QGMDarkModeManager] Initialized with mode:" << (mDarkModeEnabled ? "Dark" : "Light");
}

QGMDarkModeManager::~QGMDarkModeManager() {
    saveSettings();
}

// ============================================================================
// Theme Switching
// ============================================================================

void QGMDarkModeManager::setDarkMode(bool enabled) {
    if (mDarkModeEnabled != enabled) {
        mDarkModeEnabled = enabled;
        qDebug() << "[QGMDarkModeManager] Switching to" << (enabled ? "Dark" : "Light") << "mode";
        applyTheme();
        saveSettings();
        emit themeChanged(mDarkModeEnabled);
    }
}

void QGMDarkModeManager::toggleDarkMode() {
    setDarkMode(!mDarkModeEnabled);
}

void QGMDarkModeManager::applyTheme() {
    qDebug() << "[QGMDarkModeManager] Applying" << (mDarkModeEnabled ? "Dark" : "Light") << "theme";

    // Create and apply palette
    QPalette palette = mDarkModeEnabled ? createDarkPalette() : createLightPalette();
    qApp->setPalette(palette);

    // Apply stylesheet
    QString styleSheet = mDarkModeEnabled ? buildDarkStyleSheet() : buildLightStyleSheet();
    qApp->setStyleSheet(styleSheet);

    qDebug() << "[QGMDarkModeManager] Theme applied successfully";
}

// ============================================================================
// Palette Creation
// ============================================================================

QPalette QGMDarkModeManager::createLightPalette() const {
    QPalette palette;

    // Window background and text
    palette.setColor(QPalette::Window, DongArchColors::LightTheme::Background);
    palette.setColor(QPalette::WindowText, DongArchColors::LightTheme::Text);

    // Base (widget backgrounds) and text
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, DongArchColors::LightTheme::Panel);
    palette.setColor(QPalette::Text, DongArchColors::LightTheme::Text);

    // Button colors
    palette.setColor(QPalette::Button, DongArchColors::LightTheme::Panel);
    palette.setColor(QPalette::ButtonText, DongArchColors::LightTheme::Text);

    // Link colors
    palette.setColor(QPalette::Link, DongArchColors::SELECTION_BLUE);
    palette.setColor(QPalette::LinkVisited, DongArchColors::SELECTION_BLUE);

    // Highlight colors
    palette.setColor(QPalette::Highlight, DongArchColors::SOIL_LAYER_BROWN);
    palette.setColor(QPalette::HighlightedText, Qt::white);

    // Tooltip colors
    palette.setColor(QPalette::ToolTipBase, DongArchColors::CORTEX_WHITE);
    palette.setColor(QPalette::ToolTipText, DongArchColors::RIDGE_DARK);

    return palette;
}

QPalette QGMDarkModeManager::createDarkPalette() const {
    QPalette palette;

    // Window background and text
    palette.setColor(QPalette::Window, DongArchColors::DarkTheme::Background);
    palette.setColor(QPalette::WindowText, DongArchColors::DarkTheme::Text);

    // Base (widget backgrounds) and text
    palette.setColor(QPalette::Base, DongArchColors::DarkTheme::Panel);
    palette.setColor(QPalette::AlternateBase, DongArchColors::DarkTheme::Background);
    palette.setColor(QPalette::Text, DongArchColors::DarkTheme::Text);

    // Button colors
    palette.setColor(QPalette::Button, DongArchColors::DarkTheme::Panel);
    palette.setColor(QPalette::ButtonText, DongArchColors::DarkTheme::Text);

    // Link colors
    palette.setColor(QPalette::Link, QColor(100, 181, 246));  // Lighter blue for dark mode
    palette.setColor(QPalette::LinkVisited, QColor(149, 117, 205));  // Lighter purple

    // Highlight colors (use warmer tone for dark mode)
    palette.setColor(QPalette::Highlight, DongArchColors::CERAMIC_OCHRE);
    palette.setColor(QPalette::HighlightedText, Qt::white);

    // Tooltip colors
    palette.setColor(QPalette::ToolTipBase, DongArchColors::DarkTheme::Panel);
    palette.setColor(QPalette::ToolTipText, DongArchColors::DarkTheme::Text);

    // Disabled colors
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));

    return palette;
}

// ============================================================================
// Stylesheet Generation
// ============================================================================

QString QGMDarkModeManager::buildLightStyleSheet() const {
    return QString(R"(
        /* ================================================================
         * DongArch3D Light Theme Stylesheet
         * Task 108: Dark Mode Support
         * ================================================================ */

        /* Main Window */
        QMainWindow {
            background-color: %1;
        }

        /* Dock Widgets */
        QDockWidget {
            background-color: %2;
            border: 1px solid %3;
            titlebar-close-icon: url(:/icons/close-light.png);
            titlebar-normal-icon: url(:/icons/float-light.png);
        }

        QDockWidget::title {
            background-color: %4;
            color: %5;
            padding: 4px;
            text-align: left;
            font-weight: bold;
        }

        /* Tool Palette (Task 102) */
        QTableWidget {
            background-color: white;
            alternate-background-color: %2;
            gridline-color: %3;
            border: 1px solid %3;
        }

        QTableWidget::item:selected {
            background-color: %6;
            color: white;
        }

        QTableWidget::item:hover {
            background-color: %7;
        }

        /* Measurements Panel (Task 104) */
        QGroupBox {
            background-color: %2;
            border: 2px solid %3;
            border-radius: 5px;
            margin-top: 10px;
            font-weight: bold;
            color: %5;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
            color: %6;
        }

        /* Toolbar (Task 106) */
        QToolBar {
            background-color: %2;
            border: 1px solid %3;
            spacing: 3px;
        }

        QToolButton {
            background-color: transparent;
            border: 1px solid transparent;
            border-radius: 3px;
            padding: 3px;
        }

        QToolButton:hover {
            background-color: %7;
            border: 1px solid %3;
        }

        QToolButton:pressed {
            background-color: %6;
        }

        QToolButton:checked {
            background-color: %6;
            border: 1px solid %4;
        }

        /* Status Bar (Task 107) */
        QStatusBar {
            background-color: %2;
            color: %5;
            border-top: 1px solid %3;
        }

        QStatusBar::item {
            border: none;
        }

        /* Push Buttons */
        QPushButton {
            background-color: %2;
            border: 1px solid %3;
            border-radius: 4px;
            padding: 5px 15px;
            color: %5;
        }

        QPushButton:hover {
            background-color: %7;
            border: 1px solid %6;
        }

        QPushButton:pressed {
            background-color: %6;
            color: white;
        }

        /* Menu Bar */
        QMenuBar {
            background-color: %2;
            color: %5;
        }

        QMenuBar::item:selected {
            background-color: %6;
            color: white;
        }

        /* Menu */
        QMenu {
            background-color: white;
            border: 1px solid %3;
        }

        QMenu::item:selected {
            background-color: %6;
            color: white;
        }

        /* Line Edit */
        QLineEdit {
            background-color: white;
            border: 1px solid %3;
            border-radius: 3px;
            padding: 3px;
            color: %5;
        }

        QLineEdit:focus {
            border: 2px solid %6;
        }

        /* Spin Box */
        QSpinBox, QDoubleSpinBox {
            background-color: white;
            border: 1px solid %3;
            border-radius: 3px;
            padding: 3px;
        }

        /* Combo Box */
        QComboBox {
            background-color: white;
            border: 1px solid %3;
            border-radius: 3px;
            padding: 3px;
        }

        QComboBox:hover {
            border: 1px solid %6;
        }

        /* Scroll Bar */
        QScrollBar:vertical {
            background-color: %2;
            width: 12px;
        }

        QScrollBar::handle:vertical {
            background-color: %3;
            border-radius: 6px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: %6;
        }

        QScrollBar:horizontal {
            background-color: %2;
            height: 12px;
        }

        QScrollBar::handle:horizontal {
            background-color: %3;
            border-radius: 6px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: %6;
        }
    )")
    .arg(DongArchColors::toHtmlColor(DongArchColors::LightTheme::Background))  // %1
    .arg(DongArchColors::toHtmlColor(DongArchColors::LightTheme::Panel))       // %2
    .arg(DongArchColors::toHtmlColor(DongArchColors::LightTheme::Border))      // %3
    .arg(DongArchColors::toHtmlColor(DongArchColors::LIGHT_OCHRE))             // %4
    .arg(DongArchColors::toHtmlColor(DongArchColors::LightTheme::Text))        // %5
    .arg(DongArchColors::toHtmlColor(DongArchColors::SOIL_LAYER_BROWN))        // %6
    .arg(DongArchColors::toHtmlColor(DongArchColors::HOVER_HIGHLIGHT));        // %7
}

QString QGMDarkModeManager::buildDarkStyleSheet() const {
    return QString(R"(
        /* ================================================================
         * DongArch3D Dark Theme Stylesheet
         * Task 108: Dark Mode Support
         * ================================================================ */

        /* Main Window */
        QMainWindow {
            background-color: %1;
        }

        /* Dock Widgets */
        QDockWidget {
            background-color: %2;
            border: 1px solid %3;
            color: %4;
            titlebar-close-icon: url(:/icons/close-dark.png);
            titlebar-normal-icon: url(:/icons/float-dark.png);
        }

        QDockWidget::title {
            background-color: %5;
            color: %4;
            padding: 4px;
            text-align: left;
            font-weight: bold;
        }

        /* Tool Palette (Task 102) */
        QTableWidget {
            background-color: %2;
            alternate-background-color: %1;
            gridline-color: %3;
            border: 1px solid %3;
            color: %4;
        }

        QTableWidget::item:selected {
            background-color: %6;
            color: white;
        }

        QTableWidget::item:hover {
            background-color: %7;
        }

        /* Measurements Panel (Task 104) */
        QGroupBox {
            background-color: %2;
            border: 2px solid %3;
            border-radius: 5px;
            margin-top: 10px;
            font-weight: bold;
            color: %4;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
            color: %6;
        }

        /* Toolbar (Task 106) */
        QToolBar {
            background-color: %2;
            border: 1px solid %3;
            spacing: 3px;
        }

        QToolButton {
            background-color: transparent;
            border: 1px solid transparent;
            border-radius: 3px;
            padding: 3px;
            color: %4;
        }

        QToolButton:hover {
            background-color: %7;
            border: 1px solid %3;
        }

        QToolButton:pressed {
            background-color: %6;
        }

        QToolButton:checked {
            background-color: %6;
            border: 1px solid %8;
        }

        /* Status Bar (Task 107) */
        QStatusBar {
            background-color: %2;
            color: %4;
            border-top: 1px solid %3;
        }

        QStatusBar::item {
            border: none;
        }

        /* Push Buttons */
        QPushButton {
            background-color: %2;
            border: 1px solid %3;
            border-radius: 4px;
            padding: 5px 15px;
            color: %4;
        }

        QPushButton:hover {
            background-color: %7;
            border: 1px solid %6;
        }

        QPushButton:pressed {
            background-color: %6;
            color: white;
        }

        QPushButton:disabled {
            color: #7F7F7F;
        }

        /* Menu Bar */
        QMenuBar {
            background-color: %2;
            color: %4;
        }

        QMenuBar::item:selected {
            background-color: %6;
            color: white;
        }

        /* Menu */
        QMenu {
            background-color: %2;
            border: 1px solid %3;
            color: %4;
        }

        QMenu::item:selected {
            background-color: %6;
            color: white;
        }

        /* Line Edit */
        QLineEdit {
            background-color: %5;
            border: 1px solid %3;
            border-radius: 3px;
            padding: 3px;
            color: %4;
        }

        QLineEdit:focus {
            border: 2px solid %6;
        }

        /* Spin Box */
        QSpinBox, QDoubleSpinBox {
            background-color: %5;
            border: 1px solid %3;
            border-radius: 3px;
            padding: 3px;
            color: %4;
        }

        /* Combo Box */
        QComboBox {
            background-color: %5;
            border: 1px solid %3;
            border-radius: 3px;
            padding: 3px;
            color: %4;
        }

        QComboBox:hover {
            border: 1px solid %6;
        }

        QComboBox QAbstractItemView {
            background-color: %2;
            selection-background-color: %6;
            border: 1px solid %3;
            color: %4;
        }

        /* Scroll Bar */
        QScrollBar:vertical {
            background-color: %2;
            width: 12px;
        }

        QScrollBar::handle:vertical {
            background-color: %3;
            border-radius: 6px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: %6;
        }

        QScrollBar:horizontal {
            background-color: %2;
            height: 12px;
        }

        QScrollBar::handle:horizontal {
            background-color: %3;
            border-radius: 6px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: %6;
        }

        /* Tab Widget */
        QTabWidget::pane {
            border: 1px solid %3;
            background-color: %2;
        }

        QTabBar::tab {
            background-color: %5;
            border: 1px solid %3;
            padding: 5px 10px;
            color: %4;
        }

        QTabBar::tab:selected {
            background-color: %2;
            border-bottom: 2px solid %6;
        }

        QTabBar::tab:hover {
            background-color: %7;
        }

        /* Label */
        QLabel {
            color: %4;
        }
    )")
    .arg(DongArchColors::toHtmlColor(DongArchColors::DarkTheme::Background))   // %1
    .arg(DongArchColors::toHtmlColor(DongArchColors::DarkTheme::Panel))        // %2
    .arg(DongArchColors::toHtmlColor(DongArchColors::DarkTheme::Border))       // %3
    .arg(DongArchColors::toHtmlColor(DongArchColors::DarkTheme::Text))         // %4
    .arg(DongArchColors::toHtmlColor(QColor(60, 60, 62)))                      // %5 - Slightly lighter panel
    .arg(DongArchColors::toHtmlColor(DongArchColors::CERAMIC_OCHRE))           // %6
    .arg(DongArchColors::toHtmlColor(QColor(80, 80, 82)))                      // %7 - Hover color
    .arg(DongArchColors::toHtmlColor(DongArchColors::SECTION_ORANGE));         // %8
}

QString QGMDarkModeManager::getCurrentStyleSheet() const {
    return mDarkModeEnabled ? buildDarkStyleSheet() : buildLightStyleSheet();
}

QString QGMDarkModeManager::getLightStyleSheet() const {
    return buildLightStyleSheet();
}

QString QGMDarkModeManager::getDarkStyleSheet() const {
    return buildDarkStyleSheet();
}

// ============================================================================
// Settings Persistence
// ============================================================================

void QGMDarkModeManager::loadSettings() {
    QSettings settings("DongArch3D", "DongArch3D");
    mDarkModeEnabled = settings.value("appearance/darkMode", false).toBool();
    qDebug() << "[QGMDarkModeManager] Loaded dark mode setting:" << mDarkModeEnabled;
}

void QGMDarkModeManager::saveSettings() {
    QSettings settings("DongArch3D", "DongArch3D");
    settings.setValue("appearance/darkMode", mDarkModeEnabled);
    qDebug() << "[QGMDarkModeManager] Saved dark mode setting:" << mDarkModeEnabled;
}
