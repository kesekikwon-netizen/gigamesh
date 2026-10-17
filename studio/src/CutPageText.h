// DongArch3D Studio — ② 막으로 자르기 페이지의 화면 문자열·표기 함수 (Qt Core 만 — 헤드리스 시험 가능).
// 원문: 화면 디자인 v2 phase=cut (`rtabs`·`ribbonCut`·`ctxBand`·`statusBar`·`rightCut`·`midCut`),
// 원문과 다르게 정한 곳은 JOB-006 명세 §3.1·§8-2. ② 페이지의 새 화면 문자열은 이 헤더 한 곳에 둔다(툴팁 제외).
#pragma once
#include "ArtifactDoc.h"

#include <QLocale>
#include <QString>
#include <cmath>
#include <cstddef>

namespace studio::cutpage {

inline const QString kTabName = QStringLiteral("② 막으로 자르기");
inline const QString kLockTip = QStringLiteral("🔒 바닥 확정 후 사용 — ① 바닥 정하기에서 「이 자세로 확정」 (Enter)");
inline const QString kCtxBadge = QStringLiteral("✂ 막으로 자르기");
inline const QString kCtxHint = QStringLiteral("3D 를 좌우로 끌어 회전판을 돌리면 막이 지나는 자리가 바뀌고 오른쪽 단면이 바로 바뀝니다");
inline const QString kStatus = QStringLiteral("⑥ 단면 보기 — 오른쪽 단면이 뜻한 자리인지 보고 다음: 「도면」 내보내기 (Ctrl+P)");
inline const QString k3dTitle = QStringLiteral("3D — 막으로 자르기");
inline const QString k3dTip = QStringLiteral("왼쪽 끌기 = 회전판 돌리기 · 오른쪽 끌기 = 보는 방향만");
inline const QString kGrpMove = QStringLiteral("막 움직이기 (회전판)");
inline const QString kGrpSide = QStringLiteral("단면 위치");
inline const QString kTileX = QStringLiteral("X 막 옆에서");  // 원문 두 줄 → 한 줄 (리본 높이 불변)
inline const QString kTileY = QStringLiteral("Y 막 정면에서");
inline const QString kTileZ = QStringLiteral("Z 막 높이로");
inline const QString kPinned = QStringLiteral("☑ 중심선에 고정 (돌리기만)");
inline const QString kAngle = QStringLiteral("각도");
inline const QString kStep1 = QStringLiteral("1° ·");
inline const QString kStep15 = QStringLiteral("15°");
inline const QString kLive = QStringLiteral("끄는 동안 실시간");
inline const QString kStandard = QStringLiteral("국내 보고서 표준");
inline const QString kCheckUser = QStringLiteral("사용자 도판으로 확인");

// 0.1 단위 정수(≥ 0) → "12" · "37.5"
inline QString tenthsText(long long t) {
    return t % 10 ? QStringLiteral("%1.%2").arg(t / 10).arg(t % 10) : QString::number(t / 10);
}

// 정면각 표기: 0.1° 반올림, 정수면 정수, 음수는 −(U+2212). 예: 12°, 37.5°, −20°
inline QString angleText(double deg) {
    const long long t = std::llround(deg * 10.0);
    return (t < 0 ? QStringLiteral("−") : QString()) + tenthsText(t < 0 ? -t : t) + QStringLiteral("°");
}

inline QString plateTitle(QChar letter, double deg) {
    return QStringLiteral("%1–%1′ 실시간 단면 — %2").arg(QString(letter), angleText(deg));
}

inline QString plateSub(SectionSide s) {
    return QStringLiteral("— 3D에서 막을 움직이는 동안 바로 다시 그림 · ") +
           (s == SectionSide::Right ? QStringLiteral("외면 왼쪽 · 단면 오른쪽") : QStringLiteral("단면 왼쪽 · 외면 오른쪽"));
}

// 지금 각이 그 막 방향에서 1° 안인지 (원문: m = mod360(ang) % 180, Y: m ≤ 1 ‖ m ≥ 179, X: |m − 90| ≤ 1)
inline bool atAxis(double deg, CutAxis a) {
    const double h = std::fmod(std::fmod(std::fmod(deg, 360.0) + 360.0, 360.0), 180.0);
    return a == CutAxis::Y ? (h <= 1.0 || h >= 179.0) : std::fabs(h - 90.0) <= 1.0;
}

inline QString axisGroupName(double deg) {
    if (atAxis(deg, CutAxis::Y) || atAxis(deg, CutAxis::X)) return QStringLiteral("막 방향 (누르면 바로 맞춤)");
    return QStringLiteral("막 방향 · 지금 %1 (누르면 맞춤)").arg(angleText(deg));
}

// 면 수: 1만 이상은 0.1만 단위(2.6만 면), 그 아래는 천 단위 쉼표(9,800 면)
inline QString faceText(size_t n) {
    if (n < 10000) return QLocale(QLocale::English).toString(qulonglong(n)) + QStringLiteral(" 면");
    return tenthsText(std::llround(double(n) / 1000.0)) + QStringLiteral("만 면");
}

// ctx 오른쪽: 놓은 상태 / 끄는 중(미리보기 — 음영·외곽·치수·두께는 아직 옛 각도 값)
inline QString ctxRight(double deg, size_t lodFaces, size_t fullFaces, bool previewing) {
    if (previewing)
        return QStringLiteral("막 %1 · 끄는 중 %2 미리보기 — 음영·외곽·치수·두께는 놓은 뒤").arg(angleText(deg), faceText(lodFaces));
    return QStringLiteral("막 %1 · 끄는 동안 %2 미리보기 · 놓으면 원본 %3 다시 계산")
        .arg(angleText(deg), faceText(lodFaces), faceText(fullFaces));
}

}  // namespace studio::cutpage
