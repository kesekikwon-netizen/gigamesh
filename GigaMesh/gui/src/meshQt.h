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

#ifndef MESHQT_H
#define MESHQT_H

// generic C++ includes:
// none

// C++ includes:
#include "meshGL/meshGLShader.h"
#include <GigaMesh/mesh/voxelfilter25d.h>
#include "image2d.h"

// generic Qt includes:
#include <QObject>
#include <QStringList>
#include <QColorDialog>

// Qt Interface includes:
// none

// Qt includes:
#include "QGMMainWindow.h"
#include "QGMDialogSliderHD.h"
#include "QGMDialogMSII.h"
#include "QGMDialogConeParam.h"
#include "QGMDialogPlaneParam.h"
#include "MeshQtCSVImportExport.h"

class QGMMainWindow;

//!
//! \brief Qt extension for the Mesh class. (Layer 2)
//!
//! Extends the Mesh class with Qt functions for interactive display and
//! manipulation. As it is derived from the OpenGL Mesh we get all the functions
//! we need.
//!
//! Layer 2
//!

class MeshQt : public QObject, public MeshGLShader, public MeshQtCSVImportExport {
	Q_OBJECT

	public:
		MeshQt( const QString& rFileName, bool& rReadSuccess, \
		        const float *rMatModelView, const float *rMatProjection, \
		        MeshWidgetParams* rMeshWidgetParams, MeshGLColors* rRenderColors, \
		        QGMMainWindow* setMainWindow, QGLContext* rContext, QObject* rParent=0 );

		~MeshQt();

		virtual bool readIsRegularGrid( bool* rIsGrid ) override; // overloaded from MeshIO

		virtual void polyLinesChanged() override;

		virtual void changedFaceFuncVal() override;
		virtual void changedVertFuncVal() override;

		// Progress bar - overloaded from Primitive:
		virtual void showProgressStart( const std::string& rMsg ) override;
		virtual bool showProgress( double rVal, const std::string& rMsg ) override;
		virtual void showProgressStop( const std::string& rMsg ) override;

		// Warnings and info from base classes:
		virtual void showInformation( const std::string& rHead, const std::string& rMsg,
		                              const std::string& rToClipboard="" ) override;
		virtual void showWarning( const std::string& rHead, const std::string& rMsg ) override;
		// Entering values:
		virtual bool showEnterText( std::string&    rSomeStrg,  const char* rTitle ) override;
		virtual bool showEnterText( uint64_t&  rULongInt,  const char* rTitle ) override;
		virtual bool showEnterText( double&         rDoubleVal, const char* rTitle ) override;
		virtual bool showEnterText( std::set<int64_t>&   rIntegers,  const char* rTitle ) override;
		virtual bool showEnterText( std::vector<long>&   rIntegers,  const char* rTitle ) override;
		virtual bool showEnterText( std::vector<double>& rDoubles,   const char* rTitle ) override;
		virtual bool showEnterText( Matrix4D* rMatrix4x4, bool selectedVerticesOnly = false ) override;
		virtual bool showSlider( double* rValueToChange, double rMin, double rMax, const char* rTitle ) override;
		virtual bool showQuestion( bool* rUserChoice, const std::string& rHead, const std::string& rMsg ) override;

	public slots:
		// Paramters:
		bool   setParaSmoothLength(); //! \todo very old style - use more general with setParamFloatMesh

		// Flags, integers and floats - MeshGL:
		//----------------------------------------------
		virtual bool   setParamFlagMeshGL( MeshGLParams::eParamFlag rShowFlagNr, bool rSetState );
		        void   toggleShowFlag( MeshGLParams::eParamFlag rShowFlagNr );

		virtual bool   setParamIntMeshGLDialog( MeshGLParams::eParamInt rParamID );
		virtual bool   setParamIntMeshGL( MeshGLParams::eParamInt rParamID, int rValue );
		virtual bool   setParamFloatMeshGLDialog( MeshGLParams::eParamFlt rParamID );   // will trigger an enter-text-dialog.
		virtual bool   setParamFloatMeshGLSlider( int rParamNr, double rValue );        // wrapper for connection with preview of slider.
		virtual bool   setParamFloatMeshGLLimits( MeshGLParams::eParamFlt rParamID, double rMinValue, double rMaxValue ); // will trigger a slider-dialog.

		// Flags, integers and floats - Mesh:
		//----------------------------------------------
		virtual bool   setParamIntMesh( MeshParams::eParamInt rParam, int rValue ); //
		virtual bool   setParamFloatMeshDialog( MeshParams::eParamFlt rParamID );   // will trigger an enter-text-dialog.
		virtual bool   setParamFloatMeshSlider( int rParamNr, double rValue );      // wrapper for connection with preview of slider.
		virtual bool   setParamFloatMeshLimits( MeshParams::eParamFlt rParamID, double rMinValue, double rMaxValue ); // will trigger a slider-dialog.

		// Function calls for Mesh and MeshGL:
		//----------------------------------------------
		virtual bool   callFunctionMesh( MeshParams::eFunctionCall rFunctionID, bool rFlagOptional );
		virtual bool   callFunctionMeshGL( MeshGLParams::eFunctionCall rFunctionID, bool rFlagOptional );

		// file - import/export actions:
				bool   exportPolyLinesCoords();
				bool   exportPolyLinesCoordsProjected();
				bool   exportPolyLinesFuncVals();
				bool   exportFuncVals();
		virtual bool   exportFaceNormalAngles();
		virtual bool   exportFaceNormalAngles( std::filesystem::path filename ) override;

				void exportNormalSphereData();
		// edit actions:
		virtual bool   changedMesh();
		virtual bool   removeVerticesSelected();
		        bool   removeUncleanSmallUser();
		virtual bool   completeRestore();
		virtual bool   insertVerticesEnterManual();
		//.
				void   cutOffFeatureVertex();
				void   cutOffFeatureVertex( double minVal, double maxVal, bool setToNotANumber );
		//.
				bool   funcValSet();
		virtual bool   funcValSet( double setTo );
				void   funcValueCutOff();
		virtual bool   funcValueCutOff( double minVal, double maxVal, bool setToNotANumber );
		virtual bool   funcValsNormalize();
		virtual bool   funcValsAbs();
		virtual bool   funcValsAdd();
		virtual bool   funcValsAdd( double rVal );
		virtual bool   funcValsToFeatureVector();
		//.
		virtual bool   setConeData();
		virtual bool   setConeParameters( const Vector3D& rAxisTop, const Vector3D& rAxisBot,
		                                  double rUpperRadius, double rLowerRadius );
		virtual bool   centerAroundCone();
		virtual bool   unrollAroundCone( bool* rIsCylinderCase );
		virtual bool   unrollAroundCylinderRadius();
		//.
		virtual bool   splitByPlane();
		virtual bool   splitByPlane( Vector3D rPlaneHNF, bool rSeperate );
		virtual bool   splitByIsoValue();
		//.
		virtual bool   centerAroundSphere();
		virtual bool   unrollAroundSphere();
		//.
		virtual bool   datumAddSphere();
		virtual bool   datumAddSphere( std::vector<double> rPosAndRadius );
		//.
        virtual bool   downscaleTexture();
        //.
		virtual bool   applyMeltingSphere();
		//.
        virtual bool applyAutomaticMeshAlignment(bool askForFront=true);
		// Select actions ------------------------------------------------------------------------------------------------------------------------------
		virtual unsigned int  selectedMVertsChanged();
		virtual unsigned int  selectedMFacesChanged();
		virtual unsigned int  selectedMPolysChanged();
		virtual unsigned int  selectedMPositionsChanged();

		virtual bool   getPlaneVPos();
		virtual bool   getPlaneHNF();
		virtual bool   setPlaneVPos();
		virtual bool   setPlaneVPos( std::vector<double> rPositions );
		virtual bool   setPlaneHNF();
		virtual bool   setPlaneHNF( bool rPreferSet, bool rPreferVisDist, bool rPreferSplit );
		virtual bool   setPlaneHNF( Vector3D rPlaneHNF );
		//.
				void   selectVertFuncValLowerThan();
				void   selectVertFuncValGreatThan();
        //.
        virtual bool   selectVertNonMaximum();
		//.
		virtual bool   selectVertLocalMin();
		virtual bool   selectVertLocalMax();
		//.
		virtual bool   selectVertBorder();
		// De-Selection
		virtual bool   deSelMVertsAll();
		virtual bool   deSelMVertsNoLabel();
		// UI/Selection:
		virtual bool   selectVertSolo();
		virtual bool   selectVertNonManifoldFaces();
		virtual bool   selectVertDoubleCone();
		virtual bool   selectVertLabelAreaLT();
		virtual bool   selectVertLabelAreaRelativeLT();
		virtual bool   selectVertLabelAreaRelativeLT( double rPercent );
		virtual bool   selectVertFaceMinAngleLT();
		virtual bool   selectVertFaceMinAngleLT( double rMaxAngle );
		virtual bool   selectVertFaceMaxAngleGT();
		virtual bool   selectVertFaceMaxAngleGT( double rMinAngle );
		virtual bool   selVertLabeledNot();
		//.
		virtual bool   selectFaceNone();
		virtual bool   selectFaceSticky();
		virtual bool   selectFaceNonManifold();
		virtual bool   selectFaceZeroArea();
		virtual bool   selectFaceInSphere();
		virtual bool   selectFaceInSphere( double rRadius );
		virtual bool   selectFaceRandom();
		virtual bool   selectFaceRandom( double rRatio );
		//.
		virtual bool   selectPolyNoLabel();
		virtual bool   selectPolyRunLenGT();
		virtual bool   selectPolyRunLenGT( double rValue );
		virtual bool   selectPolyRunLenLT();
		virtual bool   selectPolyRunLenLT( double rValue );
		virtual bool   selectPolyLongest();
		virtual bool   selectPolyShortest();
		virtual bool   selectPolyLabelNo();
		virtual bool   selectPolyLabelNo(const std::vector<int>& rLabelNrs );
		virtual bool   selectPolyNotLabeled();

		virtual bool       selectPoly( std::vector<QPoint>& rPixelCoords );
        virtual bool       deselectPoly( std::vector<QPoint>& rPixelCoords );
		virtual Primitive* selectPrimitiveAt( int primitiveTypeToSelect, int xPixel, int yPixel, bool addToList );
		// Selection - Plane definition:
		virtual bool   getPlanePosToSet( int* rPosID );
		virtual bool   setPlanePos( Vector3D* rSomePos );
		virtual bool   applyTransfromToPlane( Matrix4D transMat );
		virtual bool   applyTransformation( Matrix4D rTrans, std::set<Vertex*>* rSomeVerts, bool rResetNormals = true );

		// Selection - Cone:
		virtual bool       setConeAxis( const Vector3D& rUpper, const Vector3D& rLower );
		virtual bool       setConeRadius(Vector3D& point);
		virtual coneStates getConeStatus();

		// Selection - Sphere:
		virtual bool   setSpherePoint(Vector3D &p, Vector3D &normal);
		virtual int    getSpherePointIdx();

		// view actions:
				bool   polylinesCurvScale();

		// analyze actions:
		virtual int  labelFaces();
				void intersectSphere();
		virtual void labelSelectionToSeeds();
		virtual bool labelVerticesEqualFV();
        virtual bool labelVerticesEqualRGB();
		virtual bool labelSelMVertsToBack();
		virtual bool convertBordersToPolylines();
		virtual void convertLabelBordersToPolylines();
				void convertSelectedVerticesToPolyline();
				void advancePolyThres();
		virtual bool compPolylinesIntInvRunLen();
		virtual bool compPolylinesIntInvAngle();
		virtual void getPolylineExtrema();
		virtual bool setPolylinesNormalToVert();
		//.
		        void createSkeletonLine();
		//.
		        void estimateMSIIFeat();
		        void estimateMSIIFeat( ParamsMSII params );
		virtual bool geodPatchVertSel();
		virtual bool geodPatchVertSelOrder();
		//.
		virtual bool fillPolyLines();
		//.
		        bool infoPolylinesLength();
		//.
		        bool hueToFuncVal();
		//.
		void generateOctree();
		virtual void generateOctreeVertex(int maxnr);
		virtual void detectselfintersections();
		void drawOctree();
		void removeOctreedraw();
		void deleteOctree();

		// Ellipse
#ifdef ELLIPSENFIT
		void startEllipseFit(QGMDialogFitEllipse*);
		void stepsEllipseFit(QGMDialogFitEllipse*);
		void plainsEllipseFit(QGMDialogFitEllipse*);
#endif
		//.
		void estimateVolume();
		void compVolumePlane();

		// Axis, circle centers
		virtual bool getAxisFromCircleCenters();

		// --- Visualize ---------------------------------------------------------------------------------------------------------------------------------------

		void visualizeFeatLengthEuc();
		void visualizeFeatLengthMan();
		void visualizeFeatBVFunc();
		void visualizeFeatTVSeqn();
		void visualizeFeatDistSelEuc();
		void visualizeFeatDistSelEucNorm();
		void visualizeFeatDistSelMan();
		void visualizeFeatureCosineSimToVertex();
		void visualizeFeatureTanimotoDistTo();
		void visualizeFeatAutoCorrVert();
		void visualizeFeatCorrSelectedVert();
		bool setVertFuncValMult();
		void visualizeFeatAutoCorrSelectedVert();
		//.
		void visualizeDistanceToPlane();
		void visualizeDistanceToPlane( Vector3D rPlaneHNF, bool rAbsDist );
		void visualizeDistanceToCone( bool absDist=false );
		//.
				void visualizeVertexIndices();
		virtual bool funcVert1RingRMin();
		virtual bool funcVert1RingVolInt();
				void visualizeVertexOctree();
				void visualizeVertexOctree( double rEdgeLen );
		//.
				void visualizeVertexFaceSphereAngleMax();
				void visualizeVertexFaceSphereAngleMax( double rRadius );
				void visualizeVertFaceSphereMeanAngleMax();
				void visualizeVertFaceSphereMeanAngleMax( double rRadius );

		// color settings:
		virtual void selectColor( MeshGLColors::eColorSettings rColorId );

		// Extra menu related
		virtual void cuneiformFigureLaTeX();
		        bool editMetaData();
		        void matlabFaceNormalsSel();

		// --- Extra menu - Show Information -----------------------------------------------------------------------------------------------------------
		//! \todo move the following methods to mesh.cpp similar to Mesh::showInfoMeshHTML
		virtual bool showInfoSelectionHTML();
		virtual bool showInfoFuncValHTML();
		virtual bool showInfoAxisHTML();

		// Shader
	void showNPRSettings();
	void showTransparencySettings();

	// Overloaded from MeshIO
	//============================
	public slots:
        virtual bool writeFileUserInteract(const bool asLegacy = false) override;
	    virtual bool writeFile( const QString& rFileName );
	    virtual bool writeFile( const std::filesystem::path& rFileName ) override;
	// Set flags:
		virtual bool setFileSaveFlagBinary( bool rSetTo );
		virtual bool setFileSaveFlagExportTextures( bool setTo );

	// Related to MeshIO
	//===================
				bool importTexMapFromFile(const QString& rFileName );
				bool importNormalVectorsFile(const QString& rFileName );

	// Overloaded from MeshSeedExt
	//============================
		virtual bool importFeatureVectors(const QString& rFileName );
		virtual bool importFunctionValues(const QString& rFileName );
        virtual bool importPolylines(const QString& rFileName );
        virtual bool importApplyTransMat(const QString& rFileName );
        virtual bool importLabels(const QString& rFileName );

		virtual bool exportFeatureVectors();

	signals:
		void updateGL();                             //!< requests an update from the MeshWidget.
		void sDefaultViewLight();                    //!< signal to restore the default view and lights of the MeshWidget.
		void sDefaultViewLightZoom();                //!< signal to restore the default view, lights and zoom of the MeshWidget.
        void sSetDefaultView();                      //!< signal to meshWidget -> set the mesh after transformation to the camera center and ask for saving the transformation as default.
        void sFileChanged(QString,QString);          //!< emitted when a mesh file was opened or stored (path,basename,extension)
		void statusMessage(QString);                 //!< emitted when the status changed.
        void sReloadFile();                          //!< emitted when texture was changed.
		//.
		void visualizeFeatureDist(int,double*);  //!< emitted when the distance to a feature vector has to be estimated and reflected as texture-map (see menuVisualizeFeatDistSelected() ).
		void showFlagState(MeshGLParams::eParamFlag,bool);  //!< emitted when an element of MeshGLParams::showFlagsArr is changed.
		void paramIntSet(MeshGLParams::eParamInt,int);      //!< emitted when a parameter was set.
		void hasElement(int,bool);               //!< emitted, when on ore more elements (e.g. a PolyLine) were created - or all elements of a type were removed.
		// Selection related:
		void primitiveSelected(Primitive*);      //!< emitted when a primitive was selected (e.g. by doubleclick).
		void sSetPlaneHNF(Vector3D);             //!< emitted when a plane was set.
		// Progress bar
		void sShowProgressStart(QString);        //!< to be called before processing
		void sShowProgress(double);              //!< to be called while processing with a value of [0,1]
		void sShowProgressStop(QString);         //!< to be called at the end of processing
		// guide
		void sGuideIDCommon(MeshWidgetParams::eGuideIDCommon);        //!< guide ID for sidebar
		void sGuideIDSelection(MeshWidgetParams::eGuideIDSelection);  //!< guide ID for sidebar
		// Information e.g. for display.
		void sInfoMesh(MeshGLParams::eInfoMesh,QString);        //!< Infos emotted by the mesh (MeshQt) e.g. for display purposes.

	private:
		QGMMainWindow*        mMainWindow;        //!< reference to our main/top-level window
		//! \todo source revision by removing dialogSlider and dialogConeParam as class members.
		QGMDialogSliderHD     mDialogSlider;       //!< Multi-purpose QDialog with a slider and a preview checkbox.
		QGMDialogConeParam    mDialogConeParam;    //!< QDialog for cone parameters.
};

#endif
