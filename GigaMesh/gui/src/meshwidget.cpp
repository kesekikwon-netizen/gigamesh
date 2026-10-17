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

#include "meshwidget.h"
#include "meshGL/glmacros.h"
#include "section.h"

#include <GL/glu.h>  // For gluProject (3D to 2D projection)

// generic Qt includes:
#include <QFileDialog>
#include <QQuaternion>

// Qt includes:
#include "QGMDialogEnterText.h"
#include "QGMDialogSliderHD.h"
#include "QGMDialogRuler.h"


#include <filesystem>
#include <cctype>
#include <cmath>
#include <limits>

#include "svg/SvgIncludes.h"
#include "normalSphereSelection/NormalSphereSelectionDialog.h"

#include "DialogFindTextures.h"

//#include <iostream>
//#include <typeinfo> // see: http://www.cplusplus.com/reference/std/typeinfo/type_info/

// #define DEBUG_SHOW_ALL_METHOD_CALLS

#include <GigaMesh/logging/Logging.h>

using namespace std;

// Vertex Array Object related -- see initializeGL()
// ----------------------------------------------------
using PglGenVertexArrays = void (*)(GLsizei, GLuint *);
using PglBindVertexArray = void (*)(GLuint);

// Sets default values - to be used by all contructors!
// ----------------------------------------------------
#define MESHWIDGETINITDEFAULTS       \
	mMeshVisual( nullptr ),         \
	mFrameCount( 0 ),            \
	mVAO( _NOT_A_NUMBER_UINT_ ), \
	mCortexSelectionModeActive( false ), \
	mActiveSectionPlane( nullptr ), \
	mSectionManager( nullptr )

//! Constructor.
MeshWidget::MeshWidget( const QGLFormat &format, QWidget *parent )
    : QGLWidget( format, parent ), MESHWIDGETINITDEFAULTS {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// now we can use the keyboard to navigate:
	setFocusPolicy( Qt::StrongFocus );
	setAutoFillBackground( false );
	setAutoBufferSwap( false ); // to preven flickering in ::paintEvent when QPainter.end is called!
	// Store pointer to the main window.
	mMainWindow = static_cast<QGMMainWindow*>(parent);

	//needed for selection
	setMouseTracking(true);

	// Initialize section overlay members
	m_showSectionOverlay = false;
	m_sectionLineColor = Qt::red;
	m_sectionLineWidth = 2.0f;
	m_sectionLineAlpha = 1.0f;

	if( parent != nullptr ) {
		resize( parent->geometry().size() );
	}

	//! \todo adapt to new signal-slot concept.
	// Parameters: Flags -----------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(sShowFlagMeshWidget(MeshWidgetParams::eParamFlag,bool)),   \
	                  this,  SLOT(setParamFlagMeshWidget(MeshWidgetParams::eParamFlag,bool))        );

	// Parameters: Integer FROM MainWindow  ----------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(sShowParamIntMeshWidget(MeshWidgetParams::eParamInt)),     \
	                  this,  SLOT(setParamIntegerMeshWidget(MeshWidgetParams::eParamInt))           );
	QObject::connect( mMainWindow, SIGNAL(sShowParamIntMeshWidget(MeshWidgetParams::eParamInt,int)),  \
	                  this,  SLOT(setParamIntegerMeshWidget(MeshWidgetParams::eParamInt,int))        );
	// Parameters: Integer TO MainWindow  ------------------------------------------------------------------------------------------------------------------
	QObject::connect( this,        &MeshWidget::sParamIntegerMeshWidget, \
	                  mMainWindow, &QGMMainWindow::updateWidgetShowInteger );
	// -----------------------------------------------------------------------------------------------------------------------------------------------------

	// Parameters: Floating point  -------------------------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(sShowParamFloatMeshWidget(MeshWidgetParams::eParamFlt,double,double)), \
	                  this,  SLOT(setParamFloatMeshWidget(MeshWidgetParams::eParamFlt,double,double)) );
	QObject::connect( mMainWindow, SIGNAL(sShowParamFloatMeshWidget(MeshWidgetParams::eParamFlt)), \
	                  this,  SLOT(setParamFloatMeshWidget(MeshWidgetParams::eParamFlt)) );

	// Dynamic menu -- report changes of flags to the menus ------------------------------------------------------------------------------------------------
	QObject::connect( this,       SIGNAL(sParamFlagMeshWidget(MeshWidgetParams::eParamFlag,bool)),  \
	                  mMainWindow, SLOT(updateWidgetShowFlag(MeshWidgetParams::eParamFlag,bool))     );
	QObject::connect( this,       SIGNAL(sParamIntegerMeshWidget(MeshWidgetParams::eParamInt,int)),  \
	                  mMainWindow, SLOT(updateWidgetShowInteger(MeshWidgetParams::eParamInt,int))     );

	// View Port Information -------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( this,       SIGNAL(sViewPortInfo(MeshWidgetParams::eViewPortInfo,QString)), \
	                  mMainWindow, SIGNAL(sViewPortInfo(MeshWidgetParams::eViewPortInfo,QString)) );

	// File menu ------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(sFileOpen(QString)),             this, SLOT(fileOpen(QString))              );
	QObject::connect( mMainWindow, SIGNAL(sFileReload()),                  this, SLOT(reloadFile())                   );
	//.
	QObject::connect( mMainWindow, SIGNAL(saveStillImages360HLR()),        this, SLOT(saveStillImages360HLR())        );
	QObject::connect( mMainWindow, SIGNAL(saveStillImages360VUp()),        this, SLOT(saveStillImages360VUp())        );
	QObject::connect( mMainWindow, SIGNAL(saveStillImages360PrimN()),      this, SLOT(saveStillImages360PrimN())      );
	QObject::connect( mMainWindow, SIGNAL(saveStillImages360PlaneN()),     this, SLOT(saveStillImages360PlaneN())     );
	//.
	QObject::connect( mMainWindow, SIGNAL(sphericalImagesLight()),         this, SLOT(sphericalImagesLight())         );
    QObject::connect( mMainWindow, SIGNAL(sphericalImagesLightDir()),      this, SLOT(sphericalImagesLightDir())      );
	QObject::connect( mMainWindow, SIGNAL(sphericalImages()),              this, SLOT(sphericalImages())              );
	QObject::connect( mMainWindow, SIGNAL(sphericalImagesStateNr()),       this, SLOT(sphericalImagesStateNr())       );
	//.
	QObject::connect( mMainWindow, SIGNAL(unloadMesh()),                   this, SLOT(unloadMesh())                   );

	// ---------------------------------------------------------------------------------------------------------------
	QObject::connect( this,       SIGNAL(sStatusMessage(QString)),        mMainWindow, SLOT(setStatusBarMessage(QString)) );
	// ---------------------------------------------------------------------------------------------------------------

	// View menu ------------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, SIGNAL(screenshotSVG()),                this, SLOT(screenshotSVG())                );
	QObject::connect( mMainWindow, SIGNAL(screenshotRuler()),              this, SLOT(screenshotRuler())              );

	QObject::connect( mMainWindow, SIGNAL(rotYaw()),                       this, SLOT(rotYaw())                       );
	QObject::connect( mMainWindow, SIGNAL(rotRoll()),                      this, SLOT(rotRoll())                      );
	QObject::connect( mMainWindow, SIGNAL(rotPitch()),                     this, SLOT(rotPitch())                     );
	QObject::connect( mMainWindow, SIGNAL(rotOrthoPlane()),                this, SLOT(rotOrthoPlane())                );
	//.
	QObject::connect( mMainWindow, SIGNAL(sDefaultViewLight()),            this, SLOT(defaultViewLight())             );
	QObject::connect( mMainWindow, SIGNAL(sDefaultViewLightZoom()),        this, SLOT(defaultViewLightZoom())         );
    //.
	QObject::connect( mMainWindow, SIGNAL(sSelPrimViewReference()),        this, SLOT(selPrimViewReference())         );

	// Settings menu --------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, &QGMMainWindow::selectColorBackground,  this, &MeshWidget::selectColorBackground  );
	// ----------------------------------------------------------------------------------------------------------------

    // Edit menu-------------------------------------------------------------------------------------------------------
    QObject::connect( mMainWindow, SIGNAL(sAutomaticMeshAlignmentDir()),    this, SLOT(applyAutomaticMeshAlignmentDir())     );
    //-----------------------------------------------------------------------------------------------------------------
	// Select menu ----------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, &QGMMainWindow::setPlaneHNFByView,      this, &MeshWidget::setPlaneHNFByView      );
	QObject::connect( mMainWindow, &QGMMainWindow::sOpenNormalSphereSelectionDialogVertices, [this]() {openNormalSphereSelectionDialog(false); });
	QObject::connect( mMainWindow, &QGMMainWindow::sOpenNormalSphereSelectionDialogFaces, [this]() {openNormalSphereSelectionDialog(true); });
	// ----------------------------------------------------------------------------------------------------------------

	// Function calls --------------------------------------------------------------------------------------------------
	QObject::connect( mMainWindow, &QGMMainWindow::sCallFunctionMeshWidget, this, &MeshWidget::callFunctionMeshWidget );
	// ----------------------------------------------------------------------------------------------------------------

	// User guidance - SIDEBAR -----------------------------------------------------------------------------------------
	QObject::connect( this, &MeshWidget::sGuideIDSelection, mMainWindow, &QGMMainWindow::sGuideIDSelection );
	QObject::connect( this, &MeshWidget::sGuideIDCommon,    mMainWindow, &QGMMainWindow::sGuideIDCommon    );
	// ----------------------------------------------------------------------------------------------------------------

    //set the latex placeholder descriptions (PDF Export)
    setLatexPlaceholderDefinition();

	cout << "[MeshWidget] ... done." << endl;

	//! \bug Emitting inside constructor has no effect
	emit sStatusMessage( "No Mesh loaded." );

}

//! Desctructor
MeshWidget::~MeshWidget() {
	//makeCurrent();
	cout << "[MeshWidget::" << __FUNCTION__ << "] Destructor called." << endl;
	//! .) removes shaders for rendering the background.
	
	for(QOpenGLTexture*& textureMap : mTextureMaps)
	{
		if(textureMap != nullptr)
		{
			textureMap->destroy();
			delete textureMap;
		}
	}
	
	delete mShaderGridOrtho;
	delete mShaderGridPolarLines;
	delete mShaderGridPolarCircles;
	delete mShaderGridHighLightCenter;
	delete mShaderImage;
	//doneCurrent();
}

//! Returns the resolution of back-plane of the viewport in dots per centimeter.
//! Attention: Calling this function makes only sense, when in orthographic mode!
//! @returns false in case of an error. true otherwise.
bool MeshWidget::getViewPortResolution(
                double& rRealWidth,
                double& rRealHeight
) const {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	// DEPRECATED
	//-------------------------------------------------------------------------------------------------
	// However, the result differs by a few percent between gluUnProject amd the shader inspired method.
	//GLint    viewport[4];
	//GLdouble modelview[16];
	//GLdouble projection[16];
	//GLfloat  winZ = 1.0 - FLT_EPSILON;
	//GLdouble posX1, posY1, posZ1;
	//GLdouble posX2, posY2, posZ2;
	//GLdouble posX3, posY3, posZ3;
	//.
	//glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	//glGetDoublev( GL_PROJECTION_MATRIX, projection );
	//glGetIntegerv( GL_VIEWPORT, viewport );
	//gluUnProject( 0,             0, winZ, modelview, projection, viewport, &posX1, &posY1, &posZ1 );
	//gluUnProject( 0, viewport[3]-1, winZ, modelview, projection, viewport, &posX2, &posY2, &posZ2 );
	//gluUnProject( viewport[2]-1, 0, winZ, modelview, projection, viewport, &posX3, &posY3, &posZ3 );
	//Vector3D viewPortWideSide( posX3-posX1, posY3-posY1, posZ3-posZ1, 0.0 );
	//Vector3D viewPortHighSide( posX2-posX1, posY2-posY1, posZ2-posZ1, 0.0 );
	//*rRealWidth  = viewPortWideSide.getLength3();
	//*rRealHeight = viewPortHighSide.getLength3();
	//.
	//cout << "[MeshWidget::" << __FUNCTION__ << "] realWidth:  " << *rRealWidth  << endl;
	//cout << "[MeshWidget::" << __FUNCTION__ << "] realHeight: " << *rRealHeight << endl;
	//-------------------------------------------------------------------------------------------------
	
	float projInv[16];
	invert( mMatProjection.constData(), projInv );
	Matrix4D projMatInv( projInv );

	Vector3D vecTopLeft( -1.0, -1.0, -1.0, 1.0 );
	Vector3D vecBottomRight( +1.0, +1.0, -1.0, 1.0 );

	vecTopLeft.set( projMatInv * vecTopLeft );
	vecBottomRight.set( projMatInv * vecBottomRight );
	vecTopLeft.normalizeW();
	vecBottomRight.normalizeW();

	Vector3D vecDiag = vecTopLeft-vecBottomRight;
	rRealWidth  = fabs( vecDiag.getX() );
	rRealHeight = fabs( vecDiag.getY() );
	//cout << "[MeshWidget::" << __FUNCTION__ << "] diff realWidth:  " << *rRealWidth  << endl;
	//cout << "[MeshWidget::" << __FUNCTION__ << "] diff realHeight: " << *rRealHeight << endl;

	return( true );
}

//! Returns the pixel size in world coordinates.
//! For perspective projection the back-plane is used.
//! @returns false in case of an error. true otherwise.
bool MeshWidget::getViewPortPixelWorldSize(
                double& rPixelWidth,
                double& rPixelHeight
) const {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	// DEPRECATED
	//-------------------------------------------------------------------------------------------------
	//GLint    viewport[4];
	//GLdouble modelview[16];
	//GLdouble projection[16];
	//GLfloat  winZ = 1.0 - FLT_EPSILON;
	//GLdouble posX1, posY1, posZ1;
	//GLdouble posX2, posY2, posZ2;
	//GLdouble posX3, posY3, posZ3;
	//.
	//glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	//glGetDoublev( GL_PROJECTION_MATRIX, projection );
	//glGetIntegerv( GL_VIEWPORT, viewport );
	//gluUnProject( 0,           0, winZ, modelview, projection, viewport, &posX1, &posY1, &posZ1 );
	//gluUnProject( 0, viewport[3], winZ, modelview, projection, viewport, &posX2, &posY2, &posZ2 );
	//gluUnProject( viewport[2], 0, winZ, modelview, projection, viewport, &posX3, &posY3, &posZ3 );
	//Vector3D viewPortWideSide( posX3-posX1, posY3-posY1, posZ3-posZ1, 0.0 );
	//Vector3D viewPortHighSide( posX2-posX1, posY2-posY1, posZ2-posZ1, 0.0 );
	//(*pixelWidth)  = viewPortWideSide.getLength3() / width();
	//(*pixelHeight) = viewPortHighSide.getLength3() / height();
	//-------------------------------------------------------------------------------------------------
	
	double realWidth;
	double realHeight;
	if( !getViewPortResolution( realWidth, realHeight ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getViewPortResolution failed!" << endl;
		return( false );
	}

	rPixelWidth  = realWidth/width();
	rPixelHeight = realHeight/height();
	return( true );
}

//! Returns the DPI of the viewport
//! 
//! @param rDPI return value of the DPI.
//! @returns false in case of an error
bool MeshWidget::getViewPortDPI( double& rDPI ) const
{
	double pixelWidth;
	double pixelHeight;
	if(!getViewPortPixelWorldSize(pixelWidth, pixelHeight))
		return false;

	rDPI = 25.4/pixelWidth;
	return true;
}

//! Returns the dots per meter of the viewport
//! 
//! @param rDPM return value of the DPM
//! @returns false in case of an error
bool MeshWidget::getViewPortDPM( double& rDPM ) const
{
	double pixelWidth;
	double pixelHeight;
	if(!getViewPortPixelWorldSize(pixelWidth, pixelHeight))
		return false;

	rDPM = 1000.0/pixelWidth;
	return true;
}

//! Set flag controlling the display of Primitives, etc.
//! @returns true when the flag was changed. false otherwise.
bool MeshWidget::setParamFlagMeshWidget( MeshWidgetParams::eParamFlag rFlagNr, bool rState ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	//cout << "[MeshWidget::" << __FUNCTION__ << "] " << rFlagNr << " : " << rState << endl;
	if( !MeshWidgetParams::setParamFlagMeshWidget( rFlagNr, rState ) ) {
		return false;
	}
	if( rFlagNr == ORTHO_MODE ) {
		// Print resolution (in ortho mode):
		if( rState ) {
			double dpi;
			if( getViewPortDPI( dpi ) ) {
				emit sViewPortInfo( VPINFO_DPI, QString::number( dpi, 'f', 2 ) );
			} else {
				emit sViewPortInfo( VPINFO_DPI, QString( "err" ) );
			}
		} else {
			emit sViewPortInfo( VPINFO_DPI, QString( "n.a." ) );
		}
	}
	return paramFlagChanged( rFlagNr );
}

//! Inverts the given flag controlling the display of Primitives, etc.
//! @returns false in case of an error, true otherwise.
bool MeshWidget::toggleShowFlag( MeshWidgetParams::eParamFlag rFlagNr ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	if( !MeshWidgetParams::toggleShowFlag( rFlagNr ) ) {
		return false;
	}
	return paramFlagChanged( rFlagNr );
}

//! Let the user set integer values controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise.
bool MeshWidget::setParamIntegerMeshWidget( MeshWidgetParams::eParamInt rParam ) {
	cout << __PRETTY_FUNCTION__ << endl;
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	switch( rParam ) {
		case VIDEO_FRAME_WIDTH: {
			int paramWidth;
			int paramHeight;
			getParamIntegerMeshWidget( VIDEO_FRAME_WIDTH,  &paramWidth  );
			getParamIntegerMeshWidget( VIDEO_FRAME_HEIGHT, &paramHeight );
			QGMDialogEnterText dlgEnterTxt;
			dlgEnterTxt.setWindowTitle( tr( "Edit scene size (width height):" ) );
			dlgEnterTxt.setText( QString( "%1 %2" ).arg( paramWidth ).arg( paramHeight ) );
			if( dlgEnterTxt.exec() == QDialog::Rejected ) {
				cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: user cancel!" << endl;
				return false;
			}
			vector<long> paramValues;
			if( !dlgEnterTxt.getText( paramValues ) ) {
				cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: bad input (1)!" << endl;
				return false;
			}
			if( paramValues.size() != 2 ) {
				cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: bad input (2)!" << endl;
				return false;
			}
			MeshWidget::setParamFlagMeshWidget( MeshWidgetParams::VIDEO_FRAME_FIXED, false );
			MeshWidget::setParamIntegerMeshWidget( VIDEO_FRAME_WIDTH,  paramValues[0] );
			MeshWidget::setParamIntegerMeshWidget( VIDEO_FRAME_HEIGHT, paramValues[1] );
			MeshWidget::setParamFlagMeshWidget( MeshWidgetParams::VIDEO_FRAME_FIXED, true );
			return true;
		    }
		default: {
			// Ask for the single integer parameter
			int paramValue;
			getParamIntegerMeshWidget( rParam, &paramValue );
			QGMDialogEnterText dlgEnterTxt;
			dlgEnterTxt.setWindowTitle( tr( "Edit parameter") + QString(" (%1):" ).arg( rParam ) );
			dlgEnterTxt.setInt( paramValue );
			if( dlgEnterTxt.exec() == QDialog::Rejected ) {
				cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: user cancel!" << endl;
				return false;
			}
			if( !dlgEnterTxt.getText( &paramValue ) ) {
				cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: bad input!" << endl;
				return false;
			}
			return MeshWidget::setParamIntegerMeshWidget( rParam, paramValue );
			}
	}
	LOG::debug() << "[MeshWidget::" << __FUNCTION__ << "] ERROR: undefined!\n";
	return false;
}

//! Let the user set floating point parameter values controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise i.e. user cancel.
bool MeshWidget::setParamFloatMeshWidget( MeshWidgetParams::eParamFlt rParamID, double rMinValue, double rMaxValue ) {
	double currValue;
	if( !getParamFloatMeshWidget( rParamID, &currValue ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Could not get the current value for '" << rParamID << "'!" << endl;
		return false;
	}
	// Handle exception i.e. log
	if( rParamID ==  MeshWidgetParams::MATERIAL_SHININESS ) {
		currValue = log(currValue+1.0);
	}
	QGMDialogSliderHD dlgSlider;
	dlgSlider.setIdx( rParamID );
	dlgSlider.setMin( rMinValue );
	dlgSlider.setMax( rMaxValue );
	dlgSlider.setPos( currValue );
	dlgSlider.setWindowTitle( tr( "Floating point parameter") + QString(" %1 [%1,%2]" ).arg( rParamID ).arg( rMinValue ).arg( rMaxValue ) );
	//dlgSlider.suppressPreview();
	QObject::connect( &dlgSlider, &QGMDialogSliderHD::valuePreviewIdFloat, this, &MeshWidget::setParamFloatMeshWidgetSlider );
	if( dlgSlider.exec() == QDialog::Rejected ) {
		return false;
	}
	double newValue;
	if( !dlgSlider.getValue( &newValue ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Could not get the new value for '" << rParamID << "'!" << endl;
		return false;
	}
	bool retVal = setParamFloatMeshWidget( rParamID, newValue );
	return retVal;
}

//! Let the user set floating point parameter values controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise i.e. user cancel.
bool MeshWidget::setParamFloatMeshWidget( MeshWidgetParams::eParamFlt rParamID ) {
	double currValue;
	if( !getParamFloatMeshWidget( rParamID, &currValue ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Could not get the current value for '" << rParamID << "'!" << std::endl;
		return false;
	}
	// Handle exception i.e. log
	if( rParamID ==  MeshWidgetParams::MATERIAL_SHININESS ) {
		currValue = log(currValue+1.0);
	}
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setID( rParamID );
	dlgEnterText.setDouble( currValue );
	dlgEnterText.setWindowTitle( tr( "Floating point parameter") + QString(" %1" ).arg( rParamID ) );
	//dlgSlider.suppressPreview();
	//QObject::connect( &dlgSlider, SIGNAL(valuePreview(int,double)),  this, SLOT(setParamFloatMeshWidget(int,double)) );
	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return false;
	}
	double newValue;
	if( !dlgEnterText.getText( &newValue ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Could not get the new value for '" << rParamID << "'!" << std::endl;
		return false;
	}
	bool retVal = setParamFloatMeshWidget( rParamID, newValue );
	return retVal;
}

//! Set integer values controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise.
bool MeshWidget::setParamIntegerMeshWidget( MeshWidgetParams::eParamInt rParam, int rValue ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( !MeshWidgetParams::setParamIntegerMeshWidget( rParam, rValue ) ) {
		return false;
	}

	switch( rParam ) {
		case STATE_NUMBER:
			//! \todo implent check for parameters.
			break;
		case MOUSE_MODE:
			if( rValue == MeshWidgetParams::MOUSE_MODE_MOVE_PLANE ||
			    rValue == MeshWidgetParams::MOUSE_MODE_MOVE_PLANE_AXIS ||
			    rValue == MeshWidgetParams::MOUSE_MODE_ROTATE_PLANE_AXIS) {
				emit sParamFlagMesh( MeshGLParams::SHOW_MESH_PLANE_TEMP, true );
			} else {
				emit sParamFlagMesh( MeshGLParams::SHOW_MESH_PLANE_TEMP, false );
			}
			break;
		case SELECTION_MODE:
			switch( static_cast<MeshWidgetParams::eSelectionModes>(rValue) ) {
				case MeshWidgetParams::SELECTION_MODE_NONE:
					emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_NONE );
				break;
				case MeshWidgetParams::SELECTION_MODE_VERTEX:
					emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SELPRIM_VERTEX );
				break;
				case MeshWidgetParams::SELECTION_MODE_FACE:
					emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SELPRIM_FACE );
				break;
				case MeshWidgetParams::SELECTION_MODE_VERTICES:
					emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SELMVERTS );
				break;
				case MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO:
					emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SELMVERTS_LASSO );
				break;
                case MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO:
                    emit sGuideIDSelection( MeshWidgetParams::GUIDE_DESELECT_SELMVERTS_LASSO );
                break;
				case MeshWidgetParams::SELECTION_MODE_MULTI_FACES:
					emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_SELMFACES );
				break;
				case MeshWidgetParams::SELECTION_MODE_PLANE_3FP: {
					int posID;
					mMeshVisual->getPlanePosToSet( &posID ); // This will emit a signal for the user guide on the sidebar
					mMeshVisual->setParamFlagMeshGL( MeshGLParams::SHOW_MESH_PLANE, true ); // Show the mesh plane.
				} break;
				case MeshWidgetParams::SELECTION_MODE_CONE:
					mMeshVisual->getConeStatus(); // This will emit a signal for the user guide on the sidebar
				break;
				case MeshWidgetParams::SELECTION_MODE_SPHERE:
					mMeshVisual->getSpherePointIdx(); // This will emit a signal for the user guide on the sidebar
				break;
				case MeshWidgetParams::SELECTION_MODE_POSITIONS:
					emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_POSITIONS );
				break;
                case MeshWidgetParams::SELECTION_MODE_THREE_POSITIONS:
                    emit sGuideIDSelection( MeshWidgetParams::GUIDE_SELECT_THREE_POSITIONS );
                break;
				default:
					// do nothing
					cerr << "[MeshWidget::" << __FUNCTION__ << "] SELECTION_MODE unknown paramNr: " << rParam << " val: " << rValue << endl;
				break;
			}
			break;
		case HISTOGRAM_TYPE:
			break;
		default:
			// Do nothing.
			//cerr << "[MeshWidget::" << __FUNCTION__ << "] unknown paramNr: " << rParam << " val: " << rValue << endl;
			break;
	}
	emit sParamIntegerMeshWidget( rParam, rValue );

	setView();
	update();
	return true;
}

//! Only to be used for dialogSlider+Preview:
//! Set floating point value controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise.
bool MeshWidget::setParamFloatMeshWidgetSlider( int rParamID, double rValue ) {
	return MeshWidget::setParamFloatMeshWidget( static_cast<MeshWidgetParams::eParamFlt>(rParamID), rValue );
}


//! New style function call by signal.
//!
//! @returns false in case of an error. True otherwise.
bool MeshWidget::callFunctionMeshWidget( MeshWidgetParams::eFunctionCall rFunctionID, bool rFlagOptional ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] ID: " << rFunctionID << std::endl;
#endif
	bool retVal = true;

	switch( rFunctionID ) {
		case EXPORT_POLYLINES_INTERSECT_PLANE:
			retVal &= exportPlaneIntersectPolyLinesSVG();
			break;
		case SCREENSHOT_CURRENT_VIEW_SINGLE:
			retVal &= screenshotSingle();
			break;
		case SCREENSHOT_CURRENT_VIEW_SINGLE_PDF:
			retVal &= screenshotPDFUser();
			break;
		case SCREENSHOT_VIEWS_IMAGES:
			screenshotViews(); //! \todo change to boolean and add 'retVal &= '
			break;
		case SCREENSHOT_VIEWS_PDF:
			retVal &= screenshotViewsPDFUser();
			break;
		case SCREENSHOT_VIEWS_DIRECTORY:
			retVal &= screenshotViewsDirectory();
			break;
		case DIRECTORY_FUNCVAL_TO_RGB:
			retVal &= directoryFuncValToRGB();
			break;
		case EDIT_SET_CONEAXIS_CENTRALPIXEL:
			retVal &= userSetConeAxisCentralPixel();
			break;
		case SET_CURRENT_VIEW_TO_DEFAULT:
			retVal &= currentViewToDefault();
			break;
		case SET_ORTHO_DPI:
			retVal &= orthoSetDPI();
			break;
		case SET_RENDER_DEFAULT:
			retVal &= setParamFlagMeshWidget(  LIGHT_ENABLED,               true                       ); // Turn on the light
			retVal &= setParamFloatMeshWidget( MATERIAL_SHININESS,          MATERIAL_SHININESS_DEFAULT ); // Default
			retVal &= setParamFloatMeshWidget( MATERIAL_SPECULAR,           MATERIAL_SPECULAR_DEFAULT  ); // Default
			retVal &= setParamFloatMeshWidget( LIGHT_FIXED_CAM_INTENSITY,   1.000                      ); // Default
			retVal &= setParamFloatMeshWidget( LIGHT_FIXED_WORLD_INTENSITY, 1.000                      ); // Default
			retVal &= setParamFloatMeshWidget( AMBIENT_LIGHT,               AMBIENT_LIGHT_DEFAULT      ); // Default
			break;
		case SET_RENDER_MATTED:
			retVal &= setParamFlagMeshWidget(  LIGHT_ENABLED,               true                       ); // Turn on the light
			retVal &= setParamFloatMeshWidget( MATERIAL_SHININESS,          MATERIAL_SHININESS_DEFAULT );
			retVal &= setParamFloatMeshWidget( MATERIAL_SPECULAR,           0.0                        ); // Turn specular off
			retVal &= setParamFloatMeshWidget( LIGHT_FIXED_CAM_INTENSITY,   1.33                       ); // +33% Brightness
			retVal &= setParamFloatMeshWidget( LIGHT_FIXED_WORLD_INTENSITY, 1.33                       ); // +33% Brightness
			retVal &= setParamFloatMeshWidget( AMBIENT_LIGHT,               0.025                      ); // lower ambient than default
			break;
		case SET_RENDER_METALLIC:
			retVal &= setParamFlagMeshWidget(  LIGHT_ENABLED,               true                       ); // Turn on the light
			retVal &= setParamFloatMeshWidget( MATERIAL_SHININESS,          2.42991                    );
			retVal &= setParamFloatMeshWidget( MATERIAL_SPECULAR,           0.4                        );
			retVal &= setParamFloatMeshWidget( LIGHT_FIXED_CAM_INTENSITY,   1.25                       ); // +25% Brightness
			retVal &= setParamFloatMeshWidget( LIGHT_FIXED_WORLD_INTENSITY, 1.33                       ); // +25% Brightness
			retVal &= setParamFloatMeshWidget( AMBIENT_LIGHT,               AMBIENT_LIGHT_DEFAULT      );
			break;
		case SET_RENDER_LIGHT_SHADING:
			retVal &= setParamFlagMeshWidget(  LIGHT_ENABLED,               true                       ); // Turn on the light
			retVal &= setParamFloatMeshWidget( MATERIAL_SHININESS,          MATERIAL_SHININESS_DEFAULT );
			retVal &= setParamFloatMeshWidget( MATERIAL_SPECULAR,           0.0                        ); // Turn specular off
			retVal &= setParamFloatMeshWidget( LIGHT_FIXED_CAM_INTENSITY,   0.45                       ); // Dimmed
			retVal &= setParamFloatMeshWidget( LIGHT_FIXED_WORLD_INTENSITY, 0.45                       ); // Dimmed
			retVal &= setParamFloatMeshWidget( AMBIENT_LIGHT,               0.65                       ); // Strong ambient
			break;
		case SET_RENDER_FLAT_AND_EDGES:
			mMeshVisual->setParamFlagMeshGL( MeshGLParams::SHOW_SMOOTH, not( rFlagOptional ) );
			mMeshVisual->setParamFlagMeshGL( MeshGLParams::SHOW_FACES_EDGES, rFlagOptional );
			break;
		case SET_GRID_NONE: {
			bool gridRect, gridPolLin, gridPolCirc;
			retVal &= getParamFlagMeshWidget( SHOW_GRID_RECTANGULAR,   &gridRect    );
			retVal &= getParamFlagMeshWidget( SHOW_GRID_POLAR_LINES,   &gridPolLin  );
			retVal &= getParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, &gridPolCirc );
			if( gridRect | gridPolLin | gridPolCirc ) {
				// Turn off whatever grid is active.
				retVal &= setParamFlagMeshWidget( SHOW_GRID_RECTANGULAR,   false );
				retVal &= setParamFlagMeshWidget( SHOW_GRID_POLAR_LINES,   false );
				retVal &= setParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, false );
			} else {
				// When there is no grid active toggle to rectangular grid.
				retVal &= setParamFlagMeshWidget( SHOW_GRID_RECTANGULAR,   true  );
				retVal &= setParamFlagMeshWidget( SHOW_GRID_POLAR_LINES,   false );
				retVal &= setParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, false );
			}
			} break;
		case SET_GRID_RASTER:
			retVal &= setParamFlagMeshWidget( SHOW_GRID_RECTANGULAR,   true  );
			retVal &= setParamFlagMeshWidget( SHOW_GRID_POLAR_LINES,   false );
			retVal &= setParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, false );
			break;
		case SET_GRID_POLAR:
			retVal &= setParamFlagMeshWidget( SHOW_GRID_RECTANGULAR,   false );
			retVal &= setParamFlagMeshWidget( SHOW_GRID_POLAR_LINES,   true  );
			retVal &= setParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, true  );
			break;
		case SET_VIEW_AXIS_UP:
			retVal &= setViewAxisUp();
			break;
		case SET_VIEW_PARAMETERS:
			retVal &= setViewMatrix();
			break;
		case SHOW_VIEW_PARAMETERS:
			retVal &= showViewMatrix();
			break;
		case SHOW_VIEW_2D_BOUNDING_BOX:
			retVal &= showView2DBoundingBox();
			break;
		default:
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] Function Call Id: " << rFunctionID << std::endl;
			retVal = false;
	}

	return( retVal );
}


//! Set floating point value controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise.
bool MeshWidget::setParamFloatMeshWidget( MeshWidgetParams::eParamFlt rParamID, double rValue ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	bool redrawScene = false;
	// 1. set and check parameter
	if( !MeshWidgetParams::setParamFloatMeshWidget( rParamID, rValue ) ) {
		return false;
	}
	// 2. adapt
	switch( rParamID ) {
		case GRID_SHIFT_DEPTH:
		case ORTHO_ZOOM: {
			double dpi;
			if( getViewPortDPI(dpi) ) {
				emit sViewPortInfo( VPINFO_DPI, QString::number( dpi, 'f', 2 ) );
			} else {
				emit sViewPortInfo( VPINFO_DPI, QString( "err" ) );
			}
			redrawScene = true;
			} break;
		case ORTHO_SHIFT_HORI:
		case ORTHO_SHIFT_VERT:
		case FOV_ANGLE:
		case AMBIENT_LIGHT:
		case MATERIAL_SPECULAR:
            redrawScene = true;
			break;
		case LIGHT_FIXED_WORLD_ANGLE_PHI:
		case LIGHT_FIXED_WORLD_ANGLE_THETA:
			// Show direction of the light fixed to the object position:
			emit sViewPortInfo( VPINFO_LIGHT_WORLD, QString::number( mParamFlt[LIGHT_FIXED_WORLD_ANGLE_THETA], 'f', 1 ) + " / " + 
			                                        QString::number( mParamFlt[LIGHT_FIXED_WORLD_ANGLE_PHI], 'f', 1 ) );
			redrawScene = true;
			break;
		case LIGHT_FIXED_CAM_INTENSITY:
            redrawScene = true;
			break;
		case LIGHT_FIXED_CAM_ANGLE_PHI:
		case LIGHT_FIXED_CAM_ANGLE_THETA:
			// Show direction of the light fixed to the camera position:
			emit sViewPortInfo( VPINFO_LIGHT_CAM, QString::number( mParamFlt[LIGHT_FIXED_CAM_ANGLE_THETA], 'f', 1 ) + " / " + 
			                                      QString::number( mParamFlt[LIGHT_FIXED_CAM_ANGLE_PHI], 'f', 1 ) );
			redrawScene = true;
			break;
		case LIGHT_FIXED_WORLD_INTENSITY:
		case MATERIAL_SHININESS:
			// updateGL requrired.
			redrawScene = true;
			break;
		default:
			// Do nothing
			break;
	}
	// 3. optional: draw OpenGL scene with new parameters, when the parameter influences the visualization
	if( redrawScene ) {
		setView();
		update();
	}
	return true;
}

//! Set strings controlling the display of Primitives, etc.
//! @returns true when the string was changed. false otherwise.
bool MeshWidget::setParamStringMeshWidget( const MeshWidgetParams::eParamStr rParamID, const string& rString ) {
	//cout << "[MeshWidget::" << __FUNCTION__ << "] " << showFlagNr << " : " << setState << endl;
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( !MeshWidgetParams::setParamStringMeshWidget( rParamID, rString ) ) {
		return false;
	}
	setView();
	update();
	return true;
}

//! Handles signals, when flags were changed.
//! @returns false in case of an error, true otherwise.
bool MeshWidget::paramFlagChanged( const eParamFlag rFlagNr ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	bool flagState;
	if( !getParamFlagMeshWidget( rFlagNr, &flagState ) ) {
		return false;
	}
	emit sParamFlagMeshWidget( rFlagNr, flagState );
	switch( rFlagNr ) {
		case SHOW_FOG:
			break;
		case LIGHT_ENABLED:
			if( flagState ) {
				emit sStatusMessage( "Light On." );
			} else {
				emit sStatusMessage( "Light Off." );
			}
			break;
		case LIGHT_FIXED_CAM:
			break;
		case LIGHT_FIXED_WORLD:
			break;
		case LIGHT_AMBIENT:
			if( flagState ) {
				emit sStatusMessage( "Ambient light ON." );
			} else {
				emit sStatusMessage( "Ambient light OFF." );
			}
			break;
		case SHOW_HISTOGRAM:
		case SHOW_HISTOGRAM_LOG:
			break;
		default:
			// Do nothing.
			break;
	}
	setView();
	update();
	return true;
}

// --- SLOTS -----------------------------------------------------------------------

//! Removes the actual Mesh from the memory and loads a new Mesh from a given file.
bool MeshWidget::fileOpen( const QString& fileName ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif

	//check if child dialogs exist an close them
	foreach (QObject* obj, this->children()) {
		if(obj->isWidgetType())
		{
			QWidget* widget = static_cast<QWidget*>(obj);
			widget->close();
		}
	}



	//emit statusMessage( "Loading Mesh from " + fileName );
	if( mMeshVisual != nullptr ) {
		// remove mesh, when one is present:
		delete mMeshVisual;
		mMeshVisual = nullptr;
	}

	bool readOk;

	mMeshVisual = new MeshQt( fileName, readOk,
							  mMatModelView.constData(), mMatProjection.constData(),
							  static_cast<MeshWidgetParams*>(this), static_cast<MeshGLColors*>(this),
							  mMainWindow, context()
	                         );

	if( not( readOk ) ) {
		if( mMeshVisual != nullptr ) {
			delete mMeshVisual;
			mMeshVisual = nullptr;
		}
		return false;
	}

    bool continueWithMesh;
    bool userCancel;
    uint64_t nrOfFaces = mMeshVisual->getFaceNr();
    if (nrOfFaces > 35000000){
        SHOW_QUESTION( tr("You are loading a large mesh"), tr("This may cause a crash if there is not enough video RAM available.") + QString("<br /><br />") + tr("Do you want to continue?"), continueWithMesh, userCancel );
        if( !continueWithMesh || userCancel ) {
            //stop rendering the mesh to prevent a crash
            delete mMeshVisual;
            mMeshVisual = nullptr;
            return false;
        }
    }

	QObject::connect( this,        SIGNAL(sParamFlagMesh(MeshGLParams::eParamFlag,bool)), mMeshVisual, SLOT(setParamFlagMeshGL(MeshGLParams::eParamFlag,bool)) );
    QObject::connect( this,        SIGNAL(sSelectPoly(std::vector<QPoint>&)),                  mMeshVisual, SLOT(selectPoly(std::vector<QPoint>&)) );
    QObject::connect( this,        SIGNAL(sDeSelectPoly(std::vector<QPoint>&)),                  mMeshVisual, SLOT(deselectPoly(std::vector<QPoint>&)) );
	QObject::connect( mMeshVisual, SIGNAL(updateGL()),                                    this,        SLOT(update())                                          );
	// Interaction -----------------------------------------------------------------------------------------------------------------------------------------
	QObject::connect( this,        SIGNAL(sApplyTransfromToPlane(Matrix4D)), mMeshVisual, SLOT(applyTransfromToPlane(Matrix4D)) );
	QObject::connect( mMeshVisual, &MeshQt::sDefaultViewLight,               this,        &MeshWidget::defaultViewLight         );
	QObject::connect( mMeshVisual, &MeshQt::sDefaultViewLightZoom,           this,        &MeshWidget::defaultViewLightZoom     );
    QObject::connect( mMeshVisual, &MeshQt::sSetDefaultView,                 this,        &MeshWidget::currentViewToDefault    );
    QObject::connect( mMeshVisual, &MeshQt::sReloadFile,                     this,        [this]{ reloadFile(false); });
	// -----------------------------------------------------------------------------------------------------------------------------------------------------

	// cheks mesh for problems and fix them
	checkMeshSanity();

	// Guess some initial distance for fog:
	double bboxLength = mMeshVisual->getBoundingBoxRadius() * 2.0;

	setParamFloatMeshWidget( FOG_LINEAR_START, bboxLength  * 0.75);
	setParamFloatMeshWidget( FOG_LINEAR_END,   bboxLength  * 1.75);

	// Setup the dynamic menu by emitting flags:
	cout << "[MeshWidget::" << __FUNCTION__ << "] Sending initial signals with flags and parameters ..." << endl;
	for( int i=MeshWidgetParams::PARAMS_FLAG_UNDEFINED; i<MeshWidgetParams::PARAMS_FLAG_COUNT; i++ ) {
		bool paramFlag;
		getParamFlagMeshWidget( static_cast<MeshWidgetParams::eParamFlag>(i), &paramFlag );
		emit sParamFlagMeshWidget( static_cast<MeshWidgetParams::eParamFlag>(i), paramFlag );
	}
	for( int i=MeshWidgetParams::PARAMS_INT_UNDEFINED; i<MeshWidgetParams::PARAMS_INT_COUNT; i++ ) {
		int paramValueInt;
		getParamIntegerMeshWidget( static_cast<MeshWidgetParams::eParamInt>(i), &paramValueInt );
		emit sParamIntegerMeshWidget( static_cast<MeshWidgetParams::eParamInt>(i), paramValueInt );
	}
	for( int i=MeshWidgetParams::PARAMS_FLT_UNDEFINED; i<MeshWidgetParams::PARAMS_FLT_COUNT; i++ ) {
		double paramValueFloat;
		getParamFloatMeshWidget( static_cast<MeshWidgetParams::eParamFlt>(i), &paramValueFloat );
		//! \todo add: emit sParamFloatMeshWidget( (MeshWidgetParams::eParamFlt)i, paramValueFloat );
	}
	// Initial update of the sidebar
	// Show direction of the light fixed to the object position:
	emit sViewPortInfo( VPINFO_LIGHT_WORLD, QString::number( mParamFlt[LIGHT_FIXED_WORLD_ANGLE_THETA], 'f', 1 ) + " / " +
	                                        QString::number( mParamFlt[LIGHT_FIXED_WORLD_ANGLE_PHI], 'f', 1 ) );
	// Show direction of the light fixed to the camera position:
	emit sViewPortInfo( VPINFO_LIGHT_CAM, QString::number( mParamFlt[LIGHT_FIXED_CAM_ANGLE_THETA], 'f', 1 ) + " / " +
	                                      QString::number( mParamFlt[LIGHT_FIXED_CAM_ANGLE_PHI], 'f', 1 ) );
	// Go to default selection - toggle otherwise signals won't be sendt:
	setParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, MeshWidgetParams::SELECTION_MODE_NONE );
	setParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, MeshWidgetParams::SELECTION_MODE_VERTEX );

	// Update OpenGL context to default view
	qglClearColor( Qt::white );
	defaultViewLightZoom();

	//should be valid for QGLContext and QGL* classes. If upgraded to QOpenGLContext, then use context()->defaultFramebufferObject() instead of 0
	mMeshVisual->setParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, 0);

	// Initialize Pin size depending on the bounding box diagonal
	Vector3D boundingbox;
	mMeshVisual->getBoundingBoxSize( boundingbox );
	double boundIngBoxDiag = sqrt( pow( boundingbox.getX(), 2.0 ) + pow( boundingbox.getY(), 2.0 ) +
	                               pow( boundingbox.getZ(), 2.0 ) );
	mMeshVisual->setParamFloatMeshGL( MeshGLParams::PIN_SIZE,
	                                  boundIngBoxDiag / 75.0 );  //75.0 magick number, seems to work well for a good pinSize

	cout << "[MeshWidget::" << __FUNCTION__ << "] Done." << endl;

	emit loadedMeshIsTextured( mMeshVisual->getModelMetaDataRef().hasTextureCoordinates() && mMeshVisual->getModelMetaDataRef().hasTextureFiles() );
	return( true );
}

//! Reloads the current file from disk.
bool MeshWidget::reloadFile(const bool askQuestion) {
	if( mMeshVisual == nullptr ) {
		return false;
	}
    if(askQuestion){
        bool userReload;
        bool userCancel;
        SHOW_QUESTION( tr("Reload file"), tr("Do you really want to reload this file?"), userReload, userCancel );
        if( ( userCancel ) || not( userReload ) ) {
            return false;
        }
    }
	auto fileName = mMeshVisual->getFullName();
    return fileOpen( QString::fromStdWString( fileName.wstring() ) );
}

//! Ask to turn off settings, which usually are hindering the rendering of image stacks,
//! @returns false, when the user selected cancel.
bool MeshWidget::saveStillImagesSettings() {
	//! .) Ask for OrthoGrid (if on)
	bool orthoMode;
	getParamFlagMeshWidget( ORTHO_MODE, &orthoMode );
	if( orthoMode ) {
		bool showGridRect;
		bool showGridPolarLines;
		bool showGridPolarCircles;
		getParamFlagMeshWidget( SHOW_GRID_RECTANGULAR, &showGridRect );
		getParamFlagMeshWidget( SHOW_GRID_POLAR_LINES, &showGridPolarLines );
		getParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, &showGridPolarCircles );
		if( showGridRect | showGridPolarLines | showGridPolarCircles ) {
			bool orthoGridOff;
			bool userCancel;
			SHOW_QUESTION( tr("Rectangular/Polar Grid"), tr("Do you want to turn off the grid?") + QString("<br /><br />") + tr("Recommended: YES"), orthoGridOff, userCancel );
			if( userCancel ) {
				return false;
			}
			if( orthoGridOff ) {
				setParamFlagMeshWidget( SHOW_GRID_RECTANGULAR, false );
				setParamFlagMeshWidget( SHOW_GRID_POLAR_LINES, false );
				setParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, false );
			}
            else{
                //render the images with the grid --> no transparency
                setParamFlagMeshWidget( SCREENSHOT_PNG_BACKGROUND_OPAQUE, true );
            }
		}
	}
	//! .) Ask for Cropping (if on)
	bool cropScreenshots;
	getParamFlagMeshWidget( CROP_SCREENSHOTS, &cropScreenshots );
	if( cropScreenshots ) {
		bool croppingOff;
		bool userCancel;
		SHOW_QUESTION( tr("Cropping of screenshots"), tr("Do you want to turn off cropping?")+ QString("<br /><br />") + tr("Recommended: YES"), croppingOff, userCancel );
		if( userCancel ) {
			return false;
		}
		if( croppingOff ) {
			setParamFlagMeshWidget( CROP_SCREENSHOTS, false );
		}
	}


    //ask for ttl metadata export
    bool ttlMetadataExport;
    bool userCancel;
    SHOW_QUESTION( tr("TTL Metadata"), tr("Do you want to save the metadata per image as .ttl ?"), ttlMetadataExport, userCancel );
    if( userCancel ) {
        return false;
    }
    setParamFlagMeshWidget( EXPORT_TTL_WITH_PNG, ttlMetadataExport );

    return true;
}

//! Screenshots for a video - 360° rotation about a given center and a given normal.
void MeshWidget::saveStillImages360( Vector3D rotCenter, Vector3D rotAxis ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif

    QString filePath = QString::fromStdWString( mMeshVisual->getFileLocation().wstring() );
	QString savePath = QFileDialog::getExistingDirectory(mMainWindow, tr( "Folder for image stack" ),
													 filePath);

	if(savePath.length() == 0)
	{
		std::cout << "[MeshWidget::" << __FUNCTION__ << "] user abort\n";
		return;
	}

	if(savePath[savePath.length() - 1] != '/')
		savePath.push_back('/');

	if( !saveStillImagesSettings() ) {
		return;
	}

	//! .) Ask for tiled rendering
	bool useTiled;
	bool userCancel;
	SHOW_QUESTION( tr("Tiled rendering"), tr("Do you want to use tiled rendering?") + QString("<br /><br />") + tr("Typically: NO"), useTiled, userCancel );
	if( userCancel ) {
		return;
	}

	//! .) Estimate angles using VIDEO_SLOW_STARTSTOP, VIDEO_FRAMES_PER_SEC and VIDEO_DURATION
	float* stepAngles;
	int    stepAnglesNr;
	saveStillImages360Angles( &stepAngles, &stepAnglesNr );
	//! .) Save sequence of still images
	for( int i=0; i<stepAnglesNr; i++ ) {
		rotArbitAxis( rotCenter, rotAxis, stepAngles[i]*180.0/M_PI );

		QString fileName = QString("%1gigamesh_still_image_%2.png").arg(savePath).arg(i,5,10,QChar('0'));
		double realWidth, realHeigth;
		screenshotSingle( fileName, useTiled, realWidth, realHeigth );
	}
	delete[] stepAngles;
    //reset the png transparency setting
    setParamFlagMeshWidget( SCREENSHOT_PNG_BACKGROUND_OPAQUE, false );
    //reset the ttl export setting
    setParamFlagMeshWidget( EXPORT_TTL_WITH_PNG, true );
}

//! Screenshots for a video - 360° horizonal rotation from left to right.
//! Typical for pottery videos.
//!
//! Hint: ffmpeg -i gigamesh_still_image_%05d.png -s 720x576 -b 4000k test.mpg
void MeshWidget::saveStillImages360HLR() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
    Vector3D cameraPitchAxis( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 );
    saveStillImages360( mCenterView, cameraPitchAxis );
}

//! Screenshots for a video - 360° vertical rotation upwards.
//! Typical for cuneiform tablet videos.
void MeshWidget::saveStillImages360VUp() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
    //05.12.22 E.S code changed with saveStillImages360HLR
    saveStillImages360( mCenterView, mCameraUp );
}

//! Screenshots for a video - 360° vertical rotation about
//! the normal of a selected primitive.
void MeshWidget::saveStillImages360PrimN() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	Primitive* primSel = mMeshVisual->getPrimitiveSelected();
	if( primSel == nullptr ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] No primitive selected!" << endl;
		return;
	}
	Vector3D primNormal = primSel->getNormal( true );
	saveStillImages360( mCenterView, primNormal );
}

//! Screenshots for a video - 360° vertical rotation about
//! the plane of the mesh.
void MeshWidget::saveStillImages360PlaneN() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	Vector3D planeHNF;
	if( !mMeshVisual->Mesh::getPlaneHNF( &planeHNF ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] No plane selected!" << endl;
		return;
	}
	planeHNF.setH( 0.0 );
	planeHNF.normalize3();
	saveStillImages360( mCenterView, planeHNF );
}

//! Estimates the parameter (angle in radiant) for a 360° camera movement.
//! stepAnglesNr is typically equal to the number of still images computed.
void MeshWidget::saveStillImages360Angles( float** stepAngles, int* stepAnglesNr ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	double videoDuration;
	double videoFPS;
	double videoSlowStartStop;
	getParamFloatMeshWidget( VIDEO_DURATION, &videoDuration );
	getParamFloatMeshWidget( VIDEO_FRAMES_PER_SEC, &videoFPS );
	getParamFloatMeshWidget( VIDEO_SLOW_STARTSTOP, &videoSlowStartStop );
	(*stepAnglesNr) = floor( videoDuration * videoFPS );
	(*stepAngles)   = new float[(*stepAnglesNr)];
	int    nrImagesStartStop = floor( videoSlowStartStop * videoFPS );
	float  angleCurvIntegral = 0.0f;
	float  gaussVal = 1.0f;
	// Compute the slow start and stop angles:
	for( int i=0; i<nrImagesStartStop; i++ ) {
		gaussVal = 1.0 - exp( -( ( pow( 3.0*static_cast<float>(i)/static_cast<float>(nrImagesStartStop), 2 ) ) / 2.0 ) );
		(*stepAngles)[i] = gaussVal;
		(*stepAngles)[(*stepAnglesNr)-i-1] = gaussVal;
		angleCurvIntegral += 2.0 * gaussVal;
	}
	// Constant angle velocity:
	for( int i=nrImagesStartStop; i<(*stepAnglesNr)-nrImagesStartStop; i++ ) {
		(*stepAngles)[i]   = gaussVal;
		angleCurvIntegral += gaussVal;
	}
	// Normalization to 2*pi (360°)
	for( int i=0; i<(*stepAnglesNr); i++ ) {
		(*stepAngles)[i] *= 2.0 * M_PI / angleCurvIntegral;
		//cout << (*stepAngles)[i]*180.0/M_PI << endl;
	}
}

//! Compute spherical images with moving light (instead of moving the object).
//! (1) Ask for the filename
void MeshWidget::sphericalImagesLight() {
	if( mMeshVisual == nullptr ) {
		return;
	}
	string fileNamePattern;
    getParamStringMeshWidget( FILENAME_EXPORT_VR, &fileNamePattern );
    QString filePath = QString::fromStdWString( mMeshVisual->getFileLocation().wstring() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Save as - Using a pattern for spherical images" ), \
	                                                 filePath + QString( fileNamePattern.c_str() ), \
													 tr( "Image (*.png *.tiff *.tif)" ),
	                                                 nullptr, 
	                                                 QFileDialog::DontUseNativeDialog  ); // Native dialog won't show patterns anymore on recent versions of Qt+Linux.
	if( fileName == nullptr ) {
		return;
	}
	sphericalImagesLight( fileName );
}

//! Compute spherical images with moving light (instead of moving the object).
//! (2) - ask about tiled rendering.
void MeshWidget::sphericalImagesLight( const QString& rFileName ) {
	bool userCancel;
	bool useTiled;
	SHOW_QUESTION( tr("Tiled rendering"), tr("Do you want to use tiled rendering?") + QString("<br /><br />") + tr("Typically: NO"), useTiled, userCancel );
	if( userCancel ) {
		return;
	}
	sphericalImagesLight( rFileName, useTiled );
}

//! Compute spherical images with moving light (instead of moving the object).
//! (3) - ask about the step size.
//! -ask if the user wants to use  a constant theta
void MeshWidget::sphericalImagesLight( const QString& rFileName, const bool rUseTiled  ) {
    double stepLight;
    getParamFloatMeshWidget( LIGHT_STEPPING, &stepLight );

    // Ask user for the light stepping using the current setting
    QGMDialogEnterText dlgEnterTxt;
    int stepLightAngle = stepLight * 180;
    dlgEnterTxt.setWindowTitle( "Set light steps (angle between 1 and 180 degrees)" );
    dlgEnterTxt.setInt( stepLightAngle );
    if( dlgEnterTxt.exec() == QDialog::Rejected ) {
        return;
    }
    if( !dlgEnterTxt.getText( &stepLightAngle ) ) {
        stepLightAngle = stepLight * 180;
    }
    stepLight = stepLightAngle/180.0;

    bool userCancel;
    bool useConstTheta;
    SHOW_QUESTION( tr("Constant Theta"), tr("Do you want to use a constant theta (see spherical coordinate system) ?") + QString("<br /><br />"), useConstTheta, userCancel );
    if( userCancel ) {
        return;
    }

    if( !saveStillImagesSettings() ) {
        return;
    }

    // Execute:
    if(useConstTheta){
        sphericalImagesLightConstTheta(rFileName, rUseTiled, stepLight );
    }else{
        sphericalImagesLight( rFileName, rUseTiled, stepLight );
    }

}


//! Compute spherical images with moving light (instead of moving the object).
//! (4) - execute.
//!
//! String for Object2VR: 'gigamesh_still_image_'+ fill(row,5,'0') + '_' +fill(column,5,'0')+ '.png'
void MeshWidget::sphericalImagesLight( const QString& rFileName, const bool rUseTiled, const double rStepLight ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif

	cout << "[MeshWidget::" << __FUNCTION__ << "] Pattern string for Object2VR: '" << rFileName.toStdString() << "'" << endl;
	int stateNr;
	getParamIntegerMeshWidget( STATE_NUMBER, &stateNr );

	// Store original light position:
	double phiOri;
	double thetaOri;
	getParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_PHI, &phiOri );
	getParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_THETA, &thetaOri );

	char buffer[255];
	int  colIdx = 0;
	int  rowIdx = 0;
	unsigned int imageCount = 0;

    for( float i=-0.5; i<+0.5; i+=rStepLight ) {
		colIdx = 0;
		double dx = sin( i * M_PI );
        for( float j=+0.5; j>-0.5; j-=rStepLight ) {
			sprintf( buffer, rFileName.toStdString().c_str(), colIdx, rowIdx, stateNr );
			double dy = sin( j * M_PI );
			double dz = sqrt( 1 - dx*dx - dy*dy );
			Vector3D dirVec( dx, dy, dz );
			setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_PHI,   dirVec.getSphPhiDeg()   );
            setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_THETA, dirVec.getSphThetaDeg() );
			setView();
			repaint();
			double realWidth, realHeigth;
			screenshotSingle( buffer, rUseTiled, realWidth, realHeigth );
			imageCount++;
			colIdx++;
		}
		rowIdx++;
	}

	// Restore original light position:
	setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_PHI,     phiOri );
	setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_THETA, thetaOri );
	setView();
	repaint();

	//! String for Object2VR: 'gigamesh_still_image_'+ fill(column,5,'0') + '_' +fill(row,5,'0')+ '.png'
	// Information for the user
	int dirFileNameSplitPos = rFileName.lastIndexOf( QDir::separator() );
	QString fileNameWithoutPath = rFileName;
	QString pathFromFileName;
	if( dirFileNameSplitPos > 0 ) {
        fileNameWithoutPath = rFileName.mid( dirFileNameSplitPos+1 );
        pathFromFileName    = rFileName.left( dirFileNameSplitPos );
	}
	QString obj2vrPattern = "'" + fileNameWithoutPath + "'";
	obj2vrPattern.replace( QString( "%05i_%05i_%02i" ), QString( "'+fill(column,5,'0')+'_'+fill(row,5,'0')+'_'+fill(state,2,'0')+'" ) );
	SHOW_MSGBOX_INFO( tr("Spherical image stack saved"),
					  QString( "<table>" ) +
					  QString( "<tr><td>") + tr("Images:")  + QString("</td><td align='right'> %1 / %2</td></tr>" ).arg( imageCount ).arg( colIdx*rowIdx ) +
					  QString( "<tr><td>") + tr("Columns:") + QString("</td><td align='right'> %1</td></tr>" ).arg( colIdx ) +
					  QString( "<tr><td>") + tr("Rows:")    + QString("</td><td align='right'> %1</td></tr>" ).arg( rowIdx ) +
					  QString( "<tr><td>") + tr("State:")   + QString("</td><td align='right'> %1</td></tr>" ).arg( stateNr ) +
					  QString( "<tr><td>") + tr("Path:")    + QString("</td><td align='right'> %1</td></tr>" ).arg( pathFromFileName ) +
					  QString( "<tr><td>") + tr("Pattern:") + QString("</td><td align='right'> %1</td></tr>" ).arg( fileNameWithoutPath ) +
					  QString( "<tr><td><b>") + tr("Pattern&nbsp;(O2VR):") + QString("</b></td><td align='right'><b><nobr>%1</nobr></b></td></tr>" ).arg( obj2vrPattern ) +
					  QString( "</table>" )
	                );
	// Copy the pattern directly to the clipboard:
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText( obj2vrPattern );
	cout << "[MeshWidget::" << __FUNCTION__ << "] Pattern string for Object2VR: " << obj2vrPattern.toStdString() << endl;
	emit sStatusMessage( "Spherical image stack was saved to: " + rFileName );
}
//! Compute spherical images with moving light (instead of moving the object).
//! With a constant Theta
//! (5) - execute.

void MeshWidget::sphericalImagesLightConstTheta( const QString& rFileName, const bool rUseTiled, const double rStepLight ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
    cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif

    cout << "[MeshWidget::" << __FUNCTION__ << "] Pattern string for Object2VR: '" << rFileName.toStdString() << "'" << endl;
    int stateNr;
    getParamIntegerMeshWidget( STATE_NUMBER, &stateNr );

    // Store original light position:
    double phiOri;
    double thetaOri;
    getParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_PHI, &phiOri );
    getParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_THETA, &thetaOri );

    char buffer[255];
    unsigned int imageCount = 0;
    int rStepAngle = rStepLight * 180;
    for( int i=0; i<360; i+=rStepAngle ) {
            sprintf( buffer, rFileName.toStdString().c_str(), i, stateNr );
            setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_PHI,i-180);
            setView();
            repaint();
            double realWidth, realHeigth;
            screenshotSingle( buffer, rUseTiled, realWidth, realHeigth );
            imageCount++;
    }

    // Restore original light position:
    setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_PHI,     phiOri );
    setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_THETA, thetaOri );
    setView();
    repaint();

    //! String for Object2VR: 'gigamesh_still_image_'+ fill(column,5,'0') + '_' +fill(row,5,'0')+ '.png'
    // Information for the user
    int dirFileNameSplitPos = rFileName.lastIndexOf( QDir::separator() );
    QString fileNameWithoutPath = rFileName;
    QString pathFromFileName;
    if( dirFileNameSplitPos > 0 ) {
        fileNameWithoutPath = rFileName.mid( dirFileNameSplitPos+1 );
        pathFromFileName    = rFileName.left( dirFileNameSplitPos );
    }

    emit sStatusMessage( "Spherical image stack was saved to: " + rFileName );
}

//! Compute spherical images with moving light for a whole directory
void MeshWidget::sphericalImagesLightDir( ) {
    if( !saveStillImagesSettings() ) {
        return;
    }
    // Store settings from current Mesh and MeshWidget
    MeshGLParams storeMeshGLParams( (MeshGLParams)mMeshVisual );
    MeshWidgetParams storeMeshWidgetParams( (MeshWidgetParams)this );
    double printResDPI;
    getViewPortDPI( printResDPI );
    //Ask for tiled rendering
    bool userCancel;
    bool useTiled;
    SHOW_QUESTION( tr("Tiled rendering"), tr("Do you want to use tiled rendering?") + QString("<br /><br />") + tr("Typically: NO"), useTiled, userCancel );
    if( userCancel ) {
        return;
    }

    //Ask for the step range
    double stepLight;
    getParamFloatMeshWidget( LIGHT_STEPPING, &stepLight );

    // Ask user for the light stepping using the current setting
    QGMDialogEnterText dlgEnterTxt;
    int stepLightAngle = stepLight * 180;
    dlgEnterTxt.setWindowTitle( "Set light steps (angle between 1 and 180 degrees)" );
    dlgEnterTxt.setInt( stepLightAngle );
    if( dlgEnterTxt.exec() == QDialog::Rejected ) {
        return;
    }
    if( !dlgEnterTxt.getText( &stepLightAngle ) ) {
        stepLightAngle = stepLight * 180;
    }
    stepLight = stepLightAngle/180.0;

    bool useConstTheta;
    SHOW_QUESTION( tr("Constant Theta"), tr("Do you want to use a constant theta (see spherical coordinate system) ?") + QString("<br /><br />"), useConstTheta, userCancel );
    if( userCancel ) {
        return;
    }

    bool renderFront;
    SHOW_QUESTION( tr("Constant Theta"), tr("Do you want to render the front or the back (Rotated 180 degrees around the x-axis)?") +
                   QString("<br /><br />") + QString("Yes - render the frontside (current view)") +
                   QString("<br /><br />") + QString("No - render the backside ") , renderFront, userCancel );
    if( userCancel ) {
        return;
    }
    // Let the user choose a path
    QString     pathChoosen;
    QStringList currFiles;
    //we need the same functionality as in screenshotViewsDirectoryFiles
    if( !screenshotViewsDirectoryFiles( pathChoosen, currFiles ) ) {
        return;
    }

    //Enter filename
    QString targetfileName( "gigamesh_still_image_%03i.png");
    dlgEnterTxt.setWindowTitle( "Filename" );
    dlgEnterTxt.setText( targetfileName );
    if( dlgEnterTxt.exec() == QDialog::Rejected ) {
        return;
    }
    if( !dlgEnterTxt.getText( &targetfileName ) ) {
        return;
    }

    // for all files
    bool retVal = true;
    for( int i=0; i<currFiles.size(); ++i ) {
        QString currentFile = pathChoosen + '/' + currFiles.at(i);
        if( !fileOpen( currentFile ) ) {
            std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: File open failed for '"
                      << currentFile.toStdString() << "'!" << std::endl;
            retVal = false;
            continue;
        }
        if( mMeshVisual == nullptr ) {
            std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No mesh loaded for '"
                      << currentFile.toStdString() << "'!" << std::endl;
            retVal = false;
            continue;
        }

        this->setParamAllMeshWidget( storeMeshWidgetParams );
        mMeshVisual->setParamAllMeshWidget( storeMeshGLParams );
        orthoSetDPI( printResDPI );
        //rotate mesh when the user wants to render the backside
        if(!renderFront){
            std::vector<double> rotationAngle = {180 * M_PI / 180.0};
            Matrix4D rotationMatrix(Matrix4D::INIT_ROTATE_ABOUT_X,&rotationAngle);
            mMeshVisual->applyTransformationToWholeMesh(rotationMatrix);
            //the transformation with the identity matrix resets the mesh to the center
            Matrix4D identity(Matrix4D::INIT_IDENTITY);
            mMeshVisual->applyTransformationDefaultViewMatrix(&identity);
        }

        QString plyFileName = currFiles.at(i);
        plyFileName.remove(QRegularExpression(".ply"));
        QString targetDir = pathChoosen + '/' + plyFileName;
        //create target dir
        try {
            std::filesystem::create_directory( targetDir.toStdWString()); // https://en.cppreference.com/w/cpp/filesystem/create_directory
        } catch ( std::exception& except ) {
            wcerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: creating '" << targetDir.toStdWString() << endl;
            cerr << "[MeshWidget::" << __FUNCTION__ << "]        " << except.what() << endl;
        }

        QString fileTemplate = pathChoosen + '/' + plyFileName + '/' + targetfileName;
        if(useConstTheta){
            sphericalImagesLightConstTheta(fileTemplate,useTiled,stepLight);
        }else{
            sphericalImagesLight(fileTemplate,useTiled,stepLight);
        }

    } // for all files

}

//!Automatic Mesh Alignment
//!calls MeshQT->applyAutomaticMeshAlignment for all files in a given directory
//! and saves the transformed mesh to the directory
void MeshWidget::applyAutomaticMeshAlignmentDir(){
    // Store settings from current Mesh and MeshWidget
    MeshGLParams storeMeshGLParams( (MeshGLParams)mMeshVisual );
    MeshWidgetParams storeMeshWidgetParams( (MeshWidgetParams)this );
    // Let the user choose a path
    QString     pathChoosen;
    QStringList currFiles;
    //we need the same functionality as in screenshotViewsDirectoryFiles
    if( !screenshotViewsDirectoryFiles( pathChoosen, currFiles ) ) {
        return;
    }

    // Enter a suffix
    QString fileNameSuffix( "_AMA");
    QGMDialogEnterText dlgEnterTxt;
    dlgEnterTxt.setWindowTitle( "Filename Suffix" );
    dlgEnterTxt.setText( fileNameSuffix );
    if( dlgEnterTxt.exec() == QDialog::Rejected ) {
        return;
    }
    if( !dlgEnterTxt.getText( &fileNameSuffix ) ) {
        return;
    }


    // for all files
    bool retVal = true;
    for( int i=0; i<currFiles.size(); ++i ) {
        QString currentFile = pathChoosen + '/' + currFiles.at(i);
        if( !fileOpen( currentFile ) ) {
            std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: File open failed for '"
                      << currentFile.toStdString() << "'!" << std::endl;
            retVal = false;
            continue;
        }
        if( mMeshVisual == nullptr ) {
            std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No mesh loaded for '"
                      << currentFile.toStdString() << "'!" << std::endl;
            retVal = false;
            continue;
        }

        this->setParamAllMeshWidget( storeMeshWidgetParams );
        mMeshVisual->setParamAllMeshWidget( storeMeshGLParams );

        mMeshVisual->applyAutomaticMeshAlignment(false);

        //save file
        QString plyFileName = currFiles.at(i);
        plyFileName.remove(QRegularExpression(".ply"));
        QString newFile = pathChoosen + '/' + plyFileName;
        newFile = newFile + fileNameSuffix +  ".ply";

        //set Metadata of the mesh if not specified before
        //otherwise gigamesh will ask the metadata and the doesn't continue the process

        std::string modelID = mMeshVisual->getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
        if( modelID.empty() ) {
            QString suggestId( QString::fromStdWString(mMeshVisual->getBaseName().wstring()) );
            suggestId.replace( "_", " " );
            suggestId.replace( QRegularExpression( "GM[oOcCfFpPxX]*$" ), "" );
            mMeshVisual->getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_ID, suggestId.toStdString() );
        }
        std::string modelMaterial = mMeshVisual->getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
        if( modelMaterial.empty() ) {
            QString newMaterial = tr( "original, clay" );
            mMeshVisual->getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_MATERIAL, newMaterial.toStdString() );
        }

        mMeshVisual->writeFile(newFile);
    } // for all files
}

//! Compute spherical images (moving the object).
//! (1) Ask for the filename
void MeshWidget::sphericalImages() {
	if( mMeshVisual == nullptr ) {
		return;
	}
	string fileNamePattern;
	getParamStringMeshWidget( FILENAME_EXPORT_VR, &fileNamePattern );
    QString filePath = QString::fromStdWString( mMeshVisual->getFileLocation().wstring());
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Save as - Using a pattern for spherical images" ), \
	                                                 filePath + QString( fileNamePattern.c_str() ), \
													 tr( "Image (*.png *.tiff *.tif)" ),
	                                                 nullptr, 
	                                                 QFileDialog::DontUseNativeDialog  ); // Native dialog won't show patterns anymore on recent versions of Qt+Linux.
	if( fileName == nullptr ) {
		return;
	}
	sphericalImages( fileName );
}

//! Compute spherical images (moving the object).
//! (2) - ask about tiled rendering.
void MeshWidget::sphericalImages( const QString& rFileName ) {
	bool userCancel;
	bool useTiled;
	SHOW_QUESTION( tr("Tiled rendering"), tr("Do you want to use tiled rendering?)") + QString("<br /><br />") + tr("Typically: NO"), useTiled, userCancel );
	if( userCancel ) {
		return;
	}
	// Execute:
	sphericalImages(rFileName,  useTiled );
}

//! Compute spherical images (moving the object).
//! (3) - execute.
void MeshWidget::sphericalImages( const QString& rFileName, const bool rUseTiled ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( !saveStillImagesSettings() ) {
		return;
	}

	// Store the initial view setup.
	Vector3D cameraUpInit     = mCameraUp;
	Vector3D cameraCenterInit = mCameraCenter;

	// Store flag for cropping and turn cropping off as we require images of the same size!
	bool cropFlagTmp = mParamFlag[CROP_SCREENSHOTS];
	setParamFlagMeshWidget( CROP_SCREENSHOTS, false );

	string fileNamePattern = rFileName.toStdString();
	cout << "[MeshWidget::sphericalImages] Pattern string for Object2VR: '" << fileNamePattern << "'" << endl;
	int stateNr;
	getParamIntegerMeshWidget( STATE_NUMBER, &stateNr );

	char buffer[255];
	int  colIdx = 0;
	int  rowIdx = 0;
	unsigned int imageCount = 0;

	QString  axisUsed;
	Vector3D axisRotFirst;
	if( mParamFlag[SPHERICAL_VERTICAL] ) {
		axisUsed = "Vertical";
		axisRotFirst = Vector3D( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 ); // camPitch
	} else {
		axisUsed = "Horizontol";
		axisRotFirst = Vector3D( mMatModelView(1,0), mMatModelView(1,1), mMatModelView(1,2), 0.0 ); // camUp
	}



	for(int angleFirstAxis = -180; angleFirstAxis < 180;
	        angleFirstAxis += std::ceil(mParamFlt[SPHERICAL_STEPPING]) ) {
		mCameraUp     = cameraUpInit;
		mCameraCenter = cameraCenterInit;
		setView();
		repaint();
		rotArbitAxis( mCenterView, axisRotFirst, angleFirstAxis );
		Vector3D cameraUpSecond     = mCameraUp;
		Vector3D cameraCenterSecond = mCameraCenter;
		Vector3D axisRotSecond;
		if( mParamFlag[SPHERICAL_VERTICAL] ) {
			axisRotSecond = Vector3D( mMatModelView(1,0), mMatModelView(1,1), mMatModelView(1,2), 0.0 ); // camUp
		} else {
			axisRotSecond = Vector3D( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 ); // camPitch
		}
		colIdx = 0;
		for( size_t angleSecondAxis = 0; angleSecondAxis < 180;
		        angleSecondAxis += std::ceil(mParamFlt[SPHERICAL_STEPPING]) ) {
			mCameraUp     = cameraUpSecond;
			mCameraCenter = cameraCenterSecond;
			setView();
			repaint();
			rotArbitAxis( mCenterView, axisRotSecond, angleSecondAxis );
			sprintf( buffer, fileNamePattern.c_str(), colIdx, rowIdx, stateNr );
			double realWidth, realHeigth;
			screenshotSingle( buffer, rUseTiled, realWidth, realHeigth );
			imageCount++;
			colIdx++;
		}
		rowIdx++;
	}
	mCameraUp     = cameraUpInit;
	mCameraCenter = cameraCenterInit;
	setView();
	repaint();

	// Reset flag for cropping
	setParamFlagMeshWidget( CROP_SCREENSHOTS, cropFlagTmp );

	//! String for Object2VR: 'gigamesh_still_image_'+ fill(column,5,'0') + '_' +fill(row,5,'0')+ '.png'
	// Information for the user
	int dirFileNameSplitPos = rFileName.lastIndexOf( QDir::separator() );
	QString fileNameWithoutPath = rFileName;
	QString pathFromFileName;
	if( dirFileNameSplitPos > 0 ) {
		fileNameWithoutPath = rFileName.mid( dirFileNameSplitPos+1 );
		pathFromFileName    = rFileName.left( dirFileNameSplitPos );
	}
	QString obj2vrPattern = "'" + fileNameWithoutPath + "'";
	obj2vrPattern.replace( QString( "%05i_%05i_%02i" ), QString( "'+fill(column,5,'0')+'_'+fill(row,5,'0')+'_'+fill(state,2,'0')+'" ) );
	SHOW_MSGBOX_INFO( tr("Spherical image stack saved"),
					  QString( "<table>" ) +
					  QString( "<tr><td>") + tr("Images:")  + QString("</td><td align='right'> %1 / %2</td></tr>" ).arg( imageCount ).arg( colIdx*rowIdx ) +
					  QString( "<tr><td>") + tr("Axis:")    + QString("</td><td align='right'> %1</td></tr>" ).arg( axisUsed ) +
					  QString( "<tr><td>") + tr("Columns:") + QString("</td><td align='right'> %1</td></tr>" ).arg( colIdx ) +
					  QString( "<tr><td>") + tr("Rows:")    + QString("</td><td align='right'> %1</td></tr>" ).arg( rowIdx ) +
					  QString( "<tr><td>") + tr("State:")   + QString("</td><td align='right'> %1</td></tr>" ).arg( stateNr ) +
					  QString( "<tr><td>") + tr("Path:")    + QString("</td><td align='right'> %1</td></tr>" ).arg( pathFromFileName ) +
					  QString( "<tr><td>") + tr("Pattern:") + QString("</td><td align='right'> %1</td></tr>" ).arg( fileNameWithoutPath ) +
					  QString( "<tr><td><b>") + tr("Pattern&nbsp;(O2VR):") + QString("</b></td><td align='right'><b><nobr>%1</nobr></b></td></tr>" ).arg( obj2vrPattern ) +
					  QString( "</table>" )
	                );
	// Copy the pattern directly to the clipboard:
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText( obj2vrPattern );
	cout << "[MeshWidget::" << __FUNCTION__ << "] Pattern string for Object2VR: " << obj2vrPattern.toStdString() << endl;
	emit sStatusMessage( "Spherical image stack was saved to: " + rFileName );
}

//! Show dialog to enter the state number.
bool MeshWidget::sphericalImagesStateNr() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	int stateNr;
	getParamIntegerMeshWidget( STATE_NUMBER, &stateNr );
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Enter state number (integer)") );
	dlgEnterText.setID( STATE_NUMBER );
	dlgEnterText.setInt( stateNr );
	//QObject::connect( &dlgEnterText, SIGNAL(textEntered(int,int)), this, SLOT(setParamIntegerMeshWidget(int,int)) );
	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return false;
	}
	if( !dlgEnterText.getText( &stateNr ) ) {
		return false;
	}
	return setParamIntegerMeshWidget( STATE_NUMBER, stateNr );
}

//! Remove Mesh from memory.
void MeshWidget::unloadMesh() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( mMeshVisual != nullptr ) {
		// remove mesh, when one is present:
		delete mMeshVisual;
		mMeshVisual = nullptr;
		qglClearColor( Qt::gray );
		setView();
		update();
	}
}

// Select menu -----------------------------------------------------------------------------------------

//! Initializes the OpenGL scene like background, etc.
void MeshWidget::initializeGL() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "] " << endl;
#endif
	cout << "[MeshWidget::" << __FUNCTION__ << "] -----------------------------------------------------------" << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion() << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] Context valid: " << context()->isValid() << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion() << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] OpenGl information: VENDOR:       " << reinterpret_cast<const char*>(glGetString( GL_VENDOR )) << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "]                     RENDERDER:    " << reinterpret_cast<const char*>(glGetString( GL_RENDERER )) << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "]                     VERSION:      " << reinterpret_cast<const char*>(glGetString( GL_VERSION )) << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "]                     GLSL VERSION: " << reinterpret_cast<const char*>(glGetString( GL_SHADING_LANGUAGE_VERSION )) << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] -----------------------------------------------------------" << endl;

	QGLFormat glFormat = QGLWidget::format();
	if( !glFormat.sampleBuffers() ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Could not enable sample buffers!" << endl;
	}

	//setFormat( QGLFormat( QGL::DoubleBuffer | QGL::DepthBuffer ) ); // was in fileOpen and caused a addtional calls to initializeGL - maybe not necessary at all.
	qglClearColor( Qt::white );
	PRINT_OPENGL_ERROR( "qglClearColor( Qt::white )" );

	//! \todo Source revision for OpenGL initaliation.
	glEnable( GL_DEPTH_TEST );
	PRINT_OPENGL_ERROR( "glEnable( GL_DEPTH_TEST )" );

	// Face culling:
	glEnable( GL_CULL_FACE );
	PRINT_OPENGL_ERROR( "glEnable( GL_CULL_FACE )" );
	glCullFace( GL_BACK ); // GL_FRONT or GL_BACK (default)
	PRINT_OPENGL_ERROR( "glCullFace( GL_BACK )" );
	glFrontFace( GL_CCW ); // GL_CW or GL_CCW (default)
	PRINT_OPENGL_ERROR( "glFrontFace( GL_CCW )" );

	//glHint( GL_POINT_SMOOTH_HINT, GL_NICEST ); // Invalid enum in windows. Anyway, this is deprecated.
	//PRINT_OPENGL_ERROR( "glHint( GL_POINT_SMOOTH_HINT, GL_NICEST )" );
	//glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	//glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	//glEnable( GL_POINT_SMOOTH );  // <- removed from coreprofile
	//glEnable( GL_LINE_SMOOTH );
	//glEnable( GL_POLYGON_SMOOTH );
	// http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=46
	glEnable( GL_MULTISAMPLE );
	PRINT_OPENGL_ERROR( "glEnable( GL_MULTISAMPLE )" );

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	PRINT_OPENGL_ERROR( "glPolygonMode( GL_FRONT_AND_BACK, GL_FILL )" );

	// switch for polygon-offset to avoid the Z-Conflict <- deprecated
	//glEnable( GL_POLYGON_OFFSET_FILL );  // for all filled polygons
	//PRINT_OPENGL_ERROR( "glEnable( GL_POLYGON_OFFSET_.... )" );
	//glEnable( GL_POLYGON_OFFSET_LINE );  // for all line objects
	//PRINT_OPENGL_ERROR( "glEnable( GL_POLYGON_OFFSET_.... )" );
	//glEnable( GL_POLYGON_OFFSET_POINT ); // for all line objects
	//PRINT_OPENGL_ERROR( "glEnable( GL_POLYGON_OFFSET_.... )" );

	glClearDepth( 1.0f );                                // Depth Buffer Setup
	PRINT_OPENGL_ERROR( "glClearDepth( 1.0f )" );
	glDepthFunc( GL_LEQUAL );                            // The Type Of Depth Test To Do
	PRINT_OPENGL_ERROR( "glDepthFunc( GL_LEQUAL )" );
	// glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST ); // Really Nice Perspective Calculations <- removed from coreprofile
#ifdef DEAD_CORE_PROFILE
	glMatrixMode( GL_PROJECTION );                       // Select The Projection Matrix
	glLoadIdentity();                                    // Reset The Projection Matrix
	glMatrixMode( GL_MODELVIEW );                        // Select The Modelview Matrix
	glLoadIdentity();                                    // Reset The Modelview Matrix
	PRINT_OPENGL_ERROR( "some matrix error" );
#endif
	// Set zoom, when GigaMesh was executed with a given file.
	setViewInitialZoom();
}

//! Prepare VAO shaders as this can not be done in MeshWidget::initializeGL.
//! To be working for Windows release and debug version,
//! the initialization of the VAOs seemingly requires its own method (reason unknown).
void MeshWidget::initializeVAO() {
	//======================================================================================================================================================
	// Prepare VAO for the background shader
	// glGenVertexArrays causes GigaMesh to crash, when it is placed in initializeGL in the windows release version.
	PglGenVertexArrays glGenVertexArrays = reinterpret_cast<PglGenVertexArrays>(context()->getProcAddress( "glGenVertexArrays" ));
	PRINT_OPENGL_ERROR( "getProcAddress( glGenVertexArrays )" );
	if( glGenVertexArrays == nullptr ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getProcAddress falied for glGenVertexArrays!" << endl;
	} else {
		glGenVertexArrays( 1, &mVAO );
		PRINT_OPENGL_ERROR( "glGenVertexArrays( 1, &mVAO )" );
		cout << "[MeshWidget::" << __FUNCTION__ << "] glGenVertexArrays --- mVAO = " << mVAO << endl;
	}
}

//! Prepare VBOs and shaders as this can not be done in MeshWidget::initializeGL.
void MeshWidget::initializeShaders() {
	//======================================================================================================================================================
	// Prepare VBOs for the background shader
	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(context()->getProcAddress( "glBindVertexArray" ));
	PRINT_OPENGL_ERROR( "getProcAddress( glBindVertexArray )" );
	if( glBindVertexArray == nullptr ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getProcAddress falied for glBindVertexArray!" << endl;
	} else {
		glBindVertexArray( mVAO );
		PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );
	}

	GLfloat backVertices[] = { -1.0, -1.0,
				   +1.0, -1.0,
				   +1.0, +1.0,
				   -1.0, +1.0
				 };

	// Vertex data: one quadtriangle
	mVertBufObjs[VBO_BACKGROUND_VERTICES].create();
	mVertBufObjs[VBO_BACKGROUND_VERTICES].setUsagePattern( QOpenGLBuffer::StaticDraw );
	if( !mVertBufObjs[VBO_BACKGROUND_VERTICES].bind() ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Could not bind vertex buffer VBO_BACKGROUND_VERTICES to the context!" << endl;
	}
	mVertBufObjs[VBO_BACKGROUND_VERTICES].allocate( backVertices, 4 * 2 * sizeof( GLfloat ) );
	mVertBufObjs[VBO_BACKGROUND_VERTICES].release();

	// CREATE new shader for rendering a quadriangle in the background to display a rectangular grid
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	initializeShader( QString( ":/GMShaders/meshwidget_background_grid" ),          &mShaderGridOrtho        );
	initializeShader( QString( ":/GMShaders/meshwidget_background_polar_lines" ),   &mShaderGridPolarLines   );
	initializeShader( QString( ":/GMShaders/meshwidget_background_polar_circles" ), &mShaderGridPolarCircles );
	initializeShader( QString( ":/GMShaders/meshwidget_background_highlight_center" ),  &mShaderGridHighLightCenter );
	initializeShader( QString( ":/GMShaders/meshwidget_image2d" ),                  &mShaderImage            );

	//======================================================================================================================================================
	// LOAD texture maps
	//------------------------------------------------------------------------------------------------------------------------------------------------------
	for(QOpenGLTexture*& textureMap : mTextureMaps) {
		textureMap = nullptr;
	}
	initializeTextureMap( TEXMAP_GIGAMESH_LOGO,   ":/GMGeneric/GigaMesh_Logo.png" );
	initializeTextureMap( TEXMAP_KEYBOARD_LAYOUT, ":/GMGeneric/keyboard_layout/keyboard_move_camera.png" );

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );
}

//! Initalize raster images as texture maps using a file name.
bool MeshWidget::initializeTextureMap( eTextureMaps rTextureMap, const QString& rFileName ) {
	QImage texImage;
	if( !texImage.load( rFileName ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "} ERROR: Could not load texture map '" << rFileName.toStdString() << "'!" << endl;
		return false;
	}

	return initializeTextureMap( rTextureMap, &texImage );
}

//! Initalize raster images as texture maps using a given QImage.
bool MeshWidget::initializeTextureMap( eTextureMaps rTextureMap, QImage *rImage ) {
//	QImage texMapGL = QGLWidget::convertToGLFormat( rImage );
//	// Make sure the image is prepared:
//	if( texMapGL.isNull() ) {
//		cerr << "[MeshWidget::" << __FUNCTION__ << "} ERROR: Could not convert texture map for OpenGL!" << endl;
//		return false;
//	}
	if( mTextureMaps[rTextureMap] != nullptr ) {
		mTextureMaps[rTextureMap]->destroy();
		delete mTextureMaps[rTextureMap];
	}
	mTextureMaps[rTextureMap] = new QOpenGLTexture( rImage->mirrored() );
	mTextureMaps[rTextureMap]->setMinificationFilter(  QOpenGLTexture::Nearest ); // NEAREST is IMPORTANT - hard lesson learned: MipMapInterpolation will result in colors mixed from neighbouring color ramps!
	mTextureMaps[rTextureMap]->setMagnificationFilter( QOpenGLTexture::Nearest ); // NEAREST is IMPORTANT - hard lesson learned: MipMapInterpolation will result in colors mixed from neighbouring color ramps!
	return true;
}

//! Shader for the background: link and set locations.
void MeshWidget::initializeShader( const QString& rFileName, QOpenGLShaderProgram** rShaderProgram ) {
	(*rShaderProgram) = new QOpenGLShaderProgram();
	(*rShaderProgram)->addShaderFromSourceFile( QOpenGLShader::Vertex,   rFileName + ".vert" );
	(*rShaderProgram)->addShaderFromSourceFile( QOpenGLShader::Fragment, rFileName + ".frag" );

	// LINK Shader
	//-------------------------
	if( !(*rShaderProgram)->link() ) {
		QString linkMsgs = (*rShaderProgram)->log();
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: linking shader program (Background): " << linkMsgs.toStdString() << endl;
		linkMsgs = linkMsgs.left( linkMsgs.indexOf( "***" ) );
		SHOW_MSGBOX_CRIT( tr("GLSL Error") , linkMsgs );
	} else {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Linking shader program (Background) successfull." << endl;
	}
}

// Screenshots -------------------------------------------------------------------------------------------------------------------------------------------------




void MeshWidget::bindFramebuffer(int framebufferID)
{
	using PglBindFramebuffer = void (*)(GLenum, GLuint);
	PglBindFramebuffer bindFramebuffer = reinterpret_cast<PglBindFramebuffer>(context()->getProcAddress("glBindFramebuffer"));
	bindFramebuffer(GL_FRAMEBUFFER, framebufferID);
}


// --- Screenshots Side-Views as PDF ---------------------------------------------------------------------------------------------------------------------------

//! User interaction: ask for directory to parse.
//! @returns false in case of an error. True otherwise.
bool MeshWidget::screenshotViewsDirectoryFiles(
                QString&       rPathChoosen,     //!< Current directory.
                QStringList&   rCurrFiles        //!< List of files.
) {
	QString pathChoosen = QFileDialog::getExistingDirectory( mMainWindow,
	                                                         tr( "Choose the Directory" ) );

	if( pathChoosen.isEmpty() ) { // User cancel.
		std::cout << "[MeshWidget::" << __FUNCTION__ << "] User canceled!" << std::endl;
		return( false );
	}
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] chosen path: " << pathChoosen.toStdString() << std::endl;

	// Filter files with certain patterns (types copied from load dialog except!!! txt / TXT)
	QStringList filter3DFiles;
	filter3DFiles << "*.obj" << "*.OBJ" << "*.ply" << "*.PLY";

	rPathChoosen   = pathChoosen;
	rCurrFiles     = QDir( pathChoosen ).entryList( filter3DFiles );

//	// UNTESTED: Recursion for sub-directories
//	QFileInfoList fileInfoList  = QDir( pathChoosen ).entryInfoList();
//	QStringList   directories;
//	for( const QFileInfo& fileInfo : fileInfoList ) {
//		if( fileInfo.isDir() && fileInfo.absolutePath().contains(rPath) && !fileInfo.isHidden() ) {
//			directories.append( fileInfo.absolutePath() + '/' + fileInfo.completeBaseName() );
//		}
//	}

    return( true );
}
//! Set all possible Latex placeholder definitions
//!
//! placeholders are used to check if a userdefined latextemplate uses all possible values/placeholders
//! descriptions are used to give the user information about the placeholder value
void MeshWidget::setLatexPlaceholderDefinition()
{

    //single page
    mPdfSinglePlaceholders.clear();
    mPdfSinglePlaceholders = {
        {"__PDF_AUTHOR__","username and hostname of the system"},
        {"__OBJECT_ID__","is the value of 'ModelID' at the Meta information of the PLY"},
        {"__WEB_REFERENCE__","is the value of 'ModelReferenceWeb' at the Meta information of the PLY"},
        {"__FIGURE_IMAGE_FILE__","is the Name of the created PNG at the target directory (without '.png')"},
        {"__BOUNDING_BOX_WIDTH__","bounding box width in cm"},
        {"__BOUNDING_BOX_HEIGHT__","bounding box height in cm"},
        {"__BOUNDING_BOX_THICK__","bounding box thick in cm"},
        {"__VERTEX_COUNT__","number of vertices"},
        {"__FACE_COUNT__","number of faces"},
        {"__OBJECT_MATERIAL__","is the value of 'ModelMaterial' at the Meta information of the PLY"},
        {"__AREA_TOTAL__","surface of the mesh in mm²"},
        {"__AREA_TOTAL__","surface of the mesh in mm²"},
        {"__AREA_RESOLUTION_METRIC__","resolution in 1/cm²"},
        {"__AREA_RESOLUTION_DPI__","resolution in DPI"},
        {"__VOLUME_TOTAL__","volume in cm³"}
    };

    //views
    mPdfViewsPlaceholders.clear();
    mPdfViewsPlaceholders = {
        {"__PDF_AUTHOR__","username and hostname of the system"},
        {"__OBJECT_ID__","is the value of 'ModelID' at the Meta information of the PLY"},
        {"__WEB_REFERENCE__","is the value of 'ModelReferenceWeb' at the Meta information of the PLY"},
        {"__FIGURE_PREFIX__","is the Präfix of all generated images in directory figs at the target directory.\n Use in combination with e.g. \figureprefix_01_ha_top to include a image.\n possible Views:_01_ha_top,_02_ha_left,_03_ha_front,_04_ha_right,_05_ha_bottom, _06_ha_back"},
        {"__BOUNDING_BOX_WIDTH__","bounding box width in cm"},
        {"__BOUNDING_BOX_HEIGHT__","bounding box height in cm"},
        {"__BOUNDING_BOX_THICK__","bounding box thick in cm"},
        {"__VERTEX_COUNT__","number of vertices"},
        {"__FACE_COUNT__","number of faces"},
        {"__OBJECT_MATERIAL__","is the value of 'ModelMaterial' at the Meta information of the PLY"},
        {"__AREA_TOTAL__","surface of the mesh in mm²"},
        {"__AREA_TOTAL__","surface of the mesh in mm²"},
        {"__AREA_RESOLUTION_METRIC__","resolution in 1/cm²"},
        {"__AREA_RESOLUTION_DPI__","resolution in DPI"},
        {"__VOLUME_TOTAL__","volume in cm³"}
    };

}

bool MeshWidget::checkUserdefinedLatexFile(QString *latexTemplate, std::vector<LatexPlaceholder> rPlaceHolders)
{
    bool notUsedPlaceholderFound = false;
    QString informationTextPlaceholders;
    for(LatexPlaceholder placeholder: rPlaceHolders){
        if(!latexTemplate->contains(QRegExp(placeholder.placeholder))){
            informationTextPlaceholders = informationTextPlaceholders + placeholder.placeholder;
            informationTextPlaceholders = informationTextPlaceholders + QString(" | ");
            informationTextPlaceholders = informationTextPlaceholders + placeholder.descr +  QString("\n");
            notUsedPlaceholderFound = true;
        }
    }
    if(notUsedPlaceholderFound){
        bool userCancel;
        bool userContinue;
        SHOW_QUESTION( tr("Some Placeholders are not used"), tr("The Latex template contains not all possible Placeholders.\n This additional placeholders are still possible: ") + QString("\n\n") + informationTextPlaceholders + QString("\n\n") + tr("Continue?"), userContinue, userCancel );
        if( userCancel || !userContinue) {
            return( false );
        }
    }
    return ( true );
}

//! Help function for latex PDF Export (cc-license)
//! Ask for all possible Parameters and Versions of CC-licenses
//! based on the Latex package doclicense
//! https://ctan.org/pkg/doclicense?lang=de
bool MeshWidget::askForCCLicenseParameters(QString *ccParameter, QString *ccVersion)
{
     QStringList possibleParameters;
     possibleParameters.append("by");
     possibleParameters.append("by-sa");
     possibleParameters.append("by-nd");
     possibleParameters.append("by-nc");
     possibleParameters.append("by-nc-sa");
     possibleParameters.append("by-nc-nd");

     bool userCancel;
     SHOW_DIALOG_COMBO_BOX( tr("CC-Attribution"), tr("Which type of CC-License do you want to use?"), possibleParameters, *ccParameter, userCancel );
     if( userCancel ) {
         return( false );
     }

     QStringList possibleVersions;
     possibleVersions.append("3.0");
     possibleVersions.append("4.0");
     SHOW_DIALOG_COMBO_BOX( tr("CC-Version"), tr("Which version of CC-License do you want to use?"), possibleVersions, *ccVersion, userCancel );
     if( userCancel ) {
         return( false );
     }
     return( true );

}

//! Render front-views or side-views as
//! PNGs or PDFs having PNGs embeded created using a LaTeX template.
//!
//! User interaction: select a path to load the meshes consecutively
//! and generate views for each 3D-file found.
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool MeshWidget::screenshotViewsDirectory() {
	// Store settings from current Mesh and MeshWidget
	MeshGLParams storeMeshGLParams( (MeshGLParams)mMeshVisual );
	MeshWidgetParams storeMeshWidgetParams( (MeshWidgetParams)this );

	// PDF or PNG
	bool preferPNGoverPDF(true);
	// Ask user about PDF creation or PNG rendering only
	bool userCancel;
	SHOW_QUESTION( tr( "PNG images or PDF documents" ),
	               tr( "Do you want to create<br /><br />"
	                   "... PNG images - choose: YES<br /><br />"
	                   "... PDF documents - choose: NO" ),
	               preferPNGoverPDF, userCancel );
	if( userCancel ) {
		return( false );
	}

	// Print resolution and tiled renderin (only in Orthographic projection mode)
	bool   frontView(true);
	bool   useTiled(false);
	double printResDPI;
	getViewPortDPI( printResDPI );
	bool orthoMode;
	if( !getParamFlagMeshWidget( ORTHO_MODE, &orthoMode ) ) {
		return( false );
	}
	if( orthoMode ) {
		// Ask user for the print resolution using the current setting
		QGMDialogEnterText dlgEnterTxt;
		dlgEnterTxt.setWindowTitle( "Set DPI" );
		dlgEnterTxt.setDouble( printResDPI );
		if( dlgEnterTxt.exec() == QDialog::Rejected ) {
			return( false );
		}
		if( !dlgEnterTxt.getText( &printResDPI ) ) {
			return( false );
		}
		// Ask user about single VS side views
		SHOW_QUESTION( tr( "Front view | Side views" ),
		               tr( "Do you want render<br /><br />"
		                   "... only the front view - choose: YES<br /><br />"
		                   "... views from all sides - choose: NO" ),
		               frontView, userCancel );
		if( userCancel ) {
			return( false );
		}
		if( frontView | preferPNGoverPDF ) { // Tiled rendering is not an option for PDF and Views
			// Ask user about tiled rendering
			SHOW_QUESTION( tr( "Tiled rendering" ),
				       tr( "Do you want to use tiled rendering?" ),
				       useTiled, userCancel );
			if( userCancel ) {
				return( false );
			}
		}
	}

    //ask for pdf templates
    QString rTemplate;
    QString texFileName;
    //set standard license Values
    QString ccParameters = "by-sa";
    QString ccVersion = "4.0";
    if(!preferPNGoverPDF){
        QStringList templates;
        templates.append( "Single page" );
        templates.append( "Single page with cc-license" );
        templates.append( "Own template" );

        SHOW_DIALOG_COMBO_BOX( tr("Template"), tr("Which template do you want to use?"), templates, rTemplate, userCancel );
        if( userCancel ) {
            return( false );
        }
        //load user individual template
        if( rTemplate == "Own template"){
            texFileName = QFileDialog::getOpenFileName( this,
                                                              tr( "Import Latex template" ),
                                                              nullptr,
                                                              tr( "tex-file (*.tex)" )
                                                             );
            if( texFileName == nullptr) {
                return( false );
            }
        }
        //ask for cc license parameters and Version
        if( rTemplate == "Single page with cc-license"){
            if(!askForCCLicenseParameters(&ccParameters,&ccVersion)){
                return( false );
            }
        }
    }

	// Let the user choose a path
	QString     pathChoosen;
	QStringList currFiles;
	if( !screenshotViewsDirectoryFiles( pathChoosen, currFiles ) ) {
		return( false );
	}

	// Enter a suffix
	QString fileNameSuffix( "_Uniform");
	QGMDialogEnterText dlgEnterTxt;
	dlgEnterTxt.setWindowTitle( "Filename Suffix" );
	dlgEnterTxt.setText( fileNameSuffix );
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return( false );
	}
	if( !dlgEnterTxt.getText( &fileNameSuffix ) ) {
		return( false );
	}

	bool retVal = true;
	for( int i=0; i<currFiles.size(); ++i ) {
		QString currentFile = pathChoosen + '/' + currFiles.at(i);
		if( !fileOpen( currentFile ) ) {
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: File open failed for '"
			          << currentFile.toStdString() << "'!" << std::endl;
			retVal = false;
			continue;
		}
		if( mMeshVisual == nullptr ) {
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No mesh loaded for '"
			          << currentFile.toStdString() << "'!" << std::endl;
			retVal = false;
			continue;
		}

		this->setParamAllMeshWidget( storeMeshWidgetParams );
		mMeshVisual->setParamAllMeshWidget( storeMeshGLParams );
		if( orthoMode ) {
			orthoSetDPI( printResDPI );
		}

		// Create PNGs
		string fileNamePattern;
		getParamStringMeshWidget( FILENAME_EXPORT_VIEWS, &fileNamePattern );
		QString prefixStem( string( std::filesystem::path( currFiles.at(i).toStdString() ).stem().string() ).c_str() );

		if( frontView ) {
			if( preferPNGoverPDF ) {
				// FRONT View PNG image
				double widthReal;  // Dummy var - unused in this context.
				double heigthReal; // Dummy var - unused in this context.
				retVal |= screenshotSingle( pathChoosen+"/"+prefixStem+fileNameSuffix+".png",
				                             useTiled, widthReal, heigthReal );
			} else {
				// FRONT View PDF document
				QString prefixStem( string( std::filesystem::path( currFiles.at(i).toStdString() ).stem().string() ).c_str() );
				retVal |= screenshotPDF( pathChoosen+'/'+prefixStem+fileNameSuffix+".pdf",
                                         useTiled,
                                         rTemplate,
                                         texFileName,
                                         ccParameters,
                                         ccVersion); // Fromt Side
			}
		} else {
			if( preferPNGoverPDF ) {
				// SIDE Views PNG images
				std::vector<QString> imageFiles;
				std::vector<double>  imageSizes;
				//! \todo add boolean return to screenshotViews
				screenshotViews( QString( fileNamePattern.c_str() ),
				                 QString( pathChoosen+"/"+prefixStem+fileNameSuffix ),
				                 useTiled, imageFiles, imageSizes );
			} else {
				// SIDE Views PDF document
				QString prefixStem( string( std::filesystem::path( currFiles.at(i).toStdString() ).stem().string() ).c_str() );
                retVal |= screenshotViewsPDF( pathChoosen+'/'+prefixStem+fileNameSuffix+".pdf",
                                              rTemplate,
                                              texFileName,
                                              ccParameters,
                                              ccVersion); // TILES always ON
			}
		}
	} // for all files

	if( retVal ) {
		SHOW_MSGBOX_INFO( tr( "Directory Schreenshots" ),
		                  tr( "Screenshots have been exported for:<br /><br />" ) +
		                  pathChoosen );
	} else {
		SHOW_MSGBOX_WARN( tr( "ERROR - Directory Schreenshots" ),
		                  tr( "Errors occured creating screenshots for:<br /><br />" ) +
		                  pathChoosen );
	}
    //set false, that the userindividual latex template will check next time
    mUserContinue = false;
	return( retVal );
}

//! Render front-views or side-views as
//! PNGs or PDFs having PNGs embeded created using a LaTeX template.
//!
//! User interaction: select a path to load the meshes consecutively
//! and generate views for each 3D-file found.
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool MeshWidget::directoryFuncValToRGB() {
	// Store settings from current Mesh and MeshWidget
	MeshGLParams storeMeshGLParams( (MeshGLParams)mMeshVisual );
	MeshWidgetParams storeMeshWidgetParams( (MeshWidgetParams)this );

	// Let the user choose a path
	QString     pathChoosen;
	QStringList currFiles;
	if( !screenshotViewsDirectoryFiles( pathChoosen, currFiles ) ) {
		return( false );
	}

	// Enter a suffix
	QString fileNameSuffix( "_FuncValColor");
	QGMDialogEnterText dlgEnterTxt;
	dlgEnterTxt.setWindowTitle( "Filename Suffix" );
	dlgEnterTxt.setText( fileNameSuffix );
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		return( false );
	}
	if( !dlgEnterTxt.getText( &fileNameSuffix ) ) {
		return( false );
	}

	bool retVal = true;
	for( int i=0; i<currFiles.size(); ++i ) {
		QString currentFile = pathChoosen + '/' + currFiles.at(i);
		if( !fileOpen( currentFile ) ) {
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: File open failed for '"
			          << currentFile.toStdString() << "'!" << std::endl;
			retVal = false;
			continue;
		}
		if( mMeshVisual == nullptr ) {
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No mesh loaded for '"
			          << currentFile.toStdString() << "'!" << std::endl;
			retVal = false;
			continue;
		}

		//double minFuncValue = -0.12910306806; // Values used for HeiCuBeDa 1%
		//double maxFuncValue = +0.48764927169; // Values used for HeiCuBeDa 99%
		//double minFuncValue = -0.061743971965; // Values used for ErKon3D Springer LNCS 5%
		//double maxFuncValue = 0.205966176870; // Values used for ErKon3D Springer LNCS 98%
		//double maxFuncValue = +0.554183392987; // Values used for ErKon3D Springer LNCS

		// Setup Mesh
		this->setParamAllMeshWidget( storeMeshWidgetParams );
		mMeshVisual->setParamAllMeshWidget( storeMeshGLParams );
		mMeshVisual->runFunctionValueToRGBTransformation();

		// Write with suffix inserted
		int lastDot = currentFile.lastIndexOf( '.' );
		currentFile.insert( lastDot, fileNameSuffix );
		mMeshVisual->writeFile( currentFile );
	}

	if( retVal ) {
		SHOW_MSGBOX_INFO( tr( "3D files in directory" ),
		                  tr( "All files have been recolored:<br /><br />" ) +
		                  pathChoosen );
	} else {
		SHOW_MSGBOX_WARN( tr( "3D files in directory" ),
		                  tr( "Errors occured during recoloring for:<br /><br />" ) +
		                  pathChoosen );
	}

	return( retVal );
}

//! Render side-views as PNG and embed those within a PDF using a LaTeX template.
//!
//! User interaction: select PDF for currently presented mesh.
//! In case evince is installed the PDF will be shown.
//!
//! @returns false in case of an error. True otherwise.
bool MeshWidget::screenshotViewsPDFUser() {
	// Sanity check
	if( mMeshVisual == nullptr ) {
		return( false );
	}
    const QString filePrefix = QString::fromStdWString(mMeshVisual->getBaseName().wstring());
    const QString filePath = QString::fromStdWString( mMeshVisual->getFileLocation().wstring());
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] filePath:        " << filePath.toStdString() << std::endl;
	//qDebug() << filePath + QString( fileNamePattern.c_str() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Save as - Using a pattern for side, top and bottom views" ), \
	                                                 filePath + "/" + filePrefix + ".pdf",
													 tr( "Document (*.pdf)" ),
	                                                 nullptr,
	                                                 QFileDialog::DontUseNativeDialog  ); // Native dialog won't show patterns anymore on recent versions of Qt+Linux.
	// User cancel
	if( fileName == nullptr ) {
		return( false );
	}

    QStringList templates;
    templates.append( "Single page" );
    templates.append( "Single page with cc-license" );
    templates.append( "Own template" );

    bool userCancel;
    QString rTemplate;
    SHOW_DIALOG_COMBO_BOX( tr("Template"), tr("Which template do you want to use?"), templates, rTemplate, userCancel );
    if( userCancel ) {
        return( false );
    }

    //load user individual template
    QString texFileName;
    if( rTemplate == "Own template"){

        texFileName = QFileDialog::getOpenFileName( this,
                                                          tr( "Import Latex template" ),
                                                          nullptr,
                                                          tr( "tex-file (*.tex)" )
                                                         );
        if( texFileName == nullptr) {
            return( false );
        }
    }
    //ask for cc license parameters and Version
    QString ccParameters = "by-sa";
    QString ccVersion = "4.0";
    if( rTemplate == "Single page with cc-license"){
        if(!askForCCLicenseParameters(&ccParameters,&ccVersion)){
            return( false );
        }
    }

	// Execute
    if( !screenshotViewsPDF( fileName, rTemplate, texFileName, ccParameters, ccVersion ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: screenshotViewsPDF( " << fileName.toStdString() << " ) failed!" << std::endl;
		return( false );
	}

    // --- Show PDF, when evince, okular or atril is available --------------------------------------------------------------------------------------------------------------
	std::string pdfViewerCommand;
	getParamStringMeshWidget(MeshWidgetParams::PDF_VIEWER_COMMAND, &pdfViewerCommand);
#ifdef WIN32
	fileName.replace( QString("/") , QString("\\"));
#endif
	if( !QProcess::startDetached( QString(pdfViewerCommand.c_str()) + " \"" + fileName + "\"" ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Program " + pdfViewerCommand + " not found!" << std::endl;
        //Try okular
        getParamStringMeshWidget(MeshWidgetParams::PDF_VIEWER_COMMAND_ALT1, &pdfViewerCommand);

        if( !QProcess::startDetached( QString(pdfViewerCommand.c_str()) + " \"" + fileName + "\"" ) ) {
            std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Program " + pdfViewerCommand + " not found!" << std::endl;

            //Try atril
            getParamStringMeshWidget(MeshWidgetParams::PDF_VIEWER_COMMAND_ALT2, &pdfViewerCommand);

            if( !QProcess::startDetached( QString(pdfViewerCommand.c_str()) + " \"" + fileName + "\"" ) ) {
                std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Program " + pdfViewerCommand + " not found!" << std::endl;
            }
        }

	}
	// -----------------------------------------------------------------------------------------------------------------------------------------------------
    //set false, that the userindividual latex template will check next time
    mUserContinue = false;
	return( true );
}

//! Render side-views as PNG and embed those within a PDF using a LaTeX template.
//!
//! Accepts a filename to write a PDF with views for the current mesh (MeshWidget::mMeshVisual).
//!
//! @todo GUI Questions for the cc license parameters
//! @returns false in case of an error. True otherwise.
bool MeshWidget::screenshotViewsPDF( const QString& rFileName, QString rTemplate, const QString rTexFileName, const QString rCCparameter, const QString rCCversion ) {
	//! Always use tiled rendering.
	bool useTiled = true;
	//! Six views only (for now)
	setParamFlagMeshWidget( EXPORT_SIDE_VIEWS_SIX, true );

	// Prepare filenames.
	string fileNamePatternTmp;
	getParamStringMeshWidget( FILENAME_EXPORT_VIEWS, &fileNamePatternTmp );
	const QString fileNamePattern = QString::fromStdString(fileNamePatternTmp);
	cout << "[MeshWidget::" << __FUNCTION__ << "] fileNamePattern: " << fileNamePatternTmp << endl;
	auto prefixPath = QString::fromStdWString(std::filesystem::path( rFileName.toStdString() ).parent_path().wstring());
	const auto prefixStem = QString::fromStdWString(std::filesystem::path( rFileName.toStdString() ).stem().wstring());
	QString filePrefixTex    = prefixPath + "/"        + prefixStem;
	QString filePrefixImg    = prefixPath + "/figs/"   + prefixStem + "_PDFpage";
	QString filePrefixImgTex = "figs/"    + prefixStem + "_PDFpage"; // Relative path to images!

	// Create "figs" subfolder, when necessary
	try {
		std::filesystem::create_directory( prefixPath.toStdWString()+L"/figs" ); // https://en.cppreference.com/w/cpp/filesystem/create_directory
	} catch ( std::exception& except ) {
		wcerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: creating '" << prefixPath.toStdWString() << "/figs'!" << endl;
		cerr << "[MeshWidget::" << __FUNCTION__ << "]        " << except.what() << endl;
		return( false );
	}

	// Render screenshots in sub-directoy "figs" and add extra tag to the prefix.
	std::vector<QString> imageFileNames;
	std::vector<double> imageSizes;
	screenshotViews( fileNamePattern, filePrefixImg,
	                 useTiled, imageFileNames, imageSizes );
	for( uint64_t i=0; i<imageFileNames.size(); i++ ) {
		wcout << "[MeshWidget::" << __FUNCTION__ << "] File written: " << imageFileNames.at( i ).toStdWString() <<
		        " Size " << imageSizes.at( i*2 ) << " x " << imageSizes.at( i*2 + 1 ) << " mm (unit assumed)." << endl;
	}
	// Compute image dimensions W: 2,3,4 H: 1,3,5,6 - but out of order => 7,1,6 and 0,1,2,3
	double allImageWidth   = imageSizes.at( 5*2 +0 ) + imageSizes.at( 1*2 +0 ) + imageSizes.at( 4*2 +0 );
	double allImageHeigth  = imageSizes.at( 0*2 +1 ) + imageSizes.at( 1*2 +1 ) + imageSizes.at( 2*2 +1 )
	                       + imageSizes.at( 3*2 +1 );
	cout << "[MeshWidget::" << __FUNCTION__ << "] allImageWidth: "  << allImageWidth << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] allImageHeigth: " << allImageHeigth << endl;
	double scaleFactor    = 1.0;
	QString scaleFactorTex( "1:1" );

	// DIN A4: 210 x 297 | Template borders: 10 mm
	double borderWidth = 10.0 * 2.0;
	double ratioWidth  = allImageWidth  / ( 210.0 - borderWidth - 20.0 );
	double ratioHeigth = allImageHeigth / ( 297.0 - borderWidth - 45.0 ); // for table heigth
	cout << "[MeshWidget::" << __FUNCTION__ << "] ratioWidth:  " << ratioWidth  << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] ratioHeigth: " << ratioHeigth << endl;
	double maxRatio = max( ratioWidth, ratioHeigth );
	cout << "[MeshWidget::" << __FUNCTION__ << "] maxRatio: " << maxRatio << endl;
	double log10pow = floor( log10( maxRatio ) );
	cout << "[MeshWidget::" << __FUNCTION__ << "] log10pow: " << log10pow << endl;
	scaleFactor       = 2.0/( ceil( maxRatio*2.0/pow( 10.0, log10pow ) )*pow( 10.0, log10pow ) );
	scaleFactorTex    = "1:" + QString( "%1" ).arg( 1.0/scaleFactor, 'f' ).trimmed() ;

	// Load template
    //standard = single page
	QString latexTemplateName( ":/GMLaTeX/report_single_page_template.tex" );
    //other templates
    if(rTemplate == "Single page with cc-license"){
        latexTemplateName = ":/GMLaTeX/report_single_page_cclicense_template.tex";
    }
    if( rTemplate == "Own template"){
        latexTemplateName = rTexFileName;
    }
    QString latexTemplate;
	QFile fileLatexIn( latexTemplateName );
	if( !fileLatexIn.open( QIODevice::ReadOnly ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Template could not be opened!" << endl;
		return( false );
	}

	latexTemplate = fileLatexIn.readAll();
	fileLatexIn.close();

    //check user defined Latex file
    if( rTemplate == "Own template" && !mUserContinue){
        if(!checkUserdefinedLatexFile(&latexTemplate,mPdfSinglePlaceholders)){
            cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: User stopped process" << endl;
            return( false );
        }
        else{
            mUserContinue = true;
        }
    }

	// Fetch strings with information for the table
	vector<pair<string,string>> replacmentStrings;
	mMeshVisual->latexFetchFigureInfos( &replacmentStrings );

	// Replace placeholders
	if (mParamFlag[SPHERICAL_VERTICAL])
	{
		replacmentStrings.emplace_back(pair<string, string>("01_ha_top"   , "01_va_top"));
		replacmentStrings.emplace_back(pair<string, string>("02_ha_left"  , "03_va_side"));
		replacmentStrings.emplace_back(pair<string, string>("03_ha_front" , "02_va_front"));
		replacmentStrings.emplace_back(pair<string, string>("04_ha_right" , "05_va_side"));
		replacmentStrings.emplace_back(pair<string, string>("05_ha_bottom", "06_va_bottom"));
		replacmentStrings.emplace_back(pair<string, string>("06_ha_back"  , "04_va_back"));
	}

    //replace license placeholders
    latexTemplate.replace( QRegExp( "__CC_PARAMETERS__" ), rCCparameter );
    latexTemplate.replace( QRegExp( "__CC_VERSION__" ), rCCversion );

	latexTemplate.replace( QRegExp( "__FIGURE_PREFIX__" ), filePrefixImgTex );
	latexTemplate.replace( QRegExp( "__SCALE_FACTOR_STRING__" ), scaleFactorTex );
	latexTemplate.replace( QRegExp( "__SCALE_FACTOR__" ), QString( "%1" ).arg( scaleFactor, 'f' ).trimmed() );
	for( pair<string, string>& replacmentString : replacmentStrings ) {
		string placeHolder = replacmentString.first;
		string content = replacmentString.second;
		latexTemplate.replace( QString( placeHolder.c_str() ), QString( content.c_str() ) );
	}
	//cout << latexTemplate.toStdString() << endl;

	// Write LaTex file
	QFile fileLatexOut( filePrefixTex + ".tex" );
	if( !fileLatexOut.open( QIODevice::WriteOnly ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: LaTeX file could not be opened!" << endl;
		return( false );
	} else {
		fileLatexOut.write( latexTemplate.toStdString().data() );
	}
	fileLatexOut.close();

	QString qPrefixPath(prefixPath);
#ifdef WIN32
	qPrefixPath.replace(QString("/"), QString("\\"));
	filePrefixTex.replace(QString("/"), QString("\\"));
#endif

	// Compile and show LaTeX file.
	if( !screenshotPDFMake( qPrefixPath, filePrefixTex ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: LaTeX file could not be compiled!" << endl;
		return( false );
	}

	return( true );
}

//! Helper function for screenshots embedded into PDF.
//!
//! @returns false in case of an error. True otherwise.
bool MeshWidget::screenshotPDFMake(
                const QString& rPrefixPath,
                const QString& rFilePrefixTex
) {
	// --- Convert to pdf, when pdflatex is available ------------------------------------------------------------------------------------------------------
	bool checkPDFLaTeXFailed = false;

	std::string pdfLatexCommand;
	getParamStringMeshWidget(MeshWidgetParams::PDF_LATEX_COMMAND, &pdfLatexCommand);

	std::cout << rPrefixPath.toStdString() << " " << rFilePrefixTex.toStdString() << std::endl;
	QProcess runPDFLaTeX;
	runPDFLaTeX.setWorkingDirectory( rPrefixPath );
	runPDFLaTeX.start( QString(pdfLatexCommand.c_str()) + " -interaction=nonstopmode \"" + rFilePrefixTex + ".tex\"" );
	if( !runPDFLaTeX.waitForFinished() ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR testing PDFLaTeX had a timeout!" << endl;
		checkPDFLaTeXFailed = true;
	}
	if( runPDFLaTeX.exitStatus() != QProcess::NormalExit ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR testing PDFLaTeX had no normal exit!" << endl;
		checkPDFLaTeXFailed = true;
	}
	if( runPDFLaTeX.exitCode() != 0 ) {
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR PDFLaTeX exit code: " << runPDFLaTeX.exitCode() << endl;
		QString outPDFLaTeXErr( runPDFLaTeX.readAllStandardError() );
		cerr << "[QGMMainWindow::" << __FUNCTION__ << "] PDFLaTeX error: " << outPDFLaTeXErr.toStdString().c_str() << endl;
		checkPDFLaTeXFailed = true;
	}
	QString outPDFLaTeX( runPDFLaTeX.readAllStandardOutput() );
	cout << "[QGMMainWindow::" << __FUNCTION__ << "] PDFLaTeX check: " << outPDFLaTeX.simplified().toStdString().c_str() << endl;
	if( checkPDFLaTeXFailed ) {
		SHOW_MSGBOX_WARN_TIMEOUT( tr("PDFLaTeX error"), tr("pdflatex failed to compile the PDF!"), 5000 );
		return( false );
	}
	// -----------------------------------------------------------------------------------------------------------------------------------------------------
	return( true );
}

// --- Screenshots Side-Views as PNG ---------------------------------------------------------------------------------------------------------------------------

//! Best used in orthographic mode as it genereates screenshots of the
//! side- and top-views.
void MeshWidget::screenshotViews() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//! 1.) Ask for the filename
	if( mMeshVisual == nullptr ) {
		return;
	}
	string fileNamePattern;
	getParamStringMeshWidget( FILENAME_EXPORT_VIEWS, &fileNamePattern );
    QString filePath = QString::fromStdWString( mMeshVisual->getFileLocation().wstring());
	cout << "[MeshWidget::" << __FUNCTION__ << "] fileNamePattern: " << fileNamePattern << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] filePath:        " << filePath.toStdString() << endl;
	//qDebug() << filePath + QString( fileNamePattern.c_str() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Save as - Using a pattern for side, top and bottom views" ), \
													 filePath + "/" + QString( fileNamePattern.c_str() ),
                                                     tr( "Image (*.png)" ),
	                                                 nullptr, 
	                                                 QFileDialog::DontUseNativeDialog  ); // Native dialog won't show patterns anymore on recent versions of Qt+Linux.

	if( fileName.size() == 0 ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] No filename given." << endl;
		return;
	}
	screenshotViews( fileName );
}

//! Best used in orthographic mode as it genereates screenshots of the
//! side- and top-views.
void MeshWidget::screenshotViews( const QString& rFileName ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//! 2.) Ask for tiled rendering
	bool useTiled = false;
	{
		bool userCancel;
		SHOW_QUESTION( tr("Tiled rendering"),
					   tr("Do you want to use tiled rendering?"),
		               useTiled, userCancel );
		if( userCancel ) {
			return;
		}
	}

	//! 3.) Ask for histogram(s), when activated
	bool histogramShown = false;
	bool histogramScenceShown = false;
	getParamFlagMeshWidget( SHOW_HISTOGRAM, &histogramShown );
	getParamFlagMeshWidget( SHOW_HISTOGRAM_SCENE, &histogramScenceShown );
	if( histogramShown || histogramScenceShown ) {
		bool userCancel;
		bool discardHistograms = true;
		SHOW_QUESTION( tr("Histogram(s) shown"),
					   tr("Do you want to discard the histogram(s) for the screenshot?<br /><br />Recommended: YES"),
		               discardHistograms, userCancel );
		if( userCancel ) {
			return;
		}
		if( discardHistograms ) {
			setParamFlagMeshWidget( SHOW_HISTOGRAM, false );
			setParamFlagMeshWidget( SHOW_HISTOGRAM_SCENE, false );
		}
	}

	//! 4.) Execute
	std::vector<QString> imageFileNames;
	std::vector<double> imageSizes;
    auto filePrefix = QString::fromStdWString(mMeshVisual->getBaseName().wstring());
	screenshotViews( rFileName.toLatin1().data(), filePrefix, useTiled, imageFileNames, imageSizes );
	for( uint64_t i=0; i<imageFileNames.size(); i++ ) {
		wcout << "[MeshWidget::" << __FUNCTION__ << "] File written: " << imageFileNames.at( i ).toStdWString() <<
		        " Size " << imageSizes.at( i*2 ) << " x " << imageSizes.at( i*2 + 1 ) << " mm (unit assumed)." << endl;
	}
}

//! Writes 6 screenshots/views - either by horizontal or vertial rotation.
void MeshWidget::screenshotViews( const QString&               rFilePattern,   //!< File pattern.
                                  const QString&               rFilePrefix,    //!< File name (prefix).
                                  const bool                  rUseTiled,      //!< Toggle tiled rendering.
                                  std::vector<QString>&       rImageFiles,    //!< Name of the written image files.
                                  std::vector<double>&        rImageSizes     //!< Size of the image in world coordinates (mm, assumed).
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Number of views
	bool sixViews = true;
	getParamFlagMeshWidget( EXPORT_SIDE_VIEWS_SIX, &sixViews );
	// Orientation of the axis
	bool rotationAboutVertical;
	getParamFlagMeshWidget( SPHERICAL_VERTICAL, &rotationAboutVertical );

	//char buffer[512]; // For preparation of the rFileName using a pattern
	QString buffer;
	double realWidth  = _NOT_A_NUMBER_DBL_; // Width in world coordinates e.g. millimeter.
	double realHeigth = _NOT_A_NUMBER_DBL_; // Heigth in world coordinates.

	// Rotate about the vertical axis - ceramic style
	//===================================================
	if( rotationAboutVertical ) {
		// View 2: Front
		//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 2, "va_front" );
		buffer = rFilePattern.arg(rFilePrefix).arg(2,2,10,QChar('0')).arg("va_front");
		if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
			rImageFiles.push_back( buffer );
			rImageSizes.push_back( realWidth );
			rImageSizes.push_back( realHeigth );
		}

		// View 3: Side
		rotYaw( -90.0 );
		//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 3, "va_side" );
		buffer = rFilePattern.arg(rFilePrefix).arg(3,2,10,QChar('0')).arg("va_side");
		if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
			rImageFiles.push_back( buffer );
			rImageSizes.push_back( realWidth );
			rImageSizes.push_back( realHeigth );
		}

		// View 4: Back
		rotYaw( -90.0 );
		//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 4, "va_back" );
		buffer = rFilePattern.arg(rFilePrefix).arg(4,2,10,QChar('0')).arg("va_back");
		if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
			rImageFiles.push_back( buffer );
			rImageSizes.push_back( realWidth );
			rImageSizes.push_back( realHeigth );
		}

		// View 5: Side
		rotYaw( -90.0 );
		//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 5, "va_side" );
		buffer = rFilePattern.arg(rFilePrefix).arg(5,2,10,QChar('0')).arg("va_side");
		if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
			rImageFiles.push_back( buffer );
			rImageSizes.push_back( realWidth );
			rImageSizes.push_back( realHeigth );
		}

		// Move back to front
		rotYaw( -90.0 );

		// View 1: Top
		rotPitch( -90.0 );
		//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 1, "va_top" );
		buffer = rFilePattern.arg(rFilePrefix).arg(1,2,10,QChar('0')).arg("va_top");
		if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
			rImageFiles.push_back( buffer );
			rImageSizes.push_back( realWidth );
			rImageSizes.push_back( realHeigth );
		}

		// View 6: Bottom
		rotPitch( +180.0 );
		//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 6, "va_bottom" );
		buffer = rFilePattern.arg(rFilePrefix).arg(6,2,10,QChar('0')).arg("va_bottom");
		if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
			rImageFiles.push_back( buffer );
			rImageSizes.push_back( realWidth );
			rImageSizes.push_back( realHeigth );
		}

		// Rotate back down 90° => initial view
		rotPitch( -90.0 );

		emit sStatusMessage( "Side- and top-views saved using vertical rotation." );
		return;
	}

	// Rotate about the horizontal axis - cuneiform style
	//===================================================
	// ... and rotate up 90° ...
	// View 1: Top
	rotPitch( -90.0 );
	//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 1, "ha_top" );
	buffer = rFilePattern.arg(rFilePrefix).arg(1,2,10,QChar('0')).arg("ha_top");
	if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
		rImageFiles.push_back( buffer );
		rImageSizes.push_back( realWidth );
		rImageSizes.push_back( realHeigth );
	}

	// View 3: Front
	rotPitch( +90.0 );
	//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 3, "ha_front" );
	buffer = rFilePattern.arg(rFilePrefix).arg(3,2,10,QChar('0')).arg("ha_front");
	if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
		rImageFiles.push_back( buffer );
		rImageSizes.push_back( realWidth );
		rImageSizes.push_back( realHeigth );
	}

	// View 5: Bottom
	rotPitch( +90.0 );
	//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 5, "ha_bottom" );
	buffer = rFilePattern.arg(rFilePrefix).arg(5,2,10,QChar('0')).arg("ha_bottom");
	if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
		rImageFiles.push_back( buffer );
		rImageSizes.push_back( realWidth );
		rImageSizes.push_back( realHeigth );
	}

	// View 6: Back
	rotPitch( +90.0 );
	//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 6, "ha_back" );
	buffer = rFilePattern.arg(rFilePrefix).arg(6,2,10,QChar('0')).arg("ha_back");
	if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
		rImageFiles.push_back( buffer );
		rImageSizes.push_back( realWidth );
		rImageSizes.push_back( realHeigth );
	}

	// View 7: Backleft
	rotYaw( -90.0 );
	if( !sixViews ) { // Extra views - typically only usefull for cuneiform tablets rendered with light
		//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 7, "ha_back_left" );
		buffer = rFilePattern.arg(rFilePrefix).arg(7,2,10,QChar('0')).arg("ha_back_left");
		if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
			rImageFiles.push_back( buffer );
			rImageSizes.push_back( realWidth );
			rImageSizes.push_back( realHeigth );
		}
	}

	// View 8: Backright
	rotYaw( +180.0 );
	if( !sixViews ) { // Extra views - typically only usefull for cuneiform tablets rendered with light
		//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 8, "ha_back_right" );
		buffer = rFilePattern.arg(rFilePrefix).arg(8,2,10,QChar('0')).arg("ha_back_right");
		if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
			rImageFiles.push_back( buffer );
			rImageSizes.push_back( realWidth );
			rImageSizes.push_back( realHeigth );
		}
	}

	// Rotate to initial view:
	rotYaw( -90.0 );
	rotPitch( +180.0 );

	// View 4: Right
	rotYaw( +90.0 );
	//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 4, "ha_right" );
	buffer = rFilePattern.arg(rFilePrefix).arg(4,2,10,QChar('0')).arg("ha_right");
	if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
		rImageFiles.push_back( buffer );
		rImageSizes.push_back( realWidth );
		rImageSizes.push_back( realHeigth );
	}

	// View 2: Left
	rotYaw( -180.0 );
	//sprintf( buffer, rFilePattern.c_str(), rFilePrefix.c_str(), 2, "ha_left" );
	buffer = rFilePattern.arg(rFilePrefix).arg(2,2,10,QChar('0')).arg("ha_left");
	if( screenshotSingle( buffer, rUseTiled, realWidth, realHeigth ) ) {
		rImageFiles.push_back( buffer );
		rImageSizes.push_back( realWidth );
		rImageSizes.push_back( realHeigth );
	}

	// Rotate to initial view:
	rotYaw( +90.0 );

	emit sStatusMessage( "Side- and top-views using horizontal rotation were saved." );
}


//! Copies the Framebuffer to an RGB array, which can be stored as an Image.
//!
//! Part 1 of 3: ask for filename
bool MeshWidget::screenshotSingle() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Sanity check
	if( mMeshVisual == nullptr ) {
		return false;
	}

    QString filePath    = QString::fromStdWString( mMeshVisual->getFileLocation().wstring());
    QString fileSuggest = QString::fromStdWString( mMeshVisual->getBaseName().wstring() ) + ".png";

	QStringList filters;
    filters << tr("Image (*.png)");

	QFileDialog dialog( mMainWindow );
	dialog.setWindowTitle( tr("Save screenshot as:") );
	dialog.setOptions( QFileDialog::DontUseNativeDialog ); // without there is no suggested filename - at least using Ubuntu.
	dialog.setFileMode( QFileDialog::AnyFile );
	dialog.setAcceptMode( QFileDialog::AcceptSave );
	dialog.setDirectory( filePath );
	dialog.selectFile( fileSuggest );
	dialog.setNameFilters( filters );
	if( dialog.exec() != QDialog::Accepted ) {
		return( false ); // User cancel
	}
	QStringList fileNames = dialog.selectedFiles();
	if( fileNames.at( 0 ).length() == 0 ) { // Sanity
		return( false );
	}

	return screenshotSingle( fileNames.at( 0 ) );
}

//! Copies the Framebuffer to an RGB array, which can be stored as an Image.
//!
//! Part 2 of 3: Ask for tiled rendering and histogram(s)
bool MeshWidget::screenshotSingle( const QString& rFileName ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//! .) Ask for tiled rendering
	bool useTiled = false;
	{
		bool userCancel;
		SHOW_QUESTION( tr("Tiled rendering"), tr("Do you want to use tiled rendering?"), useTiled, userCancel );
		if( userCancel ) {
			return( false );
		}
	}

	//! .) Ask for histogram(s), when activated
	bool histogramShown = false;
	bool histogramScenceShown = false;
	getParamFlagMeshWidget( SHOW_HISTOGRAM, &histogramShown );
	getParamFlagMeshWidget( SHOW_HISTOGRAM_SCENE, &histogramScenceShown );
	if( histogramShown || histogramScenceShown ) {
		bool userCancel;
		bool discardHistograms = true;
		SHOW_QUESTION( tr("Histogram(s) shown"),
					   tr("Do you want to discard the histogram(s) for the screenshot?") + QString("<br /><br />") + tr("Recommended: YES"),
		               discardHistograms, userCancel );
		if( userCancel ) {
			return( false );
		}
		if( discardHistograms ) {
			setParamFlagMeshWidget( SHOW_HISTOGRAM, false );
			setParamFlagMeshWidget( SHOW_HISTOGRAM_SCENE, false );
		}
	}

	//! .) Execute
	double realWidth, realHeigth;
	return( screenshotSingle( rFileName, useTiled, realWidth, realHeigth ) );
}

//! Copies the Framebuffer to an RGB array, which can be stored as an Image.
//!
//! Part 3 of 3: determine file type, fetch and write.
bool MeshWidget::screenshotSingle( const QString&   rFileName,   //!< Filename to write.
                                   bool             rUseTiled,   //!< Tiled rendering.
                                   double&          rWidthReal,  //!< Return value: width in real world coordinates - only in orthographic projection. NaN otherwise.
                                   double&          rHeigthReal  //!< Return value: heigth in real world coordinates - only in orthographic projection. NaN otherwise.
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//! Deactives options for tiled rendering: grid center, histograms and keyboard layout.
	if( rUseTiled ) {
		setParamFlagMeshWidget( SHOW_KEYBOARD_CAMERA,      false );
		setParamFlagMeshWidget( SHOW_HISTOGRAM,            false );
		setParamFlagMeshWidget( SHOW_HISTOGRAM_SCENE,      false );
		setParamFlagMeshWidget( SHOW_GRID_HIGHLIGHTCENTER, false );
	}

	// Reset return value
	rWidthReal  = _NOT_A_NUMBER_DBL_;
	rHeigthReal = _NOT_A_NUMBER_DBL_;

	QString fileExtension = QFileInfo(rFileName).suffix().toLower();

	if( fileExtension.isEmpty() ) {
        cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No extension/type for file '" << rFileName.toStdString() << "' specified!" << endl;
        return( false );
	}

	cout << "[MeshWidget::" << __FUNCTION__ << "] extension: " << fileExtension.toStdString() << endl;

	bool ret = false;

	{
		OffscreenBuffer offscreenBuffer(context());
		int defaultFramebuffer;
		mMeshVisual->getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultFramebuffer);
		mMeshVisual->setParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, offscreenBuffer.getFboID());
		repaint();


        if( rUseTiled ) {
            int shaderChoice;
            mMeshVisual->getParamIntMeshGL( MeshGLParams::SHADER_CHOICE, &shaderChoice );
            bool drawNPR = ( MeshGLParams::SHADER_NPR==shaderChoice );
            if( !drawNPR ) {
                ret = screenshotTiledPNG( rFileName, rWidthReal, rHeigthReal, &offscreenBuffer, 1 );
            } else {
                double borderSize;
                mMeshVisual->getParamFloatMeshGL( MeshGLParams::NPR_OUTLINE_WIDTH, &borderSize );
                ret = screenshotTiledPNG( rFileName, rWidthReal, rHeigthReal, &offscreenBuffer, static_cast<int>(borderSize) );
            }
        } else {
            ret = screenshotPNG( rFileName, rWidthReal, rHeigthReal, &offscreenBuffer );
        }


		bindFramebuffer(defaultFramebuffer);

		mMeshVisual->setParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, defaultFramebuffer);
		repaint();
	}
	if(!ret)
		cerr << "[MeshIO::" << __FUNCTION__ << "] Unknown extension/type '" << fileExtension.toStdString() << "' specified!" << endl;
	return( ret );
}


//! Render current view as PNG and embed it within a PDF using a LaTeX template.
//!
//! User interaction: select PDF for currently presented mesh.
//! In case evince is installed the PDF will be shown.
//!
//! @returns false in case of an error. True otherwise.
bool MeshWidget::screenshotPDFUser() {
	// Sanity check
	if( mMeshVisual == nullptr ) {
		return( false );
	}
    QString filePrefix = QString::fromStdWString(mMeshVisual->getBaseName().wstring());
    QString filePath = QString::fromStdWString( mMeshVisual->getFileLocation().wstring() );
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] filePath:        " << filePath.toStdString() << std::endl;
	//qDebug() << filePath + QString( fileNamePattern.c_str() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Save as - Using a pattern for side, top and bottom views" ), \
	                                                 filePath + "/" + filePrefix + ".pdf",
													 tr( "Document (*.pdf)" ),
	                                                 nullptr,
	                                                 QFileDialog::DontUseNativeDialog  ); // Native dialog won't show patterns anymore on recent versions of Qt+Linux.
	// User cancel
	if( fileName == nullptr ) {
		return( false );
	}

	// Ask for tiled rendering
    bool userCancel;
	bool useTiled = false;
	{

		SHOW_QUESTION( tr("Tiled rendering"), tr("Do you want to use tiled rendering?") + QString("\n\n") + tr("Recommended: YES"), useTiled, userCancel );
		if( userCancel ) {
			return( false );
		}
	}

    QStringList templates;
    templates.append( "Single page" );
    templates.append( "Single page with cc-license" );
    templates.append( "Own template" );

    QString rTemplate;
    SHOW_DIALOG_COMBO_BOX( tr("Template"), tr("Which template do you want to use?"), templates, rTemplate, userCancel );
    if( userCancel ) {
        return( false );
    }

    //load user individual template
    QString texFileName;
    if( rTemplate == "Own template"){

        texFileName = QFileDialog::getOpenFileName( this,
                                                          tr( "Import Latex template" ),
                                                          nullptr,
                                                          tr( "tex-file (*.tex)" )
                                                         );
        if( texFileName == nullptr) {
            return( false );
        }
    }
    //ask for cc license parameters and Version
    QString ccParameters = "by-sa";
    QString ccVersion = "4.0";
    if( rTemplate == "Single page with cc-license"){
        if(!askForCCLicenseParameters(&ccParameters,&ccVersion)){
            return( false );
        }
    }
	// Execute
    if( !screenshotPDF( fileName, useTiled, rTemplate, texFileName, ccParameters, ccVersion ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: screenshotPDF(...) failed!" << std::endl;
		return( false );
	}

    // --- Show PDF, when evince, okular or atril is available --------------------------------------------------------------------------------------------------------------

	std::string pdfViewerCommand;
	getParamStringMeshWidget(MeshWidgetParams::PDF_VIEWER_COMMAND, &pdfViewerCommand);

#ifdef WIN32
	fileName.replace( QString("/") , QString("\\"));
#endif

    if( !QProcess::startDetached( QString(pdfViewerCommand.c_str()) + " \"" + fileName + "\"" ) ) {
        std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Program " + pdfViewerCommand + " not found!" << std::endl;
        //Try okular
        getParamStringMeshWidget(MeshWidgetParams::PDF_VIEWER_COMMAND_ALT1, &pdfViewerCommand);

        if( !QProcess::startDetached( QString(pdfViewerCommand.c_str()) + " \"" + fileName + "\"" ) ) {
            std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Program " + pdfViewerCommand + " not found!" << std::endl;

            //Try atril
            getParamStringMeshWidget(MeshWidgetParams::PDF_VIEWER_COMMAND_ALT2, &pdfViewerCommand);

            if( !QProcess::startDetached( QString(pdfViewerCommand.c_str()) + " \"" + fileName + "\"" ) ) {
                std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Program " + pdfViewerCommand + " not found!" << std::endl;
            }
        }

    }
	// -----------------------------------------------------------------------------------------------------------------------------------------------------
    //set false, that the userindividual latex template will check next time
    mUserContinue = false;
	return( true );
}

//! Render current view as PNG and embed it within a PDF using a LaTeX template.
//!
//! Accepts a filename to write a PDF with views for the current mesh (MeshWidget::mMeshVisual).
//!
//! @todo GUI Questions for the cc license parameters
//! @returns false in case of an error. True otherwise.
bool MeshWidget::screenshotPDF( const QString& rFileName, const bool rUseTiled, const QString rTemplate, const QString rTexFileName, const QString rCCparameter, const QString rCCversion ) {
	// Prepare filename.
	string prefixPath = std::filesystem::path( rFileName.toStdString() ).parent_path().string();
	string prefixStem = std::filesystem::path( rFileName.toStdString() ).stem().string();
	QString filePrefixTex = QString( prefixPath.c_str() ) + "/" + QString( prefixStem.c_str() );

	// Render screenshots.
	double widthReal;
	double heigthReal;
	screenshotSingle( filePrefixTex+".png", rUseTiled, widthReal, heigthReal );
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] allImageWidth: "  << widthReal << std::endl;
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] allImageHeigth: " << heigthReal << std::endl;
	double scaleFactor    = 1.0;
	QString scaleFactorTex( "1:1" );

	// DIN A4: 210 x 297 | Template borders: 10 mm
	double borderWidth = 10.0 * 2.0;
	double ratioWidth  = widthReal  / ( 210.0 - borderWidth - 20.0 );
	double ratioHeigth = heigthReal / ( 297.0 - borderWidth - 45.0 ); // for table heigth
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] ratioWidth:  " << ratioWidth  << std::endl;
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] ratioHeigth: " << ratioHeigth << std::endl;
	double maxRatio = max( ratioWidth, ratioHeigth );
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] maxRatio: " << maxRatio << std::endl;
	double log10pow = floor( log10( maxRatio ) );
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] log10pow: " << log10pow << std::endl;
	scaleFactor       = 2.0/( ceil( maxRatio*2.0/pow( 10.0, log10pow ) )*pow( 10.0, log10pow ) );
	scaleFactorTex    = "1:" + QString( "%1" ).arg( 1.0/scaleFactor, 'f' ).trimmed() ;

	// Load template
    //standard = single page
	QString latexTemplateName( ":/GMLaTeX/report_single_page_single_view_template.tex" );
    //other templates
    if(rTemplate == "Single page with cc-license"){
        latexTemplateName = ":/GMLaTeX/report_single_page_single_view_cclicense_template.tex";
    }
    if( rTemplate == "Own template"){
        latexTemplateName = rTexFileName;
    }
	QString latexTemplate;
	QFile fileLatexIn( latexTemplateName );
	if( !fileLatexIn.open( QIODevice::ReadOnly ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Template could not be opened!" << endl;
		return( false );
	} else {
		latexTemplate = fileLatexIn.readAll();
	}
	fileLatexIn.close();

    //check user defined Latex file
    if( rTemplate == "Own template" && !mUserContinue){
        if(!checkUserdefinedLatexFile(&latexTemplate,mPdfSinglePlaceholders)){
            cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: User stopped process" << endl;
            return( false );
        }
        else{
            mUserContinue = true;
        }
    }
	// Fetch strings with information for the table
	vector<pair<string,string>> replacmentStrings;
	mMeshVisual->latexFetchFigureInfos( &replacmentStrings );

    //replace license placeholders
    latexTemplate.replace( QRegExp( "__CC_PARAMETERS__" ), rCCparameter );
    latexTemplate.replace( QRegExp( "__CC_VERSION__" ), rCCversion );

	// Replace placeholders
	latexTemplate.replace( QRegExp( "__FIGURE_IMAGE_FILE__" ), "\""+QString( prefixStem.c_str() )+"\"" );
	latexTemplate.replace( QRegExp( "__SCALE_FACTOR_STRING__" ), scaleFactorTex );
	latexTemplate.replace( QRegExp( "__SCALE_FACTOR__" ), QString( "%1" ).arg( scaleFactor, 'f' ).trimmed() );
	for( pair<string, string>& replacmentString : replacmentStrings ) {
		string placeHolder = replacmentString.first;
		string content = replacmentString.second;
		latexTemplate.replace( QString( placeHolder.c_str() ), QString( content.c_str() ) );
	}
	//cout << latexTemplate.toStdString() << endl;

	// Write LaTex file
	QFile fileLatexOut( filePrefixTex + ".tex" );
	if( !fileLatexOut.open( QIODevice::WriteOnly ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: LaTeX file could not be opened!" << std::endl;
		return( false );
	}

	fileLatexOut.write( latexTemplate.toStdString().data() );
	fileLatexOut.close();

	// Compile and show LaTeX file.

	QString qPrefixPath(prefixPath.c_str());
#ifdef WIN32
	qPrefixPath.replace(QString("/"), QString("\\"));
	filePrefixTex.replace(QString("/"), QString("\\"));
#endif

	if( !screenshotPDFMake( qPrefixPath , filePrefixTex ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: LaTeX file could not be compiled!" << std::endl;
		return( false );
	}

	return( true );
}

// Fetch screenshots -------------------------------------------------------------------------------------------

bool MeshWidget::prepareTile( uint64_t    rTilesX,  //!< in: number of tiles in X direction
                          uint64_t    rTilesY,  //!< in: number of tiles in Y direction
	                      unsigned char** rImRGBA,  //!< out: array to store the final image
                          uint64_t*   rImWidth, //!< out: framebuffer width
                          uint64_t*   rImHeight, //!< out: framebuffer height
                          uint64_t    rBorderSize
    ) {
	//! Prepares and RGBA array to fetch rTilesX by rTilesY tiles from the frame and z-buffer.
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	(*rImWidth)  = viewport[2] - 2*rBorderSize;
	(*rImHeight) = viewport[3] - 2*rBorderSize;
	cout << "[MeshWidget::" << __FUNCTION__ << "] Size: " << rTilesX << "*" << rTilesY << "*" << (*rImWidth) << "*" << 4 << "*" << (*rImHeight) << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] Size: " << rTilesX*rTilesY*(*rImWidth)*4*(*rImHeight) << endl;
	(*rImRGBA) = new unsigned char[rTilesX*rTilesY*(*rImWidth)*4*(*rImHeight)];
	return true;
}

//! Fetch image data (RGBA) from the frame and z-buffer.
//! As this method is called in a loop we need to pre-allocate imArrayGL and pixelZBuffer,
//! because allocating and deleting with an loop will cause an exception.
bool MeshWidget::fetchFrameAndZBufferTile( unsigned int rTilesX,    //!< Number of tiles - horizontally
                                           unsigned int rTilesY,    //!< Number of tiles - vertically
                                           unsigned int rTX,        //!< Current vertical tile position
                                           unsigned int rTY,        //!< Current horizontal tile position
                                           unsigned char* rImRGBA,  //!< Array for the full image
                                           uint64_t rImWidth,  //!< Widht of the frame buffer
                                           uint64_t rImHeight, //!< Height of the frame buffer
										   OffscreenBuffer* offscreenBuffer, //!< Offscreenbuffer to fetch the render-result from
                                           long       rBorderSize
	) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Checks:
	if( rTX >= rTilesX ) {
		return false;
	}
	if( rTY >= rTilesY ) {
		return false;
	}

	unsigned char* colorData = nullptr;
	offscreenBuffer->getColorTextureRegion(colorData, rImWidth, rImHeight, rBorderSize, rBorderSize);

	float* depthData = nullptr;
	offscreenBuffer->getDepthTextureRegion(depthData, rImWidth, rImHeight, rBorderSize, rBorderSize);

	/*
	glReadPixels( rBorderSize, rBorderSize, rImWidth, rImHeight, GL_DEPTH_COMPONENT, GL_FLOAT, rPixelZBuffer );
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glReadPixels( rBorderSize, rBorderSize, rImWidth, rImHeight, GL_RGB, GL_UNSIGNED_BYTE, rImArrayGL );
	*/

	uint64_t tileOffsetX = rImWidth*4*static_cast<uint64_t>(rTX);
	uint64_t tileOffsetY = rImWidth*4*rImHeight*static_cast<uint64_t>(rTilesX)*rTY;
	for( unsigned int iy=0; iy<rImHeight; iy++ ) {
		for( unsigned int ix=0; ix<rImWidth; ix++ ) {
			uint64_t posInTile = tileOffsetX+tileOffsetY+(ix+iy*rImWidth*rTilesX)*4;
			rImRGBA[posInTile]   = colorData[(rImWidth-1-ix)*3+  (iy)*rImWidth*3];
			rImRGBA[posInTile+1] = colorData[(rImWidth-1-ix)*3+1+(iy)*rImWidth*3];
			rImRGBA[posInTile+2] = colorData[(rImWidth-1-ix)*3+2+(iy)*rImWidth*3];
			if( depthData[(rImWidth-1-ix)+(iy)*rImWidth] < 1.0f ) {
				rImRGBA[posInTile+3] = 255;
			} else {
				// Use white color together with full transparency
				rImRGBA[posInTile]   = 255; //! \todo fetch background color used by the widget
				rImRGBA[posInTile+1] = 255;
				rImRGBA[posInTile+2] = 255;
				rImRGBA[posInTile+3] =   0;
			}
		}
	}

	delete[] depthData;
	delete[] colorData;
	return true;
}

//! Fetch image data (RGBA) from the frame and z-buffer.
//!
//! Typically used for PNGs, which support transparency.
//!
//! Optional flag: crop using the z-buffer.
bool MeshWidget::fetchFrameAndZBuffer(unsigned char*&   rImRGBA,          //!< Image as RGBA byte array (return value)
                uint64_t&    rImWidth,         //!< Image heigth (return value)
                uint64_t&    rImHeight,        //!< Image width (return value)
                bool              rCropUsingZBuffer, //!< Crop image using the z-buffer
                OffscreenBuffer*  offscreenBuffer,	//!< Offscreenbuffer to fetch the render-result from
                bool keepBackground  //!< keep background or replace it by transparency
                                      ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//rImWidth  = width();
	//rImHeight = height();


	//GLubyte* imArrayGL  = new GLubyte[rImWidth*3*rImHeight];
	//float* pixelZBuffer = new float[rImWidth*rImHeight];


	//glReadPixels( 0, 0, static_cast<GLsizei>(rImWidth), static_cast<GLsizei>(rImHeight), GL_DEPTH_COMPONENT, GL_FLOAT, pixelZBuffer );
	// OpenGL's default 4 byte pack alignment would leave extra bytes at the
	// end of each image row so that each full row contained a number of bytes
	// divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit components would
	// be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
	// just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
	// the rows are packed as tight as possible (no row padding), set the pack
	// alignment to 1.
	//glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	//glReadPixels( 0, 0, static_cast<GLsizei>(rImWidth), static_cast<GLsizei>(rImHeight), GL_RGB, GL_UNSIGNED_BYTE, imArrayGL );
	unsigned char* imArrayGL;
	float* pixelZBuffer;

	int colWidth, colHeight;

	imArrayGL = offscreenBuffer->getColorTexture(colWidth, colHeight);
	pixelZBuffer = offscreenBuffer->getDepthTexture(colWidth, colHeight);
	rImWidth = colWidth;
	rImHeight = colHeight;
	rImRGBA   = new unsigned char[rImWidth*4*rImHeight];


	// Set the crop size to the image size. Otherwise the values are invalid, when there is nothing to crop.
	uint64_t xMin = 0;
	uint64_t xMax = rImWidth-1;
	uint64_t yMin = 0;
	uint64_t yMax = rImHeight-1;

	for( uint64_t iy=0; iy<rImHeight; iy++ ) {
		for( uint64_t ix=0; ix<rImWidth; ix++ ) {
			rImRGBA[(ix+iy*rImWidth)*4]   = imArrayGL[ix*3+  (rImHeight-1-iy)*rImWidth*3];
			rImRGBA[(ix+iy*rImWidth)*4+1] = imArrayGL[ix*3+1+(rImHeight-1-iy)*rImWidth*3];
			rImRGBA[(ix+iy*rImWidth)*4+2] = imArrayGL[ix*3+2+(rImHeight-1-iy)*rImWidth*3];
			rImRGBA[(ix+iy*rImWidth)*4+3] = 255;
			if(!keepBackground)
			{
				if( pixelZBuffer[ix+(rImHeight-1-iy)*rImWidth] < 1.0f ) {
					if( ix < xMin ) {
						xMin = ix;
					}
					if( ix > xMax ) {
						xMax = ix;
					}
					if( iy < yMin ) {
						yMin = iy;
					}
					if( iy > yMax ) {
						yMax = iy;
					}
				} else {
					// Use white color together with full transparency
					rImRGBA[(ix+iy*rImWidth)*4]   = 255; //! \todo fetch background color used by the widget
					rImRGBA[(ix+iy*rImWidth)*4+1] = 255;
					rImRGBA[(ix+iy*rImWidth)*4+2] = 255;
					rImRGBA[(ix+iy*rImWidth)*4+3] =   0;
				}
			}
		}
	}
	if( rCropUsingZBuffer ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Crop x: " << xMin << " - " << xMax << endl;
		cout << "[MeshWidget::" << __FUNCTION__ << "] Crop y: " << yMin << " - " << yMax << endl;
		uint64_t imWidthCrop  = xMax - xMin + 1;
		uint64_t imHeightCrop = yMax - yMin + 1;
		unsigned char* imArrayCrop  = new unsigned char[imWidthCrop*4*imHeightCrop];
		for( uint64_t iy=0; iy<imHeightCrop; iy++ ) {
			for( uint64_t ix=0; ix<imWidthCrop; ix++ ) {
				imArrayCrop[(ix+iy*imWidthCrop)*4]   = rImRGBA[(ix+xMin)*4+  ((iy+yMin))*rImWidth*4];
				imArrayCrop[(ix+iy*imWidthCrop)*4+1] = rImRGBA[(ix+xMin)*4+1+((iy+yMin))*rImWidth*4];
				imArrayCrop[(ix+iy*imWidthCrop)*4+2] = rImRGBA[(ix+xMin)*4+2+((iy+yMin))*rImWidth*4];
				imArrayCrop[(ix+iy*imWidthCrop)*4+3] = rImRGBA[(ix+xMin)*4+3+((iy+yMin))*rImWidth*4];
			}
		}
		delete[] ( rImRGBA );
		rImRGBA   = imArrayCrop;
		rImWidth  = imWidthCrop;
		rImHeight = imHeightCrop;
	}

	return( true );
}

//! Fetch image data (RGB) from the frame.
//!
//! Typically used for TIFFs, which do not support any transparency.
//!
//! Optional flag: crop using the z-buffer.
bool MeshWidget::fetchFrameBuffer(
                unsigned char** rImArray,
                int* rImWidth,
                int* rImHeight,
				bool rCropUsingZBuffer,
				OffscreenBuffer* offscreenBuffer
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	/*
	(*rImWidth)  = width();
	(*rImHeight) = height();
	(*rImArray)  = new unsigned char[(*rImWidth)*3*(*rImHeight)];

	GLubyte* imArrayGL = new GLubyte[(*rImWidth)*3*(*rImHeight)];

	// Set the crop size to the image size. Otherwise the values are invalid, when there is nothing to crop.
	int xMin = 0;
	int xMax = (*rImWidth)-1;
	int yMin = 0;
	int yMax = (*rImHeight)-1;

	float* pixelZBuffer = nullptr;
	if( rCropUsingZBuffer ) {
		pixelZBuffer = new float[(*rImWidth)*(*rImHeight)];
		glReadPixels( 0, 0, (*rImWidth), (*rImHeight), GL_DEPTH_COMPONENT, GL_FLOAT, pixelZBuffer );
	}
	// OpenGL's default 4 byte pack alignment would leave extra bytes at the
	// end of each image row so that each full row contained a number of bytes
	// divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit components would
	// be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
	// just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
	// the rows are packed as tight as possible (no row padding), set the pack
	// alignment to 1.
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glReadPixels( 0, 0, (*rImWidth), (*rImHeight), GL_RGB, GL_UNSIGNED_BYTE, imArrayGL );
	*/

	unsigned char* imArrayGL;
	int w;
	int h;
	imArrayGL = offscreenBuffer->getColorTexture(w, h);
	float* pixelZBuffer = nullptr;

	*rImWidth = w;
	*rImHeight = h;
	*rImArray = new unsigned char[w*h*3];
	// Set the crop size to the image size. Otherwise the values are invalid, when there is nothing to crop.
	int xMin = 0;
	int xMax = (*rImWidth)-1;
	int yMin = 0;
	int yMax = (*rImHeight)-1;

	if(rCropUsingZBuffer)
	{
		pixelZBuffer = offscreenBuffer->getDepthTexture(w, h);
	}

	for( int iy=0; iy<(*rImHeight); iy++ ) {
		for( int ix=0; ix<(*rImWidth); ix++ ) {
			(*rImArray)[(ix+iy*(*rImWidth))*3]   = imArrayGL[ix*3+  ((*rImHeight)-1-iy)*(*rImWidth)*3];
			(*rImArray)[(ix+iy*(*rImWidth))*3+1] = imArrayGL[ix*3+1+((*rImHeight)-1-iy)*(*rImWidth)*3];
			(*rImArray)[(ix+iy*(*rImWidth))*3+2] = imArrayGL[ix*3+2+((*rImHeight)-1-iy)*(*rImWidth)*3];
			if( ( pixelZBuffer != nullptr ) && ( pixelZBuffer[ix+((*rImHeight)-1-iy)*(*rImWidth)] < 1.0 ) ) {
				if( ix < xMin ) {
					xMin = ix;
				}
				if( ix > xMax ) {
					xMax = ix;
				}
				if( iy < yMin ) {
					yMin = iy;
				}
				if( iy > yMax ) {
					yMax = iy;
				}
			}
		}
	}
	if( rCropUsingZBuffer ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Crop x: " << xMin << " - " << xMax << endl;
		cout << "[MeshWidget::" << __FUNCTION__ << "] Crop y: " << yMin << " - " << yMax << endl;
		int imWidthCrop  = xMax - xMin;
		int imHeightCrop = yMax - yMin;
		unsigned char* imArrayCrop  = new unsigned char[imWidthCrop*3*imHeightCrop];
		for( int iy=0; iy<imHeightCrop; iy++ ) {
			for( int ix=0; ix<imWidthCrop; ix++ ) {
				imArrayCrop[(ix+iy*imWidthCrop)*3]   = imArrayGL[(ix+xMin)*3+  ((*rImHeight)-1-(iy+yMin))*(*rImWidth)*3];
				imArrayCrop[(ix+iy*imWidthCrop)*3+1] = imArrayGL[(ix+xMin)*3+1+((*rImHeight)-1-(iy+yMin))*(*rImWidth)*3];
				imArrayCrop[(ix+iy*imWidthCrop)*3+2] = imArrayGL[(ix+xMin)*3+2+((*rImHeight)-1-(iy+yMin))*(*rImWidth)*3];
			}
		}
		delete[] (*rImArray);
		(*rImArray)  = imArrayCrop;
		(*rImWidth)  = imWidthCrop;
		(*rImHeight) = imHeightCrop;
		
	}
	return( true );
}

// Write screenshots -------------------------------------------------------------------------------------------------------------------------------------------

//! Saves the PNG with transparency to the given filename.
bool MeshWidget::screenshotPNG(const QString& rFileName,
                                double&         rWidthReal,
                                double&         rHeigthReal,
                                OffscreenBuffer* offscreenBuffer
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	cout << "[MeshWidget::" << __FUNCTION__ << "] " << rFileName.toStdString() << endl;

	// Set default for non-orthographic projection and errors:
	rWidthReal  = _NOT_A_NUMBER_DBL_;
	rHeigthReal = _NOT_A_NUMBER_DBL_;

	uint64_t imWidth;
	uint64_t imHeight;
	unsigned char* imRGBA;

	bool opaqueBackground = false;
	MeshWidgetParams::getParamFlagMeshWidget(MeshWidgetParams::SCREENSHOT_PNG_BACKGROUND_OPAQUE, &opaqueBackground);

	if( !fetchFrameAndZBuffer( imRGBA, imWidth, imHeight, mParamFlag[CROP_SCREENSHOTS], offscreenBuffer, opaqueBackground ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: fetchFrameAndZBuffer failed!" << endl;
		return( false );
	}

	// Set resolution in DPI, when the orthographic projection is used.
	bool orthoMode = false;
	if( !getParamFlagMeshWidget( ORTHO_MODE, &orthoMode ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getParamFlagMeshWidget failed!" << endl;
		return( false );
	}

	// Compute dots per meter for PNG export with Qt
	double dpm = 0.0;

	if( !getViewPortDPM(dpm) )
	{
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getViewPortDPM failed!" << endl;
		return false;
	}

	// Write the file:
	bool writeOk = writePNG( rFileName, imWidth, imHeight, imRGBA, dpm, dpm );
	if( writeOk ) {
		if( orthoMode ) {
			getViewPortResolution(rWidthReal, rHeigthReal);
		}
		emit sStatusMessage( "Screenshot saved as PNG with transparency to: " + rFileName );
	} else {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Could not save screenshot as PNG with transparency to: " << rFileName.toStdString() << "!" << endl;
		emit sStatusMessage( "ERROR: Could not save screenshot as PNG with transparency to: " + rFileName + "!" );
	}

	// Free arrays:
	delete[] imRGBA;

	// Done
	return( writeOk );
}

//! called when the user request to select a background color.
void MeshWidget::selectColorBackground() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	QColor rgbNew = QColorDialog::getColor( Qt::white, nullptr, 
	                                        QString( "Background Color"), 
	                                        QColorDialog::ShowAlphaChannel );
	if( !rgbNew.isValid() ) { // Cancel was pressed.
		return;
	}
	qglClearColor( rgbNew );
	update();
}

//! Enter or exit cortex vertex selection mode
void MeshWidget::enterCortexSelectionMode(bool rEnabled) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "] " << (rEnabled ? "Entering" : "Exiting") << " cortex selection mode" << endl;
#endif
	mCortexSelectionModeActive = rEnabled;

	// TODO: Change cursor to crosshair when enabled
	// TODO: Implement mouse picking logic in mousePressEvent
	update();
}

//! Clear all cortex vertex flags
void MeshWidget::clearCortexSelection() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( mMeshVisual == nullptr ) {
		return;
	}

	// Use the public API to clear flags from all vertices
	mMeshVisual->clearVertexFlagForAll( Primitive::FLAG_CORTEX );

	// Invalidate VBO to force regeneration
	mMeshVisual->cortexVerticesChanged();

	// Trigger redraw
	update();
}

//! Select or deselect cortex vertex at screen coordinates using depth buffer picking
bool MeshWidget::selectCortexVertexAt( int xPixel, int yPixel ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "] x=" << xPixel << ", y=" << yPixel << endl;
#endif
	if( mMeshVisual == nullptr ) {
		return false;
	}

	// Get viewport and flip Y coordinate for OpenGL
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	int yPixelFlipped = viewport[3] - yPixel;

	// Use existing depth buffer picking to get vertex (addToList=false to avoid modifying selection)
	Primitive* clickedPrimitive = mMeshVisual->selectPrimitiveAt( Primitive::IS_VERTEX, xPixel, yPixelFlipped, false );

	if( clickedPrimitive == nullptr ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] No vertex found at clicked position" << endl;
		return false;
	}

	// Cast to Vertex and toggle FLAG_CORTEX
	Vertex* clickedVertex = static_cast<Vertex*>( clickedPrimitive );
	bool currentState = clickedVertex->getFlag( Primitive::FLAG_CORTEX );

	if( currentState ) {
		clickedVertex->clearFlag( Primitive::FLAG_CORTEX );
		cout << "[MeshWidget::" << __FUNCTION__ << "] Vertex cortex flag CLEARED" << endl;
	} else {
		clickedVertex->setFlag( Primitive::FLAG_CORTEX );
		cout << "[MeshWidget::" << __FUNCTION__ << "] Vertex cortex flag SET" << endl;
	}

	// Invalidate VBO to force regeneration with new cortex flags
	mMeshVisual->cortexVerticesChanged();

	// Trigger redraw
	update();
	return true;
}

//! Called when cortex vertex flags have changed - invalidates VBO
void MeshWidget::cortexVerticesChanged() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Trigger redraw - VBO will be regenerated automatically in paintGL()
	update();
}

// === MENU ===================================================================

// View menu -----------------------------------------------------------------

//! Get the current view settings including the view matrix as plain text.
//! 
//! @returns false in case of an error. True otherwise.
bool MeshWidget::getViewSettingsTxt( 
        QString& rSettingsStr
) const {
	//! \todo Revise to more descriptive format below as perpared for meta-data export.
	//-------------------------------------------------------------------------------------
	// Old style view settings format as one line. 
	//-------------------------------------------------------------------------------------
	rSettingsStr += QString( "%1"   ).arg( mCenterView.getX() );    // LookAt point X
	rSettingsStr += QString( " %1"  ).arg( mCenterView.getY() );    // LookAt point Y
	rSettingsStr += QString( " %1"  ).arg( mCenterView.getZ() );    // LookAt point Z
	rSettingsStr += QString( "  %1" ).arg( mCameraCenter.getX() );  // Camera center X
	rSettingsStr += QString( " %1"  ).arg( mCameraCenter.getY() );  // Camera center Y
	rSettingsStr += QString( " %1"  ).arg( mCameraCenter.getZ() );  // Camera center Z
	rSettingsStr += QString( "  %1" ).arg( mCameraUp.getX() );      // Camera orientation / up vector X
	rSettingsStr += QString( " %1"  ).arg( mCameraUp.getY() );      // Camera orientation / up vector Y
	rSettingsStr += QString( " %1"  ).arg( mCameraUp.getZ() );      // Camera orientation / up vector Z

	bool orthoMode;
	getParamFlagMeshWidget( ORTHO_MODE, &orthoMode );
	rSettingsStr += QString( "  %1").arg( orthoMode );            // Orthographic or perspective view

	if( orthoMode ) {
		double paramFloat;
		getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &paramFloat );
		rSettingsStr += QString( "  %1" ).arg( paramFloat );       // Vertical offset in orthograpic projection
		getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &paramFloat );
		rSettingsStr += QString( " %1" ).arg( paramFloat );       // Horizontal offset in orthograpic projection
		getViewPortDPI( paramFloat );
		rSettingsStr += QString( " %1" ).arg( paramFloat );       // Zoom factor (in DPI)
		// ORTHO_ZOOM does not work as expected, when the viewport is different!
		// getParamFloatMeshWidget( ORTHO_ZOOM, &paramFloat );
		// rSettingsStr += QString( " %1" ).arg( paramFloat );       // Zoom factor (relates to DPI)
	}
	rSettingsStr += "\n\nAbove old style formating: LookAt(x,y,z) Camera center(x,y,z) and orientation (x,y,z) "
	                "Orthographic projection(bool) ...shift (horizontal,vertical) ...zoom(float) in DPI\n";

	//-------------------------------------------------------------------------------------
	// Preparation for meta-data export
	//-------------------------------------------------------------------------------------
	// Additionally fetch the actual matrices:
	rSettingsStr += QString( "\nProjection matrix:" );
	const float* matProjection = mMatProjection.constData();
	for( unsigned int i=0; i<16; i++ ) {
		rSettingsStr += QString(" %1").arg( matProjection[i] );
	}
	rSettingsStr += QString( "\nModelview matrix:" );
	const float* matModelView = mMatModelView.constData();
	for( unsigned int i=0; i<16; i++ ) {
		rSettingsStr += QString(" %1").arg( matModelView[i] );
	}
	// As well as further parameters (again):
	double resolutionDPI = 0.0;
	double bBoxRadius = mMeshVisual->getBoundingBoxRadius(); // used together with ORTHO_ZOOM for the scaling (multiplication)
	double orthoZoom;
	double shiftHori;
	double shiftVert;
	getParamFloatMeshWidget( ORTHO_ZOOM, &orthoZoom );
	getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &shiftHori );
	getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &shiftVert );
	getViewPortDPI( resolutionDPI );
	rSettingsStr += QString( "\nDPI: %1" ).arg( resolutionDPI );
	rSettingsStr += QString( "\nShift, horizontally: %1" ).arg( shiftHori );
	rSettingsStr += QString( "\nShift, vertically: %1" ).arg( shiftVert );
	rSettingsStr += QString( "\nOrtho zoom: %1" ).arg( orthoZoom );
	rSettingsStr += QString( "\nBounding box radius: %1" ).arg( bBoxRadius );

	return( true );
}

//! Get the current view settings including the view matrix as JSON.
//! 
//! @returns false in case of an error. True otherwise.
bool MeshWidget::getViewSettingsJSON( 
        QString& rSettingsStr
) const {
	//! \todo Revise to more descriptive format below as perpared for meta-data export.
	//-------------------------------------------------------------------------------------
	// Old style view settings format as one line. 
	//-------------------------------------------------------------------------------------
	rSettingsStr += QString( "%1"   ).arg( mCenterView.getX() );    // LookAt point X
	rSettingsStr += QString( " %1"  ).arg( mCenterView.getY() );    // LookAt point Y
	rSettingsStr += QString( " %1"  ).arg( mCenterView.getZ() );    // LookAt point Z
	rSettingsStr += QString( "  %1" ).arg( mCameraCenter.getX() );  // Camera center X
	rSettingsStr += QString( " %1"  ).arg( mCameraCenter.getY() );  // Camera center Y
	rSettingsStr += QString( " %1"  ).arg( mCameraCenter.getZ() );  // Camera center Z
	rSettingsStr += QString( "  %1" ).arg( mCameraUp.getX() );      // Camera orientation / up vector X
	rSettingsStr += QString( " %1"  ).arg( mCameraUp.getY() );      // Camera orientation / up vector Y
	rSettingsStr += QString( " %1"  ).arg( mCameraUp.getZ() );      // Camera orientation / up vector Z

	bool orthoMode;
	getParamFlagMeshWidget( ORTHO_MODE, &orthoMode );
	rSettingsStr += QString( "  %1").arg( orthoMode );            // Orthographic or perspective view

	if( orthoMode ) {
		double paramFloat;
		getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &paramFloat );
		rSettingsStr += QString( "  %1" ).arg( paramFloat );       // Vertical offset in orthograpic projection
		getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &paramFloat );
		rSettingsStr += QString( " %1" ).arg( paramFloat );       // Horizontal offset in orthograpic projection
		getViewPortDPI( paramFloat );
		rSettingsStr += QString( " %1" ).arg( paramFloat );       // Zoom factor (in DPI)
		// ORTHO_ZOOM does not work as expected, when the viewport is different!
		// getParamFloatMeshWidget( ORTHO_ZOOM, &paramFloat );
		// rSettingsStr += QString( " %1" ).arg( paramFloat );       // Zoom factor (relates to DPI)
	}
	rSettingsStr += "\n\nAbove old style formating: LookAt(x,y,z) Camera center(x,y,z) and orientation (x,y,z) "
	                "Orthographic projection(bool) ...shift (horizontal,vertical) ...zoom(float) in DPI\n";

	//-------------------------------------------------------------------------------------
	// Preparation for meta-data export
	//-------------------------------------------------------------------------------------
	// Additionally fetch the actual matrices:
	rSettingsStr += QString( "\nProjection matrix:" );
	const float* matProjection = mMatProjection.constData();
	for( unsigned int i=0; i<16; i++ ) {
		rSettingsStr += QString(" %1").arg( matProjection[i] );
	}
	rSettingsStr += QString( "\nModelview matrix:" );
	const float* matModelView = mMatModelView.constData();
	for( unsigned int i=0; i<16; i++ ) {
		rSettingsStr += QString(" %1").arg( matModelView[i] );
	}
	// As well as further parameters (again):
	double resolutionDPI = 0.0;
	double bBoxRadius = mMeshVisual->getBoundingBoxRadius(); // used together with ORTHO_ZOOM for the scaling (multiplication)
	double orthoZoom;
	double shiftHori;
	double shiftVert;
	getParamFloatMeshWidget( ORTHO_ZOOM, &orthoZoom );
	getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &shiftHori );
	getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &shiftVert );
	getViewPortDPI( resolutionDPI );
	rSettingsStr += QString( "\nDPI: %1" ).arg( resolutionDPI );
	rSettingsStr += QString( "\nShift, horizontally: %1" ).arg( shiftHori );
	rSettingsStr += QString( "\nShift, vertically: %1" ).arg( shiftVert );
	rSettingsStr += QString( "\nOrtho zoom: %1" ).arg( orthoZoom );
	rSettingsStr += QString( "\nBounding box radius: %1" ).arg( bBoxRadius );

	return( true );
}


//! Get the current view settings including the view matrix as turtle.
//! 
//! @returns false in case of an error. True otherwise.
bool MeshWidget::getViewSettingsTTL( 
        QString& rSettingsStr, QString& uri
) const {
	//! \todo Revise to more descriptive format below as perpared for meta-data export.
	//-------------------------------------------------------------------------------------
	// Old style view settings format as one line. 
	//-------------------------------------------------------------------------------------
    //QString uri=QString("giga:123"); //Where do we get the Mesh ID?
    rSettingsStr += uri+" rdf:type giga:Mesh .\n";    
    rSettingsStr += "giga:lookAtPointX rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:lookAtPointX rdfs:label \"LookAt point X\"@en .\n";
    rSettingsStr += uri+" giga:lookAtPointX \""+QString( "%1"   ).arg( mCenterView.getX() )+"\"^^xsd:double .\n";    // LookAt point X
    rSettingsStr += "giga:lookAtPointY rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:lookAtPointY rdfs:label \"LookAt point Y\"@en .\n";
    rSettingsStr += uri+" giga:lookAtPointY \""+QString( "%1"   ).arg( mCenterView.getY() )+"\"^^xsd:double .\n";   
    rSettingsStr += "giga:lookAtPointZ rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:lookAtPointZ rdfs:label \"LookAt point Z\"@en .\n";
    rSettingsStr += uri+" giga:lookAtPointZ \""+QString( "%1"   ).arg( mCenterView.getZ() )+"\"^^xsd:double .\n";   
    rSettingsStr += "giga:cameraCenterX rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:cameraCenterX rdfs:label \"Camera Center X\"@en .\n";
    rSettingsStr += uri+" giga:cameraCenterX \""+QString( "%1"   ).arg( mCameraCenter.getX() )+"\"^^xsd:double .\n";   
    rSettingsStr += "giga:cameraCenterY rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:cameraCenterY rdfs:label \"Camera Center Y\"@en .\n";
    rSettingsStr += uri+" giga:cameraCenterY \""+QString( "%1"   ).arg( mCameraCenter.getY() )+"\"^^xsd:double .\n";   
    rSettingsStr += "giga:cameraCenterZ rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:cameraCenterZ rdfs:label \"Camera Center Z\"@en .\n";
    rSettingsStr += uri+" giga:cameraCenterZ \""+QString( "%1"   ).arg( mCameraCenter.getZ() )+"\"^^xsd:double .\n";   
    rSettingsStr += "giga:cameraUpX rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:cameraUpX rdfs:label \"Camera orientation / up vector X\"@en .\n";
    rSettingsStr += uri+" giga:cameraUpX \""+QString( "%1"   ).arg( mCameraUp.getX() )+"\"^^xsd:double .\n";   
    rSettingsStr += "giga:cameraUpY rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:cameraUpY rdfs:label \"Camera orientation / up vector Y\"@en .\n";
    rSettingsStr += uri+" giga:cameraUpY \""+QString( "%1"   ).arg( mCameraUp.getY() )+"\"^^xsd:double .\n";  
    rSettingsStr += "giga:cameraUpZ rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:cameraUpZ rdfs:label \"Camera orientation / up vector Z\"@en .\n";
    rSettingsStr += uri+" giga:cameraUpZ \""+QString( "%1"   ).arg( mCameraUp.getZ() )+"\"^^xsd:double .\n"; 
    rSettingsStr += "giga:orthoMode rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:orthoMode rdfs:label \"Orthographic view\"@en .\n";
    rSettingsStr += "giga:perspectiveView rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:perspectiveView rdfs:label \"Perspective view\"@en .\n";
    bool orthoMode;
	getParamFlagMeshWidget( ORTHO_MODE, &orthoMode );
    if(orthoMode){
        rSettingsStr += uri+" giga:orthoMode \"true\"^^xsd:boolean .\n"; 
        rSettingsStr += uri+" giga:perspectiveView \"false\"^^xsd:boolean .\n";     
    }else{
        rSettingsStr += uri+" giga:orthoMode \"false\"^^xsd:boolean .\n"; 
        rSettingsStr += uri+" giga:perspectiveView \"true\"^^xsd:boolean .\n";   
    }

	if( orthoMode ) {
		double paramFloat;
		getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &paramFloat );
        rSettingsStr += "giga:orthographicProjectionVerticalOffset rdf:type owl:DatatypeProperty .\n";
        rSettingsStr += "giga:orthographicProjectionVerticalOffset rdfs:label \"Vertical offset in orthographic projection\"@en .\n";
        rSettingsStr += uri+" giga:orthographicProjectionVerticalOffset \""+QString( "%1" ).arg( paramFloat )+"\"^^xsd:double .\n"; 
		getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &paramFloat );
        rSettingsStr += "giga:orthographicProjectionHorizontalOffset rdf:type owl:DatatypeProperty .\n";
        rSettingsStr += "giga:orthographicProjectionHorizontalOffset rdfs:label \"Horizontal offset in orthographic projection\"@en .\n";
        rSettingsStr += uri+" giga:orthographicProjectionHorizontalOffset \""+QString( "%1" ).arg( paramFloat )+"\"^^xsd:double .\n"; 
		getViewPortDPI( paramFloat );
        rSettingsStr += "giga:zoomFactor rdf:type owl:DatatypeProperty .\n";
        rSettingsStr += "giga:zoomFactor rdfs:label \"Zoom factor\"@en .\n";
        rSettingsStr += QString(uri+" giga:zoomFactor \""+QString( "%1" ).arg( paramFloat )+"\"^^xsd:double .\n"); 
		// ORTHO_ZOOM does not work as expected, when the viewport is different!
		// getParamFloatMeshWidget( ORTHO_ZOOM, &paramFloat );
		// rSettingsStr += QString( " %1" ).arg( paramFloat );       // Zoom factor (relates to DPI)
	}
    //rSettingsStr += "\n\nAbove old style formating: LookAt(x,y,z) Camera center(x,y,z) and orientation (x,y,z) "
	//                "Orthographic projection(bool) ...shift (horizontal,vertical) ...zoom(float) in DPI\n";

	//-------------------------------------------------------------------------------------
	// Preparation for meta-data export
	//-------------------------------------------------------------------------------------
	// Additionally fetch the actual matrices:
	const float* matProjection = mMatProjection.constData();
	rSettingsStr += "giga:projectionMatrix rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:projectionMatrix rdfs:label \"Projection Matrix\"@en .\n";
    rSettingsStr += QString(uri+" giga:projectionMatrix \"");
    for( unsigned int i=0; i<16; i++ ) {
		rSettingsStr += QString("%1").arg( matProjection[i] );
        rSettingsStr += ";";
	}
	rSettingsStr+="\"^^xsd:string .\n"; 
	rSettingsStr += "giga:modelViewMatrix rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:modelViewMatrix rdfs:label \"Modelview Matrix\"@en .\n";
    rSettingsStr += QString(uri+" giga:modelViewMatrix \"");
	const float* matModelView = mMatModelView.constData();
	for( unsigned int i=0; i<16; i++ ) {
		rSettingsStr += QString("%1").arg( matModelView[i] );
        rSettingsStr += ";";
	}
	rSettingsStr+="\"^^xsd:string .\n"; 
	// As well as further parameters (again):
	double resolutionDPI = 0.0;
	double bBoxRadius = mMeshVisual->getBoundingBoxRadius(); // used together with ORTHO_ZOOM for the scaling (multiplication)
	double orthoZoom;
	double shiftHori;
	double shiftVert;
	getParamFloatMeshWidget( ORTHO_ZOOM, &orthoZoom );
	getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &shiftHori );
	getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &shiftVert );
	getViewPortDPI( resolutionDPI );
    rSettingsStr += "giga:resolutionDPI rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:resolutionDPI rdfs:label \"Resolution DPI\"@en .\n";
    rSettingsStr += uri+" giga:resolutionDPI \""+QString( "%1" ).arg( resolutionDPI )+"\"^^xsd:double .\n"; 
    rSettingsStr += "giga:horizontalShift rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:horizontalShift rdfs:label \"Horizontal Shift\"@en .\n";
    rSettingsStr += uri+" giga:horizontalShift \""+QString( "%1" ).arg( shiftHori )+"\"^^xsd:double .\n"; 
    rSettingsStr += "giga:verticalShift rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:verticalShift rdfs:label \"Vertical Shift\"@en .\n";
    rSettingsStr += uri+" giga:verticalShift \""+QString( "%1" ).arg( shiftHori )+"\"^^xsd:double .\n"; 
    rSettingsStr += "giga:orthoZoom rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:orthoZoom rdfs:label \"Ortho Zoom\"@en .\n";
    rSettingsStr += uri+" giga:orthoZoom \""+QString( "%1" ).arg( orthoZoom )+"\"^^xsd:double .\n"; 
    rSettingsStr += "giga:bboxRadius rdf:type owl:DatatypeProperty .\n";
    rSettingsStr += "giga:bboxRadius rdfs:label \"Bounding Box Radius\"@en .\n";
    rSettingsStr += uri+" giga:bboxRadius \""+QString( "%1" ).arg( bBoxRadius )+"\"^^xsd:double .\n"; 
	return( true );
}


//! Show the 2D bounding box of the projected mesh.
//!
//! @returns false in case of an error. True otherwise.
bool MeshWidget::showView2DBoundingBox() {
	// Sanity check
	if( mMeshVisual == nullptr ) {
		return( false );
	}

	// Fetch 2D bounding box
	double minX, maxX, minY, maxY, minZ, maxZ;
	// Matrix4D matView( ( mMatProjection * mMatModelView ).constData() );
	Matrix4D matView( ( mMatModelView ).constData() );
	if( !mMeshVisual->getBoundingBoxProjected( matView, minX, maxX, minY, maxY, minZ, maxZ ) ) {
		return( false );
	}

	SHOW_MSGBOX_INFO( tr( "Bounding Box of the projection" ),
	                  tr( "%1 %2<br />" ).arg( minX ).arg( maxX ) +
	                  tr( "%1 %2<br />" ).arg( minY ).arg( maxY ) +
	                  tr( "%1 %2<br />" ).arg( minZ ).arg( maxZ )
	                );

	return( true );
}


//! Show view matrix and copy it to the clipboard.
//! 
//! @returns false in case of an error. True otherwise.
bool MeshWidget::showViewMatrix() {
	QString viewMatrixClip;
	getViewSettingsTxt( viewMatrixClip );

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText( viewMatrixClip );

	//! \todo use "GL_MODELVIEW_MATRIX" instead of mCenterView/mCameraCenter/mCameraUp
	QString viewMatrixInfo = "<table><tr>";
	viewMatrixInfo += QString( "<td>") + tr("View&nbsp;center:") + QString("</td>" );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCenterView.getX() );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCenterView.getY() );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCenterView.getZ() );
	viewMatrixInfo += "</tr><tr>";
	viewMatrixInfo += QString( "<td>") + tr("Camera&nbsp;center:") + QString("</td>" );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCameraCenter.getX() );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCameraCenter.getY() );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCameraCenter.getZ() );
	viewMatrixInfo += "</tr><tr>";
	viewMatrixInfo += QString( "<td>") + tr("Camera&nbsp;UP:") + QString("</td>" );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCameraUp.getX() );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCameraUp.getY() );
	viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mCameraUp.getZ() );
	if( mParamFlag[ORTHO_MODE] ) {
		double paramFloat;
		getViewPortDPI( paramFloat );
		viewMatrixInfo += "</tr><tr>";
		viewMatrixInfo += QString( "<td>")+ tr("Shift&nbsp;Hori/Vert:") + QString("</td>" );
		viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mParamFlt[ORTHO_SHIFT_HORI] );
		viewMatrixInfo += QString("<td>&nbsp;%1</td>").arg( mParamFlt[ORTHO_SHIFT_VERT] );
		viewMatrixInfo += QString("<td>&nbsp;%1&nbsp;DPI</td>").arg( paramFloat );
	}
	viewMatrixInfo += "</tr></table>\n"
	                  "<br /><br />\n";

	SHOW_MSGBOX_INFO( tr("View Matrix"), viewMatrixInfo + tr("Already copied to clipboard!") );
	return( true );
}

//! Dialog to set the viewport/camera.
//! 
//! @returns false in case of an error or user cancel. True otherwise.
bool MeshWidget::setViewMatrix() {
	// Fetch clipboard
	QClipboard *clipboard = QApplication::clipboard();
	QString clipBoardStr = clipboard->text( QClipboard::Clipboard );
	// Fetch first line only and therefore ignore any additional data
	QTextStream tempStream( &clipBoardStr );
	QString clipBoardFristLineOnly = tempStream.readLine();

	// Setup dialog:
	QGMDialogEnterText dlgEnterTxt;
	dlgEnterTxt.setWindowTitle( tr( "Enter view parameters (9 to 13)" ) );
	dlgEnterTxt.setText( clipBoardFristLineOnly );
	if( dlgEnterTxt.exec() == QDialog::Rejected ) {
		std::cout << "[MeshWidget::" << __FUNCTION__ << "] WARNING: User cancel!" << std::endl;
		return( false );
	}

	// User interaction:
	vector<double> camMatrix;
	if( !dlgEnterTxt.getText( camMatrix ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Wrong user input!" << std::endl;
		return( false );
	}

	// Finally set the view:
	if( !setViewMatrix( camMatrix ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: setViewMatrix failed!" << std::endl;
		return( false );
	}

	// Done.
	return( true );
}

//! Set the viewport/camera using
//! 9 camera parameters for the camera setup and
//! additional parameters for the orhtographic or prespective projection.
//! 
//! @returns false in case of an error or user cancel. True otherwise.
bool MeshWidget::setViewMatrix( vector<double> rMatrix ) {
	if( ( rMatrix.size() < 9 ) && ( rMatrix.size() > 13 ) ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Wrong number of values: " 
		          << rMatrix.size() << " - expected (9 to 13)!" << std::endl;
		return( false );
	}
	mCenterView   = Vector3D( rMatrix.at( 0 ), rMatrix.at( 1 ), rMatrix.at( 2 ), 1.0 );
	mCameraCenter = Vector3D( rMatrix.at( 3 ), rMatrix.at( 4 ), rMatrix.at( 5 ), 1.0 );
	mCameraUp     = Vector3D( rMatrix.at( 6 ), rMatrix.at( 7 ), rMatrix.at( 8 ), 1.0 );
	if( rMatrix.size() > 9 ) {
		bool orthoMode = rMatrix.at( 9 );
		setParamFlagMeshWidget( ORTHO_MODE, orthoMode );
		std::cout << "[MeshWidget::" << __FUNCTION__ << "] Ortho mode set." << std::endl;
	}
	if( rMatrix.size() == 13 ) {
		double paramFloat = rMatrix.at( 10 );
		setParamFloatMeshWidget( ORTHO_SHIFT_HORI, paramFloat );
		paramFloat = rMatrix.at( 11 );
		setParamFloatMeshWidget( ORTHO_SHIFT_VERT, paramFloat );
		paramFloat = rMatrix.at( 12 );
		orthoSetDPI( paramFloat );
		std::cout << "[MeshWidget::" << __FUNCTION__ << "] Ortho parameters set." << std::endl;
	}
	setView();
	update();

	// Done.
	return( true );
}

//! Use the axis as up vector of the OpenGL camera setup.
//! Algins the view as direction of the look-at position.
//! @returns false in case of an error. True otherwise.
bool MeshWidget::setViewAxisUp() {
	if( mMeshVisual == nullptr ) {
		return false;
	}

	// Fetch axis, when present:
	Vector3D axisTop;
	Vector3D axisBottom;
	if( !mMeshVisual->getConeAxis( &axisTop, &axisBottom ) ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] No axis defined." << endl;
		return false;
	}

	// The axis direction is the new Camera Up direction
	mCameraUp = axisTop - axisBottom;
	mCameraUp.normalize3();
	// cout << "[MeshWidget::" << __FUNCTION__ << "] Axis / Cam Up:  " << mCameraUp << endl;
	// cout << "[MeshWidget::" << __FUNCTION__ << "] Look At (LA):   " << mCenterView << endl;

	// This will be the new CameraCenter
	double camCenterDist = ( mCameraCenter - mCenterView ).getLength3();
	// cout << "[MeshWidget::" << __FUNCTION__ << "] Distance between Camera and ViewCenter: " << camCenterDist << endl;
	Vector3D lookAtProjectedToAxis = mCenterView.projectOnto( mCameraUp );
	lookAtProjectedToAxis.setH( 1.0 );
	// cout << "[MeshWidget::" << __FUNCTION__ << "] Projection LA':  " << lookAtProjectedToAxis << endl;
	Vector3D directionLookAtToAxis = lookAtProjectedToAxis - mCenterView;
	directionLookAtToAxis.normalize3();
	// cout << "[MeshWidget::" << __FUNCTION__ << "] Viewdirection LA'-LA:  " << directionLookAtToAxis << endl;
	mCameraCenter = mCenterView + directionLookAtToAxis * camCenterDist;
	// cout << "[MeshWidget::" << __FUNCTION__ << "] New Camera Center:  " << mCameraCenter << endl;

	// Execute:
	setView();
	update();

	return true;
}

//! Show dialog to enter a screenresolution in DPI for the orthographic projection (only) and
//! set the windowviewport to a specific resolution in DPI (assuming mm as unit).
bool MeshWidget::orthoSetDPI() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	bool orthoMode = false;
	getParamFlagMeshWidget( ORTHO_MODE, &orthoMode );
	if( !orthoMode ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Not in ortho mode." << endl;
		return( false );
	}

	double dotsPerInch;
	double pixelWidth;
	double pixelHeight;
	if( !getViewPortPixelWorldSize( pixelWidth, pixelHeight ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getViewPortPixelWorldSize failed!" << endl;
		return( false );
	}
	dotsPerInch = ( 25.4/pixelWidth + 25.4/pixelHeight ) / 2.0; // "pseudo-average"

	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Set Dots Per Inch (DPI)") );
	dlgEnterText.setDouble( dotsPerInch );

	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return( false );
	}
	if( !dlgEnterText.getText( &dotsPerInch ) ) {
		return( false );
	}

	return orthoSetDPI( dotsPerInch );
}

//! Set the windowviewport to a specific resolution in DPI (assuming mm as unit).
bool MeshWidget::orthoSetDPI( double rSetTo ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Sanity check
	if( mMeshVisual == nullptr ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No mesh visualized (nullptr)!" << endl;
		return( false );
	}

	// Check mode
	bool orthoMode = false;
	getParamFlagMeshWidget( ORTHO_MODE, &orthoMode );
	if( !orthoMode ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Not in ortho mode." << endl;
		return( false );
	}

	rSetTo /= 25.4;
	double bBoxRadius = mMeshVisual->getBoundingBoxRadius();
	// As we use the bounding box radius as reference and set the width in setView, we have to change the ORTHO_ZOOM to:
	double orthoZoom  = (static_cast<double>(width()))/(2.0*bBoxRadius*rSetTo);

	return setParamFloatMeshWidget( ORTHO_ZOOM, orthoZoom );
}

//! Saves a SVG and a PNG with transparency to the given filename.
//! Part 1 of 2: ask for filename
bool MeshWidget::screenshotSVG() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// 0.) Sanity check
	if( mMeshVisual == nullptr ) {
		SHOW_MSGBOX_CRIT( tr("ERROR"), tr("No mesh present.") );
		return false;
	}

	//! 1.) Ask for the filename
    QString filePath = QString::fromStdWString( mMeshVisual->getFileLocation().wstring());
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Save screenshot as" ),
	                                                 filePath +
                                                     QString::fromStdWString( mMeshVisual->getBaseName().wstring() ) +
	                                                 ".svg",
													 tr( "Scaleable Vector Graphic (*.svg)" ) );
	if( fileName == nullptr ) { // Cancel pressed
		SHOW_MSGBOX_WARN( tr("User abort"), tr("No files saved.") );
		return false;
	}

	// Optional:
	QString fileNamePNG = fileName.left( fileName.lastIndexOf( "." ) ) + ".png";
	fileNamePNG = QFileDialog::getSaveFileName( mMainWindow, tr( "Save screenshot as" ), \
												fileNamePNG, tr( "Portable Network Graphics (*.png)" ) );

	bool retVal = screenshotSVG( fileName, fileNamePNG );
	if( retVal ) {
		if( fileNamePNG.length() > 0 ) {
			SHOW_MSGBOX_INFO( tr("Files saved"), tr("PNG and SVG") );
		} else {
			SHOW_MSGBOX_INFO( tr("File saved"), tr("SVG only") );
		}
	} else {
		SHOW_MSGBOX_CRIT( tr("ERROR"), tr("Could not save file(s).") );
	}
	return retVal;
}

//! Saves a SVG and a PNG with transparency to the given filename.
//! Part 2 of 2: execute
//!
//! @return false in case of an error. True otherwise.
bool MeshWidget::screenshotSVG( const QString& rFileName, const QString& rFileNamePNG ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//cout << "[MeshWidget::" << __FUNCTION__ << "] " << fileName.toStdString() << endl;

	// Pixel neighbourhood (8):
	enum ePixelNeighbour{
		PIXEL_CENTER,
		PIXEL_TOPLEFT,
		PIXEL_TOP,
		PIXEL_TOPRIGHT,
		PIXEL_RIGHT,
		PIXEL_BOTRIGHT,
		PIXEL_BOT,
		PIXEL_BOTLEFT,
		PIXEL_LEFT
	};

	uint64_t imWidth;
	uint64_t imHeight;
	unsigned char* imRGBA;
	{
		OffscreenBuffer offscreenBuffer(context());

		int defaultBuffer = 0;
		mMeshVisual->getParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, &defaultBuffer);
		mMeshVisual->setParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, offscreenBuffer.getFboID());

		//paint scene into offscreenBuffer
		repaint();


		if( !fetchFrameAndZBuffer( imRGBA, imWidth, imHeight, false, &offscreenBuffer ) ) { // mParamFlag[CROP_SCREENSHOTS]
			return( false );
		}

		bindFramebuffer(defaultBuffer);
		mMeshVisual->setParamIntMeshGL(MeshGLParams::DEFAULT_FRAMEBUFFER_ID, defaultBuffer);
		repaint();
	}

	SvgWriter svgWriter;
	auto svgImage = std::make_unique<SvgImage>();
	svgImage->setImage(imRGBA, imWidth, imHeight, 4, 0,0);

	// estimate silhouette from alpha channel - dx
	unsigned char* silhArr = new unsigned char[imWidth*imHeight];
	// init with zeros:
	for( size_t i=0; i<imWidth*imHeight; i++ ) {
		silhArr[i] = 0;
	}
	// everything expcet the borders - inspired by Matlab's pwperim
	for( size_t ix=1; ix<imWidth-1; ix++ ) {
		for( size_t iy=1; iy<imHeight-1; iy++ ) {
			if( imRGBA[(ix+iy*imWidth)*4+3] < 255 ) {
				// when pixel is OFF:
				continue;
			}
			// when one of the four neighbours is off, than we are on the perimeter and set the pixel ON
			if( imRGBA[(ix-1+iy*imWidth)*4+3] < 255 ) {
				silhArr[ix+iy*imWidth] = 255;
				continue;
			}
			if( imRGBA[(ix+1+iy*imWidth)*4+3] < 255 ) {
				silhArr[ix+iy*imWidth] = 255;
				continue;
			}
			if( imRGBA[(ix+(iy+1)*imWidth)*4+3] < 255 ) {
				silhArr[ix+iy*imWidth] = 255;
				continue;
			}
			if( imRGBA[(ix+(iy-1)*imWidth)*4+3] < 255 ) {
				silhArr[ix+iy*imWidth] = 255;
				continue;
			}
		}
	}
	// horizontal border treatment:
	for( size_t ix=0; ix<imWidth; ix++ ) {
		int iy = 0;
		if( imRGBA[(ix+iy*imWidth)*4+3] == 255 ) {
			// when pixel is ON - we set ON
			silhArr[ix+iy*imWidth] = 255;
		}
		iy = imHeight-1;
		if( imRGBA[(ix+iy*imWidth)*4+3] == 255 ) {
			// when pixel is ON - we set ON
			silhArr[ix+iy*imWidth] = 255;
		}
	}
	// vertical border treatment:
	for( size_t iy=0; iy<imHeight; iy++ ) {
		int ix = 0;
		if( imRGBA[(ix+iy*imWidth)*4+3] == 255 ) {
			// when pixel is ON - we set ON
			silhArr[ix+iy*imWidth] = 255;
		}
		ix = imWidth-1;
		if( imRGBA[(ix+iy*imWidth)*4+3] == 255 ) {
			// when pixel is ON - we set ON
			silhArr[ix+iy*imWidth] = 255;
		}
	}

	auto svgSillouettePath = std::make_unique<SvgPath>();

	double realWidth; // <- used for orthomode only.
	double realHeight;
	if( mParamFlag[ORTHO_MODE] ) {
		getViewPortResolution( realWidth, realHeight );
		realWidth  *= mParamFlt[SVG_SCALE];
		realHeight *= mParamFlt[SVG_SCALE];
		//cout << "[MeshWidget::screenshotSVG] Ortho Image Size: " << realWidth << " x " << realHeight << " mm" << endl;
		svgImage->setScale(realWidth/imWidth, realHeight / imHeight);
		svgSillouettePath->setScale(realWidth/imWidth, realHeight / imHeight);
		svgWriter.setSize(realWidth, realHeight);
	} else {
		svgWriter.setSize(imWidth, imHeight);
	}

	svgWriter.addElement(std::move(svgImage));

	// draw sillhoutte
	bool pointsLeft = true;
	while( pointsLeft ) {
		pointsLeft = false;
		int64_t sx = 0;
		int64_t sy = 0;
		// find a first point
		for( uint64_t ix=0; ix<imWidth; ix++ ) {
			for( uint64_t iy=0; iy<imHeight; iy++ ) {
				if( silhArr[ix+iy*imWidth] != 0 ) {
					sx = ix;
					sy = iy;
					// break:
					ix = imWidth;
					iy = imHeight;
					pointsLeft = true;
				}
			}
		}
		if( !pointsLeft ) {
			break;
		}
		//cout << "[MeshWidget::screenshotSVG] Drawing line." << endl;
		svgSillouettePath->setColor(0.0, 0.0, 0.0);
		svgSillouettePath->moveTo(sx, sy);
		// trace the line
		int  pixelNeighLast = PIXEL_CENTER;
		int  pixelNeighCurr = PIXEL_CENTER;
		while( sx >= 0 ) {
			//cout << "sx: " << sx << " sy: " << sy << endl;
			bool drawEnd = true;
			if( pixelNeighLast != pixelNeighCurr ) {
				svgSillouettePath->lineTo(sx, sy);
				pixelNeighLast = pixelNeighCurr;
				drawEnd = false;
			}
			silhArr[sx+sy*imWidth] = 0;
			if( ( (sx+1) < imWidth ) && ( silhArr[(sx+1)+(sy)*imWidth] != 0 ) ) {
				sx++;
				pixelNeighCurr = PIXEL_RIGHT;
			} else if( ( (sx+1) < imWidth ) && ( sy > 0 ) && ( silhArr[(sx+1)+(sy-1)*imWidth] != 0 ) ) {
				sx++;
				sy--;
				pixelNeighCurr = PIXEL_BOTRIGHT;
			} else if( ( sy > 0 ) && ( silhArr[(sx)+(sy-1)*imWidth] != 0 ) ) {
				sy--;
				pixelNeighCurr = PIXEL_BOT;
			} else if( ( sx > 0 ) && ( sy > 0 ) && ( silhArr[(sx-1)+(sy-1)*imWidth] != 0 ) ) {
				sx--;
				sy--;
				pixelNeighCurr = PIXEL_BOTLEFT;
			} else if( ( sx > 0 ) && ( silhArr[(sx-1)+(sy)*imWidth] != 0 ) ) {
				sx--;
				pixelNeighCurr = PIXEL_LEFT;
			} else if( ( sx > 0 ) && ( (sy+1) < imHeight ) && ( silhArr[(sx-1)+(sy+1)*imWidth] != 0 ) ) {
				sx--;
				sy++;
				pixelNeighCurr = PIXEL_TOPLEFT;
			} else if( ( (sy+1) < imHeight ) && ( silhArr[(sx)+(sy+1)*imWidth] != 0 ) ) {
				sy++;
				pixelNeighCurr = PIXEL_TOP;
			} else if( ( (sx+1) < imWidth ) && ( (sy+1) < imHeight ) && ( silhArr[(sx+1)+(sy+1)*imWidth] != 0 ) ) {
				sx++;
				sy++;
				pixelNeighCurr = PIXEL_TOPRIGHT;
			} else {
				if( drawEnd ) {
					svgSillouettePath->lineTo(sx, sy);
				}
				sx = -1;
			}
		}
	}

	svgSillouettePath->setLineWidth(2.0);
	svgWriter.addElement(std::move(svgSillouettePath));

	// matrices for projected polylines: GL_MODELVIEW_MATRIX x GL_PROJECTION_MATRIX
	// ... works the same for orthographic and perspective projections
	// see http://www.songho.ca/opengl/gl_projectionmatrix.html
	// and http://www.songho.ca/opengl/gl_transform.html
	//double matrixGL[16];
	//glGetDoublev( GL_MODELVIEW_MATRIX, matrixGL );
	Vector3D cameraViewDir( -mMatModelView(2,0), -mMatModelView(2,1), -mMatModelView(2,2), 0.0 ); //( matrixGL[2], matrixGL[6], matrixGL[10], 0.0 );
	//Matrix4D matView( matrixGL );
	//glGetDoublev( GL_PROJECTION_MATRIX, matrixGL );
	//matView *= Matrix4D( matrixGL );
	Matrix4D matView( ( mMatProjection * mMatModelView ).constData() );
	//matView.dumpInfo( true, "matView" );

	int xywh[4];
	glGetIntegerv( GL_VIEWPORT, xywh );
	//cout << "[MeshWidget::" << __FUNCTION__ << "] " << xywh[0] << " " << xywh[1] << " " << xywh[2] << " " << xywh[3] << endl;

	// Export polylines
	double polyLineWidth   = 2.0;     // in pixel.
	double polyScaleWdith  = xywh[2]; // in pixel.
	double polyScaleHeight = xywh[3]; // in pixel.
	if( mParamFlag[ORTHO_MODE] ) {
		polyLineWidth  *= realWidth/imWidth; // in real world units.
		polyScaleWdith  = realWidth;         // in real world units.
		polyScaleHeight = realHeight;        // in real world units.
	}
	if( !screenshotSVGexportPolyLines( cameraViewDir, matView, polyScaleWdith, polyScaleHeight, polyLineWidth , svgWriter) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: screenshotSVGexportPolyLines falied!" << endl;
	}

	// Export bounding box
	Vector3D bBoxA = mMeshVisual->getBoundingBoxA() * matView; bBoxA.normalizeW(); bBoxA += Vector3D( 1.0, 1.0, 1.0, 0.0 );
	Vector3D bBoxB = mMeshVisual->getBoundingBoxB() * matView; bBoxB.normalizeW(); bBoxB += Vector3D( 1.0, 1.0, 1.0, 0.0 );
	Vector3D bBoxC = mMeshVisual->getBoundingBoxC() * matView; bBoxC.normalizeW(); bBoxC += Vector3D( 1.0, 1.0, 1.0, 0.0 );
	Vector3D bBoxD = mMeshVisual->getBoundingBoxD() * matView; bBoxD.normalizeW(); bBoxD += Vector3D( 1.0, 1.0, 1.0, 0.0 );
	Vector3D bBoxE = mMeshVisual->getBoundingBoxE() * matView; bBoxE.normalizeW(); bBoxE += Vector3D( 1.0, 1.0, 1.0, 0.0 );
	Vector3D bBoxF = mMeshVisual->getBoundingBoxF() * matView; bBoxF.normalizeW(); bBoxF += Vector3D( 1.0, 1.0, 1.0, 0.0 );
	Vector3D bBoxG = mMeshVisual->getBoundingBoxG() * matView; bBoxG.normalizeW(); bBoxG += Vector3D( 1.0, 1.0, 1.0, 0.0 );
	Vector3D bBoxH = mMeshVisual->getBoundingBoxH() * matView; bBoxH.normalizeW(); bBoxH += Vector3D( 1.0, 1.0, 1.0, 0.0 );
	double bBoxAScreenX, bBoxAScreenY;
	double bBoxBScreenX, bBoxBScreenY;
	double bBoxCScreenX, bBoxCScreenY;
	double bBoxDScreenX, bBoxDScreenY;
	double bBoxEScreenX, bBoxEScreenY;
	double bBoxFScreenX, bBoxFScreenY;
	double bBoxGScreenX, bBoxGScreenY;
	double bBoxHScreenX, bBoxHScreenY;
	if( mParamFlag[ORTHO_MODE] ) {
		bBoxAScreenX = (bBoxA.getX()/2.0)     * realWidth;
		bBoxAScreenY = (1.0-bBoxA.getY()/2.0) * realHeight;
		bBoxBScreenX = (bBoxB.getX()/2.0)     * realWidth;
		bBoxBScreenY = (1.0-bBoxB.getY()/2.0) * realHeight;
		bBoxCScreenX = (bBoxC.getX()/2.0)     * realWidth;
		bBoxCScreenY = (1.0-bBoxC.getY()/2.0) * realHeight;
		bBoxDScreenX = (bBoxD.getX()/2.0)     * realWidth;
		bBoxDScreenY = (1.0-bBoxD.getY()/2.0) * realHeight;
		bBoxEScreenX = (bBoxE.getX()/2.0)     * realWidth;
		bBoxEScreenY = (1.0-bBoxE.getY()/2.0) * realHeight;
		bBoxFScreenX = (bBoxF.getX()/2.0)     * realWidth;
		bBoxFScreenY = (1.0-bBoxF.getY()/2.0) * realHeight;
		bBoxGScreenX = (bBoxG.getX()/2.0)     * realWidth;
		bBoxGScreenY = (1.0-bBoxG.getY()/2.0) * realHeight;
		bBoxHScreenX = (bBoxH.getX()/2.0)     * realWidth;
		bBoxHScreenY = (1.0-bBoxH.getY()/2.0) * realHeight;
	} else {
		bBoxAScreenX = (bBoxA.getX()/2.0)     * xywh[2];
		bBoxAScreenY = (1.0-bBoxA.getY()/2.0) * xywh[3];
		bBoxBScreenX = (bBoxB.getX()/2.0)     * xywh[2];
		bBoxBScreenY = (1.0-bBoxB.getY()/2.0) * xywh[3];
		bBoxCScreenX = (bBoxC.getX()/2.0)     * xywh[2];
		bBoxCScreenY = (1.0-bBoxC.getY()/2.0) * xywh[3];
		bBoxDScreenX = (bBoxD.getX()/2.0)     * xywh[2];
		bBoxDScreenY = (1.0-bBoxD.getY()/2.0) * xywh[3];
		bBoxEScreenX = (bBoxE.getX()/2.0)     * xywh[2];
		bBoxEScreenY = (1.0-bBoxE.getY()/2.0) * xywh[3];
		bBoxFScreenX = (bBoxF.getX()/2.0)     * xywh[2];
		bBoxFScreenY = (1.0-bBoxF.getY()/2.0) * xywh[3];
		bBoxGScreenX = (bBoxG.getX()/2.0)     * xywh[2];
		bBoxGScreenY = (1.0-bBoxG.getY()/2.0) * xywh[3];
		bBoxHScreenX = (bBoxH.getX()/2.0)     * xywh[2];
		bBoxHScreenY = (1.0-bBoxH.getY()/2.0) * xywh[3];
	}
	// Draw using the projected coordinates

	auto svgBoundingBoxPath = std::make_unique<SvgPath>();
	svgBoundingBoxPath->setColor(253.0 / 255.0, 174.0/255.0 , 97.0/255.0);

	// A-B-C-D
	svgBoundingBoxPath->moveTo( bBoxAScreenX, bBoxAScreenY);
	svgBoundingBoxPath->lineTo( bBoxBScreenX, bBoxBScreenY );
	svgBoundingBoxPath->lineTo( bBoxCScreenX, bBoxCScreenY );
	svgBoundingBoxPath->lineTo( bBoxDScreenX, bBoxDScreenY );
	svgBoundingBoxPath->lineTo( bBoxAScreenX, bBoxAScreenY );

	// E-F-G-H
	svgBoundingBoxPath->moveTo(bBoxEScreenX, bBoxEScreenY );
	svgBoundingBoxPath->lineTo( bBoxFScreenX, bBoxFScreenY );
	svgBoundingBoxPath->lineTo( bBoxGScreenX, bBoxGScreenY );
	svgBoundingBoxPath->lineTo( bBoxHScreenX, bBoxHScreenY );
	svgBoundingBoxPath->lineTo( bBoxEScreenX, bBoxEScreenY );

	// A-E, B-F, C-G and D-H
	svgBoundingBoxPath->moveTo( bBoxAScreenX, bBoxAScreenY );
	svgBoundingBoxPath->lineTo( bBoxEScreenX, bBoxEScreenY );
	svgBoundingBoxPath->moveTo( bBoxBScreenX, bBoxBScreenY );
	svgBoundingBoxPath->lineTo( bBoxFScreenX, bBoxFScreenY );
	svgBoundingBoxPath->moveTo( bBoxCScreenX, bBoxCScreenY );
	svgBoundingBoxPath->lineTo( bBoxGScreenX, bBoxGScreenY );
	svgBoundingBoxPath->moveTo( bBoxDScreenX, bBoxDScreenY );
	svgBoundingBoxPath->lineTo( bBoxHScreenX, bBoxHScreenY );

	if( mParamFlag[ORTHO_MODE] ) {
		svgBoundingBoxPath->setLineWidth(2.0 * realWidth / imWidth);
	} else {
		svgBoundingBoxPath->setLineWidth(2.0);
	}
	svgWriter.addElement(std::move(svgBoundingBoxPath));

	// Export selected vertices
	set<Vertex*> vertSel;
	if( !mMeshVisual->getSelectedVerts( &vertSel ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: meshVisual.getSelectedVerts failed!" << endl;
	}
	set<Vertex*>::iterator itVertex;
	// Determine function value range to normalize
	double funcValMin = +DBL_MAX;
	double funcValMax = -DBL_MAX;
	double funcValMinMIN = +DBL_MAX;
	double funcValMaxMIN = -DBL_MAX;
	double funcValMinMAX = +DBL_MAX;
	double funcValMaxMAX = -DBL_MAX;
	for( itVertex=vertSel.begin(); itVertex!=vertSel.end(); itVertex++ ) {
		Vertex* currVert = (*itVertex);
		double funcVal;
		if( !currVert->getFuncValue( &funcVal ) ) {
			cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: vertex.getFuncValue failed!" << endl;
			continue;
		}
		if( funcValMin > funcVal ) {
			funcValMin = funcVal;
		}
		if( funcValMax < funcVal ) {
			funcValMax = funcVal;
		}
		if( currVert->getFlag( Primitive::FLAG_LOCAL_MIN ) ) {
			if( funcValMinMIN > funcVal ) {
				funcValMinMIN = funcVal;
			}
			if( funcValMaxMIN < funcVal ) {
				funcValMaxMIN = funcVal;
			}
		}
		if( currVert->getFlag( Primitive::FLAG_LOCAL_MAX ) ) {
			if( funcValMinMAX > funcVal ) {
				funcValMinMAX = funcVal;
			}
			if( funcValMaxMAX < funcVal ) {
				funcValMaxMAX = funcVal;
			}
		}
	}
	double funcValRange = funcValMax - funcValMin;
	double funcValRangeMIN = funcValMaxMIN - funcValMinMIN;
	double funcValRangeMAX = funcValMaxMAX - funcValMinMAX;
	for( itVertex=vertSel.begin(); itVertex!=vertSel.end(); itVertex++ ) {
		Vertex* currVert = (*itVertex);
		float angleToCam = currVert->angleToNormal( cameraViewDir );
		// Skip vertices with normals pointing away from the camera.
		// This will provide a basic filter for selected vertices not visible to the viewer, which will do the trick for most objects.
		if( ( mParamFlag[ORTHO_MODE] ) && ( angleToCam > M_PI/2.0 ) ) {
			continue;
		}
		if( ( !mParamFlag[ORTHO_MODE] ) && ( angleToCam > ( M_PI/2.0 - mParamFlt[FOV_ANGLE]*M_PI/360.0 ) ) ) { // 360=2*180 because we need fovAngle/2 in radiant
			continue;
		}
		// Project coordinates
		Vector3D coordProjected = Vector3D( currVert->getPositionVector() ) * matView;
		coordProjected.normalizeW();
		coordProjected += Vector3D( 1.0, 1.0, 1.0, 0.0 );
		//coordProjected.dumpInfo();
		//cout << "[MeshWidget::" << __FUNCTION__ << "] " << coordProjected.getX()*xywh[2] << " " << coordProjected.getY()*xywh[3] << endl;
		double screenCoordX;
		double screenCoordY;
		if( mParamFlag[ORTHO_MODE] ) {
			screenCoordX = (coordProjected.getX()/2.0)     * realWidth;
			screenCoordY = (1.0-coordProjected.getY()/2.0) * realHeight;
		} else {
			screenCoordX = (coordProjected.getX()/2.0)     * xywh[2];
			screenCoordY = (1.0-coordProjected.getY()/2.0) * xywh[3];
		}
		double funcVal;
		if( !currVert->getFuncValue( &funcVal ) ) {
			cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: vertex.getFuncValue failed!" << endl;
			continue;
		}

		// Draw using the projected coordinates
		double scaleRadius = 2.0 * ( funcVal - funcValMin ) / funcValRange + 1.0;

		auto svgCircle = std::make_unique<SvgCircle>(screenCoordX, screenCoordY, 0.1 * scaleRadius);
		svgCircle->setFilled(true);
		svgCircle->setStroke(false);

		if( currVert->getFlag( Primitive::FLAG_LOCAL_MIN ) ) {
			scaleRadius = 2.0 * ( funcVal - funcValMinMIN ) / funcValRangeMIN + 1.0;

			svgCircle->setFillColor(215.0/255.0, 25.0/255.0, 28.0/255.0);
			svgCircle->setRadius(scaleRadius * 0.1);
		}
		if( currVert->getFlag( Primitive::FLAG_LOCAL_MAX ) ) {
			scaleRadius = 2.0 * ( funcVal - funcValMinMAX ) / funcValRangeMAX + 1.0;

			svgCircle->setFillColor(26.0/255.0, 150.0/255.0, 65.0/255.0);
			svgCircle->setRadius(scaleRadius * 0.1);
		}

		svgWriter.addElement(std::move(svgCircle));
	}

	// Block RULER --- START -------------------------------------------------------------------------------------------------------------------------------
	if( mParamFlag[ORTHO_MODE] ) {
		// Adapt RULER parameters to SVG scaling
		double rulerHeight    = mParamFlt[RULER_HEIGHT]     * mParamFlt[SVG_SCALE];
		double rulerWdith     = mParamFlt[RULER_WIDTH]      * mParamFlt[SVG_SCALE];
		double rulerUnit      = mParamFlt[RULER_UNIT]       * mParamFlt[SVG_SCALE];
		double rulerUnitTicks = mParamFlt[RULER_UNIT_TICKS] * mParamFlt[SVG_SCALE];

		// Group the ruler elements
		svgWriter.startGroup();

		auto svgRulerTicks = std::make_unique<SvgPath>();
		auto svgRulerText = std::make_unique<SvgText>();
		auto svgRulerFrame = std::make_unique<SvgRect>(0.0, 0.0, rulerWdith, rulerHeight);



		// frame
		svgRulerFrame->setStrokeWidth(0.05*rulerHeight);
		//svgRulerFrame->setLineWidth
		svgWriter.addElement(std::move(svgRulerFrame));
		// blocks (units)
		for( size_t i=0; i<=rulerWdith/rulerUnit-1; i++ ) {
			auto svgRulerBlock = std::make_unique<SvgRect>(i*rulerUnit, (rulerHeight/2.0)*!((i+2)%2), rulerUnit, rulerHeight/2.0);
			svgRulerBlock->setFill(true);
			svgRulerBlock->setStrokeWidth(0.05*rulerHeight);
			svgWriter.addElement(std::move(svgRulerBlock));
		}
		int max_i=static_cast<int>( rulerWdith/rulerUnit );

		svgRulerFrame = std::make_unique<SvgRect>(max_i*rulerUnit, (rulerHeight/2.0)*!((max_i+2)%2), rulerWdith-max_i*rulerUnit, rulerHeight/2.0);
		svgRulerFrame->setFill(true);
		svgRulerFrame->setStrokeWidth(0.05*rulerHeight);
		svgWriter.addElement(std::move(svgRulerFrame));

		//ticks
		svgRulerTicks->setLineWidth(0.05*rulerHeight);
		if( rulerUnitTicks ) {
			for( size_t i=1; i<rulerWdith/rulerUnitTicks; i++ ) {
				svgRulerTicks->moveTo(i*rulerUnitTicks, 0);
				svgRulerTicks->lineTo( i*rulerUnitTicks, rulerHeight);
			}
			svgWriter.addElement(std::move(svgRulerTicks));
		}

		// WIDTH_UNIT label:
		string rulerWidthUnit;
		getParamStringMeshWidget( RULER_WIDTH_UNIT, &rulerWidthUnit );
		svgRulerText->setSize( rulerHeight * 0.7);
		svgRulerText->setFont("Georgia");
		svgRulerText->setPosition(rulerWdith + 1, rulerHeight * 0.75);
		char  str[255];
		float widthlabel=static_cast<int>( rulerWdith * 100.0 )/( 100.0 * mParamFlt[SVG_SCALE] );
		sprintf( str, "%.2f %s", widthlabel, rulerWidthUnit.c_str() );

		svgRulerText->setText(str);
		svgWriter.addElement(std::move(svgRulerText));

		// Ticks RULER:
		// baseline and ends
		svgRulerTicks = std::make_unique<SvgPath>();
		svgRulerTicks->setLineWidth(0.05*rulerHeight);

		svgRulerTicks->moveTo(0, 3*rulerHeight );
		svgRulerTicks->lineTo( rulerWdith, 3*rulerHeight );
		svgRulerTicks->moveTo( 0, 2.5*rulerHeight );
		svgRulerTicks->lineTo( 0, 3*rulerHeight );
		svgRulerTicks->moveTo( rulerWdith, 2.5*rulerHeight );
		svgRulerTicks->lineTo( rulerWdith, 3*rulerHeight );

		// small Ticks
		if( rulerUnitTicks ) {
			for( size_t i=1; i<rulerWdith/rulerUnitTicks; i++ ) {
				svgRulerTicks->moveTo( i*rulerUnitTicks, 2.75*rulerHeight );
				svgRulerTicks->lineTo( i*rulerUnitTicks, 3*rulerHeight );
			}
		}

		// Ticks
		if( rulerUnit ) {
			for( size_t i=1; i<rulerWdith/rulerUnit; i++ ) {
				svgRulerTicks->moveTo( i*rulerUnit, 2.5*rulerHeight );
				svgRulerTicks->lineTo( i*rulerUnit, 3*rulerHeight );
			}
		}

		svgRulerTicks->setLineCap(SvgPath::LineCap::CAP_SQUARE);
		svgWriter.addElement(std::move(svgRulerTicks));

		// WIDTH_UNIT label:
		svgRulerText = std::make_unique<SvgText>();
		svgRulerText->setPosition(rulerWdith + 1, rulerHeight * 2.9);
		svgRulerText->setSize( rulerHeight * 0.7);
		sprintf( str, "%.2f %s", widthlabel, rulerWidthUnit.c_str() );

		svgRulerText->setText(str);
		svgRulerText->setFont("Georgia");
		svgWriter.addElement(std::move(svgRulerText));

		// Draw all the ruler elements as group
		svgWriter.endGroup();
	}
	// Block RULER --- END ---------------------------------------------------------------------------------------------------------------------------------

	// SVG:
	svgWriter.writeToFile(rFileName.toStdWString());
	// PNG -- optional
	if( rFileNamePNG.length() > 0 ) {
		QImage img(imRGBA, imWidth, imHeight, 4 * imWidth * sizeof (unsigned char), QImage::Format_RGBA8888);
		img.save(rFileNamePNG);

		emit sStatusMessage( "Screenshot saved as PNG and SVG with an embedded PNG having transparency to: " + rFileName + " and " + rFileNamePNG );
	} else {
		emit sStatusMessage( "Screenshot saved as SVG with an embedded PNG having transparency to: " + rFileName );
	}

	// free arrays:
	delete[] imRGBA;
	delete[] silhArr;

	return( true );
}

//! Check if inkscape is available/installed
//! @returns false in case of error or no inkscape on the system
bool MeshWidget::checkInkscapeAvailability() {
    // --- Check external Tools i.e. Inkscape and convert/ImageMagick --------------------------------------------------------------------------------------
    QSettings settings;
    auto inkscapePath = settings.value("Inkscape_Path", "").toString();
    if(inkscapePath.length() == 0)
        inkscapePath = "inkscape";
    bool checkInkscapeFailed = false;
    QProcess testRunInkscape;
    testRunInkscape.start( inkscapePath + " --version" );
    if( !testRunInkscape.waitForFinished() ) {
        cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR testing Inkscape had a timeout!" << endl;
        checkInkscapeFailed = true;
    }
    if( testRunInkscape.exitStatus() != QProcess::NormalExit ) {
        cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR testing Inkscape had no normal exit!" << endl;
        checkInkscapeFailed = true;
    }
    if( testRunInkscape.exitCode() != 0 ) {
        cerr << "[QGMMainWindow::" << __FUNCTION__ << "] ERROR Inkscape exit code: " << testRunInkscape.exitCode() << endl;
        QString outInkscapeErr( testRunInkscape.readAllStandardError() );
        cerr << "[QGMMainWindow::" << __FUNCTION__ << "] Inkscape error: " << outInkscapeErr.toStdString().c_str() << endl;
        checkInkscapeFailed = true;
    }
    QString outInkscape( testRunInkscape.readAllStandardOutput() );
    cout << "[QGMMainWindow::" << __FUNCTION__ << "] Inkscape check: " << outInkscape.simplified().toStdString().c_str() << endl;
    if( checkInkscapeFailed ) {
        SHOW_MSGBOX_WARN_TIMEOUT( tr("Inkscape error"), tr("Checking Inkscape for presence and functionality failed!"), 5000 );
    }
    return( true );
}

//! Export polylines defined by an intersecting plane as SVG.
//!
//! @returns false in case of an error or user abort or no qualified polylines present. True otherwise.
bool MeshWidget::exportPlaneIntersectPolyLinesSVG() {

	// 0.) Sanity check
	if( mMeshVisual == nullptr ) {
		SHOW_MSGBOX_CRIT( tr("ERROR"), tr("No mesh present.") );
		return( false );
	}

	std::set<unsigned int> axisPolylines;
	std::set<unsigned int> planePolylines;

    // 0.5.) Is Inkscape available?
    if(!checkInkscapeAvailability()){
        return( false );
    }

	// 1.) get qualified polylines and store their ID's into the appropriate set
	for(unsigned int i = 0; i<mMeshVisual->getPolyLineNr(); ++i)
	{
		PolyLine* polyLine = mMeshVisual->getPolyLinePos(i);
		Plane* polyLinePlane = nullptr;
		bool hasPolyLinePlane = polyLine->getIntersectPlane(&polyLinePlane);

		if(hasPolyLinePlane)
		{
			polyLinePlane->getDefinedBy() == Plane::AXIS_POINTS_AND_POSITION ?
			            axisPolylines.insert(i) :
			            planePolylines.insert(i);
		}

		delete polyLinePlane;
	}

	// 2.) abort if there are no qualified polylines
	if(axisPolylines.empty() && planePolylines.empty())
	{
		SHOW_MSGBOX_WARN_TIMEOUT( tr("Warning"), tr("No qualified polylines i.e. intersections by planes present."), 5000 );
		LOG::info() << "[MeshWidget::" << __FUNCTION__ << "] No qualified polylines found." << "\n";
		return( false );
	}

	// 3.) Ask for the filename
    QString baseName = QString::fromStdWString( mMeshVisual->getBaseName().wstring() );
    QString filePath = QString::fromStdWString( mMeshVisual->getFileLocation().wstring() );
	QString fileName = QFileDialog::getSaveFileName( mMainWindow, tr( "Save intersections as" ), \
	                                                 filePath + baseName + "_profiles.svg", \
	                                                 tr( "Scaleable Vector Graphic (*.svg)" ) );
	if( fileName.isEmpty() ) { // Cancel pressed
		SHOW_MSGBOX_WARN( tr("User abort"), tr("No files saved.") );
		return( false );
	}

#ifdef WIN32
	fileName.replace(QString("/"), QString("\\"));
#endif

	const double fontHeight     = 5.0;
	const double fontExtraSpace = fontHeight+3.0;

	SvgWriter svgWriter;

	double canvasWidth  = 0.0;
	double canvasHeight = 0.0;

	// Add space for text:
	canvasHeight += fontExtraSpace;

	//! \todo change to inventory no. (meta-data)
	// 4.) Write filename within SVG
	auto svgTextName = std::make_unique<SvgText>();
	svgTextName->setFont("Sans");
	svgTextName->setSize(fontHeight*mParamFlt[SVG_SCALE]);
	svgTextName->setPosition(0.0, fontHeight*mParamFlt[SVG_SCALE]);
	svgTextName->setText(baseName.toStdString());

	svgWriter.addElement(std::move(svgTextName));

	//helper function to get sorrounding bbox from set of polylines
	auto getPolylinesBBOX = [this] (const std::set<unsigned int>& ids, bool useAxis,
	        double& minX, double& maxX, double& minY, double& maxY) {
		minX = +_INFINITE_DBL_;
		minY = +_INFINITE_DBL_;

		maxX = -_INFINITE_DBL_;
		maxY = -_INFINITE_DBL_;

		for(auto id : ids)
		{
			PolyLine* currPolyLine = mMeshVisual->getPolyLinePos(id);
			double tminX, tminY, tminZ, tmaxX, tmaxY, tmaxZ;
			currPolyLine->getBoundingBox(&tminX, &tminY, &tminZ, &tmaxX, &tmaxY, &tmaxZ, true, useAxis);

			minX = std::min(minX, tminX);
			maxX = std::max(maxX, tmaxX);

			minY = std::min(minY, tminY);
			maxY = std::max(maxY, tmaxY);
		}
	};

	if(!axisPolylines.empty())
	{
		// 5.) Export axis intersections
		//write axis polyline label
		auto svgTextName = std::make_unique<SvgText>();
		svgTextName->setFont("Sans");
		svgTextName->setSize(fontHeight*mParamFlt[SVG_SCALE]);
		svgTextName->setPosition(0.0, (fontHeight + canvasHeight)*mParamFlt[SVG_SCALE]);
		svgTextName->setText("Plane-Axis intersections:");
		svgWriter.addElement(std::move(svgTextName));

		canvasHeight += fontExtraSpace;

		double minX,maxX,minY,maxY;

		getPolylinesBBOX(axisPolylines, true, minX, maxX, minY, maxY);
		const double bboxWidth  = maxX - minX;
		const double bboxHeight = maxY - minY;

		const double polyLineWidth = 0.5;
		if( !screenshotSVGexportPlaneIntersections( minX, canvasHeight + maxY, polyLineWidth, minX, svgWriter, axisPolylines ) ) {
			LOG::error() << "[MeshWidget::" << __FUNCTION__ << "] ERROR: screenshotSVGexportPolyLines falied!\n";
		}

		//update offset + spacer
		canvasWidth = std::max(canvasWidth, bboxWidth);
		canvasHeight += bboxHeight + fontExtraSpace;
	}

	if(!planePolylines.empty())
	{
		// 6.) Export plane intersections
		//write axis polyline label
		auto svgTextName = std::make_unique<SvgText>();
		svgTextName->setFont("Sans");
		svgTextName->setSize(fontHeight*mParamFlt[SVG_SCALE]);
		svgTextName->setPosition(0.0, (fontHeight + canvasHeight)*mParamFlt[SVG_SCALE]);
		svgTextName->setText("Plane intersections:");
		svgWriter.addElement(std::move(svgTextName));

		canvasHeight += fontExtraSpace;

		double minX,maxX,minY,maxY;

		getPolylinesBBOX(planePolylines, false, minX, maxX, minY, maxY);
		const double bboxWidth  = maxX - minX;
		const double bboxHeight = maxY - minY;

		const double polyLineWidth = 0.5;
		if( !screenshotSVGexportPlaneIntersections( minX, canvasHeight + maxY, polyLineWidth, ( maxX - minX) * -0.5, svgWriter, planePolylines ) ) {
			LOG::error() << "[MeshWidget::" << __FUNCTION__ << "] ERROR: screenshotSVGexportPolyLines falied!\n";
		}

		//update canvas size
		canvasWidth = std::max(canvasWidth, bboxWidth);
		canvasHeight += bboxHeight + fontExtraSpace;
	}

	LOG::debug() << "[MeshWidget::" << __FUNCTION__ << "] Drawing size width: " << canvasWidth << " height: " << canvasHeight << "\n";
	svgWriter.setSize(canvasWidth  * mParamFlt[SVG_SCALE], canvasHeight * mParamFlt[SVG_SCALE]);

	// 7.) SVG final steps:
	svgWriter.writeToFile(fileName.toStdWString());

	std::string inkscapeCommand;
	getParamStringMeshWidget(MeshWidgetParams::INKSCAPE_COMMAND, &inkscapeCommand);
	// 8.) Display with Inkscape:
	if( !QProcess::startDetached( QString(inkscapeCommand.c_str()) + " \"" + fileName + "\"" ) ) {
		LOG::error() << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Inkscape won't start!\n";
	}

	return( true );
}

//! Draw polygonal lines to a cairo SVG canvas.
//!
//! @return false in case of an error. True otherwise.
bool MeshWidget::screenshotSVGexportPlaneIntersections(double rOffsetX,
                double rOffsetY,
                double rPolyLineWidth,
                double axisOffset,
                SvgWriter& svgWriter, const std::set<unsigned int>& polylineIDs) {

	// Axis coordinates. Note x=0.0
	double minYforAxis = +_INFINITE_DBL_;
	double maxYforAxis = -_INFINITE_DBL_;
	// x-value for horizontal lines from the axis to the top and bottom points.
	double minYBottomX = _NOT_A_NUMBER_DBL_;
	double maxYTopX    = _NOT_A_NUMBER_DBL_;

	unsigned int ctrPolyLinesDrawn = 0;
	for( auto i : polylineIDs) {

		auto svgLine = std::make_unique<SvgPath>();
		svgLine->setColor(0.0,0.0,0.8);
		svgLine->setLineWidth(static_cast<float>(rPolyLineWidth));
		svgLine->setLineCap(SvgPath::LineCap::CAP_ROUND);
		svgLine->setLineJoin(SvgPath::LineJoin::JOIN_ROUND);

		// Intersections using a plane
		PolyLine* currPoly     = mMeshVisual->getPolyLinePos( i );

		Plane*    intersectPlane( nullptr );
		bool      hasIntersectPlane = currPoly->getIntersectPlane( &intersectPlane );
		// Skip all polylines NOT computed using a plane.
		if( !hasIntersectPlane ) {
			//cout << "[MeshWidget::" << __FUNCTION__ << "] Plane: " << intersectPlane << endl;
			continue;
		}
		ctrPolyLinesDrawn++;

		delete intersectPlane;

		size_t polyLen = static_cast<size_t>(currPoly->length());
		vector<Vector3D> polyCoords;    // 3D-Coordinates in object space.
		vector<double> screenCoords;    // Coordinates in drawing/SVG space.
		currPoly->getVertexCoordsInPlane( &polyCoords, true );
		screenCoords.reserve( polyCoords.size()*2 );
		for(const auto& vertexPos : polyCoords)
		{
			screenCoords.push_back( (+vertexPos.getX()-rOffsetX) * mParamFlt[SVG_SCALE] );
			screenCoords.push_back( (-vertexPos.getY()+rOffsetY) * mParamFlt[SVG_SCALE] );
		}
		// Draw using the projected coordinates
		svgLine->moveTo( screenCoords[0], screenCoords[1] );
		for( size_t j=1; j<polyLen; j++ ) {
			// Draw line
			double currXcoord = screenCoords.at(j*2);
			double currYcoord = screenCoords.at(j*2+1);

			svgLine->lineTo( currXcoord, currYcoord );
			// Determine points for drawing the axis
			if( minYforAxis > currYcoord ) {
				// Set lower point of the axis
				minYforAxis = currYcoord;
				// Store the x-coordinate for the horizontal line:
				if( currXcoord != 0.0 ) { // Intentionally ignore 0.0 to avoid lines of length zero.
					minYBottomX = currXcoord;
				}
			}
			if( maxYforAxis < currYcoord ) {
				// Set lower point of the axis
				maxYforAxis = currYcoord;
				// Store the x-coordinate for the horizontal line:
				if( currXcoord != 0.0 ) { // Intentionally ignore 0.0 to avoid lines of length zero.
					maxYTopX = currXcoord;
				}
			}
			// Simple just for drawing the axis:
			// minYforAxis = min( minYforAxis, screenCoords.at(j*2+1) );
			// maxYforAxis = max( maxYforAxis, screenCoords.at(j*2+1) );
		}

		// Draw now
		svgWriter.addElement( std::move(svgLine));
		// cout << "[MeshWidget::" << __FUNCTION__ << "] ------------- " << endl;
	}

	std::string widthUnit;
	getParamStringMeshWidget( RULER_WIDTH_UNIT, &widthUnit );

	//helper function to draw line with length as label
	auto drawCenterLabledLine = [&svgWriter, &rPolyLineWidth, &widthUnit, this] (double x1, double y1, double x2, double y2, bool vert = false, bool dashed = false) {
		auto svgHLine = std::make_unique<SvgPath>();
		svgHLine->setColor(0.75,0.0,0.0);
		svgHLine->setLineWidth(static_cast<float>(rPolyLineWidth) * 1.5F);
		svgHLine->moveTo(x1, y1);
		svgHLine->lineTo(x2, y2);
		svgHLine->setLineCap(SvgPath::LineCap::CAP_ROUND);
		svgHLine->setLineJoin(SvgPath::LineJoin::JOIN_ROUND);

		if( dashed ) {
			static const double dashedLine[] = { 1.0, 2.0, 5.0, 2.0 };
			static int lenDashed = sizeof( dashedLine ) / sizeof( dashedLine[0] );
			svgHLine->setLineDash(dashedLine, lenDashed, 1);
		}

		svgWriter.addElement(std::move(svgHLine));

		auto svgLineText = std::make_unique<SvgText>();

		double lineWidthPixels = 0.0;
		if(vert)
		{
			//-rOffsetX * mParamFlt[SVG_SCALE] + 2.0, (minYforAxis + maxYforAxis) * 0.5
			svgLineText->setPosition(x1 + 2.0, (y1 + y2) * 0.5);
			svgLineText->setRotation(90);
			lineWidthPixels = std::abs(y2 - y1);
		}
		else
		{
			svgLineText->setPosition((x1 + x2) * 0.5, y1 + 4.0);
			lineWidthPixels = std::abs(x2 - x1);
		}
		svgLineText->setText(std::to_string(lineWidthPixels / mParamFlt[SVG_SCALE]) + widthUnit);

		svgLineText->setSize(4.0);
		svgLineText->setTextAnchor(SvgText::TextAnchor::ANCHOR_MIDDLE);

		svgWriter.addElement(std::move(svgLineText));
	};

	// Draw horizontal lines, when present:
	if( isnormal( minYBottomX ) ) {
		drawCenterLabledLine(-axisOffset * mParamFlt[SVG_SCALE], minYforAxis,
		                     minYBottomX                     , minYforAxis);
	}
	if( isnormal( maxYTopX ) ) {
		drawCenterLabledLine(-axisOffset * mParamFlt[SVG_SCALE], maxYforAxis,
		                     maxYTopX                        , maxYforAxis);
	}

	bool dashedAxis = true;
	getParamFlagMeshWidget( EXPORT_SVG_AXIS_DASHED, &dashedAxis );

	drawCenterLabledLine(-axisOffset * mParamFlt[SVG_SCALE], minYforAxis ,
	                     -axisOffset * mParamFlt[SVG_SCALE], maxYforAxis ,
	                     true, dashedAxis);

	LOG::info() << "[MeshWidget::" << __FUNCTION__ << "] Polylines: " << mMeshVisual->getPolyLineNr() << "\n";
	LOG::info() << "[MeshWidget::" << __FUNCTION__ << "] ctrPolyLinesDrawn: " << ctrPolyLinesDrawn << "\n";

	return( true );
}

//! Draw polygonal lines to a SVG canvas.
//!
//! @return false in case of an error. True otherwise.
bool MeshWidget::screenshotSVGexportPolyLines(Vector3D& cameraViewDir, Matrix4D& matView, double polyScaleWdith, double polyScaleHeight, double polyLineWidth , SvgWriter& svgWriter) {
	LOG::info() << "[MeshWidget::" << __FUNCTION__ << "] Polylines: " << mMeshVisual->getPolyLineNr() << "\n";

	for( unsigned int i=0; i<mMeshVisual->getPolyLineNr(); i++ ) {
		auto svgPolyLinePath = std::make_unique<SvgPath>();
		svgPolyLinePath->setLineWidth(polyLineWidth);
		svgPolyLinePath->setColor(0.9,0.0,0.0);

		PolyLine* currPoly     = mMeshVisual->getPolyLinePos( i );

		Plane*    intersectPlane( nullptr );
		bool      hasIntersectPlane = currPoly->getIntersectPlane( &intersectPlane );
		if( hasIntersectPlane ) {
			// Intersecting lines become dark green:
			svgPolyLinePath->setColor(0.0,0.5,0.0);
			//cout << "[MeshWidget::" << __FUNCTION__ << "] Plane: " << intersectPlane << endl;
		}

		delete intersectPlane;

		// Determine polylines with pointing away from the camera.
		// This will provide a basic filter for polylines not seen by the viewer, which will do the trick for most objects.
		float     angleToCam   = currPoly->angleToNormal( cameraViewDir );
		//cout << "[MeshWidget::" << __FUNCTION__ << "] angleToCam: " << angleToCam*180.0/M_PI << endl;
		bool pointingAway = false;
		if( ( mParamFlag[ORTHO_MODE] ) && ( angleToCam > M_PI/2.0 ) ) {
			pointingAway = true;
		}
		if( ( !mParamFlag[ORTHO_MODE] ) && ( angleToCam > ( M_PI/2.0 - mParamFlt[FOV_ANGLE]*M_PI/360.0 ) ) ) { // 360=2*180 because we need fovAngle/2 in radiant
			pointingAway = true;
		}

		// Export only:
		//  +) Polylines computed using intersecting planes.
		//  +) other polylines pointing towards the view direction.
		if( !hasIntersectPlane && pointingAway ) {
			continue;
		}

		int polyLen = currPoly->length();
		vector<double> polyCoords;    // 3D-Coordinates in object space.
		vector<float>  screenCoords;  // Coordinates in drawing/SVG space.
		currPoly->getVertexCoords( &polyCoords );
		screenCoords.resize( polyLen*2, _NOT_A_NUMBER_DBL_ );
		for( int j=0; j<polyLen; j++ ) {
			Vector3D coordProjected = Vector3D( &polyCoords[j*3], 1.0f ) * matView;
			coordProjected.normalizeW();
			coordProjected += Vector3D( 1.0, 1.0, 1.0, 0.0 );
			//coordProjected.dumpInfo();
			//cout << "[MeshWidget::" << __FUNCTION__ << "] " << coordProjected.getX()*xywh[2] << " " << coordProjected.getY()*xywh[3] << endl;
			screenCoords.at(j*2)   = (coordProjected.getX()/2.0)     * polyScaleWdith;
			screenCoords.at(j*2+1) = (1.0-coordProjected.getY()/2.0) * polyScaleHeight;
		}

		// Draw using the projected coordinates
		svgPolyLinePath->moveTo(screenCoords[0], screenCoords[1]);
		for( int j=1; j<polyLen; j++ ) {
			svgPolyLinePath->lineTo(screenCoords.at(j*2), screenCoords.at(j*2+1));
		}

		// Draw now
		svgWriter.addElement(std::move(svgPolyLinePath));
	}

	return( true );
}

//! Shows dialog to enter a filename for a ruler matching the screenshot resolution.
void MeshWidget::screenshotRuler() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//! Ortho mode only.
	if( !mParamFlag[ORTHO_MODE] ) {
		QMessageBox msgBox;
		msgBox.setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
		msgBox.setText( tr("Not possible in perspective projection mode.") );
		msgBox.setIcon( QMessageBox::Warning );
		msgBox.setInformativeText( tr("Do you want to change to orthographic projection?") );
		msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
		int ret = msgBox.exec();
		if( ret == QMessageBox::Yes ) {
			setParamFlagMeshWidget( ORTHO_MODE, true );
		} else {
			return;
		}
	}

	// Prepare filename using a pattern
	string fileNamePattern;
	getParamStringMeshWidget( FILENAME_EXPORT_RULER, &fileNamePattern );
	char buffer[256];

	double resolutionDPI = _NOT_A_NUMBER_DBL_;
	double pixelWidth, pixelHeight;
	bool  pixelWorldSizeSet = getViewPortPixelWorldSize( pixelWidth, pixelHeight );

	QString dpiString = "";
	if( pixelWorldSizeSet ) {
		resolutionDPI = round( 25.4/pixelWidth );
		dpiString = QString("_%1DPI").arg(resolutionDPI);
	}

	sprintf( buffer, fileNamePattern.c_str(), mMeshVisual->getBaseName().c_str(), dpiString.toStdString().c_str() );

	string rulerWidthUnit;
	getParamStringMeshWidget( RULER_WIDTH_UNIT, &rulerWidthUnit );

	QGMDialogRuler dialogRuler;
	dialogRuler.setFileName( QString( buffer ) );
	dialogRuler.setWidth( mParamFlt[RULER_WIDTH] );
	dialogRuler.setHeight( mParamFlt[RULER_HEIGHT] );
	dialogRuler.setUnit( mParamFlt[RULER_UNIT] );
	dialogRuler.setUnitTicks( mParamFlt[RULER_UNIT_TICKS] );
	dialogRuler.setWidthUnit( QString( rulerWidthUnit.c_str() ) );
	// Connect to MeshWidgetParams
	QObject::connect( &dialogRuler, SIGNAL(screenshotRuler(QString)), this, SLOT(screenshotRuler(QString)) );
	QObject::connect( &dialogRuler, SIGNAL(setParamFloatMeshWidget(MeshWidgetParams::eParamFlt,double)), \
	                  this, SLOT(setParamFloatMeshWidget(MeshWidgetParams::eParamFlt,double)) );
	// Show dialog
	if( dialogRuler.exec() == QDialog::Rejected ) {
		emit sStatusMessage( "Screenshot ruler: User cancel!" );
		return;
	}
}

//! Render full object using tiled rendering.
bool MeshWidget::screenshotTiledPNG(
                QString   rFileName,                //!< Filename for storage.
                double&   rWidthReal,               //!< Return value: width in real world coordinates - only in orthographic projection. NaN otherwise.
                double&   rHeigthReal,              //!< Return value: heigth in real world coordinates - only in orthographic projection. NaN otherwise.
                OffscreenBuffer* offscreenBuffer,   //!< Offscreen rendering when set.
                int rBorderSize                     //!< Border treatment e.g. used b NPR shading.
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Default for non-orthographic mode and errors.
	rWidthReal  = _NOT_A_NUMBER_DBL_;
	rHeigthReal = _NOT_A_NUMBER_DBL_;

	//! Ortho mode only.
	if( !mParamFlag[ORTHO_MODE] ) {
		//! \todo Tiled rendering for perspective rendering.
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Tiled rendering NOT implemented for perspective view!" << endl;
		SHOW_MSGBOX_WARN_TIMEOUT( tr("Not implemented"), tr("Tiled rendering is not available for perspective views."), 5000 );
		return false;
	}

	//! 0. Store view settings, which we need to change.
	Vector3D orimCenterView = mCenterView;
	Vector3D orimCenterCamera = mCameraCenter;
	bool showGridRect;
	bool showGridPolarCircles;
	bool showGridPolarLines;
	getParamFlagMeshWidget( SHOW_GRID_RECTANGULAR, &showGridRect );
	getParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, &showGridPolarCircles );
	getParamFlagMeshWidget( SHOW_GRID_POLAR_LINES, &showGridPolarLines );
	setParamFlagMeshWidget( SHOW_GRID_RECTANGULAR, false );
	setParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, false );
	setParamFlagMeshWidget( SHOW_GRID_POLAR_LINES, false );

	//! 1. Reset view center to bounding box center.
	//mCenterView = mMeshVisual->getBoundingBoxCenter();
	//move whole camera to view at center, not just the view direction
	Vector3D vecToCenter = mMeshVisual->getBoundingBoxCenter() - mCenterView;
	mCenterView += vecToCenter;
	mCameraCenter += vecToCenter;

	double bBoxRadius = mMeshVisual->getBoundingBoxRadius();
	//! 2. Fetch the shift values and NOT compute them. Reason: computing is more precise and the tiles will be off for a few pixels due to numerics.
	double realWidth  = 0.0;
	double realHeight = 0.0;
	if( !getViewPortResolution( realWidth, realHeight ) ) {
		return( false );
	}

	//! 3. Estimate the number of tiles for each direction.
	GLint    viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	double windowRatio = static_cast<double>( viewport[3] - 2*rBorderSize ) /
	                     static_cast<double>( viewport[2] - 2*rBorderSize );

	unsigned int tilesX = static_cast<unsigned int>( ceil( ( 2.0*bBoxRadius ) / (realWidth - 2.0*static_cast<double>(rBorderSize) ) ) );
	unsigned int tilesY = static_cast<unsigned int>( ceil( static_cast<double>(tilesX) / windowRatio ) );

	cout << "[MeshWidget::" << __FUNCTION__ << "] Tiles: " << tilesX << " x " << tilesY << endl;
	//! 4. Prepare array holding the final image.
	unsigned char* imRGBA;
	uint64_t imWidth;
	uint64_t imHeight;
	prepareTile( tilesX, tilesY, &imRGBA, &imWidth, &imHeight, static_cast<uint64_t>( rBorderSize ) );

	double widthRatio = realWidth / imWidth;
	double heightRatio = realHeight / imHeight;

	//! 5. Fetch tiles by shifting.
	GLdouble orthoViewPort[4];
	orthoViewPort[2] = +bBoxRadius + (rBorderSize * heightRatio);              // bottom in glOrtho
	orthoViewPort[3] = +bBoxRadius - realHeight + (rBorderSize * heightRatio); // top in glOrtho
	//! a. Allocate memory for the frame- and z-buffer.
	GLubyte* imArrayGL  = new GLubyte[imWidth*3*imHeight];
	float* pixelZBuffer = new float[imWidth*imHeight];
	//! b. Fetch tiles.
	double tileCount = static_cast<double>( tilesX * tilesY );
	mMeshVisual->showProgressStart( "Tiled Rendering - High-res Screenhot" );
	//! \todo Add/implement showProgressStart for MeshWidget!
	for( unsigned int iy=0; iy<tilesY; iy++ ) {
		orthoViewPort[0] = -bBoxRadius + realWidth - (rBorderSize * widthRatio); // left in glOrtho
		orthoViewPort[1] = -bBoxRadius - (rBorderSize * widthRatio);             // right in glOrtho
		cout << "[MeshWidget::" << __FUNCTION__ << "] |";
		for( unsigned int ix=0; ix<tilesX; ix++ ) {
 			setView( orthoViewPort );
 			repaint(); // update() will not work at this point as it tries to prevent flickering!
			fetchFrameAndZBufferTile( tilesX, tilesY, ix, iy, imRGBA, imWidth, imHeight, offscreenBuffer, rBorderSize );
			orthoViewPort[0] += realWidth - (2*rBorderSize * widthRatio);
			orthoViewPort[1] += realWidth - (2*rBorderSize * widthRatio);
			cout << "X";
			double tilesDone = static_cast<double>( iy * tilesX + ix ) / tileCount;
			mMeshVisual->showProgress( tilesDone, "Tiled Rendering - High-res Screenhot" );
		}
		orthoViewPort[2] -= realHeight - (2*rBorderSize * heightRatio);
		orthoViewPort[3] -= realHeight - (2*rBorderSize * heightRatio);
		cout << "| Line: " << (iy+1) << " / " << tilesY << endl;
	}
	mMeshVisual->showProgressStop( "Tiled Rendering - High-res Screenhot" );
	//! c. Free memory for the frame- and z-buffer.
	delete[] imArrayGL;
	delete[] pixelZBuffer;
	//! 6. Adjust height size and crop.
	imWidth  *= tilesX;
	imHeight *= tilesY;
	// Adapt for cropped tiled rendering:
	realWidth  *= static_cast<double>(tilesX);
	realHeight *= static_cast<double>(tilesY);
	//! \todo fix cropping
	if( !cropRGBAbyAlpha( &imWidth, &imHeight, &imRGBA, realWidth, realHeight ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] could not crop image!" << endl;
		emit sStatusMessage( "ERROR: Could not crop screenshot!" );
		return false;
	}

	//! 7. Images with width or height > 30k have to be split due to problems with libpng/libcairo.
	//!    This has to be done despite the PNG standard allows for larger images.
	uint64_t largeSplitSize;
	{ //! \todo Think about getParamUnsignedInt.
		int largeSplitSizeFetch;
		getParamIntegerMeshWidget( IMAGE_SPLIT_SIZE, &largeSplitSizeFetch );
		largeSplitSize = static_cast<uint64_t>(largeSplitSizeFetch);
	}
	uint64_t largeTilesX = ( imWidth  / largeSplitSize ) + 1;
	uint64_t largeTilesY = ( imHeight / largeSplitSize ) + 1;
	bool splitLargeImage = ( largeTilesX > 1 ) || ( largeTilesY > 1 );
	if( splitLargeImage ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Splitting into " << largeTilesX << " and " << largeTilesY << " sub-images required." << endl;
	} else {
		cout << "[MeshWidget::" << __FUNCTION__ << "] No splitting into sub-images required." << endl;
	}

	//! 8. Write to file and free memory! Appends DPI to the filename.
	int   extSeperator = rFileName.lastIndexOf( "." );
	double pixelWidth    = _NOT_A_NUMBER_DBL_;
	double pixelHeight   = _NOT_A_NUMBER_DBL_;
	double resolutionDPI = _NOT_A_NUMBER_DBL_;
	bool  pixelWorldSizeSet = getViewPortPixelWorldSize( pixelWidth, pixelHeight );
	if( pixelWorldSizeSet ) {
		resolutionDPI = round( 25.4/pixelWidth );
		cout << "[MeshWidget::" << __FUNCTION__ << "] Resolution in DPI: " << resolutionDPI << endl;
	} else {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Resolution in DPI not set." << endl;
	}
	bool appendDPItoFilename = false;
	getParamFlagMeshWidget( MeshWidgetParams::SCREENSHOT_FILENAME_WITH_DPI, &appendDPItoFilename );

	double dpm;
	if(!getViewPortDPM(dpm))
	{
		return false;
	}

	if( splitLargeImage ) {
		for( uint64_t subImageX=0; subImageX<largeTilesX; subImageX++ ) {
			for( uint64_t subImageY=0; subImageY<largeTilesY; subImageY++ ) {
				uint64_t subImageWidth  = min( largeSplitSize, imWidth-subImageX*largeSplitSize );
				uint64_t subImageHeight = min( largeSplitSize, imHeight-subImageY*largeSplitSize );
				cout << "[MeshWidget::" << __FUNCTION__ << "] Sub-image size: " << subImageWidth << " x " << subImageHeight << "" << endl;
				unsigned char* imSubRGBA = new unsigned char[subImageWidth*4*subImageHeight];

				uint64_t offSetX = subImageX*largeSplitSize;
				uint64_t offSetY = subImageY*largeSplitSize;
				for( uint64_t ix=0; ix<subImageWidth; ix++ ) {
					for( uint64_t iy=0; iy<subImageHeight; iy++ ) {
						imSubRGBA[ix*4+iy*subImageWidth*4]   = imRGBA[(offSetX+ix)*4+(offSetY+iy)*imWidth*4];
						imSubRGBA[ix*4+iy*subImageWidth*4+1] = imRGBA[(offSetX+ix)*4+(offSetY+iy)*imWidth*4+1];
						imSubRGBA[ix*4+iy*subImageWidth*4+2] = imRGBA[(offSetX+ix)*4+(offSetY+iy)*imWidth*4+2];
						imSubRGBA[ix*4+iy*subImageWidth*4+3] = imRGBA[(offSetX+ix)*4+(offSetY+iy)*imWidth*4+3];
					}
				}
				// Prepate filename
				QString rFileNameSubImage( rFileName );
				if( ( extSeperator != -1 ) && ( pixelWorldSizeSet ) ) {
					QString strDPI = QString( "_%2_%1." ).arg( subImageX ).arg( subImageY );
					if( appendDPItoFilename ) {
						strDPI += QString( "_%1DPI" ).arg( resolutionDPI );
					}
					rFileNameSubImage.replace( extSeperator, 1, strDPI );
				}
				// Write file
				if( !writePNG( rFileNameSubImage, subImageWidth, subImageHeight, imSubRGBA,
							   dpm, dpm ) ) {
					cerr << "[MeshWidget::" << __FUNCTION__ << "] could not write to '" << rFileNameSubImage.toStdString() << "'!" << endl;
					emit sStatusMessage( "ERROR: Could not save screenshot as PNG with transparency to: " + rFileNameSubImage + "!" );
					return false;
				}
				cout << "[MeshWidget::" << __FUNCTION__ << "] File written: '" << rFileNameSubImage.toStdString() << "" << endl;
				delete[] imSubRGBA;
			}
		}
	} else {
		if( appendDPItoFilename && ( extSeperator != -1 ) && ( pixelWorldSizeSet ) ) {
			QString strDPI = QString( "_%1DPI." ).arg( resolutionDPI );
			rFileName.replace( extSeperator, 1, strDPI );
		}
		if( !writePNG( rFileName, imWidth, imHeight, imRGBA, dpm, dpm ) ) {
			cerr << "[MeshWidget::" << __FUNCTION__ << "] could not write to '" << rFileName.toStdString() << "'!" << endl;
			emit sStatusMessage( "ERROR: Could not save screenshot as PNG with transparency to: " + rFileName + "!" );
			return false;
		}
		cout << "[MeshWidget::" << __FUNCTION__ << "] File written: '" << rFileName.toStdString() << "" << endl;
	}

	// Pass real world dimensions (ortho only!)
	rWidthReal  = realWidth;
	rHeigthReal = realHeight;

	//! 9. Free memory.
	delete[] imRGBA;

	//! 10. Restore view settings.
	mCenterView = orimCenterView;
	mCameraCenter = orimCenterCamera;
	setParamFlagMeshWidget( SHOW_GRID_RECTANGULAR, showGridRect );
	setParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, showGridPolarCircles );
	setParamFlagMeshWidget( SHOW_GRID_POLAR_LINES, showGridPolarLines );
	setView();
	update();

	return( true );
}

//! Exports a ruler matching the screenshot resolution.
bool MeshWidget::screenshotRuler(const QString& fileName ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//! Ortho mode only.
	if( !mParamFlag[ORTHO_MODE] ) {
		return false;
	}

	double pixelWidth;
	double pixelHeight;
	int imWidth;
	int imHeight;
	unsigned char* imArray;
	fetchRuler( &imArray, &imWidth, &imHeight, pixelWidth, pixelHeight );

	Image2D imRuler;
	//cout << "[MeshWidget::" << __FUNCTION__ << "] " << 10.0/pixelWidth << " " << 10.0/pixelHeight << " dots/cm." << endl;
	imRuler.setResolution( 10.0/pixelWidth, 10.0/pixelHeight );
    imRuler.writePNG( fileName.toStdString(), imWidth, imHeight, imArray, true );

	delete[] imArray;

	emit sStatusMessage( "Ruler saved to: " + fileName );
	return true;
}

//! Fetch array for a ruler. See also screenshotRuler( QString fileName )-
bool MeshWidget::fetchRuler(
                unsigned char** rImArray,
                int* rImWidth,
                int* rImHeight,
                double& rPixelWidth,
                double& rPixelHeight
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( !getViewPortPixelWorldSize( rPixelWidth, rPixelHeight ) ) {
		return( false );
	}

	// Round to even number of pixels:
	*rImWidth  = 2*round( mParamFlt[RULER_WIDTH]  / ( 2.0*rPixelWidth  ) );
	*rImHeight = 2*round( mParamFlt[RULER_HEIGHT] / ( 2.0*rPixelHeight ) );
	//cout << "[MeshWidget::" << __FUNCTION__ << "] " << *imWidth << " " << *imHeight << " pixel." << endl;

	(*rImArray) = new unsigned char[(*rImWidth)*(*rImHeight)*3];

	// Set all pixel to white
	for( int i=0; i<(*rImWidth)*(*rImHeight)*3; i++ ) {
		(*rImArray)[i] = 255;
	}
	// Set checker pattern
	float runLenChecker = 0.0;
	bool  checker       = false;
	float runLenTick    = 0.0;
	for( int i=0; i<*rImWidth; i++ ) {
		if( runLenChecker > mParamFlt[RULER_UNIT] ) {
			runLenChecker = runLenChecker - mParamFlt[RULER_UNIT];
			checker = !checker;
		}
		if( checker ) {
			for( int j=0; j<*rImHeight/2; j++ ) {
				(*rImArray)[(*rImWidth*j+i)*3]   = 0;
				(*rImArray)[(*rImWidth*j+i)*3+1] = 0;
				(*rImArray)[(*rImWidth*j+i)*3+2] = 0;
			}
		} else {
			for( int j=*rImHeight/2; j<*rImHeight; j++ ) {
				(*rImArray)[(*rImWidth*j+i)*3]   = 0;
				(*rImArray)[(*rImWidth*j+i)*3+1] = 0;
				(*rImArray)[(*rImWidth*j+i)*3+2] = 0;
			}
		}
		if( ( mParamFlt[RULER_UNIT_TICKS] > 0.0 ) && ( runLenTick > mParamFlt[RULER_UNIT_TICKS] ) ) {
			for( int j=0; j<*rImHeight; j++ ) {
				(*rImArray)[(*rImWidth*j+i)*3]   = 0;
				(*rImArray)[(*rImWidth*j+i)*3+1] = 0;
				(*rImArray)[(*rImWidth*j+i)*3+2] = 0;
			}
			runLenTick = runLenTick - mParamFlt[RULER_UNIT_TICKS];
		}
		runLenChecker += rPixelWidth;
		runLenTick    += rPixelWidth;
	}
	// Draw 1px wide black border - vertical
	for( int i=0; i<*rImWidth; i++ ) {
		(*rImArray)[i*3]   = 0;
		(*rImArray)[i*3+1] = 0;
		(*rImArray)[i*3+2] = 0;
		(*rImArray)[((*rImHeight-1)**rImWidth+i)*3]   = 0;
		(*rImArray)[((*rImHeight-1)**rImWidth+i)*3+1] = 0;
		(*rImArray)[((*rImHeight-1)**rImWidth+i)*3+2] = 0;
	}
	// Draw 1px wide black border - horizontal
	for( int i=0; i<*rImHeight; i++ ) {
		(*rImArray)[(*rImWidth*i)*3]   = 0;
		(*rImArray)[(*rImWidth*i)*3+1] = 0;
		(*rImArray)[(*rImWidth*i)*3+2] = 0;
		(*rImArray)[(*rImWidth*i+*rImWidth-1)*3]   = 0;
		(*rImArray)[(*rImWidth*i+*rImWidth-1)*3+1] = 0;
		(*rImArray)[(*rImWidth*i+*rImWidth-1)*3+2] = 0;
	}
	return true;
}

//! Write an RGBA PNG to file.
//! Using either Cairo or Qt.
bool MeshWidget::writePNG( const QString& rFileName,        //!< Filename for writing the image.
                           uint64_t rImWidth,  //!< Image height in pixel.
                           uint64_t rImHeight, //!< Image width in pixel.
                           unsigned char* rImRGBA,  //!< RGBA data.
                           int rDotsPerMeterWidth,  //!< Resolution in dots per meter for the width.
                           int rDotsPerMeterHeight  //!< Resolution in dots per meter for the heigth.
                         ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
    //get TTL export setting
    bool ttlExport;
    getParamFlagMeshWidget( EXPORT_TTL_WITH_PNG, &ttlExport );

	cout << "[MeshWidget::" << __FUNCTION__ << "] Image size: " << rImWidth << " x " << rImHeight << " pixel." << endl;
	QImage imageToWrite( rImRGBA, static_cast<int>(rImWidth), static_cast<int>(rImHeight), QImage::Format_RGBA8888 );
    //QMetaData meta(rFileName);
    QDateTime current=QDateTime::currentDateTime();
    //QExifImageHeader header;
    QString exifttl="@prefix exif:<https://www.w3.org/2003/12/exif/> .\n@prefix rdf:<http://www.w3.org/1999/02/22-rdf-syntax-ns#> .\n@prefix foaf:<http://xmlns.com/foaf/0.1/> . \n@prefix skos: <http://www.w3.org/2004/02/skos/core#> .\n@prefix xsd:<http://www.w3.org/2001/XMLSchema#> .\n@prefix rdfs:<http://www.w3.org/2000/01/rdf-schema#> .\n@prefix owl:<http://www.w3.org/2002/07/owl#> .\n@prefix dcat:<http://www.w3.org/ns/dcat#> .\n@prefix prov:<http://www.w3.org/ns/prov#> .\n@prefix giga:<http://www.gigamesh.eu/ont#> .\n@prefix dc:<http://purl.org/dc/terms/> .\n@prefix ex:<http://purl.org/net/ns/ex#> .\n@prefix geo:<http://www.opengis.net/ont/geosparql#> .\n@prefix wdt:<http://www.wikidata.org/prop/direct/> .\n@prefix om:<http://www.ontology-of-units-of-measure.org/resource/om-2/>.\n";
    QString id=(rFileName);
    if (ttlExport){
        id = "giga:"+id.replace(0,rFileName.lastIndexOf("/")+1,"");
        getViewSettingsTTL(exifttl,id);
        exifttl+=id+" rdf:type foaf:Image .\n";
        exifttl+=id+" dc:license <https://creativecommons.org/licenses/by-sa/4.0/> .\n";
        exifttl+= "exif:make rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:make \"Gigamesh Software\"^^xsd:string .\n";
        exifttl+= "exif:makerNote rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:makerNote \"Generated by Gigamesh\"^^xsd:string .\n";
        exifttl+= "exif:model rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:model \"Gigamesh Software Current Version\"^^xsd:string .\n";
        exifttl+= "exif:software rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:software \"Gigamesh\"^^xsd:string .\n";
        exifttl+= "exif:dateTime rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:dateTime \""+current.toString(Qt::ISODate)+"\"^^xsd:dateTime .\n";
        exifttl+= "exif:dateTimeOriginal rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:dateTimeOriginal \""+current.toString(Qt::ISODate)+"\"^^xsd:dateTime .\n";
        exifttl+= "exif:dateTimeDigitized rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:dateTimeDigitized \""+current.toString(Qt::ISODate)+"\"^^xsd:dateTime .\n";
        exifttl+= "exif:exifVersion rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:exifVersion \"2.20\"^^xsd:string .\n";
        exifttl+= "exif:fileSource rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:fileSource \""+(rFileName)+"\"^^xsd:string .\n";
        exifttl+= "exif:colorSpace rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:colorSpace \"\"^^xsd:string .\n";
        exifttl+= "exif:width rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:width \""+QString::number(rImWidth)+"\"^^xsd:integer .\n";
        exifttl+= "exif:height rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:height \""+QString::number(rImHeight)+"\"^^xsd:integer .\n";
        exifttl+= "exif:resolutionUnit rdf:type owl:ObjectProperty .\n";
        exifttl+=id+" exif:resolutionUnit om:millimeter .\n";
        exifttl+= "exif:xResolution rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:xResolution \""+QString::number(rImWidth*1000/static_cast<uint64_t>(rDotsPerMeterWidth))+"\"^^xsd:integer .\n";
        exifttl+= "exif:yResolution rdf:type owl:DatatypeProperty .\n";
        exifttl+=id+" exif:yResolution \""+QString::number(rImHeight*1000/static_cast<uint64_t>(rDotsPerMeterHeight))+"\"^^xsd:integer .\n";
    }
    //TODO exif:userComment, exif:copyright und exif:artist mit User infos, Lizenz und Kommentar füllen
    //This part may be reused if a library to write EXIF information which is GPL compatible is found
    /*QExifValue value_ImageWidth = header.value(QExifImageHeader::ImageWidth);
    
    QExifValue value_GpsLatitude = header.value(QExifImageHeader::GpsLatitude);
    QExifValue value_Make = header.value(QExifImageHeader::Make);
    QExifValue value_Model = header.value(QExifImageHeader::Model);
    header.setValue(QExifImageHeader::GpsLatitude, value_GpsLatitude);
    header.setValue(QExifImageHeader::ImageWidth, (int)rImWidth);
    header.setValue(QExifImageHeader::ImageWidth, (int)rImHeight);
    header.setValue(QExifImageHeader::Make, value_Make);
    header.setValue(QExifImageHeader::Model, value_Model);
    header.saveToJpeg("C:/myfile.jpg");*/
	// Set print resolution
	imageToWrite.setDotsPerMeterX( rDotsPerMeterWidth  );
	imageToWrite.setDotsPerMeterY( rDotsPerMeterHeight );
	// Write image
	QImageWriter imageWriter;
	imageWriter.setFileName( rFileName );
	imageWriter.setFormat( "png" );
	imageWriter.setText( "Description", "Generated by GigaMesh" );
    exifttl+= "exif:compression rdf:type owl:DatatypeProperty .\n";
    exifttl+=id+" exif:compression \""+QString::number(imageWriter.compression())+"\"^^xsd:integer .\n";
    if( !imageWriter.write( imageToWrite ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: QImageWriter failed!" << endl;
		return( false );
	}
	if( ( rDotsPerMeterWidth > 0 ) && ( rDotsPerMeterHeight > 0 ) ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Print resolution: "
		     << rDotsPerMeterWidth << " x " << rDotsPerMeterHeight << " in dots per meter." << endl;
		cout << "[MeshWidget::" << __FUNCTION__ << "] Ortho Image Size: "
		     << rImWidth*1000/static_cast<uint64_t>(rDotsPerMeterWidth) << " x "
		     << rImHeight*1000/static_cast<uint64_t>(rDotsPerMeterHeight) << " mm (unit assumed)." << endl;
	} else {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Print resolution: "
		     << " NONE - typical for perspective renderings." << endl;
	}
    if(ttlExport){
        QFile file(rFileName+".ttl");

        if( file.open( QIODevice::ReadWrite ) ){
            QTextStream stream( &file );
            stream << exifttl << endl;
        }
        file.close();
    }
	return( true );
}

//! Crop RGBA-array e.g. from framebuffer by alpha-channel.
bool MeshWidget::cropRGBAbyAlpha(
                uint64_t* rImWidth,
                uint64_t* rImHeight,
                unsigned char** rImRGBA,
                double& rRealWidth,
                double& rRealHeight
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	uint64_t xMin = ULONG_MAX;
	uint64_t xMax = 0;
	uint64_t yMin = ULONG_MAX;
	uint64_t yMax = 0;

	for( uint64_t iy=0; iy<(*rImHeight); iy++ ) {
		for( uint64_t ix=0; ix<(*rImWidth); ix++ ) {
			uint64_t posInImage = (ix+iy*(*rImWidth))*4;
			unsigned char alpha = (*rImRGBA)[posInImage+3];
			if( alpha == 0 ) {
				continue;
			}
			if( ix < xMin ) {
				xMin = ix;
			}
			if( ix > xMax ) {
				xMax = ix;
			}
			if( iy < yMin ) {
				yMin = iy;
			}
			if( iy > yMax ) {
				yMax = iy;
			}
		}
	}

	// Fallback to 1px X 1px, if the whole image has 0 alpha
	if(xMax < xMin || yMax < yMin) {
		xMin = 0; xMax = 1;
		yMin = 0; yMax = 1;
	}

	//cout << "[MeshWidget::" << __FUNCTION__ << "] Crop x: " << xMin << " - " << xMax << endl;
	//cout << "[MeshWidget::" << __FUNCTION__ << "] Crop y: " << yMin << " - " << yMax << endl;
	uint64_t imWidthCrop  = xMax - xMin;
	uint64_t imHeightCrop = yMax - yMin;
	unsigned char* imArrayCrop  = new unsigned char[imWidthCrop*4*imHeightCrop];
	for( uint64_t iy=0; iy<imHeightCrop; iy++ ) {
		for( uint64_t ix=0; ix<imWidthCrop; ix++ ) {
			uint64_t posInImageNew  = (ix+iy*imWidthCrop)*4;
			uint64_t posInImageOld  = (ix+xMin+(iy+yMin)*(*rImWidth))*4;
			imArrayCrop[posInImageNew]   = (*rImRGBA)[posInImageOld];
			imArrayCrop[posInImageNew+1] = (*rImRGBA)[posInImageOld+1];
			imArrayCrop[posInImageNew+2] = (*rImRGBA)[posInImageOld+2];
			imArrayCrop[posInImageNew+3] = (*rImRGBA)[posInImageOld+3];
		}
	}
	// Adapt real world dimensions, when given

	    rRealWidth *= static_cast<float>(imWidthCrop)/static_cast<float>(*rImWidth);
		rRealHeight *= static_cast<float>(imHeightCrop)/static_cast<float>(*rImHeight);

	// Remove old array and set new size
	delete[] (*rImRGBA);
	(*rImRGBA)   = imArrayCrop;
	(*rImWidth)  = imWidthCrop;
	(*rImHeight) = imHeightCrop;

	// Done.
	return( true );
}

//! Manually enter yaw angle.
bool MeshWidget::rotYaw() {
	double rotAngle;
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Yaw angle (degree)") );
	dlgEnterText.setDouble( 0.5f );
	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return false;
	}
	if( !dlgEnterText.getText( &rotAngle ) ) {
		return false;
	}
	return rotYaw( rotAngle );
}

//! Manually enter pitch angle.
bool MeshWidget::rotPitch() {
	double rotAngle;
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Pitch angle (degree)") );
	dlgEnterText.setDouble( 0.5f );
	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return false;
	}
	if( !dlgEnterText.getText( &rotAngle ) ) {
		return false;
	}
	return rotPitch( rotAngle );
}

//! Manually enter roll angle.
bool MeshWidget::rotRoll() {
	double rotAngle;
	QGMDialogEnterText dlgEnterText;
	dlgEnterText.setWindowTitle( tr("Roll angle (degree)") );
	dlgEnterText.setDouble( 0.5f );
	if( dlgEnterText.exec() == QDialog::Rejected ) {
		return false;
	}
	if( !dlgEnterText.getText( &rotAngle ) ) {
		return false;
	}
	return rotRoll( rotAngle );
}

//! Rotate about camera up vector
bool MeshWidget::rotYaw( double rAngle //!< yaw angle in degree
	) {
	// Inspired by (old):
	//double transArr[16] = {
	//       cameraPitchAxis.getX(), mCameraUp.getX(), -cameraRollAxis.getX(),  0.0,
	//       cameraPitchAxis.getY(), mCameraUp.getY(), -cameraRollAxis.getY(),  0.0,
	//       cameraPitchAxis.getZ(), mCameraUp.getZ(), -cameraRollAxis.getZ(),  0.0,
	//                     0.0,             0.0,                    0.0,       1.0
	//};
	// DEPRECATED using OpenGL 3.2 and greater
	//double transArr[16];
	//glGetDoublev( GL_MODELVIEW_MATRIX, transArr );
	//Vector3D camUp( transArr[1], transArr[5], transArr[9], 0.0 );
	Vector3D camUp( mMatModelView(1,0), mMatModelView(1,1), mMatModelView(1,2), 0.0 );
	rotArbitAxis( mCenterView, camUp, rAngle );
	return true;
}

//! Rotate about view direction x camera up vector
bool MeshWidget::rotPitch( double rAngle //!< pitch angle in degree
	) {
	// DEPRECATED using OpenGL 3.2 and greater
	//double transArr[16];
	//glGetDoublev( GL_MODELVIEW_MATRIX, transArr );
	//Vector3D camPitch( transArr[0], transArr[4], transArr[8], 0.0 );
	Vector3D camPitch( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 );
	rotArbitAxis( mCenterView, camPitch, rAngle );
	return true;
}

//! Rotate about view direction
bool MeshWidget::rotRoll( double rAngle //!< roll angle in degree
	) {
	// DEPRECATED using OpenGL 3.2 and greater
	//double transArr[16];
	//glGetDoublev( GL_MODELVIEW_MATRIX, transArr );
	//Vector3D camRoll( -transArr[2], -transArr[6], -transArr[10], 0.0 );
	Vector3D camRoll( -mMatModelView(2,0), -mMatModelView(2,1), -mMatModelView(2,2), 0.0 );
	rotArbitAxis( mCenterView, camRoll, rAngle );
	return true;
}

bool MeshWidget::rotRollPitchYaw(double rAngle, double pAngle, double yAngle)
{
    Vector3D camUp( mMatModelView(1,0), mMatModelView(1,1), mMatModelView(1,2), 0.0 );
    Vector3D camPitch( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 );
    Vector3D camRoll( -mMatModelView(2,0), -mMatModelView(2,1), -mMatModelView(2,2), 0.0 );

    Matrix4D yawMat( mCenterView, camUp, yAngle*M_PI/180.0 );
    Matrix4D rollMat( mCenterView, camRoll, rAngle*M_PI/180.0 );
    Matrix4D pitchMat( mCenterView, camPitch, pAngle*M_PI/180.0 );

    pitchMat *= rollMat;
    yawMat *= pitchMat;

    mCameraCenter *= yawMat;
    mCameraUp     *= yawMat;

    setView();
    //repaint();
    update();
    return true;
}

//! Rotate the camera in an orthogonal position to the mesh plane
//! and use the point of intersection as new rotation center.
void MeshWidget::rotOrthoPlane() {
	// Sanity:
	if( mMeshVisual == nullptr ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No mesh present!" << endl;
		return;
	}
	Vector3D planeHNF;
	if( !mMeshVisual->Mesh::getPlaneHNF( &planeHNF ) ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] No valid plane present!" << endl;
		return;
	}

	//! Intersect view direction (roll axis) and plane to determine the point of rotation.
	const float* matProj = mMatModelView.constData();
	Vector3D camRoll( -matProj[2], -matProj[6], -matProj[10], 0.0 );
	// old school:
	//double transArr[16];
	//glGetDoublev( GL_MODELVIEW_MATRIX, transArr );
	//Vector3D camRoll( -transArr[2], -transArr[6], -transArr[10], 0.0 );

	Vector3D rayIntersect;
	if( !mMeshVisual->getPlaneIntersectLineDir( &mCameraCenter, &camRoll, &rayIntersect ) ) {
		cout << "[MeshWidget::" << __FUNCTION__ << "] Can not intersect plane and view direction! Parallel?" << endl;
		return;
	}
	//! Set the point of rotation as mCenterView.
	mCenterView.set( rayIntersect );

	//! Determine the rotation axis: camRoll x Normal of the plane.
	Vector3D rotAxis = planeHNF % camRoll;
	//! Determine the rotation angle betweenn camRoll and the normal of the plane.
	double rotAngle = angle( camRoll, planeHNF, rotAxis ) * 180.0/M_PI;
	//! Avoid rotations of the view with angles larger than 90° as it will turn around the object, which will happen when the plane's normal is pointing aways from the camera.
	if( rotAngle > 90.0f ) {
		rotAngle -= 180.0f;
	}
	if( rotAngle < -90.0f ) {
		rotAngle += 180.0f;
	}
	//! Finally rotate the view.
	rotArbitAxis( mCenterView, rotAxis, -rotAngle );
}

//! Rotate about view direction
void MeshWidget::rotArbitAxis( Vector3D rCenter, //!< arbitrary rotation axis, position
	                       Vector3D rAxis,   //!< arbitrary rotation axis, direction
	                       double   rAngle   //!< roll angle in degree
	) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( rAngle == 0.0 ) {
		// than there is no rotation at all.
		return;
	}
	Matrix4D transMat( rCenter, rAxis, rAngle*M_PI/180.0 );
    mCameraCenter *= transMat;
	mCameraUp     *= transMat;
	setView();
    repaint(); //! \todo check if "repaint" can be replaced by "update" for performance reasons. However if this method is called for e.g. rendering image stacks "repaint" has to be used.
    //update();
}

//! Rotate the mesh plane left/right by angle
bool MeshWidget::rotPlaneYaw(double rAngle)
{
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
    cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif

    Matrix4D transMatLeftRight( mCenterView, mCameraUp, -rAngle*M_PI/180.0 );
    emit sApplyTransfromToPlane( transMatLeftRight );
    setView();
    update();
    return true;
}

//! Rotate the mesh plane up/down by angle
bool MeshWidget::rotPlanePitch(double rAngle)
{
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
    cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
    Vector3D cameraPitchAxis( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 );
    Matrix4D transMatUpDown( mCenterView, cameraPitchAxis, -rAngle*M_PI/180.0 );
    emit sApplyTransfromToPlane( transMatUpDown );
    setView();
    update();
    return true;
}

//! Rotate the mesh plane clockwise/counterclockwise by angle
bool MeshWidget::rotPlaneRoll(double rAngle)
{
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
    Vector3D camRollAxis( -mMatModelView(2,0), -mMatModelView(2,1), -mMatModelView(2,2), 0.0 );
    Matrix4D transMatRot(mCenterView, camRollAxis, -rAngle*M_PI/180.0);
    emit sApplyTransfromToPlane( transMatRot );
    setView();
    update();
    return true;
}

// ============================================================================

// OpenGL Stuff ----------------------------------------------------------------

//! Re-draws the OpenGL widget, when its size has changed by calling setView().
void MeshWidget::resizeGL( [[maybe_unused]]int width , [[maybe_unused]]int height  ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "] width: " << width << " height: " << height << endl;
#endif
	setView();
	update();
}

//! This function is called whenever the widget needs to be painted. It is a reimplementation
//! in a subclass. There is no need to call makeCurrent() because this has already been done
//! when this function is called.
//!
//! Remarks: moved from paintGL to paintEvent.
//! paintEvent is called when update() or repaint() are called.
//!
//! About QShaderProgram and rendertext:
//! Using renderText() in this setting is not supported. renderText() depends heavily upon the old, fixed function GL pipeline, and simply won't work if you e.g. have an active shader program. We are in the process of deprecating this function all together, since there are many settings where it won't work as expected. It will work if you stick to GL 1.x or GL 2.x functionality, and disable any current shader program. As for now I'd recommend you draw the text to a texture (e.g. via a framebuffer object), and then draw that texture as part of your paintGL() function. You can for instance use QPainter to draw the text into the framebuffer object bound texture.
//! Source: https://bugreports.qt-project.org/browse/QTBUG-10362
//! 
//! Thanks to OpenGL CoreProfile also the QPainter is rendered unuseable. So the informations are provided in a regular widget OUTSIDE the OpenGL context.
//!
//! Actions performed:
void MeshWidget::paintEvent( QPaintEvent *rEvent ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	PRINT_OPENGL_ERROR( "OLD_ERROR" );

	QGLWidget::paintEvent( rEvent );
	PRINT_OPENGL_ERROR( "QGLWidget::paintEvent( rEvent )" );

	//! Initialize shaders (ONCE!)
    if( mVAO == _NOT_A_NUMBER_UINT_ ) {
		initializeVAO();
		initializeShaders();
	}

	//! Clear color buffer.
	glClear( GL_COLOR_BUFFER_BIT );
	PRINT_OPENGL_ERROR( "glClear( GL_COLOR_BUFFER_BIT )" );

	//! Clear depth buffer.
	glClear( GL_DEPTH_BUFFER_BIT );
	PRINT_OPENGL_ERROR( "glClear( GL_DEPTH_BUFFER_BIT )" );

	if( mMeshVisual == nullptr ) {
		//! Do nothing, when no mesh is present.
		//cerr << "{MeshWidget::" << __FUNCTION__ << "] ERROR: Mesh not present!" << endl;
		swapBuffers(); // has to be called, when setAutoBufferSwap(false) was set in the constructor to preven flickering when QPainter.end is called!
		return;
	}

	//! Enable OpenGL depth test.
	glEnable( GL_DEPTH_TEST );
	//! Setup Alpha blendig for OpenGL.
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_BLEND );
    //! Draw the Mesh.
    mMeshVisual->glPaint();

	//! Optional; draw RGB histogram of the scene. Highly recommended to be calle before paintScreenInfo, paintOrthoGrid and paintHistogram!
	paintHistogramScence(); // This sould be first -- otherwise unwanted elements like logos are accounted to (this color) histogram.

	//! Draw (optional) Grid(s) in orthographic and perspective projection. The latter may give the impression of scale, but can not be used as such!
	glBlendFunc( GL_DST_COLOR, GL_ZERO ); // Allows for a nice overlay.
	// Background canvas using a VBOs and shaders
	bool showGridRect;
	bool showGridPolarLines;
	bool showGridPolarCircles;
	bool showGridHighLightCenter;
	getParamFlagMeshWidget( SHOW_GRID_RECTANGULAR, &showGridRect );
	getParamFlagMeshWidget( SHOW_GRID_POLAR_LINES, &showGridPolarLines );
	getParamFlagMeshWidget( SHOW_GRID_POLAR_CIRCLES, &showGridPolarCircles );
	getParamFlagMeshWidget( SHOW_GRID_HIGHLIGHTCENTER, &showGridHighLightCenter );
	if( showGridRect ) {
		paintBackgroundShader( &mShaderGridOrtho );
	}
	if( showGridPolarLines ) {
		paintBackgroundShader( &mShaderGridPolarLines );
	}
	if( showGridPolarCircles ) {
		paintBackgroundShader( &mShaderGridPolarCircles );
	}
	if( ( showGridRect || showGridPolarLines || showGridPolarCircles ) && showGridHighLightCenter ) {
		paintBackgroundShader( &mShaderGridHighLightCenter );
	}


	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ); // Set to the de-facto default.

    mMeshVisual->glPaintTransparent(); //draw transparent faces. Necessary to do it after the background has been drawn, but before overlays

	// Render section planes
	renderAllSectionPlanes();
	renderSectionPolylines();

	// Face culling conflicts with QPainter. Therefore it is turned of and the flag state stored to turn it back on, after painting is done.
	// ... and QPainter can't be used with OpenGL CoreProfile
	//GLboolean cullFace;
	//glGetBooleanv( GL_CULL_FACE, &cullFace );
	//glDisable( GL_CULL_FACE );

	//! Draw (optional) Histogram of a mesh property.
	paintHistogram();

	//! Draw the selection of polygonal area.
	paintSelection();


	mMeshVisual->glPaintOverlay();

	//! Draw the GigaMesh logo.
	bool showGigaMeshLogo = true;
	bool showGigaMeshLogoForced = true;
	getParamFlagMeshWidget( SHOW_GIGAMESH_LOGO_CANVAS, &showGigaMeshLogo );
	getParamFlagMeshWidget( SHOW_GIGAMESH_LOGO_FORCED, &showGigaMeshLogoForced );
	if( showGigaMeshLogo | showGigaMeshLogoForced ) {
		paintRasterImage( TEXMAP_GIGAMESH_LOGO, -20, -20, 150, 150 );
	}

	//! Draw the keyboard layout.
	bool showKeyboardCamera = false;
	getParamFlagMeshWidget( SHOW_KEYBOARD_CAMERA, &showKeyboardCamera );
	if( showKeyboardCamera ) {
		int layoutImWidth  =  768;
		int layoutImHeigth =  320;
		paintRasterImage( TEXMAP_KEYBOARD_LAYOUT,
		                  max( (width()-layoutImWidth)/2, 20 ), 20,
		                  layoutImWidth, layoutImHeigth );
	}

	//! Compute frames per second and pass it to the sidebar.
	float framesPerSecond = 0.0f;
	if ( ( mFrameCount == 30 ) || ( mFrameCount == 0 ) ) {
		mFrameTime.start();
		mFrameCount = 0;
	} else {
		framesPerSecond = float(1000*mFrameCount) / mFrameTime.elapsed();
	}
	mFrameCount++;
	emit sViewPortInfo( VPINFO_FRAMES_PER_SEC, QString::number( framesPerSecond, 'f', 2 ) );


	//! \todo move to changedSelPrim
	// Show selection:
	Primitive* primSelected = mMeshVisual->getPrimitiveSelected();
	if( primSelected != nullptr ) {
		uint64_t labelNr;
		if( primSelected->getLabel( labelNr ) ) {
			emit sViewPortInfo( VPINFO_LABEL_ID, QString::number( labelNr ) );
		} else {
			emit sViewPortInfo( VPINFO_LABEL_ID, QString( "not set" ) );
		}
		double funcVal;
		if( primSelected->getFuncValue( &funcVal ) ) {
			emit sViewPortInfo( VPINFO_FUNCTION_VALUE, QString::number( funcVal ) );
		} else {
			emit sViewPortInfo( VPINFO_FUNCTION_VALUE, QString( "err" ) );
		}
	} else {
		emit sViewPortInfo( VPINFO_FUNCTION_VALUE, QString( "n.a." ) );
		emit sViewPortInfo( VPINFO_LABEL_ID, QString( "n.a." ) );
	}


	// Example for adding text using QPainter:
	// NOTE: that this will NOT WORK with OpenGL CoreProfile!
	//--------------------------------------------------------
	//QPainter painter;
	//painter.begin(this);
	//painter.setRenderHint(QPainter::Antialiasing);
	//painter.setPen(QPen(Qt::red));
	//QFont textFont;
	//textFont.setPixelSize(50);
	//painter.setFont(textFont);
	//painter.drawText(QRect(50, 50, 500, 100), Qt::AlignCenter, QString("TestPaint"));
	//painter.end();

	// DongArch Section Overlay (Arch3D Liner style)
	// Paint section cutline as 2D overlay on top of 3D scene
	if (m_showSectionOverlay && m_sectionPolylineScreen.size() >= 2) {
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing, true);

		QColor lineColor = m_sectionLineColor;
		lineColor.setAlphaF(m_sectionLineAlpha);
		QPen pen(lineColor, m_sectionLineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		painter.setPen(pen);

		for (int i = 1; i < m_sectionPolylineScreen.size(); ++i) {
			painter.drawLine(m_sectionPolylineScreen[i-1], m_sectionPolylineScreen[i]);
		}

		painter.end();
		qDebug() << "[MeshWidget::paintEvent] Drew section overlay with" << m_sectionPolylineScreen.size() << "points";
	}

	// Turn on backface culling, when disabled because of QPainter
	// ... and QPainter can't be used with OpenGL CoreProfile
	//if( cullFace ) {
	//	glEnable( GL_CULL_FACE );
	//}

	swapBuffers(); // has to be called, when setAutoBufferSwap(false) was set in the constructor to preven flickering when QPainter.end is called!
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "] DONE." << endl;
#endif
}

//! Paint the background with a given shader.
//! @returns false in case of an error. true otherwise.
bool MeshWidget::paintBackgroundShader( QOpenGLShaderProgram** rShaderProgram ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	PRINT_OPENGL_ERROR( "OLD ERROR" );

	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(context()->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	// Lets be shady :)
	if( !(*rShaderProgram)->bind() ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
		return false;
	}

	GLboolean oldDepthMask = true;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &oldDepthMask);
	glDepthMask(false);

	double realWidth;
	double realHeight;
	if( !getViewPortResolution( realWidth, realHeight ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getViewPortResolution failed!" << endl;
		return false;
	}
	//cout << "PW: " << pixelWidth << " PH: " << pixelHeight << endl;
	double gridShiftDepth;
	bool   gridHighLightCenter;
	getParamFloatMeshWidget( GRID_SHIFT_DEPTH, &gridShiftDepth );
	getParamFlagMeshWidget( SHOW_GRID_HIGHLIGHTCENTER, &gridHighLightCenter );
	(*rShaderProgram)->setUniformValue( "uDepthPos", static_cast<GLfloat>(gridShiftDepth)   );
	(*rShaderProgram)->setUniformValue( "uScaleX",   static_cast<GLfloat>(realWidth/2.0)  );
	(*rShaderProgram)->setUniformValue( "uScaleY",   static_cast<GLfloat>(realHeight/2.0) );

	bool gridCenterFront;
	getParamFlagMeshWidget( SHOW_GRID_HIGHLIGHTCENTER_FRONT, &gridCenterFront);
	(*rShaderProgram)->setUniformValue( "uHighlightDepth", (gridCenterFront ? 0.0F : 0.999F ));

	double xOffset = 0.0;
	double yOffset = 0.0;
	getGridCenterPosOffsets( xOffset, yOffset );

	(*rShaderProgram)->setUniformValue( "uCenterOffset", xOffset, yOffset);
	PRINT_OPENGL_ERROR( "setUniformValue" );

	// Strided Data -- map buffer
	if( !mVertBufObjs[VBO_BACKGROUND_VERTICES].bind() ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Could not bind vertex buffer VBO_BACKGROUND_VERTICES to the context!" << endl;
	}
	// Strided data -- first there floats are the position vectors.
	(*rShaderProgram)->setAttributeBuffer( "vertPosition", GL_FLOAT, 0, 2, 0 );
	PRINT_OPENGL_ERROR( "setAttributeBuffer" );
	(*rShaderProgram)->enableAttributeArray( "vertPosition" );
	PRINT_OPENGL_ERROR( "enableAttributeArray" );

	//mVertBufObjs[VBO_BACKGROUND_QUAD].bind();
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	PRINT_OPENGL_ERROR( "glDrawArrays( GL_TRIANGLE_FAN, 0, 4 )" );

	// End of being shady
	//(*rShaderProgram)->disableAttributeArray( "vertPosition" );
	(*rShaderProgram)->release();
	mVertBufObjs[VBO_BACKGROUND_VERTICES].release();

	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );

	glDepthMask(oldDepthMask);
	return true;
}

//! Paint a raster image i.e. a logo.
bool MeshWidget::paintRasterImage( eTextureMaps rTexMap, int rPixelX, int rPixelY, int rPixelWidth, int rPixelHeight ) {
	PRINT_OPENGL_ERROR( "OLD ERROR" );
	PglBindVertexArray glBindVertexArray = reinterpret_cast<PglBindVertexArray>(context()->getProcAddress( "glBindVertexArray" ));
	glBindVertexArray( mVAO );
	PRINT_OPENGL_ERROR( "glBindVertexArray( mVAO )" );

	// Lets be shady :)
	if( !mShaderImage->bind() ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: binding shader program!" << endl;
		return false;
	}

	using PglActiveTexture = void (*)(GLenum);
	PglActiveTexture activeTextureFunc = reinterpret_cast<PglActiveTexture>(context()->getProcAddress("glActiveTexture"));

	activeTextureFunc(GL_TEXTURE0);
	mTextureMaps[rTexMap]->bind();
	//mTextureMaps[rTexMap]->
	GLuint texId = mTextureMaps[rTexMap]->boundTextureId( QOpenGLTexture::BindingTarget2D );
	//cout << "[MeshWidget::" << __FUNCTION__ << "] texId: " << texId << endl;
	// Set the ID of the texture map:
	mShaderImage->setAttributeValue( "uLabelTexMap", texId );

	double realWidth;
	double realHeight;
	if( !getViewPortResolution( realWidth, realHeight ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getViewPortResolution failed!" << endl;
		return false;
	}
	//cout << "PW: " << pixelWidth << " PH: " << pixelHeight << endl;
	mShaderImage->setUniformValue( "uDepthPos", static_cast<GLfloat>(-1.999)   ); // 1.99 for front

	float ratioViewPort = static_cast<float>(width())/static_cast<float>(height());

	float widthViewPort  = 2.0*static_cast<float>(rPixelWidth)/static_cast<float>(width()); // max: 2.0
	float heightViewPort = 2.0*ratioViewPort*static_cast<float>(rPixelHeight)/static_cast<float>(width()); // max: 2.0

	float xPosViewPort;
	float yPosViewPort;
	if( rPixelX > 0 ) {
		xPosViewPort = 2.0*static_cast<float>(rPixelX)/static_cast<float>(width()) - 1.0;// min: -1.0;
	} else {
		xPosViewPort = 1.0 + 2.0*static_cast<float>(rPixelX)/static_cast<float>(width()) - widthViewPort;// min: -1.0;
	}
	if( rPixelY > 0 ) {
		yPosViewPort = 2.0*ratioViewPort*static_cast<float>(rPixelY)/static_cast<float>(width()) -1.0;// min: -1.0;
	} else {
		yPosViewPort = 1.0 + 2.0*ratioViewPort*static_cast<float>(rPixelY)/static_cast<float>(width()) - heightViewPort;// min: -1.0;
	}
	//cout << xPosViewPort << " | " << yPosViewPort << endl;

	// Strided Data -- map buffer {x,y,u,v}
	vector<GLfloat> coords;

	// bottom, left
	coords.push_back( xPosViewPort ); // x-pos
	coords.push_back( yPosViewPort ); // y-pos
	coords.push_back( 0.0 );  // u
	coords.push_back( 0.0 );  // v

	// top, left
	coords.push_back( xPosViewPort+widthViewPort ); // x-pos
	coords.push_back( yPosViewPort ); // y-pos
	coords.push_back( 1.0 );  // u
	coords.push_back( 0.0 );  // v

	// top, right
	coords.push_back( xPosViewPort+widthViewPort ); // x-pos
	coords.push_back( yPosViewPort+heightViewPort ); // y-pos
	coords.push_back( 1.0 );  // u
	coords.push_back( 1.0 );  // v

	// bottom, right
	coords.push_back( xPosViewPort ); // x-pos 
	coords.push_back( yPosViewPort+heightViewPort ); // y-pos
	coords.push_back( 0.0 );  // u
	coords.push_back( 1.0 );  // v

	QOpenGLBuffer someBuffer( QOpenGLBuffer::VertexBuffer );
	someBuffer.create();
	someBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
	someBuffer.bind();
	someBuffer.allocate( coords.data(), static_cast<int>(sizeof(GLfloat)*coords.size()) );

	// Strided data -- first there floats are the position vectors.
	mShaderImage->setAttributeBuffer( "vertPosition", GL_FLOAT, 0, 2, sizeof(GLfloat)*4 );
	mShaderImage->enableAttributeArray( "vertPosition" );
	mShaderImage->setAttributeBuffer( "textureCoords", GL_FLOAT, sizeof(GLfloat)*2, 2, sizeof(GLfloat)*4 );
	mShaderImage->enableAttributeArray( "textureCoords" );

	//mVertBufObjs[VBO_BACKGROUND_QUAD].bind();
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	PRINT_OPENGL_ERROR( "glDrawArrays( GL_TRIANGLE_FAN, 0, 4 )" );

	// End of being shady
	mShaderImage->release();

	someBuffer.release();
	glBindVertexArray( 0 );
	PRINT_OPENGL_ERROR( "glBindVertexArray( 0 )" );	
	someBuffer.destroy();
	return true;
}

void MeshWidget::checkMissingTextures(ModelMetaData& metadata)
{
	if(metadata.hasTextureFiles())
	{
		QStringList missingTextures;
		size_t texId = 0;
		std::list<size_t> missingTexIds;
		for(const auto& texName : metadata.getTexturefilesRef())
		{
			if(!std::filesystem::exists(texName))
			{
				missingTextures.push_back(texName.string().c_str());
				missingTexIds.push_back(texId);
			}
			++texId;
		}

		if(missingTexIds.empty())
			return;

		DialogFindTextures textureDialog(missingTextures);

		textureDialog.setModal(true);
		if(textureDialog.exec() == QDialog::Accepted)
		{
			auto fileNames = textureDialog.getFileNames();

			if(fileNames.size() != static_cast<int>(missingTexIds.size()))
				return;

			auto fileNameIt = fileNames.begin();
			auto idIt       = missingTexIds.begin();

			for(int i = 0; i<fileNames.size(); ++i)
			{
				metadata.getTexturefilesRef()[*idIt] = fileNameIt->toStdString();
				++fileNameIt;
				++idIt;
			}
		}
	}
}

void MeshWidget::checkMeshSanity()
{
	const auto meshSize = mMeshVisual->getBoundingBoxRadius();
	const auto meshCenterDistance = mMeshVisual->getBoundingBoxCenter().getLength3();

	int exponent = 0.0F;

	//! TODO: find out resonable value. Currently, assume that the float should have at least 4 binary decimal places
	if(std::numeric_limits<float>::digits - exponent < 4)
	{
		bool move = false;
		bool cancel = false;
		SHOW_QUESTION(tr("Center Mesh"), tr("The mesh is very far off center compared to its radius(distance: %1mm). This may cause numeric problems. Would you like to move the mesh to the origin?").arg(meshCenterDistance),
					  move, cancel);

		if(!cancel && move)
		{
			const auto transVector = mMeshVisual->getBoundingBoxCenter();

			Matrix4D transMat(transVector);

			mMeshVisual->applyTransformationToWholeMesh(transMat);

			SHOW_MSGBOX_INFO(tr("Transfrom Vector"), tr("To move the mesh back to its original position, you can translate it back by the following vector:\n(%1, %2, %3)").
							 arg(transVector.getX()).
							 arg(transVector.getY()).
							 arg(transVector.getZ()));
		}
	}

	//! TODO: find out resonable value
	if(meshSize < 1.0)
	{
		bool rescale = false;
		bool cancel = false;
		SHOW_QUESTION(tr("Rescale Mesh"), tr("The mesh appears to be unusually small (radius: %1mm). Would you like to scale the Mesh up?").arg(meshSize), rescale, cancel);

		if(!cancel && rescale)
		{
			mMeshVisual->callFunction(MeshParams::APPLY_TRANSMAT_ALL_SCALE);
		}
	}

	checkMissingTextures(mMeshVisual->getModelMetaDataRef());
}

//! Paint the selected volume (prisms defined by polgonal selection).
//! This method is also an example for a 2D overlay on a 3D scene.
void MeshWidget::paintSelection() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( mSelectionPoly.size() == 0 ) {
		return;
	}
	eMouseModes mouseMode;
	getParamIntegerMeshWidget( MOUSE_MODE, reinterpret_cast<int*>(&mouseMode) );
	if( mouseMode != MOUSE_MODE_SELECT ) {
		return;
	}
	MeshWidgetParams::eSelectionModes selectionMode;
	getParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, reinterpret_cast<int*>(&selectionMode) );
    if( !(selectionMode == MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO || selectionMode == MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO) ) {
		return;
	}

	QImage imPoly( width(), height(), QImage::Format_ARGB32 );
	imPoly.fill( QColor( 255, 255, 255, 0 ) );

	QPen somePen( QColor( 255, 0, 0 ) );
	somePen.setWidthF( 2.5 );

	QPainter painter;
	painter.begin( &imPoly );
	painter.setRenderHint( QPainter::Antialiasing );
	painter.setPen( somePen );
	painter.drawPolyline( mSelectionPoly.data(), static_cast<int>(mSelectionPoly.size()) );
	somePen.setStyle( Qt::DashLine );
	painter.setPen( somePen );
	painter.drawLine( mSelectionPoly.front(), mSelectionPoly.back() );
	painter.end();
	// Paint the image using a shader.
    initializeTextureMap( TEXMAP_SELECTION_POLYGON_OVERLAY, &imPoly );
	paintRasterImage( TEXMAP_SELECTION_POLYGON_OVERLAY, 0, 0, width(), height() );
}

//! Overlay a histogram to the GL scene
bool MeshWidget::paintHistogram() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Show and only when enabled.
	if( !mParamFlag[SHOW_HISTOGRAM] ) {
		return false;
	}
	// Sanity check
	if( mMeshVisual == nullptr ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No mesh present!" << endl;
		return false;
	}

	// Nothing to much do, when it is already computed.
	int imWidth  = mParamInt[HISTOGRAM_WIDTH];
	int imHeight = mParamInt[HISTOGRAM_HEIGHT];

	double valMin;
	double valMax;
	vector<unsigned int> numArray;
	numArray.resize( (imWidth-2), 0 );
	if( !mMeshVisual->getHistogramValues( static_cast<Mesh::eHistogramType>(mParamInt[HISTOGRAM_TYPE]), &numArray, &valMin, &valMax ) ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: from getHistogramValues!" << endl;
		return false;
	}
	// Determine maximim bin height:
	unsigned int binMax = 0;
	for( unsigned int element : numArray ) {
		if( binMax < element ) {
			binMax = element;
		}
	}
	// Optional logarithm:
	if( mParamFlag[SHOW_HISTOGRAM_LOG] ) {
		double fact = (static_cast<double>(imHeight)-2.0)/log( static_cast<double>(imHeight)-2.0 );
		for( unsigned int& element : numArray ) {
			//		cout << element << " ";
			element = log( ( static_cast<double>(element)+1.0 ) )*fact;
			//		cout << element << endl;
		}
		binMax = log( ( static_cast<double>(binMax)+1.0 ) )*fact;
	}

	if( binMax == 0 ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: binMax is ZERO!" << endl;
		return false;
	}

	QImage imHist( imWidth, imHeight, QImage::Format_RGB32 );
	imHist.fill( QColor( 255, 255, 255 ) );

	QPainter painter;
	painter.begin( &imHist );
	painter.drawRect( 0, 0, imWidth-1, imHeight-1 );
	// Draw vertical bins:
	for( int pixelX=1; pixelX<(imWidth-1); pixelX++ ) {
		unsigned int binSize = (imHeight-2) - (numArray.at( pixelX-1 )*(imHeight-2))/binMax;
		painter.setPen( QColor( 128, 128, 128, 255 ) );
		painter.drawLine( pixelX, imHeight-1, pixelX, binSize+1 );
	}
	painter.end();
	// Paint the image using a shader.
	initializeTextureMap( TEXMAP_HISTOGRAM_MESH_FUNCVAL, &imHist );
	paintRasterImage( TEXMAP_HISTOGRAM_MESH_FUNCVAL, mParamInt[HISTOGRAM_POSX], mParamInt[HISTOGRAM_POSY], imWidth, imHeight );
	return true;
}

//! Shows the RGB histogram of the scene.
bool MeshWidget::paintHistogramScence() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( !mParamFlag[SHOW_HISTOGRAM_SCENE] ) {
		return false;
	}
	GLsizei  scWidth   = width();
	GLsizei  scHeight  = height();

	unsigned int numArrayRed[256];
	unsigned int numArrayGrn[256];
	unsigned int numArrayBle[256];
	for( size_t i=0; i<256; i++ ) {
		numArrayRed[i] = 0;
		numArrayGrn[i] = 0;
		numArrayBle[i] = 0;
	}

	{ // Block for reading the scene's colors:
		GLfloat* pixelZBuffer = new GLfloat[scWidth*scHeight];;
		glReadPixels( 0, 0, scWidth, scHeight, GL_DEPTH_COMPONENT, GL_FLOAT, pixelZBuffer );
		// OpenGL's default 4 byte pack alignment would leave extra bytes at the
		// end of each image row so that each full row contained a number of bytes
		// divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit components would
		// be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
		// just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
		// the rows are packed as tight as possible (no row padding), set the pack
		// alignment to 1.
		GLubyte* imArrayGL    = new GLubyte[scWidth*3*scHeight];
		glPixelStorei( GL_PACK_ALIGNMENT, 1 );
		glReadPixels( 0, 0, scWidth, scHeight, GL_RGB, GL_UNSIGNED_BYTE, imArrayGL );
		for( int i=0; i<scWidth*scHeight; i++ ) {
			if( pixelZBuffer[i] == 1.0 ) {
				continue;
			}
			unsigned int idxRed = imArrayGL[i*3];
			unsigned int idxGrn = imArrayGL[i*3+1];
			unsigned int idxBle = imArrayGL[i*3+2];
			numArrayRed[idxRed]++;
			numArrayGrn[idxGrn]++;
			numArrayBle[idxBle]++;
		}
		delete[] imArrayGL;
		delete[] pixelZBuffer;
	}

	unsigned int imWidth  = 258;
	unsigned int imHeight = 102;

	// Determine maximim bin height:
	unsigned int binMax = 0;
	for( unsigned int i=0; i<256; i++ ) {
		if( binMax < numArrayRed[i] ) {
			binMax = numArrayRed[i];
		}
		if( binMax < numArrayGrn[i] ) {
			binMax = numArrayGrn[i];
		}
		if( binMax < numArrayBle[i] ) {
			binMax = numArrayBle[i];
		}
	}
	if( binMax == 0 ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: binMax is ZERO!" << endl;
		return false;
	}

	// Allocate image
	QImage imHist( imWidth, imHeight, QImage::Format_RGB32 );
	imHist.fill( QColor( 64, 64, 64 ) );

	// Optional logarithm:
	if( mParamFlag[SHOW_HISTOGRAM_SCENE_LOG] ) {
		double fact = (static_cast<double>(imHeight)-2.0)/log( static_cast<double>(imHeight)-2.0 );
		for( unsigned int i=0; i<256; i++ ) {
			numArrayRed[i] = log( ( static_cast<double>(numArrayRed[i])+1.0 ) )*fact;
			numArrayGrn[i] = log( ( static_cast<double>(numArrayGrn[i])+1.0 ) )*fact;
			numArrayBle[i] = log( ( static_cast<double>(numArrayBle[i])+1.0 ) )*fact;
		}

		binMax = log( ( static_cast<double>(binMax)+1.0 ) )*fact;
	}

	// Draw vertical bins:
	for( unsigned int pixelX=1; pixelX<(imWidth-1); pixelX++ ) {
		unsigned int binSizeRed = (imHeight-2) - (numArrayRed[pixelX-1]*(imHeight-2))/binMax;
		unsigned int binSizeGrn = (imHeight-2) - (numArrayGrn[pixelX-1]*(imHeight-2))/binMax;
		unsigned int binSizeBle = (imHeight-2) - (numArrayBle[pixelX-1]*(imHeight-2))/binMax;
		//cout << binSizeRed << endl;
		for( unsigned int iy=1; iy<(imHeight-1); iy++ ) {
			QColor pixelCol( imHist.pixel( pixelX, iy ) );
			if( iy > binSizeRed ) {
				pixelCol.setRed( 255 );
			}
			if( iy > binSizeGrn ) {
				pixelCol.setGreen( 255 );
			}
			if( iy > binSizeBle ) {
				pixelCol.setBlue( 255 );
			}
			imHist.setPixel( pixelX, iy, pixelCol.rgb() );
		}
	}
	// Paint the image using a shader.
	initializeTextureMap( TEXMAP_HISTOGRAM_SCENE, &imHist );
	paintRasterImage( TEXMAP_HISTOGRAM_SCENE, -mParamInt[HISTOGRAM_POSX], mParamInt[HISTOGRAM_POSY], imWidth, imHeight );

	return true;
}

//! Handles the event when a mouse button is pressed down.
void MeshWidget::mousePressEvent( QMouseEvent *rEvent ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// HighDPI Support
	double scaleFactor = 1.0;
	getParamFloatMeshWidget( HIGHDPI_ZOOM_FACTOR, &scaleFactor );

	mLastPos = rEvent->pos() * scaleFactor;

	Qt::MouseButtons mouseButtonsPressed = rEvent->buttons();

	eMouseModes currMouseMode;
	getParamIntegerMeshWidget( MOUSE_MODE, reinterpret_cast<int*>(&currMouseMode) );

	MeshWidgetParams::eSelectionModes currSelectionMode;
	getParamIntegerMeshWidget( SELECTION_MODE, reinterpret_cast<int*>(&currSelectionMode) );

	//! Cortex selection mode - intercept left clicks for cortex vertex picking
	if( mCortexSelectionModeActive && (mouseButtonsPressed == Qt::LeftButton) ) {
		selectCortexVertexAt( mLastPos.x(), mLastPos.y() );
		return;
	}

	//! Selection of a point of a polyline (left click):
	if( ( mouseButtonsPressed == Qt::LeftButton ) &&
	    ( currMouseMode == MOUSE_MODE_SELECT ) &&
        ( currSelectionMode == MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO || currSelectionMode == MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO )
	  ) {
		mSelectionPoly.push_back( mLastPos );
		if( mSelectionPoly.size() == 1 ) {
			mSelectionPoly.push_back( mLastPos );
		}
		return;
	}

	//! Close the selection of a polyline by right-click:
	if( ( mouseButtonsPressed == Qt::RightButton ) &&
	    ( currMouseMode == MOUSE_MODE_SELECT ) &&
        ( currSelectionMode == MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO || MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO ) &&
	    ( mSelectionPoly.size() > 1 )
	  ) {
		// Check if the last two points are the same, as this can cause a segmentation fault.
		QPoint posLast     = mSelectionPoly.at( mSelectionPoly.size()-1 );
		QPoint posPrevLast = mSelectionPoly.at( mSelectionPoly.size()-2 );
		if( posLast == posPrevLast ) {
			mSelectionPoly.pop_back();
		}
		// Correct for different origin in image coordinates.
		for( auto & somePoint: mSelectionPoly ) {
			somePoint.setY( height() - somePoint.y() );
		}
        if ( currSelectionMode == MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO){
            emit sSelectPoly( mSelectionPoly );
        }
        if( currSelectionMode == MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO ){
            emit sDeSelectPoly( mSelectionPoly );
        }
		mSelectionPoly.clear();
		return;
	}

	//! Section plane creation by dragging (Archaeological illustration mode)
	if( ( mouseButtonsPressed == Qt::LeftButton ) &&
	    ( currMouseMode == MOUSE_MODE_SELECT ) &&
	    ( currSelectionMode == MeshWidgetParams::SELECTION_MODE_SECTION_DRAG )
	  ) {
		// Get 3D point on mesh from screen coordinates
		if( mMeshVisual != nullptr ) {
			GLint viewport[4];
			glGetIntegerv( GL_VIEWPORT, viewport );
			const int yPixel = viewport[3] - mLastPos.y();
			const int xPixel = mLastPos.x();
			Vector3D clickPos;

			if( mMeshVisual->getWorldPointOnMesh( xPixel, yPixel, &clickPos ) ) {
				if( mSectionDragPoints.empty() ) {
					// First point: start dragging
					mSectionDragPoints.push_back( clickPos );
					std::cout << "[MeshWidget] Section drag started at: " << clickPos << std::endl;
				}
			}
		}
		return;
	}

	//! Selection other than POLYLINE
	if( ( mouseButtonsPressed & ( Qt::LeftButton | Qt::MiddleButton | Qt::RightButton ) ) &&
	    ( currMouseMode == MOUSE_MODE_SELECT )
	  ) {
        if( currSelectionMode == MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO || currSelectionMode == MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO ) {
			std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: Wrong selection mode (SELECTION_MODE_POLYLINE)!" << std::endl;
			return;
		}
		userSelectByMouseClick( mLastPos, mouseButtonsPressed );
		return;
	}
}

//! Double click event of the mouse, which will set the point of rotation
//! (lookAt) for the virtual camera.
void MeshWidget::mouseDoubleClickEvent( QMouseEvent* rEvent )
{
	// Sanity check
	if( mMeshVisual == nullptr ) {
		return;
	}

	// HighDPI Support
	double scaleFactor = 1.0;
	getParamFloatMeshWidget( HIGHDPI_ZOOM_FACTOR, &scaleFactor );

	if( rEvent->button() == Qt::LeftButton ) {
		// Correct for OpenGL:
		GLint viewport[4];
		glGetIntegerv( GL_VIEWPORT, viewport );
		const int yPixel = viewport[3] - rEvent->pos().y()*scaleFactor;
		const int xPixel = rEvent->pos().x()*scaleFactor;
		Vector3D clickPos;

		if( mMeshVisual->getWorldPointOnMesh( xPixel, yPixel, &clickPos ) ) {
			const Vector3D transVec = clickPos - mCenterView;
			mCenterView = clickPos;
			mCameraCenter += transVec;
			setView();
			update();
		}
	}
}

//! Handles the event when the mouse button is released again
void MeshWidget::mouseReleaseEvent(QMouseEvent *rEvent)
{
	auto mouseButtonsPressed = rEvent->buttons();
	if( mouseButtonsPressed &
	    ( Qt::LeftButton | Qt::MiddleButton | Qt::RightButton ) ) {
		return;
	}

	//! Finalize section plane creation (Archaeological illustration mode)
	eMouseModes currMouseMode;
	getParamIntegerMeshWidget( MOUSE_MODE, reinterpret_cast<int*>(&currMouseMode) );
	int currSelectionMode;
	getParamIntegerMeshWidget( SELECTION_MODE, &currSelectionMode );

	if( ( currMouseMode == MOUSE_MODE_SELECT ) &&
	    ( currSelectionMode == MeshWidgetParams::SELECTION_MODE_SECTION_DRAG ) &&
	    ( !mSectionDragPoints.empty() )
	  ) {
		// Finalize plane: switch from temp to permanent
		emit sParamFlagMesh( MeshGLParams::SHOW_MESH_PLANE_TEMP, false );
		emit sParamFlagMesh( MeshGLParams::SHOW_MESH_PLANE, true );

		std::cout << "[MeshWidget] Section plane finalized." << std::endl;

		// Clear drag points for next section
		mSectionDragPoints.clear();
		update();
	}

	setParamFlagMeshWidget(MeshWidgetParams::SHOW_MESH_REDUCED, false);
}

//! Handles the event when the mouse is moved.
//! See also MeshWidgetParams::MOUSE_MODE
//! This should do most of the rotation on screen ...
void MeshWidget::mouseMoveEvent( QMouseEvent* rEvent ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// HighDPI Support
	double scaleFactor = 1.0;
	getParamFloatMeshWidget( HIGHDPI_ZOOM_FACTOR, &scaleFactor );

	GLfloat dx = GLfloat( rEvent->x()*scaleFactor - mLastPos.x() ) / 2.0;
	GLfloat dy = GLfloat( rEvent->y()*scaleFactor - mLastPos.y() ) / 2.0;

	mLastPos = rEvent->pos() * scaleFactor;

	eMouseModes currMouseMode;
	getParamIntegerMeshWidget( MOUSE_MODE, reinterpret_cast<int*>(&currMouseMode) );

	int currSelectionMode;
	getParamIntegerMeshWidget( SELECTION_MODE, &currSelectionMode );

	//! Real-time section plane preview (Archaeological illustration mode)
	if( ( rEvent->buttons() == Qt::LeftButton ) &&
	    ( currMouseMode == MOUSE_MODE_SELECT ) &&
	    ( currSelectionMode == MeshWidgetParams::SELECTION_MODE_SECTION_DRAG ) &&
	    ( !mSectionDragPoints.empty() )
	  ) {
		if( mMeshVisual != nullptr ) {
			GLint viewport[4];
			glGetIntegerv( GL_VIEWPORT, viewport );
			const int yPixel = viewport[3] - mLastPos.y();
			const int xPixel = mLastPos.x();
			Vector3D secondPoint;

			if( mMeshVisual->getWorldPointOnMesh( xPixel, yPixel, &secondPoint ) ) {
				// Calculate plane from two points
				Vector3D firstPoint = mSectionDragPoints[0];
				Vector3D midPoint = (firstPoint + secondPoint) * 0.5;
				Vector3D dragDir = (secondPoint - firstPoint);

				// Normal perpendicular to drag direction and camera up (cross product)
				Vector3D normal = dragDir % mCameraUp;
				if( normal.getLength3() > 0.001 ) {
					normal.normalize3();

					// Calculate HNF: (nx, ny, nz, d) where d = -dot(normal, point_on_plane)
					double d = -(normal.getX() * midPoint.getX() +
					             normal.getY() * midPoint.getY() +
					             normal.getZ() * midPoint.getZ());
					Vector3D planeHNF( normal.getX(), normal.getY(), normal.getZ(), d );

					// Update plane in real-time (for preview)
					mMeshVisual->setPlaneHNF( planeHNF );

					// Calculate intersection polyline with mesh (Archaeological illustration mode)
					if( mMeshVisual->calcIntersectionPolylineWithPlane( planeHNF, &mSectionIntersectionPoints ) ) {
						std::cout << "[MeshWidget] Real-time section: " << mSectionIntersectionPoints.size()
						          << " intersection points" << std::endl;
					}

					emit sParamFlagMesh( MeshGLParams::SHOW_MESH_PLANE_TEMP, true );
					update();
				}
			}
		}
		return;
	}

	//! Display the selection of a polyline:
	if( ( rEvent->buttons() == Qt::NoButton ) &&
	    ( currMouseMode == MOUSE_MODE_SELECT ) &&
        ( currSelectionMode == MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO ||  currSelectionMode == MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO)
	  ) {
		if( mSelectionPoly.size()>0 ) {
			mSelectionPoly.pop_back();
			mSelectionPoly.push_back( mLastPos );
			update();
			return;
		}
	}

	bool lightEnabled;
	bool lightFixedCam;
	bool lightFixedWorld;
	getParamFlagMeshWidget( LIGHT_ENABLED, &lightEnabled );
	getParamFlagMeshWidget( LIGHT_FIXED_CAM, &lightFixedCam );
	getParamFlagMeshWidget( LIGHT_FIXED_WORLD, &lightFixedWorld );

	//double stepLight; // to be removed?
	//getParamFloatMeshWidget( LIGHT_STEPPING, &stepLight );

	//! Move lightFixedCam, when the Left Mouse Button is pressed and the lights are on.
	if( ( rEvent->buttons() == Qt::LeftButton ) &&
		( currMouseMode == MOUSE_MODE_MOVE_LIGHT_FIXED_CAM ) &&
		( lightEnabled ) &&
		( lightFixedCam )
	) {
		double lightAnglePhi;
		double lightAngleTheta;
		getParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_PHI,   &lightAnglePhi   );
		getParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_THETA, &lightAngleTheta );
		Vector3D vecLightDir( lightAnglePhi, lightAngleTheta, false );

		float moveAngleLeftRight =  static_cast<float>(dx) * M_PI / ( 180.0 * 2.0 );
		float moveAngleUpDown    =  static_cast<float>(dy) * M_PI / ( 180.0 * 2.0 ); // ( 180.0 * factor ) ... factor influences the speed of the rotation

		// rotate left/right
		if( moveAngleUpDown != 0.0 ) {
			Vector3D vecOrigin( 0.0, 0.0, 0.0, 1.0 );
			Vector3D vecAxis( 1.0, 0.0, 0.0, 0.0 );
			Matrix4D transMatUpDown( vecOrigin, vecAxis, moveAngleUpDown );
			vecLightDir.applyTransformation( transMatUpDown );
		}
		// rotate forward / backward
		if( moveAngleLeftRight != 0.0 ) {
			Vector3D vecOrigin( 0.0, 0.0, 0.0, 1.0 );
			Vector3D vecAxis( 0.0, 1.0, 0.0, 0.0 );
			Matrix4D transMatLeftRight( vecOrigin, vecAxis, moveAngleLeftRight );
			vecLightDir.applyTransformation( transMatLeftRight );
		}
		setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_PHI,   vecLightDir.getSphPhiDeg()   );
		setParamFloatMeshWidget( LIGHT_FIXED_CAM_ANGLE_THETA, vecLightDir.getSphThetaDeg() );

		setView();
		update();
		return;
	}
	//! Move lightFixedWorld, when the Left Mouse Button is pressed and the lights are on.
	if( ( rEvent->buttons() == Qt::LeftButton ) &&
	    ( currMouseMode == MOUSE_MODE_MOVE_LIGHT_FIXED_OBJECT ) &&
	    ( lightEnabled ) &&
	    ( lightFixedWorld )
	  ) {
		double lightAnglePhi;
		double lightAngleTheta;
		getParamFloatMeshWidget( LIGHT_FIXED_WORLD_ANGLE_PHI,   &lightAnglePhi   );
		getParamFloatMeshWidget( LIGHT_FIXED_WORLD_ANGLE_THETA, &lightAngleTheta );
		Vector3D vecLightDir( lightAnglePhi, lightAngleTheta, false );

		float moveAngleLeftRight =  static_cast<float>(dx) * M_PI / ( 180.0 * 2.0 );
		float moveAngleUpDown    =  static_cast<float>(dy) * M_PI / ( 180.0 * 2.0 ); // ( 180.0 * factor ) ... factor influences the speed of the rotation
		// rotate left/right
		if( moveAngleUpDown != 0.0 ) {
			Vector3D cameraPitchAxis( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 );
			Matrix4D transMatUpDown( mCenterView, cameraPitchAxis, moveAngleUpDown );
			vecLightDir.applyTransformation( transMatUpDown );
		}
		// rotate forward / backward
		if( moveAngleLeftRight != 0.0 ) {
			Matrix4D transMatLeftRight( mCenterView, mCameraUp, moveAngleLeftRight );
			vecLightDir.applyTransformation( transMatLeftRight );
		}
		setParamFloatMeshWidget( LIGHT_FIXED_WORLD_ANGLE_PHI,   vecLightDir.getSphPhiDeg()   );
		setParamFloatMeshWidget( LIGHT_FIXED_WORLD_ANGLE_THETA, vecLightDir.getSphThetaDeg() );
		setView();
		update();
		return;
	}

	bool planeShown = false;
	if( mMeshVisual != nullptr ) {
		bool showPlane;
		mMeshVisual->getParamFlagMeshGL( MeshGLParams::SHOW_MESH_PLANE, &showPlane );
		bool showPlaneTemp;
		mMeshVisual->getParamFlagMeshGL( MeshGLParams::SHOW_MESH_PLANE_TEMP, &showPlaneTemp );
		planeShown = showPlane | showPlaneTemp;
	}

	//! Move/Rotate the plane, when the Left Mouse Button is pressed.
	if( ( rEvent->buttons() == Qt::LeftButton ) &&
	    ( currMouseMode == MOUSE_MODE_MOVE_PLANE ) &&
	    ( planeShown )
	  ) {
		double moveAngleLeftRight =  dx * M_PI / ( 180.0 * 2.0 );
		double moveAngleUpDown    =  dy * M_PI / ( 180.0 * 2.0 ); // ( 180.0 * factor ) ... factor influences the speed of the rotation
		// rotate left/right
		if( moveAngleUpDown != 0.0 ) {
			Vector3D cameraPitchAxis( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 );
			Matrix4D transMatUpDown( mCenterView, cameraPitchAxis, moveAngleUpDown );
			emit sApplyTransfromToPlane( transMatUpDown );
		}
		// rotate forward / backward
		if( moveAngleLeftRight != 0.0 ) {
			Matrix4D transMatLeftRight( mCenterView, mCameraUp, moveAngleLeftRight );
			emit sApplyTransfromToPlane( transMatLeftRight );
		}
		setView();
		update();
		return;
	}
	//! Move/Translate the plane, when the Right Mouse Button is pressed.
	if( ( rEvent->buttons() == Qt::RightButton ) &&
		( currMouseMode == MOUSE_MODE_MOVE_PLANE ) &&
	    ( planeShown )
	  ) {
		double pixelWidth;
		double pixelHeight;
		getViewPortPixelWorldSize( pixelWidth, pixelHeight );
		// move left/right
		if( dy != 0.0 ) {
			Vector3D moveLR = mCameraUp        * +dy * pixelHeight;
			Matrix4D transMatUpDown( moveLR );
			emit sApplyTransfromToPlane( transMatUpDown );
		}
		// move up/down
		if( dx != 0.0 ) {
			Vector3D cameraPitchAxis( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 );
			Vector3D moveUD = cameraPitchAxis * -dx * pixelWidth;
			Matrix4D transMatLeftRight( moveUD );
			emit sApplyTransfromToPlane( transMatLeftRight );
		}
		setView();
		update();
		return;
	}

	//! Move the plane along Axis, when the Left Mouse Button is pressed.
	if( ( rEvent->buttons() == Qt::LeftButton) &&
	    ( currMouseMode == MeshWidgetParams::MOUSE_MODE_MOVE_PLANE_AXIS) &&
	    ( mMeshVisual->getConeAxisDefined() ) &&
	    ( planeShown))
	{
		const double dLen = [&](){
			double pixelWidth;
			double pixelHeight;
			getViewPortPixelWorldSize( pixelWidth, pixelHeight );
			return std::abs(dx) >= std::abs(dy) ?
			            static_cast<double>(dx) * pixelWidth :
			            static_cast<double>(dy) * pixelHeight;
		}();

		Vector3D axisTop;
		Vector3D axisBottom;
		mMeshVisual->getConeAxis(&axisTop, &axisBottom);

		Vector3D transVec = (axisTop - axisBottom);
		transVec.normalize3();
		transVec *= dLen;

		std::vector<double> vTranslate = {transVec.getX(), transVec.getY(), transVec.getZ()};

		const Matrix4D transMat(Matrix4D::INIT_TRANSLATE, &vTranslate);

		emit sApplyTransfromToPlane( transMat );

		setView();
		update();
		return;
	}

	//! Rotate the plane around Axis, when the Right Mouse Button is pressed.
	if( ( rEvent->buttons() == Qt::LeftButton) &&
	    ( currMouseMode == MeshWidgetParams::MOUSE_MODE_ROTATE_PLANE_AXIS) &&
	    ( mMeshVisual->getConeAxisDefined() ) &&
	    ( planeShown) &&
	    ( mMeshVisual->getPlaneDefinition() == Plane::AXIS_POINTS_AND_POSITION))
	{
		const double dLen = [&](){
			double pixelWidth;
			double pixelHeight;
			getViewPortPixelWorldSize( pixelWidth, pixelHeight );
			return std::abs(dx) >= std::abs(dy) ?
			            static_cast<double>(dx) * pixelWidth :
			            static_cast<double>(dy) * pixelHeight;
		}();

		Vector3D axisTop;
		Vector3D axisBottom;
		mMeshVisual->getConeAxis(&axisTop, &axisBottom);

		Vector3D rotAxis = (axisTop - axisBottom);
		rotAxis.normalize3();

		std::array<double,9> planePositions;
		mMeshVisual->getPlanePositions(planePositions.data());
		Vector3D planeZPoint(&planePositions[6], 1.0);

		const Vector3D rotPoint = planeZPoint.projectOntoLine(axisTop, axisBottom);

		const Matrix4D rotMat(rotPoint,rotAxis, dLen * 0.1);

		planeZPoint = rotMat * planeZPoint;

		mMeshVisual->setPlaneAxisPos(axisTop, axisBottom, planeZPoint);

		setView();
		update();
		return;
	}

	//! Move/Rotate the scene, when the left mouse button is pressed.
	if( ( rEvent->buttons() == Qt::LeftButton ) &&
		( currMouseMode == MOUSE_MODE_MOVE_CAMERA )
	) {
		// rotate up/down
		//rotPitch( -dy );
		// rotate left/right
		//rotYaw( -dx );

		//perform Pitch and Yaw. Smoother, since it only makes one update/setview
		rotRollPitchYaw( 0.0, -dy, -dx );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);

		bool showMeshReduced = false;
		getParamFlagMeshWidget(MeshWidgetParams::ENABLE_SHOW_MESH_REDUCED, &showMeshReduced);
		setParamFlagMeshWidget(MeshWidgetParams::SHOW_MESH_REDUCED, showMeshReduced);
		return;
	}

	//! Move/Translate the scene parallel to the camera'splane, when the right mouse button is pressed.
	if( ( rEvent->buttons() == Qt::RightButton ) &&
	    ( currMouseMode == MOUSE_MODE_MOVE_CAMERA )
	  ) {
		//double viewDistanceDecrement;
		//getParamFloatMeshWidget( VIEW_DIST_DECREMENT, &viewDistanceDecrement );

		double pixelWidth;
		double pixelHeight;
		getViewPortPixelWorldSize( pixelWidth, pixelHeight );
		pixelWidth *= 2.0;
		pixelHeight *= 2.0;

		Vector3D cameraPitchAxis( mMatModelView(0,0), mMatModelView(0,1), mMatModelView(0,2), 0.0 );
		mCameraCenter += cameraPitchAxis  * -dx * pixelWidth + mCameraUp * dy * pixelHeight;
		mCenterView   += cameraPitchAxis  * -dx * pixelWidth + mCameraUp * dy * pixelHeight;

		bool showMeshReduced = false;
		getParamFlagMeshWidget(MeshWidgetParams::ENABLE_SHOW_MESH_REDUCED, &showMeshReduced);
		setParamFlagMeshWidget(MeshWidgetParams::SHOW_MESH_REDUCED, showMeshReduced);

		setView();
		update();
		return;
	}
}

//! Handles the event when a key on the keyboard is pressed (and hold).
//! See also MeshWidget::keyReleaseEvent.
//!
//! Sets the flag for camera-movement or model-movement.
//!
//! Keys:
void MeshWidget::keyPressEvent( QKeyEvent *rEvent ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	//cout << "[MeshWidget::" << __FUNCTION__ << "] Key: " << rEvent->key() << endl;
	//qDebug() << "[MeshWidget::" << __FUNCTION__ << "] Key: " << QKeySequence( (Qt::Key)rEvent->key() ).toString();

	// The "return" statements ensure "...do not call the base class implementation if you act upon the key."
	// See: qt-project.org/doc/qwidget.html#keyPressEvent
	// At the end of this method <parent>::keyPressEvent has to be called, otherwise the key-handling becomes f**ked up.

	double moveAngle = 1.0; // Default: one degree rotations.
	getParamFloatMeshWidget( ROTATION_STEP, &moveAngle );
	//! A/D: rotate left/right
	if( rEvent->key() == Qt::Key_A ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneYaw( +moveAngle ) : rotYaw( +moveAngle );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	if( rEvent->key() == Qt::Key_D ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneYaw( -moveAngle ) : rotYaw( -moveAngle );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	//! W/S: rotate up/down
	if( rEvent->key() == Qt::Key_W ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlanePitch( +moveAngle ) : rotPitch( +moveAngle );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	if( rEvent->key() == Qt::Key_S ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlanePitch( -moveAngle ) : rotPitch( -moveAngle );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	//! Q/E,R camera roll counterclockwise/clockwise
	if( rEvent->key() == Qt::Key_Q ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneRoll( +moveAngle ) : rotRoll( +moveAngle );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	if( rEvent->key() == Qt::Key_E ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneRoll( -moveAngle ) : rotRoll( -moveAngle );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	if( rEvent->key() == Qt::Key_R ) { // Half step i.e. half degree roll clockwise
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneRoll( -moveAngle/2.0 ) : rotRoll( -moveAngle/2.0 );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	if( rEvent->key() == Qt::Key_T ) { // 1/10 degree roll clockwise
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneRoll( -moveAngle/10.0 ) : rotRoll( -moveAngle/10.0 );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}

	//! Zoom with plus and minus key. See MeshWidget::wheelEventZoom
	if( rEvent->key() == Qt::Key_Plus ) {
		rEvent->modifiers() & Qt::ShiftModifier ? wheelEventZoom( +10.0 ) : wheelEventZoom( +1.0 );
		return;
	}
	if( rEvent->key() == Qt::Key_Minus ) {
		wheelEventZoom( -1.0 );
		return;
	}
	if( ( rEvent->key() == 8722 ) || // Shift Minus - at least on a German numeric keyboard with an EN locale.
	    ( rEvent->key() == Qt::Key_Underscore ) ) { // Shift Minus at a German keyboad equal underscore.
		wheelEventZoom( -9.0 );
		return;
	}

	// --- 90° Rotations ----
	//! Y/X rotate left/right about 90°
	if( rEvent->key() == Qt::Key_Y ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneYaw( +90.0) : rotYaw( +90.0 );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	if( rEvent->key() == Qt::Key_X ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneYaw( -90.0) : rotYaw( -90.0 );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	//! C/V rotate up/down about 90°
	if( rEvent->key() == Qt::Key_C ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlanePitch( +90.0) : rotPitch( +90.0 );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	if( rEvent->key() == Qt::Key_V ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlanePitch( -90.0) : rotPitch( -90.0 );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	//! B/N camera roll clockwise/counterclockwise
	if( rEvent->key() == Qt::Key_B ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneRoll( +90.0) : rotRoll( +90.0 );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}
	if( rEvent->key() == Qt::Key_N ) {
		rEvent->modifiers() & Qt::ShiftModifier ? rotPlaneRoll( -90.0) : rotRoll( -90.0 );
		emit camRotationChanged(mCameraCenter - mCenterView, mCameraUp);
		return;
	}

	if ( ( rEvent->key() == Qt::Key_Escape ) &&
	     ( mMainWindow->isFullScreen() ) ){ // abort Fullscreen
			mMainWindow->toggleFullscreen();
			return;
	}

	//! Cursor keys shift the viewport in orthographic projection mode
	bool orthoMode;
	getParamFlagMeshWidget( ORTHO_MODE, &orthoMode );
	if( orthoMode ) {
		double pixelWidth;
		double pixelHeight;
		if( !getViewPortPixelWorldSize( pixelWidth, pixelHeight ) ) {
			cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: getViewPortPixelWorldSize() failed!" << endl;
		}
		if( rEvent->key() == Qt::Key_Left ) {
			double horiShift;
			getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &horiShift );
			horiShift += pixelWidth;
			setParamFloatMeshWidget( ORTHO_SHIFT_HORI, horiShift );
			return;
		}
		if( rEvent->key() == Qt::Key_Right ) {
			double horiShift;
			getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &horiShift );
			horiShift -= pixelWidth;
			setParamFloatMeshWidget( ORTHO_SHIFT_HORI, horiShift );
			return;
		}
		if( rEvent->key() == Qt::Key_Up ) {
			double vertShift;
			getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &vertShift );
			vertShift -= pixelHeight;
			setParamFloatMeshWidget( ORTHO_SHIFT_VERT, vertShift );
			return;
		}
		if( rEvent->key() == Qt::Key_Down ) {
			double vertShift;
			getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &vertShift );
			vertShift += pixelHeight;
			setParamFloatMeshWidget( ORTHO_SHIFT_VERT, vertShift );
			return;
		}
	}
	//cout << "[MeshWidget::" << __FUNCTION__ << "] Key: " << rEvent->key() << " ignored." << endl;
	QGLWidget::keyPressEvent( rEvent );
}


//! Determines the central pixel of the viewport and uses it for computing an axis (of the cone).
//! Strongly related to MeshGL::getRayWorld.
//! See also Mesh::setConeAxis.
//!
//! @returns false in case of an error. True otherwise.
bool MeshWidget::userSetConeAxisCentralPixel() {
	// Sanity check
	if( mMeshVisual == nullptr ) {
		return( false );
	}

	float projInv[16];
	float modvInv[16];
	invert( mMatProjection.constData(), projInv );
	invert( mMatModelView.constData(), modvInv );
	Matrix4D projMatInv( projInv );
	Matrix4D modvMatInv( modvInv );

	// Helper function for MeshWidgetParams::GRID_CENTER_POSITION
	double xOffset = 0.0;
	double yOffset = 0.0;
	if( !getGridCenterPosOffsets( xOffset, yOffset ) ) {
		return( false );
	}

	Vector3D rayTop( modvMatInv * ( projMatInv * Vector3D( -xOffset, -yOffset, -1.0, 1.0 ) ) );
	Vector3D rayBot( modvMatInv * ( projMatInv * Vector3D( -xOffset, -yOffset, +1.0, 1.0 ) ) );
	// Old for center:
	// Vector3D rayTop( modvMatInv * ( projMatInv * Vector3D( 0.0, 0.0, -1.0, 1.0 ) ) );
	// Vector3D rayBot( modvMatInv * ( projMatInv * Vector3D( 0.0, 0.0, +1.0, 1.0 ) ) );
	rayTop.normalizeW();
	rayBot.normalizeW();

	return mMeshVisual->setConeAxis( &rayTop, &rayBot );
}


//! Performs a selection at a given widget coordinate considering the mouse button involved in this choice.
bool MeshWidget::userSelectByMouseClick(
                QPoint rPoint,
                QFlags<Qt::MouseButton> rMouseButton
) {
	bool retVal = false;
	switch( rMouseButton ) {
		case Qt::LeftButton:
			retVal = userSelectAtMouseLeft( rPoint );
			break;
		case Qt::RightButton:
			retVal = userSelectAtMouseRight( rPoint );
			break;
	}
	return( retVal );
}

//! Performs a selection at the given screen/widget coordinate.
//! Triggered by pressing the left mouse button.
bool MeshWidget::userSelectAtMouseLeft( const QPoint& rPoint ) {
	// Sanity
	if( mMeshVisual == nullptr ) {
		return( false );
	}

	// Correct for OpenGL:
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	int yPixel = viewport[3] - rPoint.y();
	int xPixel = rPoint.x();

	MeshWidgetParams::eSelectionModes selectionMode;
	this->getParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, reinterpret_cast<int*>(&selectionMode) );

	bool retVal = false;
	switch( selectionMode ) {
		case MeshWidgetParams::SELECTION_MODE_NONE:
			// Nothing to do.
			return( true );
			break;
		case MeshWidgetParams::SELECTION_MODE_VERTEX:
			retVal = mMeshVisual->selectPrimitiveAt( Primitive::IS_VERTEX, xPixel, yPixel, false );
			break;
		case MeshWidgetParams::SELECTION_MODE_FACE:
			retVal = mMeshVisual->selectPrimitiveAt( Primitive::IS_FACE, xPixel, yPixel, false );
			break;
		case MeshWidgetParams::SELECTION_MODE_VERTICES:
			retVal = mMeshVisual->selectPrimitiveAt( Primitive::IS_VERTEX, xPixel, yPixel, true );
			break;
		case MeshWidgetParams::SELECTION_MODE_MULTI_FACES:
			retVal = mMeshVisual->selectPrimitiveAt( Primitive::IS_FACE, xPixel, yPixel, true );
			break;
		case MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO:
			// Nothing to do.
			return( true );
			break;
        case MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO:
            // Nothing to do.
            return( true );
            break;
		case MeshWidgetParams::SELECTION_MODE_PLANE_3FP:
			retVal = mMeshVisual->selectPlaneThreePoints( xPixel, yPixel );
			break;
		case MeshWidgetParams::SELECTION_MODE_POSITIONS:
			retVal = mMeshVisual->selectPositionAt( rPoint.x(), yPixel, false );
			break;
        case MeshWidgetParams::SELECTION_MODE_THREE_POSITIONS:
            if (mMeshVisual->isMoreThanNconSelPosition(1)){
                //last point of the position set --> start automatically a new one
                retVal = mMeshVisual->selectPositionAt( rPoint.x(), yPixel, true );
            }
            else{
                //add point to the same set
                retVal = mMeshVisual->selectPositionAt( rPoint.x(), yPixel, false );
            }

            break;
		case MeshWidgetParams::SELECTION_MODE_CONE: {
			retVal = mMeshVisual->selectConePoints( xPixel, yPixel );
			break;
		}
		case MeshWidgetParams::SELECTION_MODE_SPHERE: {
			retVal = mMeshVisual->selectSpherePoints( xPixel, yPixel );
			break;
		}
		default:
			std::cerr << "[MeshGL::" << __FUNCTION__ << "] invalid selection mode: " << selectionMode << "!" << std::endl;
			retVal = false;
	}

	if( !retVal ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR. Unknown!" << std::endl;
	}
	return( retVal );
}


//! Performs a selection at the given screen/widget coordinate.
//! Triggered by pressing the right mouse button.
bool MeshWidget::userSelectAtMouseRight( const QPoint& rPoint ) {
	// Sanity
	if( mMeshVisual == nullptr ) {
		return( false );
	}

	// Correct for OpenGL:
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	int yPixel = viewport[3] - rPoint.y();

	MeshWidgetParams::eSelectionModes selectionMode;
	this->getParamIntegerMeshWidget( MeshWidgetParams::SELECTION_MODE, reinterpret_cast<int*>(&selectionMode) );

	bool retVal = false;
	switch( selectionMode ) {
		case MeshWidgetParams::SELECTION_MODE_NONE:
		case MeshWidgetParams::SELECTION_MODE_VERTEX:
		case MeshWidgetParams::SELECTION_MODE_FACE:
		case MeshWidgetParams::SELECTION_MODE_VERTICES:
		case MeshWidgetParams::SELECTION_MODE_VERTICES_LASSO:
        case MeshWidgetParams::DESELECTION_MODE_VERTICES_LASSO:
		case MeshWidgetParams::SELECTION_MODE_MULTI_FACES:
		case MeshWidgetParams::SELECTION_MODE_PLANE_3FP:
		case MeshWidgetParams::SELECTION_MODE_CONE:
		case MeshWidgetParams::SELECTION_MODE_SPHERE:
			// Nothing to do.
			retVal = true;
			break;
		case MeshWidgetParams::SELECTION_MODE_POSITIONS:
			retVal = mMeshVisual->selectPositionAt( rPoint.x(), yPixel, true );
			break;
        case MeshWidgetParams::SELECTION_MODE_THREE_POSITIONS:
            // Nothing to do.
            retVal = true;
            break;
		default:
			std::cerr << "[MeshGL::" << __FUNCTION__ << "] invalid selection mode: " << selectionMode << "!" << std::endl;
			retVal = false;
	}

	if( !retVal ) {
		std::cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR. Unknown!" << std::endl;
	}
	return( retVal );
}

//! Restore the view and the lights to the default setting.
//! Was identical to the (removed) slot resetView().
void MeshWidget::defaultViewLight() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	setViewInitial();
	setView();
	update();
}

//! Restore the view, the lights and zoom (ortho) to the default setting.
void MeshWidget::defaultViewLightZoom() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	setViewInitial();
	// setParamFloatMeshWidget( ORTHO_ZOOM, 1.0 );
	setViewInitialZoom();
	setView();
	update();
}

//! Set the COG of the selected Primitive to the reference point for the camera's view.
void MeshWidget::selPrimViewReference() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	Primitive* primSelected = mMeshVisual->getPrimitiveSelected();
	if( primSelected != nullptr ) {
		mCenterView = primSelected->getCenterOfGravity();
		setView();
	}
}

//! Object transformation to use the current view as default view.
//! @returns false in case of an error. True otherwise.
bool MeshWidget::currentViewToDefault() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Sanity check
	if( mMeshVisual == nullptr ) {
		return( false );
	}

	// INSPIRED by:
	// *) VIEW TRANSFORMATION: http://msdn.microsoft.com/en-us/library/aa915179.aspx
	//Another approach involves creating the composite view matrix directly. This approach uses the camera's world space position and a look-at point in the scene to derive vectors that describe the orientation of the camera space coordinate axes. The camera position is subtracted from the look-at point to produce a vector for the camera's direction vector (vector n).
	//Then the cross product of the vector n and the y-axis of world space is taken and normalized to produce a right vector (vector u).
	// Next, the cross product of the vectors u and n is taken to determine an up vector (vector v).
	// The right (u), up (v), and view-direction (n) vectors describe the orientation of the coordinate axes for camera space in terms of world space. The x, y, and z translation factors are computed by taking the negative of the dot product between the camera position and the u, v, and n vectors.
	// *) BETTER with Change of basis (Basiswechsel, Vektorrechnung): http://en.wikipedia.org/wiki/Change_of_basis
	//double transArr[16] = {
	//       cameraPitchAxis.getX(), mCameraUp.getX(), -cameraRollAxis.getX(),  0.0,
	//       cameraPitchAxis.getY(), mCameraUp.getY(), -cameraRollAxis.getY(),  0.0,
	//       cameraPitchAxis.getZ(), mCameraUp.getZ(), -cameraRollAxis.getZ(),  0.0,
	//                     0.0,             0.0,                    0.0,       1.0
	//};
	//  *) EVEN BETTER - use the OpenGL transformation matrix:
	// ... AND finally DEPRECATED with OpenGL 3.2 in 2014
	//double transArr[16];
	//glGetDoublev( GL_MODELVIEW_MATRIX, transArr );
	//Matrix4D transMat( transArr );

	Matrix4D transMat( mMatModelView.data() );
	if( !mMeshVisual->applyTransformationDefaultViewMatrix( &transMat ) ) {
		// User cancel or error
		return( false );
	}

	bool retVal = true;

	// setup initial view:
	setViewInitial();
	setView();
	update();

	QString clipBoardText;
	QString msgboxText = "<html>\n";
	//for( int i=0; i<16; i++ ) {
	//	msgboxText += QString( " %1" ).arg( transArr[i] );
	//}
	msgboxText += "<table>";
	msgboxText += QString("<th><td colspan\"4\"><b>")+ tr("Transformation matrix") + QString("</b></td></th>");
	for( int i=0; i<4; i++ ) {
		msgboxText += "<tr>\n";
		for( int j=0; j<4; j++ ) {
			clipBoardText += QString( "%1 " ).arg( transMat(i,j) );
			msgboxText += "<td align=\"right\">" + QString( "%1 " ).arg( transMat(i,j) ) + "</td>";
		}
		clipBoardText += "\n";
		msgboxText += "</tr>\n";
	}
	msgboxText += "</table><br /><br />";
	msgboxText += tr("... already copied to the clipboard.");
	msgboxText += "</html>\n";

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText( clipBoardText );
	mMeshVisual->setParamFlagMesh( MeshParams::FILE_TRANSFORMATION_APPLIED, true );
	SHOW_MSGBOX_INFO_SAVE( tr("Object transformed"), msgboxText, mMeshVisual, &MeshQt::writeFileUserInteract );

	return( retVal );
}

void MeshWidget::openNormalSphereSelectionDialog(bool faces)
{
	auto dialog = this->findChild<QDialog*>(tr("NormalSphereSelectionDialog"));

	//avoid multiple instances of the dialog
	if(dialog != nullptr)
	{
		return;
	}

	auto normalSphereDialog = new NormalSphereSelectionDialog(this, faces);

	normalSphereDialog->setAttribute(Qt::WA_DeleteOnClose);
	normalSphereDialog->setWindowFlags( normalSphereDialog->windowFlags() | Qt::Tool);

	normalSphereDialog->show();

	normalSphereDialog->setMeshNormals(mMeshVisual);

	connect(this, &MeshWidget::camRotationChanged, normalSphereDialog, &NormalSphereSelectionDialog::updateRotationExternal);
	connect(normalSphereDialog, &NormalSphereSelectionDialog::rotationChanged, this, &MeshWidget::setCameraRotation);
}

void MeshWidget::setCameraRotation(QQuaternion rotationQuat)
{

	auto distToCamera = (mCenterView- mCameraCenter).getLength3();

	QVector3D camCenter = (rotationQuat.conjugated() * QVector3D(0.0,0.0,1.0)).normalized();
	QVector3D up = (rotationQuat.conjugated() * QVector3D(0.0,1.0,0.0)).normalized();

	mCameraCenter = Vector3D(camCenter.x(), camCenter.y(), camCenter.z(), 0.0) * distToCamera + mCenterView;

	mCameraUp = Vector3D(up.x(), up.y(), up.z(), 0.0);

	setView();
	update();

}


//! Handles the event when the mouse-wheel is turned.
//! Used for to zoom in/out of our Mesh.
void MeshWidget::wheelEvent( QWheelEvent* rEvent ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	// Why division by 120? Because it is a value set by trolltech.
	// => see http://doc.qt.io/qt-5/qwheelevent.html#angleDelta
	QPoint wheelAngleDelta = rEvent->angleDelta();
	double wheelDelta = static_cast<double>(wheelAngleDelta.y()) / 120.0; // will be +1 or -1. However there might be mouswheels with finer resolution!
	// cout << "[MeshWidget::" << __FUNCTION__ << "] WheelDelta: " << wheelDelta << endl;
	wheelEventZoom( wheelDelta );
}

//! Helper function for MeshWidget::wheelEvent and MeshWidget::keyPressEvent
//! sharing the zoom for the scene.
void MeshWidget::wheelEventZoom(
                double rWheelDelta   //!< Zoom increment. Typically -1.0 or +1.0.
) {
//	eMouseModes currMouseMode;
//	getParamIntegerMeshWidget( MOUSE_MODE, reinterpret_cast<int*>(&currMouseMode) );
//	if( currMouseMode != MeshWidgetParams::MOUSE_MODE_MOVE_CAMERA ) {
//		return;
//	}

	bool orthoMode;
	getParamFlagMeshWidget( ORTHO_MODE, &orthoMode );
	if( orthoMode ) {
		double pixelWidth;
		double pixelHeight;
		getViewPortPixelWorldSize( pixelWidth, pixelHeight );
		double currentDPI = 25.4/pixelWidth;
		double base10inc = pow( 10.0, floor( log10( currentDPI )-1.0 ) );
		double newDPI = currentDPI + rWheelDelta * base10inc;
		if( floor( log10( currentDPI ) ) > floor( log10( newDPI ) ) ) {
			base10inc = pow( 10.0, floor( log10( newDPI )-1.0 ) );
			newDPI = currentDPI + rWheelDelta * base10inc;
		}
		if( floor( log10( currentDPI ) ) < floor( log10( newDPI ) ) ) {
			base10inc = pow( 10.0, floor( log10( newDPI )-1.0 ) );
			newDPI = currentDPI + rWheelDelta * base10inc;
		}
		orthoSetDPI( newDPI );
		// OLD:
		// ... we changed the viewport instead of changing the view distance to zoom in/out.
		// double orthoZoom;
		// getParamFloatMeshWidget( ORTHO_ZOOM, &orthoZoom );
		// orthoZoom += wheelDelta * 0.025;
		// if( orthoZoom <= FLT_EPSILON ) {
		//	orthoZoom = FLT_EPSILON;
		// }
		// setParamFloatMeshWidget( ORTHO_ZOOM, orthoZoom );
	} else {
		double viewDistanceDecrement;
		getParamFloatMeshWidget( VIEW_DIST_DECREMENT, &viewDistanceDecrement );
		Vector3D viewVector = mCenterView - mCameraCenter;
		float newDist = viewVector.getLength3() - rWheelDelta * viewDistanceDecrement;
		viewVector.setLength3( -newDist );
		mCameraCenter = mCenterView + viewVector;
	}
	setView();
	update();
}

//! Takes care to properly resize this OpenGL widget.
void MeshWidget::resizeEvent( [[maybe_unused]] QResizeEvent * event  ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	std::cout << "[MeshWidget::" << __FUNCTION__ << "] " << event->size().width()
	          << " x " << event->size().height() << std::endl;
#endif
	//QGLWidget::resizeEvent( event );
	if( mMeshVisual == nullptr ) {
		// No mesh present -> nothing to do.
		return;
	}
	setView();
	update();
}

//! Initial Setup of the camera, the light position(s) and the material properties.
void MeshWidget::setViewInitial() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	if( mMeshVisual == nullptr ) {
		cerr << "[MeshWidget::" << __FUNCTION__ << "] ERROR: No mesh!" << endl;
		return;
	}

	double bBoxRadius   = mMeshVisual->getBoundingBoxRadius();
	double distToCamera = bBoxRadius / ( tan( M_PI*mParamFlt[FOV_ANGLE]/360.0 ) );

	// Camera setup:
	mCenterView   = mMeshVisual->getBoundingBoxCenter();
	mCameraCenter = Vector3D( 0.0, 0.0, 1.0, 0.0 ) * distToCamera + mCenterView;
	mCameraUp     = Vector3D( 0.0, 1.0, 0.0, 0.0 );
	// setViewModelMat();
	setParamFloatMeshWidget( VIEW_DIST_DECREMENT, bBoxRadius / 100.0 ); // zoom steps: 1% of the initial viewDistance;

	// Ambient light:
	mParamFlt[AMBIENT_LIGHT] = AMBIENT_LIGHT_DEFAULT;

	// Material:
	mParamFlt[MATERIAL_SHININESS] = MATERIAL_SHININESS_DEFAULT;
	mParamFlt[MATERIAL_SPECULAR]  = MATERIAL_SPECULAR_DEFAULT;

	// Lights:
	mParamFlt[LIGHT_FIXED_WORLD_ANGLE_PHI]   = LIGHTX_ANGLE_PHI_DEFAULT;
	mParamFlt[LIGHT_FIXED_WORLD_ANGLE_THETA] = LIGHTX_ANGLE_THETA_DEFAULT;
	mParamFlt[LIGHT_FIXED_CAM_INTENSITY]     = LIGHTX_INTENSITY_DEFAULT;
	mParamFlt[LIGHT_FIXED_CAM_ANGLE_PHI]     = LIGHTX_ANGLE_PHI_DEFAULT;
	mParamFlt[LIGHT_FIXED_CAM_ANGLE_THETA]   = LIGHTX_ANGLE_THETA_DEFAULT;
	mParamFlt[LIGHT_FIXED_WORLD_INTENSITY]   = LIGHTX_INTENSITY_DEFAULT;

	mParamFlt[ORTHO_SHIFT_HORI] = 0.0f;
	mParamFlt[ORTHO_SHIFT_VERT] = 0.0f;
// 	setParamFloatMeshWidget( ORTHO_SHIFT_HORI, 0.0 );
// 	setParamFloatMeshWidget( ORTHO_SHIFT_VERT, 0.0 );
}

//! Set the inital zoom factor to a rounded DPI value.
void MeshWidget::setViewInitialZoom() {
	if( mMeshVisual == nullptr ) {
		return;
	}
	double bBoxRadius   = mMeshVisual->getBoundingBoxRadius();
	Vector3D boundingBox;
	mMeshVisual->getBoundingBoxSize( boundingBox );
	double initZoomX =  boundingBox.getX()/(2.0*bBoxRadius);                    // Scale to x-range
	double initZoomY =  (boundingBox.getY()*width())/(2.0*bBoxRadius*height()); // Scale to y-range
	double initZoom = max( initZoomX, initZoomY );
	// setParamFloatMeshWidget( ORTHO_ZOOM, initZoom ); // <- without rounding maximizing the object to the viewport.
	double dpiZoom = static_cast<double>(width())/(2.0*bBoxRadius*initZoom)*25.4;
	dpiZoom /= 2.0;
	double dpiRound = floor( dpiZoom / pow( 10.0, floor( log10( dpiZoom ) ) ) ) * pow( 10.0, floor( log10( dpiZoom ) ) );
	dpiRound *= 2.0;
	orthoSetDPI( dpiRound );
	cout << "[MeshWidget::" << __FUNCTION__ << "] Size: " << width() << " x " << height() << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] DPI Zoom:    " << dpiZoom << endl;
	cout << "[MeshWidget::" << __FUNCTION__ << "] DPI Rounded: " << dpiRound << endl;
}

//! rOrthoViewPort has to be of length 4. It is used for tiled rendering inspired by Brian Pauls
//! Tile Rendering Library http://www.mesa3d.org/brianp/TR.html
//!
//! Setup of the view:
void MeshWidget::setView( GLdouble* rOrthoViewPort //!< position and dimension of the viewport in orthomode
    ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshWidget::" << __FUNCTION__ << "]" << endl;
#endif
	PRINT_OPENGL_ERROR( "OLD_ERROR" );

//	//! Initalize projection matrix.
//	glMatrixMode( GL_PROJECTION );
//	PRINT_OPENGL_ERROR( "glMatrixMode( GL_PROJECTION )" );
//	glLoadIdentity();
//	PRINT_OPENGL_ERROR( "glLoadIdentity()" );


	//! Setup Viewport
	GLfloat windowRatio = static_cast<GLfloat>(width())/static_cast<GLfloat>(height());
	glViewport( 0, 0, width(), height() );

	//! Estimate clipping planes (front and back) for the frustum.
	// distance between camera and the bounding box center
	// and the camera plane:
	Vector3D cameraPlaneNormal = mCenterView - mCameraCenter;

	cameraPlaneNormal.normalize3();
	// as cameraView.X/Y/Z = A/B/C of the Hessian Normal Form (HNF), we need D and we know that |cameraView| == 0.0, we get:
	// and use HNF to get the distance:
	double cameraPlaneD = -compMult( mCameraCenter, cameraPlaneNormal ).sum3();

	double     bBoxRadius;
	Vector3D   bBoxCenter( 0.0f, 0.0f, 0.0f, 1.0 );
	if( mMeshVisual == nullptr ) {
		bBoxRadius = 1.0f;
		cout << "[MeshWidget::" << __FUNCTION__ << "] No mesh present using defaults." << endl;
	} else {
		bBoxRadius   = mMeshVisual->getBoundingBoxRadius();
		bBoxCenter   = mMeshVisual->getBoundingBoxCenter();
	}
	Vector3D   pointNear    = bBoxCenter - ( cameraPlaneNormal * bBoxRadius );
	Vector3D   pointFar     = bBoxCenter + ( cameraPlaneNormal * bBoxRadius );

	GLdouble zNear = compMult( pointNear, cameraPlaneNormal ).sum3() + cameraPlaneD;
	GLdouble zFar  = compMult( pointFar,  cameraPlaneNormal ).sum3() + cameraPlaneD;
	//cout << "[MeshWidget::setView] distNear: " << zNear << endl;
	//cout << "[MeshWidget::setView] distFar: " << zFar << endl;
	//float distTest  = compMult( mCameraCenter,  cameraPlaneNormal ).sum3() + cameraPlaneD;
	//cout << "[MeshWidget::setView] distTest: " << distTest << endl;

	// in case we get really close to the bounding box and eventually any surface detail
	if( zNear <= 1.0 ) {
		zNear = 1.0;
	}

	//! Switch between orthographic and perspective projection.
	mMatProjection.setToIdentity();
	if( mParamFlag[ORTHO_MODE] ) {
		if( rOrthoViewPort != nullptr ) {
			//glOrtho( rOrthoViewPort[0], rOrthoViewPort[1], rOrthoViewPort[2], rOrthoViewPort[3], zNear, zFar );
			mMatProjection.ortho( rOrthoViewPort[0], rOrthoViewPort[1], rOrthoViewPort[2], rOrthoViewPort[3], zNear, zFar );
		} else {
			double orthoZoom;
			double shiftHori;
			double shiftVert;
			getParamFloatMeshWidget( ORTHO_ZOOM, &orthoZoom );
			getParamFloatMeshWidget( ORTHO_SHIFT_HORI, &shiftHori );
			getParamFloatMeshWidget( ORTHO_SHIFT_VERT, &shiftVert );
			//glOrtho( -bBoxRadius*orthoZoom +shiftHori, bBoxRadius*orthoZoom +shiftHori, -(bBoxRadius*orthoZoom/windowRatio) +shiftVert, (bBoxRadius*orthoZoom/windowRatio) +shiftVert, zNear, zFar );
			mMatProjection.ortho( -bBoxRadius*orthoZoom +shiftHori, bBoxRadius*orthoZoom +shiftHori, 
			                      -(bBoxRadius*orthoZoom/windowRatio) +shiftVert, 
			                       (bBoxRadius*orthoZoom/windowRatio) +shiftVert, zNear, zFar );
		}
		//PRINT_OPENGL_ERROR( "glOrtho( ... )" );
	} else {
		//gluPerspective( mParamFlt[FOV_ANGLE], windowRatio, zNear, zFar );
		//PRINT_OPENGL_ERROR( "gluPerspective( ... )" );
		mMatProjection.perspective( mParamFlt[FOV_ANGLE], windowRatio, zNear, zFar );
	}
#ifdef DEAD_CORE_PROFILE
	float* matProjectionRAW = mMatProjection.data();
	glMatrixMode( GL_PROJECTION );
	PRINT_OPENGL_ERROR( "glMatrixMode( GL_PROJECTION )" );
	glLoadMatrixf( matProjectionRAW );
	PRINT_OPENGL_ERROR( "glLoadMatrixd( matProjectionRAW );" );
#endif

//	//! Initalize modelview matrix.
//	glMatrixMode( GL_MODELVIEW );
//	PRINT_OPENGL_ERROR( "glMatrixMode( GL_MODELVIEW )" );
//	glLoadIdentity();
//	PRINT_OPENGL_ERROR( "glLoadIdentity()" );

	//! Set point to look at and the viewport.
	//gluLookAt( mCameraCenter.getX(), mCameraCenter.getY(), mCameraCenter.getZ(), mCenterView.getX(), mCenterView.getY(), mCenterView.getZ(), mCameraUp.getX(), mCameraUp.getY(), mCameraUp.getZ() );
	//PRINT_OPENGL_ERROR( "gluLookAt( ... )" );
	setViewModelMat();
#ifdef DEAD_CORE_PROFILE
	float* matModelViewRAW = mMatModelView.data();
	glMatrixMode( GL_MODELVIEW );
	PRINT_OPENGL_ERROR( "glMatrixMode( GL_MODELVIEW )" );
	glLoadMatrixf( matModelViewRAW );
	PRINT_OPENGL_ERROR( "glLoadMatrixd( matModelViewRAW );" );
#endif

	// Update of the sidebar
	if( mParamFlag[ORTHO_MODE] ) {
		double dpi;
		if( getViewPortDPI(dpi) ) {
			emit sViewPortInfo( VPINFO_DPI, QString::number( dpi, 'f', 2 ) );
		} else {
			emit sViewPortInfo( VPINFO_DPI, QString( "err" ) );
		}
	} else {
		emit sViewPortInfo( VPINFO_DPI, QString( "n.a." ) );
	}
}

//! Setup of the Modelview matrix used by the shaders.
//! See MeshWidget::setView and MeshWidget::setViewInitial
void MeshWidget::setViewModelMat() {
	QVector3D vecEye(    mCameraCenter.getX(), mCameraCenter.getY(), mCameraCenter.getZ() );
	QVector3D vecCenter( mCenterView.getX(),   mCenterView.getY(),   mCenterView.getZ() );
	QVector3D vecUp(     mCameraUp.getX(),     mCameraUp.getY(),     mCameraUp.getZ() );
	mMatModelView.setToIdentity();
	mMatModelView.lookAt( vecEye, vecCenter, vecUp );
}

//! Overwritten method using a manual set zoom factor.
//! Sort of a fix for HighDPI displays with scaling.
//! Windows does not seem to be affected.
int MeshWidget::height() const {
	double scaleFactor = 1.0;
	getParamFloatMeshWidget( HIGHDPI_ZOOM_FACTOR, &scaleFactor );
	return QGLWidget::height()*scaleFactor;
}

//! Overwritten method using a manual set zoom factor.
//! Sort of a fix for HighDPI displays with scaling.
//! Windows does not seem to be affected.
int MeshWidget::width() const {
	double scaleFactor = 1.0;
	getParamFloatMeshWidget( HIGHDPI_ZOOM_FACTOR, &scaleFactor );
	return QGLWidget::width()*scaleFactor;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
//calclulates the PlaneHNF given the camera-center and the view-center
bool MeshWidget::setPlaneHNFByView()
{
    //calculate normal of the plane.
    //Alternatively, take vector between CameraCenter and Mesh-Center, then the plane is guaranteed to cut the mesh,
    //but then it is not guaranteed to be oriented towards the camera...
    Vector3D pHNF = mCenterView - mCameraCenter;
    pHNF.normalize3();
    //get distance to the plane to the origin, which is simply the dot-product
    pHNF.setH(-dot3(pHNF, mCenterView));

    //probably better to use the signal/slot system...
    mMeshVisual->setPlaneHNF(pHNF);

    emit sParamFlagMesh(MeshGLParams::SHOW_MESH_PLANE, true);
    update();
    return true;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// implementation of helper class for offscreen buffer

MeshWidget::OffscreenBuffer::OffscreenBuffer(QGLContext* context) : mContext(context)
{
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	mTexWidth = viewportSize[2];
	mTexHeight = viewportSize[3];

	mColorTextureBuffer = new unsigned char[mTexWidth * mTexHeight * 3];
	mDepthTextureBuffer = new float[mTexWidth * mTexHeight];

	using PglGenFramebuffers = void (*)(GLsizei, GLuint *);
	using PglBindFramebuffer = void (*)(GLenum, GLuint);
	using PglFramebufferTexture2D = void (*)(GLenum, GLenum, GLenum, GLuint, GLint);

	PglGenFramebuffers genFramebuffers = reinterpret_cast<PglGenFramebuffers>(mContext->getProcAddress("glGenFramebuffers"));
	PglBindFramebuffer bindFramebuffer = reinterpret_cast<PglBindFramebuffer>(mContext->getProcAddress("glBindFramebuffer"));
	PglFramebufferTexture2D framebufferTexture2D = reinterpret_cast<PglFramebufferTexture2D>(mContext->getProcAddress("glFramebufferTexture2D"));

	genFramebuffers(1, &mFboID);

	bindFramebuffer(GL_FRAMEBUFFER, mFboID);

	glGenTextures(1, &mColorTextureID);
	glBindTexture(GL_TEXTURE_2D, mColorTextureID);
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mTexWidth, mTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorTextureID, 0);

	glGenTextures(1, &mDepthTextureID);
	glBindTexture(GL_TEXTURE_2D, mDepthTextureID);
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, mTexWidth, mTexHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	framebufferTexture2D(GL_FRAMEBUFFER,  GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTextureID, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

MeshWidget::OffscreenBuffer::~OffscreenBuffer()
{
	using PglBindFramebuffer = void (*)(GLenum, GLuint);
	using PglDeleteFramebuffers = void (*)(GLsizei, GLuint *);

	PglBindFramebuffer bindFramebuffer = reinterpret_cast<PglBindFramebuffer>(mContext->getProcAddress("glBindFramebuffer"));
	PglDeleteFramebuffers deleteFramebuffer = reinterpret_cast<PglDeleteFramebuffers>(mContext->getProcAddress("glDeleteFramebuffers"));

	bindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &mColorTextureID);
	glDeleteTextures(1, &mDepthTextureID);

	deleteFramebuffer(1, &mFboID);

	delete[] mColorTextureBuffer;
	delete[] mDepthTextureBuffer;
}

GLuint MeshWidget::OffscreenBuffer::getFboID()
{
	return mFboID;
}

//! Copies the contents of the color-texture into data. The array size is of width * height * 3
//! @return data Pointer with the Colorbuffer data. => do not delete this, this pointer is managed by this class!
//! @param width Return parameter of the color-textures width
//! @param height Return parameter of the color-textures height
unsigned char* MeshWidget::OffscreenBuffer::getColorTexture(int &width, int &height)
{
	glBindTexture(GL_TEXTURE_2D, mColorTextureID);

	width = mTexWidth;
	height = mTexHeight;

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, mColorTextureBuffer);

	glBindTexture(GL_TEXTURE_2D, 0);
	return mColorTextureBuffer;
}

void MeshWidget::OffscreenBuffer::getColorTextureRegion(unsigned char *&data, int width, int height, int xOffset, int yOffset)
{
	if(xOffset + width > mTexWidth || yOffset + height > mTexHeight || xOffset < 0 || yOffset < 0 || width <= 0 || height <= 0)
		return;

	unsigned char* tempData;
	int tempWidth, tempHeight;

	tempData = getColorTexture(tempWidth, tempHeight);

	//copy subregion of width/height into data
	data = new unsigned char[width * height * 3];
	size_t targetIndex = 0;
	for(int y = 0; y<height; ++y)
	{
		std::copy(&tempData[(yOffset + y) * (mTexWidth * 3) + xOffset * 3] , &tempData[(yOffset + y) * (mTexWidth * 3) + xOffset  * 3 + width * 3], &data[targetIndex]);
		targetIndex += width * 3;
	}
}

//! Copies the contents of the depth-texture into data. The array size is of width * height
//! @return data Pointer to store the imagedata into.
//! @param width Return parameter of the depth-textures width
//! @param height Return parameter of the depth-textures height
float* MeshWidget::OffscreenBuffer::getDepthTexture(int &width, int &height)
{

	glBindTexture(GL_TEXTURE_2D, mDepthTextureID);

	width = mTexWidth;
	height = mTexHeight;

	glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, mDepthTextureBuffer);

	glBindTexture(GL_TEXTURE_2D, 0);

	return mDepthTextureBuffer;
}

void MeshWidget::OffscreenBuffer::getDepthTextureRegion(float *&data, int width, int height, int xOffset, int yOffset)
{
	if(xOffset + width > mTexWidth || yOffset + height > mTexHeight || xOffset < 0 || yOffset < 0 || width <= 0 || height <= 0)
		return;

	float* tempData;
	int tempWidth, tempHeight;

	tempData = getDepthTexture(tempWidth, tempHeight);

	//copy subregion of width/height into data
	data = new float[width * height];
	size_t targetIndex = 0;
	for(int y = 0; y<height; ++y)
	{
		std::copy(&tempData[(yOffset + y) * mTexWidth + xOffset] , &tempData[(yOffset + y) * mTexWidth + xOffset + width], &data[targetIndex]);
		targetIndex += width;
	}
}

//! Set the active section plane for interactive editing
//! @param section Pointer to the section to edit, or nullptr to deactivate
void MeshWidget::setActiveSectionPlane(Section* section)
{
	mActiveSectionPlane = section;
	update(); // Trigger repaint
}

// ============================================================================
// v4 Phase 1: Align - ViewPoint 6개
// ============================================================================

//! 하면 뷰 설정 (Bottom view)
void MeshWidget::setViewBottom() {
	if (mMeshVisual == nullptr) {
		return;
	}

	double bBoxRadius = mMeshVisual->getBoundingBoxRadius();
	double distToCamera = bBoxRadius / (tan(M_PI * mParamFlt[FOV_ANGLE] / 360.0));

	mCenterView = mMeshVisual->getBoundingBoxCenter();
	mCameraCenter = Vector3D(0.0, -distToCamera, 0.0, 0.0) + mCenterView;
	mCameraUp = Vector3D(0.0, 0.0, +1.0, 0.0);  // Z축이 위

	update();
}

//! 후면 뷰 설정 (Back view)
void MeshWidget::setViewBack() {
	if (mMeshVisual == nullptr) {
		return;
	}

	double bBoxRadius = mMeshVisual->getBoundingBoxRadius();
	double distToCamera = bBoxRadius / (tan(M_PI * mParamFlt[FOV_ANGLE] / 360.0));

	mCenterView = mMeshVisual->getBoundingBoxCenter();
	mCameraCenter = Vector3D(0.0, 0.0, -distToCamera, 0.0) + mCenterView;
	mCameraUp = Vector3D(0.0, +1.0, 0.0, 0.0);  // Y축이 위

	update();
}

//! 좌측면 뷰 설정 (Left view)
void MeshWidget::setViewLeft() {
	if (mMeshVisual == nullptr) {
		return;
	}

	double bBoxRadius = mMeshVisual->getBoundingBoxRadius();
	double distToCamera = bBoxRadius / (tan(M_PI * mParamFlt[FOV_ANGLE] / 360.0));

	mCenterView = mMeshVisual->getBoundingBoxCenter();
	mCameraCenter = Vector3D(-distToCamera, 0.0, 0.0, 0.0) + mCenterView;
	mCameraUp = Vector3D(0.0, +1.0, 0.0, 0.0);  // Y축이 위

	update();
}

// ============================================================================
// v4 Phase 1: Align - Rotation 변환
// ============================================================================

//! X축 중심 90도 회전
void MeshWidget::rotateAroundX() {
	if (mMeshVisual == nullptr) {
		return;
	}

	std::vector<double> angle90 = {M_PI / 2.0};
	Matrix4D rotMatrix(Matrix4D::INIT_ROTATE_ABOUT_X, &angle90);
	mMeshVisual->applyTransformationToWholeMesh(rotMatrix, true);

	update();
}

//! Y축 중심 90도 회전
void MeshWidget::rotateAroundY() {
	if (mMeshVisual == nullptr) {
		return;
	}

	std::vector<double> angle90 = {M_PI / 2.0};
	Matrix4D rotMatrix(Matrix4D::INIT_ROTATE_ABOUT_Y, &angle90);
	mMeshVisual->applyTransformationToWholeMesh(rotMatrix, true);

	update();
}

//! Z축 중심 90도 회전
void MeshWidget::rotateAroundZ() {
	if (mMeshVisual == nullptr) {
		return;
	}

	std::vector<double> angle90 = {M_PI / 2.0};
	Matrix4D rotMatrix(Matrix4D::INIT_ROTATE_ABOUT_Z, &angle90);
	mMeshVisual->applyTransformationToWholeMesh(rotMatrix, true);

	update();
}

// ============================================================================
// v4 Phase 1: Align - Ground Plane
// ============================================================================

//! 메시를 Ground Plane (Z=0)에 정렬
void MeshWidget::fitMeshToGroundPlane() {
	if (mMeshVisual == nullptr) {
		return;
	}

	// Bounding box의 최저점 (A, B, C, D 중 Z가 가장 작은 값)을 Z=0으로 이동
	Vector3D bBoxCenter = mMeshVisual->getBoundingBoxCenter();
	Vector3D bBoxSize;
	mMeshVisual->getBoundingBoxSize(bBoxSize);

	// Center에서 size/2를 빼면 최저점
	double minZ = bBoxCenter.getZ() - bBoxSize.getZ() / 2.0;

	std::vector<double> translation = {0.0, 0.0, -minZ};
	Matrix4D translateMatrix(Matrix4D::INIT_TRANSLATE, &translation);

	mMeshVisual->applyTransformationToWholeMesh(translateMatrix, false);

	update();
}

// === v4 Phase 2: Cutline (단면 라인 추출) ================================================

//! Extract cutline from mesh-plane intersection
//! Uses GigaMesh's calcIntersectionPolylineWithPlane (mesh.cpp:4041)
//! @param planeHNF Plane in Hessian Normal Form (A, B, C, D)
//! @param outPoints Output vector for intersection points
//! @return true if intersection found, false otherwise
bool MeshWidget::extractCutline(const Vector3D& planeHNF, std::vector<Vector3D>* outPoints) {
	if (mMeshVisual == nullptr || outPoints == nullptr) {
		return false;
	}

	return mMeshVisual->calcIntersectionPolylineWithPlane(planeHNF, outPoints);
}

// === Section Overlay Rendering (DongArch Arch3D Liner Style) ==========================

//! Set section polyline for 2D overlay rendering
//! @param screenPoints Screen-space 2D points of section cutline
void MeshWidget::setSectionPolyline(const QVector<QPointF>& screenPoints) {
	m_sectionPolylineScreen = screenPoints;
	m_showSectionOverlay = !screenPoints.isEmpty();
	update(); // Trigger paintEvent
	qDebug() << "[MeshWidget::setSectionPolyline]" << screenPoints.size() << "points received";
}

//! Set section polyline from 3D world coordinates
//! Automatically projects 3D points to screen space using current camera/viewport
//! @param worldPoints 3D world coordinates of section cutline
void MeshWidget::setSectionPolyline3D(const std::vector<Vector3D>& worldPoints) {
	if (worldPoints.empty()) {
		clearSectionOverlay();
		return;
	}

	// Get OpenGL transformation matrices
	GLdouble modelMatrix[16];
	GLdouble projMatrix[16];
	GLint viewport[4];

	makeCurrent();  // Ensure OpenGL context is active
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Project each 3D point to screen coordinates
	QVector<QPointF> screenPoints;
	screenPoints.reserve(worldPoints.size());

	for (const Vector3D& worldPt : worldPoints) {
		GLdouble winX, winY, winZ;

		// Use gluProject to transform 3D -> 2D
		// Note: gluProject uses doubles, Vector3D provides getX/Y/Z as doubles
		gluProject(
			worldPt.getX(), worldPt.getY(), worldPt.getZ(),
			modelMatrix, projMatrix, viewport,
			&winX, &winY, &winZ
		);

		// Convert OpenGL window coords (origin bottom-left) to Qt coords (origin top-left)
		double screenX = winX;
		double screenY = viewport[3] - winY;  // Flip Y axis

		screenPoints.append(QPointF(screenX, screenY));
	}

	// Use existing 2D setter
	setSectionPolyline(screenPoints);

	qDebug() << "[MeshWidget::setSectionPolyline3D] Projected" << worldPoints.size() << "3D points to screen";
}

//! Set section overlay rendering style
//! @param color Line color
//! @param width Line width in pixels
//! @param alpha Transparency (0.0 = transparent, 1.0 = opaque)
void MeshWidget::setSectionStyle(const QColor& color, float width, float alpha) {
	m_sectionLineColor = color;
	m_sectionLineWidth = width;
	m_sectionLineAlpha = std::clamp(alpha, 0.0f, 1.0f);
	update();
	qDebug() << "[MeshWidget::setSectionStyle] Color:" << color.name() << "Width:" << width << "Alpha:" << alpha;
}

//! Clear section overlay
void MeshWidget::clearSectionOverlay() {
	m_sectionPolylineScreen.clear();
	m_showSectionOverlay = false;
	update();
	qDebug() << "[MeshWidget::clearSectionOverlay] Overlay cleared";
}

// Note: renderSectionPolylines() is implemented in meshwidget_sections.cpp

