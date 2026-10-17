/**
 * QGMDarkModeManager.h
 *
 * DongArch3D Dark Mode Management System
 * Task 108: Dark Mode Support
 *
 * Handles theme switching between light and dark modes for the entire application.
 * Uses DongArchColors.h theme definitions and applies Qt stylesheets dynamically.
 *
 * @file QGMDarkModeManager.h
 * @author DongArch3D Team
 * @date 2025-11-08
 * @version 1.0.0
 */

#ifndef QGMDARKMODEMANAGER_H
#define QGMDARKMODEMANAGER_H

#include <QObject>
#include <QString>
#include <QApplication>
#include <QPalette>
#include "DongArchColors.h"

/**
 * @brief Dark Mode Manager for DongArch3D
 *
 * Singleton class that manages theme switching between light and dark modes.
 * Applies color schemes and stylesheets to all widgets in the application.
 *
 * Usage:
 * @code
 *   QGMDarkModeManager* manager = QGMDarkModeManager::instance();
 *   manager->setDarkMode(true);  // Enable dark mode
 *   manager->toggleDarkMode();   // Toggle current mode
 *   bool isDark = manager->isDarkMode();
 * @endcode
 */
class QGMDarkModeManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Get singleton instance
     * @return QGMDarkModeManager* Singleton instance
     */
    static QGMDarkModeManager* instance();

    /**
     * @brief Check if dark mode is currently enabled
     * @return bool True if dark mode is active
     */
    bool isDarkMode() const { return mDarkModeEnabled; }

    /**
     * @brief Get current theme stylesheet
     * @return QString QSS stylesheet for current theme
     */
    QString getCurrentStyleSheet() const;

    /**
     * @brief Get light theme stylesheet
     * @return QString QSS stylesheet for light theme
     */
    QString getLightStyleSheet() const;

    /**
     * @brief Get dark theme stylesheet
     * @return QString QSS stylesheet for dark theme
     */
    QString getDarkStyleSheet() const;

public slots:
    /**
     * @brief Set dark mode on/off
     * @param enabled True to enable dark mode, false for light mode
     */
    void setDarkMode(bool enabled);

    /**
     * @brief Toggle between light and dark modes
     */
    void toggleDarkMode();

    /**
     * @brief Apply current theme to application
     *
     * Applies palette and stylesheet to QApplication and all widgets.
     * Called automatically when theme changes.
     */
    void applyTheme();

signals:
    /**
     * @brief Emitted when theme changes
     * @param isDarkMode True if switched to dark mode, false if light mode
     */
    void themeChanged(bool isDarkMode);

private:
    // Singleton pattern: private constructor
    explicit QGMDarkModeManager(QObject *parent = nullptr);
    ~QGMDarkModeManager() override;

    // Disable copy constructor and assignment operator
    QGMDarkModeManager(const QGMDarkModeManager&) = delete;
    QGMDarkModeManager& operator=(const QGMDarkModeManager&) = delete;

    /**
     * @brief Create and apply light theme palette
     * @return QPalette Light theme palette
     */
    QPalette createLightPalette() const;

    /**
     * @brief Create and apply dark theme palette
     * @return QPalette Dark theme palette
     */
    QPalette createDarkPalette() const;

    /**
     * @brief Build light theme stylesheet (QSS)
     * @return QString QSS stylesheet for light theme
     */
    QString buildLightStyleSheet() const;

    /**
     * @brief Build dark theme stylesheet (QSS)
     * @return QString QSS stylesheet for dark theme
     */
    QString buildDarkStyleSheet() const;

    /**
     * @brief Load theme preference from settings
     */
    void loadSettings();

    /**
     * @brief Save theme preference to settings
     */
    void saveSettings();

private:
    static QGMDarkModeManager* mInstance;  //!< Singleton instance
    bool mDarkModeEnabled;                 //!< Current theme state (true = dark, false = light)
};

#endif // QGMDARKMODEMANAGER_H
