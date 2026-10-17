/**
 * DongArch3D Preferences Dialog
 *
 * 환경설정 다이얼로그 - 4개 탭으로 구성된 통합 설정 창
 *
 * @file DongArchPreferencesDialog.h
 * @author DongArch3D Team
 * @date 2025-11-08
 * @version 1.0.0
 */

#ifndef DONGARCH_PREFERENCES_DIALOG_H
#define DONGARCH_PREFERENCES_DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QSettings>
#include <QColor>
#include <QColorDialog>

/**
 * @brief DongArch3D 환경설정 다이얼로그
 *
 * 4개의 탭으로 구성:
 * - 일반: 언어, 폰트, 테마
 * - 렌더링: 배경색, 조명, 그리드
 * - 단위: mm/cm/m, 각도 단위
 * - 고급: 캐시, 로그, GPU 설정
 */
class DongArchPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 생성자
     * @param parent 부모 위젯
     */
    explicit DongArchPreferencesDialog(QWidget *parent = nullptr);

    /**
     * @brief 소멸자
     */
    ~DongArchPreferencesDialog();

private slots:
    /**
     * @brief 적용 버튼 클릭
     */
    void onApply();

    /**
     * @brief 확인 버튼 클릭 (적용 후 닫기)
     */
    void onAccept();

    /**
     * @brief 기본값 복원 버튼 클릭
     */
    void onRestoreDefaults();

    /**
     * @brief 배경색 선택 버튼 클릭
     */
    void onSelectBackgroundColor();

    /**
     * @brief 테마 변경
     * @param index 콤보박스 인덱스 (0=밝은 모드, 1=어두운 모드)
     */
    void onThemeChanged(int index);

private:
    /**
     * @brief UI 초기화
     */
    void initUI();

    /**
     * @brief 일반 탭 생성
     * @return 일반 탭 위젯
     */
    QWidget* createGeneralTab();

    /**
     * @brief 렌더링 탭 생성
     * @return 렌더링 탭 위젯
     */
    QWidget* createRenderingTab();

    /**
     * @brief 단위 탭 생성
     * @return 단위 탭 위젯
     */
    QWidget* createUnitsTab();

    /**
     * @brief 고급 탭 생성
     * @return 고급 탭 위젯
     */
    QWidget* createAdvancedTab();

    /**
     * @brief QSettings에서 설정 로드
     */
    void loadSettings();

    /**
     * @brief QSettings에 설정 저장
     */
    void saveSettings();

    /**
     * @brief 기본값으로 UI 초기화
     */
    void applyDefaults();

    /**
     * @brief UI 설정을 프로그램에 적용
     */
    void applyChanges();

    /**
     * @brief 배경색 버튼 UI 업데이트
     */
    void updateBackgroundColorButton();

    // UI Components - 일반 탭
    QComboBox* mLanguageCombo;
    QSpinBox* mFontSizeSpinBox;
    QComboBox* mThemeCombo;

    // UI Components - 렌더링 탭
    QPushButton* mBackgroundColorButton;
    QColor mBackgroundColor;
    QCheckBox* mLightingCheckBox;
    QSlider* mLightingIntensitySlider;
    QLabel* mLightingIntensityLabel;
    QCheckBox* mGridCheckBox;
    QSpinBox* mGridSpacingSpinBox;

    // UI Components - 단위 탭
    QComboBox* mLengthUnitCombo;
    QComboBox* mAngleUnitCombo;
    QSpinBox* mDecimalPlacesSpinBox;

    // UI Components - 고급 탭
    QSpinBox* mCacheSizeSpinBox;
    QComboBox* mLogLevelCombo;
    QCheckBox* mGpuAccelerationCheckBox;
    QCheckBox* mAutoSaveCheckBox;
    QSpinBox* mAutoSaveIntervalSpinBox;

    // Main layout components
    QTabWidget* mTabWidget;
    QDialogButtonBox* mButtonBox;
    QPushButton* mApplyButton;
    QPushButton* mRestoreDefaultsButton;

    // Settings storage
    QSettings* mSettings;
};

#endif // DONGARCH_PREFERENCES_DIALOG_H
