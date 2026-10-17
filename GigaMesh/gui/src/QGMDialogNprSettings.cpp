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

#include "QGMDialogNprSettings.h"
#include "ui_qgmdialognprsettings.h"

//#include "meshGL/meshGL_params.h"
//#include "meshGL/meshGLShader.h"

QGMDialogNprSettings::QGMDialogNprSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QGMDialogNprSettings)
{
    ui->setupUi(this);

    int sliderSteps;

    //setup Outline Size
    sliderSteps = (ui->doubleSpinBox_Outline_Size->maximum() - ui->doubleSpinBox_Outline_Size->minimum()) / ui->doubleSpinBox_Outline_Size->singleStep();
    ui->slider_Outline_Size->setMaximum(sliderSteps);

    connect(ui->doubleSpinBox_Outline_Size, SIGNAL(valueChanged(double)), this , SLOT(outlineSizeChanged(double)));
    connect(ui->slider_Outline_Size, SIGNAL(valueChanged(int)), this , SLOT(outlineSizeChanged(int)));

    //setup Outline Threshold
    sliderSteps = (ui->doubleSpinBox_Outline_threshold->maximum() - ui->doubleSpinBox_Outline_threshold->minimum()) / ui->doubleSpinBox_Outline_threshold->singleStep();
    ui->slider_Outline_threshold->setMaximum(sliderSteps);

    connect(ui->doubleSpinBox_Outline_threshold, SIGNAL(valueChanged(double)), this , SLOT(outlineThresholdChanged(double)));
    connect(ui->slider_Outline_threshold, SIGNAL(valueChanged(int)), this , SLOT(outlineThresholdChanged(int)));

    //setup Hatch Size
    //sliderSteps = (ui->doubleSpinBox_Hatch_Size->maximum() - ui->doubleSpinBox_Hatch_Size->minimum()) / ui->doubleSpinBox_Hatch_Size->singleStep();
    //ui->Slider_Hatch_Size->setMaximum(sliderSteps);

    connect(ui->doubleSpinBox_Hatch_Size, SIGNAL(valueChanged(double)), this , SLOT(hatchSizeChanged(double)));
    connect(ui->Slider_Hatch_Size, SIGNAL(valueChanged(int)), this , SLOT(hatchSizeChanged(int)));

    //setup Hatch Rotation
    sliderSteps = (ui->doubleSpinBox_Hatch_Rotation->maximum() - ui->doubleSpinBox_Hatch_Rotation->minimum()) / ui->doubleSpinBox_Hatch_Rotation->singleStep();
    ui->Slider_Hatch_Rotation->setMaximum(sliderSteps);

    connect(ui->doubleSpinBox_Hatch_Rotation, SIGNAL(valueChanged(double)), this , SLOT(hatchRotationChanged(double)));
    connect(ui->Slider_Hatch_Rotation, SIGNAL(valueChanged(int)), this , SLOT(hatchRotationChanged(int)));

    //setup Specular Size
    sliderSteps = (ui->doubleSpinBox_Specular_Size->maximum() - ui->doubleSpinBox_Specular_Size->minimum()) / ui->doubleSpinBox_Specular_Size->singleStep();
    ui->Slider_Specular_Size->setMaximum(sliderSteps);
    connect(ui->doubleSpinBox_Specular_Size, SIGNAL(valueChanged(double)), this, SLOT(specularSizeChanged(double)));
    connect(ui->Slider_Specular_Size, SIGNAL(valueChanged(int)),this, SLOT(specularSizeChanged(int)));

    connect(ui->pushButton_Outline_Color, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(ui->pushButton_Hatch_Color, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(ui->pushButton_Toon_Diff1, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(ui->pushButton_Toon_Diff2, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(ui->pushButton_Toon_Diff3, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(ui->pushButton_Toon_Diff4, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(ui->pushButton_Toon_Diff5, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(ui->pushButton_Toon_Spec, SIGNAL(clicked()), this, SLOT(changeColor()));

    connect(ui->checkBox_Outline_enable, SIGNAL(stateChanged(int)), this, SLOT(changeBool()));
    connect(ui->checkBox_Hatch_Enable, SIGNAL(stateChanged(int)), this, SLOT(changeBool()));
    connect(ui->checkBox_Toon_Enable, SIGNAL(stateChanged(int)), this, SLOT(changeBool()));
    connect(ui->checkBox_Preview, SIGNAL(stateChanged(int)), this, SLOT(changeBool()));
    connect(ui->checkBox_Hatch_LightInfluence, SIGNAL(stateChanged(int)) , this, SLOT(changeBool()));
    connect(ui->checkBox_Enable_Specular, SIGNAL(stateChanged(int)), this, SLOT(changeBool()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(exitAccept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(exitReject()));

	//m_pColorDialog = new QColorDialog(this);
	//connect(m_pColorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(colorChanged(QColor)));

    ui->comboBox_Hatch_Style->blockSignals(true);
    ui->comboBox_Hatch_Style->insertItem(0,tr("Lines"));
    ui->comboBox_Hatch_Style->insertItem(1,tr("Dots"));
    ui->comboBox_Hatch_Style->insertItem(2,tr("Random Dither"));
    ui->comboBox_Hatch_Style->insertItem(3,tr("Bayer Dither"));
    ui->comboBox_Hatch_Style->blockSignals(false);
    connect(ui->comboBox_Hatch_Style, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged()));
    connect(ui->comboBox_HatchSource, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged()));
    connect(ui->comboBox_OutlineSource, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged()));
    connect(ui->comboBox_ToonSource, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged()));

    connect(ui->buttonGroup_Toon_type, SIGNAL(buttonToggled(QAbstractButton*,bool)), this, SLOT(changeToonType(QAbstractButton*,bool)));
    connect(ui->spinBox_lighting_steps, SIGNAL(valueChanged(int)), this, SLOT(changeIntSpinner(int)));
    connect(ui->spinBox_hue_steps, SIGNAL(valueChanged(int)), this, SLOT(changeIntSpinner(int)));
    connect(ui->spinBox_sat_steps, SIGNAL(valueChanged(int)), this, SLOT(changeIntSpinner(int)));
    connect(ui->spinBox_val_steps, SIGNAL(valueChanged(int)), this, SLOT(changeIntSpinner(int)));
}


void QGMDialogNprSettings::init(
                MeshGL*       rMesh,
                MeshGLColors* rRenderColors
) {
	m_meshGL      = rMesh;
	mRenderColors = rRenderColors;

    m_meshGL->getParamFloatMeshGL(MeshGL::NPR_OUTLINE_WIDTH, &m_outlineSize);
    m_meshGL->getParamFloatMeshGL(MeshGL::NPR_OUTLINE_THRESHOLD, &m_outlineThreshold);
    m_meshGL->getParamFloatMeshGL(MeshGL::NPR_HATCH_SCALE, &m_hatchSize);
    m_meshGL->getParamFloatMeshGL(MeshGL::NPR_HATCH_ROTATION, &m_hatchRotation);
    m_meshGL->getParamFloatMeshGL(MeshGL::NPR_SPECULAR_SIZE, &m_specularSize);

    m_meshGL->getParamFlagMeshGL(MeshGL::SHOW_NPR_OUTLINES, &m_bOutlineEnabled);
    m_meshGL->getParamFlagMeshGL(MeshGL::SHOW_NPR_HATCHLINES, &m_bHatchEnabled);
    m_meshGL->getParamFlagMeshGL(MeshGL::SHOW_NPR_TOON, &m_bToonEnabled);
    m_meshGL->getParamFlagMeshGL(MeshGL::NPR_USE_SPECULAR, &m_bSpecularEnabled);
    m_meshGL->getParamFlagMeshGL(MeshGL::NPR_HATCHLINE_LIGHT_INFLUENCE, &m_bHatchLInfluence);

    m_meshGL->getParamIntMeshGL(MeshGL::NPR_HATCH_STYLE, &m_hatchStyle);
    m_meshGL->getParamIntMeshGL(MeshGL::NPR_OUTLINE_SOURCE, &m_outlineSource);
    m_meshGL->getParamIntMeshGL(MeshGL::NPR_HATCH_SOURCE, &m_hatchSource);
    m_meshGL->getParamIntMeshGL(MeshGL::NPR_TOON_SOURCE, &m_toonSource);

    GLfloat colorVal[4];
    mRenderColors->getColorSettings(MeshGLColors::COLOR_NPR_OUTLINE, colorVal);
    m_OutlineColor.setRgbF(colorVal[0], colorVal[1], colorVal[2]);

    mRenderColors->getColorSettings(MeshGLColors::COLOR_NPR_HATCHLINE, colorVal);
    m_HatchColor.setRgbF(colorVal[0], colorVal[1], colorVal[2]);

    mRenderColors->getColorSettings(MeshGLColors::COLOR_NPR_SPECULAR, colorVal);
    m_Specular.setRgbF(colorVal[0], colorVal[1], colorVal[2]);

    mRenderColors->getColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE1, colorVal);
    m_Diffuse1.setRgbF(colorVal[0], colorVal[1], colorVal[2]);

    mRenderColors->getColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE2, colorVal);
    m_Diffuse2.setRgbF(colorVal[0], colorVal[1], colorVal[2]);

    mRenderColors->getColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE3, colorVal);
    m_Diffuse3.setRgbF(colorVal[0], colorVal[1], colorVal[2]);

    mRenderColors->getColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE4, colorVal);
    m_Diffuse4.setRgbF(colorVal[0], colorVal[1], colorVal[2]);

    mRenderColors->getColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE5, colorVal);
    m_Diffuse5.setRgbF(colorVal[0], colorVal[1], colorVal[2]);

    m_meshGL->getParamIntMeshGL(MeshGL::NPR_TOON_TYPE, &m_toonType);
    m_meshGL->getParamIntMeshGL(MeshGL::NPR_TOON_LIGHTING_STEPS, &m_toonLightingSteps);
    m_meshGL->getParamIntMeshGL(MeshGL::NPR_TOON_HUE_STEPS, &m_toonHueSteps);
    m_meshGL->getParamIntMeshGL(MeshGL::NPR_TOON_SAT_STEPS, &m_toonSatSteps);
    m_meshGL->getParamIntMeshGL(MeshGL::NPR_TOON_VAL_STEPS, &m_toonValSteps);

    resetValues();
}

QGMDialogNprSettings::~QGMDialogNprSettings()
{
	//delete m_pColorDialog;
    delete ui;
}

void QGMDialogNprSettings::resetValues()
{
    QList<QWidget* > childWidgets = this->findChildren<QWidget * >();

    for(auto& widget : childWidgets)
    {
        widget->blockSignals(true);
    }

    ui->checkBox_Hatch_Enable->setChecked(m_bHatchEnabled);
    ui->checkBox_Outline_enable->setChecked(m_bOutlineEnabled);
    ui->checkBox_Toon_Enable->setChecked(m_bToonEnabled);
    ui->checkBox_Enable_Specular->setChecked(m_bSpecularEnabled);

    ui->checkBox_Hatch_LightInfluence->setChecked(m_bHatchLInfluence);
    ui->comboBox_Hatch_Style->setCurrentIndex(m_hatchStyle);

    ui->comboBox_HatchSource->setCurrentIndex(m_hatchSource);
    ui->comboBox_OutlineSource->setCurrentIndex(m_outlineSource);
    ui->comboBox_ToonSource->setCurrentIndex(m_toonSource);

    ui->doubleSpinBox_Outline_Size->setValue(m_outlineSize);
    ui->slider_Outline_Size->setValue((m_outlineSize - ui->doubleSpinBox_Outline_Size->minimum()) / ui->doubleSpinBox_Outline_Size->singleStep());
    ui->doubleSpinBox_Outline_threshold->setValue(m_outlineThreshold);
    ui->slider_Outline_threshold->setValue((m_outlineThreshold - ui->doubleSpinBox_Outline_threshold->minimum()) / ui->doubleSpinBox_Outline_threshold->singleStep());

    //ui->Slider_Hatch_Size->setValue((m_hatchSize - ui->doubleSpinBox_Hatch_Size->minimum()) / ui->doubleSpinBox_Hatch_Size->singleStep());
    ui->doubleSpinBox_Hatch_Rotation->setValue(m_hatchRotation);
    ui->Slider_Hatch_Rotation->setValue((m_hatchRotation - ui->doubleSpinBox_Hatch_Rotation->minimum()) / ui->doubleSpinBox_Hatch_Rotation->singleStep());
    ui->doubleSpinBox_Specular_Size->setValue(m_specularSize);
    ui->Slider_Specular_Size->setValue((m_specularSize - ui->doubleSpinBox_Specular_Size->minimum()) / ui->doubleSpinBox_Specular_Size->singleStep());

    ui->spinBox_lighting_steps->setValue(m_toonLightingSteps);
    ui->spinBox_hue_steps->setValue(m_toonHueSteps);
    ui->spinBox_sat_steps->setValue(m_toonSatSteps);
    ui->spinBox_val_steps->setValue(m_toonValSteps);

    if(m_toonType == 0)
    {
        ui->radioButton_toon_select1->setChecked(true);
    }
    else
    {
        ui->radioButton_toon_select2->setChecked(true);
    }

    QPalette palette;
    palette.setColor(QPalette::Base, m_OutlineColor);
    ui->lineEdit_Outline_Color->setPalette(palette);
    palette.setColor(QPalette::Base, m_HatchColor);
    ui->lineEdit_Hatch_Color->setPalette(palette);
    palette.setColor(QPalette::Base, m_Diffuse1);
    ui->lineEdit_Toon_Diff1->setPalette(palette);
    palette.setColor(QPalette::Base, m_Diffuse2);
    ui->lineEdit_Toon_Diff2->setPalette(palette);
    palette.setColor(QPalette::Base, m_Diffuse3);
    ui->lineEdit_Toon_Diff3->setPalette(palette);
    palette.setColor(QPalette::Base, m_Diffuse4);
    ui->lineEdit_Toon_Diff4->setPalette(palette);
    palette.setColor(QPalette::Base, m_Diffuse5);
    ui->lineEdit_Toon_Diff5->setPalette(palette);
    palette.setColor(QPalette::Base, m_Specular);
    ui->lineEdit_Toon_Spec->setPalette(palette);

    ui->doubleSpinBox_Hatch_Size->setValue(m_hatchSize);
    hatchSizeChanged(m_hatchSize);

    for(auto& widget : childWidgets)
    {
        widget->blockSignals(false);
    }
}

void QGMDialogNprSettings::outlineSizeChanged(int value)
{
    //m_outlineSize = ui->doubleSpinBox_Outline_Size->minimum() + ui->doubleSpinBox_Outline_Size->singleStep() * value;
    ui->doubleSpinBox_Outline_Size->blockSignals(true);
    ui->doubleSpinBox_Outline_Size->setValue(ui->doubleSpinBox_Outline_Size->minimum() + ui->doubleSpinBox_Outline_Size->singleStep() * value);
    ui->doubleSpinBox_Outline_Size->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::outlineSizeChanged(double value)
{
    //m_outlineSize = value;
    ui->slider_Outline_Size->blockSignals(true);
    ui->slider_Outline_Size->setValue((value - ui->doubleSpinBox_Outline_Size->minimum()) / ui->doubleSpinBox_Outline_Size->singleStep());
    ui->slider_Outline_Size->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::outlineThresholdChanged(int value)
{
    //m_outlineThreshold = ui->doubleSpinBox_Outline_threshold->minimum() + ui->doubleSpinBox_Outline_threshold->singleStep() * value;
    ui->doubleSpinBox_Outline_threshold->blockSignals(true);
    ui->doubleSpinBox_Outline_threshold->setValue(ui->doubleSpinBox_Outline_threshold->minimum() + ui->doubleSpinBox_Outline_threshold->singleStep() * value);
    ui->doubleSpinBox_Outline_threshold->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::outlineThresholdChanged(double value)
{
    //m_outlineThreshold = value;
    ui->slider_Outline_threshold->blockSignals(true);
    ui->slider_Outline_threshold->setValue((value - ui->doubleSpinBox_Outline_threshold->minimum()) / ui->doubleSpinBox_Outline_threshold->singleStep());
    ui->slider_Outline_threshold->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::hatchSizeChanged(int value)
{
    ui->doubleSpinBox_Hatch_Size->blockSignals(true);
    double logMin = log10(ui->doubleSpinBox_Hatch_Size->minimum());
    double logMax = log10(ui->doubleSpinBox_Hatch_Size->maximum());

    double logVal = ((logMax - logMin) * (value / 100.0)) + logMin;

    ui->doubleSpinBox_Hatch_Size->setValue(pow(10,logVal));

    ui->doubleSpinBox_Hatch_Size->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::hatchSizeChanged(double value)
{

    ui->Slider_Hatch_Size->blockSignals(true);

    double logMin = log10(ui->doubleSpinBox_Hatch_Size->minimum());
    double logMax = log10(ui->doubleSpinBox_Hatch_Size->maximum());
    double logVal = log10(value);

    ui->Slider_Hatch_Size->setValue(((logVal - logMin) / (logMax - logMin)) * 100.0);

    ui->Slider_Hatch_Size->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::hatchRotationChanged(int value)
{
    //m_hatchRotation = ui->doubleSpinBox_Hatch_Rotation->minimum() + ui->doubleSpinBox_Hatch_Rotation->singleStep() * value;
    ui->doubleSpinBox_Hatch_Rotation->blockSignals(true);
    ui->doubleSpinBox_Hatch_Rotation->setValue(ui->doubleSpinBox_Hatch_Rotation->minimum() + ui->doubleSpinBox_Hatch_Rotation->singleStep() * value);
    ui->doubleSpinBox_Hatch_Rotation->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::hatchRotationChanged(double value)
{
    //m_hatchRotation = value;
    ui->Slider_Hatch_Rotation->blockSignals(true);
    ui->Slider_Hatch_Rotation->setValue((value - ui->doubleSpinBox_Hatch_Rotation->minimum()) / ui->doubleSpinBox_Hatch_Rotation->singleStep());
    ui->Slider_Hatch_Rotation->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::specularSizeChanged(int value)
{
    ui->doubleSpinBox_Specular_Size->blockSignals(true);
    ui->doubleSpinBox_Specular_Size->setValue(ui->doubleSpinBox_Specular_Size->minimum() + ui->doubleSpinBox_Specular_Size->singleStep() * value);
    ui->doubleSpinBox_Specular_Size->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::specularSizeChanged(double value)
{
    ui->Slider_Specular_Size->blockSignals(true);
    ui->Slider_Specular_Size->setValue((value - ui->doubleSpinBox_Specular_Size->minimum()) / ui->doubleSpinBox_Specular_Size->singleStep());
    ui->Slider_Specular_Size->blockSignals(false);

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::comboBoxChanged()
{
    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::changeColor()
{
    m_bColorChanged = false;
    m_pColorRequest = sender();
	QColorDialog colorDialog;

	connect(&colorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(colorChanged(QColor)));
    if(m_pColorRequest == ui->pushButton_Outline_Color)
    {
		colorDialog.setCurrentColor(ui->lineEdit_Outline_Color->palette().base().color());
    }

    else if(m_pColorRequest == ui->pushButton_Hatch_Color)
    {
		colorDialog.setCurrentColor(ui->lineEdit_Hatch_Color->palette().base().color());
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff1)
    {
		colorDialog.setCurrentColor(ui->lineEdit_Toon_Diff1->palette().base().color());
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff2)
    {
		colorDialog.setCurrentColor(ui->lineEdit_Toon_Diff2->palette().base().color());
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff3)
    {
		colorDialog.setCurrentColor(ui->lineEdit_Toon_Diff3->palette().base().color());
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff4)
    {
		colorDialog.setCurrentColor(ui->lineEdit_Toon_Diff4->palette().base().color());
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff5)
    {
		colorDialog.setCurrentColor(ui->lineEdit_Toon_Diff5->palette().base().color());
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Spec)
    {
		colorDialog.setCurrentColor(ui->lineEdit_Toon_Spec->palette().base().color());
    }

	colorDialog.exec();
}

void QGMDialogNprSettings::colorChanged(const QColor &color)
{
    m_bColorChanged = true;
    QPalette palette;
    palette.setColor(QPalette::Base, color);

    if(m_pColorRequest == ui->pushButton_Outline_Color)
    {
        //m_OutlineColor = color;
        ui->lineEdit_Outline_Color->setPalette(palette);
    }
    else if(m_pColorRequest == ui->pushButton_Hatch_Color)
    {
        //m_HatchColor = color;
        ui->lineEdit_Hatch_Color->setPalette(palette);
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff1)
    {
        //m_Diffuse1 = color;
        ui->lineEdit_Toon_Diff1->setPalette(palette);
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff2)
    {
        //m_Diffuse2 = color;
        ui->lineEdit_Toon_Diff2->setPalette(palette);
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff3)
    {
        //m_Diffuse3 = color;
        ui->lineEdit_Toon_Diff3->setPalette(palette);
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff4)
    {
        //m_Diffuse4 = color;
        ui->lineEdit_Toon_Diff4->setPalette(palette);
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Diff5)
    {
        //m_Diffuse5 = color;
        ui->lineEdit_Toon_Diff5->setPalette(palette);
    }

    else if(m_pColorRequest == ui->pushButton_Toon_Spec)
    {
        //m_Specular = color;
        ui->lineEdit_Toon_Spec->setPalette(palette);
    }

    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::changeBool()
{
    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::changeToonType(QAbstractButton *button, bool checked)
{
    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::changeIntSpinner(int value)
{
    if(ui->checkBox_Preview->isChecked())
        emitValues();
}

void QGMDialogNprSettings::emitValues()
{
    m_meshGL->setParamFloatMeshGL(MeshGL::NPR_OUTLINE_WIDTH, ui->doubleSpinBox_Outline_Size->value());
    m_meshGL->setParamFloatMeshGL(MeshGL::NPR_OUTLINE_THRESHOLD, ui->doubleSpinBox_Outline_threshold->value());
    m_meshGL->setParamFloatMeshGL(MeshGL::NPR_HATCH_SCALE, ui->doubleSpinBox_Hatch_Size->value());
    m_meshGL->setParamFloatMeshGL(MeshGL::NPR_HATCH_ROTATION, ui->doubleSpinBox_Hatch_Rotation->value());
    m_meshGL->setParamFloatMeshGL(MeshGL::NPR_SPECULAR_SIZE, ui->doubleSpinBox_Specular_Size->value());

    m_meshGL->setParamFlagMeshGL(MeshGL::SHOW_NPR_OUTLINES, ui->checkBox_Outline_enable->isChecked());
    m_meshGL->setParamFlagMeshGL(MeshGL::SHOW_NPR_HATCHLINES, ui->checkBox_Hatch_Enable->isChecked());
    m_meshGL->setParamFlagMeshGL(MeshGL::SHOW_NPR_TOON, ui->checkBox_Toon_Enable->isChecked());
    m_meshGL->setParamFlagMeshGL(MeshGL::NPR_USE_SPECULAR, ui->checkBox_Enable_Specular->isChecked());
    m_meshGL->setParamFlagMeshGL(MeshGL::NPR_HATCHLINE_LIGHT_INFLUENCE, ui->checkBox_Hatch_LightInfluence->isChecked());

    m_meshGL->setParamIntMeshGL(MeshGL::NPR_HATCH_STYLE, ui->comboBox_Hatch_Style->currentIndex());
    m_meshGL->setParamIntMeshGL(MeshGL::NPR_OUTLINE_SOURCE, ui->comboBox_OutlineSource->currentIndex());
    m_meshGL->setParamIntMeshGL(MeshGL::NPR_HATCH_SOURCE, ui->comboBox_HatchSource->currentIndex());
    m_meshGL->setParamIntMeshGL(MeshGL::NPR_TOON_SOURCE, ui->comboBox_ToonSource->currentIndex());

    m_meshGL->setParamIntMeshGL(MeshGL::NPR_TOON_LIGHTING_STEPS, ui->spinBox_lighting_steps->value());
    m_meshGL->setParamIntMeshGL(MeshGL::NPR_TOON_HUE_STEPS, ui->spinBox_hue_steps->value());
    m_meshGL->setParamIntMeshGL(MeshGL::NPR_TOON_SAT_STEPS, ui->spinBox_sat_steps->value());
    m_meshGL->setParamIntMeshGL(MeshGL::NPR_TOON_VAL_STEPS, ui->spinBox_val_steps->value());

    m_meshGL->setParamIntMeshGL(MeshGL::NPR_TOON_TYPE, ui->radioButton_toon_select1->isChecked() ? 0 : 1);


    qreal r,g,b,a;
    GLfloat colorVal[4];
    ui->lineEdit_Outline_Color->palette().color(QPalette::Normal, QPalette::Base).getRgbF(&r, &g, &b, &a);
    colorVal[0] = r; colorVal[1] = g, colorVal[2] = b, colorVal[3] = a;
    mRenderColors->setColorSettings(MeshGLColors::COLOR_NPR_OUTLINE, colorVal);

    ui->lineEdit_Hatch_Color->palette().color(QPalette::Normal, QPalette::Base).getRgbF(&r, &g, &b, &a);
    colorVal[0] = r; colorVal[1] = g, colorVal[2] = b, colorVal[3] = a;
    mRenderColors->setColorSettings(MeshGLColors::COLOR_NPR_HATCHLINE, colorVal);

    ui->lineEdit_Toon_Spec->palette().color(QPalette::Normal, QPalette::Base).getRgbF(&r, &g, &b, &a);
    colorVal[0] = r; colorVal[1] = g, colorVal[2] = b, colorVal[3] = a;
    mRenderColors->setColorSettings(MeshGLColors::COLOR_NPR_SPECULAR, colorVal);

    ui->lineEdit_Toon_Diff1->palette().color(QPalette::Normal, QPalette::Base).getRgbF(&r, &g, &b, &a);
    colorVal[0] = r; colorVal[1] = g, colorVal[2] = b, colorVal[3] = a;
    mRenderColors->setColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE1, colorVal);

    ui->lineEdit_Toon_Diff2->palette().color(QPalette::Normal, QPalette::Base).getRgbF(&r, &g, &b, &a);
    colorVal[0] = r; colorVal[1] = g, colorVal[2] = b, colorVal[3] = a;
    mRenderColors->setColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE2, colorVal);

    ui->lineEdit_Toon_Diff3->palette().color(QPalette::Normal, QPalette::Base).getRgbF(&r, &g, &b, &a);
    colorVal[0] = r; colorVal[1] = g, colorVal[2] = b, colorVal[3] = a;
    mRenderColors->setColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE3, colorVal);

    ui->lineEdit_Toon_Diff4->palette().color(QPalette::Normal, QPalette::Base).getRgbF(&r, &g, &b, &a);
    colorVal[0] = r; colorVal[1] = g, colorVal[2] = b, colorVal[3] = a;
    mRenderColors->setColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE4, colorVal);

    ui->lineEdit_Toon_Diff5->palette().color(QPalette::Normal, QPalette::Base).getRgbF(&r, &g, &b, &a);
    colorVal[0] = r; colorVal[1] = g, colorVal[2] = b, colorVal[3] = a;
    mRenderColors->setColorSettings(MeshGLColors::COLOR_NPR_DIFFUSE5, colorVal);



    emit valuesChanged();
}

void QGMDialogNprSettings::exitAccept()
{
    emitValues();
    accept();
}

void QGMDialogNprSettings::exitReject()
{
    resetValues();
    emitValues();
    reject();
}


void QGMDialogNprSettings::changeEvent(QEvent*event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
	}

	QDialog::changeEvent(event);
}
