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

#include "dialogGridCenterSelect.h"
#include "ui_dialogGridCenterSelect.h"

#include <iostream>

dialogGridCenterSelect::dialogGridCenterSelect(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::dialogGridCenterSelect),
	mCenterPos(CenterPos::CENTER)
{
	ui->setupUi(this);

	connect(ui->centerSelectBtnGroup, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(selectPos(QAbstractButton*, bool)));
}

dialogGridCenterSelect::~dialogGridCenterSelect()
{
	delete ui;
}

dialogGridCenterSelect::CenterPos dialogGridCenterSelect::centerPos() const
{
	return mCenterPos;
}

void dialogGridCenterSelect::setCenterPos(const CenterPos& centerPos)
{
	mCenterPos = centerPos;

	switch (centerPos) {
		case TOP_LEFT:
			ui->radioButton_TL->setChecked(true);
			break;
		case TOP:
			ui->radioButton_T->setChecked(true);
			break;
		case TOP_RIGHT:
			ui->radioButton_TR->setChecked(true);
			break;
		case LEFT:
			ui->radioButton_L->setChecked(true);
			break;
		case CENTER:
			ui->radioButton_C->setChecked(true);
			break;
		case RIGHT:
			ui->radioButton_R->setChecked(true);
			break;

		case BOTTOM_LEFT:
			ui->radioButton_BL->setChecked(true);
			break;
		case BOTTOM:
			ui->radioButton_B->setChecked(true);
			break;
		case BOTTOM_RIGHT:
			ui->radioButton_BR->setChecked(true);
			break;
	}
}

void dialogGridCenterSelect::selectPos(QAbstractButton* button, bool active)
{
	if(!active)
		return;

	if(button == ui->radioButton_TL)
		mCenterPos = CenterPos::TOP_LEFT;
	else if(button == ui->radioButton_T)
		mCenterPos = CenterPos::TOP;
	else if(button == ui->radioButton_TR)
		mCenterPos = CenterPos::TOP_RIGHT;

	else if(button == ui->radioButton_R)
		mCenterPos = CenterPos::RIGHT;
	else if(button == ui->radioButton_C)
		mCenterPos = CenterPos::CENTER;
	else if(button == ui->radioButton_L)
		mCenterPos = CenterPos::LEFT;

	else if(button == ui->radioButton_BL)
		mCenterPos = CenterPos::BOTTOM_LEFT;
	else if(button == ui->radioButton_B)
		mCenterPos = CenterPos::BOTTOM;
	else if(button == ui->radioButton_BR)
		mCenterPos = CenterPos::BOTTOM_RIGHT;
}
