/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 */

#include "OutlineExtractor.h"
#include "algorithms/QuadricOptimizer.h"
#include "algorithms/VectorTracer.h"
#include "dongarch/common/DongArchMath.h"
#include "dongarch/illustrator/SVGExporter.h"
#include <QFile>
#include <QTextStream>
#include <QElapsedTimer>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <limits>

namespace DongArch {
namespace Outline {

OutlineExtractor::OutlineExtractor(Mesh* mesh)
    : mMesh(mesh)
    , mSilhouetteDetector(nullptr)
    , mGLInitialized(false)
    , mFBOTextureID(0)
    , mFBOWidth(1024)
    , mFBOHeight(768)
    , mVAO(0)
    , mVBO(0)
    , mEBO(0)
{
    if (mMesh) {
        mSilhouetteDetector = std::make_unique<SilhouetteDetector>(mMesh);
    }
}

OutlineExtractor::~OutlineExtractor() {
    if (mGLInitialized) {
        destroyFBO();

        if (mVAO != 0) {
            glDeleteVertexArrays(1, &mVAO);
        }
        if (mVBO != 0) {
            glDeleteBuffers(1, &mVBO);
        }
        if (mEBO != 0) {
            glDeleteBuffers(1, &mEBO);
        }
    }
}

bool OutlineExtractor::initializeGL() {
    if (mGLInitialized) {
        return true;
    }

    // OpenGL 함수 초기화
    if (!initializeOpenGLFunctions()) {
        qWarning() << "OutlineExtractor: Failed to initialize OpenGL functions";
        return false;
    }

    // 셰이더 컴파일
    if (!compileGeometryShader()) {
        qWarning() << "OutlineExtractor: Failed to compile Geometry Shader";
        return false;
    }

    if (!compileSobelShader()) {
        qWarning() << "OutlineExtractor: Failed to compile Sobel Shader";
        return false;
    }

    // FBO 생성 (기본 크기)
    if (!createFBO(mFBOWidth, mFBOHeight, true, 4)) {
        qWarning() << "OutlineExtractor: Failed to create FBO";
        return false;
    }

    mGLInitialized = true;
    return true;
}

bool OutlineExtractor::extractCPU(ViewDirection viewDir, OutlineResult& result) {
    if (!mSilhouetteDetector || !mSilhouetteDetector->isValid()) {
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    SilhouetteResult silhouetteResult;
    bool success = mSilhouetteDetector->detectSilhouette(viewDir, silhouetteResult);

    if (!success) {
        return false;
    }

    // 결과 복사
    result.method = OutlineMethod::CPU_SILHOUETTE;
    result.edges = silhouetteResult.edges;
    result.totalEdges = silhouetteResult.totalEdges;
    result.outlineEdges = silhouetteResult.silhouetteEdges;
    result.processingTime = timer.elapsed();
    result.usedFBO = false;

    return true;
}

bool OutlineExtractor::extractGPU(const Vector3D& viewVector, OutlineResult& result) {
    if (!mGLInitialized) {
        qWarning() << "OutlineExtractor: OpenGL not initialized";
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    // Dual-pass 파라미터 설정
    DualPassParams params;
    params.viewVector = viewVector;
    params.useGeometryShader = true;
    params.applySobel = false;  // Geometry Shader만

    bool success = renderSilhouetteToFBO(params);

    if (!success) {
        return false;
    }

    result.method = OutlineMethod::GPU_GEOMETRY_SHADER;
    result.processingTime = timer.elapsed();
    result.usedFBO = true;
    result.fboTextureID = mFBOTextureID;

    return true;
}

bool OutlineExtractor::extractSobel(float threshold, OutlineResult& result) {
    if (!mGLInitialized) {
        qWarning() << "OutlineExtractor: OpenGL not initialized";
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    bool success = applySobelFilter(threshold, result);

    if (!success) {
        return false;
    }

    result.method = OutlineMethod::SOBEL_FILTER;
    result.sobelThreshold = threshold;
    result.processingTime = timer.elapsed();

    return true;
}

bool OutlineExtractor::extractDualPass(const DualPassParams& params, OutlineResult& result) {
    if (!mGLInitialized) {
        qWarning() << "OutlineExtractor: OpenGL not initialized";
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    // Pass 1: Geometry Shader → FBO
    if (params.useGeometryShader) {
        bool success = renderSilhouetteToFBO(params);
        if (!success) {
            return false;
        }
    }

    // Pass 2: Sobel Filter
    if (params.applySobel) {
        bool success = applySobelFilter(params.sobelThreshold, result);
        if (!success) {
            return false;
        }
    }

    result.method = OutlineMethod::DUAL_PASS;
    result.processingTime = timer.elapsed();
    result.usedFBO = true;
    result.fboTextureID = mFBOTextureID;

    return true;
}

bool OutlineExtractor::extractHybrid(const Vector3D& viewVector, OutlineResult& result) {
    if (!mMesh) {
        return false;
    }

    // 메시 크기에 따라 CPU/GPU 선택
    // Edge 개수 대신 Face 개수 사용 (Edge ≈ Face * 3)
    size_t faceCount = mMesh->getFaceNr();
    size_t estimatedEdgeCount = faceCount * 3;

    // 임계값: 1000개 엣지 (≈ 333 faces)
    // - 작은 메시 (<1000): CPU
    // - 큰 메시 (>=1000): GPU
    const size_t HYBRID_THRESHOLD = 1000;

    if (estimatedEdgeCount < HYBRID_THRESHOLD) {
        // CPU Silhouette
        ViewDirection viewDir = ViewDirection::FRONT;  // viewVector에서 변환 필요
        return extractCPU(viewDir, result);
    } else {
        // GPU Geometry Shader + Sobel
        DualPassParams params;
        params.viewVector = viewVector;
        params.useGeometryShader = true;
        params.applySobel = true;
        params.sobelThreshold = 0.5f;

        return extractDualPass(params, result);
    }
}

// ============================================================================
// Private Methods: Dual-pass Rendering
// ============================================================================

bool OutlineExtractor::renderSilhouetteToFBO(const DualPassParams& params) {
    if (!mFBO || !mGeometryShaderProgram) {
        return false;
    }

    // FBO 바인딩
    mFBO->bind();

    // Clear
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Geometry Shader 프로그램 활성화
    mGeometryShaderProgram->bind();

    // Uniforms 설정
    mGeometryShaderProgram->setUniformValue("cameraPosition",
        static_cast<float>(params.cameraPosition.getX()),
        static_cast<float>(params.cameraPosition.getY()),
        static_cast<float>(params.cameraPosition.getZ()));

    mGeometryShaderProgram->setUniformValue("useViewDirection", true);

    // TODO: MVP 행렬 설정
    // TODO: 메시 데이터 VAO/VBO 바인딩 및 렌더링

    // 간단한 구현 (실제로는 메시 데이터 업로드 필요)
    // 여기서는 FBO만 설정

    mGeometryShaderProgram->release();
    mFBO->release();

    return true;
}

bool OutlineExtractor::applySobelFilter(float threshold, OutlineResult& result) {
    if (!mFBO || !mSobelShaderProgram) {
        return false;
    }

    // Sobel Shader 활성화
    mSobelShaderProgram->bind();

    // FBO 텍스처 바인딩
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mFBO->texture());

    // Uniforms 설정
    mSobelShaderProgram->setUniformValue("uFBO_Texture_ID", 0);
    mSobelShaderProgram->setUniformValue("uViewPortSize",
        static_cast<float>(mFBOWidth),
        static_cast<float>(mFBOHeight));

    // TODO: Full-screen quad 렌더링
    // TODO: Sobel 결과 분석 및 엣지 추출

    mSobelShaderProgram->release();

    // FBO에서 엣지 추출
    size_t edgeCount = extractEdgesFromFBO(result);
    result.outlineEdges = edgeCount;

    return true;
}

size_t OutlineExtractor::extractEdgesFromFBO(OutlineResult& result) {
    // TODO: FBO 텍스처에서 픽셀 데이터 읽기
    // TODO: Sobel 결과 분석 (threshold 이상인 픽셀 찾기)
    // TODO: 엣지 점 추출

    // 현재는 더미 구현
    return 0;
}

// ============================================================================
// Private Methods: FBO Management
// ============================================================================

bool OutlineExtractor::createFBO(int width, int height, bool useMSAA, int samples) {
    if (!mGLInitialized) {
        return false;
    }

    // 기존 FBO 해제
    destroyFBO();

    mFBOWidth = width;
    mFBOHeight = height;

    // QOpenGLFramebufferObject 생성
    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    fboFormat.setTextureTarget(GL_TEXTURE_2D);
    fboFormat.setInternalTextureFormat(GL_RGBA8);

    if (useMSAA) {
        fboFormat.setSamples(samples);
    }

    mFBO = std::make_unique<QOpenGLFramebufferObject>(width, height, fboFormat);

    if (!mFBO->isValid()) {
        qWarning() << "OutlineExtractor: FBO creation failed";
        mFBO.reset();
        return false;
    }

    mFBOTextureID = mFBO->texture();

    qDebug() << "OutlineExtractor: FBO created" << width << "x" << height
             << "MSAA:" << (useMSAA ? samples : 0);

    return true;
}

void OutlineExtractor::destroyFBO() {
    if (mFBO) {
        mFBO.reset();
        mFBOTextureID = 0;
    }
}

// ============================================================================
// Private Methods: Shader Compilation
// ============================================================================

bool OutlineExtractor::compileGeometryShader() {
    mGeometryShaderProgram = std::make_unique<QOpenGLShaderProgram>();

    // Vertex Shader
    QString vertSource = loadShaderFile(":/shaders/dongarch/silhouette.vert");
    if (vertSource.isEmpty()) {
        qWarning() << "OutlineExtractor: Failed to load silhouette.vert";
        return false;
    }

    if (!mGeometryShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSource)) {
        qWarning() << "OutlineExtractor: Vertex shader compilation failed:"
                   << mGeometryShaderProgram->log();
        return false;
    }

    // Geometry Shader
    QString geomSource = loadShaderFile(":/shaders/dongarch/silhouette.geom");
    if (geomSource.isEmpty()) {
        qWarning() << "OutlineExtractor: Failed to load silhouette.geom";
        return false;
    }

    if (!mGeometryShaderProgram->addShaderFromSourceCode(QOpenGLShader::Geometry, geomSource)) {
        qWarning() << "OutlineExtractor: Geometry shader compilation failed:"
                   << mGeometryShaderProgram->log();
        return false;
    }

    // Fragment Shader
    QString fragSource = loadShaderFile(":/shaders/dongarch/silhouette.frag");
    if (fragSource.isEmpty()) {
        qWarning() << "OutlineExtractor: Failed to load silhouette.frag";
        return false;
    }

    if (!mGeometryShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSource)) {
        qWarning() << "OutlineExtractor: Fragment shader compilation failed:"
                   << mGeometryShaderProgram->log();
        return false;
    }

    // Link
    if (!mGeometryShaderProgram->link()) {
        qWarning() << "OutlineExtractor: Shader program linking failed:"
                   << mGeometryShaderProgram->log();
        return false;
    }

    qDebug() << "OutlineExtractor: Geometry Shader compiled successfully";
    return true;
}

bool OutlineExtractor::compileSobelShader() {
    mSobelShaderProgram = std::make_unique<QOpenGLShaderProgram>();

    // Vertex Shader (simple pass-through)
    QString vertSource = R"(
        #version 330 core
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 texCoord;
        out vec2 vTexCoord;
        void main() {
            vTexCoord = texCoord;
            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    if (!mSobelShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSource)) {
        qWarning() << "OutlineExtractor: Sobel vertex shader compilation failed:"
                   << mSobelShaderProgram->log();
        return false;
    }

    // Fragment Shader (Sobel Filter - GigaMesh 기반)
    QString fragSource = R"(
        #version 330 core

        uniform sampler2D uFBO_Texture_ID;
        uniform vec2 uViewPortSize = vec2(1024.0, 768.0);

        in vec2 vTexCoord;
        out vec4 fragColor;

        void main() {
            float dx = 1.0 / uViewPortSize.x;
            float dy = 1.0 / uViewPortSize.y;

            // Sobel x-direction
            vec4 Sx = -       texture(uFBO_Texture_ID, vTexCoord + vec2(-dx, -dy))
                      - 2.0 * texture(uFBO_Texture_ID, vTexCoord + vec2(-dx,   0))
                      -       texture(uFBO_Texture_ID, vTexCoord + vec2(-dx,  dy))
                      +       texture(uFBO_Texture_ID, vTexCoord + vec2( dx, -dy))
                      + 2.0 * texture(uFBO_Texture_ID, vTexCoord + vec2( dx,   0))
                      +       texture(uFBO_Texture_ID, vTexCoord + vec2( dx,  dy));

            // Sobel y-direction
            vec4 Sy = -       texture(uFBO_Texture_ID, vTexCoord + vec2(-dx, -dy))
                      - 2.0 * texture(uFBO_Texture_ID, vTexCoord + vec2(  0, -dy))
                      -       texture(uFBO_Texture_ID, vTexCoord + vec2( dx, -dy))
                      +       texture(uFBO_Texture_ID, vTexCoord + vec2(-dx,  dy))
                      + 2.0 * texture(uFBO_Texture_ID, vTexCoord + vec2(  0,  dy))
                      +       texture(uFBO_Texture_ID, vTexCoord + vec2( dx,  dy));

            // Gradient magnitude
            fragColor = sqrt(Sx * Sx + Sy * Sy);
        }
    )";

    if (!mSobelShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSource)) {
        qWarning() << "OutlineExtractor: Sobel fragment shader compilation failed:"
                   << mSobelShaderProgram->log();
        return false;
    }

    // Link
    if (!mSobelShaderProgram->link()) {
        qWarning() << "OutlineExtractor: Sobel shader program linking failed:"
                   << mSobelShaderProgram->log();
        return false;
    }

    qDebug() << "OutlineExtractor: Sobel Shader compiled successfully";
    return true;
}

QString OutlineExtractor::loadShaderFile(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "OutlineExtractor: Failed to open shader file:" << path;
        return QString();
    }

    QTextStream in(&file);
    return in.readAll();
}

// ============================================================================
// SVG Export
// ============================================================================

bool OutlineExtractor::exportOutlineToSVG(const OutlineResult& result,
                                          ViewDirection viewDir,
                                          const QString& filePath,
                                          const QColor& lineColor,
                                          float lineWidth)
{
    if (result.edges.empty()) {
        qWarning() << "OutlineExtractor: No outline edges to export";
        return false;
    }

    using namespace DongArch::Outline::Algorithms;
    using namespace DongArch::Illustrator;

    qDebug() << "OutlineExtractor: Exporting" << result.edges.size() << "edges to SVG";

    // Step 1: Convert Edges → Polylines using VectorTracer
    VectorTracer tracer;
    TracingParams tracingParams;
    tracingParams.simplificationTolerance = 0.1;  // 0.1mm
    tracingParams.applySimplifcation = true;
    tracingParams.minPolylineLength = 2;

    // Create view matrix for projection
    QMatrix4x4 viewMatrix = VectorTracer::createOrthographicViewMatrix(viewDir);

    TracingResult tracingResult;
    bool success = tracer.traceEdgesWithProjection(result.edges, tracingParams,
                                                    viewMatrix, tracingResult);

    if (!success || tracingResult.polylines2D.empty()) {
        qWarning() << "OutlineExtractor: Failed to trace edges";
        return false;
    }

    qDebug() << "OutlineExtractor: Traced" << tracingResult.polylineCount << "polylines"
             << "(" << tracingResult.totalPoints << "points)";

    // Step 2: Calculate bounding box
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const auto& polyline : tracingResult.polylines2D) {
        for (const auto& pt : polyline) {
            minX = std::min(minX, static_cast<float>(pt.x()));
            maxX = std::max(maxX, static_cast<float>(pt.x()));
            minY = std::min(minY, static_cast<float>(pt.y()));
            maxY = std::max(maxY, static_cast<float>(pt.y()));
        }
    }

    float width = maxX - minX;
    float height = maxY - minY;

    qDebug() << "OutlineExtractor: Bounding box:" << width << "x" << height << "mm";

    // Step 3: Create SVG Exporter
    SVGExporter exporter;
    exporter.setCanvasSize(QSizeF(width + 20, height + 20));  // 10mm margin
    exporter.setUnit(SVGExporter::Unit::Millimeter);
    exporter.setBackgroundColor(Qt::transparent);

    // Step 4: Shift polylines to have 10mm margin
    std::vector<std::vector<QPointF>> shiftedPolylines;
    shiftedPolylines.reserve(tracingResult.polylines2D.size());

    for (const auto& polyline : tracingResult.polylines2D) {
        std::vector<QPointF> shiftedPolyline;
        shiftedPolyline.reserve(polyline.size());

        for (const auto& pt : polyline) {
            shiftedPolyline.emplace_back(pt.x() - minX + 10, pt.y() - minY + 10);
        }

        shiftedPolylines.push_back(std::move(shiftedPolyline));
    }

    // Step 5: Create Outline layer
    SVGLayer outlineLayer;
    outlineLayer.name = "Outline";
    outlineLayer.id = "outline_layer_1";
    outlineLayer.strokeColor = lineColor;
    outlineLayer.strokeWidth = lineWidth;
    outlineLayer.visible = true;
    outlineLayer.polylines = std::move(shiftedPolylines);

    exporter.addLayer(outlineLayer);

    // Step 6: Export to file
    success = exporter.exportToFile(filePath);

    if (success) {
        qDebug() << "OutlineExtractor: SVG exported successfully:" << filePath;
        qDebug() << "  - Polylines:" << tracingResult.polylineCount;
        qDebug() << "  - Points:" << tracingResult.totalPoints;
        qDebug() << "  - Canvas size:" << (width + 20) << "x" << (height + 20) << "mm";
    } else {
        qWarning() << "OutlineExtractor: Failed to export SVG";
    }

    return success;
}

} // namespace Outline
} // namespace DongArch
