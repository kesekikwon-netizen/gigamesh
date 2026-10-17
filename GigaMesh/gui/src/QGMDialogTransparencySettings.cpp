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

#include "QGMDialogTransparencySettings.h"
#include "ui_QGMDialogTransparencySettings.h"
#include <iostream>
#include <QMessageBox>

QGMDialogTransparencySettings::QGMDialogTransparencySettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QGMDialogTransparencySettings)
{
    ui->setupUi(this);
    int sliderSteps;

    sliderSteps = (ui->doubleSpinBox_uniform->maximum() - ui->doubleSpinBox_uniform->minimum()) / ui->doubleSpinBox_uniform->singleStep();
    ui->horizontalSlider_uniform->setMaximum(sliderSteps);

    sliderSteps = (ui->doubleSpinBox_uniform_2->maximum() - ui->doubleSpinBox_uniform_2->minimum()) / ui->doubleSpinBox_uniform_2->singleStep();
    ui->horizontalSlider_uniform_2->setMaximum(sliderSteps);

    connect(ui->buttonGroup, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(radioButtonChanged(QAbstractButton*,bool) ));
    connect(ui->buttonGroup_2, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(radioButtonChanged(QAbstractButton*,bool) ));

    connect(ui->spinBox_labelSel, SIGNAL(valueChanged(int)) , this, SLOT(intValueChanged()));
    connect(ui->spinBox_Layers, SIGNAL(valueChanged(int)) , this , SLOT(intValueChanged()));
    connect(ui->horizontalSlider_Layers, SIGNAL(valueChanged(int)), ui->spinBox_Layers, SLOT(setValue(int)));

    connect(ui->doubleSpinBox_uniform, SIGNAL(valueChanged(double)), this , SLOT(transValChanged(double)));
    connect(ui->horizontalSlider_uniform, SIGNAL(valueChanged(int)), this , SLOT(transValChanged(int)));

    connect(ui->doubleSpinBox_uniform_2, SIGNAL(valueChanged(double)), this , SLOT(transValChanged2(double)));
    connect(ui->horizontalSlider_uniform_2, SIGNAL(valueChanged(int)), this , SLOT(transValChanged2(int)));

    connect(ui->comboBox_overflow, SIGNAL(currentIndexChanged(int)), this , SLOT(comboBoxChanged()));

    connect(ui->doubleSpinBox_Gamma, SIGNAL(valueChanged(double)), this , SLOT(gammaChanged(double)));
    connect(ui->horizontalSlider_Gamma, SIGNAL(valueChanged(int)),this, SLOT(gammaChanged(int)));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(exitAccept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(exitReject()));

    m_enableGL4_3Features = false;
}

QGMDialogTransparencySettings::~QGMDialogTransparencySettings()
{
    delete ui;
}

void QGMDialogTransparencySettings::init(MeshGL *mesh, bool enableGL4Features)
{
    m_enableGL4_3Features = enableGL4Features;
    //update the variables etc.
    m_MGL = mesh;
    m_MGL->getParamFloatMeshGL(MeshGL::TRANSPARENCY_UNIFORM_ALPHA, &m_transVal);
    m_MGL->getParamFloatMeshGL(MeshGL::TRANSPARENCY_ALPHA2, &m_transVal2);
    m_MGL->getParamFloatMeshGL(MeshGL::TRANSPARENCY_GAMMA, &m_Gamma);

    m_MGL->getParamIntMeshGL(MeshGL::TRANSPARENCY_TRANS_FUNCTION, &m_TransparencyMethod);
    m_MGL->getParamIntMeshGL(MeshGL::TRANSPARENCY_BUFFER_METHOD, &m_BufferMethod);
    m_MGL->getParamIntMeshGL(MeshGL::TRANSPARENCY_SEL_LABEL, &m_selLabel);
    m_MGL->getParamIntMeshGL(MeshGL::TRANSPARENCY_NUM_LAYERS, &m_numLayers);
    m_MGL->getParamIntMeshGL(MeshGL::TRANSPARENCY_OVERFLOW_HANDLING, &m_overflowHandling);


    if(!enableGL4Features)
    {
        ui->radioButton_AtomicLoop->setEnabled(false);
        ui->radioButton_ABuffer->setEnabled(false);
    }

    resetValues();
}

void QGMDialogTransparencySettings::resetValues()
{
    QList<QWidget* > childWidgets = this->findChildren<QWidget * >();

    for(auto& widget : childWidgets)
    {
        widget->blockSignals(true);
    }

    ui->checkBox_Preview->blockSignals(true);
	//ui->checkBox_Preview->setChecked(false);

    ui->doubleSpinBox_uniform->setValue(m_transVal);
    ui->horizontalSlider_uniform->setValue((m_transVal - ui->doubleSpinBox_uniform->minimum()) / ui->doubleSpinBox_uniform->singleStep());

    ui->doubleSpinBox_uniform_2->setValue(m_transVal2);
    ui->horizontalSlider_uniform_2->setValue((m_transVal2 - ui->doubleSpinBox_uniform_2->minimum()) / ui->doubleSpinBox_uniform_2->singleStep());

    ui->doubleSpinBox_Gamma->setValue(m_Gamma);
    gammaChanged(m_Gamma);

    ui->spinBox_labelSel->setValue(m_selLabel);
    ui->horizontalSlider_Layers->setValue(m_numLayers);
    ui->spinBox_Layers->setValue(m_numLayers);

    ui->comboBox_overflow->setCurrentIndex(m_overflowHandling);

    switch(m_TransparencyMethod)
    {
        case 0:
            ui->radioButton_Uniform->setChecked(true);
            break;
        case 1:
            ui->radioButton_Vertex->setChecked(true);
            break;

        case 2:
            ui->radioButton_FuncVal->setChecked(true);
            break;

        case 3:
            ui->radioButton_Angle->setChecked(true);
            break;

        case 4:
            ui->radioButton_Label->setChecked(true);
            break;
        case 5:
            ui->radioButton_NormalVariation->setChecked(true);
            break;
        default:
            ui->radioButton_Uniform->setChecked(true);
            break;
    }

    //only allow weighted average if opengl4.3 is not enabled
    if(!m_enableGL4_3Features)
        m_BufferMethod = 0;

    switch(m_BufferMethod)
    {
        case 0:
            ui->radioButton_WBlend->setChecked(true);
            showALoopSettings(false);
            break;

        case 1:
            ui->radioButton_ABuffer->setChecked(true);
            showALoopSettings(false);
            break;
        case 2:
            ui->radioButton_AtomicLoop->setChecked(true);
            showALoopSettings(true);
            break;
        default:
            ui->radioButton_WBlend->setChecked(true);
            showALoopSettings(false);
            break;
    }

    m_checkedBox = m_TransparencyMethod;
    m_checkedBufferMethod = m_BufferMethod;

    for(auto& widget : childWidgets)
    {
        widget->blockSignals(false);
    }

}

void QGMDialogTransparencySettings::radioButtonChanged(QAbstractButton *button, bool checked)
{
    if(checked)
    {
        if(button == ui->radioButton_Uniform)
            m_checkedBox = 0;
        else if(button == ui->radioButton_Vertex)
            m_checkedBox = 1;
        else if(button == ui->radioButton_FuncVal)
            m_checkedBox = 2;
        else if(button == ui->radioButton_Angle)
            m_checkedBox = 3;
        else if(button == ui->radioButton_Label)
            m_checkedBox = 4;
        else if(button == ui->radioButton_NormalVariation)
            m_checkedBox = 5;

        else if(button == ui->radioButton_WBlend)
        {
            m_checkedBufferMethod = 0;
            showALoopSettings(false);
        }
        else if(button == ui->radioButton_ABuffer)
        {
            m_checkedBufferMethod = 1;
            showALoopSettings(false);
        }
        else if(button == ui->radioButton_AtomicLoop)
        {
            m_checkedBufferMethod = 2;
            showALoopSettings(true);
        }
        if(ui->checkBox_Preview->isChecked())
                emitValues();
    }
}

void QGMDialogTransparencySettings::showALoopSettings(bool val)
{
    ui->label_2->setVisible(val);
    ui->label_3->setVisible(val);
    ui->label_6->setVisible(val);
    ui->spinBox_Layers->setVisible(val);
    ui->horizontalSlider_Layers->setVisible(val);
    ui->comboBox_overflow->setVisible(val);
}

void QGMDialogTransparencySettings::intValueChanged()
{

    if(ui->horizontalSlider_Layers->value() != ui->spinBox_Layers->value())
    {
        ui->horizontalSlider_Layers->blockSignals(true);
        ui->horizontalSlider_Layers->setValue(ui->spinBox_Layers->value());
        ui->horizontalSlider_Layers->blockSignals(false);
    }

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}


void QGMDialogTransparencySettings::transValChanged(int value)
{
    ui->doubleSpinBox_uniform->blockSignals(true);
    ui->doubleSpinBox_uniform->setValue(ui->doubleSpinBox_uniform->minimum() + ui->doubleSpinBox_uniform->singleStep() * value);
    ui->doubleSpinBox_uniform->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogTransparencySettings::transValChanged(double value)
{
    ui->horizontalSlider_uniform->blockSignals(true);
    ui->horizontalSlider_uniform->setValue((value - ui->doubleSpinBox_uniform->minimum()) / ui->doubleSpinBox_uniform->singleStep());
    ui->horizontalSlider_uniform->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogTransparencySettings::transValChanged2(int value)
{
    ui->doubleSpinBox_uniform_2->blockSignals(true);
    ui->doubleSpinBox_uniform_2->setValue(ui->doubleSpinBox_uniform_2->minimum() + ui->doubleSpinBox_uniform_2->singleStep() * value);
    ui->doubleSpinBox_uniform_2->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogTransparencySettings::transValChanged2(double value)
{
    ui->horizontalSlider_uniform_2->blockSignals(true);
    ui->horizontalSlider_uniform_2->setValue((value - ui->doubleSpinBox_uniform_2->minimum()) / ui->doubleSpinBox_uniform_2->singleStep());
    ui->horizontalSlider_uniform_2->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogTransparencySettings::gammaChanged(int value)
{
    ui->doubleSpinBox_Gamma->blockSignals(true);
    double logMin = log10(ui->doubleSpinBox_Gamma->minimum());
    double logMax = log10(ui->doubleSpinBox_Gamma->maximum());

    double logVal = ((logMax - logMin) * (value / 100.0)) + logMin;

    ui->doubleSpinBox_Gamma->setValue(pow(10,logVal));

    ui->doubleSpinBox_Gamma->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogTransparencySettings::gammaChanged(double value)
{
    ui->horizontalSlider_Gamma->blockSignals(true);

    double logMin = log10(ui->doubleSpinBox_Gamma->minimum());
    double logMax = log10(ui->doubleSpinBox_Gamma->maximum());
    double logVal = log10(value);

    ui->horizontalSlider_Gamma->setValue(((logVal - logMin) / (logMax - logMin)) * 100.0);

    ui->horizontalSlider_Gamma->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogTransparencySettings::comboBoxChanged()
{
    if(ui->checkBox_Preview->isChecked())
		emitValues();
}

void QGMDialogTransparencySettings::changeEvent(QEvent* event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QDialog::changeEvent(event);
}

void QGMDialogTransparencySettings::emitValues()
{
    m_MGL->MeshGL::setParamFloatMeshGL(MeshGL::TRANSPARENCY_UNIFORM_ALPHA, ui->doubleSpinBox_uniform->value());
    m_MGL->MeshGL::setParamFloatMeshGL(MeshGL::TRANSPARENCY_ALPHA2, ui->doubleSpinBox_uniform_2->value());
    m_MGL->MeshGL::setParamFloatMeshGL(MeshGL::TRANSPARENCY_GAMMA, ui->doubleSpinBox_Gamma->value());

    m_MGL->MeshGL::setParamIntMeshGL(MeshGL::TRANSPARENCY_TRANS_FUNCTION, m_checkedBox);
    m_MGL->MeshGL::setParamIntMeshGL(MeshGL::TRANSPARENCY_BUFFER_METHOD, m_checkedBufferMethod);
    m_MGL->MeshGL::setParamIntMeshGL(MeshGL::TRANSPARENCY_SEL_LABEL, ui->spinBox_labelSel->value());
    m_MGL->MeshGL::setParamIntMeshGL(MeshGL::TRANSPARENCY_NUM_LAYERS, ui->spinBox_Layers->value());
    m_MGL->MeshGL::setParamIntMeshGL(MeshGL::TRANSPARENCY_OVERFLOW_HANDLING, ui->comboBox_overflow->currentIndex());

    emit valuesChanged();
}

void QGMDialogTransparencySettings::exitAccept()
{
    emitValues();
    accept();
}

void QGMDialogTransparencySettings::exitReject()
{
    resetValues();
    emitValues();
    reject();
}
