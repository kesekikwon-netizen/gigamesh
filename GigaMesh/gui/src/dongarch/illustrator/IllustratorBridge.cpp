/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 7: Illustrator Bridge Implementation
 */

#include "IllustratorBridge.h"
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <QSettings>
#endif

namespace DongArch {
namespace Illustrator {

//==============================================================================
// Constructor / Destructor
//==============================================================================

IllustratorBridge::IllustratorBridge()
    : mVersion(Version::Unknown)
    , mIsCached(false)
{
}

IllustratorBridge::~IllustratorBridge()
{
}

//==============================================================================
// Illustrator Detection
//==============================================================================

bool IllustratorBridge::isIllustratorInstalled()
{
    if (mIsCached) {
        return !mIllustratorPath.isEmpty();
    }

    QString path = getIllustratorPath();
    return !path.isEmpty();
}

QString IllustratorBridge::getIllustratorPath()
{
    if (mIsCached) {
        return mIllustratorPath;
    }

#ifdef Q_OS_WIN
    mIllustratorPath = findIllustratorPathWindows();
#elif defined(Q_OS_MAC)
    mIllustratorPath = findIllustratorPathMacOS();
#else
    mIllustratorPath = findIllustratorPathLinux();
#endif

    mIsCached = true;

    if (!mIllustratorPath.isEmpty()) {
        qDebug() << "Illustrator found at:" << mIllustratorPath;
    } else {
        qWarning() << "Illustrator not found";
    }

    return mIllustratorPath;
}

IllustratorBridge::Version IllustratorBridge::detectVersion()
{
    if (mVersion != Version::Unknown) {
        return mVersion;
    }

    QString path = getIllustratorPath();
    if (path.isEmpty()) {
        return Version::Unknown;
    }

    // Parse version from path (e.g., "Adobe Illustrator 2024")
    if (path.contains("2024")) mVersion = Version::CC2024;
    else if (path.contains("2023")) mVersion = Version::CC2023;
    else if (path.contains("2022")) mVersion = Version::CC2022;
    else if (path.contains("2021")) mVersion = Version::CC2021;
    else if (path.contains("2020")) mVersion = Version::CC2020;
    else if (path.contains("2019")) mVersion = Version::CC2019;
    else if (path.contains("2018")) mVersion = Version::CC2018;
    else if (path.contains("2017")) mVersion = Version::CC2017;
    else if (path.contains("2015")) mVersion = Version::CC2015;
    else if (path.contains("2014")) mVersion = Version::CC2014;
    else if (path.contains("CS6")) mVersion = Version::CS6;
    else mVersion = Version::Latest;

    return mVersion;
}

//==============================================================================
// Platform-specific Path Detection
//==============================================================================

QString IllustratorBridge::findIllustratorPathWindows()
{
#ifdef Q_OS_WIN
    // Common installation paths
    QStringList possiblePaths = {
        "C:/Program Files/Adobe/Adobe Illustrator 2024/Support Files/Contents/Windows/Illustrator.exe",
        "C:/Program Files/Adobe/Adobe Illustrator 2023/Support Files/Contents/Windows/Illustrator.exe",
        "C:/Program Files/Adobe/Adobe Illustrator 2022/Support Files/Contents/Windows/Illustrator.exe",
        "C:/Program Files/Adobe/Adobe Illustrator CC 2019/Support Files/Contents/Windows/Illustrator.exe",
        "C:/Program Files/Adobe/Adobe Illustrator CC 2018/Support Files/Contents/Windows/Illustrator.exe",
    };

    for (const QString& path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            return path;
        }
    }

    // Try registry
    QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\Adobe\\Illustrator", QSettings::NativeFormat);
    QStringList keys = registry.allKeys();

    for (const QString& key : keys) {
        if (key.contains("InstallPath")) {
            QString installPath = registry.value(key).toString();
            if (!installPath.isEmpty()) {
                QString exePath = installPath + "/Illustrator.exe";
                if (QFileInfo::exists(exePath)) {
                    return exePath;
                }
            }
        }
    }
#endif

    return QString();
}

QString IllustratorBridge::findIllustratorPathMacOS()
{
#ifdef Q_OS_MAC
    QStringList possiblePaths = {
        "/Applications/Adobe Illustrator 2024/Adobe Illustrator.app",
        "/Applications/Adobe Illustrator 2023/Adobe Illustrator.app",
        "/Applications/Adobe Illustrator 2022/Adobe Illustrator.app",
        "/Applications/Adobe Illustrator CC 2019/Adobe Illustrator.app",
        "/Applications/Adobe Illustrator CC 2018/Adobe Illustrator.app",
    };

    for (const QString& path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            return path;
        }
    }
#endif

    return QString();
}

QString IllustratorBridge::findIllustratorPathLinux()
{
    // Linux doesn't have native Illustrator, but might use Wine
    // This is a placeholder for Wine-based installation

    QStringList possiblePaths = {
        QDir::homePath() + "/.wine/drive_c/Program Files/Adobe/Adobe Illustrator 2024/Support Files/Contents/Windows/Illustrator.exe"
    };

    for (const QString& path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            return path;
        }
    }

    return QString();
}

//==============================================================================
// File Opening
//==============================================================================

bool IllustratorBridge::openInIllustrator(const QString& svgFilePath)
{
    QString illustratorPath = getIllustratorPath();

    if (illustratorPath.isEmpty()) {
        qWarning() << "Illustrator not found, falling back to default application";
        return openInDefaultApplication(svgFilePath);
    }

    QFileInfo svgInfo(svgFilePath);
    if (!svgInfo.exists()) {
        qCritical() << "SVG file does not exist:" << svgFilePath;
        return false;
    }

#ifdef Q_OS_WIN
    // Windows: Use ShellExecute or QProcess
    QStringList args;
    args << svgFilePath;

    QProcess process;
    process.startDetached(illustratorPath, args);

    qDebug() << "Opened" << svgFilePath << "in Illustrator";
    return true;

#elif defined(Q_OS_MAC)
    // macOS: Use "open" command
    QStringList args;
    args << "-a" << illustratorPath << svgFilePath;

    QProcess process;
    process.startDetached("open", args);

    qDebug() << "Opened" << svgFilePath << "in Illustrator (macOS)";
    return true;

#else
    // Linux: Use Wine (if available)
    qWarning() << "Opening Illustrator on Linux is not fully supported";
    return openInDefaultApplication(svgFilePath);
#endif
}

bool IllustratorBridge::openInDefaultApplication(const QString& svgFilePath)
{
    QUrl url = QUrl::fromLocalFile(svgFilePath);
    bool success = QDesktopServices::openUrl(url);

    if (success) {
        qDebug() << "Opened" << svgFilePath << "in default application";
    } else {
        qCritical() << "Failed to open" << svgFilePath;
    }

    return success;
}

//==============================================================================
// Drag & Drop
//==============================================================================

QMimeData* IllustratorBridge::createDragMimeData(const QString& svgFilePath)
{
    QMimeData* mimeData = new QMimeData();

    // File URL
    QList<QUrl> urls;
    urls.append(QUrl::fromLocalFile(svgFilePath));
    mimeData->setUrls(urls);

    // Plain text (file path)
    mimeData->setText(svgFilePath);

    return mimeData;
}

//==============================================================================
// Utilities
//==============================================================================

QString IllustratorBridge::versionToString(Version version)
{
    switch (version) {
        case Version::CS6: return "CS6";
        case Version::CC2014: return "CC2014";
        case Version::CC2015: return "CC2015";
        case Version::CC2017: return "CC2017";
        case Version::CC2018: return "CC2018";
        case Version::CC2019: return "CC2019";
        case Version::CC2020: return "CC2020";
        case Version::CC2021: return "CC2021";
        case Version::CC2022: return "CC2022";
        case Version::CC2023: return "CC2023";
        case Version::CC2024: return "CC2024";
        case Version::Latest: return "Latest";
        default: return "Unknown";
    }
}

IllustratorBridge::Version IllustratorBridge::parseVersionString(const QString& versionString)
{
    if (versionString.contains("25.")) return Version::CC2021;
    if (versionString.contains("26.")) return Version::CC2022;
    if (versionString.contains("27.")) return Version::CC2023;
    if (versionString.contains("28.")) return Version::CC2024;

    return Version::Unknown;
}

} // namespace Illustrator
} // namespace DongArch
