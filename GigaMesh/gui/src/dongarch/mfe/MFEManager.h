/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 6: Mesh File Explorer Manager
 *
 * MFE (Mesh File Explorer):
 * - 3D 파일 탐색 및 관리
 * - 파일 정보 추출 (면, 버텍스, 바운딩박스)
 * - 썸네일 미리보기
 * - 빠른 파일 열기
 *
 * This file is part of DongArch3D (based on GigaMesh).
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_MFE_MANAGER_H
#define DONGARCH_MFE_MANAGER_H

#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QImage>
#include <QVector3D>
#include <vector>
#include <memory>
#include <optional>

namespace DongArch {
namespace MFE {

//! \struct MeshFileInfo
//! \brief 메시 파일 정보
struct MeshFileInfo
{
    QString filePath;           //!< 파일 전체 경로
    QString fileName;           //!< 파일 이름
    QString fileExtension;      //!< 확장자 (.ply, .obj, .off, .stl)
    qint64 fileSize;            //!< 파일 크기 (bytes)

    // 메시 정보 (파일 파싱 필요)
    int vertexCount;            //!< 버텍스 개수
    int faceCount;              //!< 면 개수
    QVector3D boundingBoxMin;   //!< 바운딩 박스 최소점
    QVector3D boundingBoxMax;   //!< 바운딩 박스 최대점
    QVector3D center;           //!< 중심점

    // 썸네일
    QImage thumbnail;           //!< 썸네일 이미지 (128x128)

    // 메타데이터
    bool isValid;               //!< 파일이 유효한지 (파싱 성공)
    QString errorMessage;       //!< 에러 메시지 (파싱 실패 시)
};

//! \class MFEManager
//! \brief Mesh File Explorer 관리자
//!
//! 기능:
//! 1. 디렉토리 스캔 (재귀적)
//! 2. 지원 파일 필터링 (.ply, .obj, .off, .stl)
//! 3. 파일 정보 추출 (버텍스, 면 개수)
//! 4. 썸네일 생성 (빠른 미리보기)
//!
//! 사용법:
//! ```cpp
//! MFEManager mfe;
//! mfe.scanDirectory("/path/to/meshes");
//!
//! auto files = mfe.getFileList();
//! for (const auto& file : files) {
//!     qDebug() << file.fileName << ":" << file.faceCount << "faces";
//! }
//!
//! auto info = mfe.getFileInfo("/path/to/mesh.ply");
//! if (info && info->isValid) {
//!     QImage thumb = info->thumbnail;
//! }
//! ```
class MFEManager
{
public:
    //! 지원하는 파일 형식
    enum class FileFormat {
        PLY,    //!< Stanford PLY
        OBJ,    //!< Wavefront OBJ
        OFF,    //!< Object File Format
        STL,    //!< STereoLithography
        Unknown //!< 지원하지 않는 형식
    };

    MFEManager();
    ~MFEManager();

    //! 디렉토리 스캔
    //! \param directoryPath 스캔할 디렉토리 경로
    //! \param recursive 재귀적으로 하위 폴더 스캔 여부
    //! \return 스캔된 파일 개수
    int scanDirectory(const QString& directoryPath, bool recursive = true);

    //! 파일 목록 조회
    //! \return 스캔된 메시 파일 목록
    const std::vector<MeshFileInfo>& getFileList() const { return mFileList; }

    //! 특정 파일 정보 조회
    //! \param filePath 파일 경로
    //! \return 파일 정보 (없으면 std::nullopt)
    std::optional<MeshFileInfo> getFileInfo(const QString& filePath) const;

    //! 파일 형식 감지
    //! \param filePath 파일 경로
    //! \return 파일 형식
    static FileFormat detectFormat(const QString& filePath);

    //! 파일 형식을 문자열로 변환
    //! \param format 파일 형식
    //! \return 문자열 (예: "PLY", "OBJ")
    static QString formatToString(FileFormat format);

    //! 지원하는 파일 확장자 목록
    //! \return 확장자 목록 (예: "*.ply", "*.obj")
    static QStringList getSupportedExtensions();

    //! 파일 필터 문자열 (파일 다이얼로그용)
    //! \return 필터 문자열 (예: "Mesh Files (*.ply *.obj *.off *.stl)")
    static QString getFileFilter();

    //! 파일 정보 강제 새로고침
    //! \param filePath 파일 경로
    //! \return 새로고침된 파일 정보
    std::optional<MeshFileInfo> refreshFileInfo(const QString& filePath);

    //! 썸네일 생성 (별도 스레드에서 실행 가능)
    //! \param filePath 파일 경로
    //! \param size 썸네일 크기 (정사각형)
    //! \return 썸네일 이미지
    static QImage generateThumbnail(const QString& filePath, int size = 128);

    //! 썸네일 캐시 경로 가져오기
    //! \param filePath 원본 파일 경로
    //! \return 캐시 파일 경로
    static QString getThumbnailCachePath(const QString& filePath);

    //! 캐시된 썸네일 로드
    //! \param filePath 원본 파일 경로
    //! \return 캐시된 썸네일 (없으면 null image)
    static QImage loadCachedThumbnail(const QString& filePath);

    //! 썸네일 캐시에 저장
    //! \param filePath 원본 파일 경로
    //! \param thumbnail 썸네일 이미지
    //! \return 성공 여부
    static bool saveThumbnailToCache(const QString& filePath, const QImage& thumbnail);

    //! 전체 파일 목록 초기화
    void clear();

private:
    //! 간단한 메시 데이터 구조 (썸네일 렌더링용)
    struct SimpleMesh {
        std::vector<float> vertices;  // x,y,z triplets
        std::vector<unsigned int> indices;  // Triangle indices
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    //! 간단한 메시 로딩 (썸네일용 - 빠른 파싱)
    //! \param filePath 파일 경로
    //! \param format 파일 형식
    //! \return 간단한 메시 데이터
    static SimpleMesh loadSimpleMesh(const QString& filePath, FileFormat format);

    //! 메시를 이미지로 렌더링 (Offscreen)
    //! \param mesh 메시 데이터
    //! \param size 이미지 크기
    //! \return 렌더링된 이미지
    static QImage renderMeshToImage(const SimpleMesh& mesh, int size);

    //! 플레이스홀더 썸네일 생성
    //! \param size 이미지 크기
    //! \param text 표시할 텍스트
    //! \return 플레이스홀더 이미지
    static QImage createPlaceholderThumbnail(int size, const QString& text);

    //! 파일 정보 파싱 (버텍스/면 개수, 바운딩박스)
    //! \param filePath 파일 경로
    //! \param format 파일 형식
    //! \return 파일 정보
    MeshFileInfo parseFileInfo(const QString& filePath, FileFormat format);

    //! PLY 파일 파싱
    //! \param filePath 파일 경로
    //! \return 파일 정보
    MeshFileInfo parsePLY(const QString& filePath);

    //! OBJ 파일 파싱
    //! \param filePath 파일 경로
    //! \return 파일 정보
    MeshFileInfo parseOBJ(const QString& filePath);

    //! OFF 파일 파싱
    //! \param filePath 파일 경로
    //! \return 파일 정보
    MeshFileInfo parseOFF(const QString& filePath);

    //! STL 파일 파싱
    //! \param filePath 파일 경로
    //! \return 파일 정보
    MeshFileInfo parseSTL(const QString& filePath);

    //! 디렉토리 재귀 스캔 헬퍼
    //! \param dirPath 디렉토리 경로
    //! \param recursive 재귀 여부
    void scanDirectoryRecursive(const QString& dirPath, bool recursive);

    std::vector<MeshFileInfo> mFileList;  //!< 스캔된 파일 목록
};

} // namespace MFE
} // namespace DongArch

#endif // DONGARCH_MFE_MANAGER_H
