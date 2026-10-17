/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#ifndef DONGARCH_CUTLINE_SLOT_MANAGER_H
#define DONGARCH_CUTLINE_SLOT_MANAGER_H

#include "CutlineParams.h"
#include <GigaMesh/mesh/vector3d.h>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QString>
#include <vector>
#include <array>

namespace DongArch {
namespace Cutline {

//! Cutline 슬롯 매니저
//!
//! **기능**:
//! - 5개 슬롯 관리 (CRUD)
//! - JSON 파일로 세션 저장/로드
//! - 자동 백업 (앱 종료 시)
//!
//! **파일 위치**: `~/.dongarch3d/cutline_sessions.json`
class CutlineSlotManager : public QObject {
    Q_OBJECT

public:
    explicit CutlineSlotManager(QObject* parent = nullptr);
    ~CutlineSlotManager() override;

    //! 슬롯 저장
    bool saveSlot(int slotId, const CutlineParams& params, const std::vector<Vector3D>& polyline);

    //! 슬롯 로드
    CutlineSlot loadSlot(int slotId) const;

    //! 슬롯 삭제
    void clearSlot(int slotId);

    //! 모든 슬롯 가져오기
    const std::array<CutlineSlot, 5>& getAllSlots() const { return mSlots; }

    //! 슬롯이 비어있는지 확인
    bool isSlotEmpty(int slotId) const;

    //! 세션 파일 저장
    bool saveToFile(const QString& filePath = QString());

    //! 세션 파일 로드
    bool loadFromFile(const QString& filePath = QString());

    //! 기본 세션 파일 경로
    static QString getDefaultSessionFilePath();

signals:
    //! 슬롯이 저장됨
    void slotSaved(int slotId);

    //! 슬롯이 로드됨
    void slotLoaded(int slotId, const CutlineParams& params, const std::vector<Vector3D>& polyline);

    //! 슬롯이 삭제됨
    void slotCleared(int slotId);

private:
    //! 슬롯 ID 유효성 검사
    bool isValidSlotId(int slotId) const {
        return slotId >= 1 && slotId <= 5;
    }

    //! 인덱스 변환 (1-based → 0-based)
    int slotIdToIndex(int slotId) const {
        return slotId - 1;
    }

    std::array<CutlineSlot, 5> mSlots;   //!< 5개 슬롯
};

} // namespace Cutline
} // namespace DongArch

#endif // DONGARCH_CUTLINE_SLOT_MANAGER_H
