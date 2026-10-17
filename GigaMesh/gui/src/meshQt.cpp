//
// GigaMesh - The GigaMesh Software Framework is a modular software for display,
// editing and visualization of 3D-data typically acquired with structured light or
// structure from motion.
// Copyright (C) 2009-2020 Hubert Mara
//
// This file is part of GigaMesh.
//
// GigaMesh is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// GigaMesh is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
//

#include "meshQt.h"

// Qt includes:
#include "QGMDialogEnterText.h"
#include "QGMDialogCutOffValues.h"
#include "QGMDialogComboBox.h"
#include "QGMDialogNprSettings.h"
#include "QGMDialogTransparencySettings.h"
#include "qruntpsrpmscriptdialog.h"
#include "QGMDialogMatrix.h"

#include <QFileDialog>
#include <QInputDialog>

#include <automaticalignmentpyinterface.h>
// Other includes:
// none for now.

#define MESHQTINITDEFAULTS                        \
	mMainWindow( nullptr )

using namespace std;

//! Constructor for our most advanced Mesh class.
//! Loads a given file.
MeshQt::MeshQt( const QString&           rFileName,           //!< File to read
		bool&             rReadSuccess,             //!< pointer to a flag passing thru the result of readFile
		const float*      rMatModelView,       //!< Modelview Matrix from MeshWidget
		const float*      rMatProjection,      //!< Projection Matrix from MeshWidget
		MeshWidgetParams* rMeshWidgetParams,   //!< Parameter pointer to the MeshWidget settings.
		MeshGLColors*     rRenderColors,       //!< Colors i.e. second set of paramters of the MeshWidget.
		QGMMainWindow*    setmMainWindow,      //!< required to connect signals
		QGLContext*       rContext,            //!< OpenGL context for rendering
		QObject*          rParent              //!< Qt default
    ) : QObject( rParent ), MeshGLShader( rContext, rFileName.toStdWString(), rReadSuccess ),
                                     MeshQtCSVImportExport(this,
                                                            [this](){return &(this->mVertices);},
                                                            [this](){return &(this->mSelectedPositions);},
                                                            [this](const double value, const string& message){
	                                                                                    showProgress(value, message);},
                                                            [this](const string& message){showProgressStart(message);},
                                                            [this](const string& message){showProgressStop(message);}),
                                     MESHQTINITDEFAULTS {
	// Matrices:
	mMatModelView      = rMatModelView;
	mMatProjection     = rMatProjection;
	mWidgetParams      = rMeshWidgetParams;
	mRenderColors      = rRenderColors;
	// Generic
	mMainWindow        = setmMainWindow;

	// Setup signal connections TO the mMainWindow, which will send status messages and
	// have to be connected before any other activity like reading a file!
	//=====================================================================================================================
	QObject::connect( this, &MeshQt::statusMessage,   mMainWindow, &QGMMainWindow::setStatusBarMessage   );
	QObject::connect( this, &MeshQt::sFileChanged,    mMainWindow, &QGMMainWindow::fileChanged           );

	if( rFileName.size() <= 0 ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] No Filename given!" << endl;
		//! \bug Emitting inside constructor has no effect
		emit statusMessage( "Could not open file - no filename given!" );
		return;
	}


	if( !rReadSuccess ) {
		//! \bug Emitting inside constructor has no effect
		emit statusMessage( "Could not open file " + rFileName + "!" );
		return;
	}
	//! \bug Emitting inside constructor has no effect
	emit statusMessage( "Mesh loaded ... establishing data structure ..." );
	// Check if there is at least one vertex - otherwise something has happend before.
	if( getVertexNr() == 0 ) {
		rReadSuccess = false;
		cerr << "[MeshQt::" << __FUNCTION__ << "] Empty - No primitives loaded!" << endl;
		emit statusMessage( "File " + rFileName + " did not provide a valid mesh or point cloud!" );
		return;
	}
	//! \bug Emitting inside constructor has no effect
	emit statusMessage( "Mesh loaded and data structure established." );
	// set fileNameBase for display purposes:
	//! \bug Emitting inside constructor has no effect
    emit sFileChanged( QString::fromStdWString( getFullName().wstring() ), QString::fromStdWString( getBaseName().wstring() ) );

	QSettings settings;
    settings.setValue( "lastPath", QString::fromStdWString( getFileLocation().wstring() ) );

	// Setup signal connections TO the mMainWindow items
	//=====================================================================================================================
	QObject::connect( this, &MeshQt::showFlagState,   mMainWindow, &QGMMainWindow::updateMeshShowFlag      );
	QObject::connect( this, &MeshQt::paramIntSet,     mMainWindow, &QGMMainWindow::sShowParamIntMeshGL     );
	QObject::connect( this, &MeshQt::hasElement,      mMainWindow, &QGMMainWindow::updateMeshElementFlag   );

	// Setup signal connections FROM the mMainWindow to Mesh
	//=====================================================================================================================
	QObject::connect( mMainWindow, &QGMMainWindow::sShowParamFloatMeshDialog, this, &MeshQt::setParamFloatMeshDialog     );
	QObject::connect( mMainWindow, &QGMMainWindow::sShowParamFloatMeshLimits, this, &MeshQt::setParamFloatMeshLimits     );
	// Function calls:
	QObject::connect( mMainWindow, &QGMMainWindow::sCallFunctionMesh,         this, &MeshQt::callFunctionMesh            );

	// Setup signal connections FROM the mMainWindow to MeshGL
	//=====================================================================================================================
	QObject::connect( mMainWindow, &QGMMainWindow::sShowParamFlagMeshGL,        this, &MeshQt::setParamFlagMeshGL        );
	QObject::connect( mMainWindow, &QGMMainWindow::sShowParamIntMeshGL,         this, &MeshQt::setParamIntMeshGL         );
	QObject::connect( mMainWindow, &QGMMainWindow::sShowParamIntMeshGLDialog,   this, &MeshQt::setParamIntMeshGLDialog   );
	QObject::connect( mMainWindow, &QGMMainWindow::sShowParamFloatMeshGLDialog, this, &MeshQt::setParamFloatMeshGLDialog );
	QObject::connect( mMainWindow, &QGMMainWindow::sShowParamFloatMeshGLLimits, this, &MeshQt::setParamFloatMeshGLLimits );
	QObject::connect( mMainWindow, &QGMMainWindow::sShowParamColorMeshGL,       this, &MeshQt::selectColor               );
	// Function calls:
	QObject::connect( mMainWindow, &QGMMainWindow::sCallFunctionMeshGL,         this, &MeshQt::callFunctionMeshGL         );

	// SIDEBAR --- Progress --------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( this, &MeshQt::sShowProgressStart,   mMainWindow, &QGMMainWindow::sShowProgressStart   );
	QObject::connect( this, &MeshQt::sShowProgress,        mMainWindow, &QGMMainWindow::sShowProgress        );
	QObject::connect( this, &MeshQt::sShowProgressStop,    mMainWindow, &QGMMainWindow::sShowProgressStop    );


	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// ATTENTION: New Signal-Slot-Concept since Qt5 !!!
	// See: https://wiki.qt.io/New_Signal_Slot_Syntax
	// --------------------------------------------------------------------------------------------------------------------
	// Overloading Slots and Signals is now a bad idea!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//! \todo Source revision for the following connections: Adapt to Qt5 signal-slot concept!
	//! \todo Adapt to function calls using MeshParams / MeshGLParams as seen above!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// SIDEBAR --- Information -----------------------------------------------------------------------------------------------------------------------------
	QObject::connect( this, SIGNAL(sInfoMesh(MeshGLParams::eInfoMesh,QString)), mMainWindow, SIGNAL(sInfoMesh(MeshGLParams::eInfoMesh,QString)) );
	// SIDEBAR --- Guide -----------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( this, &MeshQt::sGuideIDSelection, mMainWindow, &QGMMainWindow::sGuideIDSelection );
	QObject::connect( this, &MeshQt::sGuideIDCommon,    mMainWindow, &QGMMainWindow::sGuideIDCommon    );

	// File menu -------------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, &QGMMainWindow::sFileImportFunctionValues, this, &MeshQt::importFunctionValues );
    QObject::connect( mMainWindow, &QGMMainWindow::sFileImportPolylines, this, &MeshQt::importPolylines );
    QObject::connect( mMainWindow, &QGMMainWindow::sFileImportTransMat, this, &MeshQt::importApplyTransMat );
    QObject::connect( mMainWindow, &QGMMainWindow::sFileImportLabels, this, &MeshQt::importLabels);
	// Old Qt Style connections:
	QObject::connect( mMainWindow, SIGNAL(sFileImportFeatureVectors(QString)), this, SLOT(importFeatureVectors(QString)) );
	QObject::connect( mMainWindow, SIGNAL(sExportFeatureVectors()), this, SLOT(exportFeatureVectors()) );
	QObject::connect( mMainWindow, SIGNAL(sFileImportTexMap(QString)),         this, SLOT(importTexMapFromFile(QString)) );
	QObject::connect( mMainWindow, SIGNAL(sFileImportNormals(QString)),        this, SLOT(importNormalVectorsFile(QString)) );
	//.
	QObject::connect( mMainWindow, SIGNAL(sFileSaveFlagBinary(bool)),          this, SLOT(setFileSaveFlagBinary(bool))      );
	QObject::connect( mMainWindow, SIGNAL(sFileSaveFlagExportTexture(bool)),this, SLOT(setFileSaveFlagExportTextures(bool)));
	//.
	QObject::connect( mMainWindow, SIGNAL(exportPolyLinesCoords()),          this, SLOT(exportPolyLinesCoords())          );
	QObject::connect( mMainWindow, SIGNAL(exportPolyLinesCoordsProjected()), this, SLOT(exportPolyLinesCoordsProjected()) );
	QObject::connect( mMainWindow, SIGNAL(exportPolyLinesFuncVals()),        this, SLOT(exportPolyLinesFuncVals())        );
	//.
	QObject::connect( mMainWindow, SIGNAL(exportFuncVals()),                 this, SLOT(exportFuncVals())                 );
	QObject::connect( mMainWindow, SIGNAL(exportFaceNormalAngles()),         this, SLOT(exportFaceNormalAngles())         );

	QObject::connect( mMainWindow, &QGMMainWindow::exportNormalSphereData, this , &MeshQt::exportNormalSphereData);
	// Edit menu ----------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(removeVerticesSelected()),     this, SLOT(removeVerticesSelected()) );
	QObject::connect( mMainWindow, SIGNAL(removeUncleanSmall()),         this, SLOT(removeUncleanSmallUser()) );
	//.
	QObject::connect( mMainWindow, SIGNAL(cutOffFeatureVertex()),        this, SLOT(cutOffFeatureVertex())    );
	//.
	QObject::connect( mMainWindow, SIGNAL(funcValSet()),                 this, SLOT(funcValSet())             );
	QObject::connect( mMainWindow, SIGNAL(funcValueCutOff()),            this, SLOT(funcValueCutOff())        );
	QObject::connect( mMainWindow, SIGNAL(funcValsNormalize()),          this, SLOT(funcValsNormalize())      );
	QObject::connect( mMainWindow, SIGNAL(funcValsAbs()),                this, SLOT(funcValsAbs())            );
	QObject::connect( mMainWindow, SIGNAL(funcValsAdd()),                this, SLOT(funcValsAdd())            );
	QObject::connect( mMainWindow, SIGNAL(sFuncValToFeatureVector()),    this, SLOT(funcValsToFeatureVector()));
	//.
	QObject::connect( mMainWindow, SIGNAL(setConeData()),                this, SLOT(setConeData()));
	QObject::connect( mMainWindow, SIGNAL(centerAroundCone()),           this, SLOT(centerAroundCone()));
	//.
	QObject::connect( mMainWindow, SIGNAL(sSplitByPlane()),              this, SLOT(splitByPlane()));
	QObject::connect( mMainWindow, SIGNAL(sSplitByIsoValue()),           this, SLOT(splitByIsoValue()));
	//.
	QObject::connect( mMainWindow, SIGNAL(centerAroundSphere()),         this, SLOT(centerAroundSphere()));
	QObject::connect( mMainWindow, SIGNAL(unrollAroundSphere()),         this, SLOT(unrollAroundSphere()));
	//.
	QObject::connect( mMainWindow, SIGNAL(sDatumAddSphere()),            this, SLOT(datumAddSphere())         );
	//.
    QObject::connect( mMainWindow, SIGNAL(sDownscaleTexture()),          this, SLOT(downscaleTexture())         );
    //.
	QObject::connect( mMainWindow, SIGNAL(sApplyMeltingSphere()),        this, SLOT(applyMeltingSphere())     );
	//.
    QObject::connect( mMainWindow, SIGNAL(sAutomaticMeshAlignment()),    this, SLOT(applyAutomaticMeshAlignment())     );

    //.


	// View menu -------------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(polylinesCurvScale()),                         this, SLOT(polylinesCurvScale())               );

	// De-Selection-- --------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(sDeSelVertsAll()),                             this, SLOT(deSelMVertsAll())                   );
	QObject::connect( mMainWindow, SIGNAL(sDeSelVertsNoLabel()),                         this, SLOT(deSelMVertsNoLabel())               );
	// Selection-- -----------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(getPlaneVPos()),                               this, SLOT(getPlaneVPos())                     );
	QObject::connect( mMainWindow, SIGNAL(getPlaneHNF()),                                this, SLOT(getPlaneHNF())                      );
	QObject::connect( mMainWindow, SIGNAL(setPlaneVPos()),                               this, SLOT(setPlaneVPos())                     );
	QObject::connect( mMainWindow, SIGNAL(setPlaneHNF()),                                this, SLOT(setPlaneHNF())                      );
	//.
	QObject::connect( mMainWindow, SIGNAL(selectVertFuncLT()),                           this, SLOT(selectVertFuncValLowerThan())       );
	QObject::connect( mMainWindow, SIGNAL(selectVertFuncGT()),                           this, SLOT(selectVertFuncValGreatThan())       );
    //.
    QObject::connect( mMainWindow, SIGNAL(selectVertNonMax()),                           this, SLOT(selectVertNonMaximum())             );
	//.
	QObject::connect( mMainWindow, SIGNAL(selectVertLocalMin()),                         this, SLOT(selectVertLocalMin())               );
	QObject::connect( mMainWindow, SIGNAL(selectVertLocalMax()),                         this, SLOT(selectVertLocalMax())               );
	//.
	QObject::connect( mMainWindow, SIGNAL(selectVertSolo()),                             this, SLOT(selectVertSolo())                   );
	QObject::connect( mMainWindow, SIGNAL(selectVertNonManifoldFaces()),                 this, SLOT(selectVertNonManifoldFaces())       );
	QObject::connect( mMainWindow, SIGNAL(selectVertDoubleCone()),                       this, SLOT(selectVertDoubleCone())             );
	QObject::connect( mMainWindow, SIGNAL(selectVertLabelAreaLT()),                      this, SLOT(selectVertLabelAreaLT())            );
	QObject::connect( mMainWindow, SIGNAL(selectVertLabelAreaRelativeLT()),              this, SLOT(selectVertLabelAreaRelativeLT())    );
	QObject::connect( mMainWindow, SIGNAL(selectVertBorder()),                           this, SLOT(selectVertBorder())                 );
	QObject::connect( mMainWindow, SIGNAL(selectVertFaceMinAngleLT()),                   this, SLOT(selectVertFaceMinAngleLT())         );
	QObject::connect( mMainWindow, SIGNAL(selectVertFaceMaxAngleGT()),                   this, SLOT(selectVertFaceMaxAngleGT())         );
	QObject::connect( mMainWindow, SIGNAL(sSelVertLabeledNot()),                         this, SLOT(selVertLabeledNot())                );
	//.
	QObject::connect( mMainWindow, SIGNAL(selectFaceNone()),                             this, SLOT(selectFaceNone())                   );
	QObject::connect( mMainWindow, SIGNAL(selectFaceSticky()),                           this, SLOT(selectFaceSticky())                 );
	QObject::connect( mMainWindow, SIGNAL(selectFaceNonManifold()),                      this, SLOT(selectFaceNonManifold())            );
	QObject::connect( mMainWindow, SIGNAL(selectFaceZeroArea()),                         this, SLOT(selectFaceZeroArea())               );
	QObject::connect( mMainWindow, SIGNAL(selectFaceInSphere()),                         this, SLOT(selectFaceInSphere())               );
	QObject::connect( mMainWindow, SIGNAL(selectFaceRandom()),                           this, SLOT(selectFaceRandom())                 );
	//.
	QObject::connect( mMainWindow, SIGNAL(selectPolyNoLabel()),                          this, SLOT(selectPolyNoLabel())                );
	QObject::connect( mMainWindow, SIGNAL(selectPolyNotLabeled()),                       this, SLOT(selectPolyNotLabeled())             );
	QObject::connect( mMainWindow, SIGNAL(selectPolyRunLenGT()),                         this, SLOT(selectPolyRunLenGT())               );
	QObject::connect( mMainWindow, SIGNAL(selectPolyRunLenLT()),                         this, SLOT(selectPolyRunLenLT())               );
	QObject::connect( mMainWindow, SIGNAL(selectPolyLongest()),                          this, SLOT(selectPolyLongest())                );
	QObject::connect( mMainWindow, SIGNAL(selectPolyShortest()),                         this, SLOT(selectPolyShortest())               );
	QObject::connect( mMainWindow, SIGNAL(selectPolyLabelNo()),                          this, SLOT(selectPolyLabelNo())                );

	// Analyze menu ----------------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(labelFaces()),                                 this, SLOT(labelFaces())                        );
	QObject::connect( mMainWindow, SIGNAL(labelSelectionToSeeds()),                      this, SLOT(labelSelectionToSeeds())             );
	QObject::connect( mMainWindow, SIGNAL(labelVerticesEqualFV()),                       this, SLOT(labelVerticesEqualFV())              );
    QObject::connect( mMainWindow, SIGNAL(labelVerticesEqualRGB()),                      this, SLOT(labelVerticesEqualRGB())             );
	QObject::connect( mMainWindow, SIGNAL(sLabelSelMVertsToBack()),                      this, SLOT(labelSelMVertsToBack())              );
	//.
	QObject::connect( mMainWindow, SIGNAL(convertSelectedVerticesToPolyline()),          this, SLOT(convertSelectedVerticesToPolyline()) );
	QObject::connect( mMainWindow, SIGNAL(convertBordersToPolylines()),                  this, SLOT(convertBordersToPolylines())         );
	QObject::connect( mMainWindow, SIGNAL(convertLabelBordersToPolylines()),             this, SLOT(convertLabelBordersToPolylines())    );
    QObject::connect( mMainWindow, SIGNAL(createSkeletonLine()),                         this, SLOT(createSkeletonLine()) );
	QObject::connect( mMainWindow, SIGNAL(advancePolyThres()),                           this, SLOT(advancePolyThres())                  );
	QObject::connect( mMainWindow, SIGNAL(sPolylinesCompIntInvRunLen()),                 this, SLOT(compPolylinesIntInvRunLen())         );
	QObject::connect( mMainWindow, SIGNAL(sPolylinesCompIntInvAngle()),                  this, SLOT(compPolylinesIntInvAngle())          );
	QObject::connect( mMainWindow, SIGNAL(sPolylinesCompCurv()),                         this, SLOT(getPolylineExtrema())                );
	QObject::connect( mMainWindow, SIGNAL(setLengthSmooth()),                            this, SLOT(setParaSmoothLength())               );
	QObject::connect( mMainWindow, SIGNAL(sPolylinesCopyNormalToVertices()),             this, SLOT(setPolylinesNormalToVert())          );
	//.
	QObject::connect( mMainWindow, SIGNAL(estimateMSIIFeat()),                           this, SLOT(estimateMSIIFeat())                  );
	//.
	QObject::connect( mMainWindow, SIGNAL(sGeodPatchVertSel()),                          this, SLOT(geodPatchVertSel())                  );
	QObject::connect( mMainWindow, SIGNAL(sGeodPatchVertSelOrder()),                     this, SLOT(geodPatchVertSelOrder())             );
	//.
	QObject::connect( mMainWindow, SIGNAL(fillHoles()),                                  this, SLOT(fillPolyLines())                     );
	//.
	QObject::connect( mMainWindow, SIGNAL(sPolylinesLength()),                           this, SLOT(infoPolylinesLength())               );
	//.
	QObject::connect( mMainWindow, SIGNAL(hueToFuncVal()),                               this, SLOT(hueToFuncVal())                      );
	QObject::connect( mMainWindow, SIGNAL(intersectSphere()),                            this, SLOT(intersectSphere())                   );
	//.
	QObject::connect( mMainWindow, SIGNAL(estimateVolume()),                             this, SLOT(estimateVolume())                    );
	QObject::connect( mMainWindow, SIGNAL(compVolumePlane()),                            this, SLOT(compVolumePlane())                   );

	// --- Octree reöated ----------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(generateOctree()),                     this, SLOT(generateOctree())                    );
	QObject::connect( mMainWindow, SIGNAL(detectselfintersections()),            this, SLOT(detectselfintersections())           );
	QObject::connect( mMainWindow, SIGNAL(drawOctree()),                         this, SLOT(drawOctree())                        );
	QObject::connect( mMainWindow, SIGNAL(removeOctreedraw()),                   this, SLOT(removeOctreedraw())                  );
	QObject::connect( mMainWindow, SIGNAL(deleteOctree()),                       this, SLOT(deleteOctree())                      );

	// #####################################################################################################################################################
	// # FUNCTION VALUE
	// #####################################################################################################################################################
	// # Feature Vector reöated
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatLengthEuc()),              this, SLOT(visualizeFeatLengthEuc())               );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatLengthMan()),              this, SLOT(visualizeFeatLengthMan())               );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatBVFunc()),                 this, SLOT(visualizeFeatBVFunc())                  );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatTVSeqn()),                 this, SLOT(visualizeFeatTVSeqn())                  );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatDistSelVertEuc()),         this, SLOT(visualizeFeatDistSelEuc())              );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatDistSelVertEucNorm()),     this, SLOT(visualizeFeatDistSelEucNorm())          );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatDistSelVertMan()),         this, SLOT(visualizeFeatDistSelMan())              );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatDistSelVertCosSim()),      this, SLOT(visualizeFeatureCosineSimToVertex())    );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatDistSelVertTanimoto()),    this, SLOT(visualizeFeatureTanimotoDistTo())       );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatCorrSelVert()),            this, SLOT(visualizeFeatCorrSelectedVert())        );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatAutoCorrVert()),           this, SLOT(visualizeFeatAutoCorrVert())            );
	QObject::connect( mMainWindow, SIGNAL(sFuncVertFeatAutoCorrSelVert()),        this, SLOT(visualizeFeatAutoCorrSelectedVert())    );
	// # Distance to plane, line, selected primitive, cone and sphere
	QObject::connect( mMainWindow, SIGNAL(visualizeDistanceToPlane()),            this, SLOT(visualizeDistanceToPlane())             );
	QObject::connect( mMainWindow, SIGNAL(visualizeDistanceToCone()),             this, SLOT(visualizeDistanceToCone())              );
	// # Other
	QObject::connect( mMainWindow, SIGNAL(visualizeVertexIndices()),              this, SLOT(visualizeVertexIndices())               );
	QObject::connect( mMainWindow, SIGNAL(sFuncVert1RingRMin()),                  this, SLOT(funcVert1RingRMin())                    );
	QObject::connect( mMainWindow, SIGNAL(sFuncVert1RingVolInt()),                this, SLOT(funcVert1RingVolInt())                  );
	QObject::connect( mMainWindow, SIGNAL(visualizeVertexOctree()),               this, SLOT(visualizeVertexOctree())                );
	QObject::connect( mMainWindow, SIGNAL(visualizeVertexFaceSphereAngleMax()),   this, SLOT(visualizeVertexFaceSphereAngleMax())    );
	QObject::connect( mMainWindow, SIGNAL(visualizeVertFaceSphereMeanAngleMax()), this, SLOT(visualizeVertFaceSphereMeanAngleMax())  );
	// #####################################################################################################################################################

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Dialogs:
	QObject::connect( &mDialogConeParam, &QGMDialogConeParam::coneParameters, this, &MeshQt::setConeParameters );

	QObject::connect( mMainWindow, SIGNAL(sShowNPRSettings()), this, SLOT(showNPRSettings()));
	QObject::connect( mMainWindow, SIGNAL(sShowTransparencySettings()), this, SLOT(showTransparencySettings()));

	// Treat MeshGLParams (Flag/Boolean and Intereger)
	for( int i=0; i<MeshGLParams::PARAMS_FLAG_COUNT; i++ ) {
		bool currFlag;
		getParamFlagMeshGL( static_cast<MeshGLParams::eParamFlag>(i), &currFlag );
		//! \bug Emitting inside constructor has no effect
		emit showFlagState( static_cast<MeshGLParams::eParamFlag>(i), currFlag );
	}
	for( int i=0; i<MeshGLParams::PARAMS_INT_COUNT; i++ ) {
		int currParamInt;
		getParamIntMeshGL( static_cast<MeshGLParams::eParamInt>(i), &currParamInt );
		//! \bug Emitting inside constructor has no effect
		emit paramIntSet( static_cast<MeshGLParams::eParamInt>(i), currParamInt );
	}
	if( mPolyLines.size() > 0 ) {
		//! \bug Emitting inside constructor has no effect
		emit hasElement( Primitive::IS_POLYLINE, true );
		//! \bug Emitting inside constructor has no effect
		emit showFlagState( MeshGL::SHOW_POLYLINES, true );
	} else {
	//! \bug Emitting inside constructor has no effect
		emit hasElement( Primitive::IS_POLYLINE, false );
	//! \bug Emitting inside constructor has no effect
		emit showFlagState( MeshGL::SHOW_POLYLINES, false );
	}
	polyLinesChanged();
	// Erase any previous selection:
	//! \bug Emitting inside constructor has no effect
	emit primitiveSelected( nullptr );

	// Update selection
	callFunction( MeshParams::REFRESH_SELECTION_DISPLAY );

	cout << "[MesQt::" << __FUNCTION__ << "] Done." << endl;
}

//! Deconstructor for our most advanced Mesh class - disconnect signals.
MeshQt::~MeshQt() {
	QObject::disconnect( this );
}

// overloaded from MeshIO --------------------------------------------------------------------------------------------------------------------------------------

//! Ask the user if the file to be read contains a regular grid - see MeshIO::readFile()
bool MeshQt::readIsRegularGrid( bool* rIsGrid ) {
	bool userCancel;
	SHOW_QUESTION( tr("Read XYZ file"), tr("Is the content of the file a regular grid (e.g. DTM exported by GIS)?"), (*rIsGrid), userCancel );
	if( userCancel ) {
		return false;
	}
	return true;
}

// overloaded from MeshGL / Mesh -------------------------------------------------------------------------------------------------------------------------------

//! Emits signal to tell the other widgets (menus!) that the mesh has or has no polylines.
void MeshQt::polyLinesChanged() {
	MeshGL::polyLinesChanged();
	auto selMPolysNr = mPolyLines.size();
	if( selMPolysNr > 0 ) {
		emit hasElement( Primitive::IS_POLYLINE, true );
	} else {
		emit hasElement( Primitive::IS_POLYLINE, false );
	}
}

//! Handles UI, when the function values of the faces were changed.
//! Switches to function value visualization.
void MeshQt::changedFaceFuncVal() {
	MeshGL::changedFaceFuncVal();
}

//! Handles UI, when the function values of the vertices were changed.
//! Switches to function value visualization.
void MeshQt::changedVertFuncVal() {
	MeshGL::changedVertFuncVal();
	setParamIntMeshGL( TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
}

//! Open progress bar window.
void MeshQt::showProgressStart( const string& rMsg ) {
	emit sShowProgressStart( QString::fromStdString( rMsg ) );
	MeshGL::showProgressStart( rMsg );
}

//! Update progress bar window.
//! See also MeshGL::showProgress and Mesh::showProgress
//!
//! @returns false in case of an error. True otherwise.
//! Will return true even, when the signal firing is suppressed.
bool MeshQt::showProgress(
                double rVal,   //!< Percentage given as range 0.0 to 1.0
                const string& rMsg    //!< Name or short description of the function
) {
	if( MeshGL::showProgress( rVal, rMsg ) ) {
		// Fire signal.
		emit sShowProgress( rVal );
	}
	return true;
}

//! Close progress bar window.
void MeshQt::showProgressStop( const string& rMsg ) {
	MeshGL::showProgressStop( rMsg );
	emit sShowProgressStop( QString::fromStdString( rMsg ) );
}

//! Show requested information.
void MeshQt::showInformation(
                const std::string& rHead,
                const std::string& rMsg,
                const std::string& rToClipboard
) {
	// Write string to clipboard, if not empty
	std::string notifyAboutClipboard = "";
	if( rToClipboard.length() > 0 ) {
		QClipboard *clipboard = QApplication::clipboard();
		clipboard->setText( rToClipboard.c_str() );
		notifyAboutClipboard = (QString("<br /><br />") + tr("Already copied to clipboard!")).toStdString();
	}
	// Show Messagebox
	SHOW_MSGBOX_INFO( rHead.c_str(), ( rMsg + notifyAboutClipboard ).c_str() );
	// Pass to CLI
	MeshGL::showInformation( rHead, rMsg, rToClipboard );
}

//! Show warning with a timeout.
//! The timeout is nessary, because it may block some of the automated methods e.g. "fill holes".
void MeshQt::showWarning( const string& rHead, const string& rMsg ) {
	SHOW_MSGBOX_WARN_TIMEOUT( rHead.c_str(), rMsg.c_str(), 5000 );
	MeshGL::showWarning( rHead, rMsg );
}

//! Let the user enter a string.
//! @returns false in case of a bad value or user cancel.
bool MeshQt::showEnterText( std::string& rSomeStrg, const char* rTitle ) {
	QGMDialogEnterText dlgEnterTxt;
	if( rTitle == nullptr ) {
		dlgEnterTxt.setWindowTitle( tr("Enter text:") );
	} else {
		dlgEnterTxt.setWindowTitle( rTitle );
	}

	// Check if a string was given. If not fetch the clipboard.
	if( rSomeStrg.size() == 0 ) {
		dlgEnterTxt.fetchClipboard( QGMDialogEnterText::CHECK_NONE );
	} else {
		dlgEnterTxt.setText( rSomeStrg );
	}

	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return( false );
	}
	if( !dlgEnterTxt.getText( rSomeStrg ) ) {
		return( false );
	}
	return( true );
}

//! Let the user enter one uint64_t integer value.
//! @returns false in case of a bad value or user cancel.
bool MeshQt::showEnterText(
                uint64_t& rULongInt,
                const char* rTitle          //!< Title to be shown for the dialog.
) {
	QGMDialogEnterText dlgEnterTxt;
	if( rTitle == nullptr ) {
		dlgEnterTxt.setWindowTitle( tr("Enter unsigned integer value:") );
	} else {
		dlgEnterTxt.setWindowTitle( rTitle );
	}

	//! \todo this check should be made available for all methods and most likely moved to QGMDialogEnterText
	QClipboard *clipboard = QApplication::clipboard();
	QString clipBoardStr = clipboard->text( QClipboard::Clipboard );
	bool isULong;
	clipBoardStr.toULong( &isULong );
	if( isULong ) {
		dlgEnterTxt.fetchClipboard( QGMDialogEnterText::CHECK_INTEGER_UNSIGNED );
	} else {
		dlgEnterTxt.setText( QString::number( rULongInt ) );
	}

	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return false;
	}
	if( !dlgEnterTxt.getText( &rULongInt ) ) {
		return false;
	}
	return true;
}

//! Let the user enter one double value.
//! @returns false in case of a bad value or user cancel.
bool MeshQt::showEnterText(
                double& rDoubleVal,
                const char* rTitle          //!< Title to be shown for the dialog.
) {
	QGMDialogEnterText dlgEnterTxt;
	if( rTitle == nullptr ) {
		dlgEnterTxt.setWindowTitle( tr("Enter one double precision value:") );
	} else {
		dlgEnterTxt.setWindowTitle( rTitle );
	}
	dlgEnterTxt.setDouble( rDoubleVal );
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return false;
	}
	if( !dlgEnterTxt.getText( &rDoubleVal ) ) {
		return false;
	}
	return true;
}

//! Let the user enter integer values.
//! @returns false in case of a bad value or user cancel.
bool MeshQt::showEnterText(
		std::set<int64_t> &rIntegers,   //!< Pointer to a set for passing values.
		const char* rTitle              //!< Title to be shown for the dialog.
) {
	QGMDialogEnterText dlgEnterTxt;
	if( rTitle == nullptr ) {
		dlgEnterTxt.setWindowTitle( tr("Enter integer values:") );
	} else {
		dlgEnterTxt.setWindowTitle( rTitle );
	}
	if( rIntegers.size() > 0 ) {
		dlgEnterTxt.setText( rIntegers );
	} else {
		dlgEnterTxt.fetchClipboard( QGMDialogEnterText::CHECK_INTEGER_MULTIPLE );
	}
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return( false );
	}
	if( !dlgEnterTxt.getText( rIntegers ) ) {
		return( false );
	}
	return( true );
}

//! Let the user enter integer values.
//! @returns false in case of a bad value or user cancel.
bool MeshQt::showEnterText(
                vector<long>& rIntegers,       //!< Pointer to a set for passing values.
                const char*  rTitle            //!< Title to be shown for the dialog.
) {
	QGMDialogEnterText dlgEnterTxt;
	if( rTitle == nullptr ) {
		dlgEnterTxt.setWindowTitle( tr("Enter integer values:") );
	} else {
		dlgEnterTxt.setWindowTitle( rTitle );
	}
	dlgEnterTxt.fetchClipboard( QGMDialogEnterText::CHECK_INTEGER_MULTIPLE );
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return false;
	}
	if( !dlgEnterTxt.getText( rIntegers ) ) {
		return false;
	}
	return true;
}

//! Let the user enter double precision values.
bool MeshQt::showEnterText(
	vector<double>& rDoubles,   //!< Pointer to a vector for passing values.
	const char* rTitle          //!< Title to be shown for the dialog.
) {
	QGMDialogEnterText dlgEnterTxt;
	if( rTitle == nullptr ) {
		dlgEnterTxt.setWindowTitle( tr("Enter double precision values:" ));
	} else {
		dlgEnterTxt.setWindowTitle( rTitle );
	}
	if( rDoubles.size() > 0 ) {
		dlgEnterTxt.setText( rDoubles );
	} else {
		dlgEnterTxt.fetchClipboard( QGMDialogEnterText::CHECK_DOUBLE_MULTIPLE );
	}
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return false;
	}
	if( !dlgEnterTxt.getText( rDoubles ) ) {
		return false;
	}
	return true;
}

//! Let the user enter a 4x4 matrix i.e. 16 floating point values.
bool MeshQt::showEnterText(Matrix4D* rMatrix4x4 , bool selectedVerticesOnly) {

	QGMDialogMatrix dlgEnterMatrix;
	dlgEnterMatrix.setWindowTitle( tr("Enter 4x4 Matrix"));
	dlgEnterMatrix.fetchClipboard();

	const auto cog = getCenterOfGravity();
	const auto bboxCenter = getBoundingBoxCenter();

	dlgEnterMatrix.setMeshCog(cog);
	dlgEnterMatrix.setMeshBBoxCenter(bboxCenter);

	connect(&dlgEnterMatrix, &QGMDialogMatrix::applyClicked, [this, &dlgEnterMatrix, selectedVerticesOnly](){
		vector<double> values;
		dlgEnterMatrix.getValues(values);
		Matrix4D matrix(values);
		if(selectedVerticesOnly)
		{
			removeAllDatumObjects();
			applyTransformation(matrix, &mSelectedMVerts);
		}
		else
		{
			applyTransformationToWholeMesh(matrix);
		}
	});

	if(dlgEnterMatrix.exec() == QDialog::Rejected )
	{
		return false;
	}

	vector<double> returnValues;

	dlgEnterMatrix.getValues(returnValues);
	rMatrix4x4->set( returnValues );
	return true;
}

//! Let the user choose a floating point value using a slider.
//! @returns true, when a valid value was choosen and confirmed by the user. False otherwise.
bool MeshQt::showSlider( double *rValueToChange, double rMin, double rMax, const char* rTitle ) {
	QGMDialogSliderHD dlgSlider;
	dlgSlider.setMin( rMin );
	dlgSlider.setMax( rMax );
	dlgSlider.setPos( (*rValueToChange) );
	dlgSlider.setWindowTitle( rTitle );
	dlgSlider.suppressPreview();

	//! \todo enable preview and connect with MeshParams
	// Overloading slots is not a good idea in combination with the new Qt5 signal-slot concpet.
	// The result can be seen below as ugly casts.
	// This is a broken example:
	// QObject::connect( &dlgSlider, static_cast<void (QGMDialogSliderHD::*)(int,double)>(&QGMDialogSliderHD::valuePreview),
	//                   this, static_cast<void (MeshQt::*)(MeshParams::eParamFlt,double)>(&MeshQt::setParamFloatMesh) );

	if( dlgSlider.exec() == QDialog::Rejected ) {
		return false;
	}
	dlgSlider.getValue( rValueToChange );
	return true;
}

//! Let the user choose a boolean value.
//! @returns true, when a valid value was choosen and confirmed by the user. False otherwise i.e. user cancel or error.
bool MeshQt::showQuestion( bool* rUserChoice, const string& rHead, const string& rMsg ) {
	bool userCancel = false;
	bool userChoice = false;

	if(rUserChoice)
	{
		userChoice = (*rUserChoice);
	}

	SHOW_QUESTION( QString( rHead.c_str() ), QString( rMsg.c_str() ), userChoice, userCancel );
	if( userCancel ) {
		return false;
	}
	(*rUserChoice) = userChoice;
	return true;
}

// SLOTS -----------------------------------------------------------------------------------

bool MeshQt::exportPolyLinesCoords() {
	//! Handle GUI request to export polylines as ASCII file.
	//! 1. Ask for filename, etc.

    QString filePath = QString::fromStdWString( getFileLocation().wstring() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Export polygonal lines as vertex list" ), \
													 filePath,    tr( "Polygonal lines ASCII (*.pline)" ) );
	if( fileName.length() == 0 ) {
		return false;
	}

   //27.04. E. Stoetzner deconstruct questions

    //bool userCancel;
	// Ask for vertex normals
    bool withNormals = true;
//	SHOW_QUESTION( tr("Export vertex normals"), tr("Export polylines with vertex normals"), withNormals, userCancel );
//	if( userCancel ) {
//		return false;
//	}

	// Ask for vertex indices
    bool withVertIdx = true;
//	SHOW_QUESTION( tr("Export vertex indices"), tr("Export polylines with indices of the vertices"), withVertIdx, userCancel );
//	if( userCancel ) {
//		return false;
//	}

	if( !MeshGL::exportPolyLinesCoords( fileName.toStdString(), withNormals, withVertIdx ) ) {
		SHOW_MSGBOX_CRIT( tr("Export polylines"), tr("Failed") );
		emit statusMessage( "ERROR: Polylines NOT exported to: " + fileName );
		return false;
	}
	//setPathLast( fileName );
	emit statusMessage( "Polylines exported to: " + fileName );
	return true;
}

//! Handle GUI request to export polylines as ASCII file projected to the mesh plane.
bool MeshQt::exportPolyLinesCoordsProjected() {
    QString filePath = QString::fromStdWString( getFileLocation().wstring() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Export polygonal lines as vertex list" ),
                                                     filePath + QString::fromStdWString(getBaseName().wstring()) +
	                                                 ".pline", tr( "Polygonal lines ASCII (*.pline)" ) );
	if( fileName.length() == 0 ) {
		return false;
	}

	bool withVertIdx;
	bool userCancel;
	SHOW_QUESTION( tr("Export vertex indices"), tr("Export polylines with indices of the vertices"), withVertIdx, userCancel );
	if( userCancel ) {
		return false;
	}

	if( !MeshGL::exportPolyLinesCoordsProjected( fileName.toStdString(), withVertIdx ) ) {
		SHOW_MSGBOX_CRIT( tr("Export polylines"), tr("Failed") );
		emit statusMessage( "ERROR: Polylines NOT exported to: " + fileName );
		return false;
	}
	emit statusMessage( "Polylines exported to: " + fileName );
	return true;
}

//! Handle GUI request to export the run-length and the function values of (selected) polylines.
bool MeshQt::exportPolyLinesFuncVals() {
    QString filePath = QString::fromStdWString( getFileLocation().wstring() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Export polygonal lines as vertex list" ), \
	                                                 filePath +
                                                     QString::fromStdWString(getBaseName().wstring()) + ".txt", tr( "Run-length and function values ASCII (*.txt)" ) );
	if( fileName.length() == 0 ) {
		return false;
	}
	if( !MeshGL::exportPolyLinesFuncVals( fileName.toStdString() ) ) {
		SHOW_MSGBOX_CRIT( tr("Export polylines"), tr("Failed") );
		emit statusMessage( "ERROR: Polylines NOT exported to: " + fileName );
		return false;
	}
	emit statusMessage( "Polylines exported to: " + fileName );
	return true;
}

//! Handle GUI request to export the function values of the vertices as ASCII file.
//! @returns false in case of an error. True otherwise.
bool MeshQt::exportFuncVals() {
    QString filePath = QString::fromStdWString( getFileLocation().wstring() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Export function values" ),
	                                                 filePath + "/" +
                                                     QString::fromStdWString(getBaseName().wstring()) + "_funcvals.txt",
													 tr( "ASCII text (*.txt)" ) );
	if( fileName.length() == 0 ) {
		return false;
	}

	bool withVertIdx;
	bool userCancel;
	SHOW_QUESTION( tr("Export function values"), tr("Export function values with indices of the vertices"), withVertIdx, userCancel );
	if( userCancel ) {
		return false;
	}

	if( !Mesh::exportFuncVals( fileName.toStdString(), withVertIdx ) ) {
		SHOW_MSGBOX_CRIT( tr("Export function values"), tr("Failed") );
		emit statusMessage( "ERROR: Function values NOT exported to: " + fileName );
		return false;
	}
	emit statusMessage( "Function values exported to: " + fileName );
	return true;
}

bool MeshQt::exportFaceNormalAngles() {
	//! Handle GUI request to export normals in spherical coordinates.
	//! see MeshQt::exportFaceNormalAngles and MeshGL::exportFaceNormalAngles
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Export Face Normals, Spherical" ), nullptr, tr( "Face normals, spherical ASCII (*.facen)" ) );
	if( fileName.length() > 0 ) {
		return exportFaceNormalAngles( fileName.toStdString() );
	}
	return false;
}

bool MeshQt::exportFaceNormalAngles( filesystem::path filename ) {
	//! Handle GUI request to export normals in spherical coordinates.
	//! see MeshGL::exportFaceNormalAngles
	return MeshGL::exportFaceNormalAngles( filename );
}

void MeshQt::exportNormalSphereData()
{
    QString filePath = QString::fromStdWString( getFileLocation().wstring() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Export normal data mapped on IcoSPhere" ), \
													 filePath,    tr( "CSV file (*.csv)" ) );
	if( fileName.length() == 0 ) {
		return;
	}

	bool userCancel;
	// Ask for ico-sphere subdivision level
	QString subdivision = "6";

	SHOW_INPUT_DIALOG( tr("Subdivisions"), tr("Subdivision level of the icosphere"), subdivision, subdivision, userCancel);

	if( userCancel ) {
		return;
	}

	bool exportFaceNormals = false;
	SHOW_QUESTION( tr("Normal selection"), tr("Export face normals (yes) or vertex normals (no)"), exportFaceNormals, userCancel);
	if(userCancel) {
		return;
	}

	// Ask for vertex normals in sphere coordinates
	bool sphereCoordinates;
	SHOW_QUESTION( tr("Normals representation"), tr("Export normals in sphere-coordinates?"), sphereCoordinates, userCancel );
	if( userCancel ) {
		return;
	}

	std::list<sVertexProperties> vertexProps;

	auto fAddNormal = [&vertexProps](const Vector3D& normal) -> void {
		if( std::isnan(normal.getX()) || std::isnan(normal.getY()) || std::isnan(normal.getZ()) )
		{
			return;
		}
		sVertexProperties prop;
		prop.mNormalX = normal.getX();
		prop.mNormalY = normal.getY();
		prop.mNormalZ = normal.getZ();

		vertexProps.emplace_back(prop);
	};

	if(exportFaceNormals)
	{
		auto faceCount = getFaceNr();

		for( uint64_t faceIdx = 0; faceIdx < faceCount; ++faceIdx)
		{
			Face* currFace = getFacePos( faceIdx );
			auto normal = currFace->getNormal(false);
			fAddNormal(normal);
		}
	}
	else
	{
		auto vertCount = getVertexNr();

		for( uint64_t vertIdx=0; vertIdx<vertCount; vertIdx++ ) {
			Vertex* currVertex = getVertexPos( vertIdx );
			auto normal = currVertex->getNormal(false);
			fAddNormal(normal);
		}
	}

	MeshIO::writeIcoNormalSphereData(fileName.toStdString(), vertexProps , subdivision.toInt(), sphereCoordinates);
}

// --- Edit actions ---------------------------------------------------------------------------

//! Method called after changes to the mesh. Takes care about e.g. reseting the OpenGL buffers.
//! @returns false in case of an error. True otherwise.
bool MeshQt::changedMesh() {
	bool retVal = true;
	// Reset OpenGL:
	this->glRemove();
	this->glPrepare();
	retVal &= MeshGL::changedMesh();
	// Update OpenGL
	emit updateGL();
	// Done
	return( retVal );
}

//! Removes all vertices (and their related faces) stored in verticesSelected - see MeshGL::removeVerticesSelected
//! Returns false in case of an error or when no vertices were removed.
bool MeshQt::removeVerticesSelected() {
	// Store old mesh size to determine the number of changes
	int  oldVertexNr = getVertexNr();
	int  oldFaceNr   = getFaceNr();
	bool retVal = MeshGL::removeVerticesSelected();
	SHOW_MSGBOX_INFO( tr("Selected primitives removed"), tr( "%1 Vertices\n%2 Faces" ).arg( oldVertexNr - getVertexNr() ).arg( oldFaceNr - getFaceNr() ) );
	return retVal;
}

//! Select small, solo, non-manifold and double-cones and removes them.
//!
//! Set parameters by user interaction.
//!
//! @returns false in case of an error. True otherwise.
bool MeshQt::removeUncleanSmallUser() {

	// Set small areas to be removed.
	QGMDialogSliderHD dlgSlider;
	dlgSlider.setMin(   0.0 + DBL_EPSILON );
	dlgSlider.setMax( 100.0 - DBL_EPSILON );
	dlgSlider.setPos( 10.0 );
	dlgSlider.suppressPreview();
	dlgSlider.setWindowTitle(  tr("Maximum label area (relative)") );
	if( dlgSlider.exec() == QDialog::Rejected ) {
		return false;
	}
	double percentArea;
	if( !dlgSlider.getValue( &percentArea ) ) {
		return false;
	}
	// As the slider dialog will return a value of ] 0.0 ... 100.0 [ we divide by 100.
	percentArea /= 100.0;

    /**
	// Optional border erosion
	bool applyErosion;
	bool userCancel;
	SHOW_QUESTION( tr("Apply border erosion"), tr("Do you want to remove dangling faces along the borders?") +
	               QString("<br /><br />") + tr("Recommended: YES"), applyErosion, userCancel );
	if( userCancel ) {
		return( false );
	}
    **/
    //get parameters from Settings
    bool applyErosion;
    getParamFlagMeshGL(MeshGLParams::REMOVE_DANGLING_FACES, &applyErosion);
	// Ask if we wan't to store the result, when finished.
	bool saveFile;
    bool userCancel;
	SHOW_QUESTION( tr("Store results"), tr("Do you want to store the result as file?"), saveFile, userCancel );
	if( userCancel ) {
		return( false );
	}
	QString fileName = "";
	if( saveFile ) {
		// Show file dialog
        QString fileSuggest = QString::fromStdWString( getBaseName().wstring() );
        //check if the gigamesh name convention is used
        QRegularExpression nameContainsGMwithSuffix( ".*_GM[cCoOfFpP]*$" );
        QRegularExpressionMatch match = nameContainsGMwithSuffix.match( fileSuggest );
        if( match.hasMatch()){
            //GM or GMO not followed by "C" --> no match
            QRegularExpression nameContainsC( "(.*_GM*.C)(.*$)" );
            match = nameContainsC.match( fileSuggest );
            if( !match.hasMatch() ) {
                    //regex to add the C after GM or GMO
                    QRegularExpression nameContainsGM( "(.*_GM[oO]?)(.*$)" );
                    fileSuggest.replace( nameContainsGM, "\\1C\\2" );
                }
          } else {
                fileSuggest += "_GMC";
         }
        fileSuggest += ".ply";

        QString fileLocation = QString::fromStdWString( getFileLocation().wstring() );
        fileLocation += fileSuggest;
        fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Save as" ), fileLocation, tr( "3D-Files (*.obj *.ply *.wrl *.txt *.xyz)" ) );
	}

	// Store old mesh size to determine the number of changes
	uint64_t oldVertexNr = getVertexNr();
	uint64_t oldFaceNr   = getFaceNr();
	bool retVal = removeUncleanSmall( fileName.toStdString(), percentArea, applyErosion );
	SHOW_MSGBOX_INFO( tr("Primitives removed"), tr( "%1 Vertices\n%2 Faces" ).arg( oldVertexNr - getVertexNr() ).arg( oldFaceNr - getFaceNr() ) );
	return retVal;
}


//! Automatic mesh polishing.
//!
//! Iterativly applies Mesh::removeUncleanSmall and Mesh::fillPolyLines
//! 
//! This method loops till mesh is fully restored 
//! tracking the changes of the number of vertices and faces.
//!
//! @returns false in case of an error. True otherwise.
bool MeshQt::completeRestore() {

	// Set small areas to be removed.
	QGMDialogSliderHD dlgSlider;
	dlgSlider.setMin(   0.0 + DBL_EPSILON );
	dlgSlider.setMax( 100.0 - DBL_EPSILON );
	dlgSlider.setPos( 10.0 );
	dlgSlider.suppressPreview();
	dlgSlider.setWindowTitle(  tr("Maximum label area (relative)") );
	if( dlgSlider.exec() == QDialog::Rejected ) {
		return( false );
	}
	double percentArea;
	if( !dlgSlider.getValue( &percentArea ) ) {
		return( false );
	}
	// As the slider dialog will return a value of ] 0.0 ... 100.0 [ we divide by 100.
	percentArea /= 100.0;

	// Ask for largest hole to be left out.

	bool userCancel;
	bool prevent;
	SHOW_QUESTION(
		tr("Remove longest polyline -- keep largest hole"),
		tr("Do you want to remove the longest polyline, to prevent it from getting filled?\n\ni.e. Do you want to keep the largest hole in the mesh?"),
		prevent, userCancel
	);
	if( userCancel ) {
		return( false );
	}
     /**
	// Optional border erosion
	bool applyErosion;
	SHOW_QUESTION( tr("Apply border erosion"), tr("Do you want to remove dangling faces along the borders?") +
	               QString("<br /><br />") + tr("Recommended: YES"), applyErosion, userCancel );
	if( userCancel ) {
		return( false );
	}

	// Libpsalm has troubles with larger complex holes. It seems to break for holes with more than 3.000 edges.
	uint64_t maxNrVertices = 3000;
	showEnterText( maxNrVertices, "Maximum number of vertices for border filling. 0 means no limit." );
    **/
	// Ask if we wan't to store the result, when finished.
	bool saveFile;
	SHOW_QUESTION( tr("Store results"), tr("Do you want to store the result as file?"), saveFile, userCancel );
	if( userCancel ) {
		return( false );
	}
	QString fileName = "";
	if( saveFile ) {

        QString fileSuggest = QString::fromStdWString( getBaseName().wstring() );
        //check if the gigamesh name convention is used
        QRegularExpression nameContainsGMwithSuffix( ".*_GM[cCoOfFpP]*$" );
        QRegularExpressionMatch match = nameContainsGMwithSuffix.match( fileSuggest );
        if( match.hasMatch()){
            //check if CF is still used
            QRegularExpression nameContainsGMCF( ".*_GMC.F*$" );
            match = nameContainsGMCF.match( fileSuggest );
            if( !match.hasMatch() ) {
                //add the CF for cleaned and filled
                //regex groups with "(....)" are used for the replace methode --> it's possible to copy the groups
                //with \\... in the replace function
                //check if the name contains a C (GMOC, GMC)
                QRegularExpression nameContainsC( "(.*_GM)([cC]|[oO][cC])(.?[^F]*)$" );
                match = nameContainsC.match( fileSuggest );
                if( match.hasMatch() ) {
                    fileSuggest.replace( nameContainsC, "\\1\\2F\\3" );
                }
                else{
                    //have to add a CF in the case with no "C" inside the abbreviation
                    //GMO followed by a optional character not "C"
                    QRegularExpression nameContainsNoC( "(.*_GM[oO])(.*)$" );
                    match = nameContainsNoC.match( fileSuggest );
                    if( match.hasMatch() ) {
                        fileSuggest.replace( nameContainsNoC, "\\1CF\\2" );
                    }
                }
          }
          } else {
                fileSuggest += "_GMCF";
         }
        fileSuggest += ".ply";

		// Show file dialog
        QString fileLocation = QString::fromStdWString( getFileLocation().wstring() );
        fileLocation += fileSuggest;
		fileName = QFileDialog::getSaveFileName( \
		               mMainWindow, tr( "Save as" ), \
                       fileLocation, tr( "3D-Files (*.obj *.ply *.wrl *.txt *.xyz)" ) \
		           );
	}
    //get parameters from Settings
    bool applyErosion;
    int maxNrVertices;
    getParamIntMeshGL( MeshGLParams::MAX_VERTICES_HOLE_FILLING, &maxNrVertices );
    getParamFlagMeshGL(MeshGLParams::REMOVE_DANGLING_FACES, &applyErosion);

	// Iterative cleaning is done in the Mesh class.
	uint64_t iterationCount;
	std::string resultMsg;
	MeshGL::completeRestore( fileName.toStdString(), percentArea,
	                         applyErosion, prevent, maxNrVertices,
	                         &resultMsg, iterationCount );
	SHOW_MSGBOX_INFO( tr("Complete Restore finished"), QString( resultMsg.c_str() ) );

	return( true );
}

//! Manually insert vertices by entering triplets of coordinates.
//! GUI ... see OpenGL class for updating buffers.
//! @returns true in case of an error or user abort. False otherwise.
bool MeshQt::insertVerticesEnterManual() {
	QGMDialogEnterText dlgEnterTxt;
	dlgEnterTxt.setWindowTitle( tr("Enter triplets of coordinates:") );
	dlgEnterTxt.fetchClipboard( QGMDialogEnterText::CHECK_DOUBLE_MULTIPLE );
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return false;
	}
	vector<double> floatValues;
	if( !dlgEnterTxt.getText( floatValues ) ) {
		showWarning( tr("No coordinates entered").toStdString(), tr("Inserting vertices requires triplets of floating point values!").toStdString() );
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: No (proper) floating point values entered!" << endl;
		return false;
	}
	if( floatValues.size() % 3 != 0 ) {
		showWarning( tr("Wrong number of elements").toStdString(),
					 tr( "Inserting vertices requires triplets of floating point values!\n\n%1 were entered!" ).arg( floatValues.size() ).toStdString()
		           );
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: Wrong number of elementes entered!" << endl;
		return false;
	}

	if( MeshGL::insertVerticesCoordTriplets( &floatValues ) ) {
		setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SOLO,      false );
		setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SYNTHETIC, true  );
		return true;
	}

	return false;
}

//! Show and connect dialog to cut off element values of Vertex feature vectors.
void MeshQt::cutOffFeatureVertex() {
	QGMDialogCutOffValues dialogCutOffValues; // QDialog to cut-off feature elements (e.g. to remove outlier)
	QObject::connect( &dialogCutOffValues, SIGNAL(cutOffValues(double,double,bool)), this, SLOT(cutOffFeatureVertex(double,double,bool)) );
	dialogCutOffValues.exec();
}

//! Handle a GUI request to remove outliers of feature elements of vertices.
void MeshQt::cutOffFeatureVertex( double minVal, double maxVal, bool setToNotANumber ) {
	//cout << "[MeshQt::cutOffFeatureElements] " << endl;
	int retVal = MeshGL::cutOffFeatureElements( Primitive::IS_VERTEX, minVal, maxVal, setToNotANumber );
	if( retVal > 0 ) {
		stringstream message;
		message << retVal << " Feature elements were cut-off.";
		emit statusMessage( QString( message.str().c_str() ) );
	} else if( retVal == 0 ) {
		emit statusMessage( "No Feature elements were cut-off." );
	} else {
		emit statusMessage( "ERROR - A problem occured during feature elements were cut-off." );
	}
}

//--- Edit - Function Values -----------------------------------------------------------------------------------------------------------------------------------

bool MeshQt::funcValSet() {
	//! Calls the dialog to enter the value, which should be set.
	//! (1/2) Dialog
	QGMDialogEnterText dlgEnterTextVal;
	dlgEnterTextVal.setDouble( 0.0 );
	dlgEnterTextVal.setWindowTitle( tr("Set function values to:") );
	QObject::connect( &dlgEnterTextVal, SIGNAL(textEntered(double)), this, SLOT(funcValSet(double)) );
	if( dlgEnterTextVal.exec() == QDialog::Rejected ) {
		return false;
	}
	return true;
}

bool MeshQt::funcValSet( double setTo ) {
	//! See Mesh::setVertFuncVal
	//! (2/2) Execute
	return setVertFuncVal( setTo );
}

void MeshQt::funcValueCutOff() {
	//! Show and connect dialog to cut off function values of the vertices.
	QGMDialogCutOffValues dialogCutOffValues; // QDialog to cut-off feature elements (e.g. to remove outlier)
	QObject::connect( &dialogCutOffValues, SIGNAL(cutOffValues(double,double,bool)), this, SLOT(funcValueCutOff(double,double,bool)) );
	dialogCutOffValues.exec();
}

bool MeshQt::funcValueCutOff( double minVal, double maxVal, bool setToNotANumber ) {
	//! Cuts of function values (of vertices).
	//! When setToNotANumber is false, the values lower than minVal will be set to minVal.
	//! When setToNotANumber is true, the values lower minVal will be set to not-a-number.
	//! Analog: maxVal.
	return setVertFuncValCutOff( minVal, maxVal, setToNotANumber );
}

bool MeshQt::funcValsNormalize() {
	//! See Mesh::setVertFuncValNormalize
	return setVertFuncValNormalize();
}

bool MeshQt::funcValsAbs() {
	//! See Mesh::setVertFuncValAbs
	return setVertFuncValAbs();
}

bool MeshQt::funcValsAdd() {
	//! (1/2) User input for MeshGL::funcValsAdd
	QGMDialogEnterText dlgEnterTextVal;
	dlgEnterTextVal.setDouble( 0.0 );
	dlgEnterTextVal.setWindowTitle( tr("Add constant") );
	QObject::connect( &dlgEnterTextVal, SIGNAL(textEntered(double)), this, SLOT(funcValsAdd(double)) );
	if( dlgEnterTextVal.exec() == QDialog::Rejected ) {
		return false;
	}
	return true;
}

bool MeshQt::funcValsAdd( double rVal ) {
	//! (2/2) Execute - see Mesh::setVertFuncValAdd
	return setVertFuncValAdd( rVal );
}

bool MeshQt::funcValsToFeatureVector()
{
	QGMDialogEnterText dlgEnterTextVal;
	dlgEnterTextVal.setInt(0);
	dlgEnterTextVal.setWindowTitle( tr("Set dimension (warning, resizes feature vectors if necessary!)") );

	QObject::connect(&dlgEnterTextVal, QOverload<int>::of(&QGMDialogEnterText::textEntered), [this](int dim) {this->funcValToFeatureVector(dim);});

	return dlgEnterTextVal.exec() == QDialog::Accepted;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Shows the cone data dialog window and populates it with
//! some default values.
bool MeshQt::setConeData() {

	Vector3D axisTop, axisBot;
	double upperRadius, lowerRadius;
	if( Mesh::getConeAxis(&axisTop, &axisBot) ) {
		mDialogConeParam.setAxis(axisTop, axisBot);
	}

	if( Mesh::getConeRadii( &upperRadius, &lowerRadius ) ) {
		mDialogConeParam.setRadii(upperRadius, lowerRadius);
	}

	mDialogConeParam.show();
	return(true);
}

//! Handles closing the cone parameter dialog window and applies the changes
//! by the user.
bool MeshQt::setConeParameters(
                const Vector3D& rAxisTop,
                const Vector3D& rAxisBot,
                double rUpperRadius,
                double rLowerRadius
) {
	Mesh::setConeAxis( rAxisTop, rAxisBot );
	Mesh::setConeRadius( rUpperRadius, rLowerRadius );
	mWidgetParams->setParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, MeshWidgetParams::SELECTION_MODE_CONE );

	emit updateGL();

	return( true );
}

//! Centers the mesh around the cone.
//! @returns false in case of error. True otherwise.
bool MeshQt::centerAroundCone() {
	if( Mesh::centerAroundCone() ) {
		emit statusMessage( "No cone selected. Use \"Select\" menu to select a cone." );
		return( false );
	}

	this->glRemove();
	this->glPrepare();

	emit sDefaultViewLightZoom();
	emit updateGL();
	emit statusMessage( "Mesh centered using the axis." );

	return( true );
}

//! Unrolls the mesh around a user-specified cone.
//!
//! Interacts with the user.
//!
//! The function value corresponds to the angle of the conical/cylindrical coordinate system
//! and will be shown with the Jet colormap.
//!
//! @returns false in case of error or user cancel. True otherwise.
bool MeshQt::unrollAroundCone( bool* rIsCylinderCase ) {
	// Sanity check
	if( rIsCylinderCase == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!"  << endl;
		return( false );
	}

	// Check if a single primitive was selected.
	if( mPrimSelected != nullptr ) {
		Vector3D primCoG = mPrimSelected->getCenterOfGravity();
		Vector3D axisTop, axisBottom;
		getConeAxis( &axisTop, &axisBottom );
		double angleCandidate = primCoG.angleInLineCoord( &axisTop, &axisBottom );
		// Continue only if a valid angle candidate was found:
		if( isnormal( angleCandidate ) || ( angleCandidate == 0.0 ) ) {
			cout << "[Mesh::" << __FUNCTION__ << "] Angle candidate: " << angleCandidate << " in degree: " << angleCandidate*180.0/M_PI << endl;
			bool userChoice;
			if( showQuestion( &userChoice, tr("Set cutting plane").toStdString(),
							  (tr( "A primitive was selected, which can be used to define the prime meridian as well as the cutting plane (meridian 180°). Press<br /><br />" ) +
							  tr( "YES to use the selection for the prime meridian.<br /><br />" ) +
							  tr( "NO to use the selection for the cutting plane.<br /><br />" ) +
							  tr( "CANCEL to ignore the selection." )).toStdString()
			   ) ) {
				if( userChoice ) {
					// YES for the prime meridian:
					setParamFloatMesh( AXIS_PRIMEMERIDIAN, angleCandidate + M_PI );
				} else {
					// NO for the 180° meridian i.e. cutting plane:
					setParamFloatMesh( AXIS_PRIMEMERIDIAN, angleCandidate );
				}	// CANCEL to ignore.
			}
		} else {
			cout << "[Mesh::" << __FUNCTION__ << "] Invalid value for the angle in line coordinates" << endl;
		}
	}

	if( !MeshGL::unrollAroundCone( rIsCylinderCase ) ) {
		showWarning( tr("No cone defined").toStdString(), tr("Use the \"Select\" menu to interactively define a cone.").toStdString() );
		emit statusMessage( "No cone defined. Use the \"Select\" menu to interactively define a cone." );
		return( false );
	}

	this->glRemove();
	this->glPrepare();

	emit sDefaultViewLightZoom();
	emit statusMessage( "Unroll of the cone is finished." );

	setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_RGB );

	// Ask the user to Straighten the arc-shaped cone rollout.
	if( !(*rIsCylinderCase) ) {
		bool userChoice;
		if( showQuestion( &userChoice, tr("Straighten rollout - Cylinderstyle").toStdString(),
						  tr("Straighten the arc-shaped rollout?<br /><br />Note: this will increase local distortions of the surface!<br /><br />This function can be called later:<br />'Edit' - 'Cylinder - unroll Mesh'" ).toStdString()) ) {
			if( userChoice ) {
				setParamFloatMesh( AXIS_PRIMEMERIDIAN, M_PI_2 ); //! \todo this prevents a wrong split and looks like a quick and dirty solution for a possible bug in Mesh::unrollAroundCylinderRadius
				callFunction( UNROLL_AROUNG_CYLINDER );
				vector<double> rotAngle { M_PI_2 };
				Matrix4D rotUp( Matrix4D::INIT_ROTATE_ABOUT_X, &rotAngle );
				applyTransformationToWholeMesh( rotUp, true );
				setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_RGB );
				emit sDefaultViewLightZoom();
				emit statusMessage( "Straighten of the cone arc is finished." );
			}
		} else {
			// User cancel.
			return( false );
		}
	}

	bool userChoice;
	if( showQuestion( &userChoice, tr("Flip rollout").toStdString(), \
					  tr("Do you want to flip the rollout i.e. rotate the mesh by 180°").toStdString() ) ) {
		if( userChoice ) {
			vector<double> rotAngle { M_PI };
			Matrix4D rotFlip( Matrix4D::INIT_ROTATE_ABOUT_Z, &rotAngle );
			applyTransformationToWholeMesh( rotFlip, true );
			emit sDefaultViewLightZoom();
			emit statusMessage( "Rollout rotated by 180° about the z-axis." );
		}
	} else {
		// User cancel.
		return( false );
	}

	// Swith to function value ...
	setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_FUNCVAL );
	setParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE,  MeshGLParams::GLSL_COLMAP_PARULA  );
	// ... for profiles as isolines.
	double isoDistance;
	getFuncValIsoStepping( 10, &isoDistance );
	// Set parameters and show isolines AKA profiles:
	setParamFloatMeshGL( MeshGLParams::ISOLINES_DISTANCE,  isoDistance );
	setParamFloatMeshGL( MeshGLParams::ISOLINES_OFFSET,            0.0 );
	setParamFlagMeshGL(  MeshGLParams::SHOW_FUNC_VALUES_ISOLINES, true );

	// Done.
	return( true );
}

//! Unrolls the mesh around a user-specified cylinder.
bool MeshQt::unrollAroundCylinderRadius() {
	using namespace std::chrono;
	high_resolution_clock::time_point tStart = high_resolution_clock::now();

	if( Mesh::unrollAroundCylinderRadius() ) {
		high_resolution_clock::time_point tEnd = high_resolution_clock::now();
		duration<double> time_span = duration_cast<duration<double>>( tEnd - tStart );
		cout << "[MeshQt::" << __FUNCTION__ << "] unrollAroundCylinderRadius:" << time_span.count() << " seconds." << std::endl;

		this->glRemove();
		this->glPrepare();

		double radiusUsed = 0.0;
		getParamFloatMesh( CYLINDER_RADIUS, &radiusUsed );

		emit sDefaultViewLightZoom();
		emit statusMessage( QString( "Unroll of the cylinder having a radius of %1 mm is finished." ).arg( radiusUsed ) );
	} else {
		emit statusMessage( "No cylinder defined. Set axis and radius!" );
	}

	return true;
}

//! Splits the mesh by a given plane.
//! (1/2) User dialog: Set flags for plane dialog box.
bool MeshQt::splitByPlane() {
	return setPlaneHNF( false, false, true );
}

//! Splits the mesh by a given plane.
//! (2/2) Execute.
bool MeshQt::splitByPlane( Vector3D rPlaneHNF, bool rSeperate ) {
	if( !Mesh::splitByPlane( rPlaneHNF, rSeperate ) ) {
		emit statusMessage( "No valid plane given." );
		return false;
	}
	return true;
}

//! Split the mesh using the iso-value and the vertices function values.
bool MeshQt::splitByIsoValue() {
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	double isoValue;
	getParamFloatMesh( MeshParams::FUNC_VALUE_THRES, &isoValue );
	bool duplicateVertices = true;
	if( !Mesh::splitByIsoLine( isoValue, duplicateVertices ) ) {
		emit statusMessage( "Split by Iso-Value failed." );
		return false;
	}
	return true;
}

//! Centers the mesh around the sphere.
bool MeshQt::centerAroundSphere() {
	if(Mesh::centerAroundSphere()) {
		this->glRemove();
		this->glPrepare();

		emit sDefaultViewLightZoom();
	}
	else {
		emit statusMessage("No sphere selected. Use \"Select\" menu to select a sphere.");
	}

	return(true);
}

//! Unrolls the mesh around a user-specified sphere.
bool MeshQt::unrollAroundSphere() {
	if(hasDatumObjects())
	{
		bool proceed = true;
		if(!showQuestion(&proceed, "Warning", "Warning: There are datum-objects in the scene that will be deleted by this operation.\nDo you want to continue?"))
			return false;

		if(!proceed)
			return false;
	}

	removeAllDatumObjects();

	using namespace std::chrono;
	high_resolution_clock::time_point tStart = high_resolution_clock::now();

	if( Mesh::unrollAroundSphere() ) {
		high_resolution_clock::time_point tEnd = high_resolution_clock::now();
		duration<double> time_span = duration_cast<duration<double>>( tEnd - tStart );
		cout << "[MeshQt::" << __FUNCTION__ << "] unrollAroundSphere:" << time_span.count() << " seconds." << std::endl;

		this->glRemove();
		this->glPrepare();

		emit sDefaultViewLightZoom();
		emit statusMessage( "Unroll of the sphere is finished." );
	} else {
		emit statusMessage( "No sphere selected. Use \"Select\" menu to select a sphere." );
		showWarning( tr("No sphere selected.").toStdString(), tr("Use \"Select\" menu to select a sphere.").toStdString() );
		return( false );
	}

	// Switch to color per vertex ...
	setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_RGB );

	// Always flip as it seems that the Mesh::unrollAroundSphere() always show the object upside down.
	//! \todo Check if this can be improved in Mesh::unrollAroundSphere()
	vector<double> rotAngle { M_PI };
	Matrix4D rotFlip( Matrix4D::INIT_ROTATE_ABOUT_Z, &rotAngle );
	applyTransformationToWholeMesh( rotFlip, true );
	emit sDefaultViewLightZoom();

	// Ask user to change the orientation in case the rollout is upside-down
	bool userChoice;
	if( showQuestion( &userChoice, tr("Flip rollout").toStdString(),
					  tr("Do you want to flip the rollout i.e. rotate the mesh by 180°").toStdString() ) ) {
		if( userChoice ) {
			vector<double> rotAngle { M_PI };
			Matrix4D rotFlip( Matrix4D::INIT_ROTATE_ABOUT_Z, &rotAngle );
			applyTransformationToWholeMesh( rotFlip, true );
			emit sDefaultViewLightZoom();
			emit statusMessage( "Rollout rotated by 180° about the z-axis." );
		}
	} else {
		// User cancel.
		return( false );
	}

	// Switch to function value ...
	setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_FUNCVAL );
	setParamIntMeshGL( MeshGLParams::GLSL_COLMAP_CHOICE,  MeshGLParams::GLSL_COLMAP_PARULA  );
	// ... for profiles as isolines.
	double isoDistance;
	getFuncValIsoStepping( 10, &isoDistance );
	// Set parameters and show isolines AKA profiles:
	setParamFloatMeshGL( MeshGLParams::ISOLINES_DISTANCE,  isoDistance );
	setParamFloatMeshGL( MeshGLParams::ISOLINES_OFFSET,            0.0 );
	setParamFlagMeshGL(  MeshGLParams::SHOW_FUNC_VALUES_ISOLINES, true );

	// Done.
	return( true );
}

//! Manually add a datum sphere.
//! (1/2) Dialog.
bool MeshQt::datumAddSphere() {
	QGMDialogEnterText dlgEnterTxt;
	dlgEnterTxt.setWindowTitle( tr("Enter a position vector and a radius") );
	dlgEnterTxt.fetchClipboard( QGMDialogEnterText::CHECK_DOUBLE_MULTIPLE );
	QObject::connect( &dlgEnterTxt, SIGNAL(textEntered(vector<double>)), this, SLOT(datumAddSphere(vector<double>)) );
	dlgEnterTxt.exec();
	return true;
}

//! Manually add a datum sphere.
//! Reuires 4 values: x,y and z for the sphere's center and a radius.
//! Optional: red, green and blue can be given.
//! (2/2) Execute.
bool MeshQt::datumAddSphere( vector<double> rPosAndRadius ) {
	if( rPosAndRadius.size() == 7 ) {
		Mesh::datumAddSphere( Vector3D( rPosAndRadius[0], rPosAndRadius[1], rPosAndRadius[2], 1.0 ), rPosAndRadius.at(3), rPosAndRadius.at(4), rPosAndRadius.at(5), rPosAndRadius.at(6) );
		return true;
	}
	if( rPosAndRadius.size() != 4 ) {
		SHOW_MSGBOX_WARN( tr("Enter a position vector and a radius"), tr( "Requires 4 values: x, y, z and a radius.\n%1 values given!" ).arg( rPosAndRadius.size() ) );
		return false;
	}
	Mesh::datumAddSphere( Vector3D( rPosAndRadius[0], rPosAndRadius[1], rPosAndRadius[2], 1.0 ), rPosAndRadius.at(3) );
	emit updateGL();
    return true;
}
//! Downscale the corresponding textures of the mesh
//! Asks for the ration of downscaling
//! Overwrites the original file
bool MeshQt::downscaleTexture()
{
    ModelMetaData modelMetaData = MeshIO::getModelMetaDataRef();
    std::vector<std::filesystem::path> textures = modelMetaData.getTexturefilesRef();
    if(!(textures.size() > 0)){
        cout << "[MeshQt::" << __FUNCTION__ << "] No texture for this mesh available!" << std::endl;
        return false;
    }
    else{
        // Ask for vertex normals
        bool continueDownscaling = true;
        bool userCancel = false;
        SHOW_QUESTION( tr("Continue"), tr("This function overwrites the current texture. Do you want to continue?"), continueDownscaling, userCancel );
        if( userCancel || !continueDownscaling) {
            return false;
        }

        //Percentage of the texture that should be kept
        //in double because the showSlider function needs it
        double suggestPercent = 25;
        // Show dialog
        if( !showSlider(&suggestPercent,0,100,"How many percent of the texture do you want to keep?")) {
            std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: bad input (1)!" << std::endl;
            return( false );
        }
        if(suggestPercent > 100)suggestPercent=100;
        //the scaling factor defines how much the image is scaled down
        double factor = suggestPercent/100;
        //change all textures that are assigned to the mesh
        for(auto texture : textures){
            QString texturePath = QString::fromStdString(std::filesystem::relative(texture).string());
            QImage textureImg(texturePath);
            int width = textureImg.width();
            int height = textureImg.height();

            int newWidth = round(factor*width);
            int newHeight = round(factor*height);
            QImage textureImgScaled = textureImg.scaled(newWidth, newHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            textureImgScaled.save(texturePath);
        }
        //simulate the reload click
        emit sReloadFile();
        //set view to texture automatically
        //mMainWindow->sShowParamIntMeshGL(MeshGLParams::SHADER_CHOICE,5);

    }

    return true;
}

//! Enter radius and melting with sqrt(r^2-x^2-y^2).
//! See MeshGL::applyMeltingSphere
bool MeshQt::applyMeltingSphere() {
	QGMDialogEnterText dlgEnterTxt;
	dlgEnterTxt.setWindowTitle( tr("Enter radius") );
	dlgEnterTxt.setDouble( 1.0 );
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return false;
	}
	double radius;
	if( !dlgEnterTxt.getText( &radius ) ) {
		return false;
	}
    return MeshGL::applyMeltingSphere( radius, 1.0 );
}

//!Automatic Mesh Alignment
//! calculates principal components (PCA) of the vertices
//! Sets the PCs as new camera vectors
//! result should be that the biggest extension of the is equal to the y-axis
bool MeshQt::applyAutomaticMeshAlignment(bool askForFront)
{

    std::cout << "[MeshQT::" << __FUNCTION__ << "] Start:Automatic Mesh Alignment" << std::endl;
    Matrix4D pcaTransformationMatrix;
    AutomaticAlignmentPyInterface pyInterface(&mVertices);
    if(!pyInterface.startPythonScript(&pcaTransformationMatrix)){
        QMessageBox msgBox;
        msgBox.setText( tr("There is a problem to run the required Python script! \n The Python Module: Pandas is necessary! \n Check your Python-Path in Settings->External Programs!") );
        msgBox.setIcon( QMessageBox::Critical );
        msgBox.exec();
        return false;
    }


    //Matrix4D transMat(transMatVec);
    applyTransformationToWholeMesh(pcaTransformationMatrix);




    //calculate the determinant of the rotation part inside the transformation matrix
    //if the determinant is negative then mirror the mesh
    double det = pcaTransformationMatrix.getX(0)*pcaTransformationMatrix.getY(1)*pcaTransformationMatrix.getZ(2) +
                pcaTransformationMatrix.getY(0)*pcaTransformationMatrix.getZ(1)*pcaTransformationMatrix.getZ(2) +
                pcaTransformationMatrix.getZ(0)*pcaTransformationMatrix.getX(1)*pcaTransformationMatrix.getY(2) -
                pcaTransformationMatrix.getZ(0)*pcaTransformationMatrix.getY(1)*pcaTransformationMatrix.getX(2) -
                pcaTransformationMatrix.getY(0)*pcaTransformationMatrix.getX(1)*pcaTransformationMatrix.getZ(2) -
                pcaTransformationMatrix.getX(0)*pcaTransformationMatrix.getZ(1)*pcaTransformationMatrix.getY(2);


    std::vector<double> rotationAngle = {-90 * M_PI / 180.0};

    if (det < 0){
        Matrix4D xyMirror(Matrix4D::INIT_IDENTITY);
        xyMirror.set(2,2,-1);
        xyMirror.set(1,1,-1);
        xyMirror.set(0,0,-1);
        applyTransformationToWholeMesh(xyMirror);
        //rotate the mesh clockwise
        rotationAngle = {90 * M_PI / 180.0};

    }

    //rotate about z --> it changes the highest pc to y axis
    Matrix4D zRotation(Matrix4D::INIT_ROTATE_ABOUT_Z,&rotationAngle);
    applyTransformationToWholeMesh(zRotation);
    changedMesh();

    //Recompute the normals
    //It's necessary to prevent that the mesh is sometimes dark after the transformation
    resetFaceNormals();
    resetVertexNormals();




    //do not ask if the method is called by the directory function
    if(askForFront){
        //Decide which part of the mesh is the front
        //only for stone tools

        bool curvatureAlignment = false;
        bool userCancel = false;
        SHOW_QUESTION( tr("Front Alignment"), tr( "Do you want to align the front of the mesh based on the curvature?" ), curvatureAlignment, userCancel );
        if( userCancel ) {
            return false;
        }
        if (curvatureAlignment){
            //define the start centroids of k-means
            //use the points in the middle of the mesh and the minimum and the maximum of the dimension with the smallest extension
            //0 = x
            //1 = y
            //2 = z
            int smallestExtension = 0;
            double xExtension = abs(Mesh::getMinX() - Mesh::getMaxX());
            double yExtension = abs(Mesh::getMinY() - Mesh::getMaxY());
            double zExtension = abs(Mesh::getMinZ() - Mesh::getMaxZ());
            if (xExtension > yExtension){
                smallestExtension = 1;
            }
            if (xExtension > zExtension && yExtension > zExtension){
                smallestExtension = 2;
            }

            Vector3D centroid1;
            Vector3D centroid2;
            switch(smallestExtension){
                case 0:
                    centroid1 = Vector3D(1.0,0.0,0.0);
                    centroid2 = Vector3D(-1.0,0.0,0.0);
                    break;
                case 1:
                    centroid1 = Vector3D(0.0,1.0,0.0);
                    centroid2 = Vector3D(0.0,-1.0,0.0);
                    break;
                case 2:
                    centroid1 = Vector3D(0.0,0.0,1.0);
                    centroid2 = Vector3D(0.0,0.0,-1.0);
                    break;
            }
            std::vector<Vector3D> centroids = {centroid1,centroid2};
            std::vector<std::set<Vertex*>> clusterSets = Mesh::computeVertexNormalKMeans(&centroids,true);

            //calculate ambient occlusion to get some kind of curviture values
            //the results are stored in vertices as function values
            //for parameters we use the recommended values of the GUI
            /**
            int depthBuffeRes = 512;
            unsigned int numberOfDirections = 1000;
            int maxValueBufferRes = 512;
            float zTolerance = 0.0;
            if( !funcVertAmbientOcclusionHW( depthBuffeRes, maxValueBufferRes, numberOfDirections, zTolerance ) ) {
                    return false;
            }

            //calculate the average curviture of the cluster -> the cluster/mesh-side with the higher curviture should be in front
            double meanClust1 = 0.0;
            double meanClust2 = 0.0;

            std::set<Vertex*>::iterator itr;
            for (itr = clusterSets.at(0).begin(); itr != clusterSets.at(0).end(); itr++){
                double funcVal = 0.0;
                (*itr)->getFuncValue(&funcVal);
                meanClust1 += funcVal;
            }
            meanClust1 = meanClust1/clusterSets.at(0).size();

            for (itr = clusterSets.at(1).begin(); itr != clusterSets.at(1).end(); itr++){
                double funcVal = 0.0;
                (*itr)->getFuncValue(&funcVal);
                meanClust2 += funcVal;
            }
            meanClust2 = meanClust2/clusterSets.at(1).size();
            if (meanClust1 < meanClust2){
                //rotate 180 degree to get the side with higher curviture to front
                const double s = sin(180 * M_PI / 180.0);
                const double c = cos(180 * M_PI / 180.0);
                Matrix4D rotationMatrix( {  c, 0.0,  -s, 0.0,
                                            0.0, 1.0, 0.0, 0.0,
                                              s, 0.0,   c, 0.0,
                                            0.0, 0.0, 0.0, 1.0} );
                applyTransformationToWholeMesh(rotationMatrix);

            }
            **/
            if (clusterSets.at(0).size() < clusterSets.at(1).size()){
                //rotate 180 degree to get the side with higher curviture to front
                const double s = sin(180 * M_PI / 180.0);
                const double c = cos(180 * M_PI / 180.0);
                Matrix4D rotationMatrix( {  c, 0.0,  -s, 0.0,
                                            0.0, 1.0, 0.0, 0.0,
                                              s, 0.0,   c, 0.0,
                                            0.0, 0.0, 0.0, 1.0} );
                applyTransformationToWholeMesh(rotationMatrix);

            }
        }
    }
    //the transformation with the identity matrix resets the mesh to the center
    Matrix4D identity(Matrix4D::INIT_IDENTITY);
    applyTransformationDefaultViewMatrix(&identity);

    //calculate: where is the spike of the mesh
    //the spike should always point to the top
    Vector3D center = getCenterOfGravity();
    float distanceTop = mMaxY - center.getY();
    float distanceBottom = center.getY() - mMinY;
    if (distanceTop < distanceBottom){
        rotationAngle = {180 * M_PI / 180.0};
        Matrix4D zRotation(Matrix4D::INIT_ROTATE_ABOUT_Z,&rotationAngle);
        applyTransformationToWholeMesh(zRotation);
    }

    // setup initial view (emit Signal to meshwidget.cpp:
    emit sDefaultViewLight();


    return true;
}



// --- Select actions ------------------------------------------------------------------------------------------------------------------------------------------

//! Update the shown number of selected vertices.
//! @returns the number of selected vertices or NaN in case of an error.
unsigned int MeshQt::selectedMVertsChanged() {
	cout << "[MeshQt::" << __FUNCTION__ << "]." << endl;
	if( mMainWindow == nullptr ) {
		return _NOT_A_NUMBER_INT_;
	}
	unsigned int selMVertsNr = MeshGLShader::selectedMVertsChanged();
	setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SELECTION, ( selMVertsNr != 0 ) );
	emit sInfoMesh( MeshGLParams::MGLINFO_SELECTED_VERTICES, QString::number( selMVertsNr ) );
	emit updateGL();
	return selMVertsNr;
}

//! Update the shown number of selected faces.
//! @returns the number of selected faces or NaN in case of an error.
unsigned int MeshQt::selectedMFacesChanged() {
	cout << "[MeshQt::" << __FUNCTION__ << "]." << endl;
	if( mMainWindow == nullptr ) {
		return _NOT_A_NUMBER_INT_;
	}
	unsigned int selMVertsNr = MeshGLShader::selectedMFacesChanged();
	setParamFlagMeshGL( MeshGLParams::SHOW_FACES_SELECTION, true );
	emit sInfoMesh( MeshGLParams::MGLINFO_SELECTED_FACES, QString::number( selMVertsNr ) );
	emit updateGL();
	return selMVertsNr;
}

//! Update the shown number of selected polylines.
//! @returns the number of selected polylines or NaN in case of an error.
unsigned int MeshQt::selectedMPolysChanged() {
	cout << "[MeshQt::" << __FUNCTION__ << "]." << endl;
	if( mMainWindow == nullptr ) {
		return _NOT_A_NUMBER_INT_;
	}
	unsigned int selMVertsNr = MeshGLShader::selectedMPolysChanged();
	emit sInfoMesh( MeshGLParams::MGLINFO_SELECTED_POLYLINES, QString::number( selMVertsNr ) );
	emit updateGL();
	return selMVertsNr;
}

//! Update the shown number of selected positions.
//! @returns the number of selected polylines or NaN in case of an error.
unsigned int MeshQt::selectedMPositionsChanged() {
	cout << "[MeshQt::" << __FUNCTION__ << "]." << endl;
	if( mMainWindow == nullptr ) {
		return _NOT_A_NUMBER_INT_;
	}
	unsigned int selMVertsNr = MeshGLShader::selectedMPositionsChanged();
	emit sInfoMesh( MeshGLParams::MGLINFO_SELECTED_POSITIONS, QString::number( selMVertsNr ) );
	emit updateGL();
	return selMVertsNr;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Show the coordinates of (the) three points defining the Mesh plane.
bool MeshQt::getPlaneVPos() {
	string planeInfo;
	if( !MeshGL::getPlaneVPos( &planeInfo ) ) {
		return false;
	}
	QString planeDesc( planeInfo.c_str() );
	SHOW_MSGBOX_INFO( tr("Plane Vertices"), planeDesc + "\n" + tr("Already copied to clipboard!") );
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText( planeDesc.replace( "\n", " " ) );
	return true;
}

//! Show the Hesse Normal Form (HNF) defining the Mesh plane.
bool MeshQt::getPlaneHNF() {
	string planeInfo;
	if( !MeshGL::getPlaneHNF( &planeInfo ) ) {
		return false;
	}
	QString planeDesc( planeInfo.c_str() );
	SHOW_MSGBOX_INFO( tr("Plane HNF"), planeDesc + "\n" + tr("Already copied to clipboard!") );
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText( tr("GigaMesh HNF  ") + planeDesc.replace( "\n", " " ) );
	return true;
}

//! Set mesh plane using 3 points in R3 (3x3 coordinates).
//! (1) Show dialogbox to enter coordinates as string.
bool MeshQt::setPlaneVPos() {
	QGMDialogEnterText dlgEnterTxt;
	dlgEnterTxt.setWindowTitle( tr("Enter 3x3 coordinates defining a plane") );
	dlgEnterTxt.fetchClipboard( QGMDialogEnterText::CHECK_DOUBLE_MULTIPLE );
	QObject::connect( &dlgEnterTxt, SIGNAL(textEntered(vector<double>)), this, SLOT(setPlaneVPos(vector<double>)) );
	dlgEnterTxt.exec();
	return true;
}

//! Set mesh plane using 3 points in R3 (3x3 coordinates).
//! (2) Execute - Set position.
bool MeshQt::setPlaneVPos( vector<double> rPositions ) {
	return MeshGL::setPlaneVPos( &rPositions );
}

//! Set mesh plane using the Hesse Normal Form (HNF).
//! (0) Set flags for dialog.
bool MeshQt::setPlaneHNF() {
	return setPlaneHNF( true, false, false );
}

//! Set mesh plane using the Hesse Normal Form (HNF).
//! (1) Show dialogbox to enter the HNF parameters.
bool MeshQt::setPlaneHNF( bool rPreferSet, bool rPreferVisDist, bool rPreferSplit ) {
	QGMDialogPlaneParam dlgPlaneParam;
	Vector3D planeHNF;
	if( !Mesh::getPlaneHNF( &planeHNF ) ) {
		return false;
	}
	if( !dlgPlaneParam.setPlaneHNF( planeHNF ) ) {
		// User cancel or error
		return false;
	}
	if( rPreferSet ) {
		dlgPlaneParam.preferSet();
	}
	if( rPreferVisDist ) {
		dlgPlaneParam.preferVisDist();
	}
	if( rPreferSplit ) {
		dlgPlaneParam.preferSplit();
	}
	QObject::connect( &dlgPlaneParam, SIGNAL(sComputeDistance(Vector3D,bool)), this, SLOT(visualizeDistanceToPlane(Vector3D,bool)) );
	QObject::connect( &dlgPlaneParam, SIGNAL(sSetPlaneHNF(Vector3D)),          this, SLOT(setPlaneHNF(Vector3D))                   );
	QObject::connect( &dlgPlaneParam, SIGNAL(sSplitByPlane(Vector3D,bool)),    this, SLOT(splitByPlane(Vector3D,bool))             );
	dlgPlaneParam.exec();
	return true;
}

//! Set mesh plane using the Hesse Normal Form (HNF).
//! (2) Execute - Set position.
bool MeshQt::setPlaneHNF( Vector3D rPlaneHNF ) {
    return MeshGL::setPlaneHNF( &rPlaneHNF );
}

//! Select all vertices with a feature value lower than ...
void MeshQt::selectVertFuncValLowerThan() {
	//dialogSlider.setIdx( ... );
	double minVal;
	double maxVal;
	if( !getFuncValuesMinMax( minVal, maxVal ) ) {
		QMessageBox msgBox;
		msgBox.setText( tr("No function values set.") );
		msgBox.setIcon( QMessageBox::Warning );
		msgBox.exec();
		return;
	}
	QGMDialogSliderHD dlgSlider;
	dlgSlider.setMin( minVal );
	dlgSlider.setMax( maxVal );
	// set to a safe value:
	dlgSlider.setPos( minVal );
	// or (override) to the last Threshold:
	double previousThres;
	getParamFloatMesh( FUNC_VALUE_THRES, &previousThres );
	if( !std::isnan( previousThres ) ) {
		dlgSlider.setPos( previousThres );
	}
	// or (override) to the value of the seleted Primitve:
	if( getPrimitiveSelected() != nullptr ) {
		getPrimitiveSelected()->getFuncValue( &previousThres );
		if( !std::isnan( previousThres ) ) {
			dlgSlider.setPos( previousThres );
		}
	}
	dlgSlider.suppressPreview();
	dlgSlider.setWindowTitle( tr("Select vertices with function value lower than") );
	//QObject::connect( &dialogSlider, SIGNAL(valuePreview(int,float)),  this, SLOT(setViewParams(int,float)) );
	//QObject::connect( &dialogSlider, SIGNAL(valueSelected(double)), this, SLOT(selectVertFuncValLowerThan(double)) );
	if( dlgSlider.exec() == QDialog::Rejected ) {
		return;
	}
	double newValue;
	if( !dlgSlider.getValue( &newValue ) ) {
		return;
	}
	setParamFloatMesh( MeshGL::FUNC_VALUE_THRES, newValue );
	MeshGL::selectVertFuncValLowerThan( newValue );
}

//! Select all vertices with a feature value greater than ...
void MeshQt::selectVertFuncValGreatThan() {
	//dialogSlider.setIdx( ... );
	double minVal;
	double maxVal;
	if( !getFuncValuesMinMax( minVal, maxVal ) ) {
		QMessageBox msgBox;
		msgBox.setText( tr("No function values set.") );
		msgBox.setIcon( QMessageBox::Warning );
		msgBox.exec();
		return;
	}
	QGMDialogSliderHD dlgSlider;
	dlgSlider.setInverted( true );	// invert (first!)
	dlgSlider.setMin( minVal );
	dlgSlider.setMax( maxVal );
	// set to a safe value:
	dlgSlider.setPos( minVal );
	// or (override) to the last Threshold:
	double previousThres;
	getParamFloatMesh( FUNC_VALUE_THRES, &previousThres );
	if( !std::isnan( previousThres ) ) {
		dlgSlider.setPos( previousThres );
	}
	// or (override) to the value of the seleted Primitve:
	if( getPrimitiveSelected() != nullptr ) {
		getPrimitiveSelected()->getFuncValue( &previousThres );
		if( !std::isnan( previousThres ) ) {
			dlgSlider.setPos( previousThres );
		}
	}
	dlgSlider.suppressPreview();
	dlgSlider.setWindowTitle( tr("Select vertices with function value greater than") );
	//QObject::connect( &dialogSlider, SIGNAL(valuePreview(int,float)),  this, SLOT(setViewParams(int,float)) );
	//QObject::connect( &dialogSlider, SIGNAL(valueSelected(double)), this, SLOT(selectVertFuncValGreatThan(double)) );
	if( dlgSlider.exec() == QDialog::Rejected ) {
		return;
	}
	double newValue;
	if( !dlgSlider.getValue( &newValue ) ) {
		return;
	}
	setParamFloatMesh( MeshGL::FUNC_VALUE_THRES, newValue );
	MeshGL::selectVertFuncValGreatThan( newValue );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------


//! Select all vertices via Non-Maximum Suppression...
bool MeshQt::selectVertNonMaximum() {
    double EPSdot = 0.05;
    QGMDialogEnterText dlgEnterText;

	dlgEnterText.setWindowTitle( tr("Enter Epsilon for dotproduct.") );
    dlgEnterText.setDouble( EPSdot );
    if( dlgEnterText.exec() == QDialog::Rejected ) {
        return false;
    }
    dlgEnterText.getText( &EPSdot );

    setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SELECTION, true );

    return MeshGL::selectVertNonMaximum( EPSdot );
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Add all border vertices to the selection.
//! Returns true, when vertices were added.
bool MeshQt::selectVertBorder() {
	return MeshGL::selectVertBorder();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Selects all vertices tagged as local minimum.
bool MeshQt::selectVertLocalMin() {
	return MeshGL::selectVertLocalMin();
}

//! Selects all vertices tagged as local maximum.
bool MeshQt::selectVertLocalMax() {
	return MeshGL::selectVertLocalMax();
}

// De-Selection ------------------------------------------------------------------------------------------------------------------------------------------------

//! Deselects all vertices (Clears SelMVerts).
//! @returns true, when vertices were deselected.
bool MeshQt::deSelMVertsAll() {
	return MeshGL::deSelMVertsAll();
}

//! Deselects all vertices (from SelMVerts) having no label - see MeshGL::deSelMVertsNoLabel and Primitive::isLabled.
//! @returns true, when vertices were deselected.
bool MeshQt::deSelMVertsNoLabel() {
	return MeshGL::deSelMVertsNoLabel();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Adds solo vertices to the selection.
bool MeshQt::selectVertSolo() {
	return MeshGL::selectVertSolo();
}

//! Add all vertices from non-manifold faces to the selection.
bool MeshQt::selectVertNonManifoldFaces() {
	return MeshGL::selectVertNonManifoldFaces();
}

//! Add all vertices being a double-cone center to the selection.
bool MeshQt::selectVertDoubleCone() {
	return MeshGL::selectVertDoubleCone();
}

//! Add all vertices from small labels (absolute area).
//! Based on Vertex labeling.
bool MeshQt::selectVertLabelAreaLT() {
	double areaMax = 10;
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Maximum label area (absolut)") );
	dlgEnterText.setDouble( areaMax );
	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return false;
	}
	dlgEnterText.getText( &areaMax );
	return MeshGL::selectVertLabelAreaLT( areaMax );
}

bool MeshQt::selectVertLabelAreaRelativeLT() {
	//! Add all vertices from small labels (precentage of total area).
	//! Based on Vertex labeling.
	//! Method part 1 of 2
	mDialogSlider.setMin(   0.0 + DBL_EPSILON );
	mDialogSlider.setMax( 100.0 - DBL_EPSILON );
	mDialogSlider.setPos( 1.0 );
	//dialogSlider.setPos( getParaSmoothLength() );
	mDialogSlider.suppressPreview();
	mDialogSlider.setWindowTitle(  tr("Maximum label area (relative)") );
	//QObject::connect( &dialogSlider, SIGNAL(valuePreview(int,double)),  this, SLOT(setViewParams(int,double)) );
	QObject::connect( &mDialogSlider, SIGNAL(valueSelected(double)), this, SLOT(selectVertLabelAreaRelativeLT(double)) );
	mDialogSlider.show();
	return true;
}

bool MeshQt::selectVertLabelAreaRelativeLT( double rPercent ) {
	//! Add all vertices from small labels (precentage of total area).
	//! Based on Vertex labeling.
	//! Method part 2 of 2
	//! As the slider dialog will return a value of ] 0.0 ... 100.0 [ we divide by 100.
	rPercent /= 100.0;
	return MeshGL::selectVertLabelAreaRelativeLT( rPercent );
}

bool MeshQt::selectVertFaceMinAngleLT() {
	//! Add all vertices from faces with a minimum angle lower than ... to be selected.
	//! Method part 1 of 2
	mDialogSlider.setMin(   0.0 + DBL_EPSILON );
	mDialogSlider.setMax( 180.0 - DBL_EPSILON );
	mDialogSlider.setPos( 170.0 );
	//dialogSlider.setPos( getParaSmoothLength() );
	mDialogSlider.suppressPreview();
	mDialogSlider.setWindowTitle(  tr("Minimum angle (degree)") );
	//QObject::connect( &dialogSlider, SIGNAL(valuePreview(int,double)),  this, SLOT(setViewParams(int,double)) );
	QObject::connect( &mDialogSlider, SIGNAL(valueSelected(double)), this, SLOT(selectVertFaceMinAngleLT(double)) );
	mDialogSlider.show();
	return true;
}

bool MeshQt::selectVertFaceMinAngleLT( double rMaxAngle ) {
	//! Add all vertices from faces with a minimum angle lower than rMaxAngle (in degree).
	//! Method part 2 of 2
	//! As the slider dialog will return an angle in degree, we convert the angle to radiant.
	rMaxAngle *= M_PI/180.0;
	return MeshGL::selectVertFaceMinAngleLT( rMaxAngle );
}

bool MeshQt::selectVertFaceMaxAngleGT() {
	//! Add all vertices from faces with a maximum angle larger than ... to be selected.
	//! Method part 1 of 2
	mDialogSlider.setMin(   0.0 + DBL_EPSILON );
	mDialogSlider.setMax( 180.0 - DBL_EPSILON );
	mDialogSlider.setPos( 170.0 );
	//dialogSlider.setPos( getParaSmoothLength() );
	mDialogSlider.suppressPreview();
	mDialogSlider.setInverted( true );
	mDialogSlider.setWindowTitle(  tr("Maximum angle (degree)") );
	//QObject::connect( &dialogSlider, SIGNAL(valuePreview(int,double)),  this, SLOT(setViewParams(int,double)) );
	QObject::connect( &mDialogSlider, SIGNAL(valueSelected(double)), this, SLOT(selectVertFaceMaxAngleGT(double)) );
	mDialogSlider.show();
	return true;
}

bool MeshQt::selectVertFaceMaxAngleGT( double rMinAngle ) {
	//! Add all vertices from faces with a maximum angle larger than rMinAngle (in degree).
	//! Method part 2 of 2
	//! As the slider dialog will return an angle in degree, we convert the angle to radiant.
	rMinAngle *= M_PI/180.0;
	return MeshGL::selectVertFaceMaxAngleGT( rMinAngle );
}

//! Add vertices having no label no. set, which are also not background!.
bool MeshQt::selVertLabeledNot() {
	return MeshGL::selVertLabeledNot();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

bool MeshQt::selectFaceNone() {
	//! Empties the selection of faces.
	return MeshGL::selectFaceNone();
}

bool MeshQt::selectFaceSticky() {
	//! Selects all sticky faces - see Mesh::selectFaceSticky
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	return MeshGL::selectFaceSticky();
}

bool MeshQt::selectFaceNonManifold() {
	//! Selects all non-manifold faces - see Mesh::selectFaceNonManifold
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	return MeshGL::selectFaceNonManifold();
}

bool MeshQt::selectFaceZeroArea() {
	//! Selects all faces with zero area - see Mesh::selectFaceZeroArea
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	return MeshGL::selectFaceZeroArea();
}

bool MeshQt::selectFaceInSphere() {
	//! Select all faces within a sphere of radius r.
	//! Position vector is used from selection and radius has to be entered by user.
	//! (1/2) Dialog for radius.
	Primitive* primSel = getPrimitiveSelected();
	if( ( primSel == nullptr ) || ( primSel->getType() != Primitive::IS_VERTEX ) ) {
		SHOW_MSGBOX_WARN( tr("No selection"), tr("No vertex selected.") );
		return false;
	}
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Enter radius") );
	dlgEnterText.setDouble( 1.0 );
	QObject::connect( &dlgEnterText, SIGNAL(textEntered(double)), this, SLOT(selectFaceInSphere(double)) );
	if( dlgEnterText.exec() == QDialog::Accepted ) {
		return true;
	}
	return false;
}

bool MeshQt::selectFaceInSphere( double rRadius ) {
	//! Select all faces within a sphere of radius r.
	//! Position vector is used from selection and radius has to be entered by user.
	//! (2/2) Execute.
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	Primitive* primSel = getPrimitiveSelected();
	if( ( primSel == nullptr ) || ( primSel->getType() != Primitive::IS_VERTEX ) ) {
		SHOW_MSGBOX_WARN( tr("No selection"), tr("No vertex selected.") );
		return false;
	}
	return MeshGL::selectFaceInSphere( static_cast<Vertex*>(primSel), rRadius );
}

//! Select a random number of faces.
//! (1/2) Ask for amount.
bool MeshQt::selectFaceRandom() {
	QGMDialogSliderHD dlgSlider;
	dlgSlider.setMin(   0.0 );
	dlgSlider.setMax( 100.0 );
	dlgSlider.setPos(  10.0 );
	dlgSlider.setWindowTitle( tr("Set ratio for random selection") );
	dlgSlider.suppressPreview();

	QObject::connect( &dlgSlider, SIGNAL(valueSelected(double)), this, SLOT(selectFaceRandom(double)) );
	//QObject::connect(&dlgSlider, SIGNAL(valuePreview(double)), this, SLOT(...(double)));

	dlgSlider.exec();
	return true;
}

//! Select a random number of faces.
//! (2/2) Execute.
bool MeshQt::selectFaceRandom( double rRatio ) {
	bool retVal = MeshGL::selectFaceRandom( rRatio );
	return retVal;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

bool MeshQt::selectPolyNoLabel() {
	//! Selects all polylines not being part/border of a label - see Mesh::selectPolyNoLabel
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	return MeshGL::selectPolyNoLabel();
}

bool MeshQt::selectPolyRunLenGT() {
	//! Selects the largest polyline with a minimum run length (1)- see Mesh::selectPolyRunLenGT
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	double currVal;
	getParamFloatMesh( FUNC_VALUE_THRES, &currVal );
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Select polyline with a minimum length of:") );
	dlgEnterText.setDouble( currVal );
	QObject::connect( &dlgEnterText, SIGNAL(textEntered(double)), this, SLOT(selectPolyRunLenGT(double)) );
	if( dlgEnterText.exec() == QDialog::Accepted ) {
		return true;
	}
	return false;
}

bool MeshQt::selectPolyRunLenGT( double rValue ) {
	//! Selects the largest polyline with a minimum run length (2)- see Mesh::selectPolyRunLenGT
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	return MeshGL::selectPolyRunLenGT( rValue );
}

bool MeshQt::selectPolyRunLenLT() {
	//! Selects the largest polyline with a maximum run length (1)- see Mesh::selectPolyRunLenLT
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	double currVal;
	getParamFloatMesh( FUNC_VALUE_THRES, &currVal );
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Select polyline with a maximum length of:") );
	dlgEnterText.setDouble( currVal );
	QObject::connect( &dlgEnterText, SIGNAL(textEntered(double)), this, SLOT(selectPolyRunLenLT(double)) );
	if( dlgEnterText.exec() == QDialog::Accepted ) {
		return true;
	}
	return false;
}

bool MeshQt::selectPolyRunLenLT( double rValue ) {
	//! Selects the largest polyline with a maximum run length (2)- see Mesh::selectPolyRunLenLT
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	return MeshGL::selectPolyRunLenLT( rValue );
}

bool MeshQt::selectPolyLongest() {
	//! Selects the largest polyline - see Mesh::selectPolyLargest
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	return MeshGL::selectPolyLongest();
}

bool MeshQt::selectPolyShortest() {
	//! Selects the shortest polyline - see Mesh::selectPolyShortest
	cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	return MeshGL::selectPolyShortest();
}

//! Add polylines with given label numbers.
//! Method part 1 of 2 - UI
bool MeshQt::selectPolyLabelNo() {
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Enter label numbers") );
	dlgEnterText.fetchClipboard( QGMDialogEnterText::CHECK_INTEGER_MULTIPLE );
	QObject::connect( &dlgEnterText, SIGNAL(textEntered(vector<int>)), this, SLOT(selectPolyLabelNo(vector<int>)) );
	dlgEnterText.exec();
	return true;
}

//! Add polylines with given label numbers.
//! Method part 2 of 2 - Execute
bool MeshQt::selectPolyLabelNo( const vector<int>& rLabelNrs ) {
	return MeshGL::selectPolyLabelNo( rLabelNrs );
}

//! Add polylines with no relation to a label to the selection.
bool MeshQt::selectPolyNotLabeled() {
	return MeshGL::selectPolyNotLabeled();
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
//! Deselect vertices within a polygonal area defined by the camera view direction (=prism) .
bool MeshQt::deselectPoly( vector<QPoint> &rPixelCoords ) {
    vector<PixCoord> polyCoords;
    for( auto const& polyPoint: rPixelCoords ) {
        PixCoord someCoord;
        someCoord.x = polyPoint.x();
        someCoord.y = polyPoint.y();
        polyCoords.push_back( someCoord );
    }

    bool retVal = MeshGL::selectPoly( polyCoords, true );
    if( !retVal ) {
        SHOW_MSGBOX_WARN( tr("Deselection aborted"), tr("ERROR occured: No vertices were deselected!") );
    }
    return retVal;
}

//! Select vertices within a polygonal area defined by the camera view direction (=prism) .
bool MeshQt::selectPoly( vector<QPoint> &rPixelCoords ) {
	vector<PixCoord> polyCoords;
	for( auto const& polyPoint: rPixelCoords ) {
		PixCoord someCoord;
		someCoord.x = polyPoint.x();
		someCoord.y = polyPoint.y();
		polyCoords.push_back( someCoord );
	}
	bool retVal = MeshGL::selectPoly( polyCoords );
	if( !retVal ) {
		SHOW_MSGBOX_WARN( tr("Selection aborted"), tr("ERROR occured: No vertices were selected!") );
	}
	return retVal;
}

//! Selects ONE primitive of a certain type locate at the given pixel position.
Primitive* MeshQt::selectPrimitiveAt( int primitiveTypeToSelect, int xPixel, int yPixel, bool addToList ) {
	Primitive* primSelected = MeshGL::selectPrimitiveAt( primitiveTypeToSelect, xPixel, yPixel, addToList );
	if( primSelected == nullptr ) {
		emit sInfoMesh( MGLINFO_SELECTED_PRIMITIVE, "none" );
	} else {
		emit sInfoMesh( MGLINFO_SELECTED_PRIMITIVE, QString( primSelected->getTypeStr().c_str() ) );
	}
	// draw the selected pixel:
	emit primitiveSelected( primSelected );
	emit updateGL();
	return primSelected;
}

// Selection - Plane definition -----------------------------------------------------

//! Emit signal for guide, when the position ID for the next vertex defining a plane is requested.
bool MeshQt::getPlanePosToSet( int* rPosID ) {
	if( rPosID == nullptr ) {
		return false;
	}
	if( !MeshGL::getPlanePosToSet( rPosID ) ) {
		return false;
	}
	switch( (*rPosID) ) {
		case Plane::PLANE_VERT_A:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_PLANE_3FP_A );
			break;
		case Plane::PLANE_VERT_B:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_PLANE_3FP_B );
			break;
		case Plane::PLANE_VERT_C:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_PLANE_3FP_C );
			break;
		default:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_ERROR );
			cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: Invalid posID!" << endl;
			return false;
	}
	return true;
}

//! Emits a signal, when the third position of the plane was set.
//! And shows the distance to plane menu.
bool MeshQt::setPlanePos( Vector3D* rSomePos ) {
	if( !MeshGL::setPlanePos( rSomePos ) ) {
		return false;
	}
	emit updateGL();
	// Trigger signal for user guide by calling getPlanePosToSet
	int posID;
	if( !getPlanePosToSet( &posID ) )  {
		return false;
	}
	// Signal last position set:
	if( isPlanePosCSet() ) {
		Vector3D planeHNF;
		if( !Mesh::getPlaneHNF( &planeHNF ) ) {
			return false;
		}
		//planeHNF.dumpInfo();
		emit sSetPlaneHNF( planeHNF );
	}
	return true;
}

//! Transform the plane's position.
//! This slot is typically connected to the MeshWidget's signals handling mouse and keyboard events.
bool MeshQt::applyTransfromToPlane( Matrix4D transMat ) {
	if( !MeshGL::applyTransfromToPlane( transMat ) ) {
		return false;
	}
	emit updateGL();
	return true;
}

bool MeshQt::applyTransformation(Matrix4D rTrans, std::set<Vertex*>* rSomeVerts, bool rResetNormals)
{
	bool retValue = MeshGL::applyTransformation(rTrans, rSomeVerts, rResetNormals);

	emit sDefaultViewLightZoom();

	return retValue;
}

// Selection - Cone ------------------------------------------------------------

//! Overloaded function; will call Mesh::setConeAxis() and emit a signal to
//! redraw the mesh.
//!
//! Set selection mode to "Cone" to show the axis.
//!
//! @returns false in case of an error. True otherwise.
bool MeshQt::setConeAxis( const Vector3D& rUpper, const Vector3D& rLower ) {
	// Main: set the axis and check/return the result.
	if( !MeshGL::setConeAxis( rUpper, rLower ) ) {
		return( false );
	}
	setParamFlagMeshGL( MeshGLParams::SHOW_MESH_AXIS, true );
	emit updateGL();

	// Ask what to do next: Selection of a cone for rollouts OR postions for profile lines.
	bool rUserChoice;
	QString rHead = tr("Rollout or Profile lines");
	QString rMsg = tr("Choose the next step:\n\n");
	rMsg += tr("YES to continue with the cone selection for rollouts.\n\n");
	rMsg += tr("NO to continue with position selection for profile lines.\n\n");
	rMsg += tr("CANCEL to keep the current selection mode.");
	if( !showQuestion( &rUserChoice, rHead.toStdString(), rMsg.toStdString() ) ) {
		// User cancel.
		return( true );
	}

	// User choice:
	if( rUserChoice ) {
		mWidgetParams->setParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, MeshWidgetParams::SELECTION_MODE_CONE );
	} else {
		mWidgetParams->setParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, MeshWidgetParams::SELECTION_MODE_POSITIONS );
	}

	return( true );
}

//! Overloaded function; will call Mesh::setConeRadius() and emit a signal to
//! redraw the mesh.
bool MeshQt::setConeRadius(Vector3D& pointIntersect)
{
	if(!Mesh::setConeRadius(pointIntersect)) {
		return(false);
	}

	emit updateGL();

	return(true);
}

//! Emit signal for user guide
Mesh::coneStates MeshQt::getConeStatus() {
	coneStates stateID = Mesh::getConeStatus();
	switch( stateID ) {
		case CONE_UNDEFINED:
			emit sGuideIDCommon( MeshWidgetParams::GUIDE_COMMON_NONE );
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_CONE_AXIS );
		break;
		case CONE_DEFINED_AXIS:
			emit sGuideIDCommon( MeshWidgetParams::GUIDE_COMMON_NONE );
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_CONE_PR1 );
		break;
		case CONE_DEFINED_UPPER_RADIUS:
			emit sGuideIDCommon( MeshWidgetParams::GUIDE_COMMON_NONE );
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_CONE_PR2 );
		break;
		case CONE_DEFINED_LOWER_RADIUS:
			emit sGuideIDCommon( MeshWidgetParams::GUIDE_COMMON_CONE_DEFINED );
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_CONE_AXIS );
		break;
		case CONE_UNROLLED:
			//emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_CONE_UNROLLED );
		break;
		default:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_ERROR );
	}
	return stateID;
}

// Selection - Sphere ---------------------------------------------------------

//! Overloaded fucntion; will call Mesh::setSpherePoint() and emit a signal to
//! redraw the mesh.
//! Set prime meridian to 180 degree, which will center the slider for adjusting.
bool MeshQt::setSpherePoint(Vector3D &p, Vector3D &normal) {

	if(!MeshGL::setSpherePoint(p, normal)) {
		return(false);
	}
	getSpherePointIdx();
	setParamFloatMesh( AXIS_PRIMEMERIDIAN, M_PI );
	emit updateGL();
	return(true);
}

//! Returns the index of the current - out of four - points defining a sphere.
//! Emits signal for the user guide.
int MeshQt::getSpherePointIdx() {
	int pointIdx = MeshGL::getSpherePointIdx();
	switch( pointIdx ) {
		case 0:
			if( getSphereRadius() > 0.0 ) {
				emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SPHERE_P4 );
			} else {
				emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SPHERE_P0 );
			}
		break;
		case 1:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SPHERE_P1 );
		break;
		case 2:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SPHERE_P2 );
		break;
		case 3:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SPHERE_P3 );
		break;
		default:
			emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_ERROR );
	}
	return pointIdx;
}

// View actions ---------------------------------------------------------------

//! Call dialog to enter scale for polyline normals.
bool MeshQt::polylinesCurvScale() {
	double currVal;
	getParamFloatMeshGL( MeshGLParams::POLYLINE_NORMAL_SCALE, &currVal );
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Scale polyline normals:") );
	dlgEnterText.setDouble( currVal );
	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return false;
	}
	dlgEnterText.getText( &currVal );
	setParamFloatMeshGL( MeshGLParams::POLYLINE_NORMAL_SCALE, currVal );
	return true;
}

// Analyze----------------------------------------------------------------------------

//! Show dialog for MSII feature descriptor estimation and/or computation.
void MeshQt::estimateMSIIFeat() {
	QGMDialogMSII dialogMSII; //!< QDialog for parameters of the Mesh::fetchSphereVolume method.
	if( ( mPrimSelected != nullptr ) && ( mPrimSelected->getType() == Primitive::IS_VERTEX ) ) {
		dialogMSII.setVertex( static_cast<Vertex*>(mPrimSelected)->getIndex() );
	}
	QObject::connect( &dialogMSII, SIGNAL(estimateMSIIFeat(ParamsMSII)), this, SLOT(estimateMSIIFeat(ParamsMSII)) );
	dialogMSII.exec();
}

//! Handle a GUI request from QGMDialogMSII - Computes and/or estimates Mult-Scale Integral Invariants (MSII)
void MeshQt::estimateMSIIFeat( ParamsMSII params ) {
	// For performance evaluation:
	time_t timeStart = time( nullptr );
	// Find Vertex ...
	Vertex* someVertex;
	// ... by index or original index:
	if( params.IdIsOri ) {
		someVertex = getVertexByIdxOriginal( params.seedVertexId ) ;
	} else {
		someVertex = getVertexByIdx( params.seedVertexId ) ;
	}
	// when no Vertex was found we can't do anything and return
	if( someVertex == nullptr ) {
		emit statusMessage( "Fetch sphere volume failed: no vertex found." );
		cout << "[MeshQt::" << __FUNCTION__ << "] Vertex " << params.seedVertexId << " not found." << endl;
		return;
	}
	mPrimSelected = someVertex;

	timeStart = clock();
	// New 2.5D MSII/Dali-Filter:
	//---------------------------
	// Fetch neighbouring faces:
	set<Face*>    facesInSphereOLD;
	vector<Face*> facesInSphere;
	// Do not use, because of O(n^2):  set<Face*> facesInSphereOLD = fetchSphere( someVertex, params.radius, ... );
	// These are also slower
	//fetchSphereMarching( someVertex, &facesInSphere, radius );
	//fetchSphereMarchingDualFront( someVertex, &facesInSphere, params.radius );
	{ // ... than the bit array methods:
		// Allocate the bit arrays for vertices:
		uint64_t* vertBitArrayVisited;
		int vertNrLongs = getBitArrayVerts( &vertBitArrayVisited );
		// Allocate the bit arrays for faces:
		uint64_t* faceBitArrayVisited;
		int faceNrLongs = getBitArrayFaces( &faceBitArrayVisited );
		bool orderToFuncVal = false;
		// Choose one of the two methods:
		if( true ) {
			timeStart = clock();
			if( !fetchSphereBitArray( someVertex, &facesInSphere, params.radius, vertNrLongs, vertBitArrayVisited,
						  faceNrLongs, faceBitArrayVisited, orderToFuncVal ) ) {
				cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: fetchSphereBitArray FAILED!" << endl;
				return;
			}
			cout << "[MeshQt::" << __FUNCTION__ << "] fetchSphereBitArray: " << static_cast<float>( time( nullptr ) - timeStart ) << " seconds."  << endl;
		} else {
			timeStart = clock();
			// Will return a longer list of faces as it adds a 1-ring
			if( !fetchSphereBitArray1R( someVertex, facesInSphere, params.radius, vertNrLongs, vertBitArrayVisited,
			                            faceNrLongs, faceBitArrayVisited, orderToFuncVal ) ) {
				cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: fetchSphereBitArray1R FAILED!" << endl;
				return;
			}
			cout << "[MeshQt::" << __FUNCTION__ << "] fetchSphereBitArray1R: " << static_cast<float>( time( nullptr ) - timeStart ) << " seconds."  << endl;
		}
		delete[] vertBitArrayVisited;
		delete[] faceBitArrayVisited;
	}
	// Convert facesInSphere to facesInSphereOLD for backward compatibility:
	vector<Face*>::iterator itFace;
	for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
		facesInSphereOLD.insert( (*itFace) );
	}

	if( params.writeToMesh ) {
		Mesh tempMesh( &facesInSphereOLD );
		char fileNameStr[512];
		sprintf( fileNameStr, params.fileNameMesh.string().c_str(), params.seedVertexId );
		tempMesh.writeFile( string( fileNameStr ) );
	}
	if( params.dumpAsMatlabString ) {
		cout << "[MeshQt::" << __FUNCTION__ << "] faces_nr = " << facesInSphere.size() << endl;
	}
	// 2. Prepare raster (2.5D) array:
	double* rasterArray2 = new double[params.cubeEdgeLengthInVoxels*params.cubeEdgeLengthInVoxels];
	// 3. Raster faces:
	float  patchArea = fetchSphereCubeVolume25D( someVertex, &facesInSphere, params.radius, rasterArray2, params.cubeEdgeLengthInVoxels );
	if( !isnan( patchArea ) ) {
		if( params.writeRaster ) {
			char    fileNameStr[512];
            QString fileNamePattern = QString::fromStdWString( params.fileNameRaster.wstring() );
            fileNamePattern.replace( QString( ".png" ), QString( "_NEW.png" ) );
			sprintf( fileNameStr, fileNamePattern.toStdString().c_str(), params.seedVertexId );
			Image2D someImage;
			//someImage.setSilent();
            someImage.writePNG( fileNameStr, params.cubeEdgeLengthInVoxels, params.cubeEdgeLengthInVoxels, rasterArray2, -params.cubeEdgeLengthInVoxels/2.0, params.cubeEdgeLengthInVoxels/2.0, false );
		}
	} else {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: fetchSphereCubeVolume25D FAILED!" << endl;
	}
	if( params.dumpAsMatlabString ) {
		cout << "[MeshQt::" << __FUNCTION__ << "] patch_area = " << patchArea << " mm2" << endl;
	}
	// dump our raster as matlab string:
	if( false ) {
		cout << "rasterArray2 = [ " << endl;
		for( int i=0; i<params.cubeEdgeLengthInVoxels; i++ ) {
			for( int j=0; j<params.cubeEdgeLengthInVoxels; j++ ) {
				cout << " " << rasterArray2[i*params.cubeEdgeLengthInVoxels+j];
			}
			cout << ";" << endl;
		}
		cout << "];" << endl;
	}

	//for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
	//	cout << params.multiscaleRadii[i] << " ";
	//}
	//cout << endl;
	if( params.dumpAsMatlabString ) {
		cout << "[MeshQt::" << __FUNCTION__ << "] Radii r_rel = [";
		for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
			cout << " " << params.multiscaleRadii[i];
		}
		cout << " ];" << endl;
		cout << "[MeshQt::" << __FUNCTION__ << "] Radii r_abs = [";
		for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
			cout << " " << params.multiscaleRadii[i]*params.radius;
		}
		cout << " ];" << endl;
	}

	voxelFilter2DElements* sparseFilters;
	double** voxelFilters2D = generateVoxelFilters2D( params.multiscaleRadiiNr, params.multiscaleRadii, params.cubeEdgeLengthInVoxels, &sparseFilters );
	if( params.dumpAsMatlabString ) {
		cout << "[MeshQt::" << __FUNCTION__ << "] (new) Sphere Volumes vol_ref(" << params.seedVertexId << ",:) = [";
		for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
			double sum = 0.0;
			for( int j=0; j<params.cubeEdgeLengthInVoxels*params.cubeEdgeLengthInVoxels; j++ ) {
				if( std::isnan( voxelFilters2D[i][j] ) ) {
					//cout << __PRETTY_FUNCTION__ << " SUM_DOUBLE_ARRAY ignore Not-A-Number at index " << j << endl;
					continue;
				}
				sum += voxelFilters2D[i][j];
			}
			cout << " " << sum;
		}
		cout << " ]; " << endl;
	}
	if( params.writeFilterMasks ) {
		char    fileNameStr[512];
		for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
			sprintf( fileNameStr, params.fileNameFilterMasks.string().c_str(), params.seedVertexId, i, params.multiscaleRadii[i] );
			Image2D someImage;
			//someImage.setSilent();
            someImage.writePNG( fileNameStr, params.cubeEdgeLengthInVoxels, params.cubeEdgeLengthInVoxels, voxelFilters2D[i], 0.0, static_cast<double>(params.cubeEdgeLengthInVoxels)/2.0, false );
		}
	}

/*
	double* rasterArrayDBL = (double*) calloc( cubeEdgeLengthInVoxels*cubeEdgeLengthInVoxels, sizeof(double) );
	for( int i=0; i<cubeEdgeLengthInVoxels*cubeEdgeLengthInVoxels; i++ ) {
		rasterArrayDBL[i] = (double) rasterArray[i];
	}
*/
	double voxelFilterResults[19];
	//applyVoxelFilters2D( voxelFilterResults, rasterArray2, multiscaleRadiiSize, voxelFilters2D, cubeEdgeLengthInVoxels );
	applyVoxelFilters2D( voxelFilterResults, rasterArray2, &sparseFilters, params.multiscaleRadiiNr, params.cubeEdgeLengthInVoxels );
	if( params.dumpAsMatlabString ) {
		cout << "[MeshQt::" << __FUNCTION__ << "] (new) Feature Vector feat(" << params.seedVertexId << ",:) = [";
		for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
			cout << " " << voxelFilterResults[i];
		}
		cout << " ]; " << endl;
	}
	// Output string for volume descriptor
	QString volumeDescStr;
	for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
		volumeDescStr += QString( "&nbsp;%1" ).arg( voxelFilterResults[i], 0, 'f', 5, '0' );
	}

	if( params.writeFilterResult ) {
		double** filterResults2D = applyVoxelFilters2DImage( rasterArray2, params.multiscaleRadiiNr, voxelFilters2D, params.cubeEdgeLengthInVoxels );
		// dump our filter result as matlab string:
		if( false ) {
			cout << "filterResults2D{1} = [ " << endl;
			for( int i=0; i<params.cubeEdgeLengthInVoxels; i++ ) {
				for( int j=0; j<params.cubeEdgeLengthInVoxels; j++ ) {
					cout << " " << filterResults2D[0][i*params.cubeEdgeLengthInVoxels+j];
				}
				cout << ";" << endl;
			}
			cout << "];" << endl;
		}
		cout << "[MeshQt::" << __FUNCTION__ << "] Feature Vector (2Di) " << params.seedVertexId << ": ";
		char    fileNameStr[512];
		for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
			sprintf( fileNameStr, params.fileNameFilterResult.string().c_str(), params.seedVertexId, i, params.multiscaleRadii[i] );
			Image2D someImage;
            someImage.writePNG( fileNameStr, params.cubeEdgeLengthInVoxels, params.cubeEdgeLengthInVoxels, filterResults2D[i], 0.0, static_cast<double>(params.cubeEdgeLengthInVoxels), false );
			double sum = 0.0;
			for( int j=0; j<params.cubeEdgeLengthInVoxels*params.cubeEdgeLengthInVoxels; j++ ) {
				if( std::isnan( filterResults2D[i][j] ) ) { \
					cout << __PRETTY_FUNCTION__ << " SUM_DOUBLE_ARRAY ignore Not-A-Number at index " << j << endl;
				}
				sum += filterResults2D[i][j];
			}
			cout << " " << sum;
		}
		cout << endl;
	}

	// Compute and estimate Surface Integral Invariant of type A (II.SA)
	double* absolutRadii     = new double[params.multiscaleRadiiNr];
	double* surfaceAreasComp = new double[params.multiscaleRadiiNr];
	double* surfaceAreasEst  = new double[params.multiscaleRadiiNr];
	for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
		absolutRadii[i] = params.multiscaleRadii[i] * params.radius;
	}
	timeStart = clock();
	fetchSphereArea( someVertex, &facesInSphere, static_cast<unsigned int>(params.multiscaleRadiiNr), absolutRadii, surfaceAreasComp );
	cout << "[MeshQt::" << __FUNCTION__ << "] fetchSphereArea: " << static_cast<float>( time( nullptr ) - timeStart ) << " seconds."  << endl;
	timeStart = clock();
	fetchSphereAreaEst( someVertex, &facesInSphere, static_cast<unsigned int>(params.multiscaleRadiiNr), absolutRadii, surfaceAreasEst );
	cout << "[MeshQt::" << __FUNCTION__ << "] fetchSphereAreaEst: " << static_cast<float>( time( nullptr ) - timeStart ) << " seconds."  << endl;
	// Output string for surface descriptor
	QString surfaceDescStrComp;
	QString surfaceDescStrEst;
	for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
		surfaceDescStrComp += QString( "&nbsp;%1" ).arg( surfaceAreasComp[i], 0, 'f', 5, '0' );
		surfaceDescStrEst  += QString( "&nbsp;%1" ).arg( surfaceAreasEst[i], 0, 'f', 5, '0' );
	}

	//! \todo source clean-up and proper orientation of the box!
	//! \todo reactivate box - normalAverageInSphere was removed together with 2099 legacy code
	// draw a box:
//	float hnfD   = -compMult( normalAverageInSphere, someVertex->getPositionVector() ).sum3();
//	float xOther =  someVertex->getX() + 1.0;
//	float yOther =  someVertex->getY() + 1.0;
//	float zOther = -( normalAverageInSphere.getX()*xOther + normalAverageInSphere.getY()*yOther +hnfD ) / normalAverageInSphere.getZ();
//	// the result of the arbritary point minus the location vector is one of
//	// the planes vectprs_
//	Vector3D vecInPlaneDirection( xOther-someVertex->getX(), yOther-someVertex->getY(), zOther-someVertex->getZ() );
//	// by the cross product we get the second vector describing the plane
//	// and used to determine the cube inside the sphere for further processing:
//	Vector3D normalVec3 = normalAverageInSphere % vecInPlaneDirection;
//	normalAverageInSphere.setLength3( params.radius );
//	vecInPlaneDirection.setLength3( params.radius );
//	normalVec3.setLength3( params.radius );
//	RectBox* someBox = new RectBox( someVertex->getPositionVector(), normalAverageInSphere, vecInPlaneDirection, normalVec3 );
//	mDatumBoxes.push_back( someBox );

	// draw spheres:
	Sphere* someSphere;
	for( int i=0; i<params.multiscaleRadiiNr; i++ ) {
		double setSphereRadius = params.radius * params.multiscaleRadii[params.multiscaleRadiiNr-i-1];
		//float  sphereColHSV[3] = { multiscaleRadii[multiscaleRadiiSize-i-1], 1.0, 1.0 };
		unsigned char sphereColRGB[3];
		getColor( sphereColRGB, params.multiscaleRadiiNr-i-1, Primitive::COLMAP_BREWER_SET1 );
		//cout << i << ": " << multiscaleRadii[multiscaleRadiiSize-i-1] << "radius: " << setSphereRadius << endl;
		someSphere = new Sphere( someVertex->getCenterOfGravity(), setSphereRadius, sphereColRGB[0], sphereColRGB[1], sphereColRGB[2] );
		mDatumSpheres.push_back( someSphere );
	}

	// clean up memory:
	cout << "[MeshQt::" << __FUNCTION__ << "] before free" << endl;
	delete[] params.multiscaleRadii;
	cout << "[MeshQt::" << __FUNCTION__ << "] after free" << endl;

	// set up visualization
	//! \bug will eventuallsý crash here with stack smashing - reason currently unknown :( Looks like there is an infite loop.
	setParamFlagMeshGL( MeshGLParams::SHOW_DATUM_SPHERES, true );
	cout << "[MeshQt::" << __FUNCTION__ << "] after flags1" << endl;
	setParamFlagMeshGL( MeshGLParams::SHOW_DATUM_BOXES, true );
	cout << "[MeshQt::" << __FUNCTION__ << "] after flags2" << endl;

	// Display result:
	QString strInfo;
	strInfo = "<table>";
	strInfo += tr( "<tr><td>Vertex:</td><td>&nbsp;%1</td></tr>" ).arg( params.seedVertexId );
	strInfo += tr( "<tr><td>Volume,&nbsp;norm.</td><td><b>" ) + volumeDescStr + tr( "</b></td></tr>" );
	strInfo += tr( "<tr><td>Surface,&nbsp;norm.,&nbsp;comp.</td><td><b>" ) + surfaceDescStrComp + tr( "</b></td></tr>" );
	strInfo += tr( "<tr><td>Surface,&nbsp;norm.,&nbsp;est.</td><td><b>" ) + surfaceDescStrEst + tr( "</b></td></tr>" );
	strInfo += tr( "<tr><td>Patch area</td><td>&nbsp;%1&nbsp;mm2</td></tr>" ).arg( patchArea, 0, 'f', 3, '0' );
	strInfo += tr( "<tr><td>Face&nbsp;count</td><td>&nbsp;%1</td>" ).arg( facesInSphere.size() );
	strInfo += "</table>";
	SHOW_MSGBOX_INFO( tr( "GigaMesh - MSII for Vertex %1" ).arg( params.seedVertexId ), strInfo );
	cout << "[MeshQt::" << __FUNCTION__ << "] Done." << endl;
}

//! Estimate a geodesic patch for the selected vertices - see MeshGL::geodPatchVertSel().
bool MeshQt::geodPatchVertSel() {
	bool userCancel;
	bool useFuncValsAsWeigth = false;
	getParamFlagMesh( GEODESIC_USE_FUNCVAL_AS_WEIGHT, &useFuncValsAsWeigth );
	bool geodDistToFuncVal;
	SHOW_QUESTION( tr("Compute Geodesic distance"), tr( "Store the geodesic distance as function value per vertex" ), geodDistToFuncVal, userCancel );
	if( userCancel ) {
		return false;
	}
	return MeshGL::geodPatchVertSel( useFuncValsAsWeigth, geodDistToFuncVal );
}

//! Estimate a geodesic patch for the selected vertices in sequential order - see MeshGL::geodPatchVertSelOrder().
bool MeshQt::geodPatchVertSelOrder() {
	bool userCancel;
	bool useFuncValsAsWeigth = false;
	getParamFlagMesh( GEODESIC_USE_FUNCVAL_AS_WEIGHT, &useFuncValsAsWeigth );
	bool geodDistToFuncVal;
	SHOW_QUESTION( tr("Compute Geodesic distance"), tr( "Store the geodesic distance as function value per vertex" ), geodDistToFuncVal, userCancel );
	if( userCancel ) {
		return false;
	}
	return MeshGL::geodPatchVertSelOrder( useFuncValsAsWeigth, geodDistToFuncVal );
}

//! Fill holes (=closed polylines) using psalm - see Mesh::fillPolyLines.
bool MeshQt::fillPolyLines() {
	uint64_t holesFilled;
	uint64_t holesFail;
	uint64_t holesSkipped;
	bool retVal = MeshGL::fillPolyLines( holesFilled, holesFail, holesSkipped );
	if( retVal ) {
		SHOW_MSGBOX_INFO( tr("Holes filled"), tr( "%1 filled\n%2 failed to fill\n%3 skipped" ).arg( holesFilled ).arg( holesFail ).arg( holesSkipped ) );
	} else {
		SHOW_MSGBOX_WARN( tr("Holes filled - ERROR"), tr( "ERROR: libpsalm missing!" ) );
	}
	return retVal;
}

//! Retrieve and show the length of the polylines.
bool MeshQt::infoPolylinesLength() {
	QString strPolyLineInfo;
	vector<double> polyLens;
	for( auto const& currPoly: mPolyLines ) {
		double polyLineLen;
		if( !( currPoly->getLengthAbs( &polyLineLen ) ) ) {
			return false;
		}
		polyLens.push_back( polyLineLen );
	}
	std::sort( polyLens.begin(), polyLens.end() );
	std::reverse( polyLens.begin(), polyLens.end() );
	for( auto const& currPolyLen: polyLens ) {
		strPolyLineInfo += QString( "%1<br />" ).arg( currPolyLen );
	}
	SHOW_MSGBOX_INFO( tr("Length of the Polylines"), strPolyLineInfo );
	return true;
}

//! Hue estimation, which we will be assigned as function value.
bool MeshQt::hueToFuncVal() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( getHueValues( &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Hue value of the vertices RGB color" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return true;
	}
	emit statusMessage( "ERROR: Visualization of the Vertices Feature Vectors length (euclidean metric) failed!" );
	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Handle GUI request for labeling and optional removal of labels having less than nrFaces.
//! Same as calling MeshGL::labelNone and MeshGL::labelFaces - just a signal added,
int MeshQt::labelFaces() {
	int retVal;
	int nrFaces = 0;
	QGMDialogEnterText dlgEnterTxt;
	dlgEnterTxt.setWindowTitle( tr( "Number of faces to remove during labeling" ) );
	dlgEnterTxt.setInt( nrFaces );
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: user cancel!" << endl;
		return false;
	}
	if( !dlgEnterTxt.getText( &nrFaces ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: bad input!" << endl;
		return false;
	}

	retVal = MeshGL::labelFaces( nrFaces );
	emit statusMessage( "Labeling by Faces done." );
	return retVal;
}

void MeshQt::intersectSphere() {
	//! Intersect surface with a sphere centered at the selected vertex.
	if( mPrimSelected == nullptr ) {
		cerr << "[MeshQt::intersectSphere] ERROR: Intersection failed: No Vertex selected." << endl;
		emit statusMessage( "ERROR: Intersection failed: No Vertex selected." );
		return;
	}
	MeshGL::intersectSphere( static_cast<Vertex*>(mPrimSelected), 1.0 );
}

void MeshQt::labelSelectionToSeeds() {
	//! Stores the selected vertices as seeds for labeling - see MeshGL::labelSelectionToSeeds
	MeshGL::labelSelectionToSeeds();
}

bool MeshQt::labelVerticesEqualFV() {
	//! Labels vertices having the same function value - see MeshGL::labelSelectedVertices
	return MeshGL::labelVerticesEqualFV();
}

bool MeshQt::labelVerticesEqualRGB() {
    //! Labels vertices having the same RGB values - see MeshGL::labelSelectedVertices
    return MeshGL::labelVerticesEqualRGB();
}

//! Sets the selected vertices' label to background -- see MeshGL::labelSelMVertsToBack
bool MeshQt::labelSelMVertsToBack() {
	return MeshGL::labelSelMVertsToBack();
}

void MeshQt::convertSelectedVerticesToPolyline() {
	//! Handle GUI request: convert selected Vertices to (a) Polyline(s).
	MeshGL::convertSelectedVerticesToPolyline();
	if( mPolyLines.size() > 0 ) {
		emit hasElement( Primitive::IS_POLYLINE, true );
		emit statusMessage( "Converted selected Vertices to Polyline(s)." );
		return;
	}
	emit hasElement( Primitive::IS_POLYLINE, false );
	emit statusMessage( "ERROR: Converted selected Vertices to Polyline(s) failed!" );
}

void MeshQt::advancePolyThres() {
	//! Advance the polylines to the actual threshold.
	double funcValThres;
	getParamFloatMesh( MeshParams::FUNC_VALUE_THRES, &funcValThres );
	PolyLine* newPolyLine;
	vector<PolyLine*>::iterator itPolyLines;
	vector<PolyLine*> advancedPolyLines;
	for( itPolyLines=mPolyLines.begin(); itPolyLines!=mPolyLines.end(); itPolyLines++ ) {
		newPolyLine = (*itPolyLines)->advanceFuncValThres( funcValThres );
		if( newPolyLine != nullptr ) {
			advancedPolyLines.push_back( newPolyLine );
		}
	}
	removePolylinesAll();
	mPolyLines.swap( advancedPolyLines );
	polyLinesChanged();
	emit statusMessage( "advancePolyThres!" );
}

//! Compute the integral invariants and their extrema of the polylines based on the run-length.
//! See MeshGL::compPolylinesIntInvRunLen()
bool MeshQt::compPolylinesIntInvRunLen() {
	QGMDialogEnterText dlgEnterTextRadius;
	dlgEnterTextRadius.fetchClipboard( QGMDialogEnterText::CHECK_DOUBLE );
	dlgEnterTextRadius.setWindowTitle( tr("Radius for Integral Invariant") );
	if( dlgEnterTextRadius.exec() == QDialog::Rejected ) {
		return false;
	}
	double iiRadius;
	if( !dlgEnterTextRadius.getText( &iiRadius ) ) {
		return false;
	}
	QGMDialogComboBox dlgComboBox;
	dlgComboBox.setTextLabel( tr( "Run-length Integral Invariant Mode" ) );
	dlgComboBox.setWindowTitle( tr("Run-length Integral Invariant Mode") );
	//! \todo IMPROVE setp of the combobox with enumerators.
	dlgComboBox.addItem( tr( "Both directions" ),    QVariant( static_cast<int>(PolyLine::POLY_INTEGRAL_INV_BOTH) )     );
	dlgComboBox.addItem( tr( "Forward direction" ),  QVariant( static_cast<int>(PolyLine::POLY_INTEGRAL_INV_FORWARD) )  );
	dlgComboBox.addItem( tr( "Backward direction" ), QVariant( static_cast<int>(PolyLine::POLY_INTEGRAL_INV_BACKWARD) ) );
	if( dlgComboBox.exec() == QDialog::Rejected ) {
		return false;
	}
	PolyLine::ePolyIntInvDirection direction = static_cast<PolyLine::ePolyIntInvDirection>(dlgComboBox.getSelectedItem().toInt());
	bool retVal = MeshGL::compPolylinesIntInvRunLen( iiRadius, direction );
	return retVal;
}

//! Compute the integral invariants and their extrema of the polylines based on the angle.
//! See MeshGL::compPolylinesIntInvAngle()
bool MeshQt::compPolylinesIntInvAngle() {
	QGMDialogEnterText dlgEnterTextRadius;
	dlgEnterTextRadius.fetchClipboard( QGMDialogEnterText::CHECK_DOUBLE );
	dlgEnterTextRadius.setWindowTitle( tr("Radius for Integral Invariant") );
	if( dlgEnterTextRadius.exec() == QDialog::Rejected ) {
		return false;
	}
	double iiRadius;
	if( !dlgEnterTextRadius.getText( &iiRadius ) ) {
		return false;
	}
	bool retVal = MeshGL::compPolylinesIntInvAngle( iiRadius );
	return retVal;
}

//! Compute a curvature estimation and its extrema of the polylines. See MeshGL::getPolylineExtrema()
void MeshQt::getPolylineExtrema() {
	MeshGL::getPolylineExtrema();
}

//! Copy the polylines' normals to their vertices-
bool MeshQt::setPolylinesNormalToVert() {
	bool retVal = MeshGL::setPolylinesNormalToVert();
	return retVal;
}

//! Compute skeleton lines based on extracted ridge point vertices.
void MeshQt::createSkeletonLine() {

    //show option to user to choose between the original polylines with possible gaps
    //and the experimental filling of fully occupied triangles (see experimental part of implementation)
    QGMDialogComboBox dlgComboBox;
	dlgComboBox.setTextLabel( tr( "Connect fully occupied faces? (experimental)" ) );
	dlgComboBox.setWindowTitle( tr("Experimental Polyline Repair") );
	dlgComboBox.addItem( tr( "Keep Original Polyline" ),    QVariant( 0 )     );
	dlgComboBox.addItem( tr( "Connect Full Faces to Triangles" ),    QVariant( 1 )     );
    if( dlgComboBox.exec() == QDialog::Rejected ) {
        return;
    }

    MeshGL::createSkeletonLineInit( dlgComboBox.getSelectedItem().toInt() );

    setParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_SELECTION, true );
    //setParamFlagMeshGL( MeshGLParams::SHOW_FACES_EDGES, true );

    //activate Polylines
    if( mPolyLines.size() > 0 ) {
        emit hasElement( Primitive::IS_POLYLINE, true );
        emit statusMessage( "Converted selected Vertices to Polyline(s)." );
        return;
    }
    emit hasElement( Primitive::IS_POLYLINE, false );
    emit statusMessage( "ERROR: Converted selected Vertices to Polyline(s) failed!" );
}

//! Ask the user how many primitives (maximum) should be contained within a node (cube).
void MeshQt::generateOctree() {
    //obsolete: Octree generates automatically the face and the vertex octree
    /**
	// Ask user what kind of octree to be generated.
	QStringList Element;
	Element << QString("Vertex") << QString("Face");
	bool userChoiceValid = false;
	QString text = QInputDialog::getItem( nullptr, tr("Generate Octree"), tr("Element"), Element, 0, false, &userChoiceValid );
	if( !userChoiceValid ) {
		return;
	}
    **/
	// Choice: Vertex
    //if( text == "Vertex" ) {
		QGMDialogEnterText dlgenter;
		dlgenter.setInt( 500 ); //some useful default value should be set here e.g. 500
		dlgenter.setWindowTitle( tr("Set maximum number of vertices per cube") );
		if( dlgenter.exec() == QDialog::Accepted ) {
			int nrVertices;
			if( dlgenter.getText( &nrVertices ) && ( nrVertices > 0 ) ) {
				generateOctreeVertex( nrVertices );
			} else {
				SHOW_MSGBOX_WARN( tr("Wrong value"), tr("Wrong value entered!") );
			}
		}

    //obsolete:
    /**
	// Choice: Face
	if( text == "Face" ) {
		QGMDialogEnterText dlgenter;
		dlgenter.setInt( 1000 ); //some useful default value should be set here e.g. 1000
		dlgenter.setWindowTitle( tr("Set maximum number of faces per cube") );
		if( dlgenter.exec() == QDialog::Accepted ) {
			int nrVertices;
			if( dlgenter.getText( &nrVertices ) && ( nrVertices > 0 ) ) {
				generateOctreeFace( nrVertices );
			} else {
				SHOW_MSGBOX_WARN( tr("Wrong value"), tr("Wrong value entered!") );
			}
		}
	}
    **/
}

void MeshQt::generateOctreeVertex(int maxnr) {
    MeshGL::generateOctree(maxnr);
}

void MeshQt::detectselfintersections() {
	// Sanity check
    if( mOctree == nullptr ) {
        cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: No octree defined!" << endl;
        SHOW_MSGBOX_WARN( tr("Octree missing"), tr("No octree defined!") );
		return;
	}

    vector<Face*> intersectedFaces;
    mOctree->detectselfintersections(intersectedFaces);
    mFacesSelected.insert(intersectedFaces.begin(), intersectedFaces.end());
	selectedMFacesChanged();
}

void MeshQt::drawOctree() {

    //actionViewDatumBoxes is used to draw boxes of octree
    if( ! mMainWindow->actionViewDatumBoxes->isChecked() ) {
        mMainWindow->actionViewDatumBoxes->trigger();
    }

    /**
    QStringList Element;
    Element<< QString("Vertex") << QString("Face");
    bool ok=false;
    QString text =  QInputDialog::getItem(nullptr, tr("Draw Octree"),tr("Element"), Element, 0, false, &ok);

    if(!ok) return;
    **/

    if ( mOctree != nullptr ) {

        cout<<"OCTREE DRAW Vertex"<<endl;
        vector<Octnode*> nodelist;
        mOctree->getnodelist(nodelist,Octree::VERTEX_OCTREE);
        //mOctree->getleafnodes(nodelist);
        Vector3D cubeboxx(1.0, 0.0, 0.0);
        Vector3D cubeboxy(0.0, 1.0, 0.0);
        Vector3D cubeboxz(0.0, 0.0, 1.0);
        for (Octnode*& octnode : nodelist) {
            RectBox* someBox = new RectBox( octnode->mCube.mcenter, octnode->mCube.mscale * cubeboxx, octnode->mCube.mscale * cubeboxy, octnode->mCube.mscale * cubeboxz );
            mDatumBoxes.push_back( someBox );
        }


    }
    else cout<<"NO OCTREE CONSTRUCTED: DO THIS FIRST!"<<endl;

}

void MeshQt::removeOctreedraw() {
	
	for(RectBox* rectBoxPtr : mDatumBoxes)
	{
		delete rectBoxPtr;
	}
	
	mDatumBoxes.clear();
}

void MeshQt::deleteOctree() {
	removeOctreedraw();
    delete mOctree;
    mOctree = nullptr;
    cout<<"Octree Vertex deleted"<<endl;

    /**
	QStringList Element;
	Element<< QString("Vertex") << QString("Face")<< QString("both");
	bool ok=false;
	QString text =  QInputDialog::getItem(nullptr, tr("Delete Octree"),tr("Element"), Element, 0, false, &ok);

	if(!ok) return;

	if (text == "Vertex") {
		delete mOctree;
		mOctree = nullptr;
		cout<<"Octree Vertex deleted"<<endl;
	}
	if (text == "Face") {
		delete mOctreeface;
		mOctreeface = nullptr;
		cout<<"Octree Face deleted"<<endl;
	}
	if (text == "both") {
		delete mOctree;
		mOctree = nullptr;
		delete mOctreeface;
		mOctreeface = nullptr;
		cout<<"Octree Vertex deleted"<<endl;
		cout<<"Octree Face deleted"<<endl;
	}
    **/

}



// Ellipse --------------------------------------------------------------------------------------------------------

#ifdef ELLIPSENFIT

void MeshQt::startEllipseFit( QGMDialogFitEllipse* params __attribute__((unused)) ) {
	//! Handle a GUI request from QGMDialogFitEllipse.
	MeshGL::performEllipseFit();
}

void MeshQt::stepsEllipseFit( QGMDialogFitEllipse* params ) {
	//! Handle a GUI request from QGMDialogFitEllipse.
	MeshGL::setStepping(params->spinSteps);
}

void MeshQt::plainsEllipseFit( QGMDialogFitEllipse* params ) {
	//! Handle a GUI request from QGMDialogFitEllipse.
	MeshGL::setNumberOfCutplanes(params->spinPlains);
}

#endif

//! GUI Wrapper for MeshGL::getMeshVolumeDivergence
void MeshQt::estimateVolume() {
	double volumeDXYZ[3];
	bool numericError = false;
	if( !MeshGL::getMeshVolumeDivergence( volumeDXYZ[0], volumeDXYZ[1], volumeDXYZ[2] ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: estimateVolumeDivergence failed - probably due to zero area faces!" << endl;
		numericError = true;
	}
	cout << "[MeshQt::" << __FUNCTION__ << "] Volume dx: " << volumeDXYZ[0] << endl;
	cout << "[MeshQt::" << __FUNCTION__ << "] Volume dy: " << volumeDXYZ[1] << endl;
	cout << "[MeshQt::" << __FUNCTION__ << "] Volume dz: " << volumeDXYZ[2] << endl;
	QString msgStr = tr( "<p>The mesh has a volume of<br /><br />"
	                     "%1 <i>mm<sup>3</sup></i><br />"
	                     "%2 <i>mm<sup>3</sup></i><br />"
	                     "%3 <i>mm<sup>3</sup></i><br /><br />"
	                     " in respect to the derivatives in x, y and z.<br /><br />"
	                     "Unit of <i>mm</i> assumed.</p>" ).arg( volumeDXYZ[0], 10, 'f', 3 ).arg( volumeDXYZ[1], 10, 'f', 3 ).arg( volumeDXYZ[2], 10, 'f', 3 );
	if( numericError ) {
		msgStr += tr("<p><font color=\"red\"><b>A numeric error was encountered. <br />Probably due to zero area faces.</b></font></p>");
	}
	SHOW_MSGBOX_INFO( "Volume (dx,dy,dz)", msgStr );
	emit statusMessage( msgStr );
}

//! GUI Wrapper for MeshGL::compVolumePlane
void MeshQt::compVolumePlane() {
	double volumePlanePos;
	double volumePlaneNeg;
	bool numericError = false;
	if( !MeshGL::compVolumePlane( &volumePlanePos, &volumePlaneNeg ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: compVolumePlane failed - probably due to zero area faces!" << endl;
		numericError = true;
	}
	cout << "[MeshQt::" << __FUNCTION__ << "] Volume pos: " << volumePlanePos << endl;
	cout << "[MeshQt::" << __FUNCTION__ << "] Volume neg: " << volumePlaneNeg << endl;
	QString msgStr = tr( "<p>The mesh has a volume of %1 and %2 mm3 in respect to the plane.</p>" ).arg( volumePlanePos, 10, 'f', 3 ).arg( volumePlaneNeg, 10, 'f', 3 );
	if( numericError ) {
		msgStr += tr("<p><font color=\"red\"><b>A numeric error was encountered. <br />Probably due to zero area faces.</b></font></p>");
	}
	SHOW_MSGBOX_INFO( "Volume", msgStr );
	emit statusMessage( msgStr );
}

//! GUI Wrapper for interactive Mesh::getAxisFromCircleCenters
bool MeshQt::getAxisFromCircleCenters() { // <- cancel could be passed up
	bool userCancel;
	bool userAnswer;
	SHOW_QUESTION( tr("Compute Axis"), tr("Continue with computing an axis using the circle centers?"), userAnswer, userCancel );
	if( userCancel ) {
		return false;
	}
	return userAnswer;
}

//! GUI Wrapper for Mesh::convertBordersToPolylines
bool MeshQt::convertBordersToPolylines() {
	bool retVal = MeshGL::convertBordersToPolylines();
	if( retVal ) {
		emit hasElement( Primitive::IS_POLYLINE, true );
		emit statusMessage( "Convert mesh borders to Polyline(s) done." );
		return true;
	}
	emit statusMessage( "ERROR: Converted mesh borders to Polyline(s) failed!" );
	return false;
}

//! Handle GUI request: convert borders of labels to polylines.
void MeshQt::convertLabelBordersToPolylines() {
	MeshGL::convertLabelBordersToPolylines();
	if( mPolyLines.size() > 0 ) {
		emit hasElement( Primitive::IS_POLYLINE, true );
		emit statusMessage( "Converted label borders to Polyline(s)." );
		return;
	}
	emit hasElement( Primitive::IS_POLYLINE, false );
	emit statusMessage( "ERROR: Converted label borders to Polyline(s) failed!" );
}

// Parameters -------------------------------------------------------------------------------------------------

//! Set a Mesh parameter - \todo make more general
bool MeshQt::setParaSmoothLength() {
	double smoothLength;
	getParamFloatMesh( Mesh::SMOOTH_LENGTH, &smoothLength );
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Set smooth length to:") );
	dlgEnterText.setDouble( smoothLength );
	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return false;
	}
	dlgEnterText.getText( &smoothLength );
	setParamFloatMesh( Mesh::SMOOTH_LENGTH, smoothLength );
	return true;
}

// Generic - flags --------------------------------------------------------------------------------------------

//! Set flag controlling the display of Primitives, etc.
bool MeshQt::setParamFlagMeshGL( MeshGLParams::eParamFlag rShowFlagNr, bool rSetState ) {
	bool flagState;
	getParamFlagMeshGL( rShowFlagNr, &flagState );
	//  prevents some infinite loops!
	if( rSetState == flagState ) {
		return false;
	}
	MeshGL::setParamFlagMeshGL( rShowFlagNr, rSetState );
	emit showFlagState( rShowFlagNr, rSetState );
	emit updateGL();
	return true;
}

//! Inverese a flag controlling the display of Primitives, etc.
void MeshQt::toggleShowFlag( MeshGLParams::eParamFlag rShowFlagNr ) {
	bool flagState;
	getParamFlagMeshGL( rShowFlagNr, &flagState );
	flagState = not( flagState );
	MeshGL::setParamFlagMeshGL( rShowFlagNr, flagState );
	emit showFlagState( rShowFlagNr, flagState );
	emit updateGL();
}

//! Let the user set an integer parameter - see MeshGL::setParamInt
//! @returns false in case of an error or if the user choose 'cancel'.
bool MeshQt::setParamIntMeshGLDialog( MeshGLParams::eParamInt rParamID ) {
	int oldValue;
	if( !MeshGL::getParamIntMeshGL( rParamID, &oldValue ) ) {
		return false;
	}

	QGMDialogEnterText dlgEnterParam;
	dlgEnterParam.setWindowTitle( tr( "Enter value ID %1" ).arg( rParamID ) );
	dlgEnterParam.setInt( oldValue );
	if( dlgEnterParam.exec() == QDialog::Rejected ) {
		return false;
	}
	int newValue;
	if( !dlgEnterParam.getText( &newValue ) ) {
		return false;
	}

	return setParamIntMeshGL( rParamID, newValue );
}

//! Set an integer parameter - see MeshGL::setParamInt
bool MeshQt::setParamIntMeshGL( MeshGLParams::eParamInt rParamID, int rValue ) {
	//cout << "[MeshQt::" << __FUNCTION__ << "]" << endl;
	bool retVal = MeshGL::setParamIntMeshGL( rParamID, rValue );
	if( retVal ) {
		// to update the main window
		emit paramIntSet( rParamID, rValue );
	}

	emit updateGL();
	return retVal;
}

//! Offer a dialog to enter a parameter of MeshGL of type double.
//! \todo checkout http://doc.qt.nokia.com/4.6/qobject.html#Q_ENUMS for ID/int to name/string
bool MeshQt::setParamFloatMeshGLDialog( MeshGLParams::eParamFlt rParamID ) {
	if( rParamID == MeshGLParams::ISOLINES_DISTANCE ) {
		//! For ISOLINES_DISTANCE we can divert to a slider.
		double funcValMin;
		double funcValMax;
		getFuncValMinMaxUser( &funcValMin, &funcValMax );
		return setParamFloatMeshGLLimits( rParamID, 0.0f, funcValMax-funcValMin );
	}
	if( rParamID == MeshGLParams::ISOLINES_OFFSET ) {
		//! For ISOLINES_OFFSET we can divert to a slider.
		double isoDistance;
		getParamFloatMeshGL( MeshGLParams::ISOLINES_DISTANCE, &isoDistance );
		return setParamFloatMeshGLLimits( rParamID, -isoDistance, isoDistance );
	}

	//! All other parameters have to be entered using a textbox.
	double paramVal;
	getParamFloatMeshGL( rParamID, &paramVal );

	QGMDialogEnterText dlgEnterParam;
	dlgEnterParam.setWindowTitle( tr( "Set Parameter ID %1" ).arg( rParamID ) );
	dlgEnterParam.setID( rParamID );
	dlgEnterParam.setDouble( paramVal );
	if( dlgEnterParam.exec() == QDialog::Rejected ) {
		return false;
	}
	double newValue;
	if( !dlgEnterParam.getText( &newValue ) ) {
		return false;
	}
	setParamFloatMeshGL( rParamID, newValue );
	return true;
}

//! Wrapper function for use with dialogslider::connect.
bool MeshQt::setParamFloatMeshGLSlider( int rParamNr, double rValue ) {
	bool retVal = MeshGL::setParamFloatMeshGL( static_cast<MeshGLParams::eParamFlt>(rParamNr), rValue );
	emit updateGL();
	return retVal;
}

//! Let the user set floating point parameter values controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise i.e. user cancel.
bool MeshQt::setParamFloatMeshGLLimits( MeshGLParams::eParamFlt rParamID, double rMinValue, double rMaxValue ) {
	bool logScale = false;
	if( rParamID == MeshGLParams::FUNC_VALUE_LOG_GAMMA ) {
		//! For FUNC_VALUE_LOG_GAMMA switch to logarithmic scale.
		logScale = true;
	}

	double currValue;
	if( !getParamFloatMeshGL( rParamID, &currValue ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: Could not get the current value for '" << rParamID << "'!" << endl;
		return false;
	}

	QGMDialogSliderHD dlgSlider;
	dlgSlider.setLogarithmic( logScale );
	dlgSlider.setIdx( rParamID );
	dlgSlider.setMin( rMinValue );
	dlgSlider.setMax( rMaxValue );
	dlgSlider.setPos( currValue );
	dlgSlider.setWindowTitle( tr( "Floating point parameter %1 [%2,%3]" ).arg( rParamID ).arg( rMinValue ).arg( rMaxValue ) );
	//dlgSlider.suppressPreview();
	QObject::connect( &dlgSlider, &QGMDialogSliderHD::valuePreviewIdFloat, this, &MeshQt::setParamFloatMeshGLSlider );
	if( dlgSlider.exec() == QDialog::Rejected ) {
		return false;
	}
	double newValue;
	if( !dlgSlider.getValue( &newValue ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: Could not get the new value for '" << rParamID << "'!" << endl;
		return false;
	}
	bool retVal = MeshGL::setParamFloatMeshGL( rParamID, newValue );
	return retVal;
}

//! Overloaded from MeshParams::setParamIntMesh
bool MeshQt::setParamIntMesh( MeshParams::eParamInt rParam, int rValue ) {
	if( !MeshGL::setParamIntMesh( rParam, rValue ) ) {
		return false;
	}
	return true;
}

//! Offer a dialog to enter a parameter of Mesh of type double.
//! \todo implemnt name retrieval of the enumerator
bool MeshQt::setParamFloatMeshDialog( MeshParams::eParamFlt rParamID ) {
	// Here you can add min/max like used in MeshQt::setParamFloatMeshGL
	// Example:
	//..........................................................................
	// if( rParamID == MeshGLParams::ISOLINES_DISTANCE ) {
	//	//! For ISOLINES_DISTANCE we can divert to a slider.
	//	double funcValMin;
	//	double funcValMax;
	//	getFuncValMinMaxUser( &funcValMin, &funcValMax );
	//	return setParamFloatMeshGL( rParamID, 0.0f, funcValMax-funcValMin );
	//}

	//! All other parameters have to be entered using a textbox.
	double paramVal;
	getParamFloatMesh( rParamID, &paramVal );

	QGMDialogEnterText dlgEnterParam;
	dlgEnterParam.setWindowTitle( tr( "Set Parameter ID %1" ).arg( rParamID ) );
	dlgEnterParam.setID( rParamID );
	dlgEnterParam.setDouble( paramVal );
	if( dlgEnterParam.exec() == QDialog::Rejected ) {
		return false;
	}
	double newValue;
	if( !dlgEnterParam.getText( &newValue ) ) {
		return false;
	}
	setParamFloatMesh( rParamID, newValue );
	return true;
}

//! Wrapper function for use with dialogslider::connect.
bool MeshQt::setParamFloatMeshSlider( int rParamNr, double rValue ) {
	bool retVal = MeshGL::setParamFloatMesh( static_cast<MeshParams::eParamFlt>(rParamNr), rValue );
	emit updateGL();
	return retVal;
}

//! Let the user set floating point parameter values controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise i.e. user cancel.
bool MeshQt::setParamFloatMeshLimits( MeshParams::eParamFlt rParamID, double rMinValue, double rMaxValue ) {
	double currValue;
	if( !getParamFloatMesh( rParamID, &currValue ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: Could not get the current value for '" << rParamID << "'!" << endl;
		return false;
	}

	// Special cases e.g. use degree instead of radiant.
	QString unitText = "";
	double setFactor = 1.0;
	bool   logScale  = false;
	int    steps     = 1000; // Default in QGMDialogSliderHD
	if( rParamID == AXIS_PRIMEMERIDIAN ) {
		steps     = 3600;
		setFactor = 180.0/M_PI;
		unitText = QString::fromUtf8( "\u00B0" ); // Degree symbol -- see: http://www.fileformat.info/info/unicode/char/b0/index.htm
	}

	QGMDialogSliderHD dlgSlider;
	dlgSlider.setFactor( setFactor );
	dlgSlider.setLogarithmic( logScale );
	dlgSlider.setIdx( rParamID );
	dlgSlider.setMin( rMinValue );
	dlgSlider.setMax( rMaxValue );
	dlgSlider.setPos( currValue );
	dlgSlider.setSteps( steps );
	dlgSlider.setWindowTitle( tr( "Floating point parameter %1 [%2,%3%4]" ).arg( rParamID ).arg( rMinValue*setFactor ).arg( rMaxValue*setFactor ).arg( unitText ) );
	//dlgSlider.suppressPreview();
	QObject::connect( &dlgSlider, &QGMDialogSliderHD::valuePreviewIdFloat, this, &MeshQt::setParamFloatMeshSlider );
	if( dlgSlider.exec() == QDialog::Rejected ) {
		return false;
	}
	double newValue;
	if( !dlgSlider.getValue( &newValue ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: Could not get the new value for '" << rParamID << "'!" << endl;
		return( false );
	}

	bool retVal = MeshGL::setParamFloatMesh( rParamID, newValue );
	return( retVal );
}


// --- Function/Method calls -----------------------------------------------------------------------------------------------------------------------------------

//! Overloaded from Mesh::callFunction
bool MeshQt::callFunctionMesh( MeshParams::eFunctionCall rFunctionID, bool rFlagOptional ) {
	bool retVal = true;

	// Prepare addtional information before execution:
	if( rFunctionID == AXIS_ENTER_PRIMEMERIDIAN_ROTATION ) {
		retVal &= setParamFloatMeshLimits( MeshParams::AXIS_PRIMEMERIDIAN, 0.0, 2.0*M_PI );
	}

	// Execute:
	retVal &= MeshGLShader::callFunction( rFunctionID, rFlagOptional );

	// Post-execution, when successfull:
	switch( rFunctionID )
	{
		case SELMPRIMS_POS_CIRCLE_CENTERS:
		    {
			    bool userAnswer;
				bool userCancel;
				SHOW_QUESTION( tr("Adapt View to Rotational Axis"),
							   tr("Change the view according to the rotational axis?"), userAnswer, userCancel );
				if( userAnswer && !userCancel ) {
					mMainWindow->actionViewAxisUp->trigger();
				}
		    }
			break;
		case EXPORT_COORDINATES_OF_VERTICES:
			    retVal = exportVertexCoordinatesToCSVDialog(false);
			break;
		case EXPORT_COORDINATES_OF_SELECTED_VERTICES:
			    retVal = exportVertexCoordinatesToCSVDialog(true);
			break;
		case EXPORT_SELPRIMS_POSITIONS:
			    retVal = exportSelPrimsPositionsToCSVDialog();
			break;

		default:
			break;
	}

	emit updateGL();


	return retVal;
}

//! Overloaded from Mesh::callFunction
bool MeshQt::callFunctionMeshGL( MeshGLParams::eFunctionCall rFunctionID, bool rFlagOptional ) {
	bool retVal = true;


	switch( rFunctionID )
	{
		case IMPORT_COORDINATES_OF_VERTICES:
		{
			retVal = importVertexDataFromCSVDialog([this](){
				        estBoundingBox();

						std::cout << "[MeshQtCSVImportExport::" << __FUNCTION__ << "] "
						            <<"Bounding box is now: "
						            << mMaxX - mMinX
						            << " x " << mMaxY - mMinY
						            << " x " << mMaxZ - mMinZ
						            << " mm (unit assumed)."
						            << std::endl;
			});

			resetVertexNormals();
			changedMesh();
			changedVertFuncVal();
			setParamIntMeshGL(MeshGLParams::TEXMAP_CHOICE_FACES,
			                       MeshGLParams::TEXMAP_VERT_FUNCVAL);

		}
			break;
		case RUN_TPS_RPM_TRANSFORMATION:
			showRunTpsRpmScriptDialog(nullptr, this);
			break;
		default:
			retVal &= MeshGLShader::callFunction( rFunctionID, rFlagOptional );
			break;
	}
	return retVal;
}

// --- Color settings ------------------------------------------------------------------------------------------------------------------------------------------

//! Calls QColorDialog to set a color-
void MeshQt::selectColor( MeshGLColors::eColorSettings rColorId ) {
	// Colors
	GLfloat colorTmpStored[4];
	mRenderColors->getColorSettings( rColorId, colorTmpStored );
	QColor colorTmp;
	colorTmp.setRgbF( colorTmpStored[0], colorTmpStored[1], colorTmpStored[2], colorTmpStored[3] );
	QColor colorBackFaceDefault( 128, 92, 92, 255 ); // See MeshGLColors::MeshGLColors()

	// Color Dialog
	QColorDialog colorDialog( colorTmp );
	colorDialog.setCurrentColor( colorTmp );
	colorDialog.setCustomColor( 0, colorBackFaceDefault );
	colorDialog.setWindowTitle( tr("Choose color") );
	colorDialog.setOption( QColorDialog::DontUseNativeDialog, false ); // This option appears to be interesting on Mac builds!
	colorDialog.setOption( QColorDialog::ShowAlphaChannel, true );
	if( colorDialog.exec() != QDialog::Accepted ) {
		return;
	}
	QColor newColor = colorDialog.selectedColor();
	GLfloat newColorToStore[4] = { static_cast<float>(newColor.redF()), static_cast<float>(newColor.greenF()), static_cast<float>(newColor.blueF()), static_cast<float>(newColor.alphaF()) };
	mRenderColors->setColorSettings( rColorId, newColorToStore );
	glPaint();
}

// --- Visualize -----------------------------------------------------------------------------------------------------------------------------------------------

//! Estimate the length of the primitives/Vertex feature vector using euclidean metric.
void MeshQt::visualizeFeatLengthEuc() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureVecLenEucVertex( &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Absolute lengths of the Vertices Feature Vectors (euclidean metric)" );
		setParamIntMeshGL( TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of the Vertices Feature Vectors length (euclidean metric) failed!" );
}

//! Estimate the length of the primitives/Vertex feature vector using manhattan metric.
void MeshQt::visualizeFeatLengthMan() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureVecLenManVertex( &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Absolute lengths of the Vertices Feature Vectors (manhattan metric)" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of the Vertices Feature Vectors length (manhattan metric) failed!" );
}

//! Computes the Bounded Variation (BV) of the feature vectors.
void MeshQt::visualizeFeatBVFunc() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureVecBVFunc( &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Absolute lengths of the Vertices Feature Vectors (manhattan metric)" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of the Vertices Feature Vectors length (manhattan metric) failed!" );
}

//! Computes the Total Variation (TV) of the feature vectors.
void MeshQt::visualizeFeatTVSeqn() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureVecTVSeqn( &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Absolute lengths of the Vertices Feature Vectors (manhattan metric)" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of the Vertices Feature Vectors length (manhattan metric) failed!" );
}

//! Estimate the euclidean distance to a selected primitives feature vector.
void MeshQt::visualizeFeatDistSelEuc() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureDistEucToVertex( getPrimitiveSelected(), &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Euclidean distances to the selected Vertex Feature Vector" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of euclidean distance to Vertex Features failed!" );
}

//! Estimate the euclidean distance to a selected primitives feature vector.
void MeshQt::visualizeFeatDistSelEucNorm() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureDistEucNormToVertex( getPrimitiveSelected(), &funcValues, &vertices, &vertCount ) ) {
		setVertexFuncValues( vertices, funcValues, vertCount, "Normalized euclidean distances to the selected Vertex Feature Vector" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of normalized euclidean distance to Vertex Features failed!" );
}

//! Estimate the manhattan distance to a selected primitives feature vector.
void MeshQt::visualizeFeatDistSelMan() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureDistManToVertex( getPrimitiveSelected(), &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Manhattan distances to the selected Vertex Feature Vector" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of manhattan distance to Vertex Features failed!" );
}

//! Compute the Cosine Similarity to a feature vector of a selected primitive.
void MeshQt::visualizeFeatureCosineSimToVertex() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureCosineSimToVertex( getPrimitiveSelected(), &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Cosine Similarity to the selected Vertex Feature Vector" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of Cosine Similarity to Vertex Features failed!" );
}

//! Compute the Tanimoto distance to a feature vector of a selected primitive.
void MeshQt::visualizeFeatureTanimotoDistTo() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( estFeatureTanimotoDistTo( getPrimitiveSelected(), &funcValues, &vertices, &vertCount ) ){
		setVertexFuncValues( vertices, funcValues, vertCount, "Tanimoto distance to the selected Vertex Feature Vector" );
		setParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
		delete[] vertices;
		delete[] funcValues;
		return;
	}
	emit statusMessage( "ERROR: Visualization of the Tanimoto distance to Vertex Features failed!" );
}

//! Estimate Auto-Correlation for Vertex Features - see MeshGL::estFeatureAutoCorrelationVertex
void MeshQt::visualizeFeatAutoCorrVert() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( !estFeatureAutoCorrelationVertex( &funcValues, &vertices, &vertCount ) ) {
		SHOW_MSGBOX_CRIT( tr("Function not available"), tr("ALGLIB library missing.") );
		emit statusMessage( "ERROR: Visualization of Auto-Correlation of Vertex Features failed!" );
		return;
	}
	setVertexFuncValues( vertices, funcValues, vertCount, "Auto-Correlation of Vertex Features" );
	setParamIntMeshGL( TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
	delete[] vertices;
	delete[] funcValues;
}

//! Estimate Correlation to the selected Vertex - see MeshGL::estFeatureCorrelationVertex
void MeshQt::visualizeFeatCorrSelectedVert() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( !estFeatureCorrelationVertex( getPrimitiveSelected(), &funcValues, &vertices, &vertCount ) ) {
		SHOW_MSGBOX_CRIT( tr("Function not available"), tr("ALGLIB library missing.") );
		emit statusMessage( "ERROR: Visualization of Correlation of Vertex Features failed!" );
		return;
	}
	setVertexFuncValues( vertices, funcValues, vertCount, ( QString("Correlation of Vertex Features to ")+QString(getPrimitiveSelected()->getIndex()) ).toStdString() );
	setParamIntMeshGL( TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
	delete[] vertices;
	delete[] funcValues;
}

//! Multiply the function value with a scalar entered by the user.
//! See MeshGL::setVertFuncValMult
bool MeshQt::setVertFuncValMult() {
	QGMDialogEnterText dlgEnterTextFeatVec;
	dlgEnterTextFeatVec.fetchClipboard( QGMDialogEnterText::CHECK_DOUBLE );
	dlgEnterTextFeatVec.setWindowTitle( tr("Scalar value for multplication with the vertices' function values") );
	if( dlgEnterTextFeatVec.exec() == QDialog::Rejected ) {
		return false;
	}
	double skalarToMult;
	if( !dlgEnterTextFeatVec.getText( &skalarToMult ) ) {
		return false;
	}
	return MeshGL::setVertFuncValMult( skalarToMult );
}

//! Applies autocorrelation and correlation to selection: log(|Autocrr|*100000) + |FTcorr| + 0.5 cutting of below 0.0 and above 1.0
void MeshQt::visualizeFeatAutoCorrSelectedVert() {
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	if( !estFeatureAutoCorrelationVertex( getPrimitiveSelected(), &funcValues, &vertices, &vertCount ) ) {
		SHOW_MSGBOX_CRIT( tr("Function not available"), tr("ALGLIB library missing.") );
		emit statusMessage( "ERROR: Visualization of Auto-Correlation and Correlation of Vertex Features failed!" );
		return;
	}
	setVertexFuncValues( vertices, funcValues, vertCount, ( QString("Auto-Correlation and Correlation of Vertex Features to ")+QString(getPrimitiveSelected()->getIndex()) ).toStdString() );
	setParamIntMeshGL( TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
	delete[] vertices;
	delete[] funcValues;
}

//! Set flags for plane dialog box.
void MeshQt::visualizeDistanceToPlane() {
	setPlaneHNF( false, true, false );
}

//! Compute and visualize the distances of the vertices to a plane given in Hesse form (Hessische Normalform - HNF).
void MeshQt::visualizeDistanceToPlane( Vector3D rPlaneHNF, bool rAbsDist ) {
	funcVertDistanceToPlane( rPlaneHNF, rAbsDist );
	// For isoline along the cutting plane
	setParamFloatMesh( FUNC_VALUE_THRES, 0.0 );
}

void MeshQt::visualizeVertexIndices() {
	//! Applies vertex indices as colors
	int      vertCount  = 0;
	Vertex** vertices   = nullptr;
	double*  funcValues = nullptr;
	getVertIndices( &funcValues, &vertices, &vertCount );
	if( setVertexFuncValues( vertices, funcValues, vertCount, "Vertex Indices" ) ) {
		setParamIntMeshGL( TEXMAP_CHOICE_FACES, TEXMAP_VERT_FUNCVAL );
	}
}

//! Compute r_min_i for 1-rings - see Mesh::funcVert1RingRMin
bool MeshQt::funcVert1RingRMin() {
	return MeshGL::funcVert1RingRMin();
}

//! Compute volume integral for a 1-ring, which is equivalent to V(r->0)
bool MeshQt::funcVert1RingVolInt() {
	return MeshGL::funcVert1RingVolInt();
}

void MeshQt::visualizeVertexOctree() {
	//! Visualize vertices related to octree nodes.
	//! (1/2) Dialog.
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Edge length") );
	dlgEnterText.setDouble( 1.0 );
	QObject::connect( &dlgEnterText, SIGNAL(textEntered(double)), this, SLOT(visualizeVertexOctree(double)) );
	dlgEnterText.exec();
}

void MeshQt::visualizeVertexOctree( double rEdgeLen ) {
	//! Visualize vertices related to octree nodes.
	//! (2/2) Execute.
	setVertFuncValOctreeIdx( rEdgeLen );
}

void MeshQt::visualizeVertexFaceSphereAngleMax() {
	//! Visualize the maximum face angle to the vertex normal within a spherical neighbourhood.
	//! (1/2) Dialog.
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Sphere radius") );
	dlgEnterText.setDouble( 1.0 );
	QObject::connect( &dlgEnterText, SIGNAL(textEntered(double)), this, SLOT(visualizeVertexFaceSphereAngleMax(double)) );
	dlgEnterText.exec();
}

void MeshQt::visualizeVertexFaceSphereAngleMax( double rRadius ) {
	//! Visualize the maximum face angle to the vertex normal within a spherical neighbourhood.
	//! (2/2) Execute.
	//! Measures and shows elapsed time.
	QElapsedTimer timer;
	timer.start();
	setVertFuncValFaceSphereAngleMax( rRadius );
	SHOW_MSGBOX_INFO( tr("Function values computed"), tr( "Took %1 milliseconds." ).arg( timer.elapsed() ) );
}

void MeshQt::visualizeVertFaceSphereMeanAngleMax() {
	//! Visualize the max face angle to the mean normal of the faces within a spherical neighbourhood.
	//! (1/2) Dialog.
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Sphere radius") );
	dlgEnterText.setDouble( 1.0 );
	QObject::connect( &dlgEnterText, SIGNAL(textEntered(double)), this, SLOT(visualizeVertFaceSphereMeanAngleMax(double)) );
	dlgEnterText.exec();
}

void MeshQt::visualizeVertFaceSphereMeanAngleMax( double rRadius ) {
	//! Visualize the max face angle to the mean normal of the faces within a spherical neighbourhood.
	//! (2/2) Execute.
	//! Measures and shows elapsed time.
	QElapsedTimer timer;
	timer.start();
	setVertFuncValFaceSphereMeanAngleMax( rRadius );
	SHOW_MSGBOX_INFO( tr("Function values computed"), tr( "Took %1 milliseconds." ).arg( timer.elapsed() ) );
}


// --- Colorramp------------------------------------------------------------------------------------------------------------------------------------------------

//! Visualizes the distance of each vertex to a cone selected by the user.
//! If no cone has been selected, the user is notified.
void MeshQt::visualizeDistanceToCone( bool rAbsDist ) {
	// TODO: Flag `absDist` is currently not set -- this should
	//       be changed in the caller function
    if(!(this->getConeStatus() == CONE_DEFINED_LOWER_RADIUS)) {
        emit statusMessage("No cone selected. Use \"Select\" menu to select a cone.");
        return;
    }
	setVertFuncValDistanceToCone( rAbsDist );
}


// Extra menu --------------------------------------------------------------------------------------------------------------------------------------------------

//! Edit the MetaData.
//!
//! @returns false in case of an error or user cancel.
bool MeshQt::editMetaData() {
	//! .) Edit Model ID.
	std::string modelID = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
	if( modelID.empty() ) {
		// Prepare suggestion
		QString suggestId( QString::fromStdWString(getBaseName().wstring()) );
		std::cout << "[MeshQt::" << __FUNCTION__ << "] Basename: " << suggestId.toStdString().c_str() << std::endl;
		suggestId.replace( "_", " " );
		suggestId.replace( QRegularExpression( "GM[oOcCfFpPxX]*$" ), "" );
		std::cout << "[MeshQt::" << __FUNCTION__ << "] Suggest Id: " << suggestId.toStdString().c_str() << std::endl;
		// Show dialog
		QGMDialogEnterText dlgEnterTxt;
		dlgEnterTxt.setText( suggestId );
		dlgEnterTxt.setWindowTitle( tr("Model ID") );
		if( dlgEnterTxt.exec() == QDialog::Rejected ) {
			std::cout << "[MeshQt::" << __FUNCTION__ << "] CANCELED by USER!" << std::endl;
			return( false );
		}
		QString newModelId;
		if( !dlgEnterTxt.getText( &newModelId ) ) {
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: bad input (1)!" << std::endl;
			return( false );
		}
		getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_ID, newModelId.toStdString() );
	}

	//! .) Edit Model Material.
	std::string modelMaterial = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
	if( modelMaterial.empty() ) {
		QGMDialogEnterText dlgEnterTxt;
		dlgEnterTxt.setText( tr( "original, clay" ) );
		dlgEnterTxt.setWindowTitle( tr("Model Material") );
		if( dlgEnterTxt.exec() == QDialog::Rejected ) {
			std::cout << "[MeshQt::" << __FUNCTION__ << "] CANCELED by USER!" << std::endl;
			return( false );
		}
		QString newMaterial;
		if( !dlgEnterTxt.getText( &newMaterial ) ) {
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: bad input (1)!" << std::endl;
			return( false );
		}
		getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_MATERIAL, newMaterial.toStdString() );
	}

	//! .) Edit Model Unit.

	std::string modelUnit = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_UNIT );
	std::string modelUnitLabel;
	getModelMetaDataRef().getModelMetaStringLabel( ModelMetaData::META_MODEL_UNIT, modelUnitLabel );
	if( modelUnit.empty() ) {
		//getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_UNIT, "mm" );
		QGMDialogEnterText dlgEnterTxt;
		dlgEnterTxt.setText( tr( "mm" ) );
		dlgEnterTxt.setWindowTitle( tr("Model Unit") );
		if( dlgEnterTxt.exec() == QDialog::Rejected ) {
			std::cout << "[MeshQt::" << __FUNCTION__ << "] CANCELED by USER!" << std::endl;
			return( false );
		}
		QString newUnit;
		if( !dlgEnterTxt.getText( &newUnit ) ) {
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: bad input (1)!" << std::endl;
			return( false );
		}
		getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_UNIT, newUnit.toStdString() );
    }

	return( true );
}

//! Print LaTeX string for cuneiform tablet figures.
void MeshQt::cuneiformFigureLaTeX() {

	if( !editMetaData() ) {
		return;
	}

	//! .) Fetch strings and their values.
	vector<pair<string,string> > replacmentStrings;
	latexFetchFigureInfos( &replacmentStrings );

	//! .) Fetch template(s).
	QFile latexTemplateFile( ":/GMGeneric/latextemplates/manual_report.tex" );
	if( !latexTemplateFile.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] Error: could not open file!" << endl;
		return;
	}
	QTextStream latexTemplateFileInStream( &latexTemplateFile );
	QString fileContent;
	while( !latexTemplateFileInStream.atEnd() ) {
		fileContent += latexTemplateFileInStream.readLine() + "\r\n";
	}
	//cout << "[MeshQt::" << __FUNCTION__ << "] File: " << fileContent.toStdString() << endl;
	//! .) Replace place holder with values.
	for(pair<string, string>& replacmentString : replacmentStrings) {
		string placeHolder = replacmentString.first;
		string content = replacmentString.second;
		fileContent.replace( QString( placeHolder.c_str() ), QString( content.c_str() ) );
	}
	//cout << "[MeshQt::" << __FUNCTION__ << "] File: " << fileContent.toStdString() << endl;

	//! .) Fetch info
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText( fileContent );
	SHOW_MSGBOX_INFO( tr("LaTeX String - Already copied to Clipboard"), fileContent );
}

//! Print Matlab string for face normals of the selection of faces.
void MeshQt::matlabFaceNormalsSel() {
	string matlabStr;
	dumpMatlabFaceNormalsSel( &matlabStr );
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText( QString( matlabStr.c_str() ) );
	SHOW_MSGBOX_INFO( tr("Matlab String - Already copied to Clipboard"), QString( matlabStr.c_str() ) );
}

// --- Extra menu - Show Information ---------------------------------------------------------------------------------------------------------------------------

//! Show information about the current selection.
//!
//! @returns false in case of an error. True otherwise.
bool MeshQt::showInfoSelectionHTML() {
	if( mPrimSelected == nullptr ) {
		SHOW_MSGBOX_WARN( tr("Information about SelPrim"), tr("Nothing selected!") );
		return true;
	}

	// Feature vector, when present:
	const int primFTVecLen = mPrimSelected->getFeatureVectorLen();
	QString strFeatVec( tr( "<tr><td>Feature&nbsp;Vector</td><td>none</td><tr>" ) );
	if( primFTVecLen > 0 ) {
		double* featVec = new double[primFTVecLen];
		mPrimSelected->copyFeatureVecTo( featVec );
		strFeatVec = tr( "<tr><td>Feature&nbsp;Vector</td><td>" );
		for( int i=0; i<primFTVecLen; i++ ) {
			strFeatVec += QString( "%1 " ).arg( featVec[i] );
		}
		strFeatVec += QString( "</td><tr>" );

		delete[] featVec;
	}

	if( mPrimSelected->getType() == IS_VERTEX ) {
		// Stored infomration
		Vertex* vertSel = static_cast<Vertex*>(mPrimSelected);
		uint64_t vertLabel;
		vertSel->getLabel( vertLabel );
		double vertFuncVal;
		vertSel->getFuncValue( &vertFuncVal );
		uint64_t vertFlags;
		vertSel->getFlagAll( &vertFlags );
		Vector3D vertNorm = vertSel->getNormal( false );
		// Computed information
		double distanceToAxis = _NOT_A_NUMBER_DBL_;
		double angleInAxisCoord = _NOT_A_NUMBER_DBL_;
		Vector3D axisTop;
		Vector3D axisBottom;
		if( getConeAxis( &axisTop, &axisBottom ) ) {
			distanceToAxis = vertSel->distanceToLine( &axisTop, &axisBottom );
			angleInAxisCoord = vertSel->angleInLineCoord( &axisTop, &axisBottom );
		}
		// Prepare HTML:
		QString strMsg;
		strMsg += QString( "<table>" );
		strMsg += QString( "<tr>" );
		strMsg += tr( "<td>Coordinates:</td><td>%1&nbsp;%2&nbsp;%3</td>" ).arg( vertSel->getX() ).arg( vertSel->getY() ).arg( vertSel->getZ() );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>Normal (eucl.):</td><td>%1&nbsp;%2&nbsp;%3</td>" ).arg( vertSel->getNormalX() ).arg( vertSel->getNormalY() ).arg( vertSel->getNormalZ() );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>Normal (sph. degree):</td><td>%1&nbsp;%2&nbsp;%3</td>" ).arg( vertNorm.getSphPhiDeg() ).arg( vertNorm.getSphThetaDeg() ).arg( vertNorm.getSphRadius() );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>Color (RGBA):</td><td>%1&nbsp;%2&nbsp;%3&nbsp;%4</td>" ).arg( vertSel->getR() ).arg( vertSel->getG() ).arg( vertSel->getB() ).arg( vertSel->getA() );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>ID&nbsp;ori/curr:</td><td>%1/%2</td>" ).arg( vertSel->getIndex() ).arg( vertSel->getIndexOriginal() );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>Function&nbsp;value:</td><td>%1</td>" ).arg( vertFuncVal );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>Label&nbsp;ID:</td><td>%1</td>" ).arg( vertLabel );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>Flags:</td><td>0x" ) + tr( "%1" ).arg( vertFlags, 0, 16 ).toUpper() + tr( "</td>" );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td></td><td>%1</td>" ).arg( vertFlags, 0, 2 );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>Dist. to Axis:</td><td>" ) + tr( "%1" ).arg( distanceToAxis ) + tr( "</td>" );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td>Angle to Axis:</td><td>" ) + tr( "%1 %2&deg;" ).arg( angleInAxisCoord ).arg( angleInAxisCoord*180.0/M_PI ) + tr( "</td>" );
		strMsg += QString( "</tr>" );
		strMsg += strFeatVec;
		strMsg += QString( "</table>" );
		SHOW_MSGBOX_INFO( tr("Information about SelPrim/SelVert"), strMsg );
		return( true );
	}

	if( mPrimSelected->getType() == IS_FACE ) {
		//! \todo add more information.
		Face* faceSel = static_cast<Face*>(mPrimSelected);
		double edgeLens[3];
		faceSel->getEdgeLengths( edgeLens );
		double altitudes[3];
		faceSel->getAltitudes( altitudes );
		double angles[3];
		faceSel->getAngles( angles );
		uint64_t labelIdA=_NOT_A_NUMBER_ULONG_;
		uint64_t labelIdB=_NOT_A_NUMBER_ULONG_;
		uint64_t labelIdC=_NOT_A_NUMBER_ULONG_;
		faceSel->getVertA()->getLabel( labelIdA );
		faceSel->getVertB()->getLabel( labelIdB );
		faceSel->getVertC()->getLabel( labelIdC );
		// Prepare HTML:
		QString strMsg;
				strMsg += QString( "<table style=width:'100%';table-layout:'fixed'>" );
				strMsg += QString( "<tr><td align='left'>" );
				strMsg += QString( "<table style=width:100%;table-layout:fixed>" );
		strMsg += QString( "<tr>" );
		strMsg += QString( "<td>&nbsp;</td>" );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td align='right'>Vertex&nbsp;indices:</td>" );
		strMsg += QString( "</tr><tr>" );
		strMsg += tr( "<td align='right'>Vertex&nbsp;label id:</td>" );
		strMsg += QString( "</tr><tr>" );
		strMsg += QString( "<td>&nbsp;</td>" );
		strMsg += QString( "</tr><tr>" );
                strMsg += tr( "<td align='right'>Vertex&nbsp;A&nbsp;coordinates:</td>" );
				strMsg += QString( "</tr><tr>" );
                strMsg += tr( "<td align='right'>Vertex&nbsp;B&nbsp;coordinates:</td>" );
				strMsg += QString( "</tr><tr>" );
                strMsg += tr( "<td align='right'>Vertex&nbsp;C&nbsp;coordinates:</td>" );
		strMsg += QString( "</tr><tr>" );
		strMsg += QString( "<td>&nbsp;</td>" );
		strMsg += QString( "</tr><tr>" );
                strMsg += tr( "<td align='right'>Edge&nbsp;lengths:</td>" );
				strMsg += QString( "</tr><tr>" );
                strMsg += tr( "<td align='right'>Altitudes:</td>" );
				strMsg += QString( "</tr><tr>" );
                strMsg += tr( "<td align='right'>Angles:</td>" );
		strMsg += QString( "</tr>" );
				strMsg += QString( "</table></td>" );

				strMsg += QString( "<td align='right'><table style=width:'100%';table-layout:'auto'>" );
				strMsg += QString( "<col width='220px'/><col width='220px'/><col width='220px'/>" );
		strMsg += QString( "<tr><td align='right' style='padding-left:10px'>A</td>"
		                  "<td align='right' style='padding-left:10px'>B</td>"
		                  "<td align='right' style='padding-left:10px'>C</td></tr>" );
		strMsg += QString( "<tr><td align='right' style='padding-left:10px'>%1</td>"
		                  "<td align='right' style='padding-left:10px'>%2</td>"
		                  "<td align='right' style='padding-left:10px'>%3</td></tr>" )
		                  .arg( faceSel->getVertAIndex() )
		                  .arg( faceSel->getVertBIndex() )
		                  .arg( faceSel->getVertCIndex() );
		strMsg += QString( "<tr><td align='right' style='padding-left:10px'>%1</td>"
		                  "<td align='right' style='padding-left:10px'>%2</td>"
		                  "<td align='right' style='padding-left:10px'>%3</td></tr>" )
		                   .arg( labelIdA )
		                   .arg( labelIdB )
		                   .arg( labelIdC );
		strMsg += QString( "<tr><td align='center' style='padding-left:10px'>&nbsp;</td>"
		                  "<td align='center' style='padding-left:10px'>&nbsp;</td>"
		                  "<td align='center' style='padding-left:10px'>&nbsp;</td></tr>" );
				strMsg += QString( "<tr><td align='left' style='padding-left:10px'>X:&nbsp;%1</td>"
                                    "<td align='center' style='padding-left:10px'>Y:&nbsp;%2</td>"
                                    "<td align='right' style='padding-left:10px'>Z:&nbsp;%3</td></tr>" )
                                    .arg(faceSel->getVertA()->getX())
                                    .arg(faceSel->getVertA()->getY())
                                    .arg(faceSel->getVertA()->getZ());
				strMsg += QString( "<tr><td align='left' style='padding-left:10px'>X:&nbsp;%1</td>"
                                    "<td align='center' style='padding-left:10px'>Y:&nbsp;%2</td>"
                                    "<td align='right' style='padding-left:10px'>Z:&nbsp;%3</td></tr>" )
                                    .arg(faceSel->getVertB()->getX())
                                    .arg(faceSel->getVertB()->getY())
                                    .arg(faceSel->getVertB()->getZ());
				strMsg += QString( "<tr><td align='left' style='padding-left:10px'>X:&nbsp;%1</td>"
                                    "<td align='center' style='padding-left:10px'>Y:&nbsp;%2</td>"
                                    "<td align='right' style='padding-left:10px'>Z:&nbsp;%3</td></tr>" )
                                    .arg(faceSel->getVertC()->getX())
                                    .arg(faceSel->getVertC()->getY())
                                    .arg(faceSel->getVertC()->getZ());
		strMsg += QString( "<tr><td align='center' style='padding-left:10px'>&nbsp;</td>"
		                  "<td align='center' style='padding-left:10px'>&nbsp;</td>"
		                  "<td align='center' style='padding-left:10px'>&nbsp;</td></tr>" );
				strMsg += QString( "<tr><td align='left' style='padding-left:10px'>&nbsp;%1</td>"
                                    "<td align='center' style='padding-left:10px'>&nbsp;%2</td>"
                                    "<td align='right' style='padding-left:10px'>&nbsp;%3</td></tr>" )
                                    .arg( edgeLens[0] )
                                    .arg( edgeLens[1] )
                                    .arg( edgeLens[2] );
				strMsg += QString( "<tr><td align='left' style='padding-left:10px'>&nbsp;%1</td>"
                                    "<td align='center' style='padding-left:10px'>&nbsp;%2</td>"
                                    "<td align='right' style='padding-left:10px'>&nbsp;%3</td></tr>" )
                                    .arg( altitudes[0] )
                                    .arg( altitudes[1] )
                                    .arg( altitudes[2] );
				strMsg += QString( "<tr><td align='left' style='padding-left:10px'>&nbsp;%1</td>"
                                    "<td align='center' style='padding-left:10px'>&nbsp;%2</td>"
                                    "<td align='right' style='padding-left:10px'>&nbsp;%3</td></tr>" )
                                    .arg( angles[0] )
                                    .arg( angles[1] )
                                    .arg( angles[2] );
				strMsg += QString( "<tr><td align='left' style='padding-left:10px'>&nbsp;%1&deg;</td>"
                                    "<td align='center' style='padding-left:10px'>&nbsp;%2&deg;</td>"
                                    "<td align='right' style='padding-left:10px'>&nbsp;%3&deg;</td></tr></table></td>" )
                                    .arg( angles[0]*180.0/M_PI )
                                    .arg( angles[1]*180.0/M_PI )
                                    .arg( angles[2]*180.0/M_PI );
						strMsg += QString( "</tr>" );
						strMsg += QString( "</table>" );
		SHOW_MSGBOX_INFO( tr("Information about SelPrim/SelFace"), strMsg );
		return( true );
	}

	SHOW_MSGBOX_WARN( tr("Information about SelPrim"), tr("Not yet implemented :(") );
	return( false );
}

//! Display minimum and maximum of the vertices FuncVal.
//! See Mesh::getFuncValuesMinMaxInfNanFail
//!
//! @returns false in case of an error. True otherwise.
bool MeshQt::showInfoFuncValHTML() {
	// Values to be retrieved
	double  minVal;
	double  maxVal;
	int     infCount;
	int     nanCount;
	int     failCount;
	uint64_t finiteCount;
	Vertex* minVert;
	Vertex* maxVert;

	// Retrieve information
	bool retVal = getFuncValuesMinMaxInfNanFail( minVal, maxVal, minVert, maxVert, infCount, nanCount, failCount, finiteCount );
	if( !retVal ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: call of getFuncValuesMinMaxInfNanFail failed!" << endl;
		return( false );
	}

	// Retrieve Quantiles:
	double quantil01; // 1%
	double quantil99; // 99%
	double quantil50; // 50% i.e. median
	double quantil05; // 5%
	double quantil95; // 95%
	if( !getFuncValuesMinMaxQuantil( 0.01, 0.99, quantil01, quantil99 ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: call of getFuncValuesMinMaxQuantil failed!" << endl;
		return( false );
	}
	if( !getFuncValuesMinMaxQuantil( 0.05, 0.95, quantil05, quantil95 ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: call of getFuncValuesMinMaxQuantil failed!" << endl;
		return( false );
	}
	if( !getFuncValuesMinMaxQuantil( 0.50, 0.50, quantil50, quantil50 ) ) {
		cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: call of getFuncValuesMinMaxQuantil failed!" << endl;
		return( false );
	}

	// Build HTML string
	string strMsg;
	strMsg += "<table align='center' border='0'>";
	strMsg += "<tr><td></td><td align='center'>Value</td><td align='right' style='padding-left:10px'>Index</td><td align='right' style='padding-left:10px'>Original</td></tr>";
	strMsg += "<tr>";

	// Minimum
	if( ( minVert != nullptr ) && ( isfinite( minVal ) ) ) {
		// Round small values to 5 digits
		if( abs( minVal ) < 10000.0 ) {
			minVal = round( minVal*1000.0 ) / 1000.0;
		}
		// Format string
		char minValBuffer[255];
		sprintf( minValBuffer, "%+g", minVal );

		strMsg += "<td>Minimum:</td>";
		strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + string( minValBuffer ) + "</td>";
		strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + to_string( minVert->getIndex() ) + "</td>";
		strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + to_string( minVert->getIndexOriginal() ) + "</td>";
		strMsg += "</td>\n";
	} else {
		strMsg += "<td>Minimum:</td>";
		strMsg += "<td align='right' style='padding-left:10px'>n.a.</td>";
		strMsg += "<td align='right' style='padding-left:10px'></td>";
		strMsg += "<td align='right' style='padding-left:10px'></td>";
		strMsg += "</td>\n";
	}

	// Quantile

	// Round small values to 5 digits
	if( abs( quantil01 ) < 10000.0 ) {
		quantil01 = round( quantil01*1000.0 ) / 1000.0;
	}
	// Format string
	char quantil01Buffer[255];
	sprintf( quantil01Buffer, "%+g", quantil01 );

	// Round small values to 5 digits
	if( abs( quantil05 ) < 10000.0 ) {
		quantil05 = round( quantil05*1000.0 ) / 1000.0;
	}
	// Format string
	char quantil05Buffer[255];
	sprintf( quantil05Buffer, "%+g", quantil05 );

	// Round small values to 5 digits
	if( abs( quantil95 ) < 10000.0 ) {
		quantil95 = round( quantil95*1000.0 ) / 1000.0;
	}
	// Format string
	char quantil95Buffer[255];
	sprintf( quantil95Buffer, "%+g", quantil95 );

	// Round small values to 5 digits
	if( abs( quantil99 ) < 10000.0 ) {
		quantil99 = round( quantil99*1000.0 ) / 1000.0;
	}
	// Format string
	char quantil99Buffer[255];
	sprintf( quantil99Buffer, "%+g", quantil99 );

	// Round small values to 5 digits
	if( abs( quantil50 ) < 10000.0 ) {
		quantil50 = round( quantil50*1000.0 ) / 1000.0;
	}
	// Format string
	char quantil50Buffer[255];
	sprintf( quantil50Buffer, "%+g", quantil50 );

	strMsg += "</tr><tr>";
	strMsg += "<td>Quantile&nbsp;1%</td>";
	strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + string( quantil01Buffer ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "</td>\n";
	strMsg += "</tr><tr>";
	strMsg += "<td>Quantile&nbsp;5%</td>";
	strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + string( quantil05Buffer ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "</td>\n";
	strMsg += "</tr><tr>";
	strMsg += "<td>Median</td>";
	strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + string( quantil50Buffer ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "</td>\n";
	strMsg += "</tr><tr>";
	strMsg += "<td>Quantile&nbsp;95%</td>";
	strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + string( quantil95Buffer ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "</td>\n";
	strMsg += "</tr><tr>";
	strMsg += "<td>Quantile&nbsp;99%</td>";
	strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + string( quantil99Buffer ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "<td align='right' style='padding-left:10px'></td>";
	strMsg += "</td>\n";

	// Maximum
	strMsg += "</tr><tr>";
	if( ( maxVert != nullptr ) && ( isfinite( maxVal ) ) ) {
		// Round small values to 5 digits
		if( abs( maxVal ) < 10000.0 ) {
			maxVal = round( maxVal*1000.0 ) / 1000.0;
		}
		// Format string
		char maxValBuffer[255];
		sprintf( maxValBuffer, "%+g", maxVal );

		strMsg += "<td>Maximum:</td>";
		strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + string( maxValBuffer ) + "</td>";
		strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + to_string( maxVert->getIndex() ) + "</td>";
		strMsg += "<td align='right' style='padding-left:10px'>&nbsp;" + to_string( maxVert->getIndexOriginal() ) + "</td>";
		strMsg += "</td>\n";
	} else {
		strMsg += "<td>Maximum:</td>";
		strMsg += "<td align='right' style='padding-left:10px'>n.a.</td>";
		strMsg += "<td align='right' style='padding-left:10px'></td>";
		strMsg += "<td align='right' style='padding-left:10px'></td>";
		strMsg += "</td>\n";
	}
	strMsg += "</table>\n\n";
	strMsg += "<hr />\n\n";

	strMsg += "<table align='center'>";
	strMsg += "</tr>";
	strMsg += "<td>Vertex&nbsp;count:</td><td align='right' style='padding-left:10px'>" + to_string( getVertexNr() )+ "</td>";
	strMsg += "</tr><tr>";
	strMsg += "<td>Failed:</td><td align='right' style='padding-left:10px'>" + to_string( failCount ) + "</td>";
	strMsg += "</tr><tr>";
	strMsg += "<td>Finite:</td><td align='right' style='padding-left:10px'>" + to_string( finiteCount ) + "</td>";
	strMsg += "</tr><tr>";
	strMsg += "<td>Infinity:</td><td align='right' style='padding-left:10px'>" + to_string( infCount ) + "</td>";
	strMsg += "</tr><tr>";
	strMsg += "<td>Not&nbsp;A&nbsp;Number:</td><td align='right' style='padding-left:10px'>" + to_string( nanCount ) + "</td>";
	strMsg += "</tr>";
	strMsg += "</table>\n\n";

	SHOW_MSGBOX_INFO( tr("FuncVal - Information"), QString( strMsg.c_str() ) );
	return( true );
}

//! Display the (cone) axis property.
//!
//! @returns false in case of an error. True otherwise.
bool MeshQt::showInfoAxisHTML() {
	// Check the presence of an axis
	bool axisDefined = getConeAxisDefined();
	if( !axisDefined ) {
		SHOW_MSGBOX_INFO( tr("Properties of the axis"), tr("There is no axis defined.") );
		return( true );
	}

	// Fetch axis information
	Vector3D axisTop;
	Vector3D axisBottom;
	if( !getConeAxis( &axisTop, &axisBottom ) ) {
		return( false );
	}
	Vector3D axisDirection = axisTop - axisBottom;
	axisDirection.normalize3();

	Vector3D origin( 0.0, 0.0, 0.0, 1.0 );
	double distToOrigin = origin.distanceToLine( &axisTop, &axisBottom );

	// Build HTML string - header
	string strMsg;
	strMsg += "<table align='center' border='0'>"
	          "<tr>"
	          "<td></td>"
	          "<td align='center'>X</td>"
	          "<td align='center'>Y</td>"
	          "<td align='center'>Z</td>"
	          "</tr><tr>";

	// Build HTML string - content
	strMsg += "<td>Top:</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisTop.getX() ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisTop.getY() ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisTop.getZ() ) + "</td>";
	strMsg += "</tr><tr>";
	strMsg += "<td>Bottom:</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisBottom.getX() ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisBottom.getY() ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisBottom.getZ() ) + "</td>";
	strMsg += "</tr><tr>";
	strMsg += "<td>Direction:</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisDirection.getX() ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisDirection.getY() ) + "</td>";
	strMsg += "<td align='right' style='padding-left:10px'>" + to_string( axisDirection.getZ() ) + "</td>";

	// Build HTML string - footer
	strMsg += "</tr>"
	          "</table>";

	strMsg += "<p>Distance to origin: " + to_string( distToOrigin ) + "</p>";

	SHOW_MSGBOX_INFO( tr("Properties of the axis"), QString( strMsg.c_str() ) );
	return( true );
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// Overloaded from MeshIO
//=============================================================================

//! Write 3D-data to file after asking the user for a filename.
bool MeshQt::writeFileUserInteract(const bool asLegacy) {

    if (asLegacy){
        //! Disable the export of Features, flags and labels for the legacy export
        MeshIO::setFlagExport( MeshIO::EXPORT_VERT_FLAGS, false );
        MeshIO::setFlagExport( MeshIO::EXPORT_VERT_LABEL, false );
        MeshIO::setFlagExport( MeshIO::EXPORT_VERT_FTVEC, false );
        MeshIO::setFlagExport( MeshIO::EXPORT_POLYLINE,   false );
    }
	QSettings settings;

	QString filePath    = QString( settings.value( "lastPath" ).toString() ); // or: getFileLocation().c_str()
    QString fileSuggest = QString::fromStdWString( getBaseName().wstring() );

	bool mostLikelyOrientated = false;
	getParamFlagMesh( FILE_TRANSFORMATION_APPLIED, &mostLikelyOrientated );
	if( mostLikelyOrientated ) {
		QRegularExpression nameContainsGMwithO( ".*_GM[oOcCfFpP]*$" );
		QRegularExpressionMatch match = nameContainsGMwithO.match( fileSuggest );
		if( match.hasMatch() ) {
			QRegularExpression nameContainsGM( "(.*_GM)([cCfFpP]*)$" );
			match = nameContainsGM.match( fileSuggest );
			if( match.hasMatch() ) {
				fileSuggest.replace( nameContainsGM, "\\1O\\2" );
			}
		} else {
			fileSuggest += "_GMO";
		}
	}
	fileSuggest += ".ply";

	QStringList filters;
    filters << tr("3D-Data (*.ply)")
            << tr("3D-Data (*.obj)")
            << tr("3D-Data (*.gltf)")
            << tr("3D-Data outdated (*.wrl)")
            << tr("3D-Data outdated (*.txt)")
            << tr("3D-Data outdated (*.xyz)");

	QFileDialog dialog( mMainWindow );
	dialog.setWindowTitle( tr("Save 3D-model as:") );
	dialog.setOptions( QFileDialog::DontUseNativeDialog ); // without there is no suggested filename - at least using Ubuntu.
	dialog.setFileMode( QFileDialog::AnyFile );
	dialog.setAcceptMode( QFileDialog::AcceptSave );
	dialog.setDirectory( filePath );
	dialog.selectFile( fileSuggest );
	dialog.setNameFilters( filters );
	if( dialog.exec() != QDialog::Accepted ) {
		return false; // cancled
	}
	QStringList fileNames = dialog.selectedFiles();
	if( fileNames.at( 0 ).length() == 0 ) { // Sanity
		//setPathLast( fileName );
		return false;
	}

	return writeFile( fileNames.at( 0 ) );
}

//! Write 3D-data to file
//! Overloaded from MeshIO
bool MeshQt::writeFile( const QString& rFileName ) {
	emit statusMessage( "Saving 3D-data to " + rFileName );
	if( !writeFile( rFileName.toStdString() ) ) {
		emit statusMessage( "ERROR - File " + rFileName + " NOT saved." );
		return( false );
	}

	QSettings settings;
	settings.setValue( "lastPath", rFileName );
	emit sFileChanged( QString::fromStdWString( getFullName().wstring() ), QString::fromStdWString( getBaseName().wstring() ) );
	emit statusMessage( "3D-data saved to " + rFileName );
	return( true );
}

//! Write 3D-data to file
//! Overloaded from MeshIO
bool MeshQt::writeFile( const filesystem::path& rFileName ) {
	if( !editMetaData() ) {
		return( false );
	}

	std::cout << "[MeshQt::" << __FUNCTION__ << "] to: " << rFileName << std::endl;
    std::string fileExtension = rFileName.extension().string();
    if(fileExtension == ".gltf"){
        bool userContinue;
        if( showQuestion( &userContinue, tr("Continue?").toStdString(), \
                          tr("The GLTF results in a larger file and cannot be read by GigaMesh. Do you want to continue?").toStdString() ) ) {
            if( !userContinue ) {
                // User cancel.
                return( false );
            }
        }
        bool exportNormals;
        if( showQuestion( &exportNormals, tr("Normal Export?").toStdString(), \
                          tr("Do you want to export the normals per vertex? This will result in a larger file!").toStdString() ) ) {
            if( exportNormals ) {
                // User cancel.
                MeshIO::setFlagExport(EXPORT_VERT_NORMAL,true);
            }
            else{
                MeshIO::setFlagExport(EXPORT_VERT_NORMAL,false);
            }
        }
        else{
            return( false );
        }

    }
	if( !MeshGL::writeFile( rFileName ) ) {
		std::cerr << "[MeshQt::" << __FUNCTION__ << "] could not write to file '" << rFileName << "'! " << std::endl;
		return( false );
	}
	return( true );
}

// Set Flags ---------------------------------------------------------------------------------------------------------------------------------------------------

//! Set the MeshIO::EXPORT_BINARY flag.
bool MeshQt::setFileSaveFlagBinary( bool rSetTo ) {
	return setFlagExport( MeshIO::EXPORT_BINARY, rSetTo );
}

bool MeshQt::setFileSaveFlagExportTextures(bool setTo)
{
	setFlagExport( MeshIO::EXPORT_TEXTURE_FILE, setTo);
	return setTo;
}

// Set Information ------------------------------------------------------------

//! Imports a texture map from a given file and assigns it to a new texture map.
//! @returns false in case of an error. True otherwise.
bool MeshQt::importTexMapFromFile( const QString& rFileName ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	emit statusMessage( "Importing Texture Map from " + rFileName );

	int    nrLines = 0;
	uint64_t*  refToPrimitves = nullptr;
	unsigned char*  texMap          = nullptr;
	string fileNameWithPath = rFileName.toStdString();

	if( !importTEXMap( fileNameWithPath, &nrLines, &refToPrimitves, &texMap ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] could not open file '" << fileNameWithPath << "'! " << endl;
		emit statusMessage( "ERROR - Import failed: Could not open: " + rFileName );
		return( false );
	}

	bool assignSuccess = false;
	size_t found   = fileNameWithPath.find_last_of( '/' ); // for windows systems: "\\" for crossplatform: "/\\"
	string texName = fileNameWithPath.substr( found+1 ); // .substr( 0,found ) would return the path
	if( !assignImportedTexture( nrLines, refToPrimitves, texMap ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] Could not assign nor store texture map '" << texName << "'!" << endl;
		emit statusMessage( "ERROR - Assign texture map failed: Could not store as " + QString::fromStdString( texName ) );
		// no return here as we have to free our arrays!
	} else {
		emit statusMessage( "Texture Map from " + QString::fromStdString( texName ) + " imported and assigned." );
		assignSuccess = true;
	}
	if( refToPrimitves != nullptr ) {
		free( refToPrimitves );
	}
	if( texMap != nullptr ) {
		free( texMap );
	}
	return( assignSuccess );
}

//! Imports normal vectors from a given file and assigns it to the corresponding vertices.
//! Returns true, when successful.
bool MeshQt::importNormalVectorsFile( const QString& rFileName ) {
	emit statusMessage( "Importing Texture Map from " + rFileName );
	vector<grVector3ID> normalsFromFile;
	if( !importNormals( rFileName.toStdString(), &normalsFromFile ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No normals imported (1)!" << endl;
		emit statusMessage( "ERROR - No normals imported (1)!" );
		return false;
	}
	if( normalsFromFile.size() == 0 ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No normals imported (2)!" << endl;
		emit statusMessage( "ERROR - No normals imported (2)!" );
		return false;
	}
	if( !assignImportedNormalsToVertices( normalsFromFile ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No normals assigned!" << endl;
		emit statusMessage( "ERROR - No normals assigned!" );
		return false;
	}
	emit updateGL();
	return true;
}

//! Import feature vectors and emit statusMessage.
//! See MeshSeedExt::importFeatureVectorsFromFile
//! @returns false in case of an error. True otherwise.
bool MeshQt::importFeatureVectors( const QString& rFileName ) {
	emit statusMessage( "Importing feature vectors from " + rFileName );
	if( !Mesh::importFeatureVectorsFromFile( rFileName.toStdString() ) ) {
		emit statusMessage( "ERROR - Reading file " + rFileName );
		return( false );
	}
	emit primitiveSelected( mPrimSelected );
	emit statusMessage( "Feature vectors assigned and imported from " + rFileName );
	return( true );
}

//! Import function values and emit statusMessage.
//! See ...
//! @returns false in case of an error. True otherwise.
bool MeshQt::importFunctionValues( const QString& rFileName ) {

	emit statusMessage( "Importing feature vectors from " + rFileName );

	// Ask for vertex index within the first colum
	bool hasVertexIndex = true;
	if( !showQuestion( &hasVertexIndex, "First Column", "Does the first column contain the vertex index?<br /><br />"
					   "Recommendation: YES for files computed with gigamesh-featurevectors" ) ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] User cancled." << std::endl;
		return( false );
	}

	if( !Mesh::importFuncValsFromFile( rFileName.toStdString(), hasVertexIndex ) ) {
		emit statusMessage( "ERROR - Reading file " + rFileName );
		return( false );
	}
	emit primitiveSelected( mPrimSelected );
	emit statusMessage( "Feature vectors assigned and imported from " + rFileName );
	return( true );
}

//! Import labels and emit statusMessage.
//! See ...
//! @returns false in case of an error. True otherwise.
bool MeshQt::importLabels( const QString& rFileName ) {

    emit statusMessage( "Importing labels from " + rFileName );

    // Ask for vertex index within the first colum
    bool hasVertexIndex = true;
    if( !showQuestion( &hasVertexIndex, "First Column", "Does the first column contain the vertex index?<br /><br />"
                       "YES for files with index label columns." ) ) {
        std::cout << "[Mesh::" << __FUNCTION__ << "] User cancled." << std::endl;
        return( false );
    }

    if( !Mesh::importLabelsFromFile( rFileName.toStdString(), hasVertexIndex ) ) {
        emit statusMessage( "ERROR - Reading file " + rFileName );
        return( false );
    }
    emit primitiveSelected( mPrimSelected );
    emit statusMessage( "Labels assigned and imported from " + rFileName );
    return( true );
}

//! Import Polylines and emit statusMessage.
//! See ...
//! @returns false in case of an error. True otherwise.
bool MeshQt::importPolylines( const QString& rFileName ) {
    emit statusMessage( "Importing Polylines from " + rFileName );
    if( !Mesh::importPolylinesFromFile( rFileName.toStdString()) ) {
        emit statusMessage( "ERROR - Reading file " + rFileName );
        return( false );
    }


    return( true );
}
//! Import transformation matrices from transmat.txt and emit statusMessage.
//! See ...
//! @returns false in case of an error. True otherwise.
bool MeshQt::importApplyTransMat( const QString& rFileName ) {
    emit statusMessage( "Importing transformation matrices from " + rFileName );
    if( !Mesh::importApplyTransMatFromFile( rFileName.toStdString()) ) {
        emit statusMessage( "ERROR - Reading file " + rFileName );
        return( false );
    }
    return( true );
}
//! Export feature vectors and emit statusMessage
bool MeshQt::exportFeatureVectors()
{
    QString filePath = QString::fromStdWString( getFileLocation().wstring() ) ;

	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Export feature vectors" ),
	                                                 filePath +
	                                                 QString::fromStdWString(L"/" + getBaseName().wstring() + L"_featureVectors.txt"),
													 tr( "Feature Vectors (*.txt *.mat)" ) );
	if( fileName.length() == 0 ) {
		return false;
	}

	emit statusMessage( "Exporting feature vectors to " + fileName);

	if( !Mesh::exportFeatureVectors( fileName.toStdString() ) ) {
		emit statusMessage( "ERROR - Writing file " + fileName );
		return( false );
	}

	emit statusMessage( "Feature vectors exported to " + fileName);
	return true;
}

//! Display the dialog for NPR settings.
void MeshQt::showNPRSettings() {

	auto dialogNprSettings = new QGMDialogNprSettings(mMainWindow);

	dialogNprSettings->setAttribute(Qt::WA_DeleteOnClose);
	dialogNprSettings->setWindowFlags( dialogNprSettings->windowFlags() | Qt::Tool);

	QObject::connect(dialogNprSettings, &QGMDialogNprSettings::valuesChanged, this, &MeshQt::updateGL );

	dialogNprSettings->init( this, mRenderColors );
	dialogNprSettings->show();
}

void MeshQt::showTransparencySettings()
{
	auto dialogTransparencySettings = new QGMDialogTransparencySettings(mMainWindow);

	dialogTransparencySettings->setAttribute(Qt::WA_DeleteOnClose);
	dialogTransparencySettings->setWindowFlags( dialogTransparencySettings->windowFlags() | Qt::Tool);

	QObject::connect( dialogTransparencySettings,
					  &QGMDialogTransparencySettings::valuesChanged,
					  this,
					  &MeshQt::updateGL
					);

	dialogTransparencySettings->init(this , mOpenGLContext->format().majorVersion() > 4 ||
                                     (mOpenGLContext->format().majorVersion() == 4 && mOpenGLContext->format().minorVersion() >= 3));
	dialogTransparencySettings->show();
}
