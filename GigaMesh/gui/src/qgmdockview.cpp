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

#include "qgmdockview.h"
#include "ui_qgmdockview.h"

//! Constructor.
QGMDockView::QGMDockView( QWidget *parent ) :
        QDockWidget( parent ),
        ui( new Ui::QGMDockView ) {
	ui->setupUi(this);
}

//! Destructor.
QGMDockView::~QGMDockView() {
	delete ui;
}

//! Handles incoming informations of the MeshWidget.
void QGMDockView::viewPortInfo( const MeshWidgetParams::eViewPortInfo rInfoID, const QString& rInfoString ) {
	switch( rInfoID ) {
		case MeshWidgetParams::VPINFO_FRAMES_PER_SEC:
			ui->labelFPSValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_DPI:
			ui->labelDPIValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_LIGHT_CAM:
			ui->labelLightCamValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_LIGHT_WORLD:
			ui->labelLightWorldValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_FUNCTION_VALUE:
			ui->labelFuncValValue->setText( rInfoString );
			break;
		case MeshWidgetParams::VPINFO_LABEL_ID:
			ui->labelLabelValue->setText( rInfoString );
			break;
	}
}

//! Handles incoming informations of the MeshQt.
void QGMDockView::infoMesh( const MeshGLParams::eInfoMesh rInfoID, const QString& rInfoString ) {
	switch( rInfoID ) {
		case MeshGLParams::MGLINFO_SELECTED_PRIMITIVE:
			ui->labelSelPrimValue->setText( rInfoString );
			break;
		case MeshGLParams::MGLINFO_SELECTED_VERTICES:
			ui->labelSelMVertsValue->setText( rInfoString );
			break;
		case MeshGLParams::MGLINFO_SELECTED_FACES:
			ui->labelSelMFacesValue->setText( rInfoString );
			break;
		case MeshGLParams::MGLINFO_SELECTED_POLYLINES:
			ui->labelSelMPolysValue->setText( rInfoString );
			break;
		case MeshGLParams::MGLINFO_SELECTED_POSITIONS:
			ui->labelPositionsValue->setText( rInfoString );
			break;
	}
}


void QGMDockView::changeEvent(QEvent*event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QDockWidget::changeEvent(event);
}
