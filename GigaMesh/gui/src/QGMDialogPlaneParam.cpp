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

#include "QGMDialogPlaneParam.h"

using namespace std;

//! Constructor
QGMDialogPlaneParam::QGMDialogPlaneParam( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ) {
	setupUi( this );
	// Set logo
	QIcon gigaMeshLogo( _GIGAMESH_LOGO_ );
	setWindowIcon( gigaMeshLogo );
}

// --- SLOTS ---------------------------------------------------------------------------

//! Set Hesse Normal Form (HNF).
//! The method checks if there is also a HNF in the clipboard as string formated 'GigaMesh HNF 1 2 3 4'.
bool QGMDialogPlaneParam::setPlaneHNF( Vector3D rPlaneHNF ) {
	QClipboard *clipboard = QApplication::clipboard();
	QString clipBoardStr = clipboard->text( QClipboard::Clipboard );
	if( clipBoardStr.contains( QRegExp( "^GigaMesh HNF .*$" ) ) ) {
		clipBoardStr.remove( 0, 13 );
        QStringList someTokens  = clipBoardStr.split( " ", QString::SkipEmptyParts );
		if( someTokens.count() == 4 ) {
			bool useClipboard;
			bool userCancel;
			SHOW_QUESTION( tr("Use HNF from clipboard"), tr("Parameters: ")+clipBoardStr, useClipboard, userCancel );
			if( userCancel ) {
				return false;
			}
			if( useClipboard ) {
				// Set fields:
				lineEditA->setText( someTokens.at( 0 ) );
				lineEditB->setText( someTokens.at( 1 ) );
				lineEditC->setText( someTokens.at( 2 ) );
				lineEditD->setText( someTokens.at( 3 ) );
				return true;
			}
		}
		cerr << "[QGMDialogPlaneParam::" << __FUNCTION__ << "] ERROR: invalid numbers of plane parameters!" << endl;
	}
	// Set fields:
	lineEditA->setText( QString( "%1" ).arg( rPlaneHNF.getX() ) );
	lineEditB->setText( QString( "%1" ).arg( rPlaneHNF.getY() ) );
	lineEditC->setText( QString( "%1" ).arg( rPlaneHNF.getZ() ) );
	lineEditD->setText( QString( "%1" ).arg( rPlaneHNF.getH() ) );
	return true;
}

//! Setup UI for setting the HNF.
void QGMDialogPlaneParam::preferSet() {
	cbxDistComp->setChecked( false );
	cbxPlaneSet->setChecked( true );
	cbxPlaneSplit->setChecked( false );
	cbxPlaneSplitSep->setChecked( false );
}

//! Setup UI for visualizing the distance to the plane.
void QGMDialogPlaneParam::preferVisDist() {
	cbxDistComp->setChecked( true );
	cbxPlaneSet->setChecked( false );
	cbxPlaneSplit->setChecked( false );
	cbxPlaneSplitSep->setChecked( false );
}

//! Setup UI for splitting using the plane.
void QGMDialogPlaneParam::preferSplit() {
	cbxDistComp->setChecked( false );
	cbxPlaneSet->setChecked( false );
	cbxPlaneSplit->setChecked( true );
	cbxPlaneSplitSep->setChecked( true );
}

void QGMDialogPlaneParam::accept() {
	//! Checks if all the input values are correct. If the values seem to be correct
	//! a signal is emitted, which should be connected MeshQt.
	//cout << "[QGMMainWindow::accept] lineVertex: " << lineRadius->text().toStdString() << endl;

	float planeA;
	float planeB;
	float planeC;
	float planeD;

	bool  valuesBad = false;
	bool  convertOk;

	planeA = lineEditA->text().toFloat( &convertOk );
	if( !convertOk ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineEditA, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineEditA, 255, 255, 255 ); // white
	}

	planeB = lineEditB->text().toFloat( &convertOk );
	if( !convertOk ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineEditB, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineEditB, 255, 255, 255 ); // white
	}

	planeC = lineEditC->text().toFloat( &convertOk );
	if( !convertOk ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineEditC, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineEditC, 255, 255, 255 ); // white
	}

	planeD = lineEditD->text().toFloat( &convertOk );
	if( !convertOk ) { // internally we start indexing at 0 (not at 1 like in some file formats)
		QSETCOLOR( lineEditD, 255, 128, 128 ); // light read
		valuesBad = true;
	} else {
		QSETCOLOR( lineEditD, 255, 255, 255 ); // white
	}

	//! | ( A, B, C )^T | has to be > 0.0!
	if( ( pow(planeA,2)+pow(planeB,2)+pow(planeC,2) ) <= 0.0 ) {
		valuesBad = true;
		QSETCOLOR( lineEditA, 255, 128, 128 ); // light read
		QSETCOLOR( lineEditB, 255, 128, 128 ); // light read
		QSETCOLOR( lineEditC, 255, 128, 128 ); // light read
	}
	
	if( valuesBad ) {
		return;
	}

	Vector3D planeHNF( planeA, planeB, planeC, planeD );
	//cout << "[QGMDialogPlaneParam::" << __FUNCTION__ << "] Emit Signal." << endl;
	if( cbxDistComp->isChecked() ) {
		bool absDist = cbxDistAbs->isChecked();
		emit sComputeDistance( planeHNF, absDist );
	}
	if( cbxPlaneSet->isChecked() ) {
		emit sSetPlaneHNF( planeHNF );
	}
	if( cbxPlaneSplit->isChecked() ) {
		bool seperate = cbxPlaneSplitSep->isChecked();
		emit sSplitByPlane( planeHNF, seperate );
	}

	//! Disconnects all connections - so other other objects can reuse an instance of this class.
	QObject::disconnect();
	QDialog::accept();
}


void QGMDialogPlaneParam::changeEvent(QEvent* event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		retranslateUi(this);
	}

	QDialog::changeEvent(event);
}
