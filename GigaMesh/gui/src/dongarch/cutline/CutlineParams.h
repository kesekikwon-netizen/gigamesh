/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * This file is part of DongArch3D.
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_CUTLINE_PARAMS_H
#define DONGARCH_CUTLINE_PARAMS_H

#include <GigaMesh/mesh/plane.h>
#include <GigaMesh/mesh/vector3d.h>
#include <QColor>
#include <QDateTime>
#include <QString>
#include <QJsonObject>
#include <vector>
#include <cstdint>

//! \file CutlineParams.h
//! \brief Cutline 파라미터 및 슬롯 구조체
//!
//! Arch3D Liner 매뉴얼 단면(Cutline) 요구사항 구현
//! - 실시간 미리보기 파라미터
//! - 슬롯 저장/불러오기
//! - 세션 복구 지원

namespace DongArch {
namespace Cutline {

/**
 * Arch3D-Liner-User-Guide-ver.2023.07.01.01.pdf 단면(Cutline) 요구사항 체크리스트
 *
 * [✓] 1. 실시간 미리보기 (카메라/평면 변경 시 즉시 갱신)
 * [✓] 2. Top/Front/Right 평면 모드
 * [✓] 3. 평면 오프셋 조절 (슬라이더/마우스 휠/숫자 입력)
 * [✓] 4. Douglas-Peucker 간략화 (ε 파라미터)
 * [✓] 5. Catmull-Rom Spline 스무딩
 * [✓] 6. 라인 스타일 (두께/색상/투명도)
 * [✓] 7. 슬롯 저장/불러오기 (최소 5개)
 * [✓] 8. 세션 복구 (앱 재시작 후 복원)
 * [✓] 9. SVG 내보내기 (1:1 스케일, 레이어 분리)
 * [✓] 10. 60fps 목표 성능
 * [✓] 11. 비동기 처리 (워커 스레드)
 * [✓] 12. Graceful degradation (실패 시 폴백)
 */

//! 평면 모드
enum class PlaneMode {
    TOP,    //!< XY 평면 (Z축 고정) - 평면도
    FRONT,  //!< XZ 평면 (Y축 고정) - 정면도
    RIGHT,  //!< YZ 평면 (X축 고정) - 측면도
    CUSTOM  //!< 사용자 정의 평면
};

//! Cutline 계산 파라미터
struct CutlineParams {
    // === 평면 설정 ===
    PlaneMode mode = PlaneMode::TOP;     //!< 평면 모드
    Plane plane;                         //!< Custom 모드에서 사용하는 평면
    float offset = 0.0f;                 //!< 평면 오프셋 (d) - 메시 중심 기준 정규화 [0, 1]

    // === 알고리즘 파라미터 ===
    float epsilon = 0.01f;               //!< Douglas-Peucker 허용 오차 (0.001 ~ 0.1)
    float splineTension = 0.5f;          //!< Catmull-Rom 스무딩 강도 (0.0 ~ 1.0)

    // === 스타일 ===
    QColor color = QColor(255, 0, 0);    //!< 라인 색상 (기본: 빨강)
    float width = 2.0f;                  //!< 라인 두께 (픽셀)
    float opacity = 0.8f;                //!< 불투명도 (0.0 ~ 1.0)

    // === 스냅 옵션 ===
    bool snapBoundingBox = false;        //!< 바운딩 박스 분할 기준 스냅
    bool snapFeature = false;            //!< 피처 기반 스냅
    int bbDivisions = 10;                //!< 바운딩 박스 분할 수 (10 = 1/10, 20 = 1/20)

    //! 평면 모드 이름 반환
    QString modeName() const {
        switch (mode) {
            case PlaneMode::TOP:    return QStringLiteral("Top (평면도)");
            case PlaneMode::FRONT:  return QStringLiteral("Front (정면도)");
            case PlaneMode::RIGHT:  return QStringLiteral("Right (측면도)");
            case PlaneMode::CUSTOM: return QStringLiteral("Custom (사용자 정의)");
            default:                return QStringLiteral("Unknown");
        }
    }
};

//! Cutline 슬롯 (저장/불러오기)
struct CutlineSlot {
    int id = -1;                         //!< 슬롯 ID (1-5)
    QString name;                        //!< 사용자 지정 이름
    QString memo;                        //!< 메모
    QDateTime timestamp;                 //!< 저장 시각
    CutlineParams params;                //!< 파라미터
    std::vector<Vector3D> polyline;      //!< 계산된 폴리라인

    //! 유효한 슬롯인지 확인
    bool isValid() const {
        return id >= 1 && id <= 5 && !polyline.empty();
    }

    //! 슬롯 정보 문자열
    QString infoString() const {
        return QString("[%1] %2 - %3 points (%4)")
            .arg(id)
            .arg(name.isEmpty() ? "Unnamed" : name)
            .arg(polyline.size())
            .arg(timestamp.toString("yyyy-MM-dd HH:mm:ss"));
    }

    //! JSON 직렬화
    QJsonObject toJson() const;

    //! JSON 역직렬화
    static CutlineSlot fromJson(const QJsonObject& json);
};

//! Request ID 타입 (최신성 보장)
using RequestId = uint64_t;

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_PARAMS_H
