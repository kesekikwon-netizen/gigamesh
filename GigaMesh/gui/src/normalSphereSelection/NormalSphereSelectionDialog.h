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

#ifndef NormalSphereSELECTIONDIALOG_H
#define NormalSphereSELECTIONDIALOG_H

#include <QDialog>
#include <QQuaternion>
#include <GigaMesh/mesh/vector3d.h>

class QAbstractButton;

namespace Ui {
	class NormalSphereSelectionDialog;
}

class MeshQt;

class NormalSphereSelectionDialog : public QDialog
{
		Q_OBJECT

	public:
		explicit NormalSphereSelectionDialog(QWidget *parent = nullptr, bool faceSelection = false);
		~NormalSphereSelectionDialog() override;

		void setMeshNormals(MeshQt* mesh);
		void selectMeshByNormals();

	signals:
		void rotationChanged(QQuaternion);

	private slots:
		void comboButtonBoxClicked(QAbstractButton* button);
		void renderWidgetRotationChanged(QQuaternion quat);
	public slots:
		void updateRotationExternal(Vector3D camCenter, Vector3D camUp);
	private:
		Ui::NormalSphereSelectionDialog *ui;

		MeshQt* mMesh;

		bool mFaceSelection;	//false: vertex selection, true: face selection
		// QWidget interface
	protected:
		virtual void changeEvent(QEvent* event) override;
};

#endif // NormalSphereSELECTIONDIALOG_H
