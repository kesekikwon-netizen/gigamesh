# 3D 알고리즘 학술 논문 모음

DongArch3D 프로젝트를 위한 핵심 알고리즘 학술 논문

Semantic Scholar를 통해 수집한 50개 논문 목록

## 1. Mesh-Plane Intersection (메시-평면 교차)

**Phase 2 (Cutline) 핵심 알고리즘**

GigaMesh `mesh.cpp:4041` 함수와 함께 참고

### 추천 논문 Top 3

#### 1️⃣ Fast exact parallel 3D mesh intersection algorithm using only orientation predicates (2017)
- **저자**: S. V. G. Magalhães, W. Randolph Franklin, M. Andrade
- **인용수**: 9
- **링크**: https://www.semanticscholar.org/paper/17ee83cf45d5e6a322686798e8913257b9d05440
- **DOI**: 10.1145/3139958.3140001
- **요약**: 병렬 처리 가능한 빠른 3D 메시 교차 알고리즘
- **DongArch3D 적용**: Cutline 추출 성능 최적화

#### 2️⃣ PaMO: Parallel Mesh Optimization for Intersection‐Free Low‐Poly Modeling on the GPU (2025)
- **저자**: Seonghun Oh, Xiao Yuan, Xinyue Wei, Ruoxi Shi, Fanbo Xiang, Minghua Liu, Hao Su
- **인용수**: 0 (최신 논문)
- **링크**: https://www.semanticscholar.org/paper/8ff0931c5fc85df5e4a72aea5ef312ce6923ea22
- **DOI**: 10.1111/cgf.70267
- **요약**: GPU 기반 병렬 메시 최적화
- **DongArch3D 적용**: OpenGL Compute Shader 활용 가능

#### 3️⃣ Instant Self-Intersection Repair for 3D Meshes (2025)
- **저자**: W. Jang, Yucheol Jung, Gyeongmin Lee, Seungyong Lee
- **링크**: https://www.semanticscholar.org/paper/facdd7bcd08198aed56c4f8840d4bbe4fddcce92
- **DOI**: 10.1145/3731427
- **요약**: 메시 자가 교차 즉시 수정
- **DongArch3D 적용**: 메시 후처리 단계

### 전체 논문 목록 (10개)

1. Automatic Features Characterization from 3D Facial Images (2010) - 0 citations
2. Calculation of Vessel's Geometric Properties Based on STL Model (N/A) - 0 citations
3. **PaMO: Parallel Mesh Optimization (2025) - 0 citations** ⭐
4. Textured Mesh Saliency (2024) - 2 citations
5. KerGen: A Kernel Computation Algorithm for 3D Polygon Meshes (2024) - 2 citations
6. **Fast exact parallel 3D mesh intersection (2017) - 9 citations** ⭐⭐⭐
7. **Instant Self-Intersection Repair (2025) - 0 citations** ⭐
8. NeuralSlice: Neural 3D Triangle Mesh Reconstruction (2023) - 1 citation
9. Supervertex Sampling Network (2023) - 0 citations
10. A new fully projective O(lg N) line convex polygon intersection (2024) - 2 citations

---

## 1.5 GigaMesh 창시자 논문 (MUST READ) ⭐⭐⭐⭐⭐

**DongArch3D의 기반 시스템**

#### 🎯 GigaMesh and Gilgamesh – 3D Multiscale Integral Invariant Cuneiform Character Extraction (2010)
- **저자**: Hubert Mara, Susanne Krömker, Stefan Jakob, Bernd Breuckmann
- **학회**: VAST 2010 (Eurographics)
- **링크**: https://diglib.eg.org/server/api/core/bitstreams/ed31e053-049a-483a-b576-05a5ff91ac62/content
- **DOI**: 10.2312/VAST/VAST10/131-138
- **페이지**: 131-138
- **요약**: GigaMesh 시스템의 창시 논문. 고해상도 3D 스캔 데이터 처리, Multiscale Integral Invariant (MSII) 필터링, 자기상관 기반 문자 추출
- **핵심 알고리즘**:
  - **MSII Feature Vector**: 다중 스케일 구체(sphere)를 이용한 특징 추출
  - **Autocorrelation**: 자기상관으로 문자 경계 자동 검출
  - **Voxel + Sparse Matrix**: CPU 캐시 최적화
  - **POSIX Threads**: 병렬 처리 (Amdahl's law 초과 성능)
- **성능**:
  - 256³ voxel space
  - n=10 scales
  - O(r_n log(r_n)) marching front
  - 5-45분 처리 시간 (태블릿 크기 따라)
- **DongArch3D 적용**:
  - **Phase 2 (Cutline)**: Mesh-Plane Intersection (이미 구현 ✅)
  - **Phase 3 (Outline)**: MSII 알고리즘 활용 예정
  - **Phase 4 (Clip)**: Feature Vector 분류
  - **Phase 5 (Vis)**: Distance Map
  - **GPU 가속**: 논문 6장에서 CUDA/OpenCL 제안 (우리가 구현 예정)
- **참고 Figure**:
  - Figure 3: Multiscale spheres (5 radii)
  - Figure 4: Feature distance & correlation
  - Figure 5: Autocorrelation & contour lines
- **참고 Equation**:
  - Equation 2: Feature vector f_i = (3/4π) * [V_r1/r_1³, ..., V_rn/r_n³]
  - Equation 4: Autocorrelation R_i(l) = Σ_j v_j * v_{l-j}

---

## 2. Silhouette Edge Detection (실루엣 엣지 검출)

**Phase 3 (Outline) 핵심 알고리즘**

### 추천 논문 Top 3

#### 1️⃣ Silhouette Smoothing for Real-Time Rendering of Mesh Surfaces (2008)
- **저자**: Lu Wang, Changhe Tu, Wenping Wang, Xiangxu Meng, B. Chan, Dong‐Ming Yan
- **인용수**: 9
- **링크**: https://www.semanticscholar.org/paper/233d732fbeffea4f824096da2d9e9fc3f5ad5830
- **DOI**: 10.1109/TVCG.2008.8
- **요약**: 실시간 메시 실루엣 스무딩 렌더링
- **DongArch3D 적용**: Outline 추출 후 스무딩 처리

#### 2️⃣ A Highly Parallelized Approach to Silhouette Edge Detection (2008)
- **저자**: Chad Mourning
- **인용수**: 3
- **링크**: https://www.semanticscholar.org/paper/5731278ad683660d6d31784d42d4e243546b8ec8
- **요약**: GPU 병렬 처리 실루엣 검출
- **DongArch3D 적용**: OpenGL Compute Shader 활용

#### 3️⃣ Quadric-Based Silhouette Sampling for Differentiable Rendering (2025)
- **저자**: Mariia Soroka, Christoph Peters, Steve Marschner
- **링크**: https://www.semanticscholar.org/paper/197a8c2a56ff8db76edfaf74f48ba51210bdb009
- **DOI**: 10.1145/3731146
- **요약**: 미분 가능 렌더링을 위한 실루엣 샘플링
- **DongArch3D 적용**: 머신러닝 기반 실루엣 추출 (선택)

### 전체 논문 목록 (10개)

1. **Quadric-Based Silhouette Sampling (2025) - 0 citations** ⭐
2. **A Highly Parallelized Approach (2008) - 3 citations** ⭐⭐
3. Soft Rasterizer: Differentiable Rendering (2019) - 96 citations
4. **Silhouette Smoothing for Real-Time Rendering (2008) - 9 citations** ⭐⭐⭐
5. A Fast Silhouette Detection Algorithm for Shadow Volumes (2016) - 2 citations
6. Real-Time Object-Space Edge Detection using OpenCL (2013) - 0 citations
7. Fast Image‐Space Silhouette Extraction for NPR (2015) - 3 citations
8. Effect of Curvature Estimations in Edge Detection (2012) - 0 citations
9. Efficient multi-view ray tracing using edge detection (2011) - 6 citations
10. Edge Restoration of 3D Building Model (2022) - 3 citations

---

## 3. Douglas-Peucker Algorithm (라인 간략화)

**Phase 2 (Cutline) Polyline 간략화 알고리즘**

### 추천 논문 Top 3

#### 1️⃣ Topologically Consistent Line Simplification with the Douglas-Peucker Algorithm (1999)
- **저자**: A. Saalfeld
- **인용수**: 268 ⭐⭐⭐⭐⭐ (가장 많이 인용된 논문)
- **링크**: https://www.semanticscholar.org/paper/9ea40237a48f3c2091f03866636a7548fcdf0b59
- **DOI**: 10.1559/152304099782424901
- **요약**: 위상학적으로 일관된 Douglas-Peucker 알고리즘
- **DongArch3D 적용**: Cutline Polyline 간략화 시 필수 참고

#### 2️⃣ The Douglas-Peucker Algorithm: Sufficiency Conditions for Non-Self-Intersections (2004)
- **저자**: Shin-Ting Wu, Adler C. G. da Silva, M. Márquez
- **인용수**: 28
- **링크**: https://www.semanticscholar.org/paper/81470c6ec2d15d06c1bad6c647242d01d241d99a
- **DOI**: 10.1590/S0104-65002004000100006
- **요약**: 자가 교차를 방지하는 충분 조건
- **DongArch3D 적용**: 간략화 후 라인 품질 보장

#### 3️⃣ Simplipoly: Curvature-Based Polygonal Curve Simplification (2011)
- **저자**: Chansophea Chuon, S. Guha, P. Janecek, Nguyen Duc Cong Song
- **인용수**: 9
- **링크**: https://www.semanticscholar.org/paper/4d4dcf88057b1bef71c4bb9661d90d18326083a3
- **DOI**: 10.1142/S0218195911003743
- **요약**: 곡률 기반 Polygon 간략화 (Douglas-Peucker 개선)
- **DongArch3D 적용**: 더 나은 품질의 간략화

### 전체 논문 목록 (10개)

1. **Topologically Consistent Line Simplification (1999) - 268 citations** ⭐⭐⭐⭐⭐
2. **The Douglas-Peucker Algorithm: Sufficiency Conditions (2004) - 28 citations** ⭐⭐⭐
3. A MASSIVELY PARALLEL LINE SIMPLIFICATION ALGORITHM (2011) - 1 citation
4. A Massively Parallel Algorithm for Polyline Simplification (2011) - 3 citations
5. Algorithm for automated line simplification based on QTM (2005) - 1 citation
6. Efficient and consistent line simplification for web mapping (2007) - 12 citations
7. Comparative study of Fourier descriptor algorithm (2023) - 0 citations
8. **Simplipoly: Curvature-Based Simplification (2011) - 9 citations** ⭐⭐
9. Fast Arc Detection Algorithm for PCB Bare Board (2014) - 0 citations
10. Efficiently Generating Multiple Representations (2005) - 10 citations

---

## 4. Catmull-Rom Spline (스플라인 보간)

**Phase 2 (Cutline) 곡선 스무딩 알고리즘**

### 추천 논문 Top 3

#### 1️⃣ Fast Catmull‐Rom Spline Interpolation for High‐Quality Texture Sampling (2018)
- **저자**: B. Csébfalvi
- **인용수**: 4
- **링크**: https://www.semanticscholar.org/paper/cdcbd0aa96632c5766a8f9b75ead7d06f0b6dfc2
- **DOI**: 10.1111/cgf.13375
- **요약**: 빠른 Catmull-Rom 보간 (GPU 최적화)
- **DongArch3D 적용**: 실시간 Cutline 곡선 스무딩

#### 2️⃣ X-splines: a spline model designed for the end-user (1995)
- **저자**: C. Blanc, C. Schlick
- **인용수**: 38
- **링크**: https://www.semanticscholar.org/paper/9720f9d9a18df94f79bd2ae2295c6bfdcbe57c95
- **DOI**: 10.1145/218380.218488
- **요약**: 사용자 친화적 스플라인 모델 (Catmull-Rom 개선)
- **DongArch3D 적용**: 인터랙티브 곡선 편집

#### 3️⃣ Identification of distributed dynamic excitation using Catmull–Rom spline (2020)
- **저자**: Xiaowang Li, Hai-tao Zhao, Zheng Chen, Ji'an Chen
- **인용수**: 10
- **링크**: https://www.semanticscholar.org/paper/9c994889eec4440f0e6cdeab6b8d00e84b1c2012
- **DOI**: 10.1080/17415977.2019.1594804
- **요약**: Catmull-Rom 스플라인을 이용한 동적 여기 식별
- **DongArch3D 적용**: 수학적 기반 이해

### 전체 논문 목록 (10개)

1. Quartic Catmull-Rom Spline Function with Local Parameters (N/A) - 0 citations
2. Automatic measurement system based on DeepLabCut (2025) - 0 citations
3. Approximating Pareto Set Topology by Cubic Interpolation (2019) - 1 citation
4. **X-splines: end-user spline model (1995) - 38 citations** ⭐⭐⭐
5. Recursive polynomial curve schemes (1990) - 15 citations
6. Information Technique for Curve Modeling (2020) - 0 citations
7. Comparison of splines for stem form modeling (2014) - 8 citations
8. **Identification using Catmull–Rom spline (2020) - 10 citations** ⭐⭐
9. Hardware Implementation using Catmull-Rom Spline (2020) - 6 citations
10. **Fast Catmull‐Rom Spline Interpolation (2018) - 4 citations** ⭐⭐⭐

---

## 5. Geodesic Distance (측지선 거리)

**Phase 5 (D-Tak) 핵심 알고리즘**

GigaMesh `edgegeodesic.cpp` 함수와 함께 참고

### 추천 논문 Top 3

#### 1️⃣ Parallel and Scalable Heat Methods for Geodesic Distance Computation (2018)
- **저자**: J. Tao, Juyong Zhang, Bailin Deng, Zheng Fang, Yue Peng, Ying He
- **인용수**: 29
- **링크**: https://www.semanticscholar.org/paper/7876725f720405a78a6f18b619fdbde852d6e9d3
- **DOI**: 10.1109/TPAMI.2019.2933209
- **요약**: 병렬 및 확장 가능한 Heat Method (빠른 측지선 계산)
- **DongArch3D 적용**: D-Tak 성능 최적화

#### 2️⃣ Continuous Shortest Path Vector Field Navigation on 3D Triangular Meshes (2021)
- **저자**: Sebastian Pütz, T. Wiemann, Malte Kleine Piening, J. Hertzberg
- **인용수**: 24
- **링크**: https://www.semanticscholar.org/paper/2521c055cf32bb365a6fcb05110a0d7f17156d99
- **DOI**: 10.1109/ICRA48506.2021.9560981
- **요약**: 3D 메시 상의 연속 최단 경로 벡터 필드 탐색
- **DongArch3D 적용**: 메시 상 경로 계획

#### 3️⃣ Geodesic Distance Computation via Virtual Source Propagation (2021)
- **저자**: P. Trettner, D. Bommes, L. Kobbelt
- **인용수**: 14
- **링크**: https://www.semanticscholar.org/paper/c743a2ee451dcfdcc7e89f49f6dbf612683a0bd7
- **DOI**: 10.1111/cgf.14371
- **요약**: 가상 소스 전파를 통한 측지선 거리 계산
- **DongArch3D 적용**: 정확도 향상

### 전체 논문 목록 (10개)

1. Enhanced Heat Method for Geodesic Distance (2022) - 0 citations
2. **Continuous Shortest Path Vector Field Navigation (2021) - 24 citations** ⭐⭐⭐
3. Continuous Multilayer Shortest Path (2020) - 0 citations
4. Computation Method of Human Body 3D Reeb Graph (2011) - 1 citation
5. Skeleton-based 3D Model Descriptor (2019) - 0 citations
6. A Parallel Algorithm for 3D Topology Information (2013) - 0 citations
7. **Geodesic Distance Computation via Virtual Source (2021) - 14 citations** ⭐⭐
8. Geodesic distance-based pose-invariant blind watermarking (2010) - 2 citations
9. PENERAPAN METODE FAST MARCHING (2006) - 0 citations
10. **Parallel and Scalable Heat Methods (2018) - 29 citations** ⭐⭐⭐⭐

---

## Phase별 논문 활용 계획

| Phase | 알고리즘 | 추천 논문 | 우선순위 |
|-------|---------|----------|----------|
| **Phase 2** | Mesh-Plane Intersection | Fast exact parallel (2017) | ⭐⭐⭐⭐⭐ |
| **Phase 2** | Douglas-Peucker | Topologically Consistent (1999) | ⭐⭐⭐⭐⭐ |
| **Phase 2** | Catmull-Rom Spline | Fast Catmull-Rom (2018) | ⭐⭐⭐⭐ |
| **Phase 3** | Silhouette Detection | Silhouette Smoothing (2008) | ⭐⭐⭐⭐⭐ |
| **Phase 5** | Geodesic Distance | Parallel Heat Methods (2018) | ⭐⭐⭐⭐ |

## 논문 읽기 순서 추천

### Week 1: Cutline 알고리즘 (Phase 2 준비)
1. **Day 1-2**: Topologically Consistent Douglas-Peucker (1999) ⭐⭐⭐⭐⭐
   - Polyline 간략화 핵심 이론
2. **Day 3-4**: Fast exact parallel mesh intersection (2017) ⭐⭐⭐⭐⭐
   - Mesh-Plane Intersection 최적화
3. **Day 5-6**: Fast Catmull-Rom Spline (2018) ⭐⭐⭐⭐
   - 곡선 스무딩
4. **Day 7**: GigaMesh 소스 코드 분석 (mesh.cpp:4041)

### Week 2: Outline 알고리즘 (Phase 3 준비)
5. **Day 1-3**: Silhouette Smoothing for Real-Time Rendering (2008) ⭐⭐⭐⭐⭐
   - 실루엣 검출 및 스무딩
6. **Day 4-5**: A Highly Parallelized Silhouette Detection (2008) ⭐⭐
   - GPU 병렬 처리
7. **Day 6-7**: GigaMesh NPR 셰이더 분석 (NPR_hatches.frag, NPR_ApplySobel.frag)

### Week 3: Geodesic & 고급 기법 (Phase 5 준비)
8. **Day 1-4**: Parallel and Scalable Heat Methods (2018) ⭐⭐⭐⭐
   - Geodesic Distance 계산
9. **Day 5-7**: GigaMesh edgegeodesic.cpp 분석

## 추가 학습 자료

### 온라인 강의
- **Computational Geometry (Stanford)**: https://web.stanford.edu/class/cs468/
- **Discrete Differential Geometry (CMU)**: https://www.cs.cmu.edu/~kmcrane/Projects/DDG/

### 참고 서적
- **Real-Time Rendering (4th Edition)**: Silhouette 렌더링 챕터
- **Polygon Mesh Processing**: 메시 알고리즘 전반

---

**생성일**: 2025-11-08
**프로젝트**: DongArch3D v2.0.0 (Arch3D Liner 기반)
**수집 논문 수**: 50개 (Semantic Scholar)
**최고 인용 논문**: Douglas-Peucker (1999) - 268 citations
