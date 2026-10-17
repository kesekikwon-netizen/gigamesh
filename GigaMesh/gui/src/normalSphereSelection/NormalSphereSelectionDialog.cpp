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

#include "NormalSphereSelectionDialog.h"
#include "ui_NormalSphereSelectionDialog.h"

#include "meshQt.h"
#include <QSlider>
#include <list>
#include <QAbstractButton>
#include <QRadioButton>
#include <iostream>

NormalSphereSelectionDialog::NormalSphereSelectionDialog(QWidget *parent, bool faceSelection) :
	QDialog(parent),
	ui(new Ui::NormalSphereSelectionDialog),
	mMesh(nullptr),
	mFaceSelection(faceSelection)
{
	ui->setupUi(this);

	ui->colorMap_comboBox->addItem(tr("Grayscale"));
	ui->colorMap_comboBox->addItem(tr("Hot"));
	ui->colorMap_comboBox->addItem(tr("Cold"));
	ui->colorMap_comboBox->addItem(tr("HSV (full)"));
	ui->colorMap_comboBox->addItem(tr("HSV (part)"));
	ui->colorMap_comboBox->addItem(tr("Brewer RdGy"));
	ui->colorMap_comboBox->addItem(tr("Brewer Spectral"));
	ui->colorMap_comboBox->addItem(tr("Brewer RdYlGn"));
	ui->colorMap_comboBox->addItem(tr("Hypsometric Tint"));
	ui->colorMap_comboBox->addItem(tr("Jet (Octave)"));
	ui->colorMap_comboBox->addItem(tr("Morgenstemning"));
	ui->colorMap_comboBox->addItem(tr("Hypsometric HiRise1"));
	ui->colorMap_comboBox->addItem(tr("Hypsometric HiRise2"));
	ui->colorMap_comboBox->addItem(tr("Parula"));
	ui->colorMap_comboBox->addItem(tr("Brewer YlOrBr"));
	ui->colorMap_comboBox->addItem(tr("Copper (Octave)"));
	ui->colorMap_comboBox->addItem(tr("Rust"));
	ui->colorMap_comboBox->addItem(tr("Sienna"));
	ui->colorMap_comboBox->addItem(tr("Hypsometric Arid"));

	connect(ui->colorMap_comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {this->ui->openGLWidget->setColorMapIndex(index);});
	connect(ui->selectionRadius_horizontalSlider, &QSlider::valueChanged, [this](int value) {this->ui->openGLWidget->setSelectionRadius(value);});

	connect(ui->selectionModeSelect_radioButton, &QRadioButton::pressed, [this](){this->ui->openGLWidget->setSelectionMask(255);});
	connect(ui->selectionModeClear_radioButton, &QRadioButton::pressed, [this](){this->ui->openGLWidget->setSelectionMask(0);});

	ui->colorMap_comboBox->setCurrentIndex(11);

	connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &NormalSphereSelectionDialog::comboButtonBoxClicked);

	connect(ui->openGLWidget, &NormalSphereSelectionRenderWidget::rotationChanged, this, &NormalSphereSelectionDialog::renderWidgetRotationChanged);

	connect(ui->rotationSharing_checkBox, &QRadioButton::toggled,
			[this](bool enabled){
									if(enabled) {
										emit rotationChanged(this->ui->openGLWidget->getRotation());}
								});

	connect(ui->normalScaling_checkBox, &QCheckBox::stateChanged, [this](int state){this->ui->openGLWidget->setScaleNormals(state != Qt::Unchecked);});

	connect(ui->checkBox_invertColorMap, &QCheckBox::stateChanged, [this](int state){this->ui->openGLWidget->setInvertFuncVal(state != Qt::Unchecked);});

	connect(ui->quantil_horizontalSlider, &QSlider::valueChanged,
			[this](int value)
			{
				float maxVal = this->ui->quantil_horizontalSlider->maximum();
				float valFloat = value / maxVal;
				ui->quantil_doubleSpinBox->blockSignals(true);
				ui->quantil_doubleSpinBox->setValue(valFloat);
				ui->quantil_doubleSpinBox->blockSignals(false);
				ui->openGLWidget->setUpperQuantil(valFloat);
			}
	);

	connect(ui->quantil_doubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
			[this](double value)
			{
				ui->quantil_horizontalSlider->blockSignals(true);
				ui->openGLWidget->setUpperQuantil(value);
				ui->quantil_horizontalSlider->setValue(value * ui->quantil_horizontalSlider->maximum());
				ui->quantil_horizontalSlider->blockSignals(false);
			}
	);
}

NormalSphereSelectionDialog::~NormalSphereSelectionDialog()
{
	delete ui;
}

void NormalSphereSelectionDialog::setMeshNormals(MeshQt* mesh)
{
	mMesh = mesh;

	std::vector<float> normalData;

	if(mFaceSelection)
	{
		std::vector<Face*> faces;
		mesh->getFaceList(&faces);

		normalData.reserve(faces.size() * 3);

		for(const auto face : faces)
		{
			normalData.push_back(face->getNormalX());
			normalData.push_back(face->getNormalY());
			normalData.push_back(face->getNormalZ());
		}
	}

	else
	{
		std::vector<Vertex*> vertices;
		mesh->getVertexList(&vertices);

		normalData.reserve(vertices.size() * 3);

		for(const auto vertex : vertices)
		{
			normalData.push_back(vertex->getNormalX());
			normalData.push_back(vertex->getNormalY());
			normalData.push_back(vertex->getNormalZ());
		}
	}

	ui->openGLWidget->setRenderNormals(normalData);

}

void NormalSphereSelectionDialog::selectMeshByNormals()
{
	if(mMesh == nullptr)
		return;

	if(mFaceSelection)
	{
		std::vector<Face*> faces;
		mMesh->getFaceList(&faces);

		std::set<Face*> selectedFaces;

		for(const auto face : faces)
		{
			if(ui->openGLWidget->isNormalSelected(face->getNormalX(), face->getNormalY(), face->getNormalZ()))
			{
				selectedFaces.insert(face);
			}
		}

		if(selectedFaces.empty())
			return;

		mMesh->addToSelection(selectedFaces);
	}

	else
	{
		std::vector<Vertex*> vertices;
		mMesh->getVertexList(&vertices);

		std::set<Vertex*> selectedVertices;

		for(const auto vertex : vertices)
		{

			if(ui->openGLWidget->isNormalSelected(vertex->getNormalX(), vertex->getNormalY(), vertex->getNormalZ()))
			{
				selectedVertices.insert(vertex);
			}
		}

		if(selectedVertices.empty())
			return;

		mMesh->addToSelection(selectedVertices);
	}

}

void NormalSphereSelectionDialog::comboButtonBoxClicked(QAbstractButton* button)
{
	if(button == ui->buttonBox->button(QDialogButtonBox::Apply))
		selectMeshByNormals();

	else if(button == ui->buttonBox->button(QDialogButtonBox::Reset))
		ui->openGLWidget->clearSelected();
}

void NormalSphereSelectionDialog::renderWidgetRotationChanged(QQuaternion quat)
{
	if(ui->rotationSharing_checkBox->isChecked())
	{
		emit rotationChanged(quat);
	}
}

void NormalSphereSelectionDialog::updateRotationExternal(Vector3D camCenter, Vector3D camUp)
{
	if(ui->rotationSharing_checkBox->isChecked())
	{
		QVector3D target = QVector3D(camCenter.getX(), camCenter.getY(), camCenter.getZ()).normalized();


		QQuaternion rotQuat = QQuaternion::fromDirection(target, QVector3D(camUp.getX(), camUp.getY(), camUp.getZ())).conjugated();
		ui->openGLWidget->setRotation(rotQuat);
	}
}


void NormalSphereSelectionDialog::changeEvent(QEvent* event)
{
	switch(event->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;

		default:
			break;
	}

	QDialog::changeEvent(event);
}
