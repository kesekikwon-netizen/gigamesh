/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 7: Illustrator Bridge
 *
 * Illustrator 연동:
 * - SVG 파일을 Illustrator에서 자동 열기
 * - 드래그 앤 드롭 지원
 * - Windows/macOS 플랫폼별 처리
 *
 * This file is part of DongArch3D (based on GigaMesh).
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_ILLUSTRATOR_BRIDGE_H
#define DONGARCH_ILLUSTRATOR_BRIDGE_H

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QMimeData>
#include <memory>

namespace DongArch {
namespace Illustrator {

//! \class IllustratorBridge
//! \brief Adobe Illustrator 연동
//!
//! 기능:
//! 1. Illustrator 설치 경로 자동 감지
//! 2. SVG 파일을 Illustrator에서 열기
//! 3. 드래그 앤 드롭 URL 생성
//! 4. 플랫폼별 처리 (Windows/macOS)
//!
//! 사용법:
//! ```cpp
//! IllustratorBridge bridge;
//!
//! if (bridge.isIllustratorInstalled()) {
//!     bridge.openInIllustrator("/path/to/output.svg");
//! } else {
//!     qWarning() << "Illustrator not found";
//! }
//! ```
class IllustratorBridge
{
public:
    //! Illustrator 버전
    enum class Version {
        Unknown,
        CS6,        //!< 2012
        CC2014,     //!< 2014
        CC2015,     //!< 2015
        CC2017,     //!< 2017
        CC2018,     //!< 2018
        CC2019,     //!< 2019
        CC2020,     //!< 2020
        CC2021,     //!< 2021
        CC2022,     //!< 2022
        CC2023,     //!< 2023
        CC2024,     //!< 2024
        Latest      //!< 최신 버전 (자동 감지)
    };

    IllustratorBridge();
    ~IllustratorBridge();

    //! Illustrator 설치 여부 확인
    //! \return 설치 여부
    bool isIllustratorInstalled();

    //! Illustrator 실행 파일 경로 조회
    //! \return 실행 파일 경로 (없으면 빈 문자열)
    QString getIllustratorPath();

    //! Illustrator 버전 감지
    //! \return 버전
    Version detectVersion();

    //! SVG 파일을 Illustrator에서 열기
    //! \param svgFilePath SVG 파일 경로
    //! \return 성공 여부
    bool openInIllustrator(const QString& svgFilePath);

    //! SVG 파일을 기본 프로그램에서 열기 (Illustrator가 없을 때)
    //! \param svgFilePath SVG 파일 경로
    //! \return 성공 여부
    bool openInDefaultApplication(const QString& svgFilePath);

    //! 드래그 앤 드롭 MIME 데이터 생성
    //! \param svgFilePath SVG 파일 경로
    //! \return MIME 데이터 (QMimeData에 설정)
    QMimeData* createDragMimeData(const QString& svgFilePath);

    //! Illustrator 버전을 문자열로 변환
    //! \param version 버전
    //! \return 버전 문자열 (예: "CC2024")
    static QString versionToString(Version version);

private:
    //! Windows에서 Illustrator 경로 찾기
    //! \return 실행 파일 경로
    QString findIllustratorPathWindows();

    //! macOS에서 Illustrator 경로 찾기
    //! \return 실행 파일 경로
    QString findIllustratorPathMacOS();

    //! Linux에서 Illustrator 경로 찾기 (Wine 사용)
    //! \return 실행 파일 경로
    QString findIllustratorPathLinux();

    //! 버전 번호 파싱
    //! \param versionString 버전 문자열 (예: "25.4.1")
    //! \return 버전
    Version parseVersionString(const QString& versionString);

    QString mIllustratorPath;           //!< Illustrator 실행 파일 경로 (캐시)
    Version mVersion;                   //!< Illustrator 버전 (캐시)
    bool mIsCached;                     //!< 캐시 여부
};

} // namespace Illustrator
} // namespace DongArch

#endif // DONGARCH_ILLUSTRATOR_BRIDGE_H
