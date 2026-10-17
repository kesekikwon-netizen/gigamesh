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

#ifndef QGMDOCKSIDEBAR_H
#define QGMDOCKSIDEBAR_H

#include <QDockWidget>

#include "meshGL/meshGL_params.h"
#include "meshwidget_params.h"

namespace Ui {
class QGMDockSideBar;
}

class QGMDockSideBar : public QDockWidget {
	Q_OBJECT
	
public:
	explicit QGMDockSideBar(QWidget *parent = 0);
	~QGMDockSideBar();

private slots:
	void texMapVertMono( bool rState );
	void texMapVertRGB( bool rState );
	void texMapVertFuncVal( bool rState );
	void texMapVertLabels( bool rState );

	void shaderChoiceWireframe( bool rState );
	void shaderChoiceNPR( bool rState );
	void shaderChoicePointCloud( bool rState);
	void shaderChoiceMonolithic( bool rState);
	void shaderChoiceTransparency( bool rState);
	void shaderChoiceTextured( bool rState);
	void archaeologyModeChanged( bool rState );
	void cortexRenderingChanged( bool rState );
	void selectCortexVerticesClicked( bool rState );
	void clearCortexSelectionClicked();
public slots:
	void updateMeshParamInt( MeshGLParams::eParamInt rParamNr, int rSetValue );
	void enableTextureMeshRendering(bool enable);

signals:
	void sShowParamIntMeshGL(MeshGLParams::eParamInt,int); //!< Trigger setup of a MeshGL specific parameter of type int.
	void sShowParamFlagMeshGL(MeshGLParams::eParamFlag,bool); //!< Trigger setup of a MeshGL specific parameter of type flag.
	void sShowFlagMeshWidget(MeshWidgetParams::eParamFlag,bool); //!< Trigger setup of a MeshWidget specific parameter of type flag.
	void sShowNPRSettings();
	void sShowTransparencySettings();
	void sEnterCortexSelectionMode(bool rEnabled); //!< Enter/exit cortex vertex selection mode.
	void sClearCortexSelection(); //!< Clear all cortex vertex flags.

private:
	Ui::QGMDockSideBar *ui;

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDOCKSIDEBAR_H
