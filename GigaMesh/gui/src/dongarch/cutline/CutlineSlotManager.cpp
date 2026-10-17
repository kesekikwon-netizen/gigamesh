/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "CutlineSlotManager.h"
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

namespace DongArch {
namespace Cutline {

// ============================================================================
// CutlineSlot 구현
// ============================================================================

QJsonObject CutlineSlot::toJson() const {
    QJsonObject json;

    // 메타데이터
    json["id"] = id;
    json["name"] = name;
    json["memo"] = memo;
    json["timestamp"] = timestamp.toString(Qt::ISODate);

    // 파라미터
    QJsonObject paramsJson;
    paramsJson["mode"] = static_cast<int>(params.mode);
    paramsJson["offset"] = static_cast<double>(params.offset);
    paramsJson["epsilon"] = static_cast<double>(params.epsilon);
    paramsJson["splineTension"] = static_cast<double>(params.splineTension);
    paramsJson["color"] = params.color.name();
    paramsJson["width"] = static_cast<double>(params.width);
    paramsJson["opacity"] = static_cast<double>(params.opacity);
    paramsJson["snapBoundingBox"] = params.snapBoundingBox;
    paramsJson["snapFeature"] = params.snapFeature;
    paramsJson["bbDivisions"] = params.bbDivisions;
    json["params"] = paramsJson;

    // Polyline (3D 좌표 배열)
    QJsonArray polylineJson;
    for (const auto& point : polyline) {
        QJsonObject pointJson;
        pointJson["x"] = point.getX();
        pointJson["y"] = point.getY();
        pointJson["z"] = point.getZ();
        polylineJson.append(pointJson);
    }
    json["polyline"] = polylineJson;
    json["polylineSize"] = static_cast<int>(polyline.size());

    return json;
}

CutlineSlot CutlineSlot::fromJson(const QJsonObject& json) {
    CutlineSlot slot;

    // 메타데이터
    slot.id = json["id"].toInt();
    slot.name = json["name"].toString();
    slot.memo = json["memo"].toString();
    slot.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);

    // 파라미터
    QJsonObject paramsJson = json["params"].toObject();
    slot.params.mode = static_cast<PlaneMode>(paramsJson["mode"].toInt());
    slot.params.offset = static_cast<float>(paramsJson["offset"].toDouble());
    slot.params.epsilon = static_cast<float>(paramsJson["epsilon"].toDouble());
    slot.params.splineTension = static_cast<float>(paramsJson["splineTension"].toDouble());
    slot.params.color = QColor(paramsJson["color"].toString());
    slot.params.width = static_cast<float>(paramsJson["width"].toDouble());
    slot.params.opacity = static_cast<float>(paramsJson["opacity"].toDouble());
    slot.params.snapBoundingBox = paramsJson["snapBoundingBox"].toBool();
    slot.params.snapFeature = paramsJson["snapFeature"].toBool();
    slot.params.bbDivisions = paramsJson["bbDivisions"].toInt();

    // Polyline
    QJsonArray polylineJson = json["polyline"].toArray();
    slot.polyline.reserve(polylineJson.size());
    for (const auto& pointValue : polylineJson) {
        QJsonObject pointJson = pointValue.toObject();
        double x = pointJson["x"].toDouble();
        double y = pointJson["y"].toDouble();
        double z = pointJson["z"].toDouble();
        slot.polyline.emplace_back(x, y, z);
    }

    return slot;
}

// ============================================================================
// CutlineSlotManager 구현
// ============================================================================

CutlineSlotManager::CutlineSlotManager(QObject* parent)
    : QObject(parent)
{
    // 슬롯 초기화
    for (int i = 0; i < 5; ++i) {
        mSlots[i].id = i + 1;
        mSlots[i].name = QString("Slot %1").arg(i + 1);
    }

    // 자동 로드
    QString filePath = getDefaultSessionFilePath();
    if (QFile::exists(filePath)) {
        qDebug() << "[CutlineSlotManager] Loading session from:" << filePath;
        loadFromFile(filePath);
    } else {
        qDebug() << "[CutlineSlotManager] No session file found, starting fresh";
    }
}

CutlineSlotManager::~CutlineSlotManager() {
    // 자동 저장
    QString filePath = getDefaultSessionFilePath();
    qDebug() << "[CutlineSlotManager] Auto-saving session to:" << filePath;
    saveToFile(filePath);
}

bool CutlineSlotManager::saveSlot(
    int slotId,
    const CutlineParams& params,
    const std::vector<Vector3D>& polyline) {

    if (!isValidSlotId(slotId)) {
        qWarning() << "[CutlineSlotManager] Invalid slot ID:" << slotId;
        return false;
    }

    int index = slotIdToIndex(slotId);
    CutlineSlot& slot = mSlots[index];

    // 슬롯 업데이트
    slot.id = slotId;
    slot.timestamp = QDateTime::currentDateTime();
    slot.params = params;
    slot.polyline = polyline;

    qDebug() << "[CutlineSlotManager] Saved slot" << slotId
             << "with" << polyline.size() << "points";

    emit slotSaved(slotId);
    return true;
}

CutlineSlot CutlineSlotManager::loadSlot(int slotId) const {
    if (!isValidSlotId(slotId)) {
        qWarning() << "[CutlineSlotManager] Invalid slot ID:" << slotId;
        return CutlineSlot();
    }

    int index = slotIdToIndex(slotId);
    const CutlineSlot& slot = mSlots[index];

    qDebug() << "[CutlineSlotManager] Loaded slot" << slotId
             << "with" << slot.polyline.size() << "points";

    return slot;
}

void CutlineSlotManager::clearSlot(int slotId) {
    if (!isValidSlotId(slotId)) {
        qWarning() << "[CutlineSlotManager] Invalid slot ID:" << slotId;
        return;
    }

    int index = slotIdToIndex(slotId);
    CutlineSlot& slot = mSlots[index];

    // 슬롯 초기화
    slot.id = slotId;
    slot.name = QString("Slot %1").arg(slotId);
    slot.memo.clear();
    slot.timestamp = QDateTime();
    slot.params = CutlineParams();
    slot.polyline.clear();

    qDebug() << "[CutlineSlotManager] Cleared slot" << slotId;

    emit slotCleared(slotId);
}

bool CutlineSlotManager::isSlotEmpty(int slotId) const {
    if (!isValidSlotId(slotId)) {
        return true;
    }

    int index = slotIdToIndex(slotId);
    return mSlots[index].polyline.empty();
}

bool CutlineSlotManager::saveToFile(const QString& filePath) {
    QString path = filePath.isEmpty() ? getDefaultSessionFilePath() : filePath;

    // 디렉토리 생성
    QFileInfo fileInfo(path);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qCritical() << "[CutlineSlotManager] Failed to create directory:" << dir.path();
            return false;
        }
    }

    // JSON 생성
    QJsonObject rootJson;
    rootJson["version"] = "1.0";
    rootJson["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonArray slotsJson;
    for (const auto& slot : mSlots) {
        if (!slot.polyline.empty()) {  // 비어있지 않은 슬롯만 저장
            slotsJson.append(slot.toJson());
        }
    }
    rootJson["slots"] = slotsJson;
    rootJson["slotCount"] = slotsJson.size();

    // 파일 쓰기
    QJsonDocument doc(rootJson);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "[CutlineSlotManager] Failed to open file for writing:" << path;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    qInfo() << "[CutlineSlotManager] Saved" << slotsJson.size() << "slots to:" << path;
    return true;
}

bool CutlineSlotManager::loadFromFile(const QString& filePath) {
    QString path = filePath.isEmpty() ? getDefaultSessionFilePath() : filePath;

    // 파일 읽기
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[CutlineSlotManager] Failed to open file for reading:" << path;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    // JSON 파싱
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qCritical() << "[CutlineSlotManager] Invalid JSON format";
        return false;
    }

    QJsonObject rootJson = doc.object();
    QString version = rootJson["version"].toString();
    qDebug() << "[CutlineSlotManager] Loading session version:" << version;

    // 슬롯 로드
    QJsonArray slotsJson = rootJson["slots"].toArray();
    int loadedCount = 0;

    for (const auto& slotValue : slotsJson) {
        QJsonObject slotJson = slotValue.toObject();
        CutlineSlot slot = CutlineSlot::fromJson(slotJson);

        if (slot.isValid()) {
            int index = slotIdToIndex(slot.id);
            mSlots[index] = slot;
            loadedCount++;

            emit slotLoaded(slot.id, slot.params, slot.polyline);
        }
    }

    qInfo() << "[CutlineSlotManager] Loaded" << loadedCount << "slots from:" << path;
    return true;
}

QString CutlineSlotManager::getDefaultSessionFilePath() {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) {
        dataDir = QDir::homePath() + "/.dongarch3d";
    }
    return dataDir + "/cutline_sessions.json";
}

} // namespace Cutline
} // namespace DongArch
