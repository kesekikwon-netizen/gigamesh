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

#ifndef DIALOGGRIDCENTERSELECT_H
#define DIALOGGRIDCENTERSELECT_H

#include <QAbstractButton>
#include <QDialog>


namespace Ui {
	class dialogGridCenterSelect;
}

class dialogGridCenterSelect : public QDialog
{
		Q_OBJECT

	public:
		explicit dialogGridCenterSelect(QWidget *parent = 0);
		~dialogGridCenterSelect();

		enum CenterPos {
			CENTER       = 0,
			TOP_LEFT     = 1,
			TOP          = 2,
			TOP_RIGHT    = 3,
			LEFT         = 4,
			RIGHT        = 5,
			BOTTOM_LEFT  = 6,
			BOTTOM       = 7,
			BOTTOM_RIGHT = 8
		};

		CenterPos centerPos() const;
		void setCenterPos(const CenterPos& centerPos);

	private:
		Ui::dialogGridCenterSelect *ui;

		CenterPos mCenterPos;

	private slots:
		void selectPos(QAbstractButton* button, bool active);
};

#endif // DIALOGGRIDCENTERSELECT_H
