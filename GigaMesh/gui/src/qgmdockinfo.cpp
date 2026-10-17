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

#include "qgmdockinfo.h"
#include "ui_qgmdockinfo.h"

#include <QTimer>
#include <iostream>  // v4: std::cerr for debug output

#define QGMDOCKINFOINITDEFAULTS                                               \
	ui( new Ui::QGMDockInfo ),                                            \
	mMouseMode( MeshWidgetParams::MOUSE_MODE_COUNT ) ,                    \
	mMouseModeNoControlKey( MeshWidgetParams::MOUSE_MODE_MOVE_CAMERA ),   \
	mGuideCommon( MeshWidgetParams::GUIDE_COMMON_NONE ),                  \
	mGuideSelection( MeshWidgetParams::GUIDE_SELECT_NONE )

using namespace std;

//! Constructor
QGMDockInfo::QGMDockInfo( QWidget *parent ) :
	QDockWidget( parent ), QGMDOCKINFOINITDEFAULTS {
	ui->setupUi( this );

	// Initialize comboBox for MouseMode
    ui->comboBoxMouseMode->addItem( tr("Move Camera")             , QVariant( MeshWidgetParams::MOUSE_MODE_MOVE_CAMERA            ) );
	ui->comboBoxMouseMode->addItem( tr("Move Plane")              , QVariant( MeshWidgetParams::MOUSE_MODE_MOVE_PLANE             ) );
	ui->comboBoxMouseMode->addItem( tr("Move Plane along Axis")   , QVariant(MeshWidgetParams::MOUSE_MODE_MOVE_PLANE_AXIS         ) );
	ui->comboBoxMouseMode->addItem( tr("Rotate Plane around Axis"), QVariant(MeshWidgetParams::MOUSE_MODE_ROTATE_PLANE_AXIS       ) );
	ui->comboBoxMouseMode->addItem( tr("Move Light 1 FixCamera")  , QVariant( MeshWidgetParams::MOUSE_MODE_MOVE_LIGHT_FIXED_CAM   ) );
	ui->comboBoxMouseMode->addItem( tr("Move Light 2 FixObject")  , QVariant( MeshWidgetParams::MOUSE_MODE_MOVE_LIGHT_FIXED_OBJECT) );
	ui->comboBoxMouseMode->addItem( tr("Selection")               , QVariant( MeshWidgetParams::MOUSE_MODE_SELECT                 ) );

	int selIndex = ui->comboBoxMouseMode->findData( QVariant( MeshWidgetParams::MOUSE_MODE_MOVE_CAMERA ) );
	ui->comboBoxMouseMode->setCurrentIndex(selIndex);

	// initalize guide:
	ui->labelGuide->setVisible( false );

	// initalize progress bar:
	ui->progressBar->setVisible( false );
	ui->progressLabel->setVisible( false );

	QObject::connect( this, SIGNAL(visibilityChanged(bool)), this, SLOT(visibility(bool)) );
	QObject::connect( ui->comboBoxMouseMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setMouseMode(int))   );

	// Version information
	//-------------------------------------------------------------------------------------------------------------------------
	QString buildInfo = "No Build Info";
#ifdef COMP_USER
	//cout << "[QGMDockInfo] Built by: " << COMP_USER << endl;
	//buildInfo = "Compiled by: " + QString().fromLocal8Bit( COMP_USER );
#else
	std::cerr << "[QGMDockInfo] ERROR: Build information missing!" << endl;
#endif
#ifdef COMP_DATE
	//cout << "[QGMDockInfo] ..... on: " << COMP_DATE << endl;
	buildInfo = "Build: " + QString().fromLocal8Bit( COMP_DATE );
#else
	std::cerr << "[QGMDockInfo] ERROR: Build information missing!" << endl;
#endif
	ui->labelBuild->setText( buildInfo );

#ifdef COMP_EDIT
	//cout << "[QGMDockInfo] .... for: " << COMP_EDIT << endl;
	//ui->labelEdition->setText( QString().fromLocal8Bit( COMP_EDIT ) );
#else
	std::cerr << "[QGMDockInfo] ERROR: Build information missing!" << endl;
#endif

#ifdef COMP_GITHEAD
	//cout << "[QGMDockInfo] .... git head: " << COMP_GITHEAD << endl;
#else
	std::cerr << "[QGMMainWindow] ERROR: Build information missing!" << endl;
#endif
}

//! Destructor
QGMDockInfo::~QGMDockInfo() {
	delete ui;
}

// --- Mouse Mode ----------------------------------------------------------------------------------------------------------------------------------------------

//! Select the next mouse-mode from the combobox -- typically connected to pressing the spacebar.
void QGMDockInfo::selectMouseModeDefault() {
	int selIndex = ui->comboBoxMouseMode->findData( QVariant( MeshWidgetParams::MOUSE_MODE_MOVE_CAMERA ) );

	// v4 Bug Fix: selIndex가 -1이면 crash 발생 (DongArch3D)
	if (selIndex == -1) {
		std::cerr << "[QGMDockInfo::" << __FUNCTION__ << "] WARNING: MOUSE_MODE_MOVE_CAMERA not found in comboBox!" << std::endl;
		return;
	}

	ui->comboBoxMouseMode->setCurrentIndex( selIndex );
	mMouseModeNoControlKey = QVariant( MeshWidgetParams::MOUSE_MODE_MOVE_CAMERA );
}

//! Select the alternate mouse-mode -- typically connected to the ctrl-key for selection.
void QGMDockInfo::selectMouseModeExtra( bool rActive, MeshWidgetParams::eMouseModes rMode ) {
	QSignalBlocker blockComboBox(ui->comboBoxMouseMode); //avoid sending setMouseMode from comboBoxMouseMode

	int selIndex = rActive ? ui->comboBoxMouseMode->findData( QVariant( rMode ) ) :
	                         ui->comboBoxMouseMode->findData( mMouseModeNoControlKey );

	// v4 Bug Fix: selIndex가 -1이면 crash 발생 (DongArch3D)
	if (selIndex == -1) {
		std::cerr << "[QGMDockInfo::" << __FUNCTION__ << "] WARNING: Mouse mode not found in comboBox. Mode: " << rMode << std::endl;
		return;
	}

	ui->comboBoxMouseMode->setCurrentIndex( selIndex );
	setMouseMode(selIndex);
}


//! Show the image for the user guide, when not in selection mode.
void QGMDockInfo::setGuideIDCommon( MeshWidgetParams::eGuideIDCommon rGuideID ) {
	//cout << "[QGMDockInfo::" << __FUNCTION__ << "] Selection Guide ID " << rGuideID << " !" << endl;
	mGuideCommon = rGuideID;
	if( mMouseMode == MeshWidgetParams::MOUSE_MODE_SELECT ) { //! \todo change to != when other mode guides are implemented
		return;
	}
	switch( rGuideID ) {
		case MeshWidgetParams::GUIDE_COMMON_NONE:
			ui->labelGuide->setVisible( false );
		break;
		// --- Cone -----------------------------------------------------------------------------------------
		case MeshWidgetParams::GUIDE_COMMON_CONE_DEFINED:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_cone_defined.svg") );
		break;
		//! \todo add GUIDE_ERROR treatment
		//case MeshParams::GUIDE_ERROR:
		//	ui->labelGuide->setVisible( false );
		//break;
		default:
			cerr << "[QGMDockInfo::" << __FUNCTION__ << "] ERROR: Undefined Guide ID " << rGuideID << " !" << endl;
			ui->labelGuide->setVisible( false );
	}
}

//! Show the image for the user guide in selection mode.
void QGMDockInfo::setGuideIDSelection( MeshWidgetParams::eGuideIDSelection rGuideID ) {
	//cout << "[QGMDockInfo::" << __FUNCTION__ << "] Selection Guide ID " << rGuideID << " !" << endl;
	mGuideSelection = rGuideID;
	if( mMouseMode != MeshWidgetParams::MOUSE_MODE_SELECT ) {
		return;
	}
	switch( rGuideID ) {
		case MeshWidgetParams::GUIDE_SELECT_NONE:
			ui->labelGuide->setVisible( false );
		break;
		// --- SelPrim ----------------------------------------------------------------------------------------
		case MeshWidgetParams::GUIDE_SELECT_SELPRIM_VERTEX: {
			QIcon tmpIcon( ":/GMToolBar/gigamesh_icon_vertices_select_single_vertex.svg" );
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( tmpIcon.pixmap( ui->labelGuide->size() ) );
		} break;
		case MeshWidgetParams::GUIDE_SELECT_SELPRIM_FACE: {
			QIcon tmpIcon( ":/GMToolBar/gigamesh_icon_vertices_select_single_face.svg" );
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( tmpIcon.pixmap( ui->labelGuide->size() ) );
		} break;
		// --- SelMVerts --------------------------------------------------------------------------------------
		case MeshWidgetParams::GUIDE_SELECT_SELMVERTS: {
			QIcon tmpIcon( ":/GMToolBar/gigamesh_icon_vertices_select_multiple_vertices.svg" );
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( tmpIcon.pixmap( ui->labelGuide->size() ) );
		} break;
		case MeshWidgetParams::GUIDE_SELECT_SELMVERTS_LASSO: {
			QIcon tmpIcon( ":/GMToolBar/gigamesh_icon_vertices_select_multiple_polyline.svg" );
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( tmpIcon.pixmap( ui->labelGuide->size() ) );
		} break;
        // --- Deselection SelMVerts --------------------------------------------------------------------------
        case MeshWidgetParams::GUIDE_DESELECT_SELMVERTS_LASSO: {
            QIcon tmpIcon( ":/GMToolBar/gigamesh_icon_vertices_deselect_multiple_polyline.svg" );
            ui->labelGuide->setVisible( true );
            ui->labelGuide->setPixmap( tmpIcon.pixmap( ui->labelGuide->size() ) );
        } break;
		// --- SelMFaces --------------------------------------------------------------------------------------
		case MeshWidgetParams::GUIDE_SELECT_SELMFACES: {
			QIcon tmpIcon( ":/GMToolBar/gigamesh_icon_vertices_select_multiple_faces.svg" );
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( tmpIcon.pixmap( ui->labelGuide->size() ) );
		} break;
		// --- Positions --------------------------------------------------------------------------------------
		case MeshWidgetParams::GUIDE_SELECT_POSITIONS: {
			QIcon tmpIcon( ":/GMToolBar/gigamesh_icon_vertices_select_multiple_positions.svg" );
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( tmpIcon.pixmap( ui->labelGuide->size() ) );
		} break;
        case MeshWidgetParams::GUIDE_SELECT_THREE_POSITIONS: {
            QIcon tmpIcon( ":/GMToolBar/gigamesh_icon_vertices_select_three_positions.svg" );
            ui->labelGuide->setVisible( true );
            ui->labelGuide->setPixmap( tmpIcon.pixmap( ui->labelGuide->size() ) );
        } break;
		// --- Plane  -----------------------------------------------------------------------------------------
		case MeshWidgetParams::GUIDE_SELECT_PLANE_3FP_A:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_plane_3fp_pa.png") );
		break;
		case MeshWidgetParams::GUIDE_SELECT_PLANE_3FP_B:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_plane_3fp_pb.png") );
		break;
		case MeshWidgetParams::GUIDE_SELECT_PLANE_3FP_C:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_plane_3fp_pc.png") );
		break;
		// --- Cone -----------------------------------------------------------------------------------------
		case MeshWidgetParams::GUIDE_SELECT_CONE_AXIS:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_cone_axis.svg") );
		break;
		case MeshWidgetParams::GUIDE_SELECT_CONE_PR1:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_cone_pr1.svg") );
		break;
		case MeshWidgetParams::GUIDE_SELECT_CONE_PR2:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_cone_pr2.svg") );
		break;
		// --- Sphere  -----------------------------------------------------------------------------------------
		case MeshWidgetParams::GUIDE_SELECT_SPHERE_P0:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_sphere_p0.png") );
		break;
		case MeshWidgetParams::GUIDE_SELECT_SPHERE_P1:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_sphere_p1.png") );
		break;
		case MeshWidgetParams::GUIDE_SELECT_SPHERE_P2:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_sphere_p2.png") );
		break;
		case MeshWidgetParams::GUIDE_SELECT_SPHERE_P3:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_sphere_p3.png") );
		break;
		case MeshWidgetParams::GUIDE_SELECT_SPHERE_P4:
			ui->labelGuide->setVisible( true );
			ui->labelGuide->setPixmap( QPixmap( ":/GMGeneric/select_sphere_p4.png") );
		break;
		//! \todo add GUIDE_ERROR treatment
		//case MeshParams::GUIDE_ERROR:
		//	ui->labelGuide->setVisible( false );
		//break;
		default:
			cerr << "[QGMDockInfo::" << __FUNCTION__ << "] ERROR: Undefined Guide ID " << rGuideID << " !" << endl;
			ui->labelGuide->setVisible( false );
	}
}

// --- Progress bar --------------------------------------------------------------------------------------------------------------------------------------------

//! Init progress bar
void QGMDockInfo::showProgressStart( const QString& rMsg ) {
	ui->progressBar->setVisible( true );
	ui->progressBar->setValue( 0 );
	ui->progressLabel->setText( rMsg );
	ui->progressLabel->setVisible( true );
}

//! Set progress bar value [0,1]
void QGMDockInfo::showProgress( const double rVal ) {
	ui->progressBar->setValue( rVal*100 );
}

//! Set progress finished
void QGMDockInfo::showProgressStop( const QString& rMsg ) {
	ui->progressBar->setValue( 100 );
	ui->progressLabel->setText( rMsg + ": DONE" );
	QTimer::singleShot( 3000, this, SLOT(showProgressStopReset()) );
	ui->GMLogo->setPixmap( QPixmap( ":/GMGeneric/GigaMesh_Logo_150px_green.png") );
}

// --- Private Slots -------------------------------------------------------------------------------------------------------------------------------------------

//! Handle the change of the combobox, which is responsible for the mouse mode.
void QGMDockInfo::setMouseMode( const int rComboBoxIdx ) {
	//cout << "[QGMDockInfo::" << __FUNCTION__ << "]" << endl;
	bool convertOk;
	int mouseMode = ui->comboBoxMouseMode->itemData( rComboBoxIdx ).toInt( &convertOk );
	if( !convertOk ) {
		cerr << "[QGMDockInfo::" << __FUNCTION__ << "] ERROR: conversion failed!" << endl;
		return;
	}

	mMouseMode = static_cast<MeshWidgetParams::eMouseModes>(mouseMode);
	switch( mMouseMode ) {
		case MeshWidgetParams::MOUSE_MODE_SELECT:
			setGuideIDSelection( mGuideSelection );
			break;
		default:
			setGuideIDCommon( mGuideCommon );
	}

	if(QObject::sender() == ui->comboBoxMouseMode)
	{
		mMouseModeNoControlKey = QVariant(mMouseMode);
	}

	emit sShowParamIntMeshWidget( MeshWidgetParams::MOUSE_MODE, mouseMode );
}

//! Discard progress
void QGMDockInfo::showProgressStopReset() {
	ui->progressBar->setVisible( false );
	ui->progressLabel->setVisible( false );
	ui->GMLogo->setPixmap( QPixmap( ":/GMGeneric/GigaMesh_Logo_150px.png") );
}

//! Actions undertaken, when hidden.
void QGMDockInfo::visibility( bool rVisible ) {
	emit sShowFlagMeshWidget( MeshWidgetParams::SHOW_GIGAMESH_LOGO_FORCED, not(rVisible) );
}


void QGMDockInfo::changeEvent(QEvent* event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QDockWidget::changeEvent(event);
}
