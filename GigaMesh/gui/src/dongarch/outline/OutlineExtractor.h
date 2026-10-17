/* DongArch3D - Dongguk Archaeological 3D Measurement System
 * Copyright (C) 2025 Dongguk University Cultural Heritage Research Institute
 *
 * This file is part of DongArch3D.
 *
 * DongArch3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef DONGARCH_OUTLINE_EXTRACTOR_H
#define DONGARCH_OUTLINE_EXTRACTOR_H

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/vector3d.h>
#include "SilhouetteDetector.h"
#include "dongarch/common/DongArchTypes.h"
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QColor>
#include <QString>
#include <vector>
#include <memory>

//! \file OutlineExtractor.h
//! \brief Phase 3: Outline (외곽 라인) 통합 추출기 (v4.0)
//!
//! DongArch3D v4.0 Phase 3 통합 구현
//! - CPU Silhouette Detection (SilhouetteDetector)
//! - GPU Geometry Shader Silhouette (silhouette.geom)
//! - Sobel Edge Detection (NPR_ApplySobel.frag)
//! - Dual-pass Rendering (FBO)
//!
//! 기반 기술:
//! - GigaMesh NPR Sobel Filter (70% 재사용)
//! - 2008년 논문: Geometry Shader Silhouette
//! - 2025년 논문: Quadric-based Optimization

namespace DongArch {
namespace Outline {

//! \enum OutlineMethod
//! \brief Outline 추출 방법
enum class OutlineMethod {
    CPU_SILHOUETTE,      //!< CPU 기반 Silhouette 검출
    GPU_GEOMETRY_SHADER, //!< GPU Geometry Shader 기반 (10-100배 빠름)
    SOBEL_FILTER,        //!< Sobel Edge Detection (NPR)
    DUAL_PASS,           //!< Dual-pass: Geometry Shader + Sobel
    HYBRID               //!< Hybrid: CPU + GPU
};

//! \struct OutlineResult
//! \brief Outline 추출 결과
struct OutlineResult {
    OutlineMethod method;                      //!< 사용된 추출 방법
    std::vector<EdgeRef> edges;                //!< 추출된 엣지 목록
    std::vector<DongArch::Math::Vec3> points;  //!< 추출된 점 목록 (Sobel)

    size_t totalEdges;                         //!< 전체 엣지 개수
    size_t outlineEdges;                       //!< Outline 엣지 개수

    double processingTime;                     //!< 처리 시간 (ms)
    double compressionRatio;                   //!< 압축률 (%) - Quadric 적용 시

    // FBO 관련 (GPU)
    bool usedFBO;                              //!< FBO 사용 여부
    unsigned int fboTextureID;                 //!< FBO 텍스처 ID

    // 품질 메트릭
    double sobelThreshold;                     //!< Sobel 임계값
    double averageEdgeStrength;                //!< 평균 엣지 강도
};

//! \struct DualPassParams
//! \brief Dual-pass Rendering 파라미터
struct DualPassParams {
    // Pass 1: Geometry Shader
    bool useGeometryShader = true;             //!< Geometry Shader 사용
    Vector3D viewVector;                       //!< View 벡터
    Vector3D cameraPosition;                   //!< 카메라 위치

    // Pass 2: Sobel Filter
    bool applySobel = true;                    //!< Sobel 필터 적용
    float sobelThreshold = 0.5f;               //!< Sobel 임계값

    // FBO 설정
    int fboWidth = 1024;                       //!< FBO 너비 (픽셀)
    int fboHeight = 768;                       //!< FBO 높이 (픽셀)
    bool useMSAA = true;                       //!< MSAA (안티앨리어싱) 사용
    int msaaSamples = 4;                       //!< MSAA 샘플 수

    // 최적화
    bool useQuadricOptimization = true;        //!< Quadric 최적화 적용
    double quadricErrorThreshold = 0.1;        //!< Quadric 오차 임계값
};

//! \class OutlineExtractor
//! \brief Outline (외곽 라인) 통합 추출기
//!
//! 3가지 방법 통합:
//! 1. CPU Silhouette Detection (SilhouetteDetector)
//! 2. GPU Geometry Shader (silhouette.geom)
//! 3. Sobel Edge Detection (NPR_ApplySobel.frag)
//!
//! Dual-pass Rendering:
//! - Pass 1: Geometry Shader → FBO
//! - Pass 2: Sobel Filter
class OutlineExtractor : protected QOpenGLFunctions_3_3_Core {
public:
    //! 생성자
    //! \param mesh 대상 메시 (non-owning)
    explicit OutlineExtractor(Mesh* mesh);

    //! 소멸자
    ~OutlineExtractor();

    // Copy/Move 금지 (OpenGL 리소스)
    OutlineExtractor(const OutlineExtractor&) = delete;
    OutlineExtractor& operator=(const OutlineExtractor&) = delete;
    OutlineExtractor(OutlineExtractor&&) = delete;
    OutlineExtractor& operator=(OutlineExtractor&&) = delete;

    //! OpenGL 초기화
    //! \return 성공 여부
    bool initializeGL();

    //! 메시 유효성 확인
    //! \return 메시가 유효하면 true
    bool isValid() const { return mMesh != nullptr; }

    //! Outline 추출 (CPU Silhouette)
    //! \param viewDir View 방향
    //! \param result 결과 (출력)
    //! \return 성공 여부
    bool extractCPU(ViewDirection viewDir, OutlineResult& result);

    //! Outline 추출 (GPU Geometry Shader)
    //! \param viewVector View 벡터
    //! \param result 결과 (출력)
    //! \return 성공 여부
    bool extractGPU(const Vector3D& viewVector, OutlineResult& result);

    //! Outline 추출 (Sobel Filter)
    //! \param threshold Sobel 임계값
    //! \param result 결과 (출력)
    //! \return 성공 여부
    bool extractSobel(float threshold, OutlineResult& result);

    //! Outline 추출 (Dual-pass: Geometry Shader + Sobel)
    //! \param params Dual-pass 파라미터
    //! \param result 결과 (출력)
    //! \return 성공 여부
    bool extractDualPass(const DualPassParams& params, OutlineResult& result);

    //! Outline 추출 (Hybrid: CPU + GPU 자동 선택)
    //! \param viewVector View 벡터
    //! \param result 결과 (출력)
    //! \return 성공 여부
    bool extractHybrid(const Vector3D& viewVector, OutlineResult& result);

    //! FBO 텍스처 가져오기
    //! \return FBO 텍스처 ID
    unsigned int getFBOTexture() const { return mFBOTextureID; }

    //! 셰이더 프로그램 가져오기 (디버그용)
    //! \return Geometry Shader 프로그램
    QOpenGLShaderProgram* getGeometryShaderProgram() const { return mGeometryShaderProgram.get(); }

    //! \return Sobel Shader 프로그램
    QOpenGLShaderProgram* getSobelShaderProgram() const { return mSobelShaderProgram.get(); }

    //! Outline → SVG 내보내기
    //! \param result Outline 추출 결과
    //! \param viewDir 뷰 방향 (2D 투영용)
    //! \param filePath SVG 파일 경로
    //! \param lineColor 선 색상 (기본: 검정)
    //! \param lineWidth 선 두께 (기본: 0.5mm)
    //! \return 성공 여부
    bool exportOutlineToSVG(const OutlineResult& result,
                            ViewDirection viewDir,
                            const QString& filePath,
                            const QColor& lineColor = QColor(Qt::black),
                            float lineWidth = 0.5f);

private:
    //! Pass 1: Geometry Shader로 Silhouette 렌더링 → FBO
    //! \param params Dual-pass 파라미터
    //! \return 성공 여부
    bool renderSilhouetteToFBO(const DualPassParams& params);

    //! Pass 2: Sobel Filter 적용
    //! \param threshold Sobel 임계값
    //! \param result 결과 (출력)
    //! \return 성공 여부
    bool applySobelFilter(float threshold, OutlineResult& result);

    //! FBO 생성
    //! \param width FBO 너비
    //! \param height FBO 높이
    //! \param useMSAA MSAA 사용
    //! \param samples MSAA 샘플 수
    //! \return 성공 여부
    bool createFBO(int width, int height, bool useMSAA = true, int samples = 4);

    //! FBO 해제
    void destroyFBO();

    //! Geometry Shader 프로그램 컴파일
    //! \return 성공 여부
    bool compileGeometryShader();

    //! Sobel Shader 프로그램 컴파일
    //! \return 성공 여부
    bool compileSobelShader();

    //! 셰이더 파일 로드
    //! \param path 셰이더 파일 경로
    //! \return 셰이더 소스 코드
    QString loadShaderFile(const QString& path) const;

    //! FBO에서 엣지 추출 (Sobel 결과 분석)
    //! \param result 결과 (출력)
    //! \return 추출된 엣지 개수
    size_t extractEdgesFromFBO(OutlineResult& result);

    Mesh* mMesh;  //!< 대상 메시 (non-owning)

    // CPU Silhouette Detector
    std::unique_ptr<SilhouetteDetector> mSilhouetteDetector;

    // OpenGL 리소스
    bool mGLInitialized;  //!< OpenGL 초기화 여부
    std::unique_ptr<QOpenGLShaderProgram> mGeometryShaderProgram;  //!< Geometry Shader 프로그램
    std::unique_ptr<QOpenGLShaderProgram> mSobelShaderProgram;     //!< Sobel Shader 프로그램

    // FBO (Framebuffer Object)
    std::unique_ptr<QOpenGLFramebufferObject> mFBO;  //!< FBO
    unsigned int mFBOTextureID;                      //!< FBO 텍스처 ID
    int mFBOWidth;                                   //!< FBO 너비
    int mFBOHeight;                                  //!< FBO 높이

    // Vertex Array Object / Vertex Buffer Object
    unsigned int mVAO;  //!< VAO
    unsigned int mVBO;  //!< VBO (정점 데이터)
    unsigned int mEBO;  //!< EBO (인덱스 데이터)
};

//! Outline 방법 이름 조회
//! \param method Outline 방법
//! \return 방법 이름 (한국어)
inline const char* outlineMethodName(OutlineMethod method) {
    switch (method) {
        case OutlineMethod::CPU_SILHOUETTE:      return "CPU Silhouette";
        case OutlineMethod::GPU_GEOMETRY_SHADER: return "GPU Geometry Shader";
        case OutlineMethod::SOBEL_FILTER:        return "Sobel Edge Detection";
        case OutlineMethod::DUAL_PASS:           return "Dual-pass (Geometry + Sobel)";
        case OutlineMethod::HYBRID:              return "Hybrid (CPU + GPU)";
        default:                                 return "Unknown";
    }
}

} // namespace Outline
} // namespace DongArch

#endif // DONGARCH_OUTLINE_EXTRACTOR_H
