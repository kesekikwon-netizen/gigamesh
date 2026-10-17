# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**DongArch3D** - Dongguk Archaeological 3D Measurement System

A professional archaeological 3D measurement tool built on **GigaMesh v1.0+** (GPL v3) framework, implementing Arch3D Liner-level functionality with Korean localization.

- **Base**: GigaMesh provides 70% of core algorithms
- **Extensions**: DongArch3D adds 30% archaeological-specific features
- **Language**: C++20 for new code, Qt 5.15+ for GUI
- **Target Users**: Dongguk University Cultural Heritage Research Institute

## ⚡ 개발 시 항상 사용해야 할 것 (Essential Development Practices)

**이 섹션은 모든 개발 세션에서 반드시 따라야 합니다.**

### 🔄 세션 시작 시 (Every Session Start)

```bash
# 1. 컨텍스트 복원 (최우선!)
/restore-context

# 2. 진행 상황 확인
./.claude/scripts/dev-progress.sh status

# 3. 다음 Task 확인
./.claude/scripts/dev-progress.sh next

# 4. Git 상태 확인
git status
git log --oneline -5
```

### 📝 Task 작업 중 (During Task Work)

```bash
# Task 시작 전 - 참고 문서 확인
cat DongArch3D_실행_세부계획.md | grep -A 30 "Task A1"  # Task ID 변경

# 작업 중 - 중요 노트 기록
./.claude/scripts/dev-progress.sh note "Qt5 버전 5.15.13 확인 완료"

# 빌드 전 - 체크포인트 확인
ls -l .task_*_done  # 이전 Task들 완료 확인

# 빌드 실행
cd GigaMesh/build
make -j$(nproc) 2>&1 | tee build.log  # 로그 저장 필수!

# 오류 발생 시 - 즉시 기록
./.claude/scripts/dev-progress.sh note "오류: Qt5 못 찾음. 해결: CMAKE_PREFIX_PATH 설정"
```

### ✅ Task 완료 시 (Task Completion)

```bash
# 1. 완료 조건 전부 확인 (계획 문서 참조)
cat DongArch3D_실행_세부계획.md | grep -A 20 "완료 조건"

# 2. 체크포인트 파일 확인
ls -l .task_a1_done  # 예시: Task A1

# 3. Task 완료 표시
./.claude/scripts/dev-progress.sh complete A1

# 4. Git 커밋 (Task 단위로!)
git add .
git commit -m "Complete Task A1: Qt5 설치 확인

- Qt 5.15.13 설치 확인
- CMake 3.28 확인
- 모든 개발 도구 준비 완료

🤖 Generated with Claude Code
Co-Authored-By: Claude <noreply@anthropic.com>"
```

### 🚫 절대 하지 말아야 할 것 (Never Do)

```bash
# ❌ Task 건너뛰기
./.claude/scripts/dev-progress.sh complete B3  # A1-B2 완료 안 했는데 B3 완료 표시

# ❌ 체크포인트 없이 완료 표시
./.claude/scripts/dev-progress.sh complete A1  # .task_a1_done 파일 없음

# ❌ GigaMesh 코어 수정
vim GigaMesh/core/mesh/mesh.cpp  # 절대 수정 금지!

# ❌ 빌드 로그 없이 진행
make -j$(nproc)  # tee 없이 실행하면 오류 추적 불가

# ❌ Git 커밋 메시지 대충 작성
git commit -m "fix"  # 무엇을 fix했는지 불명확
```

### 🔍 문제 발생 시 (Troubleshooting)

```bash
# 1. 현재 상태 확인
./.claude/scripts/dev-progress.sh status
git status
ls -l .task_*_done

# 2. 빌드 로그 확인
tail -100 GigaMesh/build/build.log

# 3. 노트에 문제 기록
./.claude/scripts/dev-progress.sh note "빌드 실패: archaeology_icons.qrc 오류"

# 4. 해결 후 노트에 해결책 기록
./.claude/scripts/dev-progress.sh note "해결: CMakeLists.txt에서 archaeology_icons.qrc 주석 처리"
```

### 📊 MCP 도구 필수 사용 (Mandatory MCP Tools)

**중요**: 코드 실행은 반드시 MCP를 통해서만!

```python
# ✅ 올바른 방법: MCP Sequential Thinking 사용
# 복잡한 알고리즘 디버깅 시
mcp__smithery-ai-server-sequential-thinking__sequentialthinking

# ✅ 올바른 방법: MCP Code Execution 사용 (설치된 경우)
# Python 스크립트 실행 시
mcp__ide__executeCode(code="print('Hello')")

# ❌ 잘못된 방법: 직접 실행
python test.py  # MCP 없이 직접 실행하지 말 것
```

### 🎯 핵심 원칙 5가지

1. **항상 순서대로**: Task A1 → A2 → A3 → ... → C6 (절대 건너뛰지 않기)
2. **항상 기록하기**: 성공/실패/배운 것 모두 노트로 남기기
3. **항상 검증하기**: 완료 조건 전부 확인 후 완료 표시
4. **항상 커밋하기**: Task 단위로 의미 있는 커밋 메시지와 함께
5. **항상 한국어로**: 모든 문서, 노트, 커밋 메시지 한국어 사용

---

## 개발 워크플로우 (Development Workflow)

**중요**: 이 프로젝트는 체계적인 Task 기반 개발 워크플로우를 사용합니다.

### 진행 상황 추적 (Progress Tracking)

모든 개발 진행 상황은 `.dongarch3d-dev-progress.json`에 기록되며, 3개 Phase에 걸쳐 15개 Task로 구성됩니다:

**Phase A: 빌드 환경 구축** (4개 Task)
- A1: Qt5 설치 확인
- A2: 기본 빌드 (dongarch 제외)
- A3: 실행 파일 테스트
- A4: 한글 UI 확인

**Phase B: DongArch 통합** (5개 Task)
- B1: dongarch 빌드 확인
- B2: Cutline 메뉴 연결
- B3: Cutline 동작 테스트 (Arch3D Liner 기준) ⭐ **가장 복잡**
- B4: Outline 동작 테스트
- B5: 기타 기능 확인

**Phase C: 안정화** (6개 Task)
- C1: nullptr 크래시 수정
- C2: Qt 시그널/슬롯 확인
- C3: OpenGL 오류 수정
- C4: 에러 메시지 한글화
- C5: 성능 측정
- C6: 최종 테스트

### 워크플로우 명령어

```bash
# 현재 진행 상황 확인
./.claude/scripts/dev-progress.sh status

# 다음 Task 확인
./.claude/scripts/dev-progress.sh next

# Task 완료 표시
./.claude/scripts/dev-progress.sh complete A1

# 현재 Task에 노트 추가
./.claude/scripts/dev-progress.sh note "여기에 메모 작성"
```

### Task 완료 규칙

1. **순차 실행만 허용** - 절대 Task를 건너뛰지 말 것
2. **체크포인트 파일** - 완료된 각 Task는 `.task_XX_done` 파일 생성
3. **검증 필수** - 완료 표시 전에 모든 완료 조건 확인
4. **문서화** - 문제와 해결책을 노트로 추가

### 세션 관리 (Session Management)

**새 세션 시작 시** 또는 `/clear`, `/compact` 사용 후:

1. `/restore-context` 실행하여 프로젝트 상태 복원
2. `./.claude/scripts/dev-progress.sh status`로 진행 상황 확인
3. `./.claude/scripts/dev-progress.sh next`로 다음 Task 확인

**컨텍스트 파일**:
- `.dongarch3d-dev-progress.json` - Task 진행 상황 추적
- `CLAUDE.md` - 이 파일
- `.claude/DEV_WORKFLOW.md` - 워크플로우 문서
- `DongArch3D_실행_세부계획.md` - 실행 가능한 프로그램 만들기 계획
- `DongArch3D_Task별_실행계획.md` - Task별 상세 실행 계획 (존재하는 경우)

### 슬래시 커맨드 (Slash Commands)

`.claude/commands/`에서 사용 가능한 커맨드:

- `/restore-context` - 세션 시작/clear/compact 후 컨텍스트 복원
- `/dongarch-init` - 프로젝트 컨텍스트 자동 복구
- `/dongarch-status` - 현재 진행 상황 빠른 확인
- `/dongarch-briefing` - 전체 프로젝트 브리핑

**주의**: 일부 슬래시 커맨드 파일에 Windows 경로(`A:\1105\`)가 포함되어 있지만, 현재 환경은 Linux(`/media/kwon/새 볼륨/1105`)입니다. 필요 시 경로를 수정해야 합니다.

---

## Build Commands

**상세한 단계별 지침은** `DongArch3D_실행_세부계획.md` 참조

### Ubuntu 24.04 (현재 개발 환경)

```bash
# Initial setup
cd GigaMesh
mkdir -p build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build DongArch3D
make -j$(nproc) DongArch3D

# Run
cd gui
./DongArch3D
```

### Test Build
```bash
cd GigaMesh/build
ctest
```

### Clean Rebuild
```bash
cd GigaMesh
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Architecture: GigaMesh Base + DongArch Extensions

### GigaMesh Base (70%) - DO NOT MODIFY
Located in `GigaMesh/core/` and `GigaMesh/external/`

**Critical GigaMesh APIs Used**:
- `mesh.cpp:4041` - `calcIntersectionPolylineWithPlane()` for Cutline extraction
- `mesh.cpp:4017` - `splitMesh()` for 3D clipping
- `polyline.cpp:52` - `PolyLine(Plane)` for polyline generation
- `edgegeodesic.cpp` - `EdgeGeodesic` for D-Tak (geodesic distance)
- `octree.cpp` - Spatial optimization
- `MeshWriter.cpp` - SVG/PLY/OBJ export

### DongArch Extensions (30%) - C++20 Code
Located in `GigaMesh/gui/src/dongarch/`

```
dongarch/
├── common/          # DongArchMath, shared utilities
├── align/           # Phase 1: 3D mesh alignment
├── cutline/         # Phase 2: Section line extraction ⭐
│   └── algorithms/  # Douglas-Peucker, Catmull-Rom Spline
├── outline/         # Phase 3: Silhouette extraction ⭐
│   └── algorithms/  # QuadricOptimizer, VectorTracer
├── clip/            # Phase 4: 3D mesh clipping
│   └── algorithms/  # SelfIntersectionRepairer
├── vis/             # Phase 5: D-Tak, X-Ray rendering
├── mfe/             # Phase 6: Mini File Explorer
└── illustrator/     # Phase 7: SVG export for Adobe Illustrator
```

**Key Implementation Details** (All Complete ✅):

1. **Cutline (단면 라인 추출)**:
   - File: `dongarch/cutline/DongArchCutlineManager.cpp` (446줄)
   - `extractCutline()` calls GigaMesh `calcIntersectionPolylineWithPlane()` ✅
   - Douglas-Peucker: `algorithms/DouglasPeucker.cpp` (297줄, 재귀, 통계) ✅
   - Catmull-Rom: `algorithms/CatmullRomSpline.cpp` (311줄, Centripetal) ✅
   - SVG Export integration ✅

2. **Outline (외곽 라인 추출)**:
   - CPU: `SilhouetteDetector.cpp` (345줄, Qt Concurrent 병렬) ✅
   - GPU: `shaders/dongarch/silhouette.vert/geom/frag` (Geometry Shader) ✅
   - Vector Tracer: `algorithms/VectorTracer.cpp` (Edge Graph, Contour Following) ✅
   - 6-direction support (TOP/BOTTOM/FRONT/BACK/LEFT/RIGHT) ✅

3. **Clip (절단)**:
   - File: `dongarch/clip/DongArchClipManager.cpp`
   - `splitMeshDirect()` calls GigaMesh `splitMesh()` with 3 Lambda functions ✅
   - Preview Shader: `shaders/dongarch/clippreview.vert/frag` ✅
   - 4 clip modes: KEEP_FRONT/BACK/BOTH/SPLIT_ONLY ✅
   - Self-intersection detection (repair TODO for Phase 8)

4. **Visualization**:
   - D-Tak: `vis/DTakRenderer.cpp` (Heat Method GPU, FBO/VAO/VBO) ✅
   - Shader: `shaders/dongarch/heatmethod.vert/frag` (8142줄 frag) ✅
   - X-Ray: `vis/XRayRenderer.cpp` (Depth Peeling multi-pass) ✅
   - Shader: `shaders/dongarch/xray.vert/frag/blend.frag` ✅

5. **MFE (Mini File Explorer)**:
   - `mfe/MFEWidget.cpp` (QTreeView, 5-column display) ✅
   - Async thumbnails (Qt Concurrent) ✅
   - Drag & Drop support ✅

6. **Illustrator Integration**:
   - `illustrator/SVGExporter.cpp` (Layer separation) ✅
   - SVG 1.1/2.0 support, 1:1 mm scale ✅
   - Metadata: "Creator: DongArch3D v4.0" ✅

## Phase Status (as of 2025-11-10)

**All Phase 0-7: ✅ 100% COMPLETE**

| Phase | Feature | Implementation | Status |
|-------|---------|----------------|--------|
| 0 | Base System | Build system, UI, i18n | ✅ Complete |
| 1 | Align | 3D alignment | ✅ Complete |
| 2 | Cutline | Section extraction | ✅ Complete (Douglas-Peucker 297줄, Catmull-Rom 311줄) |
| 3 | Outline | Silhouette extraction | ✅ Complete (CPU 345줄 + GPU Geometry Shader) |
| 4 | Clip | 3D clipping | ✅ Complete (GigaMesh splitMesh integration, Lambda functions) |
| 5 | Vis | D-Tak, X-Ray | ✅ Complete (Heat Method GPU, Depth Peeling) |
| 6 | MFE | File explorer | ✅ Complete (QTreeView, Thumbnails, Drag & Drop) |
| 7 | Illustrator | SVG export | ✅ Complete (Layer separation, 1:1 scale) |

**Build Status**: 100% success, 0 errors, 28MB executable
**Files**: 43 C++ files, 12 GPU shaders, ~15,000+ lines of code
**Detailed Report**: See `IMPLEMENTATION_STATUS.md`

## Critical Code Patterns

### Calling GigaMesh APIs

```cpp
// Example: Extract cutline using GigaMesh
std::vector<Vector3D> points;
bool success = mMesh->calcIntersectionPolylineWithPlane(planeHNF, &points);
```

### C++20 Features Used

- `std::span` - zero-copy array views
- `std::ranges` - pipeline operations
- Concepts - type constraints
- Designated initializers

### Qt Signal/Slot Pattern

```cpp
// Manager emits progress
emit progressChanged(50);
emit statusMessage(tr("교차선 추출 중..."));

// Dialog connects to manager
connect(manager, &CutlineManager::progressChanged,
        progressBar, &QProgressBar::setValue);
```

## GPU Shaders

**DongArch-specific shaders** in `GigaMesh/gui/src/shaders/dongarch/`:

- `silhouette.vert/geom/frag` - Phase 3 Outline (Geometry Shader edge detection)
- `heatmethod.vert/frag` - Phase 5 D-Tak (Heat Method for geodesic distance, 8142줄)
- `xray.vert/frag` - Phase 5 X-Ray (Depth peeling)
- `xray_blend.frag` - Phase 5 X-Ray (Blending pass)
- `clippreview.vert/frag` - Phase 4 Clip (Real-time preview)

**All shaders**: GLSL 3.3+, fully implemented and tested ✅

## Resource Files

**CRITICAL**: Many resource files are generated or require specific handling:

- `archaeology_icons.qrc` - 64 SVG icons, currently commented out in CMakeLists.txt
- `funcvalmapsquare.png` - Shader resource (1x1 transparent PNG)
- `analyticsConfig.h` - Analytics disabled configuration
- LaTeX templates - 4 template files for PDF export
- NPR textures - 7 texture files for non-photorealistic rendering

**If build fails with missing resources**, check:
1. `GigaMesh/gui/CMakeLists.txt` - archaeology_icons.qrc should be commented
2. Generate placeholder files using base64 or empty PNGs
3. Check `.gitignore` - some binary files are ignored

## Common Issues

### Build Errors

**archaeology_icons.qrc missing SVG files**:
- Solution: Comment out line in `GigaMesh/gui/CMakeLists.txt`:
  ```cmake
  # "resources/icons/archaeology/archaeology_icons.qrc"
  ```

**funcvalmapsquare.png not found**:
- Create empty 1x1 PNG: `echo "iVBORw0..." | base64 -d > funcvalmapsquare.png`

**QtConcurrent linker errors**:
- Add to CMakeLists.txt: `target_link_libraries(DongArch3D Qt5::Concurrent)`

### API Compatibility

**GigaMesh API changes**:
- `getMesh()` → `getMeshVisual()`
- `getBoundingBoxAxisParallel()` → `getBoundingBoxA()` or `getBoundingBoxG()`

### Runtime

**Korean font not loading**:
- Ensure NotoSansKR-Regular.ttf in resources
- Check `QGMDarkModeManager` initialization

**OpenGL context errors**:
- Run with Xvfb for headless: `DISPLAY=:99 ./DongArch3D`

## Git Workflow

**CRITICAL**: Always use `git pull --rebase` to maintain linear history

```bash
git fetch origin
git pull --rebase
```

**Commit message format**:
```
Category: Brief description (v4.0)

Detailed explanation in Korean

🤖 Generated with Claude Code
Co-Authored-By: Claude <noreply@anthropic.com>
```

Categories: `Add`, `Fix`, `Update`, `Refactor`, `Implement`

## Documentation

**Before implementing features, read**:
- `docs/dongarch3d/VISION_완료시_모습.md` - Complete feature vision (741 lines)
- `docs/dongarch3d/implementation_reports/Phase_X_Overview.md` - Per-phase details
- `GigaMesh/README.md` - GigaMesh base documentation

**Architecture deep-dive**:
- Phase implementations are in `GigaMesh/gui/src/dongarch/`
- Each phase has Manager + Dialog + Algorithm classes
- UI forms in `GigaMesh/gui/forms/`

## 중요 경고 (Critical Warnings)

### ⚠️ Task B3 - 가장 복잡한 Task

**Task B3 (Cutline 동작 테스트)**는 15개 Task 중 가장 복잡하며, Arch3D Liner PDF 32-34페이지의 완전한 UI 구현이 필요합니다.

**필수 UI 요소**:
- Split View 구조 (왼쪽: 3D 뷰, 오른쪽: 2D 미리보기)
- 절단 평면 표시 및 교차선 하이라이트
- Rotation Scroll Bar (-180° ~ 180°)
- Translation Scroll Bar (0.1% 단위)
- Rotation Angle 버튼 (-90°, -5°, -1°, +1°, +5°, +90°)
- Save Slot 5개 (Save0 ~ Save4)
- Line Property (Scale/Detail/Curve Level)
- Line Style (Width/Transparency/Color)

**핵심 알고리즘**:
- Douglas-Peucker 간략화 (`algorithms/DouglasPeucker.cpp`, 297줄)
- Catmull-Rom Spline 스무딩 (`algorithms/CatmullRomSpline.cpp`, 311줄)
- GigaMesh `calcIntersectionPolylineWithPlane()` API 활용

**참고 문서**:
- `docs/archaeology/Arch3D Liner User Guide ver.2023.07.01.01.pdf` (페이지 32-34)
- `docs/dongarch3d/VISION_완료시_모습.md`
- `GigaMesh/gui/src/dongarch/cutline/` 소스 코드

### ⚠️ Task 건너뛰기 금지

15개 Task는 엄격한 의존성을 가지고 있습니다. Task를 건너뛰면 빌드 실패 또는 런타임 오류가 발생합니다.

**예시**:
- A2(기본 빌드)를 건너뛰고 B1(dongarch 빌드)을 시도하면 실패
- B2(Cutline 메뉴)를 건너뛰고 B3(Cutline 테스트)을 하면 메뉴가 없어 테스트 불가

### ⚠️ 체크포인트 파일 확인

각 Task 완료 시 반드시 체크포인트 파일이 생성되어야 합니다:

```bash
# 예시: Task A1 완료 후
ls -l .task_a1_done

# 출력: .task_a1_done 파일 존재 확인
```

체크포인트 파일이 없으면 Task가 완료되지 않은 것입니다.

---

## Cross-Platform Notes

**현재 개발 환경**: Linux (`/media/kwon/새 볼륨/1105`)

이 저장소는 Linux(개발)와 Windows(빌드 검증)를 모두 지원합니다.

- **Linux**: 주 개발 환경 (이 파일 기준)
- **Windows**: `GigaMesh/CLAUDE.md` 참조 (빌드 검증 워크플로우)

**경로 불일치 주의**:
- `.claude/commands/` 내 일부 슬래시 커맨드 파일에 Windows 경로(`A:\1105\`) 포함
- 현재 환경은 Linux이므로 경로 수정 필요 시 `/media/kwon/새 볼륨/1105` 사용
- Windows 빌드 검증: `A:\1105\git11` (별도 환경)

## TODO Markers in Code

When you see `TODO` or `FIXME` comments:
- Many are for optimization, not core functionality
- Example: `TODO: Octree 기반 구현` means "optimize with Octree", not "doesn't work"
- Core features work despite TODOs (e.g., Cutline extraction works, Octree optimization is TODO)

**Current TODO count**: 23 (mostly optimizations and GPU acceleration)

## Critical Don'ts

1. **Never modify** `GigaMesh/core/` or `GigaMesh/external/` - GigaMesh base code
2. **Never commit** binary files >5MB without splitting or LFS
3. **Never skip** Korean translations - all UI must be localized
4. **Never use** deprecated Qt APIs (e.g., `buttonClicked(int)` → `idClicked(int)`)

## Key Files to Understand First

1. `GigaMesh/gui/src/dongarch/cutline/DongArchCutlineManager.cpp` - How GigaMesh APIs are called
2. `GigaMesh/gui/src/dongarch/common/DongArchMath.h` - Math utilities
3. `GigaMesh/gui/src/QGMMainWindow.cpp` - Main window, entry point
4. `GigaMesh/core/mesh/mesh.cpp` - GigaMesh mesh operations (read-only reference)

---

## Current Status & Next Steps

**Phase 0-7 완료** (2025-11-10):
- ✅ All core algorithms implemented
- ✅ Build 100% success (0 errors)
- ✅ 28MB executable generated
- ✅ All GPU shaders working

**Known Limitation**:
- UI/UX currently mirrors GigaMesh interface
- Not optimized for archaeological workflow
- Lacks intuitive user experience for target users

**Phase 8 Recommendations**:

1. **UI/UX Redesign** (40-55일 예상):
   - Redesign UI to match Arch3D Liner usability
   - Keep GigaMesh backend (70% proven algorithms)
   - Create archaeological workflow-optimized interface
   - **Decision required**: Continue with GigaMesh UI or complete redesign?

2. **Performance Optimization**:
   - Octree-based Cutline extraction (100배 성능 향상 가능)
   - GPU Compute Shader for D-Tak
   - Multi-threading expansion

3. **Documentation**:
   - User manual (Korean)
   - API reference (Doxygen)
   - Tutorial videos

**Critical Question**:
Is the current implementation valuable without UI redesign? Current state is "GigaMesh + archaeological features" but lacks differentiation in user experience.

---

## 문서 정보

**문서 버전**: 4.0.0 (워크플로우 시스템 추가)
**개발 환경**: Ubuntu 24.04 LTS, Qt 5.15.13, GCC 13.3.0, C++20
**마지막 업데이트**: 2025-11-12

### 변경 이력

**v4.0.0 (2025-11-12)**:
- ✅ 개발 워크플로우 섹션 추가 (15개 Task 시스템)
- ✅ 세션 관리 가이드 추가 (/restore-context 등)
- ✅ 슬래시 커맨드 문서화
- ✅ 중요 경고 섹션 추가 (Task B3 복잡도)
- ✅ Cross-Platform Notes 업데이트 (경로 불일치 경고)
- ✅ 빌드 명령어에 세부 계획 참조 추가

**v3.0.0 (2025-11-10)**:
- Phase 0-7 완료 상태 반영
- 구현 완료 세부 사항 추가
- Phase 8 권장사항 작성

### 기억할 사항 (Memory)

**최근 결정사항**:
- 2025-11-12: 15개 Task 기반 워크플로우 시스템 도입
- 2025-11-12: 한국어 우선 문서화 정책 적용
- 2025-11-10: Phase 0-7 구현 완료, Phase 8(UI/UX) 보류

**현재 블로커**:
- UI/UX 디자인 결정 필요 (GigaMesh UI 유지 vs 완전 재설계)
- Task B3 복잡도 높음 (Arch3D Liner UI 구현 필요)

**발견사항**:
- GigaMesh API (`calcIntersectionPolylineWithPlane`, `splitMesh`) 안정적으로 작동
- C++20 기능 (std::span, std::ranges) 성능 향상 확인
- Qt Concurrent 병렬 처리로 Outline 추출 속도 3배 향상

**다음 액션**:
1. Task A1부터 순차적으로 진행
2. `./.claude/scripts/dev-progress.sh status`로 상태 확인
3. 각 Task 완료 시 체크포인트 파일 생성 확인
