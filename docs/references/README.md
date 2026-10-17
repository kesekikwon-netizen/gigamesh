# DongArch3D 기술 문서 모음

**동국문화재연구원 전용 3D 실측 프로그램 개발을 위한 기술 레퍼런스**

이 폴더는 DongArch3D v2.0.0 (Arch3D Liner 기반) 프로젝트 개발에 필요한 모든 기술 문서와 학술 논문을 정리한 것입니다.

## 📚 문서 구조

```
docs/references/
├── README.md (이 파일)
├── cpp/
│   └── cpp20-resources.md           # C++20 최신 기능 가이드
├── qt/
│   └── qt5.15-resources.md          # Qt 5.15 프레임워크 문서
├── opengl/
│   └── opengl3.3-resources.md       # OpenGL 3.3 Core Profile
├── cmake/
│   └── cmake-resources.md           # CMake 빌드 시스템
└── algorithms/
    └── research-papers.md           # 3D 알고리즘 학술 논문 50개
```

## 🚀 빠른 시작

### 프로젝트를 처음 시작하는 경우

**학습 순서**:
1. **CMake** (1주) → 빌드 시스템 이해
2. **C++20** (1-2주) → 최신 C++ 기능
3. **Qt 5.15** (2주) → GUI 프레임워크
4. **OpenGL 3.3** (2주) → 3D 렌더링
5. **알고리즘 논문** (지속적) → 핵심 알고리즘 상세

### Phase별 필수 문서

| Phase | 필수 문서 | 우선순위 |
|-------|----------|----------|
| **Phase 0** | CMake, C++20 기초 | ⭐⭐⭐⭐⭐ |
| **Phase 1** | Qt Signals/Slots, OpenGL MVP 행렬 | ⭐⭐⭐⭐⭐ |
| **Phase 2** | Mesh-Plane Intersection 논문, Douglas-Peucker | ⭐⭐⭐⭐⭐ |
| **Phase 3** | Silhouette Detection 논문, OpenGL NPR | ⭐⭐⭐⭐⭐ |
| **Phase 4** | Mesh Split (GigaMesh 소스) | ⭐⭐⭐ |
| **Phase 5** | Geodesic Distance 논문 | ⭐⭐⭐ |
| **Phase 6** | Qt 고급 위젯 | ⭐⭐ |
| **Phase 7** | SVG Export (GigaMesh 소스) | ⭐⭐⭐⭐ |

## 📖 카테고리별 문서

### 1. C++20 리소스 ([cpp/cpp20-resources.md](cpp/cpp20-resources.md))

**왜 C++20인가?**
- 현재 프로젝트는 C++17 기반이지만, 새로운 DongArch3D 코드는 C++20 사용
- Concepts, Ranges, std::span 등 강력한 기능으로 코드 품질 향상

**핵심 내용**:
- C++20 주요 기능 5가지 (Concepts, Ranges, std::span, Designated Initializers, Constexpr 확장)
- DongArch3D 적용 예시
- GigaMesh (C++17)와 호환성
- 컴파일러 요구사항 (GCC 10+, MSVC 19.20+)

**학습 시간**: 1-2주

**추천 학습 순서**:
1. std::span (1일)
2. Designated Initializers (1일)
3. Ranges 기본 (3일)
4. Concepts 기본 (3-4일)

---

### 2. Qt 5.15 리소스 ([qt/qt5.15-resources.md](qt/qt5.15-resources.md))

**왜 Qt 5.15인가?**
- GigaMesh 기반 프로젝트이므로 Qt 5.15.2 사용
- 크로스 플랫폼 GUI 프레임워크 (Windows, Linux, macOS)

**핵심 내용**:
- QOpenGLWidget (3D 렌더링 위젯)
- Signals & Slots (이벤트 시스템)
- QMainWindow 구조
- Qt 국제화 (한국어 번역)
- CMake와 Qt 통합

**학습 시간**: 2주

**추천 학습 순서**:
1. Signals & Slots (2일)
2. QMainWindow, QDockWidget (2일)
3. QOpenGLWidget (3일)
4. Qt Resource System (1일)
5. Internationalization (2일)

---

### 3. OpenGL 3.3 리소스 ([opengl/opengl3.3-resources.md](opengl/opengl3.3-resources.md))

**왜 OpenGL 3.3 Core Profile인가?**
- 고정 파이프라인 제거, 셰이더 기반 렌더링
- GigaMesh가 OpenGL 3.3 사용
- NPR (Non-Photorealistic Rendering) 지원

**핵심 내용**:
- VBO/VAO (정점 버퍼)
- GLSL 셰이더 (Vertex, Fragment)
- Uniform 변수
- NPR 렌더링 (Hatching, Silhouette)
- Qt + OpenGL 통합

**학습 시간**: 2주

**추천 학습 순서**:
1. VBO/VAO 기초 (2일)
2. 기본 셰이더 작성 (2일)
3. MVP 행렬 (2일)
4. 조명 (Phong) (2일)
5. NPR 셰이더 (3일)

**추천 온라인 튜토리얼**:
- LearnOpenGL: https://learnopengl.com/ (최고의 튜토리얼)

---

### 4. CMake 리소스 ([cmake/cmake-resources.md](cmake/cmake-resources.md))

**왜 CMake인가?**
- 크로스 플랫폼 빌드 시스템
- Qt, OpenGL 의존성 자동 관리
- GigaMesh 프로젝트 구조

**핵심 내용**:
- CMakeLists.txt 구조
- find_package (Qt5, OpenGL)
- target_link_libraries (Modern CMake)
- Qt AUTOMOC, AUTORCC, AUTOUIC
- 빌드 명령어 (Windows MSVC, Linux GCC, macOS Clang)

**학습 시간**: 1주

**추천 학습 순서**:
1. CMake 기초 (2일)
2. find_package, target_link_libraries (1일)
3. Qt 통합 (2일)
4. Generator Expressions (1일)

---

### 5. 3D 알고리즘 학술 논문 ([algorithms/research-papers.md](algorithms/research-papers.md))

**왜 학술 논문인가?**
- Arch3D Liner 핵심 알고리즘의 이론적 기반
- GigaMesh 소스 코드 이해를 위한 배경 지식
- 최신 연구 동향 파악

**수록 논문 수**: 50개 (Semantic Scholar 검색)

**카테고리**:
1. **Mesh-Plane Intersection** (10개) - Phase 2 Cutline
2. **Silhouette Edge Detection** (10개) - Phase 3 Outline
3. **Douglas-Peucker Algorithm** (10개) - Phase 2 Polyline 간략화
4. **Catmull-Rom Spline** (10개) - Phase 2 곡선 스무딩
5. **Geodesic Distance** (10개) - Phase 5 D-Tak

**Top 5 추천 논문** (반드시 읽어야 함):
1. **Topologically Consistent Douglas-Peucker (1999)** - 268 citations ⭐⭐⭐⭐⭐
   - Polyline 간략화 필수 이론
2. **Silhouette Smoothing for Real-Time Rendering (2008)** - 9 citations ⭐⭐⭐⭐⭐
   - 실루엣 렌더링 핵심
3. **Parallel and Scalable Heat Methods (2018)** - 29 citations ⭐⭐⭐⭐
   - Geodesic Distance 최적화
4. **Fast exact parallel mesh intersection (2017)** - 9 citations ⭐⭐⭐⭐⭐
   - Mesh-Plane Intersection 최적화
5. **Fast Catmull-Rom Spline Interpolation (2018)** - 4 citations ⭐⭐⭐⭐
   - 실시간 곡선 스무딩

**학습 시간**: 지속적 (각 Phase 시작 전 관련 논문 읽기)

---

## 🎯 Phase별 학습 로드맵

### Phase 0: 기반 시스템 (4일)
**필수 문서**:
- ✅ CMake 기초 (cmake-resources.md)
- ✅ C++20 기초 (cpp20-resources.md)
- ✅ Qt 기초 (qt5.15-resources.md)

**목표**:
- DongArch3D 빌드 성공
- 기본 GUI 창 띄우기

---

### Phase 1: Align (정렬) (10일)
**필수 문서**:
- ✅ Qt Signals & Slots (qt5.15-resources.md)
- ✅ QOpenGLWidget (qt5.15-resources.md)
- ✅ OpenGL MVP 행렬 (opengl3.3-resources.md)

**목표**:
- 메시 로드 및 표시
- 회전/이동/줌 인터랙션
- Top/Front/Right 뷰 전환

---

### Phase 2: Cutline (단면 라인) (8일) ⭐ 최우선
**필수 문서**:
- ✅ Mesh-Plane Intersection 논문 (research-papers.md #1)
- ✅ Douglas-Peucker 논문 (research-papers.md #3)
- ✅ Catmull-Rom Spline 논문 (research-papers.md #4)
- ✅ GigaMesh mesh.cpp:4041 소스 분석

**추천 논문**:
1. Fast exact parallel mesh intersection (2017) ⭐⭐⭐⭐⭐
2. Topologically Consistent Douglas-Peucker (1999) ⭐⭐⭐⭐⭐
3. Fast Catmull-Rom Spline Interpolation (2018) ⭐⭐⭐⭐

**목표**:
- Top/Front/Right Cut 구현
- Polyline 간략화 (Douglas-Peucker)
- 곡선 스무딩 (Catmull-Rom)

---

### Phase 3: Outline (외곽 라인) (8일) ⭐ 최우선
**필수 문서**:
- ✅ Silhouette Detection 논문 (research-papers.md #2)
- ✅ OpenGL NPR 셰이더 (opengl3.3-resources.md)
- ✅ GigaMesh NPR 셰이더 (NPR_ApplySobel.frag)

**추천 논문**:
1. Silhouette Smoothing for Real-Time Rendering (2008) ⭐⭐⭐⭐⭐
2. A Highly Parallelized Silhouette Detection (2008) ⭐⭐

**목표**:
- 6방향 실루엣 추출
- View-dependent 렌더링

---

### Phase 4: Clip (3D 메시 절단) (2일)
**필수 문서**:
- ✅ GigaMesh mesh.cpp:4017 (Mesh Split)
- ✅ OpenGL Clipping Plane (opengl3.3-resources.md)

---

### Phase 5: Vis (시각화) (7일)
**필수 문서**:
- ✅ Geodesic Distance 논문 (research-papers.md #5)
- ✅ GigaMesh edgegeodesic.cpp 소스 분석

**추천 논문**:
1. Parallel and Scalable Heat Methods (2018) ⭐⭐⭐⭐

**목표**:
- X-Ray 렌더링
- D-Tak (Geodesic Distance 시각화)

---

### Phase 6: MFE (Multi-Feature Editing) (5일)
**필수 문서**:
- ✅ Qt 고급 위젯 (qt5.15-resources.md)

---

### Phase 7: Illustrator 연동 (5일)
**필수 문서**:
- ✅ GigaMesh SVG Export (MeshWriter.cpp)
- ✅ SVG 1:1 스케일 계산

---

## 🔧 개발 환경 설정

### 필수 소프트웨어

#### Windows
- Visual Studio 2019/2022 (MSVC 19.20+)
- Qt 5.15.2 (https://www.qt.io/download)
- CMake 3.10+ (https://cmake.org/download/)
- Git

#### Linux (Ubuntu 20.04+)
```bash
# GCC 10+ (C++20 필수)
sudo apt install g++-10 gcc-10

# Qt 5.15
sudo apt install qtbase5-dev qttools5-dev

# CMake
sudo apt install cmake

# OpenGL
sudo apt install libgl1-mesa-dev
```

#### macOS
```bash
# Homebrew로 설치
brew install cmake
brew install qt@5
brew install gcc@10  # Apple Clang 13+도 가능
```

---

## 📝 빌드 명령어 요약

### Windows (MSVC)
```bash
# 1. Qt 경로 설정
set CMAKE_PREFIX_PATH=C:/Qt/5.15.2/msvc2019_64

# 2. CMake 구성
cmake -G "Visual Studio 17 2022" -A x64 -B build_korean

# 3. 빌드
cmake --build build_korean --config Release --target DongArch3D
```

### Linux (GCC 10)
```bash
# 1. 컴파일러 설정
export CXX=/usr/bin/g++-10
export CC=/usr/bin/gcc-10

# 2. CMake 구성
cmake -DCMAKE_BUILD_TYPE=Release -B build

# 3. 빌드
cmake --build build --parallel $(nproc)
```

### macOS
```bash
# 1. Qt 경로 설정
export CMAKE_PREFIX_PATH=/usr/local/opt/qt@5

# 2. CMake 구성
cmake -DCMAKE_BUILD_TYPE=Release -B build

# 3. 빌드
cmake --build build --parallel $(sysctl -n hw.ncpu)
```

---

## 🌐 온라인 리소스

### 공식 문서
- **C++ Reference**: https://en.cppreference.com/
- **Qt Documentation**: https://doc.qt.io/qt-5/
- **OpenGL Reference**: https://www.khronos.org/opengl/
- **CMake Documentation**: https://cmake.org/cmake/help/latest/

### 온라인 튜토리얼
- **LearnOpenGL** (최고의 OpenGL 튜토리얼): https://learnopengl.com/
- **Modern CMake**: https://cliutils.gitlab.io/modern-cmake/
- **Qt Examples**: https://doc.qt.io/qt-5/qtexamples.html
- **C++ Stories** (C++20): https://www.cppstories.com/p/cpp20.html

### GitHub 리포지토리
- **GigaMesh**: https://gitlab.com/fcgl/GigaMesh
- **GLM (Math Library)**: https://github.com/g-truc/glm
- **cglm (C Math Library)**: https://github.com/recp/cglm

---

## 📊 문서 수집 통계

### 총 수집 문서 수

| 카테고리 | 문서 수 | 출처 |
|---------|--------|------|
| C++20 | 15개 링크 | Microsoft Docs, cppreference |
| Qt 5.15 | 20개 링크 | Qt Official, GitHub |
| OpenGL 3.3 | 15개 링크 | Khronos, LearnOpenGL |
| CMake | 15개 링크 | CMake Official, GitHub |
| 학술 논문 | 50개 | Semantic Scholar |
| **총계** | **115개** | |

### 학술 논문 통계

- **총 논문 수**: 50개
- **평균 인용수**: 12.3
- **최고 인용 논문**: Douglas-Peucker (1999) - 268 citations
- **최신 논문**: 2025년 발표 4개

---

## 🎓 학습 체크리스트

### C++20 (1-2주)
- [ ] std::span 이해 및 활용
- [ ] Designated Initializers 사용
- [ ] Ranges 기본 (filter, transform)
- [ ] Concepts 기본 이해

### Qt 5.15 (2주)
- [ ] Signals & Slots 마스터
- [ ] QMainWindow + QDockWidget 구조
- [ ] QOpenGLWidget 기본 렌더링
- [ ] Qt Resource System (.qrc)
- [ ] Qt 국제화 (한국어 번역)

### OpenGL 3.3 (2주)
- [ ] VBO/VAO 생성 및 사용
- [ ] 기본 Vertex/Fragment 셰이더 작성
- [ ] MVP 행렬 이해
- [ ] Uniform 변수 전달
- [ ] 텍스처 매핑
- [ ] NPR 셰이더 (Hatching, Sobel)

### CMake (1주)
- [ ] CMakeLists.txt 기본 구조 이해
- [ ] find_package 사용
- [ ] target_link_libraries (Modern CMake)
- [ ] Qt AUTOMOC/AUTORCC/AUTOUIC
- [ ] 빌드 명령어 숙지

### 알고리즘 논문 (지속적)
- [ ] Douglas-Peucker (1999) 읽기 ⭐⭐⭐⭐⭐
- [ ] Silhouette Smoothing (2008) 읽기 ⭐⭐⭐⭐⭐
- [ ] Fast mesh intersection (2017) 읽기 ⭐⭐⭐⭐⭐
- [ ] Parallel Heat Methods (2018) 읽기 ⭐⭐⭐⭐
- [ ] Fast Catmull-Rom Spline (2018) 읽기 ⭐⭐⭐⭐

---

## 💡 학습 팁

### 1. 실습 위주로 학습하기
- 문서를 읽기만 하지 말고 즉시 코드로 작성
- 작은 테스트 프로그램 만들어보기

### 2. GigaMesh 소스 코드 적극 활용
- 이미 70% 구현되어 있음
- mesh.cpp, meshGL.cpp, meshGLShader.cpp 분석

### 3. Phase별 우선순위 지키기
- Phase 2 (Cutline), Phase 3 (Outline)이 가장 중요
- 이 두 Phase에 집중

### 4. 커뮤니티 활용
- LearnOpenGL Discord
- Qt Forum
- Stack Overflow

---

## 🚨 주의사항

### C++ 버전
- DongArch3D는 C++20 사용 예정 (업그레이드 필요)
- GigaMesh Core는 C++17 유지
- 혼용 가능 (C++ 표준 상위 호환)

### Qt 버전
- **Qt 5.15.2** 사용 (Qt 6 아님!)
- Qt 6 문서 참고 시 API 변경사항 주의

### OpenGL 버전
- **OpenGL 3.3 Core Profile** 사용
- 고정 파이프라인 함수 (glBegin, glVertex 등) 사용 금지

### 빌드 경로
- Windows: `A:/1105/GigaMesh/build_korean/gui/Release/DongArch3D.exe`
- Linux: `A:/1105/GigaMesh/build/gui/DongArch3D`

---

## 📞 도움이 필요한 경우

### 프로젝트 문서
- **개발 계획**: `A:/1105/DongArch3D_개발계획_v3_Arch3DLiner기반.md`
- **구현 가능성 평가**: `A:/1105/Arch3D_Liner_구현가능성_평가_GigaMesh기반.md`
- **CLAUDE.md**: `A:/1105/CLAUDE.md` (프로젝트 가이드)
- **GigaMesh CLAUDE.md**: `A:/1105/GigaMesh/CLAUDE.md` (GigaMesh 가이드)

### .claude/ 폴더
- `A:/1105/.claude/architecture.md`: 시스템 구조 (재작성 예정)
- `A:/1105/.claude/guidelines.md`: 코딩 규칙 (재작성 예정)

---

**문서 생성일**: 2025-11-08
**프로젝트**: DongArch3D v2.0.0 (Arch3D Liner 기반)
**MCP 사용**: docfork-mcp, semantic scholar
**총 수집 문서**: 115개 (공식 문서 65개 + 학술 논문 50개)

**다음 단계**: CMakeLists.txt를 C++20으로 업그레이드 후 개발 시작! 🚀
