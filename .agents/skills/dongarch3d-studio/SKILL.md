---
name: dongarch3d-studio
description: >-
  Build, test, capture verification screenshots, and develop features for DongArch3D Studio
  (studio/ Qt6 shell and GigaMesh/gui/src/dongarch/karch/ geometry engine). Use this skill
  whenever modifying studio/ or karch/ code, running pipeline tests, or verifying UI_RULES.md.
---

# DongArch3D Studio Development & Verification Skill

## 1. Environment & Build Toolchain
- **Compiler**: Visual Studio 2022 BuildTools (`vcvars64.bat`, MSVC C++20, `/utf-8 /W4 /permissive- /EHsc /Zc:__cplusplus`, Release: `/O2 /Ob2 /Oi /Ot /Gy /Zc:inline /fp:fast /arch:AVX2` + LTO/IPO + PCH)
- **Generator**: CMake + Ninja (`AUTOMOC` scoped to Qt targets only)
- **Qt Path**: `D:/OSGeo4W/apps/Qt6` (`Qt6::Core`, `Qt6::Gui`, `Qt6::Widgets`)
- **Build Script**: [studio/build.ps1](../../../studio/build.ps1)
- **CI Workflow**: [.github/workflows/studio-ci.yml](../../../.github/workflows/studio-ci.yml)

## 2. Standard Build, Test & Screenshot Workflow
Always run the build and headless test suite after code modifications:

1. **Incremental Build + 11 Parallel CTest Suites (`studio_pipeline` + 10 `karch_*` unit tests)**:
   ```powershell
   powershell -File studio/build.ps1 -Test
   ```
2. **Generate 5 Standard Verification Screenshots** (`shot_home.png`, `shot_main.png`, `shot_blind.png`, `shot_max.png`, `shot_export.png`):
   ```powershell
   powershell -File studio/build.ps1 -Test -ShotDir studio/build-msvc/shots
   ```
3. **Direct CLI Flags (`DongArch3DStudio.exe`)**:
   - `--shot <file.png>`: Render current screen and exit
   - `--sample`: Load synthetic 7°-tilted pottery jar with color & paddling texture
   - `--blind`: Load narrow-neck pottery with un-scanned inner wall & hole-fill cap estimation
   - `--max`: Toggle 100% plate maximization (`Tab`) before capture
   - `--export`: Open export dialog and capture preview

## 3. Codebase Module Map

### `studio/src/` (UI Shell & Plate Engine)
- `Theme.h`: R2 color tokens, R4 font helpers (`serifFont`, `monoFont`, `uiFont`), and global QSS.
- `MeshIO.h/.cpp`: Multi-format loader (ASCII/Binary PLY with RGB/UV, fast OBJ+MTL+texture, VRML 2.0 `.wrl`, STL), floating noise removal (`< 2%`), auto `m/cm -> mm` unit scaling, scanner profile detection (`Artec Space Spider` / `Eva·Leo`), and spatial-grid 3D LOD generation (`<= 35,000` faces).
- `ArtifactDoc.h/.cpp`: Document state orchestrating K-Arch pipeline (Rim PCA -> Axis ODR -> `R0/H0` alignment -> Section/Silhouette/5 Major Dimensions), Douglas–Peucker (`simplifyDP`) & Catmull–Rom (`smoothCatmull`), and Artec blind inner-wall estimation (`scannedSolidLines_` vs `estimatedInnerLines_`).
- `MeshRaster.h/.cpp`: Software rasterizer for 3D preview and 2D plate shading (`Real` vertex color, `Depth`, `DTak` high-contrast curvature digital rubbing).
- `Plate.h/.cpp`: Unified R5 plate renderer (`Plate::paint`) and R6 paper layout/exporters (`exportSvgLayered`, `exportPdf`, `exportPng`, `exportDxf`).
- `MainWindow.h/.cpp`: R1 fixed window structure, Undo/Redo stack, global Drag & Drop, and `QFileSystemWatcher` Artec watch-folder auto-loader.
- `Ribbon.h/.cpp`, `SheetView.h/.cpp`, `View3D.h/.cpp`, `StartPage.h/.cpp`, `ExportDialog.h/.cpp`: UI widgets strictly implementing R1–R8.

### `GigaMesh/gui/src/dongarch/karch/` (`DongArch::KArch` Engine)
- **Alignment**: `TaubinFit`, `AnnulusClassifier`, `RimDetector`, `AxisPca`, `AxisOdr`
- **Section & Profiling**: `MeshSlicer`, `Silhouette`, `SectionComposer`, `DimensionAnalyzer`
- **IO & CAD**: `MeshLoader`, `SyntheticMeshGen`, `DxfWriter` (AutoCAD R12, 11 layers)

## 4. UI_RULES Verification Checklist
Before finalizing any UI or rendering change, verify against [docs/ui/UI_RULES.md](../../../docs/ui/UI_RULES.md):
- [ ] No extra Docks/panels added outside R1 fixed layout; `Tab` maximizes plate to 100%.
- [ ] All colors reference `Theme.h` tokens; no hardcoded hex values.
- [ ] Exactly one `Action` (`#B5573A`) primary button visible per screen.
- [ ] At most one active hand tool (`Hand` state); `ctxBar` hides when tool is released.
- [ ] All numbers use monospace font right-aligned with `Faint` `mm` unit labels.
- [ ] Plate drawing order follows Grid -> Shade -> Outline (`#141413`) -> Section (`#FF0000`).

## 5. Canonical UI Mockups & Pitfalls
- **Canonical Design Files (Single Source of Truth)**:
  - Spec & Rules: `docs/ui/UI_RULES.md`, `docs/ui/mockup/dongarch3d_design_spec.md`
  - Interactive HTML Reference: `docs/ui/mockup/dongarch3d_mockup.html`
  - Visual Mockups:
    - `docs/ui/mockup/mockup_01_pottery_main.png`: 토기 단면도 기본 작업 화면 (`--sample --shot`)
    - `docs/ui/mockup/mockup_02_pottery_max100.png`: `Tab` 도판 100% 최대화 (`--sample --max --shot`)
    - `docs/ui/mockup/mockup_03_lithic_4view.png`: 석기 4면 전개도(정면·우측면·배면·상면 + 박리 능선·타격점)
    - `docs/ui/mockup/mockup_04_export_dialog.png`: 도면 내보내기 대화창 (`--sample --export --shot`)
    - `docs/ui/mockup/mockup_05_start_home.png`: 시작 화면 (`--shot`)
- **CMake + Ninja + MSVC Pitfalls**:
  - Inside generator expressions (`$<$<CONFIG:Release>:...>`), always join multiple flags with `;` (`"$<$<CONFIG:Release>:/O2;/Ob2;/Oi;/Ot;/Gy;/Zc:inline;/fp:fast>"`), never spaces.
  - Never pass `/MP` when `CMAKE_GENERATOR MATCHES "Ninja"` (causes MSVC `D9030` warning with `/showIncludes`).
  - Never set global `CMAKE_AUTOMOC ON`; enable `AUTOMOC` only on `studio_core` and `DongArch3DStudio` so pure C++ targets (`karch` and 10 `test_*` executables) skip MOC generation.

## 6. REA Technical Reference (Artec · DongArch · GigaMesh)
- **Artec SDK (`cdsdk.dll`, `Snx3Device_x64.dll`, `RAY2API.dll`)**:
  - Runtime probe in `MeshIO::probeArtecSdk()` checks `cdGetDevices` without redistributing proprietary DLLs; watch-folder (`QFileSystemWatcher`) handles automatic import of `.ply/.obj/.wrl/.stl`.
- **GigaMesh 9 CLI Tools Parity**:
  - `gigamesh-clean`: `weldVertices` + `removeFloatingNoise` (`< 2%` island removal) + `erodeDanglingBorderFaces`
  - `gigamesh-borders`: `extractBorderLoops` + `exportBordersPline` (`.pline`)
  - `gigamesh-featurevectors`: Multi-Scale Integral Invariants (MSII) vertex `quality` scalar (`_r2.00_n4_v256.ply`) -> `DTak` digital rubbing
  - `gigamesh-togltf` / `gigamesh-tolegacy`: `exportGltf2` (`.gltf`) / `exportLegacyPly` (`_Legacy.ply`)

