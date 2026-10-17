# Phase 6 MFE Thumbnail Generation - Code Reference

**Quick reference for the thumbnail generation implementation**

---

## 1. Thumbnail Generation (MFEManager.cpp)

### Main Entry Point

```cpp
QImage MFEManager::generateThumbnail(const QString& filePath, int size)
{
    // 1. Check cache first
    QImage cached = loadCachedThumbnail(filePath);
    if (!cached.isNull()) {
        return cached;
    }

    // 2. Detect file format
    FileFormat format = detectFormat(filePath);
    if (format == FileFormat::Unknown) {
        return createPlaceholderThumbnail(size, "?");
    }

    // 3. Load mesh (simplified)
    SimpleMesh mesh;
    try {
        mesh = loadSimpleMesh(filePath, format);
    } catch (const std::exception& e) {
        return createPlaceholderThumbnail(size, "Error");
    }

    // 4. Render mesh to image
    QImage thumbnail = renderMeshToImage(mesh, size);

    // 5. Save to cache
    saveThumbnailToCache(filePath, thumbnail);

    return thumbnail;
}
```

---

## 2. Simplified Mesh Loading

### PLY Parser

```cpp
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

        // Read header
        while (!stream.atEnd()) {
            line = stream.readLine().trimmed();

            if (line.startsWith("element vertex")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                vertexCount = parts[2].toInt();
            }
            else if (line.startsWith("element face")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                faceCount = parts[2].toInt();
            }
            else if (line == "end_header") {
                break;
            }
        }

        // Read vertices
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
            }
        }
    }

    file.close();
    return mesh;
}
```

---

## 3. Offscreen OpenGL Rendering

```cpp
QImage MFEManager::renderMeshToImage(const SimpleMesh& mesh, int size)
{
    // Create offscreen OpenGL context
    QOpenGLContext context;
    if (!context.create()) {
        return createPlaceholderThumbnail(size, "GL Error");
    }

    QOffscreenSurface surface;
    surface.setFormat(context.format());
    surface.create();

    if (!context.makeCurrent(&surface)) {
        return createPlaceholderThumbnail(size, "GL Error");
    }

    // Create FBO
    QOpenGLFramebufferObject fbo(size, size, QOpenGLFramebufferObject::Depth);
    if (!fbo.bind()) {
        return createPlaceholderThumbnail(size, "FBO Error");
    }

    QOpenGLFunctions* gl = context.functions();

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
    float scale = 1.8f / maxSize;

    // Setup matrices
    QMatrix4x4 projection;
    projection.perspective(45.0f, 1.0f, 0.1f, 100.0f);

    QMatrix4x4 view;
    view.lookAt(QVector3D(2, 2, 2), QVector3D(0, 0, 0), QVector3D(0, 1, 0));

    QMatrix4x4 model;
    model.translate(-centerX, -centerY, -centerZ);
    model.scale(scale);

    // Use fixed-function pipeline
    gl->glMatrixMode(GL_PROJECTION);
    gl->glLoadIdentity();
    gl->glMultMatrixf(projection.constData());

    gl->glMatrixMode(GL_MODELVIEW);
    gl->glLoadIdentity();
    gl->glMultMatrixf((view * model).constData());

    // Setup lighting
    gl->glEnable(GL_LIGHTING);
    gl->glEnable(GL_LIGHT0);
    GLfloat lightPos[] = {2.0f, 2.0f, 2.0f, 0.0f};
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    gl->glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    gl->glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    gl->glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    // Material
    GLfloat matDiffuse[] = {0.6f, 0.6f, 0.7f, 1.0f};
    GLfloat matAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    gl->glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    gl->glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);

    // Render mesh
    gl->glEnableClientState(GL_VERTEX_ARRAY);
    gl->glVertexPointer(3, GL_FLOAT, 0, mesh.vertices.data());
    gl->glEnable(GL_NORMALIZE);
    gl->glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, mesh.indices.data());
    gl->glDisableClientState(GL_VERTEX_ARRAY);

    // Cleanup
    gl->glDisable(GL_LIGHTING);
    gl->glDisable(GL_DEPTH_TEST);

    // Read pixels
    QImage image = fbo.toImage();

    fbo.release();
    context.doneCurrent();

    return image;
}
```

---

## 4. Caching System

### Cache Path Generation

```cpp
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
```

### Load from Cache

```cpp
QImage MFEManager::loadCachedThumbnail(const QString& filePath)
{
    QString cachePath = getThumbnailCachePath(filePath);
    QFileInfo cacheInfo(cachePath);

    if (!cacheInfo.exists()) {
        return QImage();  // Null image
    }

    // Check if cache is still valid
    QFileInfo originalInfo(filePath);
    if (originalInfo.lastModified() > cacheInfo.lastModified()) {
        // Original file modified, cache is stale
        QFile::remove(cachePath);
        return QImage();
    }

    QImage thumbnail;
    if (!thumbnail.load(cachePath)) {
        return QImage();
    }

    return thumbnail;
}
```

### Save to Cache

```cpp
bool MFEManager::saveThumbnailToCache(const QString& filePath, const QImage& thumbnail)
{
    QString cachePath = getThumbnailCachePath(filePath);

    if (!thumbnail.save(cachePath, "PNG")) {
        qWarning() << "Failed to save thumbnail to cache:" << cachePath;
        return false;
    }

    return true;
}
```

---

## 5. Asynchronous Loading (MFEWidget.cpp)

### Start Async Loading

```cpp
void MFEWidget::loadThumbnailAsync(const QString& filePath)
{
    // Cancel any pending thumbnail generation
    if (mThumbnailWatcher->isRunning()) {
        mThumbnailWatcher->cancel();
        mThumbnailWatcher->waitForFinished();
    }

    // Show loading placeholder
    QImage loadingImage(128, 128, QImage::Format_RGB888);
    loadingImage.fill(QColor(64, 64, 64));
    QPainter painter(&loadingImage);
    painter.setPen(QColor(192, 192, 192));
    painter.setFont(QFont("Arial", 10));
    painter.drawText(loadingImage.rect(), Qt::AlignCenter, "Loading...");
    updateThumbnail(loadingImage);

    // Store path for later
    mThumbnailLoadingPath = filePath;

    // Start async thumbnail generation
    QFuture<QImage> future = QtConcurrent::run([filePath]() {
        return MFEManager::generateThumbnail(filePath, 128);
    });

    mThumbnailWatcher->setFuture(future);
}
```

### Handle Completion

```cpp
void MFEWidget::onThumbnailReady()
{
    // Check if the thumbnail is still relevant
    if (mThumbnailLoadingPath != mSelectedFilePath) {
        return;  // User selected different file
    }

    QImage thumbnail = mThumbnailWatcher->result();
    updateThumbnail(thumbnail);
}
```

---

## 6. Data Structures

### SimpleMesh

```cpp
struct SimpleMesh {
    std::vector<float> vertices;          // x,y,z triplets
    std::vector<unsigned int> indices;    // Triangle indices
    float minX, minY, minZ;               // Bounding box min
    float maxX, maxY, maxZ;               // Bounding box max
};
```

### MeshFileInfo (existing)

```cpp
struct MeshFileInfo
{
    QString filePath;
    QString fileName;
    QString fileExtension;
    qint64 fileSize;

    int vertexCount;
    int faceCount;
    QVector3D boundingBoxMin;
    QVector3D boundingBoxMax;
    QVector3D center;

    QImage thumbnail;  // Now populated by async loading

    bool isValid;
    QString errorMessage;
};
```

---

## 7. Header Declarations

### MFEManager.h

```cpp
class MFEManager
{
public:
    // Thumbnail generation
    static QImage generateThumbnail(const QString& filePath, int size = 128);
    static QString getThumbnailCachePath(const QString& filePath);
    static QImage loadCachedThumbnail(const QString& filePath);
    static bool saveThumbnailToCache(const QString& filePath, const QImage& thumbnail);

private:
    struct SimpleMesh {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };

    static SimpleMesh loadSimpleMesh(const QString& filePath, FileFormat format);
    static QImage renderMeshToImage(const SimpleMesh& mesh, int size);
    static QImage createPlaceholderThumbnail(int size, const QString& text);

    std::vector<MeshFileInfo> mFileList;
};
```

### MFEWidget.h

```cpp
class MFEWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MFEWidget(QWidget *parent = nullptr);
    ~MFEWidget();

    void setDirectory(const QString& directoryPath, bool recursive = true);
    QString getSelectedFilePath() const;
    void refresh();

signals:
    void fileSelected(const QString& filePath);
    void fileDoubleClicked(const QString& filePath);
    void fileListChanged(int fileCount);

private:
    void loadThumbnailAsync(const QString& filePath);
    void onThumbnailReady();

    std::unique_ptr<MFEManager> mManager;
    QFutureWatcher<QImage>* mThumbnailWatcher;
    QString mThumbnailLoadingPath;
};
```

---

## 8. Required Includes

### MFEManager.cpp

```cpp
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
#include <QMatrix4x4>
#include <algorithm>
#include <limits>
#include <cmath>
```

### MFEWidget.cpp

```cpp
#include <QFileDialog>
#include <QStandardItem>
#include <QPixmap>
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
```

---

## 9. Usage Examples

### Simple Thumbnail Generation

```cpp
// Generate thumbnail for a single file
QImage thumb = MFEManager::generateThumbnail("/path/to/mesh.ply");

// Display in QLabel
QLabel* label = new QLabel();
label->setPixmap(QPixmap::fromImage(thumb));
```

### Batch Thumbnail Generation

```cpp
QStringList files = {"/path/mesh1.ply", "/path/mesh2.obj", "/path/mesh3.ply"};

for (const QString& file : files) {
    QImage thumb = MFEManager::generateThumbnail(file);
    // Process thumbnail...
}
```

### Using MFEWidget

```cpp
// Create widget
MFEWidget* mfeWidget = new MFEWidget(parentWidget);

// Set directory to scan
mfeWidget->setDirectory("/path/to/meshes", true);  // recursive=true

// Connect signals
connect(mfeWidget, &MFEWidget::fileDoubleClicked, [](const QString& path) {
    qDebug() << "User wants to open:" << path;
    // Load mesh in main viewport
});

connect(mfeWidget, &MFEWidget::fileSelected, [](const QString& path) {
    qDebug() << "Selected:" << path;
    // Update properties panel
});
```

---

## 10. Error Handling Patterns

```cpp
// Pattern 1: Try-catch for mesh loading
try {
    SimpleMesh mesh = loadSimpleMesh(filePath, format);
    // Process mesh...
} catch (const std::exception& e) {
    qWarning() << "Mesh loading failed:" << e.what();
    return createPlaceholderThumbnail(size, "Error");
}

// Pattern 2: Check OpenGL context
if (!context.create()) {
    qWarning() << "Failed to create OpenGL context";
    return createPlaceholderThumbnail(size, "GL Error");
}

// Pattern 3: Validate cache
QImage cached = loadCachedThumbnail(filePath);
if (cached.isNull()) {
    // Generate new thumbnail
}
```

---

## 11. Performance Tips

### Optimize for Large Directories

```cpp
// Don't generate all thumbnails at once
// Use lazy loading (generate on-demand when file selected)

void MFEWidget::onTreeViewSelectionChanged()
{
    QString filePath = getSelectedFilePath();

    // Only generate thumbnail for selected file
    loadThumbnailAsync(filePath);  // Async, non-blocking
}
```

### Cache Warming (Optional)

```cpp
// Pre-generate thumbnails in background
void MFEWidget::warmCache()
{
    const auto& files = mManager->getFileList();

    // Use QtConcurrent::mapped for parallel generation
    QtConcurrent::map(files, [](const MeshFileInfo& info) {
        MFEManager::generateThumbnail(info.filePath);
    });
}
```

---

## 12. Debugging

### Enable Verbose Logging

```cpp
// In MFEManager.cpp
qDebug() << "Loaded mesh:" << mesh.vertices.size() / 3 << "vertices,"
         << mesh.indices.size() / 3 << "faces";

qDebug() << "Loaded thumbnail from cache:" << filePath;
qDebug() << "Saved thumbnail to cache:" << cachePath;
```

### Check Cache Contents

```bash
# Linux/macOS
ls -lh ~/.cache/dongarch3d/thumbnails/

# Windows
dir %LOCALAPPDATA%\dongarch3d\cache\thumbnails\
```

### Verify OpenGL Context

```cpp
QOpenGLContext* ctx = QOpenGLContext::currentContext();
if (ctx) {
    qDebug() << "OpenGL version:" << ctx->format().version();
} else {
    qWarning() << "No OpenGL context!";
}
```

---

## 13. Common Pitfalls

### ❌ Don't block UI thread

```cpp
// BAD - Freezes UI
QImage thumb = MFEManager::generateThumbnail(filePath);
updateUI(thumb);
```

```cpp
// GOOD - Async loading
loadThumbnailAsync(filePath);  // Returns immediately
// UI updates when ready via onThumbnailReady()
```

### ❌ Don't forget to cleanup

```cpp
// BAD - Memory leak
QFutureWatcher<QImage>* watcher = new QFutureWatcher<QImage>();
// ... never deleted
```

```cpp
// GOOD - Parent-managed
QFutureWatcher<QImage>* watcher = new QFutureWatcher<QImage>(this);
// Deleted when parent (this) is deleted
```

### ❌ Don't assume format support

```cpp
// BAD - Assumes all PLY files are ASCII
SimpleMesh mesh = loadSimpleMesh(filePath, FileFormat::PLY);
```

```cpp
// GOOD - Check for binary and handle gracefully
try {
    SimpleMesh mesh = loadSimpleMesh(filePath, FileFormat::PLY);
} catch (const std::exception& e) {
    return createPlaceholderThumbnail(size, "Binary PLY");
}
```

---

**Document Version**: 1.0.0
**Last Updated**: 2025-11-10
