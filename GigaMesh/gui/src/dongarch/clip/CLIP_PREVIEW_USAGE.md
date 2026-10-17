# DongArch Clip Interactive Preview - Usage Guide

**Phase 4: Clip (3D 메시 절단) - 실시간 미리보기 기능**

## Overview

DongArch Clip Interactive Preview provides real-time OpenGL clipping plane visualization before executing mesh split operations.

## Features

- **Real-time Preview**: OpenGL clipping plane (GL_CLIP_PLANE1) shows where mesh will be cut
- **Interactive Slider**: Adjust clip height with immediate visual feedback
- **Clip Modes**: Keep Front, Keep Back, or Keep Both (split)
- **Non-modal Dialog**: Rotate and inspect mesh while dialog is open
- **Auto-zoom**: Quickly focus on clipping plane region

## Architecture

### OpenGL Integration

```
MeshWidgetParams (meshwidget_params.h)
  ├─ DONGARCH_CLIP_PREVIEW_ENABLED (flag)
  ├─ DONGARCH_CLIP_PLANE_X (float)
  ├─ DONGARCH_CLIP_PLANE_Y (float)
  ├─ DONGARCH_CLIP_PLANE_Z (float)
  └─ DONGARCH_CLIP_PLANE_W (float)
         ↓
meshGLShader.cpp (line 287-308)
  ├─ Read parameters from MeshWidgetParams
  ├─ Set GL_CLIP_PLANE1 equation
  └─ glEnable(GL_CLIP_PLANE1)
         ↓
OpenGL Rendering
  └─ Mesh parts beyond plane are clipped in hardware
```

### Class Hierarchy

```
DongArchClipDialog (Qt Dialog)
  ├─ Height Slider (0.0 - 1.0 normalized)
  ├─ Mode Radio Buttons (Front/Back/Both)
  ├─ Preview Checkbox
  └─ Execute/Cancel Buttons
         ↓
MeshWidget::setParamFlagMeshWidget()
MeshWidget::setParamFloatMeshWidget()
         ↓
DongArchClipManager::clipByPlane()
  └─ Executes actual mesh split
```

## Usage Example (C++ Code)

### 1. Create Dialog and Show Preview

```cpp
#include "dongarch/clip/DongArchClipDialog.h"
#include "dongarch/clip/DongArchClipManager.h"

// In your main window or mesh widget:
void MainWindow::onClipMenuTriggered() {
    // Assume mMeshWidget is your MeshWidget instance
    MeshQt* mesh = mMeshWidget->getMeshVisual();
    if (!mesh) return;

    // Create clip manager
    DongArchClipManager* clipManager = new DongArchClipManager(mesh);

    // Create dialog
    DongArchClipDialog* dialog = new DongArchClipDialog(
        mMeshWidget,
        clipManager,
        this
    );

    // Connect signals (optional)
    connect(dialog, &DongArchClipDialog::clipHeightChanged,
            this, &MainWindow::onClipHeightChanged);
    connect(dialog, &DongArchClipDialog::executeClip,
            this, &MainWindow::onClipExecuted);

    // Show dialog (non-modal)
    dialog->show();
}

void MainWindow::onClipHeightChanged(double height) {
    qDebug() << "Clip height changed:" << height;
    // Mesh is automatically updated via MeshWidget parameters
}

void MainWindow::onClipExecuted() {
    qDebug() << "Clip executed successfully";
    // Update UI, refresh mesh view, etc.
}
```

### 2. Manual Preview Control (Advanced)

```cpp
// Enable preview manually
mMeshWidget->setParamFlagMeshWidget(
    MeshWidgetParams::DONGARCH_CLIP_PREVIEW_ENABLED, true);

// Set clip plane (horizontal at height 0.5)
DongArchClipManager clipManager(mesh);
Vector3D clipPlane = clipManager.createHorizontalPlane(0.5);

mMeshWidget->setParamFloatMeshWidget(
    MeshWidgetParams::DONGARCH_CLIP_PLANE_X, clipPlane.getX());
mMeshWidget->setParamFloatMeshWidget(
    MeshWidgetParams::DONGARCH_CLIP_PLANE_Y, clipPlane.getY());
mMeshWidget->setParamFloatMeshWidget(
    MeshWidgetParams::DONGARCH_CLIP_PLANE_Z, clipPlane.getZ());
mMeshWidget->setParamFloatMeshWidget(
    MeshWidgetParams::DONGARCH_CLIP_PLANE_W, clipPlane.getH());

// Trigger OpenGL update
mMeshWidget->update();

// Disable preview
mMeshWidget->setParamFlagMeshWidget(
    MeshWidgetParams::DONGARCH_CLIP_PREVIEW_ENABLED, false);
mMeshWidget->update();
```

### 3. Execute Clipping

```cpp
// Get clip plane from dialog or create manually
Vector3D clipPlane = clipManager.createHorizontalPlane(0.5);

// Set parameters
ClipParams params;
params.planeHNF = clipPlane;
params.mode = ClipMode::KEEP_BOTH;  // Split mesh
params.duplicateVertices = false;
params.noRedraw = false;
params.epsilon = 1e-10;

// Execute clipping
ClipResult result;
bool success = clipManager.clipByPlane(params, result);

if (success) {
    std::cout << "Original faces: " << result.originalFaceCount << "\n";
    std::cout << "Final faces: " << result.frontFaceCount << "\n";
    std::cout << "Processing time: " << result.processingTime << " ms\n";
}
```

## Dialog Features

### Height Slider
- Range: 0.0 (bottom) to 1.0 (top)
- Real-time update as you drag
- Synchronized with spin box for precise input
- Shows current Z coordinate

### Clip Modes
1. **Keep Front (평면 앞쪽 유지)**: Remove mesh below plane
2. **Keep Back (평면 뒤쪽 유지)**: Remove mesh above plane
3. **Keep Both (양쪽 모두 유지)**: Split mesh into two parts

### Preview Checkbox
- Enable/disable real-time preview
- When disabled, saves GPU resources
- Automatically disabled when dialog closes

### Execute Button
- Performs actual mesh split operation
- Disables preview before execution
- Closes dialog on success
- Shows result statistics in console

## OpenGL Clipping Planes Used

GigaMesh uses multiple OpenGL clipping planes:

| Plane | Purpose | Controlled By |
|-------|---------|---------------|
| GL_CLIP_PLANE0 | Mesh plane clipping | `SHOW_MESH_PLANE_AS_CLIPLANE` |
| **GL_CLIP_PLANE1** | **DongArch clip preview** | **`DONGARCH_CLIP_PREVIEW_ENABLED`** |
| GL_CLIP_PLANE2 | Selection-based clipping | `SHOW_CLIP_THRU_SEL` |

## Implementation Notes

### Why GL_CLIP_PLANE1?
- GL_CLIP_PLANE0 is used by mesh plane visualization
- GL_CLIP_PLANE2 is used by selection-based clipping
- GL_CLIP_PLANE1 was available and doesn't conflict

### When is GL_CLIP_PLANE1 Disabled?
The clipping plane is disabled in these scenarios:
1. Preview checkbox unchecked
2. Dialog closed
3. When rendering datum spheres/primitives (prevents clipping artifacts)
4. After mesh split execution

### Performance Considerations
- OpenGL hardware clipping is fast (GPU-accelerated)
- No performance impact when preview is disabled
- Non-modal dialog allows mesh inspection while preview is active

## Translation (Korean)

All UI strings are translated to Korean:
- Dialog title: "DongArch Clip - 메시 절단"
- Height label: "절단 높이"
- Mode labels: "평면 앞쪽 유지", "평면 뒤쪽 유지", "양쪽 모두 유지"
- Preview checkbox: "실시간 미리보기 활성화"
- Execute button: "실행"
- Cancel button: "취소"

Add translations to `gui/languages/GigaMesh_ko.ts` if extending UI.

## Testing

### Manual Test Procedure
1. Load a 3D mesh (PLY, OBJ)
2. Open DongArch Clip dialog
3. Move height slider → verify real-time clipping preview
4. Rotate mesh → verify clipping plane stays in place
5. Change clip mode → verify mode selection works
6. Uncheck preview → verify clipping plane disappears
7. Re-check preview → verify it reappears
8. Click Execute → verify mesh is split correctly

### Expected Results
- Smooth preview updates (60 FPS)
- No flickering or artifacts
- Mesh parts beyond plane are not rendered
- Dialog closes after successful execution

## Future Enhancements

Potential improvements:
1. **Interactive Gizmo**: 3D manipulator for plane rotation/translation
2. **Angle Controls**: Rotate clipping plane (not just horizontal)
3. **Multiple Planes**: Sequential clipping operations
4. **Preview Color**: Highlight cut surface with different color
5. **Undo/Redo**: Non-destructive clipping with history
6. **Auto-zoom**: Camera automatically focuses on clip plane

## Files Modified/Created

### New Files
- `GigaMesh/gui/src/dongarch/clip/DongArchClipDialog.h` (165 lines)
- `GigaMesh/gui/src/dongarch/clip/DongArchClipDialog.cpp` (356 lines)
- `GigaMesh/gui/src/dongarch/clip/CLIP_PREVIEW_USAGE.md` (this file)

### Modified Files
- `GigaMesh/gui/src/meshwidget_params.h`
  - Added `DONGARCH_CLIP_PREVIEW_ENABLED` flag
  - Added `DONGARCH_CLIP_PLANE_X/Y/Z/W` float parameters
- `GigaMesh/gui/src/meshGL/meshGLShader.cpp`
  - Added GL_CLIP_PLANE1 enable/disable logic (lines 287-308)
  - Added glDisable(GL_CLIP_PLANE1) in 3 locations (lines 2069, 2618, 5206)
- `GigaMesh/gui/src/dongarch/clip/DongArchClipManager.h`
  - Added `getMeshBounds()` method
  - Added `createHorizontalPlane()` method
- `GigaMesh/gui/src/dongarch/clip/DongArchClipManager.cpp`
  - Implemented `getMeshBounds()` method (12 lines)
  - Implemented `createHorizontalPlane()` method (21 lines)

## Build Integration

Add to `gui/CMakeLists.txt`:

```cmake
# DongArch Clip Dialog
set(DONGARCH_CLIP_SOURCES
    src/dongarch/clip/DongArchClipManager.cpp
    src/dongarch/clip/DongArchClipDialog.cpp  # NEW
)

set(DONGARCH_CLIP_HEADERS
    src/dongarch/clip/DongArchClipManager.h
    src/dongarch/clip/DongArchClipDialog.h    # NEW
)
```

## References

- GigaMesh OpenGL clipping planes: `meshGLShader.cpp` lines 260-285
- GigaMesh mesh splitting: `mesh.cpp` lines 4149-4300
- Qt non-modal dialogs: https://doc.qt.io/qt-5/qdialog.html
- OpenGL clipping planes: https://www.khronos.org/opengl/wiki/Clipping

---

**Version**: 1.0.0
**Author**: DongArch3D Team
**Date**: 2025-11-10
**License**: GPL v3
