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

#include "qgmdocksidebar.h"
#include "ui_qgmdocksidebar.h"

#include "QGMMainWindow.h"
#include <QStyle>

using namespace std;

QGMDockSideBar::QGMDockSideBar( QWidget *parent ) :
	QDockWidget( parent ), ui( new Ui::QGMDockSideBar )
{
	ui->setupUi( this );

	QObject::connect( ui->radioButtonSolidColor,       SIGNAL(toggled(bool)), this, SLOT(texMapVertMono(bool))       );
	QObject::connect( ui->radioButtonVertexTex,        SIGNAL(toggled(bool)), this, SLOT(texMapVertRGB(bool))        );
	QObject::connect( ui->radioButtonFuncValVertTex,   SIGNAL(toggled(bool)), this, SLOT(texMapVertFuncVal(bool))    );
	QObject::connect( ui->radioButtonVertexLabel,      SIGNAL(toggled(bool)), this, SLOT(texMapVertLabels(bool))     );
	QObject::connect( ui->radioButtonWireframe,        SIGNAL(toggled(bool)), this, SLOT(shaderChoiceWireframe(bool))        );
	QObject::connect( ui->radioButtonNPRendering,      SIGNAL(toggled(bool)), this, SLOT(shaderChoiceNPR(bool))    );
	QObject::connect( ui->checkBoxArchaeologyMode,     SIGNAL(toggled(bool)), this, SLOT(archaeologyModeChanged(bool)) );
	QObject::connect( ui->checkBoxCortexRendering,     SIGNAL(toggled(bool)), this, SLOT(cortexRenderingChanged(bool)) );
	QObject::connect( ui->pushButtonSelectCortex,      SIGNAL(toggled(bool)), this, SLOT(selectCortexVerticesClicked(bool)) );
	QObject::connect( ui->pushButtonClearCortex,       SIGNAL(clicked()), this, SLOT(clearCortexSelectionClicked()) );
	QObject::connect( ui->radioButtonPointcloud,       SIGNAL(toggled(bool)), this, SLOT(shaderChoicePointCloud(bool)));
	QObject::connect( ui->radioButton_Solid,           SIGNAL(toggled(bool)), this, SLOT(shaderChoiceMonolithic(bool)));
	QObject::connect( ui->radioButtonTransparency,     SIGNAL(toggled(bool)), this, SLOT(shaderChoiceTransparency(bool)));
	QObject::connect( ui->radioButtonTextured,         SIGNAL(toggled(bool)), this, SLOT(shaderChoiceTextured(bool)));

	QObject::connect( this, SIGNAL(sShowParamIntMeshGL(MeshGLParams::eParamInt,int)), SLOT(updateMeshParamInt(MeshGLParams::eParamInt,int)) );

	QObject::connect( ui->pushButton_NPR, SIGNAL(pressed()), this, SIGNAL(sShowNPRSettings()));
	QObject::connect( ui->pushButton_transparent, SIGNAL(pressed()), this, SIGNAL(sShowTransparencySettings()));

	ui->pushButton_transparent->setIcon(ui->pushButton_transparent->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	ui->pushButton_NPR->setIcon(ui->pushButton_NPR->style()->standardIcon(QStyle::SP_FileDialogDetailedView));

}

//! Destructor
QGMDockSideBar::~QGMDockSideBar() {
	delete( ui );
}

//! Switch to MeshGL::TEXMAP_VERT_MONO
void QGMDockSideBar::texMapVertMono( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_MONO  );
	}
}

//! Switch to MeshGL::TEXMAP_VERT_RGB
void QGMDockSideBar::texMapVertRGB( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_RGB   );
	}
}

//! Switch to MeshGL::TEXMAP_VERT_FUNCVAL
void QGMDockSideBar::texMapVertFuncVal( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_FUNCVAL );
	}
}

//! Switch to MeshGL::TEXMAP_VERT_LABELS
void QGMDockSideBar::texMapVertLabels( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::TEXMAP_CHOICE_FACES, MeshGLParams::TEXMAP_VERT_LABELS );
	}
}

//! Switch to Wireframe rendering
void QGMDockSideBar::shaderChoiceWireframe( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE,       MeshGLParams::SHADER_WIREFRAME );
		ui->radioButtonFuncValVertTex->setEnabled(false);
		ui->radioButtonVertexLabel->setEnabled(false);
		ui->radioButtonVertexTex->setEnabled(false);
		ui->radioButtonSolidColor->setEnabled(false);
	}
	else
	{
		ui->radioButtonFuncValVertTex->setEnabled(true);
		ui->radioButtonVertexLabel->setEnabled(true);
		ui->radioButtonVertexTex->setEnabled(true);
		ui->radioButtonSolidColor->setEnabled(true);
	}
}

//! Switch to NPR-Rendering
void QGMDockSideBar::shaderChoiceNPR( bool rState ) {
	if( rState ) {
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_NPR );
		ui->radioButtonFuncValVertTex->setEnabled(false);
		ui->radioButtonVertexLabel->setEnabled(false);
		ui->radioButtonVertexTex->setEnabled(false);
		ui->radioButtonSolidColor->setEnabled(false);
	}
	else
	{
		ui->radioButtonFuncValVertTex->setEnabled(true);
		ui->radioButtonVertexLabel->setEnabled(true);
		ui->radioButtonVertexTex->setEnabled(true);
		ui->radioButtonSolidColor->setEnabled(true);
	}
}

//! Switch to pointcloud rendering
void QGMDockSideBar::shaderChoicePointCloud(bool rState)
{
	if(rState)
	{
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_POINTCLOUD);
	}
}

//! Swtich to monolithic rendering
void QGMDockSideBar::shaderChoiceMonolithic(bool rState)
{
	if(rState)
	{
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_MONOLITHIC);
	}
}

void QGMDockSideBar::shaderChoiceTransparency(bool rState)
{
	if(rState)
	{
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_TRANSPARENCY);
	}
}

void QGMDockSideBar::shaderChoiceTextured(bool rState)
{
	if(rState)
	{
		emit sShowParamIntMeshGL( MeshGLParams::SHADER_CHOICE, MeshGLParams::SHADER_TEXTURED);
		ui->radioButtonFuncValVertTex->setEnabled(false);
		ui->radioButtonVertexLabel->setEnabled(false);
		ui->radioButtonVertexTex->setEnabled(false);
		ui->radioButtonSolidColor->setEnabled(false);
	}
	else
	{
		ui->radioButtonFuncValVertTex->setEnabled(true);
		ui->radioButtonVertexLabel->setEnabled(true);
		ui->radioButtonVertexTex->setEnabled(true);
		ui->radioButtonSolidColor->setEnabled(true);
	}
}

//! Enable/Disable archaeological illustration mode
void QGMDockSideBar::archaeologyModeChanged( bool rState ) {
	emit sShowFlagMeshWidget( MeshWidgetParams::ARCHAEOLOGY_MODE_ENABLED, rState );

	// Enable/disable cortex checkbox based on archaeological mode
	ui->checkBoxCortexRendering->setEnabled( rState );
	if( !rState ) {
		ui->checkBoxCortexRendering->setChecked( false ); // Uncheck if archaeology mode disabled
	}

	// Enable/disable cortex selection buttons based on archaeological mode
	ui->pushButtonSelectCortex->setEnabled( rState );
	ui->pushButtonClearCortex->setEnabled( rState );
	if( !rState ) {
		ui->pushButtonSelectCortex->setChecked( false ); // Uncheck if archaeology mode disabled
	}

	// Automatically enable NPR rendering when archaeological mode is activated
	cout << "[QGMDockSideBar::archaeologyModeChanged] rState = " << rState << endl;
	if( rState ) {
		ui->radioButtonNPRendering->setChecked( true );
		// Automatically enable cortex vertex rendering for visual feedback
		cout << "[QGMDockSideBar::archaeologyModeChanged] Emitting SHOW_VERTICES_CORTEX = true" << endl;
		emit sShowParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_CORTEX, true );
	} else {
		// Disable cortex vertex rendering when archaeology mode is disabled
		cout << "[QGMDockSideBar::archaeologyModeChanged] Emitting SHOW_VERTICES_CORTEX = false" << endl;
		emit sShowParamFlagMeshGL( MeshGLParams::SHOW_VERTICES_CORTEX, false );
	}
}

//! Enable/Disable cortex cross-hatch rendering
void QGMDockSideBar::cortexRenderingChanged( bool rState ) {
	emit sShowFlagMeshWidget( MeshWidgetParams::CORTEX_RENDERING_ENABLED, rState );
}

//! Toggle cortex vertex selection mode
void QGMDockSideBar::selectCortexVerticesClicked( bool rState ) {
	// Update button appearance based on state
	if( rState ) {
		ui->pushButtonSelectCortex->setText( "선택 중..." );
	} else {
		ui->pushButtonSelectCortex->setText( "피질 선택" );
	}

	// Emit signal to MeshWidget to enter/exit vertex selection mode
	emit sEnterCortexSelectionMode( rState );
}

//! Clear all cortex vertex selections
void QGMDockSideBar::clearCortexSelectionClicked() {
	// Emit signal to MeshWidget/MeshGL to clear cortex flags
	emit sClearCortexSelection();
}

//! Sets menu items according to the flags of MeshGL::viewParamsIntArr
void QGMDockSideBar::updateMeshParamInt(
        MeshGLParams::eParamInt rParamNr, 
        int rSetValue 
) {
	// Setup radio buttons
	ui->radioButtonFuncValVertTex->setEnabled(true);
	ui->radioButtonVertexLabel->setEnabled(true);
	ui->radioButtonVertexTex->setEnabled(true);
	ui->radioButtonSolidColor->setEnabled(true);

	// Setup parameters
	switch( rParamNr ) {
	case MeshGLParams::SHADER_CHOICE:
		switch( static_cast<MeshGLParams::eShaderChoice>(rSetValue) ) {
			case MeshGLParams::SHADER_MONOLITHIC:
				ui->radioButton_Solid->setChecked( true );
				break;
			case MeshGLParams::SHADER_WIREFRAME:
				ui->radioButtonWireframe->setChecked( true );
				ui->radioButtonFuncValVertTex->setEnabled( false );
				ui->radioButtonVertexLabel->setEnabled( false );
				ui->radioButtonVertexTex->setEnabled( false );
				ui->radioButtonSolidColor->setEnabled( false );
				break;
			case MeshGLParams::SHADER_NPR:
				ui->radioButtonNPRendering->setChecked( true );
				ui->radioButtonFuncValVertTex->setEnabled( false );
				ui->radioButtonVertexLabel->setEnabled( false );
				ui->radioButtonVertexTex->setEnabled( false );
				ui->radioButtonSolidColor->setEnabled( false );
				break;
		case MeshGLParams::SHADER_POINTCLOUD:
				ui->radioButtonPointcloud->setChecked ( true );
				break;
		case MeshGLParams::SHADER_TRANSPARENCY:
				ui->radioButtonTransparency->setChecked( true );
				break;
		case MeshGLParams::SHADER_TEXTURED:
				ui->radioButtonTextured->setChecked( true );
				break;
			default:
				std::cerr << "[QGMDockSideBar::" << __FUNCTION__ << "] ERROR: parameter SHADER_CHOICE has an unknown choice: " << rSetValue << "!" << std::endl;
		}
		break;
		case MeshGLParams::TEXMAP_CHOICE_FACES:
			switch( static_cast<MeshGLParams::eTexMapType>(rSetValue) ) {
				case MeshGLParams::TEXMAP_VERT_MONO:
					ui->radioButtonSolidColor->setChecked( true );
					break;
				case MeshGLParams::TEXMAP_VERT_RGB:
					ui->radioButtonVertexTex->setChecked( true );
					break;
				case MeshGLParams::TEXMAP_VERT_FUNCVAL:
					ui->radioButtonFuncValVertTex->setChecked( true );
					break;
				case MeshGLParams::TEXMAP_VERT_LABELS:
					ui->radioButtonVertexLabel->setChecked( true );
					break;
				default:
					std::cerr << "[QGMDockSideBar::" << __FUNCTION__ << "] ERROR: parameter TEXMAP_CHOICE has an unknown choice: " << rSetValue << "!" << std::endl;
			}
			break;
		case MeshGLParams::TEXMAP_CHOICE_VERETX_SPRITES:
		case MeshGLParams::GLSL_COLMAP_CHOICE:
		case MeshGLParams::FUNCVAL_CUTOFF_CHOICE:
		case MeshGLParams::VERTEX_SPRITE_SHAPE:
		case MeshGLParams::VIEWPARAMS_INT_UNDEFINED:
		case MeshGLParams::COLMAP_LABEL_OFFSET:
		case MeshGLParams::PARAMS_INT_COUNT:
		//NPR-params
		case MeshGLParams::NPR_HATCH_SOURCE:
		case MeshGLParams::NPR_HATCH_STYLE:
		case MeshGLParams::NPR_OUTLINE_SOURCE:
		case MeshGLParams::NPR_TOON_SOURCE:
		case MeshGLParams::NPR_TOON_TYPE:
		case MeshGLParams::NPR_TOON_LIGHTING_STEPS:
		case MeshGLParams::NPR_TOON_HUE_STEPS:
		case MeshGLParams::NPR_TOON_SAT_STEPS:
		case MeshGLParams::NPR_TOON_VAL_STEPS:
		case MeshGLParams::DEFAULT_FRAMEBUFFER_ID:
		//transparency params
		case MeshGLParams::TRANSPARENCY_NUM_LAYERS:
		case MeshGLParams::TRANSPARENCY_TRANS_FUNCTION:
		case MeshGLParams::TRANSPARENCY_SEL_LABEL:
		case MeshGLParams::TRANSPARENCY_OVERFLOW_HANDLING:
		case MeshGLParams::TRANSPARENCY_BUFFER_METHOD:
        //mesh cleaning parameter
        case MeshGLParams::MAX_VERTICES_HOLE_FILLING:
			// nothing to do.
			break;
		default:
			std::cerr << "[QGMDockSideBar::" << __FUNCTION__ << "] ERROR: unsupported/unimplemented parameter no: " << rParamNr << "!" << std::endl;
	}
}

void QGMDockSideBar::enableTextureMeshRendering(bool enable)
{
	ui->radioButtonTextured->setEnabled(enable);
}


void QGMDockSideBar::changeEvent(QEvent* event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QDockWidget::changeEvent(event);
}
