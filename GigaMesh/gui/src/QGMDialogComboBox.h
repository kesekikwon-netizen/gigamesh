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

#ifndef QGMDIALOGCOMBOBOX_H
#define QGMDIALOGCOMBOBOX_H

// generic C++ includes:
//#include <iostream>

// C++ includes:
#include <GigaMesh/mesh/gmcommon.h>

// generic Qt includes:
#include <QAction>
#include <QDialog>
#include <qstring.h>
//#include <qpalette.h> // for setting colors of e.g. textboxes

// Qt Interface includes:
#include "ui_dialogComboBox.h"

//!
//! \brief Dialog class for simple user input having a CommboBox and a TextLabel.
//!
//! Emits signal with the ID and/or of the select item.
//! Disconnects itself so it can be used for other purposes.
//!
//! Layer 2
//!

class QGMDialogComboBox : public QDialog, private Ui::QGMDialogComboBox {
    Q_OBJECT

public:
    explicit QGMDialogComboBox( QWidget *parent = nullptr, Qt::WindowFlags flags = {} );
	// Setup -----------------------------------------------------------------------------------------------------------------------------------------------
	void setTextLabel( const QString & rLabelText );
	void addItem( const QString & text, const QVariant & userData = QVariant() );

	// Retrieve selected information -----------------------------------------------------------------------------------------------------------------------
	QVariant getSelectedItem();

signals:

public slots:


	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDIALOGCOMBOBOX_H
