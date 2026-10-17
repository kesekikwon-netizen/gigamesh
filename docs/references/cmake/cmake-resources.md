# CMake 리소스 및 문서

DongArch3D 프로젝트를 위한 CMake 빌드 시스템 문서 모음

## 프로젝트 CMake 정보

- **CMake 버전**: 3.10+ (최소), 3.20+ (권장)
- **빌드 시스템**: Ninja (Windows), Make (Linux/macOS)
- **C++ 표준**: C++20
- **주요 의존성**: Qt 5.15, OpenGL 3.3

## CMake 기본 구조

### DongArch3D 프로젝트 구조
```
A:/1105/GigaMesh/
├── CMakeLists.txt (루트)
├── core/
│   └── CMakeLists.txt
├── gui/
│   └── CMakeLists.txt
└── cli/
    └── CMakeLists.txt
```

### 루트 CMakeLists.txt (GigaMesh/CMakeLists.txt)
```cmake
cmake_minimum_required(VERSION 3.10)

project(DongArch3D VERSION 2.0.0 LANGUAGES CXX C
        DESCRIPTION "동국문화재연구원 전용 실측 프로그램")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 서브디렉토리 추가
add_subdirectory(external)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(cli)
```

### GUI CMakeLists.txt (GigaMesh/gui/CMakeLists.txt)
```cmake
# Qt 찾기
find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL Network REQUIRED)

# Qt 자동 처리
set(CMAKE_AUTOMOC ON)  # Meta-Object Compiler
set(CMAKE_AUTORCC ON)  # Resource Compiler
set(CMAKE_AUTOUIC ON)  # UI Compiler

# 소스 파일
set(GUI_SOURCES
    src/main.cpp
    src/QGMMainWindow.cpp
    src/meshwidget.cpp
    # ... 더 많은 파일
)

# 리소스 파일
qt5_add_resources(GUI_RESOURCES
    forms/gigamesh.qrc
    languages/translations.qrc
)

# 실행 파일
add_executable(DongArch3D ${GUI_SOURCES} ${GUI_RESOURCES})

# 라이브러리 링크
target_link_libraries(DongArch3D PRIVATE
    Qt5::Widgets
    Qt5::OpenGL
    gigameshCore
)
```

## Modern CMake 패턴

### 1. Target-based 설계
```cmake
# ❌ 구식 방법
include_directories(${Qt5_INCLUDE_DIRS})
link_directories(${Qt5_LIBRARY_DIRS})

# ✅ 올바른 방법 (Modern CMake)
target_link_libraries(DongArch3D PRIVATE Qt5::Widgets)
# Qt5::Widgets 타겟이 자동으로 include, lib 경로 전파
```

### 2. Generator Expressions
```cmake
# 빌드 타입에 따라 다른 설정
target_compile_options(DongArch3D PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# 컴파일러별 설정
target_compile_options(DongArch3D PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
)
```

### 3. Interface Libraries
```cmake
# 헤더 전용 라이브러리
add_library(DongArchHeaders INTERFACE)
target_include_directories(DongArchHeaders INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(DongArchHeaders INTERFACE cxx_std_20)

# 사용
target_link_libraries(DongArch3D PRIVATE DongArchHeaders)
```

## Qt + CMake 통합

### find_package(Qt5)
```cmake
# Qt 찾기
find_package(Qt5 5.15 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    OpenGL
    Network
)

# Qt 버전 확인
if(Qt5_VERSION VERSION_LESS "5.15.0")
    message(FATAL_ERROR "Qt 5.15 or newer required, found ${Qt5_VERSION}")
endif()
```

### Qt 리소스 파일 (QRC)
```cmake
# 방법 1: qt5_add_resources (구식)
qt5_add_resources(QRC_SOURCES resources.qrc)
add_executable(DongArch3D main.cpp ${QRC_SOURCES})

# 방법 2: CMAKE_AUTORCC (권장)
set(CMAKE_AUTORCC ON)
add_executable(DongArch3D main.cpp resources.qrc)
```

### Qt UI 파일 (.ui)
```cmake
# 방법 1: qt5_wrap_ui (구식)
qt5_wrap_ui(UI_HEADERS mainwindow.ui)
add_executable(DongArch3D main.cpp ${UI_HEADERS})

# 방법 2: CMAKE_AUTOUIC (권장)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms)
add_executable(DongArch3D main.cpp)
```

### Qt 번역 파일 (.ts, .qm)
```cmake
# .ts 파일 업데이트
find_program(QT_LUPDATE_EXECUTABLE lupdate)
if(QT_LUPDATE_EXECUTABLE)
    add_custom_target(translations_update
        COMMAND ${QT_LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR}/gui/src
                -ts ${CMAKE_SOURCE_DIR}/gui/languages/DongArch3D_ko.ts
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# .qm 파일 컴파일
find_program(QT_LRELEASE_EXECUTABLE lrelease)
if(QT_LRELEASE_EXECUTABLE)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/DongArch3D_ko.qm
        COMMAND ${QT_LRELEASE_EXECUTABLE}
                ${CMAKE_SOURCE_DIR}/gui/languages/DongArch3D_ko.ts
                -qm ${CMAKE_BINARY_DIR}/DongArch3D_ko.qm
        DEPENDS ${CMAKE_SOURCE_DIR}/gui/languages/DongArch3D_ko.ts
    )
endif()
```

## OpenGL + CMake

### find_package(OpenGL)
```cmake
set(OpenGL_GL_PREFERENCE GLVND)  # Linux에서 GLVND 우선
find_package(OpenGL REQUIRED)

add_executable(DongArch3D main.cpp)
target_link_libraries(DongArch3D PRIVATE OpenGL::GL)
```

## 빌드 구성

### Multi-config Generators (Visual Studio, Xcode)
```bash
# 구성 시
cmake -G "Visual Studio 17 2022" -A x64 -B build_korean

# 빌드 시 (Release)
cmake --build build_korean --config Release

# 빌드 시 (Debug)
cmake --build build_korean --config Debug
```

### Single-config Generators (Make, Ninja)
```bash
# Release 빌드
cmake -DCMAKE_BUILD_TYPE=Release -B build_release
cmake --build build_release

# Debug 빌드
cmake -DCMAKE_BUILD_TYPE=Debug -B build_debug
cmake --build build_debug
```

## DongArch3D 빌드 명령어

### Windows (MSVC)
```bash
# Qt 경로 설정
set CMAKE_PREFIX_PATH=C:/Qt/5.15.2/msvc2019_64

# 구성
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% ^
      -B build_korean

# 빌드
cmake --build build_korean --config Release --target DongArch3D

# 병렬 빌드 (8코어)
cmake --build build_korean --config Release --parallel 8
```

### Linux (GCC 10+)
```bash
# Qt 경로 설정
export CMAKE_PREFIX_PATH=/opt/Qt/5.15.2/gcc_64

# GCC 10 지정 (C++20 필수)
export CXX=/usr/bin/g++-10
export CC=/usr/bin/gcc-10

# 구성
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_COMPILER=$CXX \
      -DCMAKE_C_COMPILER=$CC \
      -B build

# 빌드
cmake --build build --parallel $(nproc)
```

### macOS (Clang 13+)
```bash
# Homebrew Qt
export CMAKE_PREFIX_PATH=/usr/local/opt/qt@5

# 구성
cmake -DCMAKE_BUILD_TYPE=Release -B build

# 빌드
cmake --build build --parallel $(sysctl -n hw.ncpu)
```

## CMake 옵션 및 캐시

### 프로젝트 옵션 정의
```cmake
option(BUILD_CLI "Build the GigaMesh-CLI tools" ON)
option(BUILD_GUI "Build GigaMesh-GUI" ON)
option(ENABLE_TESTS "Build tests" ON)
option(ENABLE_ARCH3D_LINER "Enable Arch3D Liner features" ON)

# 사용
if(BUILD_GUI)
    add_subdirectory(gui)
endif()
```

### 명령줄에서 옵션 설정
```bash
# GUI만 빌드
cmake -DBUILD_CLI=OFF -DBUILD_GUI=ON -B build

# CLI만 빌드
cmake -DBUILD_CLI=ON -DBUILD_GUI=OFF -B build
```

### 캐시 변수 확인
```bash
# 캐시된 변수 목록
cmake -L build_korean

# 고급 변수 포함
cmake -LAH build_korean
```

## 의존성 관리

### FetchContent (CMake 3.11+)
```cmake
include(FetchContent)

# GLM 라이브러리 자동 다운로드
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

target_link_libraries(DongArch3D PRIVATE glm::glm)
```

### Git Submodules
```cmake
# Submodule 초기화 확인
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
```

## GitHub 참고 문서

수집된 문서:
- [Building CMake Projects: Tutorial](https://github.com/Kitware/CMake/blob/v4.1.1/Help/guide/tutorial/index.rst)
- [Defining Target Usage Requirements](https://github.com/Kitware/CMake/blob/v4.1.1/Help/manual/cmake-buildsystem.7.rst)
- [Setting C++ Standard using Interface Libraries](https://github.com/Kitware/CMake/blob/v4.1.1/Help/guide/tutorial/Adding%20Usage%20Requirements%20for%20a%20Library.rst)
- [Integrating with CMake FetchContent](https://github.com/nlohmann/json/blob/v3.12.0/docs/mkdocs/docs/integration/cmake.md)

## 학습 리소스

### 공식 문서
- **CMake 공식 문서**: https://cmake.org/cmake/help/latest/
- **CMake Tutorial**: https://cmake.org/cmake/help/latest/guide/tutorial/index.html
- **CMake Reference**: https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html

### Modern CMake 가이드
- **Modern CMake**: https://cliutils.gitlab.io/modern-cmake/
- **Effective Modern CMake**: https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1
- **CMake Best Practices**: https://github.com/cpp-best-practices/cmake_template

### 온라인 튜토리얼
- **An Introduction to Modern CMake**: https://cliutils.gitlab.io/modern-cmake/
- **CGold CMake**: https://cgold.readthedocs.io/

## CMake 학습 순서

### 1단계: 기초 (2-3일)
1. CMakeLists.txt 구조
2. add_executable, add_library
3. target_link_libraries
4. find_package

### 2단계: 프로젝트 구성 (3-4일)
5. 서브디렉토리 구조
6. option, 캐시 변수
7. Generator Expressions
8. 빌드 타입 (Debug/Release)

### 3단계: Qt 통합 (2-3일)
9. Qt 자동 처리 (AUTOMOC, AUTORCC, AUTOUIC)
10. Qt 리소스 파일
11. Qt 번역 파일

### 4단계: 고급 (1주)
12. FetchContent
13. Custom Commands/Targets
14. Export/Import
15. CPack (패키징)

## 디버깅

### CMake 출력 확인
```cmake
message(STATUS "Qt5_VERSION: ${Qt5_VERSION}")
message(STATUS "CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
```

### Verbose 빌드
```bash
# Make
make VERBOSE=1

# Ninja
ninja -v

# CMake 통합 방법
cmake --build build --verbose
```

### CMake 재구성
```bash
# 캐시 삭제 후 재구성
rm -rf build
cmake -B build

# 또는 CMakeCache.txt만 삭제
rm build/CMakeCache.txt
cmake build
```

---

**생성일**: 2025-11-08
**프로젝트**: DongArch3D v2.0.0 (Arch3D Liner 기반)
**CMake 버전**: 3.10+ (최소), 3.20+ (권장)
