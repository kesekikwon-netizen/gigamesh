/**
 * DongArch3D Welcome Screen
 *
 * 프로그램 시작 시 표시되는 웰컴 스크린
 * - 샘플 프로젝트 (3개 타일)
 * - 최근 파일 목록 (최대 10개)
 * - 튜토리얼 링크 (3개 버튼)
 *
 * @file DongArchWelcomeScreen.h
 * @author DongArch3D Team
 * @date 2025-11-08
 * @version 1.0.0
 */

#ifndef DONGARCH_WELCOME_SCREEN_H
#define DONGARCH_WELCOME_SCREEN_H

#include <QDialog>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>
#include <QSettings>

/**
 * @brief Welcome Screen Dialog
 *
 * 프로그램 시작 시 또는 "도움말 → 웰컴 스크린" 메뉴에서 호출
 */
class DongArchWelcomeScreen : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget (usually QGMMainWindow)
     * @param flags Window flags
     */
    explicit DongArchWelcomeScreen(QWidget* parent = nullptr, Qt::WindowFlags flags = {});

    /**
     * @brief Destructor
     */
    ~DongArchWelcomeScreen();

    /**
     * @brief Add recent file to the list
     * @param filePath Absolute path to recent file
     */
    void addRecentFile(const QString& filePath);

    /**
     * @brief Load recent files from QSettings
     */
    void loadRecentFiles();

    /**
     * @brief Check if "Don't show on startup" is enabled
     * @return True if welcome screen should be hidden on startup
     */
    static bool shouldShowOnStartup();

signals:
    /**
     * @brief Emitted when user wants to open a file
     * @param filePath File path to open
     */
    void openFileRequested(const QString& filePath);

    /**
     * @brief Emitted when user selects a sample project
     * @param sampleName Sample project identifier
     */
    void openSampleRequested(const QString& sampleName);

    /**
     * @brief Emitted when user clicks a tutorial link
     * @param tutorialUrl Tutorial URL
     */
    void openTutorialRequested(const QString& tutorialUrl);

protected:
    /**
     * @brief Handle language change events
     * @param event Change event
     */
    void changeEvent(QEvent* event) override;

private slots:
    /**
     * @brief Handle sample project button clicks
     */
    void onSampleProjectClicked();

    /**
     * @brief Handle recent file selection
     * @param item Selected list item
     */
    void onRecentFileClicked(QListWidgetItem* item);

    /**
     * @brief Handle tutorial link button clicks
     */
    void onTutorialClicked();

    /**
     * @brief Handle "Don't show on startup" checkbox
     * @param state Checkbox state
     */
    void onDontShowAgainChanged(int state);

    /**
     * @brief Handle "New Project" button
     */
    void onNewProjectClicked();

    /**
     * @brief Handle "Open Project" button
     */
    void onOpenProjectClicked();

private:
    /**
     * @brief Setup UI layout and widgets
     */
    void setupUI();

    /**
     * @brief Create sample projects section
     * @return QGroupBox containing sample project tiles
     */
    QGroupBox* createSampleProjectsSection();

    /**
     * @brief Create recent files section
     * @return QGroupBox containing recent files list
     */
    QGroupBox* createRecentFilesSection();

    /**
     * @brief Create tutorials section
     * @return QGroupBox containing tutorial links
     */
    QGroupBox* createTutorialsSection();

    /**
     * @brief Apply DongArchColors theme
     */
    void applyTheme();

    /**
     * @brief Retranslate UI strings
     */
    void retranslateUI();

    // --- UI Components ---
    QListWidget* mRecentFilesList;        //!< Recent files list widget
    QCheckBox* mDontShowAgainCheckbox;    //!< "Don't show on startup" checkbox
    QPushButton* mNewProjectButton;       //!< "New Project" button
    QPushButton* mOpenProjectButton;      //!< "Open Project" button
    QPushButton* mCloseButton;            //!< "Close" button

    // Sample project buttons (3)
    QPushButton* mSampleLithic;           //!< Sample: Lithic artifact
    QPushButton* mSampleCeramic;          //!< Sample: Ceramic vessel
    QPushButton* mSampleSite;             //!< Sample: Archaeological site

    // Tutorial buttons (3)
    QPushButton* mTutorialBasics;         //!< Tutorial: Basic usage
    QPushButton* mTutorialMeasurement;    //!< Tutorial: Measurement tools
    QPushButton* mTutorialNPR;            //!< Tutorial: NPR rendering

    // --- Settings Keys ---
    static constexpr const char* SETTINGS_KEY_SHOW_ON_STARTUP = "WelcomeScreen/ShowOnStartup";
    static constexpr const char* SETTINGS_KEY_RECENT_FILES = "RecentFiles/FileList";
    static constexpr int MAX_RECENT_FILES = 10;
};

#endif // DONGARCH_WELCOME_SCREEN_H
