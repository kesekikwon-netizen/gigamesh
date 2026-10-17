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

#include "QGMDialogCutOffValues.h"

QGMDialogCutOffValues::QGMDialogCutOffValues( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ) {
	//! Constructor
	setupUi( this );
	// Set logo
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
}

void QGMDialogCutOffValues::accept() {
	//! Checks if all the input values are correct. If the values seem to be correct
	//! a signal is emitted, which should be connected MeshQt.
	//cout << "[QGMDialogCutOffValues::accept] lineVertex: " << lineRadius->text().toStdString() << endl;

	bool  convertOk;
	bool  doNotAccept = false;

	double minVal = lineMin->text().toDouble( &convertOk );
	if( !convertOk ){
		QSETCOLOR( lineMin, 255, 128, 128 ); // light read
		doNotAccept = true;
	} else {
		QSETCOLOR( lineMin, 255, 255, 255 ); // white
	}
	//cout << "[QGMDialogCutOffValues::accept] lineMin: " << minVal << endl;

	double maxVal = lineMax->text().toDouble( &convertOk );
	if( !convertOk ) {
		QSETCOLOR( lineMax, 255, 128, 128 ); // light read
		doNotAccept = true;
	} else {
		QSETCOLOR( lineMax, 255, 255, 255 ); // white
	}
	//cout << "[QGMDialogCutOffValues::accept] lineMax: " << maxVal << endl;

	if( doNotAccept ) {
		return;
	}

	emit cutOffValues( minVal, maxVal, checkNAN->isChecked() );

	//! Disconnects all connections - so other other objectrrs can reuse an instance of this class.
	QObject::disconnect();
	QDialog::accept();
}

void QGMDialogCutOffValues::changeEvent(QEvent*event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		retranslateUi(this);
	}

	QDialog::changeEvent(event);
}
