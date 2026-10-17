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

#ifndef QGMDialogPlaneParam_H
#define QGMDialogPlaneParam_H

// generic C++ includes:
#include <iostream>

// C++ includes:
#include <GigaMesh/mesh/gmcommon.h>
#include <GigaMesh/mesh/vector3d.h>

// generic Qt includes:
#include <QtGui>
#include <QDialog>
#include <qstring.h>

// OpenGL stuff
#define GL_GLEXT_PROTOTYPES
#if QT_VERSION >= 0x050000
#include <QMessageBox>
#endif

// Qt Interface includes:
#include "ui_dialogPlaneParam.h"

// Qt includes:
#include "QGMMacros.h"

//!
//! \brief Dialog class for GigaMesh's Plane Parameters (Layer 2)
//!
//! ...
//!
//! Layer 2
//!

class QGMDialogPlaneParam : public QDialog, private Ui::dialogPlaneParam {
      Q_OBJECT

public:
	// Constructor and Destructor:
    QGMDialogPlaneParam( QWidget *parent = nullptr, Qt::WindowFlags flags = {} );

public slots:
	bool setPlaneHNF( Vector3D rPlaneHNF );
	void preferSet();
	void preferVisDist();
	void preferSplit();
	void accept();

signals:
	void sComputeDistance(Vector3D,bool);
	void sSetPlaneHNF(Vector3D);
	void sSplitByPlane(Vector3D,bool);


	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif
