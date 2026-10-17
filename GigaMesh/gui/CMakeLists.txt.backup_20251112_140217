cmake_minimum_required(VERSION 3.10)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Gui OpenGL Network Concurrent REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(GUI_SOURCES src/main.cpp
				src/QGMMainWindow.cpp                 src/QGMMainWindow.h
				src/meshQt.cpp                        src/meshQt.h
				src/image2d.cpp                       src/image2d.h
				src/meshwidget.cpp                    src/meshwidget.h
				src/qgmdockinfo.cpp                   src/qgmdockinfo.h
				src/QGMDockMeasurement.cpp            src/QGMDockMeasurement.h
				src/QGMDialogMSII.cpp                 src/QGMDialogMSII.h
				src/QGMDialogAbout.cpp                src/QGMDialogAbout.h
				src/QGMDialogRuler.cpp                src/QGMDialogRuler.h
				src/qgmdocksidebar.cpp                src/qgmdocksidebar.h
				src/qgmdockview.cpp                   src/qgmdockinfo.h
				src/meshwidget_params.cpp             src/meshwidget_params.h
				src/QGMDialogComboBox.cpp             src/QGMDialogComboBox.h
				src/QGMDialogSliderHD.cpp             src/QGMDialogSliderHD.h
				src/QGMDialogConeParam.cpp            src/QGMDialogConeParam.h
				src/QGMDialogEnterText.cpp            src/QGMDialogEnterText.h
                                src/QGMDialogMatrix.cpp               src/QGMDialogMatrix.h
				src/QGMDialogPlaneParam.cpp           src/QGMDialogPlaneParam.h
				src/QGMDialogNprSettings.cpp          src/QGMDialogNprSettings.h
				src/MeshQtCSVImportExport.cpp         src/MeshQtCSVImportExport.h
				src/QGMDialogCutOffValues.cpp         src/QGMDialogCutOffValues.h
				src/dialogGridCenterSelect.cpp        src/dialogGridCenterSelect.h
				src/ExternalProgramsDialog.cpp        src/ExternalProgramsDialog.h
				src/qruntpsrpmscriptdialog.cpp        src/qruntpsrpmscriptdialog.h
				src/QGMDialogTransparencySettings.cpp src/QGMDialogTransparencySettings.h
				src/meshGL/meshGL.cpp                 src/meshGL/meshGL.h
				src/meshGL/meshglcolors.cpp           src/meshGL/meshglcolors.h
				src/meshGL/meshGL_params.cpp          src/meshGL/meshGL_params.h
				src/meshGL/meshGLShader.cpp           src/meshGL/meshGLShader.h
				src/meshGL/PinRenderer.cpp            src/meshGL/PinRenderer.h
				src/meshGL/TexturedMesh.cpp           src/meshGL/TexturedMesh.h
				src/meshGL/TexturedMeshRenderer.cpp   src/meshGL/TexturedMeshRenderer.h
                                src/meshGL/ShaderManager.cpp          src/meshGL/ShaderManager.h
				src/svg/SvgWriter.cpp                 src/svg/SvgWriter.h
				src/svg/SvgCircle.cpp                 src/svg/SvgCircle.h
				src/svg/SvgImage.cpp                  src/svg/SvgImage.h
				src/svg/SvgPath.cpp                   src/svg/SvgPath.h
				src/svg/SvgRect.cpp                   src/svg/SvgRect.h
				src/svg/SvgText.cpp                   src/svg/SvgText.h
				src/section.cpp                       src/section.h
				src/sectionmanager.cpp                src/sectionmanager.h
				src/qgmdocksections.cpp               src/qgmdocksections.h
				src/qgmdockproperty.cpp               src/qgmdockproperty.h
				src/QGMDockToolPalette.cpp            src/QGMDockToolPalette.h
				src/QGMDockMeasurements.cpp           src/QGMDockMeasurements.h
				src/DongArchToolPalette.cpp           src/DongArchToolPalette.h
				src/DongArchPropertyPanel.cpp         src/DongArchPropertyPanel.h
				src/DongArchMeasurementPanel.cpp      src/DongArchMeasurementPanel.h
				src/DongArchProjectFile.cpp           src/DongArchProjectFile.h
				src/DongArchPreferencesDialog.cpp     src/DongArchPreferencesDialog.h
				src/DongArchWelcomeScreen.cpp         src/DongArchWelcomeScreen.h
				src/meshwidget_sections.cpp
				src/DialogFindTextures.cpp            src/DialogFindTextures.h
				src/normalSphereSelection/ArcBall.cpp src/normalSphereSelection/ArcBall.h
				src/normalSphereSelection/NormalSphereSelectionDialog.cpp
				src/normalSphereSelection/NormalSphereSelectionDialog.h
				src/normalSphereSelection/NormalSphereSelectionRenderWidget.cpp
				src/normalSphereSelection/NormalSphereSelectionRenderWidget.h
                                src/automaticalignmentpyinterface.cpp src/automaticalignmentpyinterface.h
                                src/DongArchProject.cpp               src/DongArchProject.h
                                src/QGMDarkModeManager.cpp            src/QGMDarkModeManager.h
                                src/DongArchColors.h
                                src/analyticsConfig.h
                                # DongArch3D v4.0: C++20 Core Structures
                                src/dongarch/common/DongArchMath.cpp  src/dongarch/common/DongArchMath.h
                                src/dongarch/common/DongArchTypes.h
                                # DongArch3D v4.0 Phase 1: Align (Rotation & Alignment)
                                src/dongarch/align/DongArchAlignManager.cpp    src/dongarch/align/DongArchAlignManager.h
                                src/dongarch/align/DongArchAlignDialog.cpp     src/dongarch/align/DongArchAlignDialog.h
                                # DongArch3D v4.0 Phase 2: Cutline (Mesh-Plane Intersection)
                                src/dongarch/cutline/DongArchCutlineManager.cpp    src/dongarch/cutline/DongArchCutlineManager.h
                                src/dongarch/cutline/DongArchCutlineDialog.cpp     src/dongarch/cutline/DongArchCutlineDialog.h
                                # DongArch3D v4.0 Phase 2 Realtime: Async Pipeline + 16ms Debounce
                                src/dongarch/cutline/CutlineParams.h
                                src/dongarch/cutline/CutlineService.cpp            src/dongarch/cutline/CutlineService.h
                                src/dongarch/cutline/CutlineController.cpp         src/dongarch/cutline/CutlineController.h
                                src/dongarch/cutline/CutlineOverlayWidget.cpp      src/dongarch/cutline/CutlineOverlayWidget.h
                                src/dongarch/cutline/DongArchCutlineRealtimeDialog.cpp  src/dongarch/cutline/DongArchCutlineRealtimeDialog.h
                                src/dongarch/cutline/CutlineSlotManager.cpp             src/dongarch/cutline/CutlineSlotManager.h
                                src/dongarch/cutline/CutlineSVGExporter.cpp             src/dongarch/cutline/CutlineSVGExporter.h
                                # DongArch3D v4.0 Phase 2 고도화: Line Smoothing Algorithms
                                src/dongarch/cutline/algorithms/DouglasPeucker.cpp    src/dongarch/cutline/algorithms/DouglasPeucker.h
                                src/dongarch/cutline/algorithms/CatmullRomSpline.cpp    src/dongarch/cutline/algorithms/CatmullRomSpline.h
                                # DongArch3D v4.0 Phase 3-A: Outline (CPU Silhouette Detection)
                                src/dongarch/outline/SilhouetteDetector.cpp        src/dongarch/outline/SilhouetteDetector.h
                                # DongArch3D v4.0 Phase 3-B: Outline (Quadric Optimization - 2025 Paper)
                                src/dongarch/outline/algorithms/QuadricOptimizer.cpp    src/dongarch/outline/algorithms/QuadricOptimizer.h
                                # DongArch3D v4.0 Phase 3-C: Outline (Vector Tracing - Douglas-Peucker + DFS)
                                src/dongarch/outline/algorithms/VectorTracer.cpp        src/dongarch/outline/algorithms/VectorTracer.h
                                # DongArch3D v4.0 Phase 3-D: Outline (Sobel Integration - Dual-Pass Rendering)
                                src/dongarch/outline/OutlineExtractor.cpp              src/dongarch/outline/OutlineExtractor.h
                                # DongArch3D v4.0 Phase 4: Clip (3D Mesh Clipping - GigaMesh splitMesh Integration)
                                src/dongarch/clip/DongArchClipManager.cpp              src/dongarch/clip/DongArchClipManager.h
                                src/dongarch/clip/DongArchClipDialog.cpp               src/dongarch/clip/DongArchClipDialog.h
                                # DongArch3D v4.0 Phase 4 Advanced: Self-Intersection Repair (2025 Paper)
                                src/dongarch/clip/algorithms/SelfIntersectionRepairer.cpp    src/dongarch/clip/algorithms/SelfIntersectionRepairer.h
                                # DongArch3D v4.0 Phase 5-A: Vis (D-Tak - Heat Method GPU for Geodesic Distance)
                                src/dongarch/vis/DTakRenderer.cpp    src/dongarch/vis/DTakRenderer.h
                                # DongArch3D v4.0 Phase 5-B: Vis (X-Ray - Depth Peeling Transparency)
                                src/dongarch/vis/XRayRenderer.cpp    src/dongarch/vis/XRayRenderer.h
                                # DongArch3D v4.0 Phase 6: MFE (Mesh File Explorer)
                                src/dongarch/mfe/MFEManager.cpp      src/dongarch/mfe/MFEManager.h
                                src/dongarch/mfe/MFEWidget.cpp       src/dongarch/mfe/MFEWidget.h
                                # DongArch3D v4.0 Phase 7: Illustrator (SVG Export & Bridge)
                                src/dongarch/illustrator/SVGExporter.cpp           src/dongarch/illustrator/SVGExporter.h
                                src/dongarch/illustrator/IllustratorBridge.cpp     src/dongarch/illustrator/IllustratorBridge.h
                                )
#add ui files, so they appear it qtcreator projects view
set(GUI_UI_FILES
	src/normalSphereSelection/NormalSphereSelectionDialog.ui
	forms/QGMDialogTransparencySettings.ui
	forms/dialogComboBox.ui
	forms/qgmdialognprsettings.ui
	forms/qgmdockview.ui
	forms/qruntpsrpmscriptdialog.ui
	forms/dialogMSII.ui
	forms/QGMDialogSliderHD.ui
	forms/dialogAbout.ui
	forms/dialogGridCenterSelect.ui
	forms/dialogRuler.ui
	forms/dialogEnterText.ui
	forms/qgmdockinfo.ui
	forms/dialogConeParam.ui
	forms/ExternalProgramsDialog.ui
	forms/mainWin.ui
	forms/dialogPlaneParam.ui
	forms/qgmdocksidebar.ui
	forms/dialogCutOffValues.ui
	forms/DialogFindTextures.ui
        forms/QGMDialogMatrix.ui
        # DongArch3D v4.0 UI Files
        src/dongarch/mfe/MFEWidget.ui
)

#NOTE: instead of adding translations via resources, the ts-files can also be build by
#      qt5_create_translation and then be added
qt5_add_resources(GUI_RESOURCES "forms/gigamesh.qrc"
								"src/shaders/shaders.qrc"
								"languages/translations.qrc"
								"latex_report_template/latex_templates.qrc"
								"python-scripts/pythonscripts.qrc"
								"resources.qrc"
								# "resources/icons/archaeology/archaeology_icons.qrc"  # TODO: SVG 파일 생성 후 활성화
)

#set here the paths of the ui-files, so autouic can find them
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms
							   ${CMAKE_CURRENT_SOURCE_DIR}/normalSphereSelection)

add_executable(DongArch3D ${GUI_SOURCES} ${GUI_RESOURCES} ${GUI_UI_FILES})

if(APPLE)
    set_target_properties(DongArch3D PROPERTIES
        MACOSX_BUNDLE ON)
endif()

add_definitions(${Qt5Widgets_DEFINITIONS})
target_include_directories(DongArch3D PRIVATE ${Qt5Widgets_INCLUDES})

add_definitions(${Qt5OpenGL_DEFINITIONS})
target_include_directories(DongArch3D PRIVATE ${Qt5OpenGL_INCLUDES})

add_definitions(${Qt5Network_DEFINITIONS})
target_include_directories(DongArch3D PRIVATE ${Qt5Network_INCLUDES})

add_dependencies(DongArch3D OpenGL::GL)

#needed to find generated ui-header files
target_include_directories(DongArch3D PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

#needed, if ui-header files need references to custom classes (e.g. NormalSphereSelectionRenderWidget.h)
target_include_directories(DongArch3D PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(DongArch3D PRIVATE ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Concurrent_LIBRARIES} psalm OpenGL::GL OpenGL::GLU gigameshCore triangle)

#extras for static linking using the MXE mingw static build (set this via "export MINGW_CI_BUILD=True" on the command line)
if(MINGW_CI_BUILD)
	qt5_import_plugins(DongArch3D INCLUDE Qt5::QSvgIconPlugin Qt5::QSvgPlugin)
	target_link_libraries(DongArch3D PRIVATE ssl crypto crypt32 iphlpapi)
endif()

if(MSVC)
    target_sources(DongArch3D PRIVATE GigaMesh_Logo.rc)
elseif(MINGW)
    target_sources(DongArch3D PRIVATE GigaMesh_Logo.rc)
endif()

install(TARGETS DongArch3D DESTINATION bin)
