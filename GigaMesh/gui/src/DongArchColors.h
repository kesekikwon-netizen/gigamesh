/**
 * DongArch3D Color Palette
 *
 * 동국문화재연구원 전용 실측 프로그램 색상 테마
 * 고고학 발굴 현장과 유물의 자연스러운 색상을 반영
 *
 * Design Philosophy: "Stratigraphic Precision"
 * - 토층(Soil Layer): 발굴 현장의 흙과 층위
 * - 석기(Lithic): 돌과 암석의 차분한 색조
 * - 토기(Ceramic): 산화철과 점토의 따뜻한 색감
 *
 * @file DongArchColors.h
 * @author DongArch3D Team
 * @date 2025-11-07
 * @version 1.0.0
 */

#ifndef DONGARCH_COLORS_H
#define DONGARCH_COLORS_H

#include <QColor>
#include <QVector>
#include <QString>

namespace DongArchColors {

// ============================================================================
// PRIMARY COLORS - 주요 색상 (고고학 유물 및 발굴 현장)
// ============================================================================

/**
 * @brief 토층 브라운 - 발굴 현장의 흙, 교란층
 *
 * 용도:
 * - 메인 액센트 색상
 * - 단면 도구 강조
 * - 선택된 항목 배경
 */
const QColor SOIL_LAYER_BROWN(139, 90, 43);        // #8B5A2B

/**
 * @brief 석기 그레이 - 돌, 암석, 석기 유물
 *
 * 용도:
 * - 석기 실측 모드 테두리
 * - 중립적인 UI 요소
 * - 측정 도구 색상
 */
const QColor LITHIC_GRAY(100, 100, 100);           // #646464

/**
 * @brief 토기 오커 - 점토, 산화철, 토기 유물
 *
 * 용도:
 * - 토기 실측 모드 강조
 * - 활성 버튼 색상
 * - 경고 및 중요 정보
 */
const QColor CERAMIC_OCHRE(204, 119, 34);          // #CC7722

/**
 * @brief 표피(Cortex) 화이트 - 석기 표피, 밝은 층위
 *
 * 용도:
 * - Cortex 영역 하이라이트
 * - 배경 색상 (밝은 모드)
 * - 텍스트 배경
 */
const QColor CORTEX_WHITE(240, 240, 230);          // #F0F0E6

/**
 * @brief 능선(Ridge) 다크 - 강조선, 윤곽선
 *
 * 용도:
 * - 능선 강조 렌더링
 * - 단면 교선
 * - 중요 선 요소
 */
const QColor RIDGE_DARK(40, 40, 40);               // #282828

/**
 * @brief 단면선 오렌지 - 단면 평면 및 교선
 *
 * 용도:
 * - 단면 생성 도구
 * - 평면 시각화
 * - 프로파일 라인
 */
const QColor SECTION_ORANGE(255, 140, 0);          // #FF8C00

// ============================================================================
// SECONDARY COLORS - 보조 색상 (층위학 및 분석)
// ============================================================================

/**
 * @brief 밝은 오커 - 표층, 밝은 토양
 */
const QColor LIGHT_OCHRE(232, 203, 168);           // #E8CBA8

/**
 * @brief 짙은 브라운 - 깊은 층위, 텍스트
 */
const QColor DARK_BROWN(92, 61, 46);               // #5C3D2E

/**
 * @brief 중간 그레이 - UI 구분선, 비활성 요소
 */
const QColor MEDIUM_GRAY(128, 128, 128);           // #808080

/**
 * @brief 밝은 그레이 - 배경, 패널
 */
const QColor LIGHT_GRAY(220, 220, 220);            // #DCDCDC

// ============================================================================
// FUNCTIONAL COLORS - 기능별 색상
// ============================================================================

/**
 * @brief 선택 색상 - 선택된 객체, 포커스
 */
const QColor SELECTION_BLUE(70, 130, 180);         // #4682B4

/**
 * @brief 호버 색상 - 마우스 오버 상태
 */
const QColor HOVER_HIGHLIGHT(255, 215, 0, 100);    // #FFD700 (투명도 100/255)

/**
 * @brief 오류 색상 - 에러, 경고
 */
const QColor ERROR_RED(220, 20, 60);               // #DC143C

/**
 * @brief 성공 색상 - 완료, 확인
 */
const QColor SUCCESS_GREEN(34, 139, 34);           // #228B22

/**
 * @brief 정보 색상 - 안내, 힌트
 */
const QColor INFO_CYAN(0, 191, 255);               // #00BFFF

// ============================================================================
// MEASUREMENT COLORS - 측정 도구 색상
// ============================================================================

/**
 * @brief 거리 측정 - 직선 거리 도구
 */
const QColor MEASURE_DISTANCE(0, 128, 255);        // #0080FF

/**
 * @brief 각도 측정 - 각도 측정 도구
 */
const QColor MEASURE_ANGLE(255, 128, 0);           // #FF8000

/**
 * @brief 면적 측정 - 면적 계산 영역
 */
const QColor MEASURE_AREA(0, 255, 128, 128);       // #00FF80 (반투명)

/**
 * @brief 그리드 색상 - 측량 그리드
 */
const QColor GRID_LINE(160, 160, 160);             // #A0A0A0

/**
 * @brief 스케일 바 - 축척 표시
 */
const QColor SCALE_BAR(0, 0, 0);                   // #000000 (검정)

// ============================================================================
// ARCHAEOLOGICAL MODE COLORS - 유물별 실측 모드 색상
// ============================================================================

/**
 * @brief 석기 모드 - 석기 실측 도구 활성화
 */
const QColor LITHIC_MODE_ACTIVE(100, 100, 100);    // LITHIC_GRAY

/**
 * @brief 토기 모드 - 토기 실측 도구 활성화
 */
const QColor CERAMIC_MODE_ACTIVE(204, 119, 34);    // CERAMIC_OCHRE

/**
 * @brief 금속기 모드 - 금속기 실측 도구 활성화
 */
const QColor METAL_MODE_ACTIVE(192, 192, 192);     // #C0C0C0 (실버)

/**
 * @brief 일반 모드 - 기본 실측 도구
 */
const QColor GENERAL_MODE_ACTIVE(139, 90, 43);     // SOIL_LAYER_BROWN

// ============================================================================
// THEME PRESETS - 테마 프리셋
// ============================================================================

/**
 * @brief Light Theme - 밝은 테마 (기본)
 */
struct LightTheme {
    inline static const QColor Background = CORTEX_WHITE;
    inline static const QColor Foreground = RIDGE_DARK;
    inline static const QColor Panel = LIGHT_GRAY;
    inline static const QColor Border = MEDIUM_GRAY;
    inline static const QColor Text = DARK_BROWN;
};

/**
 * @brief Dark Theme - 어두운 테마
 */
struct DarkTheme {
    inline static const QColor Background = QColor(45, 45, 48);      // #2D2D30
    inline static const QColor Foreground = QColor(220, 220, 220);   // #DCDCDC
    inline static const QColor Panel = QColor(37, 37, 38);           // #252526
    inline static const QColor Border = QColor(63, 63, 70);          // #3F3F46
    inline static const QColor Text = QColor(240, 240, 240);         // #F0F0F0
};

// ============================================================================
// UTILITY FUNCTIONS - 유틸리티 함수
// ============================================================================

/**
 * @brief 색상을 HTML 색상 코드로 변환
 * @param color QColor 객체
 * @return QString HTML 색상 코드 (예: "#8B5A2B")
 */
inline QString toHtmlColor(const QColor& color) {
    return QString("#%1%2%3")
        .arg(color.red(), 2, 16, QChar('0'))
        .arg(color.green(), 2, 16, QChar('0'))
        .arg(color.blue(), 2, 16, QChar('0'));
}

/**
 * @brief 색상의 밝기를 조절
 * @param color 원본 색상
 * @param factor 밝기 계수 (1.0 = 원본, >1.0 = 밝게, <1.0 = 어둡게)
 * @return QColor 조절된 색상
 */
inline QColor adjustBrightness(const QColor& color, float factor) {
    return QColor(
        qMin(255, int(color.red() * factor)),
        qMin(255, int(color.green() * factor)),
        qMin(255, int(color.blue() * factor)),
        color.alpha()
    );
}

/**
 * @brief 두 색상을 혼합
 * @param color1 첫 번째 색상
 * @param color2 두 번째 색상
 * @param ratio 혼합 비율 (0.0 = color1, 1.0 = color2)
 * @return QColor 혼합된 색상
 */
inline QColor blendColors(const QColor& color1, const QColor& color2, float ratio) {
    ratio = qBound(0.0f, ratio, 1.0f);
    return QColor(
        int(color1.red() * (1.0f - ratio) + color2.red() * ratio),
        int(color1.green() * (1.0f - ratio) + color2.green() * ratio),
        int(color1.blue() * (1.0f - ratio) + color2.blue() * ratio),
        int(color1.alpha() * (1.0f - ratio) + color2.alpha() * ratio)
    );
}

/**
 * @brief 색상 팔레트 목록 생성
 * @return QVector<QColor> 모든 주요 색상의 벡터
 */
inline QVector<QColor> getPrimaryPalette() {
    return {
        SOIL_LAYER_BROWN,
        LITHIC_GRAY,
        CERAMIC_OCHRE,
        CORTEX_WHITE,
        RIDGE_DARK,
        SECTION_ORANGE
    };
}

/**
 * @brief 색상 이름 가져오기
 * @param color QColor 객체
 * @return QString 색상 이름 (한글)
 */
inline QString getColorName(const QColor& color) {
    if (color == SOIL_LAYER_BROWN) return "토층 브라운";
    if (color == LITHIC_GRAY) return "석기 그레이";
    if (color == CERAMIC_OCHRE) return "토기 오커";
    if (color == CORTEX_WHITE) return "표피 화이트";
    if (color == RIDGE_DARK) return "능선 다크";
    if (color == SECTION_ORANGE) return "단면 오렌지";
    return "알 수 없음";
}

} // namespace DongArchColors

#endif // DONGARCH_COLORS_H
