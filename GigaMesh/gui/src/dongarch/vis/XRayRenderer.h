/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 5: X-Ray Depth Peeling Renderer
 *
 * 참조: "Dual Depth Peeling" (Bavoil & Myers, 2008)
 *       "Order Independent Transparency" (Everitt, 2001)
 *
 * X-Ray 렌더링:
 * - Multi-layer transparency (4-8 layers)
 * - Depth peeling per-fragment sorting
 * - Thickness-based opacity control
 *
 * This file is part of DongArch3D (based on GigaMesh).
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_XRAY_RENDERER_H
#define DONGARCH_XRAY_RENDERER_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QMatrix4x4>
#include <memory>
#include <vector>

namespace DongArch {
namespace Vis {

//! \class XRayRenderer
//! \brief X-Ray 스타일 렌더링 (Depth Peeling 기반)
//!
//! Depth Peeling 알고리즘:
//! 1. 첫 번째 레이어: 가장 앞쪽 표면 렌더링
//! 2. 두 번째 레이어: 첫 번째 레이어보다 뒤에 있는 표면 렌더링
//! 3. N번째 레이어: N-1번째 레이어보다 뒤에 있는 표면 렌더링
//! 4. 모든 레이어를 Alpha Blending으로 합성
//!
//! 사용법:
//! ```cpp
//! XRayRenderer xray(width, height);
//! xray.initialize();
//! xray.setLayerCount(6);
//! xray.setOpacity(0.3f);
//!
//! // 렌더링
//! xray.beginRender();
//! xray.renderLayer(0, drawMeshCallback);  // Layer 0
//! xray.renderLayer(1, drawMeshCallback);  // Layer 1
//! ...
//! QImage result = xray.endRender();
//! ```
class XRayRenderer
{
public:
    //! X-Ray 렌더링 품질
    enum class Quality {
        Low,     //!< 4 layers (빠름)
        Medium,  //!< 6 layers (기본값)
        High     //!< 8 layers (느림, 고품질)
    };

    //! X-Ray 컬러 모드
    enum class ColorMode {
        WhiteOnBlack,   //!< 흰색 메시 + 검은색 배경 (기본값)
        BlackOnWhite,   //!< 검은색 메시 + 흰색 배경
        DepthGradient,  //!< 깊이 기반 그라디언트
        Thickness       //!< 두께 기반 컬러맵
    };

    //! 생성자
    //! \param width 뷰포트 너비
    //! \param height 뷰포트 높이
    explicit XRayRenderer(int width, int height);

    //! 소멸자
    ~XRayRenderer();

    //! OpenGL 리소스 초기화
    //! \return 성공 여부
    bool initialize();

    //! 리소스 해제
    void cleanup();

    //! 뷰포트 크기 변경
    //! \param width 새 너비
    //! \param height 새 높이
    void resize(int width, int height);

    //! 렌더링 시작
    //! \param mvpMatrix Model-View-Projection 행렬
    void beginRender(const QMatrix4x4& mvpMatrix);

    //! 레이어 렌더링
    //! \param layerIndex 레이어 인덱스 (0 = 첫 번째)
    //! \param drawCallback 메시 그리기 콜백
    void renderLayer(int layerIndex, std::function<void()> drawCallback);

    //! 렌더링 종료 및 최종 이미지 반환
    //! \return 합성된 X-Ray 이미지
    QImage endRender();

    //! 레이어 개수 설정
    //! \param count 레이어 개수 (4-8)
    void setLayerCount(int count);

    //! 품질 설정 (레이어 개수 자동 설정)
    //! \param quality 품질 레벨
    void setQuality(Quality quality);

    //! 전체 불투명도 설정
    //! \param opacity 불투명도 (0.0 = 완전 투명, 1.0 = 불투명)
    void setOpacity(float opacity);

    //! 컬러 모드 설정
    //! \param mode 컬러 모드
    void setColorMode(ColorMode mode);

    //! 레이어 개수 조회
    //! \return 현재 레이어 개수
    int getLayerCount() const { return mLayerCount; }

    //! 불투명도 조회
    //! \return 현재 불투명도
    float getOpacity() const { return mOpacity; }

    //! 초기화 여부 확인
    //! \return 초기화 완료 여부
    bool isInitialized() const { return mInitialized; }

private:
    //! FBO 생성
    //! \return 성공 여부
    bool createFramebuffers();

    //! 셰이더 컴파일
    //! \return 성공 여부
    bool compileShaders();

    //! Quality를 레이어 개수로 변환
    //! \param quality 품질 레벨
    //! \return 레이어 개수
    static constexpr int qualityToLayerCount(Quality quality);

    // OpenGL 함수
    QOpenGLFunctions_3_3_Core* mGL;

    // 셰이더 프로그램
    std::unique_ptr<QOpenGLShaderProgram> mPeelingShader;  //!< Depth peeling shader
    std::unique_ptr<QOpenGLShaderProgram> mBlendShader;    //!< Final blend shader

    // Framebuffer Objects
    std::vector<std::unique_ptr<QOpenGLFramebufferObject>> mDepthFBOs;    //!< Depth textures per layer
    std::vector<std::unique_ptr<QOpenGLFramebufferObject>> mColorFBOs;    //!< Color textures per layer
    std::unique_ptr<QOpenGLFramebufferObject> mFinalFBO;                  //!< Final composite FBO

    // 렌더링 파라미터
    int mWidth;                 //!< 뷰포트 너비
    int mHeight;                //!< 뷰포트 높이
    int mLayerCount;            //!< 레이어 개수 (4-8)
    float mOpacity;             //!< 전체 불투명도 (0.0-1.0)
    ColorMode mColorMode;       //!< 컬러 모드

    // MVP 행렬
    QMatrix4x4 mMVPMatrix;

    // 상태
    bool mInitialized;          //!< 초기화 완료 여부
    int mCurrentLayer;          //!< 현재 렌더링 중인 레이어

    // 상수
    static constexpr int MAX_LAYERS = 8;     //!< 최대 레이어 개수
    static constexpr int MIN_LAYERS = 4;     //!< 최소 레이어 개수
};

//==============================================================================
// Inline Implementations
//==============================================================================

constexpr int XRayRenderer::qualityToLayerCount(Quality quality)
{
    switch (quality) {
        case Quality::Low:    return 4;
        case Quality::Medium: return 6;
        case Quality::High:   return 8;
        default:              return 6;
    }
}

} // namespace Vis
} // namespace DongArch

#endif // DONGARCH_XRAY_RENDERER_H
