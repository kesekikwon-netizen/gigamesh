# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**GigaMesh Software Framework** - Modular software for display, editing and visualization of 3D mesh data, typically acquired with structured light scanning (SLS) or structure from motion (SfM). Primary focus on archaeological objects (cuneiform tablets, ceramics, LiDAR data).

Core capabilities:
- Multi-Scale Integral Invariant (MSII) filtering for text retrieval from damaged 3D surfaces
- Mesh cleaning, repair, and inspection for 3D printing
- Unwrappings (rollouts), profile cuts, curvature visualization
- Non-photorealistic rendering (NPR) with hatching and edge detection
- Export to raster/vector graphics, GLTF, PLY, OBJ, VRML

## Build System

### Basic Build
```bash
mkdir build && cd build
cmake ..
cmake --build . -j4
```

### Build Configurations
```bash
# Release build (single-config generators like Make)
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .

# Release build (multi-config generators like MSVC)
cmake ..
cmake --build . --config Release

# Disable GUI (CLI only)
cmake .. -DBUILD_GUI=OFF

# Disable CLI (GUI only)
cmake .. -DBUILD_CLI=OFF

# Disable tests
cmake .. -DENABLE_TESTS=OFF
```

### Requirements
- CMake ≥3.10
- C++17 compatible compiler (GCC ≥8, Clang ≥7, MSVC ≥19.14, Apple Clang ≥11.0.0)
- Qt5 (Core, Widgets, Gui, OpenGL, Network)
- OpenGL
- libtiff (optional)

### Qt Creator
Open the root `CMakeLists.txt` in Qt Creator. Ensure C++17 compatible compiler in selected kit.

### Windows Build (MSVC)
```bash
# Install Qt5 via aqtinstall
pip install aqtinstall
aqt install-qt windows desktop 5.15.2 win64_msvc2019_64 -O C:/Qt

# Configure with Visual Studio 2022
cmake -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" ^
      -G "Visual Studio 17 2022" -A x64 ^
      -B build_korean

# Build (multi-config generator)
cmake --build build_korean --config Release --parallel 4

# Deploy with Qt DLLs
cd C:/Qt/5.15.2/msvc2019_64/bin
windeployqt.exe --release --no-system-d3d-compiler path/to/gigamesh.exe

# Result: Portable 56MB package with all dependencies
```

**Windows-specific notes**:
- Use Visual Studio 2019 or 2022 with MSVC toolchain
- Translations must be embedded during build (checked by `qrc_translations.cpp` size)
- If translations don't load, delete `gui/qrc_translations.cpp` and rebuild GUI target only

## Architecture

### Module Structure
```
GigaMesh/
├── core/          # Core mesh processing library (libgigamesh-core)
│   ├── mesh/      # Mesh data structures and algorithms
│   │   ├── mesh.cpp/h              # Main Mesh class
│   │   ├── vertex.cpp/h, face.cpp/h
│   │   ├── compfeaturevecs.cpp     # MSII feature vector computation
│   │   ├── edgegeodesic.cpp        # Edge detection, geodesic distance
│   │   ├── octree.cpp              # Spatial indexing
│   │   └── MeshIO/                 # File I/O (PLY, OBJ, GLTF, VRML, TXT)
│   └── logging/   # Logging utilities
├── gui/           # Qt-based GUI application
│   ├── src/
│   │   ├── QGMMainWindow.cpp/h     # Main window
│   │   ├── meshwidget.cpp/h        # Main 3D viewport widget
│   │   ├── meshGL/                 # OpenGL rendering layer
│   │   │   ├── meshGL.cpp/h        # OpenGL mesh rendering
│   │   │   ├── meshGLShader.cpp/h  # Shader management
│   │   │   └── meshGL_params.h     # Rendering parameters enum
│   │   ├── shaders/                # GLSL shader files
│   │   │   └── NPR/                # Non-photorealistic rendering shaders
│   │   │       ├── NPR_hatches.frag/vert  # Hatching implementation
│   │   │       └── NPR_ApplySobel.frag    # Edge detection
│   │   └── qgmdocksidebar.cpp/h    # UI sidebar panels
│   ├── languages/                  # Qt translation files (.ts, .qm)
│   └── resources/                  # Qt resources (textures, icons)
├── cli/           # Command-line tools
│   ├── gigamesh-featurevectors     # MSII filter for single files
│   ├── gigamesh-clean              # Batch cleaning/repairing
│   ├── gigamesh-info               # Mesh properties extraction
│   ├── gigamesh-togltf             # GLTF conversion
│   └── [8 more CLI tools]
├── external/      # Third-party dependencies
├── tests/         # Test suite
└── testdata/      # Test data files
```

### Key Design Patterns

**Mesh Processing Pipeline**:
```
File I/O (MeshReader) → Mesh (core data) → MeshQt (Qt integration) → MeshGL (OpenGL rendering)
```

**Rendering Architecture**:
- `meshGL.cpp`: Manages OpenGL context, VBOs, state
- `meshGLShader.cpp`: Loads/compiles GLSL shaders, sets uniforms
- `ShaderManager.cpp`: Shader lifecycle management
- Shaders organized by function: depth, lighting, NPR, texture mapping

**MSII Feature Vectors** (`compfeaturevecs.cpp`):
- Multi-threaded computation of integral invariants
- Used for text detection on damaged surfaces
- Produces feature vectors at multiple scales

### NPR (Non-Photorealistic Rendering) System

Located in `gui/src/shaders/NPR/`:
- `NPR_hatches.frag`: Implements hatching patterns using Tonal Art Maps (TAMs)
  - 6-level hatching density based on lighting
  - Texture-based hatching (can rotate, scale)
  - Supports Bayer/Random dithering
- `NPR_ApplySobel.frag`: Sobel edge detection filter
- Rendering controlled via `meshGL_params.h` enums:
  - `SHOW_NPR_HATCHLINES`
  - `NPR_HATCH_STYLE`
  - `NPR_HATCH_SOURCE`

### Shader Parameter System

Rendering parameters managed through `meshGL_params.h` enum system:
```cpp
enum eParamInt {
    SHOW_NPR_HATCHLINES,
    NPR_HATCH_STYLE,
    // ... 100+ parameters
};
```

Parameters propagate: UI → MeshWidget → MeshGL → Shader uniforms

### Translation System

**Supported Languages**: German (de), English (en), Japanese (ja), Korean (ko)

**Status**: Korean translation 99.9% complete (1198/1199 strings) - fully functional UI with all menus, dialogs, and settings localized.

**Translation workflow**:
```bash
# 1. Extract translatable strings from source
cd GigaMesh
lupdate -recursive . -ts gui/languages/GigaMesh_ko.ts

# 2. Apply batch translations (recommended for large updates)
# Use Python scripts to maintain translations in version control
# See ../skills/qt-app-translator/ for complete workflow

# 3. Compile to binary format
lrelease gui/languages/GigaMesh_ko.ts

# 4. Rebuild to embed updated .qm file
cd build_korean  # or your build directory
cmake --build . --config Release --target gui

# 5. Test translations
./gui/Release/gigamesh.exe
# Settings → Language → 한국어
```

**Translation resources**:
- Translations embedded via `gui/translations.qrc` (resource prefix: `:/languages`)
- Qt Linguist GUI tool: `linguist gui/languages/GigaMesh_ko.ts`
- Batch translation skill: `../skills/qt-app-translator/` (reusable for any Qt app)

## Executables

### GUI
```bash
# Linux/macOS
./build/gui/gigamesh [mesh.ply]

# Windows
.\build_korean\gui\Release\gigamesh.exe [mesh.ply]
```

### CLI Tools
```bash
# MSII feature extraction (radius in mm)
./build/cli/gigamesh-featurevectors -r 5.0 mesh.ply

# Mesh information
./build/cli/gigamesh-info mesh.ply

# Convert to GLTF
./build/cli/gigamesh-togltf mesh.ply

# All tools support --help
./build/cli/gigamesh-featurevectors --help
```

## Deployment

### Creating Portable Windows Package
```bash
# 1. Build release executable
cmake --build build_korean --config Release --target gui

# 2. Create deployment directory
mkdir GigaMesh_Korean_Portable
cp build_korean/gui/Release/gigamesh.exe GigaMesh_Korean_Portable/

# 3. Deploy Qt dependencies
cd C:/Qt/5.15.2/msvc2019_64/bin
windeployqt.exe --release --no-system-d3d-compiler path/to/GigaMesh_Korean_Portable/gigamesh.exe

# Result: Self-contained 56MB package with:
# - gigamesh.exe (5.7MB, with 130KB embedded Korean translation)
# - Qt5Core.dll, Qt5Gui.dll, Qt5Widgets.dll, Qt5OpenGL.dll
# - platforms/qwindows.dll
# - translations/qt_ko.qm (Qt's own Korean strings)
# - Other required DLLs
```

**Package contents**:
- Works on Windows without Qt installation
- Korean UI fully functional (Settings → Language → 한국어)
- Includes all mesh processing and visualization features

## Development Workflows

### Adding New Shader
1. Create `.frag/.vert` files in `gui/src/shaders/`
2. Add to `gui/resources/resources.qrc`
3. Load in `meshGLShader.cpp` via `ShaderManager`
4. Add parameter enums to `meshGL_params.h` if needed
5. Connect UI controls in `qgmdocksidebar.cpp`

### Modifying Mesh Processing
1. Core logic in `core/mesh/mesh.cpp` or specific algorithm files
2. Qt integration hooks in `gui/src/meshQt.cpp`
3. UI exposure via `QGMMainWindow.cpp` menu/dialog
4. OpenGL visualization in `gui/src/meshGL/meshGL.cpp`

### Adding UI Features
1. Dialog class: `gui/src/QGMDialog*.cpp/h`
2. Sidebar panel: `gui/src/qgmdocksidebar.cpp`
3. Connect signals/slots in `QGMMainWindow.cpp`
4. Update translations in `gui/languages/*.ts`

## File Format Support

### Reading
- PLY (Stanford Polygon) - ASCII and binary
- OBJ (Wavefront) - with MTL material
- TXT (regular grid, custom formats)

### Writing
- PLY (with GigaMesh extensions: features, labels, flags)
- OBJ
- GLTF (vertices, normals, textures, vertex colors)
- VRML
- SVG (for profile cuts, polylines)

## Testing

```bash
cd build
ctest
# Or run specific test executables in build/tests/
```

## Git Workflow

- Main branch: `master`
- Development branch: `develop`
- Feature branches: `feature/feature-name`

## Archaeological Standards Integration (Active Development)

**Context**: Integrating Raczynski-Henk 2017 international lithic illustration standards for archaeological 3D mesh rendering. This enables generating publication-quality archaeological illustrations directly from 3D scans.

**Branch**: `feature/lithic-standards`

**Project Documentation**:
- `../MVP_개발계획_상세.md` - Detailed MVP development plan (14-day, 4-phase)
- `../MVP_진행상황.md` - Current progress tracking
- `../GigaMesh_구현가망성평가.md` - Technical feasibility assessment
- `../.mvp-progress.json` - Progress tracking data

**Key Implementation Files**:

**Shader modifications** (45° hatching/lighting):
- `gui/src/shaders/NPR/NPR_hatches.frag` - Add archaeology mode with fixed 45° rotation, cortex cross-hatching
- `gui/src/shaders/NPR/NPR_hatches.vert` - Add cortex vertex attribute passing
- `gui/src/shaders/NPR/NPR_ApplySobel.frag` - Ridge thickness control for emphasis

**C++ rendering layer**:
- `gui/src/meshGL/meshGLShader.cpp` - Fixed lighting direction (upper-left 45°), cortex texture loading, uniform propagation
- `gui/src/meshGL/meshGL_params.h` - Add ARCHAEOLOGY_MODE_ENABLED, CORTEX_CROSS_HATCH_TEXTURE enums

**UI components**:
- `gui/src/qgmdocksidebar.cpp/h` - Archaeological standards panel (mode toggle, hatching density, ridge thickness, cortex selection)
- `gui/src/QGMMainWindow.cpp` - Menu items (View → Archaeological Rendering), PNG export with supersampling

**Core mesh processing**:
- `core/mesh/mesh.h/cpp` - Vertex cortex flags, JSON-based cortex selection save/load
- `core/mesh/edgegeodesic.cpp` - Ridge detection with configurable curvature threshold

**Resources**:
- `gui/resources/textures/archaeology/` - Cortex cross-hatch textures (3 density levels)
- `gui/resources/resources.qrc` - Register archaeology textures
- `gui/languages/GigaMesh_ko.ts` - Korean archaeological terminology translations

**Build with archaeology mode** (optional):
```bash
cmake -DARCHAEO_MODE=ON ..
```

**MVP Success Criteria** (6 of 7 required):
1. 45° hatching angle verification
2. Upper-left 45° lighting direction
3. Interactive cortex region selection
4. Cross-hatch pattern on cortex regions
5. Automatic ridge emphasis (2-3x line thickness)
6. High-resolution PNG export (4K/8K)
7. Expert validation: "规格 유사" (standards-compliant)

## Common Issues

### Build fails with C++17 errors
Set compiler explicitly:
```bash
# Linux/macOS
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-8 -DCMAKE_C_COMPILER=/usr/bin/gcc-8

# Windows - ensure Visual Studio 2019+ is installed
```

### Missing Qt5
```bash
# Linux
sudo apt-get install qt5-default qttools5-dev qtbase5-dev

# Windows - use aqtinstall for standalone Qt
pip install aqtinstall
aqt install-qt windows desktop 5.15.2 win64_msvc2019_64 -O C:/Qt

# macOS
brew install qt@5
```

### CMake can't find Qt5
```bash
# Set CMAKE_PREFIX_PATH explicitly
cmake -DCMAKE_PREFIX_PATH="C:/Qt/5.15.2/msvc2019_64" ..

# Avoid using Conda's Qt - it's not self-contained and causes DLL errors
```

### Translations not loading in UI
**Symptoms**: Settings → Language shows Korean (한국어) but UI still in English

**Causes and fixes**:
1. **Empty .qm file embedded** (16 bytes instead of ~130KB)
   - Recompile: `lrelease gui/languages/GigaMesh_ko.ts`
   - Verify file size: Should be 100KB+, not 16 bytes

2. **CMake cached old .qm file**
   - Delete `build/gui/qrc_translations.cpp`
   - Delete `build/gui/Release/gigamesh.exe`
   - Rebuild GUI target only: `cmake --build build --config Release --target gui`

3. **Translations not marked as finished**
   - Check .ts file for `type="unfinished"` attributes
   - Run `lupdate` to extract new strings
   - Apply translations and recompile

**Verification**:
```bash
# Check embedded translation size (should be ~130KB for Korean)
# Windows
findstr /c:"GigaMesh_ko.qm" build\gui\qrc_translations.cpp

# Linux/macOS
grep "GigaMesh_ko.qm" build/gui/qrc_translations.cpp
```

### Shader compilation errors
Check shader syntax with OpenGL version (3.3 Core used).
Shader errors logged to console when loading.

## References

### GigaMesh Framework
- Official website: https://gigamesh.eu
- GitLab: https://gitlab.com/fcgl/GigaMesh
- Tutorials: https://gigamesh.eu/tutorials
- Publications: https://gigamesh.eu/publications
- YouTube: https://www.youtube.com/channel/UCJSOsw9GX8DnkqnciyVwmLw

### Archaeological Standards Integration
- Raczynski-Henk, Y. (2017). *Drawing Lithic Artefacts*. Sidestone Press. (PDF in parent directory)
- Praun, E., et al. (2001). *Real-Time Hatching*. SIGGRAPH. (PDF in parent directory)
- Academic research documents: `../구석기_실측_학술자료_조사.md`
- Download list: `../학술자료_다운로드_목록.md`

## Project Status

**Current Phase**: Initialization (Day 0)
**Active Branch**: `feature/lithic-standards`
**Progress Tracking**: See `../MVP_진행상황.md` for real-time status

## License

GPL v3 - See LICENSE.txt
