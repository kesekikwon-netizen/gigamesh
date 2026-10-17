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

#ifndef QGMDIALOGCUTOFFVALUES_H
#define QGMDIALOGCUTOFFVALUES_H

// generic C++ includes:
#include <iostream>

// C++ includes:
#include <GigaMesh/mesh/primitive.h>

// generic Qt includes:
#include <QtGui>
#include <QDialog>
#include <qstring.h>
#include <qpalette.h> // for setting colors of e.g. textboxes

// Qt Interface includes:
#include "ui_dialogCutOffValues.h"

// Qt includes:
#include "QGMMacros.h"

//!
//! \brief Dialog class for GigaMesh's Cut-Off-Feature-Elements function e.g. to remove outliers (Layer 2)
//!
//! ...
//!
//! Layer 2
//!

class QGMDialogCutOffValues : public QDialog, private Ui::dialogCutOffValues {
    Q_OBJECT

public:
	// Constructor and Destructor:
        QGMDialogCutOffValues( QWidget *parent = nullptr, Qt::WindowFlags flags = {} );

public slots:
	void accept();

signals:
	void cutOffValues(double,double,bool); //!< Cut-Off (e.g. removal of outliers) minVal, maxVal, setToNotANumber

private:


	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif
