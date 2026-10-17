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

#ifndef QGMDIALOGRULER_H
#define QGMDIALOGRULER_H

// generic C++ includes:
#include <iostream>

// C++ includes:
#include "meshwidget_params.h"

// generic Qt includes:
#include <QtGui>
#include <QDialog>
#include <qstring.h>

// Qt Interface includes:
#include "ui_dialogRuler.h"

// Qt includes:
#include "QGMMacros.h"

//!
//! \brief Dialog class for GigaMesh's Ruler Parameters (Layer 2)
//!
//! ...
//!
//! Layer 2
//!

class QGMDialogRuler : public QDialog, private Ui::dialogRuler {
      Q_OBJECT

public:
	// Constructor and Destructor:
    QGMDialogRuler( QWidget *parent = 0, Qt::WindowFlags flags = {} );

public slots:
	void setFileName(const QString& fileName );
	void setWidth(const double width );
	void setWidthUnit(const QString& widthUnit );
	void setHeight( const double height );
	void setUnit( const double unit );
	void setUnitTicks( const double unitTicks );
	void accept();
private slots:
    void setFileDirectory();
signals:
	void screenshotRuler(QString);    //! Filename to be used to store the ruler as image.
	void setParamFloatMeshWidget(MeshWidgetParams::eParamFlt,double); //! Parameters to be set, when OK is choosen.
	void setViewParamsStr(int,QString);

private:
    QString _fileDirectory;

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif
