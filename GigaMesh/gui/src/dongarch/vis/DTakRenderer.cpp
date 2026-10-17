/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 5: D-Tak Renderer Implementation
 */

#include "DTakRenderer.h"
#include <GigaMesh/mesh/edgegeodesic.h>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <cmath>
#include <algorithm>

namespace DongArch {
namespace Vis {

//==============================================================================
// Constructor / Destructor
//==============================================================================

DTakRenderer::DTakRenderer()
    : mIsInitialized(false)
    , mMesh(nullptr)
    , mHeatSourcePosition(0.0f, 0.0f, 0.0f)
    , mMeshAverageEdgeLength(1.0f)
    , mComputeMode(ComputeMode::GPU)
    , mRenderMode(RenderMode::Distance)
    , mVertexBuffer(QOpenGLBuffer::VertexBuffer)
    , mNormalBuffer(QOpenGLBuffer::VertexBuffer)
    , mTexCoordBuffer(QOpenGLBuffer::VertexBuffer)
    , mIndexBuffer(QOpenGLBuffer::IndexBuffer)
    , mVertexCount(0)
    , mIndexCount(0)
    , mCurrentFBO(0)
    , mLastComputeTimeGPU(0.0)
    , mLastComputeTimeCPU(0.0)
{
}

DTakRenderer::~DTakRenderer()
{
    destroy();
}

//==============================================================================
// Initialization / Destruction
//==============================================================================

bool DTakRenderer::init()
{
    if (mIsInitialized) {
        return true;
    }

    // Initialize OpenGL functions
    if (!mGL.initializeOpenGLFunctions()) {
        std::cerr << "[DTakRenderer] Failed to initialize OpenGL functions" << std::endl;
        return false;
    }

    // Initialize shaders
    if (!initializeHeatMethodShader()) {
        std::cerr << "[DTakRenderer] Failed to initialize Heat Method shader" << std::endl;
        return false;
    }

    if (!initializeRenderShader()) {
        std::cerr << "[DTakRenderer] Failed to initialize render shader" << std::endl;
        return false;
    }

    // Initialize FBOs
    if (!initializeFBOs()) {
        std::cerr << "[DTakRenderer] Failed to initialize FBOs" << std::endl;
        return false;
    }

    // Initialize VAO
    if (!mVAO.create()) {
        std::cerr << "[DTakRenderer] Failed to create VAO" << std::endl;
        return false;
    }

    // Initialize VBOs
    if (!mVertexBuffer.create() || !mNormalBuffer.create() ||
        !mTexCoordBuffer.create() || !mIndexBuffer.create()) {
        std::cerr << "[DTakRenderer] Failed to create VBOs" << std::endl;
        return false;
    }

    mIsInitialized = true;
    std::cout << "[DTakRenderer] Initialization complete" << std::endl;
    return true;
}

void DTakRenderer::destroy()
{
    if (!mIsInitialized) {
        return;
    }

    mVAO.destroy();
    mVertexBuffer.destroy();
    mNormalBuffer.destroy();
    mTexCoordBuffer.destroy();
    mIndexBuffer.destroy();

    mShaderHeatMethod.reset();
    mShaderRender.reset();

    mFBO_Heat[0].reset();
    mFBO_Heat[1].reset();
    mFBO_Distance.reset();

    mIsInitialized = false;
}

//==============================================================================
// Shader Initialization
//==============================================================================

bool DTakRenderer::initializeHeatMethodShader()
{
    mShaderHeatMethod = std::make_unique<QOpenGLShaderProgram>();

    // Load vertex shader
    QString vertShaderPath = ":/shaders/dongarch/heatmethod.vert";
    if (!mShaderHeatMethod->addShaderFromSourceFile(QOpenGLShader::Vertex, vertShaderPath)) {
        std::cerr << "[DTakRenderer] Failed to load vertex shader: "
                  << vertShaderPath.toStdString() << std::endl;
        std::cerr << "Error: " << mShaderHeatMethod->log().toStdString() << std::endl;
        return false;
    }

    // Load fragment shader
    QString fragShaderPath = ":/shaders/dongarch/heatmethod.frag";
    if (!mShaderHeatMethod->addShaderFromSourceFile(QOpenGLShader::Fragment, fragShaderPath)) {
        std::cerr << "[DTakRenderer] Failed to load fragment shader: "
                  << fragShaderPath.toStdString() << std::endl;
        std::cerr << "Error: " << mShaderHeatMethod->log().toStdString() << std::endl;
        return false;
    }

    // Link shader program
    if (!mShaderHeatMethod->link()) {
        std::cerr << "[DTakRenderer] Failed to link Heat Method shader" << std::endl;
        std::cerr << "Error: " << mShaderHeatMethod->log().toStdString() << std::endl;
        return false;
    }

    std::cout << "[DTakRenderer] Heat Method shader loaded successfully" << std::endl;
    return true;
}

bool DTakRenderer::initializeRenderShader()
{
    // For now, use a simple shader for rendering geodesic distances
    // TODO: Create dedicated render shader for D-Tak visualization

    mShaderRender = std::make_unique<QOpenGLShaderProgram>();

    // Simple vertex shader (inline)
    const char* vertSource = R"(
        #version 330 core
        layout(location = 0) in vec3 vertexPosition;
        layout(location = 1) in vec3 vertexNormal;
        layout(location = 2) in vec2 vertexTexCoord;
        uniform mat4 modelMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 projectionMatrix;
        out vec2 texCoord;
        void main() {
            texCoord = vertexTexCoord;
            gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
        }
    )";

    // Simple fragment shader (inline)
    const char* fragSource = R"(
        #version 330 core
        in vec2 texCoord;
        uniform sampler2D distanceTexture;
        out vec4 fragColor;
        void main() {
            float dist = texture(distanceTexture, texCoord).r;
            // Color mapping: blue (near) -> green -> yellow -> red (far)
            vec3 color;
            if (dist < 0.25) {
                color = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0), dist * 4.0);
            } else if (dist < 0.5) {
                color = mix(vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), (dist - 0.25) * 4.0);
            } else if (dist < 0.75) {
                color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), (dist - 0.5) * 4.0);
            } else {
                color = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), (dist - 0.75) * 4.0);
            }
            fragColor = vec4(color, 1.0);
        }
    )";

    if (!mShaderRender->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSource)) {
        std::cerr << "[DTakRenderer] Failed to add render vertex shader" << std::endl;
        return false;
    }

    if (!mShaderRender->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSource)) {
        std::cerr << "[DTakRenderer] Failed to add render fragment shader" << std::endl;
        return false;
    }

    if (!mShaderRender->link()) {
        std::cerr << "[DTakRenderer] Failed to link render shader" << std::endl;
        return false;
    }

    return true;
}

//==============================================================================
// FBO Initialization
//==============================================================================

bool DTakRenderer::initializeFBOs()
{
    QOpenGLFramebufferObjectFormat fboFormat;
    fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
    fboFormat.setInternalTextureFormat(GL_RGBA32F);  // 32-bit float for precision

    int texSize = mParams.textureSize;

    // Create ping-pong FBOs for heat diffusion
    mFBO_Heat[0] = std::make_unique<QOpenGLFramebufferObject>(texSize, texSize, fboFormat);
    mFBO_Heat[1] = std::make_unique<QOpenGLFramebufferObject>(texSize, texSize, fboFormat);

    if (!mFBO_Heat[0]->isValid() || !mFBO_Heat[1]->isValid()) {
        std::cerr << "[DTakRenderer] Failed to create heat FBOs" << std::endl;
        return false;
    }

    // Create final distance FBO
    mFBO_Distance = std::make_unique<QOpenGLFramebufferObject>(texSize, texSize, fboFormat);

    if (!mFBO_Distance->isValid()) {
        std::cerr << "[DTakRenderer] Failed to create distance FBO" << std::endl;
        return false;
    }

    std::cout << "[DTakRenderer] FBOs created (" << texSize << "x" << texSize << ")" << std::endl;
    return true;
}

//==============================================================================
// Mesh Data
//==============================================================================

void DTakRenderer::setMesh(Mesh* mesh)
{
    mMesh = mesh;

    if (mMesh && mIsInitialized) {
        uploadMeshData();
    }
}

void DTakRenderer::setHeatSource(const QVector3D& position)
{
    mHeatSourcePosition = position;
}

void DTakRenderer::setHeatMethodParams(const HeatMethodParams& params)
{
    mParams = params;

    // Recreate FBOs if texture size changed
    if (mIsInitialized && !initializeFBOs()) {
        std::cerr << "[DTakRenderer] Failed to recreate FBOs" << std::endl;
    }
}

bool DTakRenderer::uploadMeshData()
{
    if (!mMesh || !mIsInitialized) {
        return false;
    }

    // Get mesh data
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;

    // Extract vertex positions and normals
    mVertexCount = mMesh->getVertexNr();
    vertices.reserve(mVertexCount * 3);
    normals.reserve(mVertexCount * 3);
    texCoords.reserve(mVertexCount * 2);

    for (size_t i = 0; i < mVertexCount; i++) {
        Vertex* v = mMesh->getVertexPos(i);
        if (!v) continue;

        vertices.push_back(static_cast<float>(v->getX()));
        vertices.push_back(static_cast<float>(v->getY()));
        vertices.push_back(static_cast<float>(v->getZ()));

        Vector3D normal = v->getNormal(true);
        normals.push_back(static_cast<float>(normal.getX()));
        normals.push_back(static_cast<float>(normal.getY()));
        normals.push_back(static_cast<float>(normal.getZ()));

        // Generate UV coordinates (simple cylindrical mapping)
        float u = static_cast<float>(i % 1024) / 1024.0f;
        float v_coord = static_cast<float>(i / 1024) / 1024.0f;
        texCoords.push_back(u);
        texCoords.push_back(v_coord);
    }

    // Extract face indices
    mIndexCount = mMesh->getFaceNr() * 3;
    indices.reserve(mIndexCount);

    for (size_t i = 0; i < mMesh->getFaceNr(); i++) {
        Face* f = mMesh->getFacePos(i);
        if (!f) continue;

        Vertex* v0 = f->getVertA();
        Vertex* v1 = f->getVertB();
        Vertex* v2 = f->getVertC();

        if (!v0 || !v1 || !v2) continue;

        indices.push_back(static_cast<unsigned int>(v0->getIndex()));
        indices.push_back(static_cast<unsigned int>(v1->getIndex()));
        indices.push_back(static_cast<unsigned int>(v2->getIndex()));
    }

    // Compute average edge length (for Laplacian)
    double totalLength = 0.0;
    size_t edgeCount = 0;
    for (size_t i = 0; i < mMesh->getFaceNr(); i++) {
        Face* f = mMesh->getFacePos(i);
        if (!f) continue;

        Vertex* v0 = f->getVertA();
        Vertex* v1 = f->getVertB();
        Vertex* v2 = f->getVertC();

        if (!v0 || !v1 || !v2) continue;

        totalLength += v0->estDistanceTo(v1);
        totalLength += v1->estDistanceTo(v2);
        totalLength += v2->estDistanceTo(v0);
        edgeCount += 3;
    }
    mMeshAverageEdgeLength = static_cast<float>(totalLength / edgeCount);

    // Upload to GPU
    mVAO.bind();

    mVertexBuffer.bind();
    mVertexBuffer.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

    mNormalBuffer.bind();
    mNormalBuffer.allocate(normals.data(), static_cast<int>(normals.size() * sizeof(float)));

    mTexCoordBuffer.bind();
    mTexCoordBuffer.allocate(texCoords.data(), static_cast<int>(texCoords.size() * sizeof(float)));

    mIndexBuffer.bind();
    mIndexBuffer.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(unsigned int)));

    mVAO.release();

    std::cout << "[DTakRenderer] Mesh uploaded: " << mVertexCount << " vertices, "
              << mIndexCount / 3 << " faces, avg edge length: " << mMeshAverageEdgeLength << std::endl;

    return true;
}

//==============================================================================
// GPU Computation
//==============================================================================

bool DTakRenderer::computeGeodesicDistanceGPU()
{
    if (!mIsInitialized || !mMesh) {
        std::cerr << "[DTakRenderer] Not initialized or no mesh" << std::endl;
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    // Pass 0: Initialize heat source
    renderHeatSourceInit();

    // Pass 1-N: Heat diffusion iterations
    for (int i = 0; i < mParams.diffusionIterations; i++) {
        renderHeatDiffusion(i);
    }

    // Final Pass: Extract geodesic distances
    renderDistanceExtraction();

    // Readback results to CPU
    if (!readbackGeodesicDistances()) {
        std::cerr << "[DTakRenderer] Failed to readback distances" << std::endl;
        return false;
    }

    mLastComputeTimeGPU = timer.elapsed();

    std::cout << "[DTakRenderer] GPU computation complete: " << mLastComputeTimeGPU << " ms" << std::endl;
    return true;
}

void DTakRenderer::renderHeatSourceInit()
{
    // Render to FBO_Heat[0]
    mFBO_Heat[0]->bind();

    mGL.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    mGL.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mShaderHeatMethod->bind();

    // Set uniforms
    mShaderHeatMethod->setUniformValue("renderMode", 0);  // Mode 0: Init
    mShaderHeatMethod->setUniformValue("heatSourcePosition", mHeatSourcePosition);
    mShaderHeatMethod->setUniformValue("heatSourceRadius", mParams.heatSourceRadius);

    // TODO: Render mesh
    // For now, just render a full-screen quad
    mVAO.bind();
    mGL.glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, nullptr);
    mVAO.release();

    mShaderHeatMethod->release();
    mFBO_Heat[0]->release();

    mCurrentFBO = 0;
}

void DTakRenderer::renderHeatDiffusion(int iteration)
{
    // Ping-pong between FBOs
    int srcFBO = mCurrentFBO;
    int dstFBO = 1 - mCurrentFBO;

    mFBO_Heat[dstFBO]->bind();

    mShaderHeatMethod->bind();

    // Set uniforms
    mShaderHeatMethod->setUniformValue("renderMode", 1);  // Mode 1: Diffusion
    mShaderHeatMethod->setUniformValue("timeStep", mParams.timeStep);
    mShaderHeatMethod->setUniformValue("meshAverageEdgeLength", mMeshAverageEdgeLength);
    mShaderHeatMethod->setUniformValue("diffusionIterations", iteration);
    mShaderHeatMethod->setUniformValue("texelSize", QVector2D(1.0f / mParams.textureSize, 1.0f / mParams.textureSize));

    // Bind previous heat texture
    mGL.glActiveTexture(GL_TEXTURE0);
    mGL.glBindTexture(GL_TEXTURE_2D, mFBO_Heat[srcFBO]->texture());
    mShaderHeatMethod->setUniformValue("heatTexture", 0);

    // Render mesh
    mVAO.bind();
    mGL.glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, nullptr);
    mVAO.release();

    mShaderHeatMethod->release();
    mFBO_Heat[dstFBO]->release();

    mCurrentFBO = dstFBO;
}

void DTakRenderer::renderDistanceExtraction()
{
    mFBO_Distance->bind();

    mShaderHeatMethod->bind();

    // Set uniforms
    mShaderHeatMethod->setUniformValue("renderMode", 2);  // Mode 2: Distance
    mShaderHeatMethod->setUniformValue("texelSize", QVector2D(1.0f / mParams.textureSize, 1.0f / mParams.textureSize));

    // Bind final heat texture
    mGL.glActiveTexture(GL_TEXTURE0);
    mGL.glBindTexture(GL_TEXTURE_2D, mFBO_Heat[mCurrentFBO]->texture());
    mShaderHeatMethod->setUniformValue("heatTexture", 0);

    // Bind gradient texture (same as heat for now)
    mGL.glActiveTexture(GL_TEXTURE1);
    mGL.glBindTexture(GL_TEXTURE_2D, mFBO_Heat[mCurrentFBO]->texture());
    mShaderHeatMethod->setUniformValue("gradientTexture", 1);

    // Render mesh
    mVAO.bind();
    mGL.glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, nullptr);
    mVAO.release();

    mShaderHeatMethod->release();
    mFBO_Distance->release();
}

bool DTakRenderer::readbackGeodesicDistances()
{
    // Read FBO texture to CPU memory
    mFBO_Distance->bind();

    int width = mFBO_Distance->width();
    int height = mFBO_Distance->height();

    std::vector<float> pixels(width * height * 4);  // RGBA
    mGL.glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, pixels.data());

    mFBO_Distance->release();

    // Extract distance values (R channel)
    mGeodesicDistances.resize(mVertexCount);
    for (size_t i = 0; i < mVertexCount && i < pixels.size() / 4; i++) {
        mGeodesicDistances[i] = pixels[i * 4];  // R channel
    }

    return true;
}

//==============================================================================
// CPU Computation (GigaMesh EdgeGeodesic)
//==============================================================================

bool DTakRenderer::computeGeodesicDistanceCPU()
{
    if (!mMesh) {
        std::cerr << "[DTakRenderer] No mesh" << std::endl;
        return false;
    }

    QElapsedTimer timer;
    timer.start();

    // TODO: Use GigaMesh EdgeGeodesic for CPU computation
    // For now, placeholder

    std::cout << "[DTakRenderer] CPU computation (EdgeGeodesic) - TODO" << std::endl;

    mLastComputeTimeCPU = timer.elapsed();
    return true;
}

bool DTakRenderer::computeGeodesicDistanceAuto()
{
    // Select mode based on mesh size
    if (mVertexCount > 100000) {
        // Large mesh: use GPU
        return computeGeodesicDistanceGPU();
    } else {
        // Small mesh: use CPU
        return computeGeodesicDistanceCPU();
    }
}

//==============================================================================
// Rendering
//==============================================================================

void DTakRenderer::render(const QMatrix4x4& projectionMatrix, const QMatrix4x4& viewMatrix)
{
    if (!mIsInitialized || !mMesh) {
        return;
    }

    mShaderRender->bind();

    // Set matrices
    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();

    mShaderRender->setUniformValue("projectionMatrix", projectionMatrix);
    mShaderRender->setUniformValue("viewMatrix", viewMatrix);
    mShaderRender->setUniformValue("modelMatrix", modelMatrix);

    // Bind distance texture
    mGL.glActiveTexture(GL_TEXTURE0);
    mGL.glBindTexture(GL_TEXTURE_2D, mFBO_Distance->texture());
    mShaderRender->setUniformValue("distanceTexture", 0);

    // Render mesh
    mVAO.bind();
    mGL.glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mIndexCount), GL_UNSIGNED_INT, nullptr);
    mVAO.release();

    mShaderRender->release();
}

//==============================================================================
// Query
//==============================================================================

float DTakRenderer::getGeodesicDistance(size_t vertexIndex) const
{
    if (vertexIndex >= mGeodesicDistances.size()) {
        return -1.0f;
    }
    return mGeodesicDistances[vertexIndex];
}

} // namespace Vis
} // namespace DongArch
