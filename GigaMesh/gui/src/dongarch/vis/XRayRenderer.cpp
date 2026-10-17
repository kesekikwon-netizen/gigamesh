/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 5: X-Ray Depth Peeling Renderer Implementation
 */

#include "XRayRenderer.h"
#include <QDebug>
#include <QOpenGLTexture>

namespace DongArch {
namespace Vis {

//==============================================================================
// Constructor / Destructor
//==============================================================================

XRayRenderer::XRayRenderer(int width, int height)
    : mGL(nullptr)
    , mWidth(width)
    , mHeight(height)
    , mLayerCount(6)  // Medium quality default
    , mOpacity(0.3f)
    , mColorMode(ColorMode::WhiteOnBlack)
    , mInitialized(false)
    , mCurrentLayer(0)
{
}

XRayRenderer::~XRayRenderer()
{
    cleanup();
}

//==============================================================================
// Initialization
//==============================================================================

bool XRayRenderer::initialize()
{
    if (mInitialized) {
        qWarning() << "XRayRenderer already initialized";
        return true;
    }

    // Get OpenGL functions
    mGL = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    if (!mGL) {
        qCritical() << "Failed to get OpenGL 3.3 functions";
        return false;
    }

    // Compile shaders
    if (!compileShaders()) {
        qCritical() << "Failed to compile X-Ray shaders";
        return false;
    }

    // Create framebuffers
    if (!createFramebuffers()) {
        qCritical() << "Failed to create X-Ray framebuffers";
        return false;
    }

    mInitialized = true;
    qDebug() << "XRayRenderer initialized successfully"
             << "(" << mWidth << "x" << mHeight << "," << mLayerCount << "layers)";

    return true;
}

void XRayRenderer::cleanup()
{
    mDepthFBOs.clear();
    mColorFBOs.clear();
    mFinalFBO.reset();
    mPeelingShader.reset();
    mBlendShader.reset();

    mInitialized = false;
}

//==============================================================================
// Resize
//==============================================================================

void XRayRenderer::resize(int width, int height)
{
    if (width == mWidth && height == mHeight) {
        return;
    }

    mWidth = width;
    mHeight = height;

    if (mInitialized) {
        // Recreate framebuffers with new size
        createFramebuffers();
    }
}

//==============================================================================
// Shader Compilation
//==============================================================================

bool XRayRenderer::compileShaders()
{
    // Depth Peeling Shader
    mPeelingShader = std::make_unique<QOpenGLShaderProgram>();

    if (!mPeelingShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/dongarch/xray.vert")) {
        qCritical() << "Failed to compile xray.vert:" << mPeelingShader->log();
        return false;
    }

    if (!mPeelingShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/dongarch/xray.frag")) {
        qCritical() << "Failed to compile xray.frag:" << mPeelingShader->log();
        return false;
    }

    if (!mPeelingShader->link()) {
        qCritical() << "Failed to link peeling shader:" << mPeelingShader->log();
        return false;
    }

    // Blend Shader
    mBlendShader = std::make_unique<QOpenGLShaderProgram>();

    if (!mBlendShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/fullscreenQuad_passthrough.vert")) {
        qCritical() << "Failed to compile fullscreen quad vertex shader:" << mBlendShader->log();
        return false;
    }

    if (!mBlendShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/dongarch/xray_blend.frag")) {
        qCritical() << "Failed to compile xray_blend.frag:" << mBlendShader->log();
        return false;
    }

    if (!mBlendShader->link()) {
        qCritical() << "Failed to link blend shader:" << mBlendShader->log();
        return false;
    }

    return true;
}

//==============================================================================
// Framebuffer Creation
//==============================================================================

bool XRayRenderer::createFramebuffers()
{
    mDepthFBOs.clear();
    mColorFBOs.clear();

    // Create depth and color FBOs for each layer
    for (int i = 0; i < mLayerCount; i++) {
        // Depth FBO
        QOpenGLFramebufferObjectFormat depthFormat;
        depthFormat.setAttachment(QOpenGLFramebufferObject::Depth);
        depthFormat.setInternalTextureFormat(GL_DEPTH_COMPONENT32F);

        auto depthFBO = std::make_unique<QOpenGLFramebufferObject>(mWidth, mHeight, depthFormat);
        if (!depthFBO->isValid()) {
            qCritical() << "Failed to create depth FBO for layer" << i;
            return false;
        }
        mDepthFBOs.push_back(std::move(depthFBO));

        // Color FBO
        QOpenGLFramebufferObjectFormat colorFormat;
        colorFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        colorFormat.setInternalTextureFormat(GL_RGBA32F);

        auto colorFBO = std::make_unique<QOpenGLFramebufferObject>(mWidth, mHeight, colorFormat);
        if (!colorFBO->isValid()) {
            qCritical() << "Failed to create color FBO for layer" << i;
            return false;
        }
        mColorFBOs.push_back(std::move(colorFBO));
    }

    // Final composite FBO
    QOpenGLFramebufferObjectFormat finalFormat;
    finalFormat.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    finalFormat.setInternalTextureFormat(GL_RGBA8);

    mFinalFBO = std::make_unique<QOpenGLFramebufferObject>(mWidth, mHeight, finalFormat);
    if (!mFinalFBO->isValid()) {
        qCritical() << "Failed to create final FBO";
        return false;
    }

    return true;
}

//==============================================================================
// Rendering
//==============================================================================

void XRayRenderer::beginRender(const QMatrix4x4& mvpMatrix)
{
    if (!mInitialized) {
        qWarning() << "XRayRenderer not initialized";
        return;
    }

    mMVPMatrix = mvpMatrix;
    mCurrentLayer = 0;

    // Clear all FBOs
    for (int i = 0; i < mLayerCount; i++) {
        mDepthFBOs[i]->bind();
        mGL->glClear(GL_DEPTH_BUFFER_BIT);
        mDepthFBOs[i]->release();

        mColorFBOs[i]->bind();
        mGL->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        mGL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mColorFBOs[i]->release();
    }

    mFinalFBO->bind();
    mGL->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    mGL->glClear(GL_COLOR_BUFFER_BIT);
    mFinalFBO->release();
}

void XRayRenderer::renderLayer(int layerIndex, std::function<void()> drawCallback)
{
    if (!mInitialized || layerIndex >= mLayerCount) {
        return;
    }

    // Bind color FBO for this layer
    mColorFBOs[layerIndex]->bind();

    // Enable depth test
    mGL->glEnable(GL_DEPTH_TEST);
    mGL->glDepthFunc(GL_LESS);

    // Bind peeling shader
    mPeelingShader->bind();
    mPeelingShader->setUniformValue("uMVPMatrix", mMVPMatrix);
    mPeelingShader->setUniformValue("uLayerIndex", layerIndex);
    mPeelingShader->setUniformValue("uOpacity", mOpacity);

    // Bind previous depth texture
    if (layerIndex > 0) {
        mGL->glActiveTexture(GL_TEXTURE0);
        mGL->glBindTexture(GL_TEXTURE_2D, mDepthFBOs[layerIndex - 1]->texture());
        mPeelingShader->setUniformValue("uPrevDepthTex", 0);
        mPeelingShader->setUniformValue("uUsePrevDepth", true);
    } else {
        mPeelingShader->setUniformValue("uUsePrevDepth", false);
    }

    // Set color mode
    int colorModeInt = static_cast<int>(mColorMode);
    mPeelingShader->setUniformValue("uColorMode", colorModeInt);

    // Draw mesh
    if (drawCallback) {
        drawCallback();
    }

    mPeelingShader->release();
    mColorFBOs[layerIndex]->release();

    mCurrentLayer = layerIndex + 1;
}

QImage XRayRenderer::endRender()
{
    if (!mInitialized) {
        return QImage();
    }

    // Blend all layers into final FBO
    mFinalFBO->bind();

    mGL->glDisable(GL_DEPTH_TEST);
    mGL->glEnable(GL_BLEND);
    mGL->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mBlendShader->bind();

    for (int i = 0; i < mLayerCount; i++) {
        mGL->glActiveTexture(GL_TEXTURE0 + i);
        mGL->glBindTexture(GL_TEXTURE_2D, mColorFBOs[i]->texture());
    }

    mBlendShader->setUniformValue("uLayerCount", mLayerCount);

    // Draw fullscreen quad
    // (실제로는 MeshWidget의 fullscreen quad 렌더링 코드 사용)
    // 여기서는 간략화

    mBlendShader->release();
    mGL->glDisable(GL_BLEND);

    mFinalFBO->release();

    // Return final image
    return mFinalFBO->toImage();
}

//==============================================================================
// Settings
//==============================================================================

void XRayRenderer::setLayerCount(int count)
{
    int clampedCount = std::clamp(count, MIN_LAYERS, MAX_LAYERS);

    if (clampedCount != mLayerCount) {
        mLayerCount = clampedCount;

        if (mInitialized) {
            // Recreate FBOs with new layer count
            createFramebuffers();
        }
    }
}

void XRayRenderer::setQuality(Quality quality)
{
    setLayerCount(qualityToLayerCount(quality));
}

void XRayRenderer::setOpacity(float opacity)
{
    mOpacity = std::clamp(opacity, 0.0f, 1.0f);
}

void XRayRenderer::setColorMode(ColorMode mode)
{
    mColorMode = mode;
}

} // namespace Vis
} // namespace DongArch
