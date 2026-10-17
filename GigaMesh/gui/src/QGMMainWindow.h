/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QGMMAINWINDOW_H
#define QGMMAINWINDOW_H

// generic C++ includes:
#include <iostream>

// C++ includes:
#include <GigaMesh/mesh/primitive.h> //! \todo source refactoring to get rid of this include.

// generic Qt includes:
#include <QtGui>
#include <QMainWindow>
#include <QtOpenGL/QGLFormat>
#include <QLabel>
#include <QProgressBar>

// Qt Interface includes:
#include "ui_mainWin.h"

// Qt includes:
#include "QGMDialogAbout.h"

// Parameters, which are set by the mainwindow's menus:
#include <GigaMesh/mesh/mesh_params.h>
#include "meshwidget_params.h"
#include "meshGL/meshGL_params.h"
#include "meshGL/meshglcolors.h"

class MeshWidget;
class QGMDockSideBar;
class QGMDockInfo;
class QGMDockView;
class QGMDockSections;
class QGMDockProperty;
class QGMDockToolPalette;
class QGMDockMeasurements;
class DongArchPropertyPanel;
class DongArchMeasurementPanel;
class SectionManager;
class Section;

// DongArch Cutline namespace forward declaration
namespace DongArch {
namespace Cutline {
	class DongArchCutlineRealtimeDialog;
}
}

// Qt classes used and included in .cpp
class QNetworkAccessManager;
class QNetworkReply;

//!
//! \brief Main window class for GigaMesh (Layer 3)
//!
//! Derived from QMainWindow: http://doc.trolltech.com/4.5/qmainwindow.html
//!
//! Interface design is - and must be - done using the Qt Designer: http://doc.trolltech.com/4.3/designer-manual.html
//!
//! This class is mainly responsible for handling the communication (signals and slots) between the
//! GUI and our pure C++ classes.
//!
//! Layer 3
//!

class QGMMainWindow : public QMainWindow, public Ui::MainWindow {
    Q_OBJECT

public:
	// Constructor and Destructor:
    QGMMainWindow( QWidget *parent = nullptr, Qt::WindowFlags flags = {} );
	~QGMMainWindow();
private:
	void initMeshWidgetSignals(); // to be called ONLY from the constructor.
	void initMeshSignals(); // to be called ONLY from the constructor.
	void setupDockLayout(); // Setup dock widget layout
	void saveDockLayout(); // Save dock layout to QSettings
	void restoreDockLayout(); // Restore dock layout from QSettings

public:
	bool setupMeshWidget( const QGLFormat& rGLFormat );
	bool setupHighDPI20();

protected:
	virtual void closeEvent( QCloseEvent* rEvent ) override;
	virtual bool event( QEvent* rEvent ) override;

	virtual void dragEnterEvent( QDragEnterEvent *e ) override;
	virtual void dropEvent( QDropEvent *e ) override;

	//  === SLOTS === The main windows slots should be kept as simple as possible. =========================================================================
public slots:

	// --- File Open, Import and Export  -----------------------------------------------------------------------------------------------------------------------
	void load();                    // Inherited from QGMMainWindow
	void load(const QString& rFileName );
	bool loadLast();
	bool fileOpen( QAction* rFileAction );
	void menuImportFunctionValues();
    void menuImportLabels();
    void menuImportPolylines();
    void menuImportTransMat ();
	void menuImportTexMap();
	void menuImportFeatureVectors();
	void menuImportNormalVectors();

	// --- MENU - MeshWidget --------------------------------------------------------------------------------------------------------------------------------
	bool setMeshWidgetFlag( QAction* rAction );
	bool setMeshWidgetParamInt( QAction* rAction );
	bool setMeshWidgetParamFloat( QAction* rAction );

    void activateInspectionOptions();

	// --- MENU - Mesh & MeshGL ----------------------------------------------------------------------------------------------------------------------------
	bool setMeshGLFlag( QAction* rAction );
	bool setMeshGLParamInt( QAction* rAction );
	bool setMeshGLParamFloat( QAction* rAction );
	bool setMeshGLColor( QAction* rAction );
	bool callFunction( QAction* rAction );

	// --- -------------------------------------------------------------------------------------------------------------------------------------------------
	//bool getParamIDandState( QAction* rAction, const char* rName, int* rID, int* rState );
	bool getFlagIDandState( QAction* rAction, const char* rName, int* rID, bool* rState );
	bool getParamIDValueState( QAction* rAction, const char* rName, const char* rValueName, int* rID, int* rValue, bool* rState );

	bool getParamID( QAction* rAction, const char* rName, int* rID );
	bool getParamValue( QAction* rAction, const char* rName, double* rValue );

	// View:
	void toggleFullscreen();
	void setWidgetSizeFixed( bool rFixed );

	// ?
	void infoKeyShortcuts();
	void visitVideoTutorials();
	void visitWebSite();
	void aboutBox();

	// --- DYNAMIC MENU -----------------------------------------------------------------------
	void updateMeshShowFlag( MeshGLParams::eParamFlag rShowFlagNr, bool rSetState );
	void updateMeshParamInt( MeshGLParams::eParamInt rParam, int rValue );
	void updateWidgetShowFlag( MeshWidgetParams::eParamFlag rFlag, bool rState );
	void updateWidgetShowInteger( MeshWidgetParams::eParamInt rParam, int rValue );
	void updateMeshElementFlag( int elementFlagNr, bool setState );

	// --- VARIOUS ----------------------------------------------------------------------------
	void fileChanged( const QString& rFileNameFull, const QString& rFileNameBase );
	void updateRecentFileMenu();
	void setStatusBarMessage(const QString& messageStr );

	// --- Menu items with dependencies -------------------------------------------------------
	void setMenuContextToSelection( Primitive* primitive );

private slots:
	// --- Network communication ---------------------------------------------------------------------------------------------------------------------------
	void slotHttpCheckVersion( QNetworkReply* rReply );
    void slotHttpAnalytics( QNetworkReply* rReply );
	// --- Language changes ---------------------------------------------------------------------------------------------------------------------------
	void slotChangeLanguage(QAction* action);

private:
	//loads a language by the given language shortcut (e.g. de, en)
	void loadLanguage(const QString& language);

	//creates language menu dynamically by the provided translations in the language resource path
	void createLanguageMenu();

	//creates Korean menu bar system (Task 105)
	void createMenuBar();

	//creates main toolbar with 20 frequently used tools (Task 106)
	void createToolBar();

	QString mCurrentLanguage;
	QTranslator mTranslator;
	QTranslator mTranslatorQt;

private slots:
	void openExternalProgramsDialog();
	void openGridPositionDialog();

	// --- Section Management (Archaeological) ---------------------------------------------
	void onCreateSection(int type);  // type is Section::SectionType
	void onRemoveSection(int index);
	void onExportSection(int index);
	void onExportAllSections();
	void onSectionPropertyChanged();  // Handle property changes from property panel

	// --- DongArch Clip (Interactive mesh cutting) -------------------------------------
	void onShowClipDialog();  // Show Clip dialog for real-time mesh cutting

	// --- Korean Menu Actions (Task 105) ----------------------------------------------
	// File Menu
	void onNewProject();
	void onOpenProject();
	void onSaveProject();
	void onSaveProjectAs();

	// --- Project File Actions (Task 112) ----------------------------------------------
	void onSaveProjectFile();        // 프로젝트 파일 저장 (.dongarch3d)
	void onOpenProjectFile();        // 프로젝트 파일 열기 (.dongarch3d)
	void onImportPLY();
	void onImportOBJ();
	void onImportSTL();
	void onExportPNG();
	void onExportSVG();
	void onExportPDF();
	void onExportDXF();

	// Edit Menu
	void onUndo();
	void onRedo();
	void onSelectAll();
	void onSelectInvert();
	void onSelectNone();

	// Measurement Menu (실측)
	void onMeasureDistance();
	void onMeasureAngle();
	void onMeasureArea();
	void onSectionTop();
	void onSectionFront();
	void onSectionSide();
	void onSectionCustom();
	void onExtractProfile();

	// Artifact Menu (유물)
	void onLithicMode();
	void onLithicCortex();
	void onLithicStrikeDirection();
	void onCeramicMode();
	void onCeramicUnroll();
	void onCeramicRimAnalysis();
	void onMetalMode();

	// View Menu
	void onViewTop();
	void onViewFront();
	void onViewRight();
	void onViewIsometric();
	void onToggleGrid();
	void onToggleDarkMode();  // Task 108: Dark Mode toggle

	// v4 Phase 1: Align - ViewPoint 6개 (나머지 3개)
	void onViewBottom();      //!< 하면 뷰 (Numpad 9)
	void onViewBack();        //!< 후면 뷰 (Ctrl+1)
	void onViewLeft();        //!< 좌측면 뷰 (Ctrl+3)

	// v4 Phase 1: Align - Rotation 변환
	void onRotateX();         //!< X축 회전 (90도)
	void onRotateY();         //!< Y축 회전 (90도)
	void onRotateZ();         //!< Z축 회전 (90도)

	// v4 Phase 1: Align - Ground Plane
	void onShowGroundPlane(); //!< Ground Plane 표시/숨김
	void onFitGroundPlane();  //!< Ground Plane 자동 정렬

	// v4 Phase 2: Cutline (단면 라인 추출)
	void onCutlineTop();      //!< Top Cutline (XY 평면, Z=height)
	void onCutlineFront();    //!< Front Cutline (XZ 평면, Y=distance)
	void onCutlineRight();    //!< Right Cutline (YZ 평면, X=distance)
	void onCutlineCustom();   //!< Custom Cutline (사용자 정의 평면)

	// Rendering Menu
	void onRenderWireframe();
	void onRenderSolid();
	void onRenderNPR();

	// Toolbar-specific actions (Task 106)
	void onSelectTool();
	void onCreateSection();
	void onSettings();

	// --- Tool Palette Actions (CRITICAL FIX) -----------------------------------------
	void onToolActivated(int toolId);  //!< Handle tool palette button clicks
	void onToolSelectionChanged(int toolId);  //!< Handle tool selection changes

	// Help Menu
	void onShowManual();
	void onShowTutorials();
	void onShowShortcuts();
	void onShowAbout();
	void onShowWelcomeScreen();  // Task 110: Welcome Screen

	// View Mode Control (Task 111: Keyboard Shortcuts)
	void onActivateRotateMode();
	void onActivatePanMode();

public slots:
	// --- Status Bar Update Functions (Task 107) --------------------------------------
	void updateStatusCoord(float x, float y, float z);          //!< 상태바 좌표 업데이트
	void updateStatusMesh(int vertices, int faces);             //!< 상태바 메시 정보 업데이트
	void updateStatusFPS(int fps);                              //!< 상태바 FPS 업데이트
	void updateStatusSelection(int count);                      //!< 상태바 선택 개수 업데이트
	void updateProgressBar(int value, int maximum, const QString& text = QString());  //!< 진행률 표시
	void setProgress(int value, int max);                       //!< 레거시 호환성 (updateProgressBar 래퍼)

private:
	// --- Status Bar Display (Task 107) -----------------------------------------------
	void setupStatusBar();

	// --- Keyboard Shortcut System (Task 111) -----------------------------------------------
	void setupKeyboardShortcuts();
	void setupNumericKeyShortcuts();
	void setupViewShortcuts();

	// --- Extra Keys --------------------------------------------------------------------------------------------------------------------------------------
	void keyPressEvent( QKeyEvent *rEvent ) override;
	void keyReleaseEvent( QKeyEvent *rEvent ) override;

signals:
	// --- File --------------------------------------------------------------------------------------------------------------------------------------------
	void sFileOpen(QString);                                 //!< causes MeshWidget to load and show a Mesh from a file (see MeshWidget::loadMeshFromFile)
	void sFileReload();                                      //!< causes MeshWidget to reload the file
	void sFileImportFunctionValues( QString );               //!< passes a filename for import of a file with function values per vertices to MeshQt::importFunctionValues
    void sFileImportLabels( QString );                       //!< passes a filename for import of a file with labels per vertices to MeshQt::importLabels
    void sFileImportPolylines( QString );                    //!< passes a filename for import of a file with Polylines to MeshQt::importPolylines
    void sFileImportTransMat( QString );                    //!< passes a filename for import of a file with transformation matrices (transmat.txt) to MeshQt::importApplyTransMat
	void sFileImportTexMap( QString );                       //!< passes a filename for import of a file with a colors per verices ("texture map") to MeshQt::importTexMapFromFile
	void sFileImportFeatureVectors( QString );               //!< passes a filename for import of a file with feature vectors to MeshWidget::importFeatureVectorsFile
	void sFileImportNormals( QString );                      //!< passes a filename for import of a file with normal vectors to MeshQt::importNormalVectorsFile
	void sFileSaveFlagBinary( bool );                        //!< passed down to MeshIO. However this has to be revised.
	void sFileSaveFlagExportTexture( bool );                 //!< passed down to MeshIO. This probably also needs to be revised.
	void exportFuncVals();                                   //!< signal MeshQt to export the function values.
	void sExportFeatureVectors();                            //!< signal MeshQt to export the feature vectors.
	//.
	void exportPolyLinesCoords();                            //!< Triggers the export of polyline coordinates (3D).
	void exportPolyLinesCoordsProjected();                   //!< Triggers the export of polyline coordinates project on a plane.
	void exportPolyLinesFuncVals();                          //!< triggers the export of run-length and function values of (selected) polylines.
	//.
	void exportFaceNormalAngles();                           //!< triggers the export of the faces normals as sphereical angles
	void exportNormalSphereData();                           //!< triggers the export of the normal sphere data
	//.
	void saveStillImages360HLR();                            //!< triggers the generation of still images for a horizontal 360° rotation (left->right).
	void saveStillImages360VUp();                            //!< triggers the generation of still images for a vertical 360° rotation (upwards).
	void saveStillImages360PrimN();                          //!< triggers the generation of still images for a 360° rotation about the selected primitves normal.
	void saveStillImages360PlaneN();                         //!< triggers the generation of still images for a 360° rotation about the mesh plane.
	//.
	void sphericalImagesLight();                             //!< triggers the generation of still images with the light source in the spherical orbit.
    void sphericalImagesLightDir();                          //!< triggers the generation of still images with the light source in the spherical orbit for a whole directory.
	void sphericalImages();                                  //!< triggers the generation of still images in a spherical orbit.
	void sphericalImagesStateNr();                           //!< triggers the selection of the state nr. used for naming the files during export.
	//.
	void unloadMesh();                                       //!< triggers the removal of the current Mesh

	// --- Edit --------------------------------------------------------------------------------------------------------------------------------------------
	void removeVerticesSelected();                           //!< triggers the removal of selected vertices (and faces they belong to).
	void removeUncleanSmall();                               //!< triggers calls to select small, solo, non-manifold and double-cones and removes them.
	//.
	void cutOffFeatureVertex();                              //!< triggers cutting of elements of feature vectors (per Vertex).
	void cutOffFeatureFace();                                //!< triggers cutting of elements of feature vectors (per Face - not implemented).
	//.
	void funcValSet();                                       //!< trigger dialog to set all function values to a given value.
	void funcValueCutOff();                                  //!< triggers cutting of the (vertex) function values.
	void funcValsNormalize();                                //!< trigger the normalization of the function values to [0.0 ... 1.0].
	void funcValsAbs();                                      //!< Compute absolut value of the vertices function values.
	void funcValsAdd();                                      //!< Add a constant value to the vertices function values.
	//-
	void transformFunctionValuesToRGB();
    //-
    void sDownscaleTexture();                                //!< triggers the downscaling options, rescale and overwrite the texture
	//.
	void setConeData();                                      //!< triggers showing the cone data dialog window
	void centerAroundCone();                                 //!< triggers centering the mesh around a user-specified cone
	//.
	void sSplitByPlane();                                    //!< triggers splitting the mesh by the specified plane.
	void sSplitByIsoValue();                                 //!< triggers splitting the mesh by an iso-value using the vertices function values.
	//.
	void centerAroundSphere();                               //!< triggers centering the mesh around a user-specified sphere
	void unrollAroundSphere();                               //!< triggers unrolling the mesh around a user-specified sphere
	//.
	void sApplyMeltingSphere();                              //!< triggers melting with sqrt(r^2-x^2-y^2)
    //.
    void sAutomaticMeshAlignment();                          //! triggers the automatic mesh alignmented with PCA
    void sAutomaticMeshAlignmentDir();                       //! triggers the automatic mesh alignmented with PCA for a whole directory
	// --- DeSelect ----------------------------------------------------------------------------------------------------------------------------------------
	void sDeSelVertsAll();                                   //!< removes all vertices from the selection (SelMVerts).
	void sDeSelVertsNoLabel();                               //!< removes vertices from the selection (SelMVerts) not assigned to a label.
	// --- Select ------------------------------------------------------------------------------------------------------------------------------------------
	void getPlaneVPos();                                     //!< Show info about the Mesh plane.
	void getPlaneHNF();                                      //!< Show the Hesse Normal Form (HNF) of the Mesh plane.
	void setPlaneVPos();                                     //!< trigger textbox to enter three vertices defining the Mesh plane position.
	void setPlaneHNF();                                      //!< Set the Hesse Normal Form (HNF) of the Mesh plane.
    void setPlaneHNFByView();                                //!< Set the Hesse Normal Form (HNF) of the Mesh plane based on the view
	//.
	void selectVertFuncLT();                                 //!< trigger selection of vertices with a function value lower than ....
	void selectVertFuncGT();                                 //!< trigger selection of vertices with a function value larger than ....
    //.
    void selectVertNonMax();                                 //!< trigger selection of vertices via Non-Maximum Suppression ....
	//.
	void selectVertLocalMin();                               //!< trigger selection of vertices tagged as local minimum.
	void selectVertLocalMax();                               //!< trigger selection of vertices tagged as local maximum.
	//.
	void selectVertSolo();                                   //!< trigger: add solo vertices to the vertex selection.
	void selectVertNonManifoldFaces();                       //!< trigger: add vertices from non-manifold faces to the vertex selection.
	void selectVertDoubleCone();                             //!< trigger: add double-cone vertices to the vertex selection
	void selectVertLabelAreaLT();                            //!< trigger: add vertices from small labels to the vertex selection.
	void selectVertLabelAreaRelativeLT();                    //!< trigger: same as selectVertLabelAreaLT(), but relative (%).
	void selectVertBorder();                                 //!< trigger: add vertices of the Mesh border (excludes solo vertices!).
	void selectVertFaceMinAngleLT();                         //!< trigger: add vertices of faces with minimum angle lower than ...
	void selectVertFaceMaxAngleGT();                         //!< trigger: add vertices of faces with maximum angle larger than ...
	void sSelVertLabeledNot();                               //!< add vertices having no label no. set (and are also not background!).
	//.
	void selectFaceNone();                                   //!< trigger: empty selection of faces.
	void selectFaceSticky();                                 //!< trigger: add sticky faces to selection.
	void selectFaceNonManifold();                            //!< trigger: add non-manifold faces to selection.
	void selectFaceZeroArea();                               //!< trigger: add faces with zero area to selection.
	void selectFaceInSphere();                               //!< trigger: add faces within a sphere to the selection.
	void selectFaceRandom();                                 //!< trigger: random selection of faces.
	//.
	void selectPolyNoLabel();                                //!< selects all polylines not belonging to a label.
	void selectPolyNotLabeled();                             //!< select polylines not related to a label.
	void selectPolyRunLenGT();                               //!< select polylines with a run length larger than ...
	void selectPolyRunLenLT();                               //!< select polylines with a run length lower than ...
	void selectPolyLongest();                                //!< selects the largest polyline.
	void selectPolyShortest();                               //!< selects the shortest polyline.
	void selectPolyLabelNo();                                //!< select polylines by label IDs.

	// --- View --------------------------------------------------------------------------------------------------------------------------------------------
	// ... Vertices
	void polylinesCurvScale();                               //!< trigger selection of scale for polyline normals.
	//.
	void screenshotsCrop(bool);                              //!< toggle croping of screenshots using the z-buffer.
	void screenshotSVG();                                    //!< trigger a screenshot stored as SVG with a PNG embedded.
	void screenshotRuler();                                  //!< trigger the export of an image of a ruler matching the screenshot resolution (in ortho mode).

	//.
	void sDefaultViewLight();                                //!< signal to restore the default view and lights.
	void sDefaultViewLightZoom();                            //!< signal to restore the default view, lights and zoom.
    void sSetDefaultView();                                  //!< signal to meshWidget -> set the mesh after transformation to the camera center and ask for saving the transformation as default.
    //.
	void rotYaw();                                           //!< trigger manual entry of a yaw angle for changing the camera position.
	void rotPitch();                                         //!< trigger manual entry of a pitch angle for changing the camera position.
	void rotRoll();                                          //!< trigger manual entry of a roll angle for changing the camera position.
	void rotOrthoPlane();                                    //!< trigger rotation of the camera to orthogonally view the mesh plane.
	//.
	void sSelPrimViewReference();                            //!< signal to set the selected primitive as reference point of the camera's view.

	// --- Analyze -----------------------------------------------------------------------------------------------------------------------------------------
	void labelFaces();                                       //!< trigger labeling with optional removal of small areas.
	void labelSelectionToSeeds();                            //!< triggers the storage of selected vertices as seeds for labeling.
	void labelVerticesEqualFV();                             //!< trigger labeling of vertices having the same function value.
    void labelVerticesEqualRGB();                            //!< trigger labeling of vertices having the same color values.
	void sLabelSelMVertsToBack();                            //!< Set the selected vertices label to background.
	//.
	void convertSelectedVerticesToPolyline();                //!< trigger conversion from selected vertices to (a) polyline(s).
	void convertBordersToPolylines();                        //!< trigger computation of mesh borders to be stored as polyline(s).
	void convertLabelBordersToPolylines();                   //!< trigger conversion from borders of label(s) to (a) polyline(s).
    void createSkeletonLine();                               //!< trigger creation of skeleton line out of selected vertices
	void advancePolyThres();                                 //!< trigger the advance of the polylines to the function value threshold Mesh::funcValueThres.
	void sPolylinesCompIntInvRunLen();                       //!< trigger integral invariant for polylines - run-length.
	void sPolylinesCompIntInvAngle();                        //!< trigger integral invariant for polylines - angle.
	void sPolylinesCompCurv();                               //!< trigger the polyline curvature and extrema estimation.
	void setLengthSmooth();                                  //!< trigger input form for paramter Mesh::smoothLength
	void sPolylinesCopyNormalToVertices();                   //!< trigger the copying the polylines' normals to their vertices.
	//.
    void nonMaxCorrCheck();                                  //!< trigger check of Non-Maximum Suppression correctness ....
	//.
	void sLabelFindFuncValMinima();                          //!< trigger search for minima within labels.
	void sLabelFindFuncValMaxima();                          //!< trigger search for maxima within labels.
	//.
	void estimateMSIIFeat();                                 //!< triggers the MSII for a single (selected) Primitve.
	//.
	void sGeodPatchVertSel();                                //!< triggers the estimation of the geodesic distances for the selected vertices.
	void sGeodPatchVertSelOrder();                           //!< triggers the estimation of the geodesic distances for the selected vertices -- sequentially computed.
	//.
	void intersectSphere();                                  //!< emitted when a sphere shall be intersected with the surface.
	//.
	void fillHoles();                                        //!< trigger filling of holes.
	//.
	void estimateVolume();                                   //!< emitted when the volume has to be estimated
	void compVolumePlane();                                  //!< compute volume to Mesh plane,
	void estimateBBox();                                     //!< emitted when the bounding box has to be estimated
	void sPolylinesLength();                                 //!< compute and show the length of the polylines.
	//.
	void hueToFuncVal();                                     //!< trigger hue estimation, which we will be assigned as function value.
	//.
	void sDatumAddSphere();                                  //!< Manually enter a datum sphere.

	// --- Octree reöated ----------------------------------------------------------------------------------------------------------------------------------
	void generateOctree();
	void generateOctree(unsigned int);
	void detectselfintersections();
	void drawOctree();
	void removeOctreedraw();
	void deleteOctree();

	// #####################################################################################################################################################
	// # FUNCTION VALUE
	// #####################################################################################################################################################
	// # Feature Vector related
	void sFuncVertFeatLengthEuc();                           //!< Visualize the length of the feature vector using euclidean metric.
	void sFuncVertFeatLengthMan();                           //!< Visualize the length of the feature vector using manhattan metric.
	void sFuncVertFeatBVFunc();                              //!< Visualize the Bounded Variation (BV) the feature vectors.
	void sFuncVertFeatTVSeqn();                              //!< Visualize the Total Variation (TV) of the feature vector.
	void sFuncVertFeatDistSelVertEuc();                      //!< Draw Euclidean feature distance to selection using OpenGL.
	void sFuncVertFeatDistSelVertEucNorm();                  //!< Draw Normalized euclidean feature distance to selection using OpenGL.
	void sFuncVertFeatDistSelVertMan();                      //!< Draw Manhattan feature distance to selection using OpenGL.
	void sFuncVertFeatDistSelVertCosSim();                   //!< Compute and visualize the Cosine Similarity to the Feature Vector of the Selected Vertex.
	void sFuncVertFeatDistSelVertTanimoto();                 //!< Compute and visualize the Tanimoto to the Feature Vector of the Selected Vertex.
	void sFuncVertFeatCorrSelVert();                         //!< Visualize Correlation to a selected Vertex using OpenGL.
	void sFuncVertFeatAutoCorrVert();                        //!< Visualize Auto Correlation of Vertices features using OpenGL.
	void sFuncVertFeatAutoCorrSelVert();                     //!< Visualize Auto-Correlation and Correlation to a selected Vertex using OpenGL.
	void sFuncValToFeatureVector();                          //!< Assign function value to Nth feature vector component
	// # Distance to plane, line, selected primitive and cone
	void visualizeDistanceToPlane();                         //!< triggers the plane distance estimation.
	void visualizeDistanceToCone();                          //!< triggers distance to cone estimation (if cone has been selected)
	// # Other
	void visualizeVertexIndices();                           //!< triggers coloring of vertices by index.
	void sFuncVert1RingRMin();                               //!< Compute r_min_i for 1-rings.
	void sFuncVert1RingVolInt();                             //!< Compute voluem integral for r->0.
	void visualizeVertexOctree();                            //!< triggers octree visualization.
	void visualizeVertexFaceSphereAngleMax();                //!< Visualize the max face angle to the vertex normal within a spherical neighbourhood.
	void visualizeVertFaceSphereMeanAngleMax();              //!< Visualize the max face angle to the mean normal of the faces within a spherical neighbourhood.
	// #####################################################################################################################################################

	// --- Texture Map -------------------------------------------------------------------------------------------------------------------------------------
	void visualizeFeature(int,int,double,double);            //!< Primitive Type, Element Nr, max and min Value

	// --- Colors ------------------------------------------------------------------------------------------------------------------------------------------
	void selectColorBackground();                            //!< triggers the interactive selection of a background color (in MeshWidget).

	// --- MeshWidget related parameters -------------------------------------------------------------------------------------------------------------------
	void sShowFlagMeshWidget(MeshWidgetParams::eParamFlag,bool);                //!< Sets a specific display flag see MeshWidgetParams::setShowFlag
	void sShowParamIntMeshWidget(MeshWidgetParams::eParamInt);                  //!< Request an user interaction to set a specific integer parameter -- see MeshWidgetParams::setViewParamsInt.
	void sShowParamIntMeshWidget(MeshWidgetParams::eParamInt,int);              //!< Sets a specific param int see MeshWidgetParams::setViewParamsInt
	void sShowParamFloatMeshWidget(MeshWidgetParams::eParamFlt);                //!< Request an user interaction to set a specific integer parameter -- see MeshWidgetParams::setViewParamsFloat. (ID only).
	void sShowParamFloatMeshWidget(MeshWidgetParams::eParamFlt,double,double);  //!< Request an user interaction to set a specific integer parameter -- see MeshWidgetParams::setViewParamsFloat. (ID/minVal/,axVal).
	void sCallFunctionMeshWidget(MeshWidgetParams::eFunctionCall,bool);         //!< Request a method/function of the MeshWidget class to be executed.
	void sEnterCortexSelectionMode(bool);                                       //!< Enter/exit cortex vertex selection mode.
	void sClearCortexSelection();                                                //!< Clear all cortex vertex flags.

	// --- MeshGL related parameters -----------------------------------------------------------------------------------------------------------------------
	void sShowParamFlagMeshGL(MeshGLParams::eParamFlag,bool);                 //!< Sets a specific display flag see MeshGLParams::setShowFlag
	void sShowParamIntMeshGL(MeshGLParams::eParamInt,int);                    //!< Sets a specific param int see MeshGLParams::setViewParamsInt
	void sShowParamIntMeshGLDialog(MeshGLParams::eParamInt);                  //!< Request an user interaction to set a specific integer parameter -- see MeshGLParams::setViewParamsInt.
	void sShowParamFloatMeshGLDialog(MeshGLParams::eParamFlt);                //!< Request an user interaction to set a specific floating point parameter -- see MeshGLParams::setViewParamsFloat. (ID only).
	void sShowParamFloatMeshGLLimits(MeshGLParams::eParamFlt,double,double);  //!< Request an user interaction to set a specific limited floating point parameter -- see MeshGLParams::setViewParamsFloat. (ID/minVal/,axVal).
	void sShowParamColorMeshGL(MeshGLColors::eColorSettings);                 //!< Request an user interaction to select a specific color.
	void sCallFunctionMeshGL(MeshGLParams::eFunctionCall,bool);               //!< Request a method/function of the MeshGL class to be executed.
	// --- Mesh related parameters -------------------------------------------------------------------------------------------------------------------------
	void sShowParamFloatMeshDialog(MeshParams::eParamFlt);                    //!< Request an user interaction to set a specific floating parameter -- see MeshParams::setViewParamsFloat. (ID only).
	void sShowParamFloatMeshLimits(MeshParams::eParamFlt,double,double);      //!< Request an user interaction to set a specific limited floating parameter -- see MeshParams::setViewParamsFloat. (ID/minVal/,axVal).
	void sCallFunctionMesh(MeshParams::eFunctionCall,bool);                   //!< Request a method/function of the Mesh class to be executed.
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	// DockInfo:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Mousemode:
	void sSelectMouseModeDefault();                                  //!< emitted, when the next mousmode of the combobox should be selected -- typically related to pressing the spacebar.
	void sSelectMouseModeExtra(bool,MeshWidgetParams::eMouseModes);  //!< emitted, when the extra mousemode should be activated/deactivated -- typicall related to pressing/releasing the ctrl/alt/shift-key.
	// User Guide:
	void sGuideIDCommon(MeshWidgetParams::eGuideIDCommon);           //!< emitted, when a new image of the used guide has to be shown.
	void sGuideIDSelection(MeshWidgetParams::eGuideIDSelection);     //!< emitted, when a new image of the used guide has to be shown.
	// Progress bar
	void sShowProgressStart(QString);                                //!< emitted, when some longer progress begins -- the string will be shown additionally to the progress bar.
	void sShowProgress(double);                                      //!< emitted, when a certain percentage of the progress is reached -- will update the progress bar.
	void sShowProgressStop(QString);                                 //!< emitted, when some progress end -- the string will be shown additionally to the progress bar.
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	// DockView:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	void sViewPortInfo(MeshWidgetParams::eViewPortInfo,QString);     //!< Infos emitted by the viewport (MeshWidget) e.g. for display purposes.
	void sInfoMesh(MeshGLParams::eInfoMesh,QString);                 //!< Infos emotted by the mesh (MeshQt) e.g. for display purposes.
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	// SideDock:
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	void sShowNPRSettings();
	void sShowTransparencySettings();
	//------------------------------------------------------------------------------------------------------------------------------------------------------

	void sOpenNormalSphereSelectionDialogVertices();
	void sOpenNormalSphereSelectionDialogFaces();

private:
	// Specialized UI class:
	MeshWidget*                       mMeshWidget;                    //!< QWidget for OpenGL

	// Sidebar DockWidgets:
	QGMDockSideBar*                   mDockSurface;                   //!< QDockWidget for surface visualization.
	QGMDockInfo*                      mDockInfo;                      //!< QDockWidget for information.
	QGMDockView*                      mDockView;                      //!< QDockWidget for information and settings regarding the viewport.
	QGMDockSections*                  mDockSections;                  //!< QDockWidget for archaeological sections management.
	QGMDockProperty*                  mDockProperty;                  //!< QDockWidget for object properties display and editing.
	QGMDockToolPalette*               mDockToolPalette;               //!< QDockWidget for tool palette (Task 101/102).
	QGMDockMeasurements*              mDockMeasurements;              //!< QDockWidget for measurements panel (Task 101/104).
	DongArchPropertyPanel*            mDongArchPropertyPanel;         //!< QDockWidget for DongArch3D property panel (Task 103).
	DongArchMeasurementPanel*         mDongArchMeasurementPanel;      //!< QDockWidget for DongArch3D measurement panel (Task 104).

	// Section Management:
	SectionManager*                   mSectionManager;                //!< Manager for archaeological cross-sections.

	// DongArch Cutline Dialog:
	DongArch::Cutline::DongArchCutlineRealtimeDialog* mCutlineDialog = nullptr;  //!< Realtime cutline dialog (Stage 1-5)

	// DongArch Section overlay helper functions (Arch3D Liner style)
	QVector<QPointF> projectPolylineToScreen(const QVector<QVector3D>& polyline3D);  //!< Project 3D polyline to 2D screen coordinates

	// Groups with Radiobuttons:
	QActionGroup* mGroupSpherePanoAxis;       //! Radio-button group to set axis for an image stack of a sphereical panorama.
	QActionGroup* mGroupVisFuncCutOff;        //! Radio-button group within the pull-down menu visualization: choice of min/max treatment of the function value visualization.
	QActionGroup* mGroupSelPrimitive;         //! Radio-button group within the pull-down menu for selection of primitives.
	QActionGroup* mGroupSelHistType;          //! Radio-button group within the pull-down menu for selection of a mesh's histogram.
	QActionGroup* mGroupPerspOrtho;           //! Radio-button group with two buttons to switch between orthographic and perspective projection.

	// +++ Groupings of items to be enabled or disabled at the same time (signal).

	// --- MeshWidget related parameters -------------------------------------------------------------------------------------------------------------------
	QActionGroup* mMeshWidgetFlag;                   //! Group of visualization related flags of class MeshWidget -- MeshWidgetParams.
	QActionGroup* mMeshWidgetInteger;                //! Group of visualization related integer parameters of class MeshWidget -- MeshWidgetParams.
	QActionGroup* mMeshWidgetFloat;                  //! Group of visualization related floating point parameters of class MeshWidget -- MeshWidgetParams.

	// --- MeshGL & MeshQt related parameters --------------------------------------------------------------------------------------------------------------
	QActionGroup* mMeshGLFlag;                       //! Group of visualization related flags of class MeshGL.
	QActionGroup* mMeshGLParInt;                     //! Group of visualization related parameters, int of class MeshGL.
	QActionGroup* mMeshGLParDbl;                     //! Group of visualization related parameters, double of class MeshGL.
	QActionGroup* mMeshGLColors;                     //! Group of visualization related parameters, colors used in class MeshGL.
	// --- Mesh related parameters -------------------------------------------------------------------------------------------------------------------------
	QActionGroup* mMeshParDbl;                       //! Group of visualization related parameters, double of class Mesh.
	QActionGroup* mMeshFunctionCalls;                //! Group of menu items, where each is connected to method of class Mesh.

	// Group(s) with Radiobuttons:
	QActionGroup* mMeshGLGroupVertexSpriteColor;     //! Radio-button group to switch between color choices for rendering vertex sprites. Relates to mMeshGLParInt.
	QActionGroup* mMeshGLGroupSelColormap;           //! Radio-button group to switch between MeshGL colorramps. Relates to mMeshGLParInt.
	QActionGroup* mMeshGLGroupSelSpriteShape;        //! Radio-button group to switch between MeshGLShader sprite shapes see MeshGLParams::VERTEX_SPRITE_SHAPE.

	// ++++ Other

	// ---
	QActionGroup* menuContextToSelection;            //! Actions requiring ONLY a selection.

	// Recent files:
	QActionGroup* mRecentFiles;                      //! Group to open recent files.

	// Network access e.g. for checking the version number.
    QNetworkAccessManager* mNetworkManagerVersion;          //! manages simple http-request of the version number
    QNetworkAccessManager* mNetworkManagerAnalytics;          //! manages simple http-POST-request to Google Analytics

	// --- Main Toolbar Actions (Task 106: 20 Essential Tools) -----------------------------------------------------------------------------------------------------------------
	// Group 1: File (3)
	QAction* mActionNewProject;
	QAction* mActionOpenProject;
	QAction* mActionSaveProject;

	// Group 2: Edit (3)
	QAction* mActionUndo;
	QAction* mActionRedo;
	QAction* mActionSelectTool;

	// Group 3: Measurement (4)
	QAction* mActionMeasureDistance;
	QAction* mActionMeasureAngle;
	QAction* mActionMeasureArea;
	QAction* mActionCreateSection;

	// Group 4: View (4)
	QAction* mActionViewTop;
	QAction* mActionViewFront;
	QAction* mActionViewSide;
	QAction* mActionViewIsometric;

	// Group 5: Rendering (3)
	QAction* mActionRenderWireframe;
	QAction* mActionRenderSolid;
	QAction* mActionRenderNPR;

	// Group 6: Utilities (3)
	QAction* mActionScreenshot;
	QAction* mActionSettings;
	QAction* mActionHelp;

	// --- Status Bar Widgets (Task 107) -----------------------------------------------
	QLabel* mStatusCoord;            //!< 상태바: 좌표 표시 (X, Y, Z)
	QLabel* mStatusMesh;             //!< 상태바: 메시 정보 (정점 수, 면 수)
	QLabel* mStatusFPS;              //!< 상태바: FPS 표시
	QLabel* mStatusSelection;        //!< 상태바: 선택 개수 표시
	QProgressBar* mProgressBar;      //!< 상태바: 진행률 표시 (기본 숨김)

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif
