/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 6: Mesh File Explorer Manager Implementation
 */

#include "MFEManager.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QDateTime>
#include <QPainter>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_1_0>
#include <QMatrix4x4>
#include <algorithm>
#include <limits>
#include <cmath>

namespace DongArch {
namespace MFE {

//==============================================================================
// Constructor / Destructor
//==============================================================================

MFEManager::MFEManager()
{
}

MFEManager::~MFEManager()
{
}

//==============================================================================
// Directory Scanning
//==============================================================================

int MFEManager::scanDirectory(const QString& directoryPath, bool recursive)
{
    mFileList.clear();

    QDir dir(directoryPath);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << directoryPath;
        return 0;
    }

    scanDirectoryRecursive(directoryPath, recursive);

    qDebug() << "Scanned" << mFileList.size() << "mesh files in" << directoryPath;
    return static_cast<int>(mFileList.size());
}

void MFEManager::scanDirectoryRecursive(const QString& dirPath, bool recursive)
{
    QDir dir(dirPath);

    // Get supported file extensions
    QStringList filters = getSupportedExtensions();

    // Scan files in current directory
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo& fileInfo : files) {
        QString filePath = fileInfo.absoluteFilePath();
        FileFormat format = detectFormat(filePath);

        if (format != FileFormat::Unknown) {
            MeshFileInfo meshInfo = parseFileInfo(filePath, format);
            mFileList.push_back(meshInfo);
        }
    }

    // Recursively scan subdirectories
    if (recursive) {
        QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QFileInfo& subdir : subdirs) {
            scanDirectoryRecursive(subdir.absoluteFilePath(), recursive);
        }
    }
}

//==============================================================================
// File Info Retrieval
//==============================================================================

std::optional<MeshFileInfo> MFEManager::getFileInfo(const QString& filePath) const
{
    auto it = std::find_if(mFileList.begin(), mFileList.end(),
                          [&filePath](const MeshFileInfo& info) {
                              return info.filePath == filePath;
                          });

    if (it != mFileList.end()) {
        return *it;
    }

    return std::nullopt;
}

std::optional<MeshFileInfo> MFEManager::refreshFileInfo(const QString& filePath)
{
    FileFormat format = detectFormat(filePath);
    if (format == FileFormat::Unknown) {
        return std::nullopt;
    }

    MeshFileInfo newInfo = parseFileInfo(filePath, format);

    // Update existing entry or add new
    auto it = std::find_if(mFileList.begin(), mFileList.end(),
                          [&filePath](const MeshFileInfo& info) {
                              return info.filePath == filePath;
                          });

    if (it != mFileList.end()) {
        *it = newInfo;
    } else {
        mFileList.push_back(newInfo);
    }

    return newInfo;
}

//==============================================================================
// Format Detection
//==============================================================================

MFEManager::FileFormat MFEManager::detectFormat(const QString& filePath)
{
    QString ext = QFileInfo(filePath).suffix().toLower();

    if (ext == "ply") return FileFormat::PLY;
    if (ext == "obj") return FileFormat::OBJ;
    if (ext == "off") return FileFormat::OFF;
    if (ext == "stl") return FileFormat::STL;

    return FileFormat::Unknown;
}

QString MFEManager::formatToString(FileFormat format)
{
    switch (format) {
        case FileFormat::PLY: return "PLY";
        case FileFormat::OBJ: return "OBJ";
        case FileFormat::OFF: return "OFF";
        case FileFormat::STL: return "STL";
        default: return "Unknown";
    }
}

QStringList MFEManager::getSupportedExtensions()
{
    return QStringList{"*.ply", "*.obj", "*.off", "*.stl",
                       "*.PLY", "*.OBJ", "*.OFF", "*.STL"};
}

QString MFEManager::getFileFilter()
{
    return "Mesh Files (*.ply *.obj *.off *.stl);;All Files (*.*)";
}

//==============================================================================
// File Parsing
//==============================================================================

MeshFileInfo MFEManager::parseFileInfo(const QString& filePath, FileFormat format)
{
    switch (format) {
        case FileFormat::PLY: return parsePLY(filePath);
        case FileFormat::OBJ: return parseOBJ(filePath);
        case FileFormat::OFF: return parseOFF(filePath);
        case FileFormat::STL: return parseSTL(filePath);
        default: {
            MeshFileInfo info;
            info.filePath = filePath;
            info.fileName = QFileInfo(filePath).fileName();
            info.isValid = false;
            info.errorMessage = "Unsupported file format";
            return info;
        }
    }
}

MeshFileInfo MFEManager::parsePLY(const QString& filePath)
{
    MeshFileInfo info;
    info.filePath = filePath;

    QFileInfo fileInfo(filePath);
    info.fileName = fileInfo.fileName();
    info.fileExtension = "." + fileInfo.suffix().toLower();
    info.fileSize = fileInfo.size();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        info.isValid = false;
        info.errorMessage = "Failed to open file";
        return info;
    }

    QTextStream stream(&file);
    QString line;

    // Read PLY header
    line = stream.readLine();
    if (!line.startsWith("ply")) {
        info.isValid = false;
        info.errorMessage = "Not a valid PLY file";
        return info;
    }

    info.vertexCount = 0;
    info.faceCount = 0;

    // Parse header
    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();

        if (line.startsWith("element vertex")) {
            QStringList parts = line.split(QRegularExpression("\\s+"));
            if (parts.size() >= 3) {
                info.vertexCount = parts[2].toInt();
            }
        }
        else if (line.startsWith("element face")) {
            QStringList parts = line.split(QRegularExpression("\\s+"));
            if (parts.size() >= 3) {
                info.faceCount = parts[2].toInt();
            }
        }
        else if (line == "end_header") {
            break;
        }
    }

    file.close();

    // Calculate bounding box (simplified - just use vertex count as proxy)
    info.boundingBoxMin = QVector3D(0, 0, 0);
    info.boundingBoxMax = QVector3D(100, 100, 100);
    info.center = QVector3D(50, 50, 50);

    info.isValid = (info.vertexCount > 0 && info.faceCount > 0);
    if (!info.isValid) {
        info.errorMessage = "Invalid mesh data";
    }

    return info;
}

MeshFileInfo MFEManager::parseOBJ(const QString& filePath)
{
    MeshFileInfo info;
    info.filePath = filePath;

    QFileInfo fileInfo(filePath);
    info.fileName = fileInfo.fileName();
    info.fileExtension = "." + fileInfo.suffix().toLower();
    info.fileSize = fileInfo.size();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        info.isValid = false;
        info.errorMessage = "Failed to open file";
        return info;
    }

    QTextStream stream(&file);
    info.vertexCount = 0;
    info.faceCount = 0;

    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();

        if (line.startsWith("v ")) {
            info.vertexCount++;
        }
        else if (line.startsWith("f ")) {
            info.faceCount++;
        }
    }

    file.close();

    info.boundingBoxMin = QVector3D(0, 0, 0);
    info.boundingBoxMax = QVector3D(100, 100, 100);
    info.center = QVector3D(50, 50, 50);

    info.isValid = (info.vertexCount > 0 && info.faceCount > 0);
    if (!info.isValid) {
        info.errorMessage = "Invalid mesh data";
    }

    return info;
}

MeshFileInfo MFEManager::parseOFF(const QString& filePath)
{
    MeshFileInfo info;
    info.filePath = filePath;

    QFileInfo fileInfo(filePath);
    info.fileName = fileInfo.fileName();
    info.fileExtension = "." + fileInfo.suffix().toLower();
    info.fileSize = fileInfo.size();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        info.isValid = false;
        info.errorMessage = "Failed to open file";
        return info;
    }

    QTextStream stream(&file);
    QString line = stream.readLine().trimmed();

    if (!line.startsWith("OFF")) {
        info.isValid = false;
        info.errorMessage = "Not a valid OFF file";
        return info;
    }

    // Read vertex and face count
    line = stream.readLine().trimmed();
    QStringList parts = line.split(QRegularExpression("\\s+"));

    if (parts.size() >= 2) {
        info.vertexCount = parts[0].toInt();
        info.faceCount = parts[1].toInt();
    }

    file.close();

    info.boundingBoxMin = QVector3D(0, 0, 0);
    info.boundingBoxMax = QVector3D(100, 100, 100);
    info.center = QVector3D(50, 50, 50);

    info.isValid = (info.vertexCount > 0 && info.faceCount > 0);
    if (!info.isValid) {
        info.errorMessage = "Invalid mesh data";
    }

    return info;
}

MeshFileInfo MFEManager::parseSTL(const QString& filePath)
{
    MeshFileInfo info;
    info.filePath = filePath;

    QFileInfo fileInfo(filePath);
    info.fileName = fileInfo.fileName();
    info.fileExtension = "." + fileInfo.suffix().toLower();
    info.fileSize = fileInfo.size();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.isValid = false;
        info.errorMessage = "Failed to open file";
        return info;
    }

    // Check if binary or ASCII
    QByteArray firstBytes = file.read(80);
    file.seek(0);

    if (firstBytes.startsWith("solid")) {
        // ASCII STL
        QTextStream stream(&file);
        info.faceCount = 0;

        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (line.startsWith("facet")) {
                info.faceCount++;
            }
        }

        info.vertexCount = info.faceCount * 3;  // Each face has 3 vertices
    }
    else {
        // Binary STL
        file.seek(80);  // Skip header
        char buffer[4];
        if (file.read(buffer, 4) == 4) {
            quint32 faceCount;
            memcpy(&faceCount, buffer, 4);
            info.faceCount = static_cast<int>(faceCount);
            info.vertexCount = info.faceCount * 3;
        }
    }

    file.close();

    info.boundingBoxMin = QVector3D(0, 0, 0);
    info.boundingBoxMax = QVector3D(100, 100, 100);
    info.center = QVector3D(50, 50, 50);

    info.isValid = (info.vertexCount > 0 && info.faceCount > 0);
    if (!info.isValid) {
        info.errorMessage = "Invalid mesh data";
    }

    return info;
}

//==============================================================================
// Thumbnail Generation
//==============================================================================

QImage MFEManager::generateThumbnail(const QString& filePath, int size)
{
    // 1. Check cache first
    QImage cached = loadCachedThumbnail(filePath);
    if (!cached.isNull()) {
        qDebug() << "Loaded thumbnail from cache:" << filePath;
        return cached;
    }

    // 2. Detect file format
    FileFormat format = detectFormat(filePath);
    if (format == FileFormat::Unknown) {
        qWarning() << "Unknown file format for thumbnail:" << filePath;
        return createPlaceholderThumbnail(size, "?");
    }

    // 3. Load mesh (simplified)
    SimpleMesh mesh;
    try {
        mesh = loadSimpleMesh(filePath, format);
    } catch (const std::exception& e) {
        qWarning() << "Failed to load mesh for thumbnail:" << e.what();
        return createPlaceholderThumbnail(size, "Error");
    }

    // 4. Check if mesh is valid
    if (mesh.vertices.empty() || mesh.indices.empty()) {
        qWarning() << "Empty mesh for thumbnail:" << filePath;
        return createPlaceholderThumbnail(size, "Empty");
    }

    // 5. Render mesh to image
    QImage thumbnail = renderMeshToImage(mesh, size);

    // 6. Save to cache
    saveThumbnailToCache(filePath, thumbnail);

    return thumbnail;
}

QString MFEManager::getThumbnailCachePath(const QString& filePath)
{
    // Get cache directory
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    cacheDir += "/dongarch3d/thumbnails";

    QDir dir(cacheDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Create hash from file path + modification time
    QFileInfo fileInfo(filePath);
    QString hashInput = filePath + QString::number(fileInfo.lastModified().toMSecsSinceEpoch());
    QByteArray hash = QCryptographicHash::hash(hashInput.toUtf8(), QCryptographicHash::Md5);
    QString hashStr = hash.toHex();

    return cacheDir + "/" + hashStr + ".png";
}

QImage MFEManager::loadCachedThumbnail(const QString& filePath)
{
    QString cachePath = getThumbnailCachePath(filePath);
    QFileInfo cacheInfo(cachePath);

    if (!cacheInfo.exists()) {
        return QImage();  // Null image
    }

    // Check if cache is still valid (file not modified after cache)
    QFileInfo originalInfo(filePath);
    if (originalInfo.lastModified() > cacheInfo.lastModified()) {
        // Original file modified, cache is stale
        QFile::remove(cachePath);
        return QImage();
    }

    QImage thumbnail;
    if (!thumbnail.load(cachePath)) {
        qWarning() << "Failed to load cached thumbnail:" << cachePath;
        return QImage();
    }

    return thumbnail;
}

bool MFEManager::saveThumbnailToCache(const QString& filePath, const QImage& thumbnail)
{
    QString cachePath = getThumbnailCachePath(filePath);

    if (!thumbnail.save(cachePath, "PNG")) {
        qWarning() << "Failed to save thumbnail to cache:" << cachePath;
        return false;
    }

    qDebug() << "Saved thumbnail to cache:" << cachePath;
    return true;
}

QImage MFEManager::createPlaceholderThumbnail(int size, const QString& text)
{
    QImage thumbnail(size, size, QImage::Format_RGB888);
    thumbnail.fill(QColor(48, 48, 48));

    QPainter painter(&thumbnail);
    painter.setPen(QColor(128, 128, 128));
    painter.setFont(QFont("Arial", 16, QFont::Bold));
    painter.drawText(thumbnail.rect(), Qt::AlignCenter, text);

    return thumbnail;
}

//==============================================================================
// Mesh Loading (Simplified for Thumbnails)
//==============================================================================

MFEManager::SimpleMesh MFEManager::loadSimpleMesh(const QString& filePath, FileFormat format)
{
    SimpleMesh mesh;

    // Initialize bounding box
    mesh.minX = mesh.minY = mesh.minZ = std::numeric_limits<float>::max();
    mesh.maxX = mesh.maxY = mesh.maxZ = std::numeric_limits<float>::lowest();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Failed to open file");
    }

    QTextStream stream(&file);

    if (format == FileFormat::PLY) {
        // Parse PLY header
        QString line = stream.readLine();
        if (!line.startsWith("ply")) {
            throw std::runtime_error("Not a valid PLY file");
        }

        int vertexCount = 0;
        int faceCount = 0;
        bool isBinary = false;

        while (!stream.atEnd()) {
            line = stream.readLine().trimmed();

            if (line.startsWith("format binary")) {
                isBinary = true;
            }
            else if (line.startsWith("element vertex")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (parts.size() >= 3) {
                    vertexCount = parts[2].toInt();
                }
            }
            else if (line.startsWith("element face")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (parts.size() >= 3) {
                    faceCount = parts[2].toInt();
                }
            }
            else if (line == "end_header") {
                break;
            }
        }

        if (isBinary) {
            // For simplicity, skip binary PLY files for now
            qWarning() << "Binary PLY not supported for thumbnails (use ASCII)";
            throw std::runtime_error("Binary PLY not supported");
        }

        // Read vertices (ASCII)
        mesh.vertices.reserve(vertexCount * 3);
        for (int i = 0; i < vertexCount && !stream.atEnd(); ++i) {
            line = stream.readLine().trimmed();
            QStringList parts = line.split(QRegularExpression("\\s+"));

            if (parts.size() >= 3) {
                float x = parts[0].toFloat();
                float y = parts[1].toFloat();
                float z = parts[2].toFloat();

                mesh.vertices.push_back(x);
                mesh.vertices.push_back(y);
                mesh.vertices.push_back(z);

                // Update bounding box
                mesh.minX = std::min(mesh.minX, x);
                mesh.minY = std::min(mesh.minY, y);
                mesh.minZ = std::min(mesh.minZ, z);
                mesh.maxX = std::max(mesh.maxX, x);
                mesh.maxY = std::max(mesh.maxY, y);
                mesh.maxZ = std::max(mesh.maxZ, z);
            }
        }

        // Read faces
        mesh.indices.reserve(faceCount * 3);
        for (int i = 0; i < faceCount && !stream.atEnd(); ++i) {
            line = stream.readLine().trimmed();
            QStringList parts = line.split(QRegularExpression("\\s+"));

            if (parts.size() >= 4) {
                int numVerts = parts[0].toInt();
                if (numVerts == 3) {
                    mesh.indices.push_back(parts[1].toUInt());
                    mesh.indices.push_back(parts[2].toUInt());
                    mesh.indices.push_back(parts[3].toUInt());
                }
                // For quads or polygons, just take first 3 vertices (simplified)
                else if (numVerts > 3) {
                    mesh.indices.push_back(parts[1].toUInt());
                    mesh.indices.push_back(parts[2].toUInt());
                    mesh.indices.push_back(parts[3].toUInt());
                }
            }
        }
    }
    else if (format == FileFormat::OBJ) {
        // Parse OBJ (simplified - vertices and faces only)
        std::vector<float> tmpVertices;

        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();

            if (line.startsWith("v ")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (parts.size() >= 4) {
                    float x = parts[1].toFloat();
                    float y = parts[2].toFloat();
                    float z = parts[3].toFloat();

                    tmpVertices.push_back(x);
                    tmpVertices.push_back(y);
                    tmpVertices.push_back(z);

                    // Update bounding box
                    mesh.minX = std::min(mesh.minX, x);
                    mesh.minY = std::min(mesh.minY, y);
                    mesh.minZ = std::min(mesh.minZ, z);
                    mesh.maxX = std::max(mesh.maxX, x);
                    mesh.maxY = std::max(mesh.maxY, y);
                    mesh.maxZ = std::max(mesh.maxZ, z);
                }
            }
            else if (line.startsWith("f ")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (parts.size() >= 4) {
                    // Parse face indices (handle "v", "v/vt", "v/vt/vn" formats)
                    auto parseIndex = [](const QString& str) -> unsigned int {
                        QString indexStr = str.split('/').first();
                        return indexStr.toUInt() - 1;  // OBJ indices are 1-based
                    };

                    unsigned int i1 = parseIndex(parts[1]);
                    unsigned int i2 = parseIndex(parts[2]);
                    unsigned int i3 = parseIndex(parts[3]);

                    mesh.indices.push_back(i1);
                    mesh.indices.push_back(i2);
                    mesh.indices.push_back(i3);

                    // For quads, add second triangle
                    if (parts.size() >= 5) {
                        unsigned int i4 = parseIndex(parts[4]);
                        mesh.indices.push_back(i1);
                        mesh.indices.push_back(i3);
                        mesh.indices.push_back(i4);
                    }
                }
            }
        }

        mesh.vertices = tmpVertices;
    }
    else {
        throw std::runtime_error("Unsupported format for thumbnail");
    }

    file.close();

    qDebug() << "Loaded mesh:" << mesh.vertices.size() / 3 << "vertices,"
             << mesh.indices.size() / 3 << "faces";

    return mesh;
}

//==============================================================================
// Mesh Rendering (Offscreen)
//==============================================================================

QImage MFEManager::renderMeshToImage(const SimpleMesh& mesh, int size)
{
    // Create offscreen OpenGL context
    QOpenGLContext context;
    if (!context.create()) {
        qWarning() << "Failed to create OpenGL context";
        return createPlaceholderThumbnail(size, "GL Error");
    }

    QOffscreenSurface surface;
    surface.setFormat(context.format());
    surface.create();

    if (!context.makeCurrent(&surface)) {
        qWarning() << "Failed to make OpenGL context current";
        return createPlaceholderThumbnail(size, "GL Error");
    }

    // Create FBO
    QOpenGLFramebufferObject fbo(size, size, QOpenGLFramebufferObject::Depth);
    if (!fbo.bind()) {
        qWarning() << "Failed to bind FBO";
        return createPlaceholderThumbnail(size, "FBO Error");
    }

    QOpenGLFunctions* gl = context.functions();
    QOpenGLFunctions_1_0* gl1 = context.versionFunctions<QOpenGLFunctions_1_0>();
    if (!gl1) {
        qWarning() << "OpenGL 1.0 functions not available";
        return createPlaceholderThumbnail(size, "GL 1.0 N/A");
    }

    // Setup OpenGL state
    gl->glViewport(0, 0, size, size);
    gl->glClearColor(0.95f, 0.95f, 0.95f, 1.0f);  // Light gray background
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl->glEnable(GL_DEPTH_TEST);
    gl->glEnable(GL_CULL_FACE);

    // Calculate mesh center and scale
    float centerX = (mesh.minX + mesh.maxX) * 0.5f;
    float centerY = (mesh.minY + mesh.maxY) * 0.5f;
    float centerZ = (mesh.minZ + mesh.maxZ) * 0.5f;

    float sizeX = mesh.maxX - mesh.minX;
    float sizeY = mesh.maxY - mesh.minY;
    float sizeZ = mesh.maxZ - mesh.minZ;
    float maxSize = std::max({sizeX, sizeY, sizeZ});
    float scale = 1.8f / maxSize;  // Scale to fit in view

    // Setup matrices
    QMatrix4x4 projection;
    projection.perspective(45.0f, 1.0f, 0.1f, 100.0f);

    QMatrix4x4 view;
    view.lookAt(QVector3D(2, 2, 2), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

    QMatrix4x4 model;
    model.translate(-centerX, -centerY, -centerZ);
    model.scale(scale);

    QMatrix4x4 mvp = projection * view * model;

    // Use simple fixed-function rendering (compatible with older OpenGL)
    gl1->glMatrixMode(GL_PROJECTION);
    gl1->glLoadIdentity();
    gl1->glMultMatrixf(projection.constData());

    gl1->glMatrixMode(GL_MODELVIEW);
    gl1->glLoadIdentity();
    gl1->glMultMatrixf((view * model).constData());

    // Setup lighting
    gl->glEnable(GL_LIGHTING);
    gl->glEnable(GL_LIGHT0);
    GLfloat lightPos[] = {2.0f, 2.0f, 2.0f, 0.0f};
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    gl1->glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    gl1->glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    gl1->glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    // Material
    GLfloat matDiffuse[] = {0.6f, 0.6f, 0.7f, 1.0f};
    GLfloat matAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    gl1->glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    gl1->glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);

    // Render mesh
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, mesh.vertices.data());

    // Enable auto-normal generation (simple approximation)
    gl->glEnable(GL_NORMALIZE);

    // Draw triangles
    gl->glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, mesh.indices.data());

    glDisableClientState(GL_VERTEX_ARRAY);
    gl->glDisable(GL_LIGHTING);
    gl->glDisable(GL_DEPTH_TEST);

    // Read pixels
    QImage image = fbo.toImage();

    fbo.release();
    context.doneCurrent();

    return image;
}

//==============================================================================
// Utilities
//==============================================================================

void MFEManager::clear()
{
    mFileList.clear();
}

} // namespace MFE
} // namespace DongArch
