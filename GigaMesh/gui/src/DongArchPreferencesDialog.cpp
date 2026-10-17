/**
 * DongArch3D Preferences Dialog Implementation
 *
 * @file DongArchPreferencesDialog.cpp
 * @author DongArch3D Team
 * @date 2025-11-08
 * @version 1.0.0
 */

#include "DongArchPreferencesDialog.h"
#include "DongArchColors.h"
#include "QGMDarkModeManager.h"

#include <QGroupBox>
#include <QMessageBox>

// ============================================================================
// Constructor / Destructor
// ============================================================================

DongArchPreferencesDialog::DongArchPreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , mBackgroundColor(Qt::white)
{
    // QSettings 초기화 (조직명, 앱 이름)
    mSettings = new QSettings("DongukCulturalHeritage", "DongArch3D", this);

    setWindowTitle(tr("환경설정"));
    setMinimumSize(600, 500);

    initUI();
    loadSettings();
}

DongArchPreferencesDialog::~DongArchPreferencesDialog()
{
    // QSettings는 parent가 있으므로 자동 삭제됨
}

// ============================================================================
// UI Initialization
// ============================================================================

void DongArchPreferencesDialog::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Tab Widget 생성
    mTabWidget = new QTabWidget(this);
    mTabWidget->addTab(createGeneralTab(), tr("일반"));
    mTabWidget->addTab(createRenderingTab(), tr("렌더링"));
    mTabWidget->addTab(createUnitsTab(), tr("단위"));
    mTabWidget->addTab(createAdvancedTab(), tr("고급"));

    mainLayout->addWidget(mTabWidget);

    // Button Box 생성
    mButtonBox = new QDialogButtonBox(this);
    mButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // 적용 버튼 추가
    mApplyButton = mButtonBox->addButton(tr("적용"), QDialogButtonBox::ApplyRole);

    // 기본값 복원 버튼 추가
    mRestoreDefaultsButton = mButtonBox->addButton(tr("기본값 복원"), QDialogButtonBox::ResetRole);

    mainLayout->addWidget(mButtonBox);

    // Signal/Slot 연결
    connect(mButtonBox, &QDialogButtonBox::accepted, this, &DongArchPreferencesDialog::onAccept);
    connect(mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(mApplyButton, &QPushButton::clicked, this, &DongArchPreferencesDialog::onApply);
    connect(mRestoreDefaultsButton, &QPushButton::clicked, this, &DongArchPreferencesDialog::onRestoreDefaults);

    // DongArchColors 스타일 적용
    setStyleSheet(QString(
        "QDialog {"
        "    background-color: %1;"
        "    color: %2;"
        "}"
        "QTabWidget::pane {"
        "    border: 1px solid %3;"
        "    background-color: %1;"
        "}"
        "QTabBar::tab {"
        "    background-color: %4;"
        "    color: %2;"
        "    padding: 8px 16px;"
        "    border: 1px solid %3;"
        "    border-bottom: none;"
        "    margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: %5;"
        "    color: white;"
        "}"
        "QTabBar::tab:hover {"
        "    background-color: %6;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid %3;"
        "    border-radius: 4px;"
        "    margin-top: 8px;"
        "    padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    padding: 0 4px;"
        "    color: %5;"
        "}"
    )
    .arg(DongArchColors::CORTEX_WHITE.name())
    .arg(DongArchColors::DARK_BROWN.name())
    .arg(DongArchColors::MEDIUM_GRAY.name())
    .arg(DongArchColors::LIGHT_GRAY.name())
    .arg(DongArchColors::SOIL_LAYER_BROWN.name())
    .arg(DongArchColors::LIGHT_OCHRE.name())
    );
}

// ============================================================================
// Tab Creation
// ============================================================================

QWidget* DongArchPreferencesDialog::createGeneralTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 언어 설정 그룹
    QGroupBox* languageGroup = new QGroupBox(tr("언어 설정"), tab);
    QFormLayout* languageLayout = new QFormLayout(languageGroup);

    mLanguageCombo = new QComboBox(languageGroup);
    mLanguageCombo->addItem(tr("한국어"), "ko");
    mLanguageCombo->addItem(tr("English"), "en");
    mLanguageCombo->addItem(tr("日本語"), "ja");
    mLanguageCombo->addItem(tr("Deutsch"), "de");
    languageLayout->addRow(tr("언어:"), mLanguageCombo);

    layout->addWidget(languageGroup);

    // 폰트 설정 그룹
    QGroupBox* fontGroup = new QGroupBox(tr("폰트 설정"), tab);
    QFormLayout* fontLayout = new QFormLayout(fontGroup);

    mFontSizeSpinBox = new QSpinBox(fontGroup);
    mFontSizeSpinBox->setRange(8, 20);
    mFontSizeSpinBox->setValue(10);
    mFontSizeSpinBox->setSuffix(" pt");
    fontLayout->addRow(tr("폰트 크기:"), mFontSizeSpinBox);

    layout->addWidget(fontGroup);

    // 테마 설정 그룹
    QGroupBox* themeGroup = new QGroupBox(tr("테마 설정"), tab);
    QFormLayout* themeLayout = new QFormLayout(themeGroup);

    mThemeCombo = new QComboBox(themeGroup);
    mThemeCombo->addItem(tr("밝은 모드"), 0);
    mThemeCombo->addItem(tr("어두운 모드"), 1);
    connect(mThemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DongArchPreferencesDialog::onThemeChanged);
    themeLayout->addRow(tr("테마:"), mThemeCombo);

    layout->addWidget(themeGroup);

    layout->addStretch();

    return tab;
}

QWidget* DongArchPreferencesDialog::createRenderingTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 배경 설정 그룹
    QGroupBox* backgroundGroup = new QGroupBox(tr("배경 설정"), tab);
    QFormLayout* backgroundLayout = new QFormLayout(backgroundGroup);

    mBackgroundColorButton = new QPushButton(tr("색상 선택..."), backgroundGroup);
    mBackgroundColorButton->setMinimumHeight(32);
    connect(mBackgroundColorButton, &QPushButton::clicked,
            this, &DongArchPreferencesDialog::onSelectBackgroundColor);
    backgroundLayout->addRow(tr("배경색:"), mBackgroundColorButton);

    layout->addWidget(backgroundGroup);

    // 조명 설정 그룹
    QGroupBox* lightingGroup = new QGroupBox(tr("조명 설정"), tab);
    QVBoxLayout* lightingLayout = new QVBoxLayout(lightingGroup);

    mLightingCheckBox = new QCheckBox(tr("조명 활성화"), lightingGroup);
    mLightingCheckBox->setChecked(true);
    lightingLayout->addWidget(mLightingCheckBox);

    QHBoxLayout* intensityLayout = new QHBoxLayout();
    QLabel* intensityTextLabel = new QLabel(tr("조명 강도:"), lightingGroup);
    mLightingIntensitySlider = new QSlider(Qt::Horizontal, lightingGroup);
    mLightingIntensitySlider->setRange(0, 100);
    mLightingIntensitySlider->setValue(80);
    mLightingIntensityLabel = new QLabel("80%", lightingGroup);
    mLightingIntensityLabel->setMinimumWidth(40);

    connect(mLightingIntensitySlider, &QSlider::valueChanged, [this](int value) {
        mLightingIntensityLabel->setText(QString("%1%").arg(value));
    });

    intensityLayout->addWidget(intensityTextLabel);
    intensityLayout->addWidget(mLightingIntensitySlider);
    intensityLayout->addWidget(mLightingIntensityLabel);
    lightingLayout->addLayout(intensityLayout);

    layout->addWidget(lightingGroup);

    // 그리드 설정 그룹
    QGroupBox* gridGroup = new QGroupBox(tr("그리드 설정"), tab);
    QVBoxLayout* gridLayout = new QVBoxLayout(gridGroup);

    mGridCheckBox = new QCheckBox(tr("그리드 표시"), gridGroup);
    mGridCheckBox->setChecked(true);
    gridLayout->addWidget(mGridCheckBox);

    QFormLayout* gridFormLayout = new QFormLayout();
    mGridSpacingSpinBox = new QSpinBox(gridGroup);
    mGridSpacingSpinBox->setRange(1, 100);
    mGridSpacingSpinBox->setValue(10);
    mGridSpacingSpinBox->setSuffix(" mm");
    gridFormLayout->addRow(tr("그리드 간격:"), mGridSpacingSpinBox);
    gridLayout->addLayout(gridFormLayout);

    layout->addWidget(gridGroup);

    layout->addStretch();

    return tab;
}

QWidget* DongArchPreferencesDialog::createUnitsTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 길이 단위 그룹
    QGroupBox* lengthGroup = new QGroupBox(tr("길이 단위 설정"), tab);
    QFormLayout* lengthLayout = new QFormLayout(lengthGroup);

    mLengthUnitCombo = new QComboBox(lengthGroup);
    mLengthUnitCombo->addItem("mm (밀리미터)", "mm");
    mLengthUnitCombo->addItem("cm (센티미터)", "cm");
    mLengthUnitCombo->addItem("m (미터)", "m");
    lengthLayout->addRow(tr("길이 단위:"), mLengthUnitCombo);

    layout->addWidget(lengthGroup);

    // 각도 단위 그룹
    QGroupBox* angleGroup = new QGroupBox(tr("각도 단위 설정"), tab);
    QFormLayout* angleLayout = new QFormLayout(angleGroup);

    mAngleUnitCombo = new QComboBox(angleGroup);
    mAngleUnitCombo->addItem(tr("도 (Degree)"), "degree");
    mAngleUnitCombo->addItem(tr("라디안 (Radian)"), "radian");
    angleLayout->addRow(tr("각도 단위:"), mAngleUnitCombo);

    layout->addWidget(angleGroup);

    // 표시 정밀도 그룹
    QGroupBox* precisionGroup = new QGroupBox(tr("표시 정밀도"), tab);
    QFormLayout* precisionLayout = new QFormLayout(precisionGroup);

    mDecimalPlacesSpinBox = new QSpinBox(precisionGroup);
    mDecimalPlacesSpinBox->setRange(0, 6);
    mDecimalPlacesSpinBox->setValue(2);
    mDecimalPlacesSpinBox->setSuffix(tr(" 자리"));
    precisionLayout->addRow(tr("소수점 자리수:"), mDecimalPlacesSpinBox);

    layout->addWidget(precisionGroup);

    layout->addStretch();

    return tab;
}

QWidget* DongArchPreferencesDialog::createAdvancedTab()
{
    QWidget* tab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(tab);

    // 캐시 설정 그룹
    QGroupBox* cacheGroup = new QGroupBox(tr("캐시 설정"), tab);
    QFormLayout* cacheLayout = new QFormLayout(cacheGroup);

    mCacheSizeSpinBox = new QSpinBox(cacheGroup);
    mCacheSizeSpinBox->setRange(100, 2000);
    mCacheSizeSpinBox->setValue(512);
    mCacheSizeSpinBox->setSingleStep(100);
    mCacheSizeSpinBox->setSuffix(" MB");
    cacheLayout->addRow(tr("캐시 크기:"), mCacheSizeSpinBox);

    layout->addWidget(cacheGroup);

    // 로그 설정 그룹
    QGroupBox* logGroup = new QGroupBox(tr("로그 설정"), tab);
    QFormLayout* logLayout = new QFormLayout(logGroup);

    mLogLevelCombo = new QComboBox(logGroup);
    mLogLevelCombo->addItem(tr("디버그 (Debug)"), "debug");
    mLogLevelCombo->addItem(tr("정보 (Info)"), "info");
    mLogLevelCombo->addItem(tr("경고 (Warning)"), "warning");
    mLogLevelCombo->addItem(tr("오류 (Error)"), "error");
    mLogLevelCombo->setCurrentIndex(1); // Default: Info
    logLayout->addRow(tr("로그 레벨:"), mLogLevelCombo);

    layout->addWidget(logGroup);

    // GPU 설정 그룹
    QGroupBox* gpuGroup = new QGroupBox(tr("GPU 설정"), tab);
    QVBoxLayout* gpuLayout = new QVBoxLayout(gpuGroup);

    mGpuAccelerationCheckBox = new QCheckBox(tr("GPU 가속 활성화"), gpuGroup);
    mGpuAccelerationCheckBox->setChecked(true);
    gpuLayout->addWidget(mGpuAccelerationCheckBox);

    QLabel* gpuNote = new QLabel(
        tr("* GPU 가속을 비활성화하면 성능이 저하될 수 있습니다."),
        gpuGroup
    );
    gpuNote->setWordWrap(true);
    gpuNote->setStyleSheet("color: gray; font-size: 9pt;");
    gpuLayout->addWidget(gpuNote);

    layout->addWidget(gpuGroup);

    // 자동 저장 설정 그룹
    QGroupBox* autoSaveGroup = new QGroupBox(tr("자동 저장 설정"), tab);
    QVBoxLayout* autoSaveLayout = new QVBoxLayout(autoSaveGroup);

    mAutoSaveCheckBox = new QCheckBox(tr("자동 저장 활성화"), autoSaveGroup);
    mAutoSaveCheckBox->setChecked(false);
    autoSaveLayout->addWidget(mAutoSaveCheckBox);

    QFormLayout* autoSaveFormLayout = new QFormLayout();
    mAutoSaveIntervalSpinBox = new QSpinBox(autoSaveGroup);
    mAutoSaveIntervalSpinBox->setRange(1, 60);
    mAutoSaveIntervalSpinBox->setValue(5);
    mAutoSaveIntervalSpinBox->setSuffix(tr(" 분"));
    autoSaveFormLayout->addRow(tr("저장 간격:"), mAutoSaveIntervalSpinBox);
    autoSaveLayout->addLayout(autoSaveFormLayout);

    layout->addWidget(autoSaveGroup);

    layout->addStretch();

    return tab;
}

// ============================================================================
// Settings Management
// ============================================================================

void DongArchPreferencesDialog::loadSettings()
{
    // 일반 탭
    QString language = mSettings->value("General/Language", "ko").toString();
    int langIndex = mLanguageCombo->findData(language);
    if (langIndex >= 0) {
        mLanguageCombo->setCurrentIndex(langIndex);
    }

    int fontSize = mSettings->value("General/FontSize", 10).toInt();
    mFontSizeSpinBox->setValue(fontSize);

    bool darkMode = mSettings->value("General/DarkMode", false).toBool();
    mThemeCombo->setCurrentIndex(darkMode ? 1 : 0);

    // 렌더링 탭
    mBackgroundColor = mSettings->value("Rendering/BackgroundColor", QColor(Qt::white)).value<QColor>();
    updateBackgroundColorButton();

    bool lightingEnabled = mSettings->value("Rendering/LightingEnabled", true).toBool();
    mLightingCheckBox->setChecked(lightingEnabled);

    int lightingIntensity = mSettings->value("Rendering/LightingIntensity", 80).toInt();
    mLightingIntensitySlider->setValue(lightingIntensity);

    bool gridEnabled = mSettings->value("Rendering/GridEnabled", true).toBool();
    mGridCheckBox->setChecked(gridEnabled);

    int gridSpacing = mSettings->value("Rendering/GridSpacing", 10).toInt();
    mGridSpacingSpinBox->setValue(gridSpacing);

    // 단위 탭
    QString lengthUnit = mSettings->value("Units/LengthUnit", "mm").toString();
    int lengthIndex = mLengthUnitCombo->findData(lengthUnit);
    if (lengthIndex >= 0) {
        mLengthUnitCombo->setCurrentIndex(lengthIndex);
    }

    QString angleUnit = mSettings->value("Units/AngleUnit", "degree").toString();
    int angleIndex = mAngleUnitCombo->findData(angleUnit);
    if (angleIndex >= 0) {
        mAngleUnitCombo->setCurrentIndex(angleIndex);
    }

    int decimalPlaces = mSettings->value("Units/DecimalPlaces", 2).toInt();
    mDecimalPlacesSpinBox->setValue(decimalPlaces);

    // 고급 탭
    int cacheSize = mSettings->value("Advanced/CacheSize", 512).toInt();
    mCacheSizeSpinBox->setValue(cacheSize);

    QString logLevel = mSettings->value("Advanced/LogLevel", "info").toString();
    int logIndex = mLogLevelCombo->findData(logLevel);
    if (logIndex >= 0) {
        mLogLevelCombo->setCurrentIndex(logIndex);
    }

    bool gpuEnabled = mSettings->value("Advanced/GpuAcceleration", true).toBool();
    mGpuAccelerationCheckBox->setChecked(gpuEnabled);

    bool autoSaveEnabled = mSettings->value("Advanced/AutoSave", false).toBool();
    mAutoSaveCheckBox->setChecked(autoSaveEnabled);

    int autoSaveInterval = mSettings->value("Advanced/AutoSaveInterval", 5).toInt();
    mAutoSaveIntervalSpinBox->setValue(autoSaveInterval);
}

void DongArchPreferencesDialog::saveSettings()
{
    // 일반 탭
    mSettings->setValue("General/Language", mLanguageCombo->currentData().toString());
    mSettings->setValue("General/FontSize", mFontSizeSpinBox->value());
    mSettings->setValue("General/DarkMode", mThemeCombo->currentIndex() == 1);

    // 렌더링 탭
    mSettings->setValue("Rendering/BackgroundColor", mBackgroundColor);
    mSettings->setValue("Rendering/LightingEnabled", mLightingCheckBox->isChecked());
    mSettings->setValue("Rendering/LightingIntensity", mLightingIntensitySlider->value());
    mSettings->setValue("Rendering/GridEnabled", mGridCheckBox->isChecked());
    mSettings->setValue("Rendering/GridSpacing", mGridSpacingSpinBox->value());

    // 단위 탭
    mSettings->setValue("Units/LengthUnit", mLengthUnitCombo->currentData().toString());
    mSettings->setValue("Units/AngleUnit", mAngleUnitCombo->currentData().toString());
    mSettings->setValue("Units/DecimalPlaces", mDecimalPlacesSpinBox->value());

    // 고급 탭
    mSettings->setValue("Advanced/CacheSize", mCacheSizeSpinBox->value());
    mSettings->setValue("Advanced/LogLevel", mLogLevelCombo->currentData().toString());
    mSettings->setValue("Advanced/GpuAcceleration", mGpuAccelerationCheckBox->isChecked());
    mSettings->setValue("Advanced/AutoSave", mAutoSaveCheckBox->isChecked());
    mSettings->setValue("Advanced/AutoSaveInterval", mAutoSaveIntervalSpinBox->value());

    mSettings->sync();
}

void DongArchPreferencesDialog::applyDefaults()
{
    // 일반 탭 기본값
    mLanguageCombo->setCurrentIndex(0); // 한국어
    mFontSizeSpinBox->setValue(10);
    mThemeCombo->setCurrentIndex(0); // 밝은 모드

    // 렌더링 탭 기본값
    mBackgroundColor = Qt::white;
    updateBackgroundColorButton();
    mLightingCheckBox->setChecked(true);
    mLightingIntensitySlider->setValue(80);
    mGridCheckBox->setChecked(true);
    mGridSpacingSpinBox->setValue(10);

    // 단위 탭 기본값
    mLengthUnitCombo->setCurrentIndex(0); // mm
    mAngleUnitCombo->setCurrentIndex(0); // degree
    mDecimalPlacesSpinBox->setValue(2);

    // 고급 탭 기본값
    mCacheSizeSpinBox->setValue(512);
    mLogLevelCombo->setCurrentIndex(1); // Info
    mGpuAccelerationCheckBox->setChecked(true);
    mAutoSaveCheckBox->setChecked(false);
    mAutoSaveIntervalSpinBox->setValue(5);
}

void DongArchPreferencesDialog::applyChanges()
{
    // 테마 변경 적용 (즉시)
    bool darkMode = (mThemeCombo->currentIndex() == 1);
    QGMDarkModeManager::instance()->setDarkMode(darkMode);

    // 폰트 크기 변경 적용
    QFont font = qApp->font();
    font.setPointSize(mFontSizeSpinBox->value());
    qApp->setFont(font);

    // TODO: 다른 설정들도 메인 윈도우에 시그널로 전달하여 적용
    // 예: emit settingsChanged();
}

void DongArchPreferencesDialog::updateBackgroundColorButton()
{
    // 버튼 색상 미리보기
    QString colorStyle = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    border: 2px solid %2;"
        "    padding: 4px;"
        "}"
    ).arg(mBackgroundColor.name())
     .arg(DongArchColors::MEDIUM_GRAY.name());

    mBackgroundColorButton->setStyleSheet(colorStyle);
    mBackgroundColorButton->setText(mBackgroundColor.name());
}

// ============================================================================
// Slots
// ============================================================================

void DongArchPreferencesDialog::onApply()
{
    saveSettings();
    applyChanges();

    QMessageBox::information(this, tr("환경설정"), tr("설정이 적용되었습니다."));
}

void DongArchPreferencesDialog::onAccept()
{
    saveSettings();
    applyChanges();
    accept();
}

void DongArchPreferencesDialog::onRestoreDefaults()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("기본값 복원"),
        tr("모든 설정을 기본값으로 복원하시겠습니까?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        applyDefaults();
        QMessageBox::information(this, tr("환경설정"), tr("기본값으로 복원되었습니다."));
    }
}

void DongArchPreferencesDialog::onSelectBackgroundColor()
{
    QColor newColor = QColorDialog::getColor(mBackgroundColor, this, tr("배경색 선택"));

    if (newColor.isValid()) {
        mBackgroundColor = newColor;
        updateBackgroundColorButton();
    }
}

void DongArchPreferencesDialog::onThemeChanged(int index)
{
    // 테마 미리보기 (실제 적용은 적용 버튼 클릭 시)
    Q_UNUSED(index);
    // 미리보기 구현 가능 (선택사항)
}
