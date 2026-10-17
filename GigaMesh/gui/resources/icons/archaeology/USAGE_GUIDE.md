# Archaeological Icon Set Usage Guide

## Overview
64 Material Design SVG icons for archaeological analysis in GigaMesh MVP.

## Integration Steps

### 1. Add to CMakeLists.txt
```cmake
# In gui/CMakeLists.txt, add to resources
qt5_add_resources(RESOURCES
    resources/archaeology_icons.qrc
    # ... other .qrc files
)
```

### 2. C++ Usage Examples

#### Action Icons (Menus/Toolbars)
```cpp
// In QGMMainWindow.cpp or similar
QAction* sectionTopAction = new QAction(
    QIcon(":/icons/archaeology/section_top.svg"),
    tr("Top Section View"),
    this
);

QAction* lithicCortexAction = new QAction(
    QIcon(":/icons/archaeology/lithic_cortex.svg"),
    tr("Mark Cortex"),
    this
);
```

#### Button Icons
```cpp
// In qgmdocksidebar.cpp
QPushButton* measureBtn = new QPushButton(this);
measureBtn->setIcon(QIcon(":/icons/archaeology/measure_distance.svg"));
measureBtn->setIconSize(QSize(24, 24));
```

#### Tab Icons
```cpp
tabWidget->addTab(
    sectionWidget,
    QIcon(":/icons/archaeology/section_profile.svg"),
    tr("Profile")
);
```

### 3. UI File Usage (.ui files)
```xml
<!-- In Qt Designer or .ui XML -->
<action name="actionSectionTop">
    <property name="icon">
        <iconset resource="../../resources/archaeology_icons.qrc">
            <normaloff>:/icons/archaeology/section_top.svg</normaloff>
        </iconset>
    </property>
    <property name="text">
        <string>Top Section</string>
    </property>
</action>
```

### 4. Dynamic Icon Selection
```cpp
// Helper function for icon mapping
QIcon getArchaeologyIcon(const QString& name) {
    return QIcon(QString(":/icons/archaeology/%1.svg").arg(name));
}

// Usage
QIcon cortexIcon = getArchaeologyIcon("lithic_cortex");
QIcon profileIcon = getArchaeologyIcon("ceramic_profile");
```

## Icon Categories & Use Cases

### Section Tools (단면 도구)
**Use in**: Section analysis panel, profile extraction tools
- `section_top` - Top view section
- `section_front` - Front view section  
- `section_side` - Side view section
- `section_free` - Free-form section plane
- `section_profile` - Profile curve extraction

### Lithic Tools (석기 도구)
**Use in**: Stone tool analysis toolbar, lithic annotation panel
- `lithic_cortex` - Mark cortex regions (cross-hatch rendering)
- `lithic_ridge` - Highlight ridges (automatic detection)
- `lithic_flake_scar` - Mark flake scars
- `lithic_edge` - Edge analysis
- `lithic_measure_length` - Length measurement

### Ceramic Tools (토기 도구)
**Use in**: Pottery analysis panel, ceramic reconstruction tools
- `ceramic_profile` - Pottery profile view
- `ceramic_rim` - Rim analysis
- `ceramic_body` - Body section
- `ceramic_reconstruct` - Reconstruction mode

### Measurement Tools (측정 도구)
**Use in**: Measurement toolbar, quantitative analysis panel
- `measure_distance` - Distance measurement
- `measure_angle` - Angle measurement
- `measure_area` - Surface area calculation
- `grid_show/hide` - Toggle measurement grid
- `scale_bar` - Add scale bar to export

## Styling & Theming

### Change Icon Color
```cpp
// Method 1: QIcon with pixmap manipulation
QIcon icon(":/icons/archaeology/section_top.svg");
QPixmap pixmap = icon.pixmap(24, 24);
// Apply color transformation...

// Method 2: Use QPalette for automatic theming
// SVG icons automatically adapt to QPalette::ButtonText color
```

### High DPI Support
```cpp
// Icons automatically scale on high-DPI displays
// SVG format ensures crisp rendering at any size
QIcon icon(":/icons/archaeology/lithic_tool.svg");
icon.pixmap(QSize(24, 24)); // Standard DPI
icon.pixmap(QSize(48, 48)); // 2x DPI
```

## Integration with Archaeological Features

### Cortex Cross-Hatching (Task 004)
```cpp
// Toggle cortex visualization
if (cortexEnabled) {
    cortexButton->setIcon(QIcon(":/icons/archaeology/lithic_cortex.svg"));
    // Enable cross-hatch shader...
}
```

### Ridge Detection (Task 005)
```cpp
// Ridge emphasis control
QAction* ridgeAction = new QAction(
    QIcon(":/icons/archaeology/lithic_ridge.svg"),
    tr("Emphasize Ridges"),
    this
);
connect(ridgeAction, &QAction::triggered, this, &Widget::enableRidgeEmphasis);
```

### Section Export (Task 006+)
```cpp
// Export profile with icon in UI
QAction* exportAction = new QAction(
    QIcon(":/icons/archaeology/section_export.svg"),
    tr("Export Section"),
    this
);
```

## Testing

### Icon Rendering Test
```cpp
// Create test dialog to verify all icons load correctly
QDialog testDialog;
QGridLayout* layout = new QGridLayout(&testDialog);

QStringList iconNames = {
    "section_top", "lithic_cortex", "ceramic_profile", 
    "measure_distance", // ... all 64 icons
};

for (int i = 0; i < iconNames.size(); ++i) {
    QPushButton* btn = new QPushButton();
    btn->setIcon(QIcon(QString(":/icons/archaeology/%1.svg").arg(iconNames[i])));
    btn->setIconSize(QSize(32, 32));
    layout->addWidget(btn, i / 8, i % 8);
}

testDialog.exec();
```

## File Locations
- **Icons**: `A:\1105\GigaMesh\gui\resources\icons\archaeology\*.svg`
- **QRC**: `A:\1105\GigaMesh\gui\resources\icons\archaeology\archaeology_icons.qrc`
- **Resource Prefix**: `:/icons/archaeology/`

## Next Steps
1. ✓ Icons created (Task 003)
2. Add to CMakeLists.txt (integrate with build)
3. Update QGMMainWindow menus (use section/measure icons)
4. Update qgmdocksidebar panels (use lithic/ceramic icons)
5. Test icon rendering on Windows/Linux
6. Document icon usage in archaeological features

## Support
For icon modifications or additions, see Material Design icon guidelines:
- Stroke: 2dp
- Cap/Join: round
- Viewport: 24x24
- Format: SVG (XML)
