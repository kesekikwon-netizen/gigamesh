/**
 * DongArch3D Welcome Screen Implementation
 *
 * @file DongArchWelcomeScreen.cpp
 * @author DongArch3D Team
 * @date 2025-11-08
 * @version 1.0.0
 */

#include "DongArchWelcomeScreen.h"
#include "DongArchColors.h"
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QPixmap>
#include <QIcon>
#include <QFont>
#include <QFileInfo>
#include <QCoreApplication>

// Constructor
DongArchWelcomeScreen::DongArchWelcomeScreen(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , mRecentFilesList(nullptr)
    , mDontShowAgainCheckbox(nullptr)
    , mNewProjectButton(nullptr)
    , mOpenProjectButton(nullptr)
    , mCloseButton(nullptr)
    , mSampleLithic(nullptr)
    , mSampleCeramic(nullptr)
    , mSampleSite(nullptr)
    , mTutorialBasics(nullptr)
    , mTutorialMeasurement(nullptr)
    , mTutorialNPR(nullptr)
{
    setupUI();
    applyTheme();
    retranslateUI();
    loadRecentFiles();
}

// Destructor
DongArchWelcomeScreen::~DongArchWelcomeScreen() {
    // Widgets are automatically deleted by Qt parent-child system
}

// Setup UI layout
void DongArchWelcomeScreen::setupUI() {
    // Window title set in retranslateUI()
    setMinimumSize(600, 450);
    resize(700, 500);

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // --- Header Section ---
    QLabel* titleLabel = new QLabel(this);
    titleLabel->setText("DongArch3D");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* subtitleLabel = new QLabel(this);
    subtitleLabel->setText("동국문화재연구원 전용 3D 실측 프로그램");
    QFont subtitleFont = subtitleLabel->font();
    subtitleFont.setPointSize(12);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #666666;");

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(10);

    // --- Content Layout (2 columns) ---
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    // Left column: Sample Projects + Tutorials
    QVBoxLayout* leftColumn = new QVBoxLayout();
    leftColumn->setSpacing(15);
    leftColumn->addWidget(createSampleProjectsSection());
    leftColumn->addWidget(createTutorialsSection());
    leftColumn->addStretch();

    // Right column: Recent Files + Action Buttons
    QVBoxLayout* rightColumn = new QVBoxLayout();
    rightColumn->setSpacing(15);
    rightColumn->addWidget(createRecentFilesSection());

    // Action buttons (New Project, Open Project)
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);

    mNewProjectButton = new QPushButton(this);
    mNewProjectButton->setIcon(QIcon::fromTheme("document-new"));
    mNewProjectButton->setMinimumHeight(40);
    connect(mNewProjectButton, &QPushButton::clicked, this, &DongArchWelcomeScreen::onNewProjectClicked);

    mOpenProjectButton = new QPushButton(this);
    mOpenProjectButton->setIcon(QIcon::fromTheme("document-open"));
    mOpenProjectButton->setMinimumHeight(40);
    connect(mOpenProjectButton, &QPushButton::clicked, this, &DongArchWelcomeScreen::onOpenProjectClicked);

    actionLayout->addWidget(mNewProjectButton);
    actionLayout->addWidget(mOpenProjectButton);
    rightColumn->addLayout(actionLayout);

    contentLayout->addLayout(leftColumn, 1);
    contentLayout->addLayout(rightColumn, 1);

    mainLayout->addLayout(contentLayout, 1);

    // --- Footer Section ---
    QHBoxLayout* footerLayout = new QHBoxLayout();
    footerLayout->setSpacing(10);

    mDontShowAgainCheckbox = new QCheckBox(this);
    QSettings settings;
    bool showOnStartup = settings.value(SETTINGS_KEY_SHOW_ON_STARTUP, true).toBool();
    mDontShowAgainCheckbox->setChecked(!showOnStartup);
    connect(mDontShowAgainCheckbox, &QCheckBox::stateChanged, this, &DongArchWelcomeScreen::onDontShowAgainChanged);

    mCloseButton = new QPushButton(this);
    mCloseButton->setMinimumWidth(100);
    mCloseButton->setDefault(true);
    connect(mCloseButton, &QPushButton::clicked, this, &QDialog::accept);

    footerLayout->addWidget(mDontShowAgainCheckbox);
    footerLayout->addStretch();
    footerLayout->addWidget(mCloseButton);

    mainLayout->addLayout(footerLayout);
}

// Create Sample Projects Section
QGroupBox* DongArchWelcomeScreen::createSampleProjectsSection() {
    QGroupBox* groupBox = new QGroupBox(this);

    QGridLayout* gridLayout = new QGridLayout(groupBox);
    gridLayout->setSpacing(10);

    // Sample 1: Lithic Artifact
    mSampleLithic = new QPushButton(groupBox);
    mSampleLithic->setIcon(QIcon(":/archaeology/lithic_handaxe.svg"));
    mSampleLithic->setIconSize(QSize(48, 48));
    mSampleLithic->setMinimumHeight(80);
    mSampleLithic->setProperty("sampleName", "sample_lithic");
    connect(mSampleLithic, &QPushButton::clicked, this, &DongArchWelcomeScreen::onSampleProjectClicked);

    // Sample 2: Ceramic Vessel
    mSampleCeramic = new QPushButton(groupBox);
    mSampleCeramic->setIcon(QIcon(":/archaeology/pottery_vessel.svg"));
    mSampleCeramic->setIconSize(QSize(48, 48));
    mSampleCeramic->setMinimumHeight(80);
    mSampleCeramic->setProperty("sampleName", "sample_ceramic");
    connect(mSampleCeramic, &QPushButton::clicked, this, &DongArchWelcomeScreen::onSampleProjectClicked);

    // Sample 3: Archaeological Site
    mSampleSite = new QPushButton(groupBox);
    mSampleSite->setIcon(QIcon(":/archaeology/site_excavation.svg"));
    mSampleSite->setIconSize(QSize(48, 48));
    mSampleSite->setMinimumHeight(80);
    mSampleSite->setProperty("sampleName", "sample_site");
    connect(mSampleSite, &QPushButton::clicked, this, &DongArchWelcomeScreen::onSampleProjectClicked);

    gridLayout->addWidget(mSampleLithic, 0, 0);
    gridLayout->addWidget(mSampleCeramic, 0, 1);
    gridLayout->addWidget(mSampleSite, 1, 0, 1, 2);  // Span 2 columns

    return groupBox;
}

// Create Recent Files Section
QGroupBox* DongArchWelcomeScreen::createRecentFilesSection() {
    QGroupBox* groupBox = new QGroupBox(this);

    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    layout->setSpacing(10);

    mRecentFilesList = new QListWidget(groupBox);
    mRecentFilesList->setMinimumHeight(200);
    mRecentFilesList->setAlternatingRowColors(true);
    mRecentFilesList->setIconSize(QSize(16, 16));
    connect(mRecentFilesList, &QListWidget::itemDoubleClicked, this, &DongArchWelcomeScreen::onRecentFileClicked);

    layout->addWidget(mRecentFilesList);

    return groupBox;
}

// Create Tutorials Section
QGroupBox* DongArchWelcomeScreen::createTutorialsSection() {
    QGroupBox* groupBox = new QGroupBox(this);

    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    layout->setSpacing(10);

    // Tutorial 1: Basic Usage
    mTutorialBasics = new QPushButton(groupBox);
    mTutorialBasics->setIcon(QIcon::fromTheme("help-contents"));
    mTutorialBasics->setMinimumHeight(40);
    mTutorialBasics->setProperty("tutorialUrl", "https://gigamesh.eu/tutorials/basic");
    connect(mTutorialBasics, &QPushButton::clicked, this, &DongArchWelcomeScreen::onTutorialClicked);

    // Tutorial 2: Measurement Tools
    mTutorialMeasurement = new QPushButton(groupBox);
    mTutorialMeasurement->setIcon(QIcon::fromTheme("measure"));
    mTutorialMeasurement->setMinimumHeight(40);
    mTutorialMeasurement->setProperty("tutorialUrl", "https://gigamesh.eu/tutorials/measurement");
    connect(mTutorialMeasurement, &QPushButton::clicked, this, &DongArchWelcomeScreen::onTutorialClicked);

    // Tutorial 3: NPR Rendering
    mTutorialNPR = new QPushButton(groupBox);
    mTutorialNPR->setIcon(QIcon::fromTheme("applications-graphics"));
    mTutorialNPR->setMinimumHeight(40);
    mTutorialNPR->setProperty("tutorialUrl", "https://gigamesh.eu/tutorials/npr");
    connect(mTutorialNPR, &QPushButton::clicked, this, &DongArchWelcomeScreen::onTutorialClicked);

    layout->addWidget(mTutorialBasics);
    layout->addWidget(mTutorialMeasurement);
    layout->addWidget(mTutorialNPR);

    return groupBox;
}

// Apply DongArchColors theme
void DongArchWelcomeScreen::applyTheme() {
    // Use Light Theme by default
    QString styleSheet = QString(
        "QGroupBox {"
        "   border: 2px solid %1;"
        "   border-radius: 5px;"
        "   margin-top: 10px;"
        "   padding: 10px;"
        "   background-color: %2;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   padding: 5px 10px;"
        "   color: %3;"
        "   font-weight: bold;"
        "}"
        "QPushButton {"
        "   background-color: %4;"
        "   border: 1px solid %1;"
        "   border-radius: 3px;"
        "   padding: 5px;"
        "   color: %3;"
        "}"
        "QPushButton:hover {"
        "   background-color: %5;"
        "}"
        "QPushButton:pressed {"
        "   background-color: %6;"
        "}"
        "QListWidget {"
        "   background-color: white;"
        "   border: 1px solid %1;"
        "   border-radius: 3px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: %7;"
        "   color: white;"
        "}"
    )
    .arg(DongArchColors::toHtmlColor(DongArchColors::MEDIUM_GRAY))
    .arg(DongArchColors::toHtmlColor(DongArchColors::CORTEX_WHITE))
    .arg(DongArchColors::toHtmlColor(DongArchColors::DARK_BROWN))
    .arg(DongArchColors::toHtmlColor(DongArchColors::LIGHT_GRAY))
    .arg(DongArchColors::toHtmlColor(DongArchColors::LIGHT_OCHRE))
    .arg(DongArchColors::toHtmlColor(DongArchColors::CERAMIC_OCHRE))
    .arg(DongArchColors::toHtmlColor(DongArchColors::SELECTION_BLUE));

    setStyleSheet(styleSheet);
}

// Retranslate UI strings
void DongArchWelcomeScreen::retranslateUI() {
    // Window title
    setWindowTitle(tr("DongArch3D - 웰컴 스크린"));

    // Sample Projects Section
    if (mSampleLithic) {
        mSampleLithic->setText(tr("석기 샘플\n주먹도끼 3D 스캔"));
    }
    if (mSampleCeramic) {
        mSampleCeramic->setText(tr("토기 샘플\n신석기 토기"));
    }
    if (mSampleSite) {
        mSampleSite->setText(tr("발굴 현장 샘플\n유적지 3D 모델"));
    }

    // Tutorials Section
    if (mTutorialBasics) {
        mTutorialBasics->setText(tr("기본 사용법"));
    }
    if (mTutorialMeasurement) {
        mTutorialMeasurement->setText(tr("실측 도구 사용법"));
    }
    if (mTutorialNPR) {
        mTutorialNPR->setText(tr("고고학 렌더링"));
    }

    // Action Buttons
    if (mNewProjectButton) {
        mNewProjectButton->setText(tr("새 프로젝트"));
    }
    if (mOpenProjectButton) {
        mOpenProjectButton->setText(tr("프로젝트 열기"));
    }

    // Footer
    if (mDontShowAgainCheckbox) {
        mDontShowAgainCheckbox->setText(tr("시작 시 이 화면 표시 안 함"));
    }
    if (mCloseButton) {
        mCloseButton->setText(tr("닫기"));
    }

    // Group Box Titles (need to find and update)
    QList<QGroupBox*> groupBoxes = findChildren<QGroupBox*>();
    if (groupBoxes.size() >= 3) {
        groupBoxes[0]->setTitle(tr("샘플 프로젝트"));
        groupBoxes[1]->setTitle(tr("최근 파일"));
        groupBoxes[2]->setTitle(tr("튜토리얼"));
    }
}

// Handle language change events
void DongArchWelcomeScreen::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUI();
    }
    QDialog::changeEvent(event);
}

// Add recent file to the list
void DongArchWelcomeScreen::addRecentFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        return;
    }

    // Check if file is already in the list
    for (int i = 0; i < mRecentFilesList->count(); ++i) {
        QListWidgetItem* item = mRecentFilesList->item(i);
        if (item->data(Qt::UserRole).toString() == filePath) {
            // Move to top
            mRecentFilesList->takeItem(i);
            mRecentFilesList->insertItem(0, item);
            return;
        }
    }

    // Add new item
    QListWidgetItem* item = new QListWidgetItem(mRecentFilesList);
    item->setText(fileInfo.fileName());
    item->setToolTip(filePath);
    item->setData(Qt::UserRole, filePath);
    item->setIcon(QIcon::fromTheme("document"));

    mRecentFilesList->insertItem(0, item);

    // Limit to MAX_RECENT_FILES
    while (mRecentFilesList->count() > MAX_RECENT_FILES) {
        delete mRecentFilesList->takeItem(MAX_RECENT_FILES);
    }

    // Save to QSettings
    QSettings settings;
    QStringList recentFiles;
    for (int i = 0; i < mRecentFilesList->count(); ++i) {
        recentFiles << mRecentFilesList->item(i)->data(Qt::UserRole).toString();
    }
    settings.setValue(SETTINGS_KEY_RECENT_FILES, recentFiles);
}

// Load recent files from QSettings
void DongArchWelcomeScreen::loadRecentFiles() {
    QSettings settings;
    QStringList recentFiles = settings.value(SETTINGS_KEY_RECENT_FILES).toStringList();

    mRecentFilesList->clear();

    for (const QString& filePath : recentFiles) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists()) {
            QListWidgetItem* item = new QListWidgetItem(mRecentFilesList);
            item->setText(fileInfo.fileName());
            item->setToolTip(filePath);
            item->setData(Qt::UserRole, filePath);
            item->setIcon(QIcon::fromTheme("document"));
        }
    }
}

// Check if welcome screen should be shown on startup
bool DongArchWelcomeScreen::shouldShowOnStartup() {
    QSettings settings;
    return settings.value(SETTINGS_KEY_SHOW_ON_STARTUP, true).toBool();
}

// --- Slots ---

// Handle sample project button clicks
void DongArchWelcomeScreen::onSampleProjectClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString sampleName = button->property("sampleName").toString();
        emit openSampleRequested(sampleName);
        accept();  // Close dialog
    }
}

// Handle recent file selection
void DongArchWelcomeScreen::onRecentFileClicked(QListWidgetItem* item) {
    if (item) {
        QString filePath = item->data(Qt::UserRole).toString();
        emit openFileRequested(filePath);
        accept();  // Close dialog
    }
}

// Handle tutorial link button clicks
void DongArchWelcomeScreen::onTutorialClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button) {
        QString tutorialUrl = button->property("tutorialUrl").toString();
        emit openTutorialRequested(tutorialUrl);
        QDesktopServices::openUrl(QUrl(tutorialUrl));
    }
}

// Handle "Don't show on startup" checkbox
void DongArchWelcomeScreen::onDontShowAgainChanged(int state) {
    QSettings settings;
    bool showOnStartup = (state == Qt::Unchecked);
    settings.setValue(SETTINGS_KEY_SHOW_ON_STARTUP, showOnStartup);
}

// Handle "New Project" button
void DongArchWelcomeScreen::onNewProjectClicked() {
    // Emit signal or call parent method
    accept();  // Close dialog and let parent handle new project
}

// Handle "Open Project" button
void DongArchWelcomeScreen::onOpenProjectClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("프로젝트 열기"),
        QString(),
        tr("DongArch3D 프로젝트 (*.dongarch3d);;PLY 파일 (*.ply);;OBJ 파일 (*.obj);;모든 파일 (*.*)")
    );

    if (!fileName.isEmpty()) {
        emit openFileRequested(fileName);
        accept();  // Close dialog
    }
}
