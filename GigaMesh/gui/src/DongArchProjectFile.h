/**
 * DongArch3D Project File Format
 *
 * .dongarch3d 프로젝트 파일 형식 (ZIP 기반)
 *
 * File Structure:
 * - project.json      : 프로젝트 메타데이터
 * - mesh/             : 3D 메시 파일들
 * - screenshots/      : 스크린샷 이미지
 * - settings.json     : 프로젝트 설정
 *
 * @file DongArchProjectFile.h
 * @author DongArch3D Team
 * @date 2025-11-08
 * @version 1.0.0
 */

#ifndef DONGARCH_PROJECT_FILE_H
#define DONGARCH_PROJECT_FILE_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QVector3D>

/**
 * @brief DongArch3D 프로젝트 파일 관리 클래스
 *
 * .dongarch3d 파일은 ZIP 압축 형식으로 다음을 포함:
 * - project.json: 프로젝트 정보 (이름, 날짜, 메시 목록)
 * - mesh/*.ply: 3D 메시 파일들
 * - screenshots/*.png: 스크린샷
 * - settings.json: 카메라, 도구 설정
 */
class DongArchProjectFile
{
public:
    /**
     * @brief 프로젝트 메타데이터 구조체
     */
    struct ProjectMetadata {
        QString version;              // 파일 버전 (예: "1.0.0")
        QString name;                 // 프로젝트 이름
        QString description;          // 설명
        QDateTime created;            // 생성 날짜
        QDateTime modified;           // 수정 날짜
        QString author;               // 작성자

        ProjectMetadata() : version("1.0.0") {}
    };

    /**
     * @brief 메시 파일 정보 구조체
     */
    struct MeshInfo {
        QString fileName;             // 파일명 (예: "artifact_001.ply")
        QString displayName;          // 표시 이름 (한글 가능)
        QString type;                 // 유물 타입 ("석기", "토기", "금속기", "일반")
        int vertexCount;              // 정점 수
        int faceCount;                // 면 수
        bool visible;                 // 가시성

        MeshInfo() : vertexCount(0), faceCount(0), visible(true) {}
    };

    /**
     * @brief 카메라 설정 구조체
     */
    struct CameraSettings {
        QVector3D position;           // 카메라 위치
        QVector3D target;             // 타겟 위치
        QVector3D up;                 // 업 벡터
        float fov;                    // 시야각 (FOV)
        bool orthographic;            // 정투영 모드

        CameraSettings() : fov(45.0f), orthographic(false) {}
    };

    /**
     * @brief 도구 설정 구조체
     */
    struct ToolSettings {
        int activeToolIndex;          // 활성 도구 인덱스 (0-19)
        bool darkModeEnabled;         // 다크 모드 활성화
        QString renderMode;           // 렌더링 모드 ("wireframe", "solid", "npr")

        ToolSettings() : activeToolIndex(0), darkModeEnabled(false), renderMode("solid") {}
    };

public:
    DongArchProjectFile();
    ~DongArchProjectFile();

    /**
     * @brief 프로젝트 저장
     * @param filePath .dongarch3d 파일 경로
     * @return 성공 여부
     */
    bool save(const QString& filePath);

    /**
     * @brief 프로젝트 로드
     * @param filePath .dongarch3d 파일 경로
     * @return 성공 여부
     */
    bool load(const QString& filePath);

    /**
     * @brief 메시 파일 추가
     * @param meshFilePath 메시 파일 절대 경로
     * @param displayName 표시 이름
     * @param type 유물 타입
     * @return 성공 여부
     */
    bool addMesh(const QString& meshFilePath, const QString& displayName, const QString& type);

    /**
     * @brief 스크린샷 추가
     * @param screenshotPath 스크린샷 파일 경로
     * @param name 스크린샷 이름
     * @return 성공 여부
     */
    bool addScreenshot(const QString& screenshotPath, const QString& name);

    /**
     * @brief 마지막 오류 메시지 가져오기
     * @return 오류 메시지
     */
    QString getLastError() const { return mLastError; }

    // Getter/Setter
    ProjectMetadata& metadata() { return mMetadata; }
    const ProjectMetadata& metadata() const { return mMetadata; }

    QVector<MeshInfo>& meshes() { return mMeshes; }
    const QVector<MeshInfo>& meshes() const { return mMeshes; }

    CameraSettings& camera() { return mCamera; }
    const CameraSettings& camera() const { return mCamera; }

    ToolSettings& tools() { return mTools; }
    const ToolSettings& tools() const { return mTools; }

private:
    /**
     * @brief project.json 생성
     * @return QJsonObject
     */
    QJsonObject createProjectJson() const;

    /**
     * @brief project.json 파싱
     * @param json QJsonObject
     * @return 성공 여부
     */
    bool parseProjectJson(const QJsonObject& json);

    /**
     * @brief settings.json 생성
     * @return QJsonObject
     */
    QJsonObject createSettingsJson() const;

    /**
     * @brief settings.json 파싱
     * @param json QJsonObject
     * @return 성공 여부
     */
    bool parseSettingsJson(const QJsonObject& json);

    /**
     * @brief ZIP 파일에 파일 추가
     * @param zipPath ZIP 파일 경로
     * @param filePath 추가할 파일 경로
     * @param internalPath ZIP 내부 경로
     * @return 성공 여부
     */
    bool addFileToZip(const QString& zipPath, const QString& filePath, const QString& internalPath);

    /**
     * @brief ZIP 파일에서 파일 추출
     * @param zipPath ZIP 파일 경로
     * @param internalPath ZIP 내부 경로
     * @param outputPath 출력 경로
     * @return 성공 여부
     */
    bool extractFileFromZip(const QString& zipPath, const QString& internalPath, const QString& outputPath);

private:
    ProjectMetadata mMetadata;
    QVector<MeshInfo> mMeshes;
    CameraSettings mCamera;
    ToolSettings mTools;
    QString mLastError;
};

#endif // DONGARCH_PROJECT_FILE_H
