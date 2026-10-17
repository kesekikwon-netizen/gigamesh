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

/*!
*	@file  QGMDialogConeParam.h
*	@brief Describes cone parameter dialog
*/

#ifndef QGMDIALOGCONEPARAM_H
#define QGMDIALOGCONEPARAM_H

#include <QtGui>

#if QT_VERSION >= 0x050000
#include <QAbstractButton>
#include <QPushButton>
#endif

#include <QIcon>
#include <QDialog>

#include <GigaMesh/mesh/vector3d.h>

#include "ui_dialogConeParam.h"
#include "QGMMacros.h"

class QGMDialogConeParam : public QDialog, private Ui::dialogConeParam {
	Q_OBJECT

public:
    QGMDialogConeParam( QWidget* parent = nullptr, Qt::WindowFlags flags = {} );

	void setAxis(const Vector3D& upperAxis, const Vector3D& lowerAxis);
	void setRadii(double upperRadius, double lowerRadius);

	void accept();
        void showEvent(QShowEvent*);

signals:
	void coneParameters(const Vector3D&, const Vector3D&, double, double);

private:
	void copyDataToClipboard();
	bool checkData(Vector3D& axisTop, Vector3D& axisBot, double& upperRadius, double& lowerRadius);

private slots:
	void apply();

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDIALOGCONEPARAM_H
