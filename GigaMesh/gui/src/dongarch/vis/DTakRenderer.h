/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * Phase 5: D-Tak Renderer (GPU-accelerated Geodesic Distance Visualization)
 *
 * 참조: "Parallel and Scalable Heat Methods for Geodesic Distance Computation"
 *       J. Tao et al., 2018, DOI: 10.1109/TPAMI.2019.2933209
 *
 * This file is part of DongArch3D (based on GigaMesh).
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_DTAK_RENDERER_H
#define DONGARCH_DTAK_RENDERER_H

#include <GigaMesh/mesh/mesh.h>
#include "dongarch/common/DongArchTypes.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <memory>
#include <vector>

namespace DongArch {
namespace Vis {

//! \\class DTakRenderer
//! \\brief D-Tak 시각화: GPU 기반 Geodesic Distance 계산 및 렌더링
//!
//! Heat Method 알고리즘을 Fragment Shader로 구현하여
//! GigaMesh CPU 버전 대비 10-100배 성능 향상
//!
//! 알고리즘:
//! 1. Heat Source Initialization (Pass 0)
//! 2. Heat Diffusion Iterations (Pass 1-N): ∂u/∂t = Δu
//! 3. Distance Extraction (Final Pass): Δφ = ∇·(-∇u / |∇u|)
//!
//! 사용법:
//! ```cpp
//! DTakRenderer renderer;
//! renderer.init();
//! renderer.setMesh(mesh);
//! renderer.setHeatSource(sourcePosition);
//! renderer.computeGeodesicDistanceGPU();  // 10-100x faster than CPU
//! renderer.render(projMatrix, viewMatrix);
//! renderer.destroy();
//! ```
class DTakRenderer
{
public:
    //! 계산 모드
    enum class ComputeMode {
        CPU,  //!< GigaMesh CPU 방식 (EdgeGeodesic)
        GPU   //!< Heat Method GPU 방식 (Fragment Shader)
    };

    //! 렌더링 모드
    enum class RenderMode {
        Distance,        //!< Geodesic distance 색상 매핑
        Heatmap,         //!< Heat diffusion 시각화
        Gradient,        //!< Gradient field 시각화
        DistanceContour  //!< 등고선 표시
    };

    //! Heat Method 파라미터
    struct HeatMethodParams {
        int diffusionIterations = 100;     //!< Heat diffusion 반복 횟수
        float timeStep = 0.1f;              //!< 시간 간격 (dt)
        float heatSourceRadius = 0.01f;     //!< Heat source 반경
        bool useMultipass = true;           //!< Multi-pass 렌더링 사용
        int textureSize = 1024;             //!< FBO 텍스처 크기
    };

    DTakRenderer();
    ~DTakRenderer();

    // Copy/Move 금지
    DTakRenderer(const DTakRenderer&) = delete;
    DTakRenderer& operator=(const DTakRenderer&) = delete;
    DTakRenderer(DTakRenderer&&) = delete;
    DTakRenderer& operator=(DTakRenderer&&) = delete;

    //! OpenGL 초기화 (유효한 OpenGL 컨텍스트에서 호출)
    //! \\return 성공 시 true
    bool init();

    //! 리소스 해제
    void destroy();

    //! 초기화 여부 확인
    //! \\return 초기화되었으면 true
    bool isInitialized() const { return mIsInitialized; }

    //! 메시 설정
    //! \\param mesh 대상 메시 (non-owning pointer)
    void setMesh(Mesh* mesh);

    //! Heat source 위치 설정
    //! \\param position 소스 위치 (world space)
    void setHeatSource(const QVector3D& position);

    //! Heat Method 파라미터 설정
    //! \\param params Heat Method 파라미터
    void setHeatMethodParams(const HeatMethodParams& params);

    //! 계산 모드 설정
    //! \\param mode CPU 또는 GPU
    void setComputeMode(ComputeMode mode) { mComputeMode = mode; }

    //! 렌더링 모드 설정
    //! \\param mode 렌더링 모드
    void setRenderMode(RenderMode mode) { mRenderMode = mode; }

    //! Geodesic distance 계산 (GPU)
    //! \\return 성공 시 true
    bool computeGeodesicDistanceGPU();

    //! Geodesic distance 계산 (CPU - GigaMesh EdgeGeodesic)
    //! \\return 성공 시 true
    bool computeGeodesicDistanceCPU();

    //! 자동 모드 선택 (메시 크기 기반)
    //! \\return 성공 시 true
    bool computeGeodesicDistanceAuto();

    //! D-Tak 렌더링
    //! \\param projectionMatrix 투영 행렬
    //! \\param viewMatrix 뷰 행렬
    void render(const QMatrix4x4& projectionMatrix,
                const QMatrix4x4& viewMatrix);

    //! Geodesic distance 값 조회 (특정 정점)
    //! \\param vertexIndex 정점 인덱스
    //! \\return Geodesic distance (실패 시 -1.0)
    float getGeodesicDistance(size_t vertexIndex) const;

    //! 성능 통계 조회
    //! \\return GPU 계산 시간 (ms)
    double getLastComputeTimeGPU() const { return mLastComputeTimeGPU; }

    //! 성능 통계 조회
    //! \\return CPU 계산 시간 (ms)
    double getLastComputeTimeCPU() const { return mLastComputeTimeCPU; }

private:
    //! Heat Method Shader 초기화
    bool initializeHeatMethodShader();

    //! 렌더링 Shader 초기화
    bool initializeRenderShader();

    //! FBO 초기화 (Multi-pass rendering)
    bool initializeFBOs();

    //! 메시 데이터를 VAO/VBO로 업로드
    bool uploadMeshData();

    //! Heat Source 초기화 (Pass 0)
    void renderHeatSourceInit();

    //! Heat Diffusion (Pass 1-N)
    void renderHeatDiffusion(int iteration);

    //! Distance Extraction (Final Pass)
    void renderDistanceExtraction();

    //! FBO 텍스처를 CPU 메모리로 읽기
    bool readbackGeodesicDistances();

    //! OpenGL 상태
    bool mIsInitialized;
    QOpenGLFunctions_3_3_Core mGL;

    //! 메시 데이터
    Mesh* mMesh;  //!< Non-owning pointer
    QVector3D mHeatSourcePosition;
    float mMeshAverageEdgeLength;

    //! Heat Method 파라미터
    HeatMethodParams mParams;
    ComputeMode mComputeMode;
    RenderMode mRenderMode;

    //! Shaders
    std::unique_ptr<QOpenGLShaderProgram> mShaderHeatMethod;  //!< Heat Method shader
    std::unique_ptr<QOpenGLShaderProgram> mShaderRender;      //!< Rendering shader

    //! VAO/VBO (메시 지오메트리)
    QOpenGLVertexArrayObject mVAO;
    QOpenGLBuffer mVertexBuffer;
    QOpenGLBuffer mNormalBuffer;
    QOpenGLBuffer mTexCoordBuffer;
    QOpenGLBuffer mIndexBuffer;
    size_t mVertexCount;
    size_t mIndexCount;

    //! FBOs (Multi-pass rendering)
    std::unique_ptr<QOpenGLFramebufferObject> mFBO_Heat[2];  //!< Ping-pong FBOs for heat diffusion
    std::unique_ptr<QOpenGLFramebufferObject> mFBO_Distance; //!< Final distance texture
    int mCurrentFBO;  //!< Current FBO index (0 or 1)

    //! Geodesic distance 결과 (CPU readback)
    std::vector<float> mGeodesicDistances;

    //! 성능 통계
    double mLastComputeTimeGPU;
    double mLastComputeTimeCPU;
};

} // namespace Vis
} // namespace DongArch

#endif // DONGARCH_DTAK_RENDERER_H
