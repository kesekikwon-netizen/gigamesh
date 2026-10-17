/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DongArchProject.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QDateTime>
#include <QTextStream>
#include <QDataStream>
#include <QDebug>

// Simple ZIP implementation using Qt
// For production, consider using QuaZip or zlib
namespace {

/*!
 * @brief Create a simple ZIP archive (store method, no compression)
 * @note This is a minimal implementation for demonstration.
 *       For production, use QuaZip or system ZIP utilities.
 */
bool createSimpleZip(const QString& sourceDir, const QString& zipPath)
{
    QFile zipFile(zipPath);
    if (!zipFile.open(QIODevice::WriteOnly)) {
        return false;
    }

    QDataStream stream(&zipFile);
    stream.setByteOrder(QDataStream::LittleEndian);

    QDir dir(sourceDir);
    QStringList allFiles;

    // Collect all files recursively
    QList<QPair<QString, QString>> fileList; // (absolutePath, relativePath)

    QStringList queue;
    queue.append("");

    while (!queue.isEmpty()) {
        QString relDir = queue.takeFirst();
        QString absDir = dir.absoluteFilePath(relDir);

        QDir currentDir(absDir);
        QFileInfoList entries = currentDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QFileInfo& entry : entries) {
            QString relPath = relDir.isEmpty() ? entry.fileName() : relDir + "/" + entry.fileName();

            if (entry.isDir()) {
                queue.append(relPath);
            } else {
                fileList.append(qMakePair(entry.absoluteFilePath(), relPath));
            }
        }
    }

    // Write simple ZIP format
    // Local file header signature
    for (const auto& filePair : fileList) {
        QFile file(filePair.first);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        QByteArray fileData = file.readAll();
        QByteArray fileName = filePair.second.toUtf8();

        // Local file header signature: 0x04034b50
        stream << quint32(0x04034b50);
        // Version needed to extract: 2.0
        stream << quint16(20);
        // General purpose bit flag
        stream << quint16(0);
        // Compression method: 0 = store (no compression)
        stream << quint16(0);
        // File last modification time (DOS format)
        stream << quint16(0);
        // File last modification date (DOS format)
        stream << quint16(0);
        // CRC-32
        stream << quint32(qChecksum(fileData.constData(), fileData.size()));
        // Compressed size
        stream << quint32(fileData.size());
        // Uncompressed size
        stream << quint32(fileData.size());
        // File name length
        stream << quint16(fileName.size());
        // Extra field length
        stream << quint16(0);

        // File name
        zipFile.write(fileName);
        // File data
        zipFile.write(fileData);

        file.close();
    }

    // Central directory structure (simplified - end of central directory record only)
    // End of central directory signature: 0x06054b50
    stream << quint32(0x06054b50);
    // Number of this disk
    stream << quint16(0);
    // Disk where central directory starts
    stream << quint16(0);
    // Number of central directory records on this disk
    stream << quint16(fileList.size());
    // Total number of central directory records
    stream << quint16(fileList.size());
    // Size of central directory (bytes)
    stream << quint32(0);
    // Offset of start of central directory
    stream << quint32(0);
    // Comment length
    stream << quint16(0);

    zipFile.close();
    return true;
}

/*!
 * @brief Extract a simple ZIP archive
 * @note This is a minimal implementation. For production, use QuaZip or system utilities.
 */
bool extractSimpleZip(const QString& zipPath, const QString& destDir)
{
    QFile zipFile(zipPath);
    if (!zipFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDataStream stream(&zipFile);
    stream.setByteOrder(QDataStream::LittleEndian);

    QDir dir(destDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    while (!stream.atEnd()) {
        quint32 signature;
        stream >> signature;

        if (signature == 0x04034b50) { // Local file header
            quint16 versionNeeded, flags, compression, modTime, modDate;
            quint32 crc32, compressedSize, uncompressedSize;
            quint16 fileNameLength, extraFieldLength;

            stream >> versionNeeded >> flags >> compression >> modTime >> modDate;
            stream >> crc32 >> compressedSize >> uncompressedSize;
            stream >> fileNameLength >> extraFieldLength;

            // Read file name
            QByteArray fileNameBytes(fileNameLength, 0);
            stream.readRawData(fileNameBytes.data(), fileNameLength);
            QString fileName = QString::fromUtf8(fileNameBytes);

            // Skip extra field
            if (extraFieldLength > 0) {
                zipFile.seek(zipFile.pos() + extraFieldLength);
            }

            // Read file data
            QByteArray fileData(uncompressedSize, 0);
            stream.readRawData(fileData.data(), uncompressedSize);

            // Write file
            QString filePath = dir.absoluteFilePath(fileName);
            QFileInfo fileInfo(filePath);
            dir.mkpath(fileInfo.absolutePath());

            QFile outFile(filePath);
            if (outFile.open(QIODevice::WriteOnly)) {
                outFile.write(fileData);
                outFile.close();
            }
        } else if (signature == 0x06054b50) { // End of central directory
            break; // We're done
        }
    }

    zipFile.close();
    return true;
}

} // anonymous namespace

DongArchProject::DongArchProject()
{
    clear();
}

DongArchProject::~DongArchProject()
{
    // Cleanup temporary files
    if (!mTempDir.isEmpty() && QDir(mTempDir).exists()) {
        QDir(mTempDir).removeRecursively();
    }
}

bool DongArchProject::save(const QString& filepath)
{
    mLastError.clear();

    // Validate file extension
    if (!filepath.endsWith(".dongarch3d", Qt::CaseInsensitive)) {
        mLastError = "File must have .dongarch3d extension";
        return false;
    }

    // Create temporary directory
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        mLastError = "Failed to create temporary directory";
        return false;
    }

    QString tempPath = tempDir.path();

    // Update manifest metadata
    mManifest["version"] = "1.0.0";
    mManifest["modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    mManifest["application"] = "DongArch3D 1.0.0 (GigaMesh Framework)";

    if (!mManifest.contains("created")) {
        mManifest["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    }

    // Build manifest structure
    QJsonArray meshesArray;
    for (const MeshInfo& mesh : mMeshes) {
        QJsonObject meshObj;
        meshObj["id"] = mesh.id;
        meshObj["filename"] = mesh.filename;
        meshObj["type"] = mesh.type;
        meshObj["name"] = mesh.name;
        if (!mesh.metadata.isEmpty()) {
            meshObj["metadata"] = mesh.metadata;
        }
        meshesArray.append(meshObj);
    }
    mManifest["meshes"] = meshesArray;

    QJsonArray sectionsArray;
    for (const SectionInfo& section : mSections) {
        QJsonObject sectionObj;
        sectionObj["id"] = section.id;
        sectionObj["filename"] = section.filename;
        sectionObj["type"] = section.type;

        QJsonArray posArray;
        for (double val : section.position) {
            posArray.append(val);
        }
        sectionObj["position"] = posArray;

        if (!section.metadata.isEmpty()) {
            sectionObj["metadata"] = section.metadata;
        }
        sectionsArray.append(sectionObj);
    }
    mManifest["sections"] = sectionsArray;

    if (!mMeasurements.isEmpty()) {
        mManifest["measurements"] = "measurements/measurements.csv";
    }

    if (!mAnnotations.isEmpty()) {
        mManifest["annotations"] = "annotations/annotations.json";
    }

    if (!mScreenshots.isEmpty()) {
        QJsonArray screenshotsArray;
        for (const QString& screenshot : mScreenshots) {
            screenshotsArray.append(screenshot);
        }
        mManifest["screenshots"] = screenshotsArray;
    }

    // Write manifest.json
    if (!writeManifest(tempPath + "/manifest.json")) {
        mLastError = "Failed to write manifest.json";
        return false;
    }

    // Copy mesh files
    for (const MeshInfo& mesh : mMeshes) {
        if (mFiles.contains(mesh.filename)) {
            QString destPath = tempPath + "/" + mesh.filename;
            QFileInfo destInfo(destPath);
            QDir().mkpath(destInfo.absolutePath());

            if (!QFile::copy(mFiles[mesh.filename], destPath)) {
                mLastError = "Failed to copy mesh file: " + mesh.filename;
                return false;
            }
        }
    }

    // Copy section files
    for (const SectionInfo& section : mSections) {
        if (mFiles.contains(section.filename)) {
            QString destPath = tempPath + "/" + section.filename;
            QFileInfo destInfo(destPath);
            QDir().mkpath(destInfo.absolutePath());

            if (!QFile::copy(mFiles[section.filename], destPath)) {
                mLastError = "Failed to copy section file: " + section.filename;
                return false;
            }
        }
    }

    // Write measurements
    if (!mMeasurements.isEmpty()) {
        QString measurementsPath = tempPath + "/measurements/measurements.csv";
        QDir().mkpath(tempPath + "/measurements");
        if (!writeMeasurements(measurementsPath)) {
            mLastError = "Failed to write measurements.csv";
            return false;
        }
    }

    // Write annotations
    if (!mAnnotations.isEmpty()) {
        QString annotationsPath = tempPath + "/annotations/annotations.json";
        QDir().mkpath(tempPath + "/annotations");

        QFile annotFile(annotationsPath);
        if (!annotFile.open(QIODevice::WriteOnly)) {
            mLastError = "Failed to create annotations.json";
            return false;
        }

        QJsonObject annotObj;
        annotObj["annotations"] = mAnnotations;
        annotFile.write(QJsonDocument(annotObj).toJson(QJsonDocument::Indented));
        annotFile.close();
    }

    // Copy screenshot files
    for (const QString& screenshot : mScreenshots) {
        if (mFiles.contains(screenshot)) {
            QString destPath = tempPath + "/" + screenshot;
            QFileInfo destInfo(destPath);
            QDir().mkpath(destInfo.absolutePath());

            if (!QFile::copy(mFiles[screenshot], destPath)) {
                mLastError = "Failed to copy screenshot: " + screenshot;
                return false;
            }
        }
    }

    // Create ZIP archive
    if (!createSimpleZip(tempPath, filepath)) {
        mLastError = "Failed to create ZIP archive";
        return false;
    }

    qDebug() << "DongArchProject: Saved successfully to" << filepath;
    return true;
}

bool DongArchProject::load(const QString& filepath)
{
    mLastError.clear();
    clear();

    // Validate file exists
    if (!QFile::exists(filepath)) {
        mLastError = "File does not exist: " + filepath;
        return false;
    }

    // Create temporary directory for extraction
    QTemporaryDir tempDir;
    if (!tempDir.isValid()) {
        mLastError = "Failed to create temporary directory";
        return false;
    }

    mTempDir = tempDir.path();
    tempDir.setAutoRemove(false); // We'll manage cleanup ourselves

    // Extract ZIP archive
    if (!extractSimpleZip(filepath, mTempDir)) {
        mLastError = "Failed to extract ZIP archive";
        return false;
    }

    // Read manifest.json
    if (!readManifest(mTempDir + "/manifest.json")) {
        mLastError = "Failed to read manifest.json";
        return false;
    }

    // Parse meshes
    QJsonArray meshesArray = mManifest["meshes"].toArray();
    for (const QJsonValue& val : meshesArray) {
        QJsonObject meshObj = val.toObject();

        MeshInfo mesh;
        mesh.id = meshObj["id"].toString();
        mesh.filename = meshObj["filename"].toString();
        mesh.type = meshObj["type"].toString();
        mesh.name = meshObj["name"].toString();
        mesh.metadata = meshObj["metadata"].toObject();

        mMeshes.append(mesh);

        // Store temporary file path
        mFiles[mesh.filename] = mTempDir + "/" + mesh.filename;
    }

    // Parse sections
    QJsonArray sectionsArray = mManifest["sections"].toArray();
    for (const QJsonValue& val : sectionsArray) {
        QJsonObject sectionObj = val.toObject();

        SectionInfo section;
        section.id = sectionObj["id"].toString();
        section.filename = sectionObj["filename"].toString();
        section.type = sectionObj["type"].toString();

        QJsonArray posArray = sectionObj["position"].toArray();
        for (const QJsonValue& posVal : posArray) {
            section.position.append(posVal.toDouble());
        }

        section.metadata = sectionObj["metadata"].toObject();

        mSections.append(section);

        // Store temporary file path
        mFiles[section.filename] = mTempDir + "/" + section.filename;
    }

    // Read measurements
    if (mManifest.contains("measurements")) {
        QString measurementsPath = mTempDir + "/" + mManifest["measurements"].toString();
        if (QFile::exists(measurementsPath)) {
            readMeasurements(measurementsPath);
        }
    }

    // Read annotations
    if (mManifest.contains("annotations")) {
        QString annotationsPath = mTempDir + "/" + mManifest["annotations"].toString();
        QFile annotFile(annotationsPath);
        if (annotFile.open(QIODevice::ReadOnly)) {
            QJsonObject annotObj = QJsonDocument::fromJson(annotFile.readAll()).object();
            mAnnotations = annotObj["annotations"].toArray();
            annotFile.close();
        }
    }

    // Read screenshots
    if (mManifest.contains("screenshots")) {
        QJsonArray screenshotsArray = mManifest["screenshots"].toArray();
        for (const QJsonValue& val : screenshotsArray) {
            QString screenshot = val.toString();
            mScreenshots.append(screenshot);
            mFiles[screenshot] = mTempDir + "/" + screenshot;
        }
    }

    qDebug() << "DongArchProject: Loaded successfully from" << filepath;
    qDebug() << "  Meshes:" << mMeshes.size();
    qDebug() << "  Sections:" << mSections.size();
    qDebug() << "  Measurements:" << mMeasurements.size();

    return true;
}

void DongArchProject::addMesh(const QString& id, const QString& plyPath,
                               const QString& type, const QString& name)
{
    MeshInfo mesh;
    mesh.id = id;
    mesh.filename = "meshes/" + id + ".ply";
    mesh.type = type;
    mesh.name = name.isEmpty() ? id : name;

    mMeshes.append(mesh);
    mFiles[mesh.filename] = plyPath;
}

void DongArchProject::addSection(const QString& id, const QJsonObject& sectionData,
                                   const QString& type)
{
    SectionInfo section;
    section.id = id;
    section.filename = "sections/" + id + ".json";
    section.type = type;

    // Extract position if present
    if (sectionData.contains("position")) {
        QJsonArray posArray = sectionData["position"].toArray();
        for (const QJsonValue& val : posArray) {
            section.position.append(val.toDouble());
        }
    } else {
        section.position = {0.0, 0.0, 0.0};
    }

    section.metadata = sectionData;

    mSections.append(section);

    // Write section data to temporary file
    QTemporaryDir tempDir;
    QString tempFile = tempDir.path() + "/section.json";
    QFile file(tempFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(sectionData).toJson(QJsonDocument::Indented));
        file.close();

        // Copy to permanent location
        QString permFile = QDir::temp().absoluteFilePath("dongarch_" + id + ".json");
        QFile::copy(tempFile, permFile);
        mFiles[section.filename] = permFile;
    }
}

void DongArchProject::addMeasurement(const Measurement& measurement)
{
    mMeasurements.append(measurement);
}

void DongArchProject::addScreenshot(const QString& screenshotPath, const QString& id)
{
    QString screenshotId = id.isEmpty() ?
        "view" + QString::number(mScreenshots.size() + 1).rightJustified(3, '0') : id;

    QString filename = "screenshots/" + screenshotId + ".png";
    mScreenshots.append(filename);
    mFiles[filename] = screenshotPath;
}

void DongArchProject::addAnnotation(const QJsonObject& annotationData)
{
    mAnnotations.append(annotationData);
}

QVector<DongArchProject::MeshInfo> DongArchProject::getMeshes() const
{
    return mMeshes;
}

QVector<DongArchProject::SectionInfo> DongArchProject::getSections() const
{
    return mSections;
}

QVector<DongArchProject::Measurement> DongArchProject::getMeasurements() const
{
    return mMeasurements;
}

QString DongArchProject::getMeshFilePath(const QString& id) const
{
    for (const MeshInfo& mesh : mMeshes) {
        if (mesh.id == id) {
            return mFiles.value(mesh.filename, QString());
        }
    }
    return QString();
}

QJsonObject DongArchProject::getSectionData(const QString& id) const
{
    for (const SectionInfo& section : mSections) {
        if (section.id == id) {
            QString filePath = mFiles.value(section.filename, QString());
            if (!filePath.isEmpty() && QFile::exists(filePath)) {
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly)) {
                    return QJsonDocument::fromJson(file.readAll()).object();
                }
            }
            return section.metadata;
        }
    }
    return QJsonObject();
}

QString DongArchProject::getVersion() const
{
    return mManifest["version"].toString("1.0.0");
}

QDateTime DongArchProject::getCreated() const
{
    return QDateTime::fromString(mManifest["created"].toString(), Qt::ISODate);
}

QDateTime DongArchProject::getModified() const
{
    return QDateTime::fromString(mManifest["modified"].toString(), Qt::ISODate);
}

void DongArchProject::clear()
{
    mManifest = QJsonObject();
    mMeshes.clear();
    mSections.clear();
    mMeasurements.clear();
    mAnnotations = QJsonArray();
    mScreenshots.clear();
    mFiles.clear();
    mLastError.clear();

    // Cleanup temporary directory
    if (!mTempDir.isEmpty() && QDir(mTempDir).exists()) {
        QDir(mTempDir).removeRecursively();
    }
    mTempDir.clear();
}

bool DongArchProject::writeManifest(const QString& filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(mManifest);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool DongArchProject::readManifest(const QString& filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    mManifest = doc.object();
    return true;
}

bool DongArchProject::writeMeasurements(const QString& filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);

    // Write CSV header
    stream << "Type,Label,Value,Unit,Metadata\n";

    // Write measurements
    for (const Measurement& m : mMeasurements) {
        stream << m.type << ","
               << m.label << ","
               << QString::number(m.value, 'f', 3) << ","
               << m.unit << ","
               << QString(QJsonDocument(m.metadata).toJson(QJsonDocument::Compact)) << "\n";
    }

    file.close();
    return true;
}

bool DongArchProject::readMeasurements(const QString& filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);

    // Skip header
    QString header = stream.readLine();

    // Read measurements
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList parts = line.split(",");

        if (parts.size() >= 4) {
            Measurement m;
            m.type = parts[0];
            m.label = parts[1];
            m.value = parts[2].toDouble();
            m.unit = parts[3];

            if (parts.size() >= 5) {
                m.metadata = QJsonDocument::fromJson(parts[4].toUtf8()).object();
            }

            mMeasurements.append(m);
        }
    }

    file.close();
    return true;
}
