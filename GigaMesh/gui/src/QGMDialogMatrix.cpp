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

#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "QGMDialogMatrix.h"
#include "ui_QGMDialogMatrix.h"

#include <QClipboard>
#include <QDoubleValidator>
#include <QDoubleSpinBox>
#include <GigaMesh/logging/Logging.h>


QGMDialogMatrix::QGMDialogMatrix(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QGMDialogMatrix)
{
	ui->setupUi(this);

	mLineEditPtrs[ 0] = ui->lineEdit_mat00;
	mLineEditPtrs[ 1] = ui->lineEdit_mat10;
	mLineEditPtrs[ 2] = ui->lineEdit_mat20;
	mLineEditPtrs[ 3] = ui->lineEdit_mat30;

	mLineEditPtrs[ 4] = ui->lineEdit_mat01;
	mLineEditPtrs[ 5] = ui->lineEdit_mat11;
	mLineEditPtrs[ 6] = ui->lineEdit_mat21;
	mLineEditPtrs[ 7] = ui->lineEdit_mat31;

	mLineEditPtrs[ 8] = ui->lineEdit_mat02;
	mLineEditPtrs[ 9] = ui->lineEdit_mat12;
	mLineEditPtrs[10] = ui->lineEdit_mat22;
	mLineEditPtrs[11] = ui->lineEdit_mat32;

	mLineEditPtrs[12] = ui->lineEdit_mat03;
	mLineEditPtrs[13] = ui->lineEdit_mat13;
	mLineEditPtrs[14] = ui->lineEdit_mat23;
	mLineEditPtrs[15] = ui->lineEdit_mat33;

	auto validatorLocale = QLocale::c();
	validatorLocale.setNumberOptions(QLocale::RejectGroupSeparator);

	mValidator.setLocale(validatorLocale);

	uint8_t index = 0;
	for(auto lineEditPtr : mLineEditPtrs)
	{
		lineEditPtr->setValidator(&mValidator);
		lineEditPtr->setProperty("index", index++);
	}

	//tabwidget events
	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &QGMDialogMatrix::tabChanged);
	tabChanged(ui->tabWidget->currentIndex());

	//free edit widgets
	connect(ui->pushButton_CopyToClipboard, &QPushButton::clicked, this, &QGMDialogMatrix::copyToClipboard);
	connect(ui->pushButton_loadFromClipboard, &QPushButton::clicked, this, &QGMDialogMatrix::fetchClipboard);

	//translate
	connect(ui->doubleSpinBox_transX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateTranslate);
	connect(ui->doubleSpinBox_transY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateTranslate);
	connect(ui->doubleSpinBox_transZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateTranslate);

	connect(ui->pushButton_COGCenter, &QPushButton::clicked, [this]() {
		ui->doubleSpinBox_transX->setValue(-mMeshCog.getX());
		ui->doubleSpinBox_transY->setValue(-mMeshCog.getY());
		ui->doubleSpinBox_transZ->setValue(-mMeshCog.getZ());
	});

	connect(ui->pushButton_BBoxCenter, &QPushButton::clicked, [this]() {
		ui->doubleSpinBox_transX->setValue(-mMeshBBoxCenter.getX());
		ui->doubleSpinBox_transY->setValue(-mMeshBBoxCenter.getY());
		ui->doubleSpinBox_transZ->setValue(-mMeshBBoxCenter.getZ());
	});

	//scale
	connect(ui->doubleSpinBox_scaleX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateScale);
	connect(ui->doubleSpinBox_scaleY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateScale);
	connect(ui->doubleSpinBox_scaleZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateScale);

	//rotate
	ui->buttonGroup_axisSel->setId(ui->radioButton_axisX   , 0);
	ui->buttonGroup_axisSel->setId(ui->radioButton_axisY   , 1);
	ui->buttonGroup_axisSel->setId(ui->radioButton_axisZ   , 2);
	ui->buttonGroup_axisSel->setId(ui->radioButton_axisFree, 3);
	connect(ui->doubleSpinBox_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &QGMDialogMatrix::updateRotate);

	auto updateRotValue = [this]() {updateRotate(ui->doubleSpinBox_angle->value());};

	connect(ui->doubleSpinBox_axisX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateRotValue);
	connect(ui->doubleSpinBox_axisY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateRotValue);
	connect(ui->doubleSpinBox_axisZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), updateRotValue);

	connect(ui->buttonGroup_axisSel, QOverload<int>::of(&QButtonGroup::buttonClicked), updateRotValue);

	connect(ui->horizontalSlider_angle, &QSlider::valueChanged, this, &QGMDialogMatrix::updateRotateBySlider);


	connect(ui->buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &QGMDialogMatrix::resetValues);
	connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &QGMDialogMatrix::applyClicked);

	updateMatrixValues({1.0, 0.0, 0.0, 0.0,
	                    0.0, 1.0, 0.0, 0.0,
	                    0.0, 0.0, 1.0, 0.0,
	                    0.0, 0.0, 0.0, 1.0});
}

QGMDialogMatrix::~QGMDialogMatrix()
{
	delete ui;
}

void QGMDialogMatrix::fetchClipboard()
{
	QString clipBoardStr = QApplication::clipboard()->text( QClipboard::Clipboard );

    const QStringList values = clipBoardStr.simplified().split( " ", QString::SkipEmptyParts );

	if(values.size() != 16)
	{
		return;
	}

	std::array<double,16> tempVals;

    unsigned int row = 0;
    unsigned int column = 0;
    //auto matIt = tempVals.begin();
	for(const auto& value : values)
	{
		bool ok = false;
		const auto val = value.toDouble(&ok);

		if(!ok)
		{
			return;
		}
        //input matrix from clipboard must be transposed
        tempVals[(row*4)+column] = val;
        row++;
        if( row == 4 ){
            column++;
            row = 0;
        }
        //(*matIt++) = val;
	}

	updateMatrixValues(tempVals);
}


void QGMDialogMatrix::getValues(std::vector<double>& values) const
{
	values.resize(16);
	auto it = mLineEditPtrs.begin();

	for(auto& value : values)
	{
		value = (*it++)->text().toDouble();
	}
}

void QGMDialogMatrix::updateMatrixValues(const std::array<double, 16>& values)
{
	auto it = mLineEditPtrs.begin();

	for(auto value : values)
	{
		(*it++)->setText(QString::number(value));
	}
}

void QGMDialogMatrix::tabChanged(int index)
{
	bool enable = index == 3;

	for(auto matrixField : mLineEditPtrs)
	{
		matrixField->setEnabled(enable);
	}
}

void QGMDialogMatrix::updateScale(double value)
{
	if(ui->checkBox_scaleUniform->isChecked())
	{
		QSignalBlocker blockScaleX(ui->doubleSpinBox_scaleX);
		QSignalBlocker blockScaleY(ui->doubleSpinBox_scaleX);
		QSignalBlocker blockScaleZ(ui->doubleSpinBox_scaleX);

		ui->doubleSpinBox_scaleX->setValue(value);
		ui->doubleSpinBox_scaleY->setValue(value);
		ui->doubleSpinBox_scaleZ->setValue(value);
	}

	const double scaleX = ui->doubleSpinBox_scaleX->value();
	const double scaleY = ui->doubleSpinBox_scaleY->value();
	const double scaleZ = ui->doubleSpinBox_scaleZ->value();


	updateMatrixValues({scaleX,    0.0,    0.0, 0.0,
	                       0.0, scaleY,    0.0, 0.0,
	                       0.0,    0.0, scaleZ, 0.0,
	                       0.0,    0.0,    0.0, 1.0});
}

void QGMDialogMatrix::updateTranslate()
{
	const double xVal = ui->doubleSpinBox_transX->value();
	const double yVal = ui->doubleSpinBox_transY->value();
	const double zVal = ui->doubleSpinBox_transZ->value();

	updateMatrixValues({1.0 ,0.0 ,0.0 ,0.0,
	                    0.0 ,1.0 ,0.0 ,0.0,
	                    0.0 ,0.0 ,1.0 ,0.0,
	                    xVal,yVal,zVal,1.0});
}

std::array<double,16> getAngleAxisMat(double aX, double aY, double aZ, double s, double c)
{
	const double len = std::sqrt(aX * aX + aY * aY + aZ * aZ);
	aX /= len;
	aY /= len;
	aZ /= len;

	const double mC = (1.0 - c);

	return {     c + aX * aX * mC,     aY*aX*mC + aZ * s,     aZ*aX*mC - aY * s, 0.0,
		    aX * aY * mC - aZ * s,      c + aY * aY * mC, aZ * aY * mC + aX * s, 0.0,
		    aX * aZ * mC + aY * s, aY * aZ * mC - aX * s,      c + aZ * aZ * mC, 0.0,
		    0.0, 0.0, 0.0, 1.0};
}

void QGMDialogMatrix::updateRotate(double angle)
{
	//update slider
	QSignalBlocker blockAngleSlider(ui->horizontalSlider_angle);

	const double sliderMax = static_cast<double>(ui->horizontalSlider_angle->maximum());
	const double sliderMin = static_cast<double>(ui->horizontalSlider_angle->minimum());
	const double percent = (angle - ui->doubleSpinBox_angle->minimum()) / (ui->doubleSpinBox_angle->maximum() - ui->doubleSpinBox_angle->minimum());

	ui->horizontalSlider_angle->setValue(percent * (sliderMax - sliderMin) + sliderMin);

	const double s = sin(angle * M_PI / 180.0);
	const double c = cos(angle * M_PI / 180.0);

	switch (ui->buttonGroup_axisSel->checkedId()) {
		case 0: // x-axis
			updateMatrixValues( {1.0, 0.0, 0.0, 0.0,
			                     0.0,   c,   s, 0.0,
			                     0.0,  -s,   c, 0.0,
			                     0.0, 0.0, 0.0, 1.0});
			break;
		case 1: // y-axis
			updateMatrixValues( {  c, 0.0,  -s, 0.0,
			                     0.0, 1.0, 0.0, 0.0,
			                       s, 0.0,   c, 0.0,
			                     0.0, 0.0, 0.0, 1.0});
			break;
		case 2: // z-axis
			updateMatrixValues( {  c,   s, 0.0, 0.0,
			                      -s,   c, 0.0, 0.0,
			                     0.0, 0.0, 1.0, 0.0,
			                     0.0, 0.0, 0.0, 1.0});
			break;
		case 3: // free-axis
			//avoid case, where all axis values are 0.0 and free axis is selected
			if(std::abs(ui->doubleSpinBox_axisX->value()) <= std::numeric_limits<double>::epsilon() &&
			   std::abs(ui->doubleSpinBox_axisY->value()) <= std::numeric_limits<double>::epsilon() &&
			   std::abs(ui->doubleSpinBox_axisZ->value()) <= std::numeric_limits<double>::epsilon())
			{
				return;
			}
			updateMatrixValues( getAngleAxisMat(ui->doubleSpinBox_axisX->value(),
			                                    ui->doubleSpinBox_axisY->value(),
			                                    ui->doubleSpinBox_axisZ->value(),
			                                    s,c));
			break;
	}

}

void QGMDialogMatrix::updateRotateBySlider(int value)
{
	const double angleMin = ui->doubleSpinBox_angle->minimum();
	const double angleMax = ui->doubleSpinBox_angle->maximum();
	const double angleRange = angleMax - angleMin;

	const double valRel     = static_cast<double>(value - ui->horizontalSlider_angle->minimum());
	const double valueRange = static_cast<double>(ui->horizontalSlider_angle->maximum() - ui->horizontalSlider_angle->minimum());

	ui->doubleSpinBox_angle->setValue( (valRel / valueRange * angleRange) + angleMin );
}

void QGMDialogMatrix::resetValues()
{
	QSignalBlocker blockAngle(ui->doubleSpinBox_angle);
	QSignalBlocker blockAxisX(ui->doubleSpinBox_axisX);
	QSignalBlocker blockAxisY(ui->doubleSpinBox_axisY);
	QSignalBlocker blockAxisZ(ui->doubleSpinBox_axisZ);
	QSignalBlocker blockAngleSlider(ui->horizontalSlider_angle);

	QSignalBlocker blockScaleX(ui->doubleSpinBox_scaleX);
	QSignalBlocker blockScaleY(ui->doubleSpinBox_scaleY);
	QSignalBlocker blockScaleZ(ui->doubleSpinBox_scaleZ);

	QSignalBlocker blockTransX(ui->doubleSpinBox_transX);
	QSignalBlocker blockTransY(ui->doubleSpinBox_transY);
	QSignalBlocker blockTransZ(ui->doubleSpinBox_transZ);

	ui->doubleSpinBox_angle->setValue(0.0);
	ui->doubleSpinBox_axisX->setValue(1.0);
	ui->doubleSpinBox_axisY->setValue(0.0);
	ui->doubleSpinBox_axisZ->setValue(0.0);
	ui->horizontalSlider_angle->setValue(0);

	ui->doubleSpinBox_scaleX->setValue(1.0);
	ui->doubleSpinBox_scaleY->setValue(1.0);
	ui->doubleSpinBox_scaleZ->setValue(1.0);

	ui->doubleSpinBox_transX->setValue(0.0);
	ui->doubleSpinBox_transY->setValue(0.0);
	ui->doubleSpinBox_transZ->setValue(0.0);

	updateMatrixValues({1.0,0.0,0.0,0.0,
	                    0.0,1.0,0.0,0.0,
	                    0.0,0.0,1.0,0.0,
	                    0.0,0.0,0.0,1.0});
}

void QGMDialogMatrix::setMeshBBoxCenter(const Vector3D& meshBBoxCenter)
{
	mMeshBBoxCenter = meshBBoxCenter;
}

void QGMDialogMatrix::setMeshCog(const Vector3D& meshCog)
{
	mMeshCog = meshCog;
}

void QGMDialogMatrix::copyToClipboard() const
{
	QString clipBoardText;

    //transpose matrix to clipboard
    //needed because the load from clipboard method needs this
    unsigned int row = 0;
    unsigned int column = 0;
    for(unsigned int i = 0; i<mLineEditPtrs.size(); i++)
	{
        clipBoardText += QString("%1 ").arg(mLineEditPtrs[(row*4)+column]->text());
        row++;
        if( row == 4 ){
            column++;
            row = 0;
        }
	}

	QApplication::clipboard()->setText(clipBoardText);
}
