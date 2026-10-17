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

#ifndef QGMDIALOGTRANSPARENCYSETTINGS_H
#define QGMDIALOGTRANSPARENCYSETTINGS_H

#include <QDialog>
#include <QRadioButton>
//#include <meshQt.h>
#include "meshGL/meshGL.h"

namespace Ui {
class QGMDialogTransparencySettings;
}

class QGMDialogTransparencySettings : public QDialog
{
    Q_OBJECT

public:
	explicit QGMDialogTransparencySettings(QWidget *parent = nullptr);
	~QGMDialogTransparencySettings() override;

    void init(MeshGL* mesh, bool enableGL4Features);


private:

    void resetValues();
    void emitValues();
    void showALoopSettings(bool val);

    Ui::QGMDialogTransparencySettings *ui;
    int m_TransparencyMethod;
    int m_BufferMethod;

    double m_transVal;
    double m_transVal2;
    double m_Gamma;

    int m_selLabel;
    int m_numLayers;
    int m_overflowHandling;

    int m_checkedBox;
    int m_checkedBufferMethod;

    MeshGL* m_MGL;

    bool m_enableGL4_3Features;


private slots:
    void exitAccept();
    void exitReject();

    void radioButtonChanged(QAbstractButton* button,bool checked);
    void intValueChanged();
    void transValChanged(int value);
    void transValChanged(double value);

    void transValChanged2(int value);
    void transValChanged2(double value);

    void gammaChanged(int value);
    void gammaChanged(double value);

    void comboBoxChanged();

signals:
    void valuesChanged();

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;

};

#endif // QGMDIALOGTRANSPARENCYSETTINGS_H
