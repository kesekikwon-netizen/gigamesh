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

#ifndef QGMDIALOGNPRSETTINGS_H
#define QGMDIALOGNPRSETTINGS_H

#include <QDialog>
#include <QColorDialog>
#include <QRadioButton>
#include <GigaMesh/mesh/gmcommon.h>
#include "meshGL/meshGL.h"
#include "meshwidget.h"

namespace Ui {
class QGMDialogNprSettings;
}

class QGMDialogNprSettings : public QDialog
{
    Q_OBJECT

public:
	explicit QGMDialogNprSettings(QWidget *parent = nullptr);
    ~QGMDialogNprSettings();

	void init( MeshGL* rMesh, MeshGLColors* rRenderColors );

private:

    void emitValues();
    void resetValues();

    Ui::QGMDialogNprSettings *ui;
    double m_outlineSize;
    double m_outlineThreshold;
    double m_hatchSize;
    double m_hatchRotation;
    double m_specularSize;

    bool m_bOutlineEnabled;
    bool m_bHatchEnabled;
    bool m_bToonEnabled;
    bool m_bPreview;
    bool m_bSpecularEnabled;

    bool m_bColorChanged;

    bool m_bHatchLInfluence;
    int m_hatchStyle;

    int m_hatchSource;
    int m_toonSource;
    int m_outlineSource;

    int m_toonType;
    int m_toonLightingSteps;
    int m_toonHueSteps;
    int m_toonSatSteps;
    int m_toonValSteps;

    QColor m_OutlineColor;
    QColor m_HatchColor;
    QColor m_Diffuse1;
    QColor m_Diffuse2;
    QColor m_Diffuse3;
    QColor m_Diffuse4;
    QColor m_Diffuse5;
    QColor m_Specular;

	//QColorDialog* m_pColorDialog;
    QObject* m_pColorRequest;

	MeshGL*         m_meshGL;
	MeshGLColors*   mRenderColors;

private slots:
    void outlineSizeChanged(int value);
    void outlineSizeChanged(double value);

    void outlineThresholdChanged(int value);
    void outlineThresholdChanged(double value);

    void hatchSizeChanged(int value);
    void hatchSizeChanged(double value);

    void hatchRotationChanged(int value);
    void hatchRotationChanged(double value);

    void specularSizeChanged(int value);
    void specularSizeChanged(double value);

    void comboBoxChanged();

    void changeColor();
    void colorChanged(const QColor& color);

    void changeBool();

    void changeToonType(QAbstractButton* button,bool checked);
    void changeIntSpinner(int value);

    void exitAccept();
    void exitReject();

signals:
    void valuesChanged();


	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDIALOGNPRSETTINGS_H
