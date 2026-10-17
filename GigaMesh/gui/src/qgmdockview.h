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

#ifndef QGMDOCKVIEW_H
#define QGMDOCKVIEW_H

#include <QDockWidget>

#include "meshwidget_params.h"
#include "meshGL/meshGL_params.h"

namespace Ui {
class QGMDockView;
}

class QGMDockView : public QDockWidget
{
	Q_OBJECT

public:
	explicit QGMDockView(QWidget *parent = 0);
	~QGMDockView();

public slots:
	void viewPortInfo(const MeshWidgetParams::eViewPortInfo rInfoID, const QString& rInfoString );
	void infoMesh(const MeshGLParams::eInfoMesh rInfoID, const QString& rInfoString );

private:
	Ui::QGMDockView *ui;

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDOCKVIEW_H
