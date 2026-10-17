/**
 * DongArch3D Project File Format Implementation
 *
 * @file DongArchProjectFile.cpp
 * @author DongArch3D Team
 * @date 2025-11-08
 * @version 1.0.0
 */

#include "DongArchProjectFile.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QTemporaryDir>

// NOTE: Qt 5.15.2에서 QZipReader/QZipWriter는 private API
// 안정적인 구현을 위해서는 QuaZip 라이브러리 필요
// 현재는 프로토타입으로 디렉토리 기반 구현
// TODO: QuaZip 통합 후 실제 ZIP 압축 구현

DongArchProjectFile::DongArchProjectFile()
{
}

DongArchProjectFile::~DongArchProjectFile()
{
}

bool DongArchProjectFile::save(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.dir().exists()) {
        mLastError = QString("디렉토리가 존재하지 않습니다: %1").arg(fileInfo.dir().path());
        return false;
    }

    // 임시 디렉토리 생성
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        mLastError = "임시 디렉토리 생성 실패";
        return false;
    }

    QString tempPath = tempDir.path();

    // 디렉토리 구조 생성
    QDir dir(tempPath);
    if (!dir.mkpath("mesh")) {
        mLastError = "mesh 디렉토리 생성 실패";
        return false;
    }
    if (!dir.mkpath("screenshots")) {
        mLastError = "screenshots 디렉토리 생성 실패";
        return false;
    }

    // project.json 저장
    QJsonObject projectJson = createProjectJson();
    QJsonDocument projectDoc(projectJson);
    QFile projectFile(tempPath + "/project.json");
    if (!projectFile.open(QIODevice::WriteOnly)) {
        mLastError = QString("project.json 생성 실패: %1").arg(projectFile.errorString());
        return false;
    }
    projectFile.write(projectDoc.toJson(QJsonDocument::Indented));
    projectFile.close();

    // settings.json 저장
    QJsonObject settingsJson = createSettingsJson();
    QJsonDocument settingsDoc(settingsJson);
    QFile settingsFile(tempPath + "/settings.json");
    if (!settingsFile.open(QIODevice::WriteOnly)) {
        mLastError = QString("settings.json 생성 실패: %1").arg(settingsFile.errorString());
        return false;
    }
    settingsFile.write(settingsDoc.toJson(QJsonDocument::Indented));
    settingsFile.close();

    // 메시 파일 복사 (실제 구현에서는 ZIP에 추가)
    for (const auto& mesh : mMeshes) {
        // TODO: 메시 파일 복사 로직
        qDebug() << "메시 파일:" << mesh.fileName << mesh.displayName;
    }

    // TODO: 실제 ZIP 압축 구현 (QuaZip 사용)
    // 현재는 프로토타입으로 JSON 파일만 저장
    QFile::copy(tempPath + "/project.json", filePath + ".project.json");
    QFile::copy(tempPath + "/settings.json", filePath + ".settings.json");

    qDebug() << "프로젝트 저장 완료 (프로토타입):" << filePath;
    return true;
}

bool DongArchProjectFile::load(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        mLastError = QString("파일이 존재하지 않습니다: %1").arg(filePath);
        return false;
    }

    // TODO: ZIP 압축 해제 구현 (QuaZip 사용)
    // 현재는 프로토타입으로 JSON 파일 직접 로드

    // project.json 로드
    QString projectJsonPath = filePath + ".project.json";
    QFile projectFile(projectJsonPath);
    if (!projectFile.exists()) {
        mLastError = "project.json을 찾을 수 없습니다";
        return false;
    }

    if (!projectFile.open(QIODevice::ReadOnly)) {
        mLastError = QString("project.json 열기 실패: %1").arg(projectFile.errorString());
        return false;
    }

    QJsonDocument projectDoc = QJsonDocument::fromJson(projectFile.readAll());
    projectFile.close();

    if (projectDoc.isNull() || !projectDoc.isObject()) {
        mLastError = "project.json 파싱 실패";
        return false;
    }

    if (!parseProjectJson(projectDoc.object())) {
        return false;
    }

    // settings.json 로드
    QString settingsJsonPath = filePath + ".settings.json";
    QFile settingsFile(settingsJsonPath);
    if (settingsFile.exists() && settingsFile.open(QIODevice::ReadOnly)) {
        QJsonDocument settingsDoc = QJsonDocument::fromJson(settingsFile.readAll());
        settingsFile.close();

        if (!settingsDoc.isNull() && settingsDoc.isObject()) {
            parseSettingsJson(settingsDoc.object());
        }
    }

    qDebug() << "프로젝트 로드 완료 (프로토타입):" << filePath;
    return true;
}

bool DongArchProjectFile::addMesh(const QString& meshFilePath, const QString& displayName, const QString& type)
{
    QFileInfo fileInfo(meshFilePath);
    if (!fileInfo.exists()) {
        mLastError = QString("메시 파일이 존재하지 않습니다: %1").arg(meshFilePath);
        return false;
    }

    MeshInfo meshInfo;
    meshInfo.fileName = fileInfo.fileName();
    meshInfo.displayName = displayName.isEmpty() ? fileInfo.baseName() : displayName;
    meshInfo.type = type.isEmpty() ? "일반" : type;
    meshInfo.vertexCount = 0;  // TODO: 실제 메시에서 읽기
    meshInfo.faceCount = 0;    // TODO: 실제 메시에서 읽기
    meshInfo.visible = true;

    mMeshes.append(meshInfo);
    return true;
}

bool DongArchProjectFile::addScreenshot(const QString& screenshotPath, const QString& name)
{
    QFileInfo fileInfo(screenshotPath);
    if (!fileInfo.exists()) {
        mLastError = QString("스크린샷 파일이 존재하지 않습니다: %1").arg(screenshotPath);
        return false;
    }

    // TODO: 스크린샷을 프로젝트에 추가
    qDebug() << "스크린샷 추가:" << name << screenshotPath;
    return true;
}

QJsonObject DongArchProjectFile::createProjectJson() const
{
    QJsonObject json;

    // 메타데이터
    json["version"] = mMetadata.version;
    json["name"] = mMetadata.name;
    json["description"] = mMetadata.description;
    json["created"] = mMetadata.created.toString(Qt::ISODate);
    json["modified"] = mMetadata.modified.toString(Qt::ISODate);
    json["author"] = mMetadata.author;

    // 메시 목록
    QJsonArray meshArray;
    for (const auto& mesh : mMeshes) {
        QJsonObject meshObj;
        meshObj["fileName"] = mesh.fileName;
        meshObj["displayName"] = mesh.displayName;
        meshObj["type"] = mesh.type;
        meshObj["vertexCount"] = mesh.vertexCount;
        meshObj["faceCount"] = mesh.faceCount;
        meshObj["visible"] = mesh.visible;
        meshArray.append(meshObj);
    }
    json["meshes"] = meshArray;

    return json;
}

bool DongArchProjectFile::parseProjectJson(const QJsonObject& json)
{
    // 메타데이터 파싱
    mMetadata.version = json["version"].toString("1.0.0");
    mMetadata.name = json["name"].toString();
    mMetadata.description = json["description"].toString();
    mMetadata.created = QDateTime::fromString(json["created"].toString(), Qt::ISODate);
    mMetadata.modified = QDateTime::fromString(json["modified"].toString(), Qt::ISODate);
    mMetadata.author = json["author"].toString();

    // 메시 목록 파싱
    mMeshes.clear();
    QJsonArray meshArray = json["meshes"].toArray();
    for (const auto& meshValue : meshArray) {
        if (!meshValue.isObject()) continue;

        QJsonObject meshObj = meshValue.toObject();
        MeshInfo meshInfo;
        meshInfo.fileName = meshObj["fileName"].toString();
        meshInfo.displayName = meshObj["displayName"].toString();
        meshInfo.type = meshObj["type"].toString("일반");
        meshInfo.vertexCount = meshObj["vertexCount"].toInt(0);
        meshInfo.faceCount = meshObj["faceCount"].toInt(0);
        meshInfo.visible = meshObj["visible"].toBool(true);

        mMeshes.append(meshInfo);
    }

    return true;
}

QJsonObject DongArchProjectFile::createSettingsJson() const
{
    QJsonObject json;

    // 카메라 설정
    QJsonObject cameraObj;
    QJsonArray posArray;
    posArray.append(mCamera.position.x());
    posArray.append(mCamera.position.y());
    posArray.append(mCamera.position.z());
    cameraObj["position"] = posArray;

    QJsonArray targetArray;
    targetArray.append(mCamera.target.x());
    targetArray.append(mCamera.target.y());
    targetArray.append(mCamera.target.z());
    cameraObj["target"] = targetArray;

    QJsonArray upArray;
    upArray.append(mCamera.up.x());
    upArray.append(mCamera.up.y());
    upArray.append(mCamera.up.z());
    cameraObj["up"] = upArray;

    cameraObj["fov"] = static_cast<double>(mCamera.fov);
    cameraObj["orthographic"] = mCamera.orthographic;

    json["camera"] = cameraObj;

    // 도구 설정
    QJsonObject toolsObj;
    toolsObj["activeToolIndex"] = mTools.activeToolIndex;
    toolsObj["darkModeEnabled"] = mTools.darkModeEnabled;
    toolsObj["renderMode"] = mTools.renderMode;

    json["tools"] = toolsObj;

    return json;
}

bool DongArchProjectFile::parseSettingsJson(const QJsonObject& json)
{
    // 카메라 설정 파싱
    if (json.contains("camera") && json["camera"].isObject()) {
        QJsonObject cameraObj = json["camera"].toObject();

        if (cameraObj.contains("position") && cameraObj["position"].isArray()) {
            QJsonArray posArray = cameraObj["position"].toArray();
            if (posArray.size() == 3) {
                mCamera.position = QVector3D(
                    posArray[0].toDouble(),
                    posArray[1].toDouble(),
                    posArray[2].toDouble()
                );
            }
        }

        if (cameraObj.contains("target") && cameraObj["target"].isArray()) {
            QJsonArray targetArray = cameraObj["target"].toArray();
            if (targetArray.size() == 3) {
                mCamera.target = QVector3D(
                    targetArray[0].toDouble(),
                    targetArray[1].toDouble(),
                    targetArray[2].toDouble()
                );
            }
        }

        if (cameraObj.contains("up") && cameraObj["up"].isArray()) {
            QJsonArray upArray = cameraObj["up"].toArray();
            if (upArray.size() == 3) {
                mCamera.up = QVector3D(
                    upArray[0].toDouble(),
                    upArray[1].toDouble(),
                    upArray[2].toDouble()
                );
            }
        }

        mCamera.fov = static_cast<float>(cameraObj["fov"].toDouble(45.0));
        mCamera.orthographic = cameraObj["orthographic"].toBool(false);
    }

    // 도구 설정 파싱
    if (json.contains("tools") && json["tools"].isObject()) {
        QJsonObject toolsObj = json["tools"].toObject();
        mTools.activeToolIndex = toolsObj["activeToolIndex"].toInt(0);
        mTools.darkModeEnabled = toolsObj["darkModeEnabled"].toBool(false);
        mTools.renderMode = toolsObj["renderMode"].toString("solid");
    }

    return true;
}

bool DongArchProjectFile::addFileToZip(const QString& zipPath, const QString& filePath, const QString& internalPath)
{
    // TODO: QuaZip 사용하여 구현
    Q_UNUSED(zipPath);
    Q_UNUSED(filePath);
    Q_UNUSED(internalPath);

    mLastError = "ZIP 압축 기능은 아직 구현되지 않았습니다 (QuaZip 필요)";
    return false;
}

bool DongArchProjectFile::extractFileFromZip(const QString& zipPath, const QString& internalPath, const QString& outputPath)
{
    // TODO: QuaZip 사용하여 구현
    Q_UNUSED(zipPath);
    Q_UNUSED(internalPath);
    Q_UNUSED(outputPath);

    mLastError = "ZIP 압축 해제 기능은 아직 구현되지 않았습니다 (QuaZip 필요)";
    return false;
}
