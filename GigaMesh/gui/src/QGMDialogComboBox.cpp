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

#include "QGMDialogComboBox.h"

// Qt includes:
#include "QGMMacros.h"

//! Constructor
QGMDialogComboBox::QGMDialogComboBox( QWidget *parent, Qt::WindowFlags flags ) :
    QDialog( parent, flags ) {
	// Init UI elements:
	setupUi( this );
	// Set logo:
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
}

//! Set the decriptive text label for the combo-box.
void QGMDialogComboBox::setTextLabel( const QString & rLabelText ) {
	mLabelText->setText( rLabelText );
}

//! Adds an entry to the combo-box.
void QGMDialogComboBox::addItem( const QString & rText, const QVariant & rUserData ) {
	mComboBox->addItem( rText, rUserData );
}

//! Returns the selected item. E.g. the value of an enumerator.
QVariant QGMDialogComboBox::getSelectedItem() {
	int index = mComboBox->currentIndex();
	QVariant userData = mComboBox->itemData ( index );
	return userData;
}


void QGMDialogComboBox::changeEvent(QEvent* event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		retranslateUi(this);
	}

	QDialog::changeEvent(event);
}
