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

#ifndef QGMDIALOGSLIDERHD_H
#define QGMDIALOGSLIDERHD_H

#include <QDialog>
#include <QDoubleValidator>

namespace Ui {
class QGMDialogSliderHD;
}

//!
//! \brief Dialog class for GigaMesh's (single) Slider with preview checkbox (Layer 2)
//!
//! ...
//!
//! Layer 2
//!

class QGMDialogSliderHD : public QDialog
{
		Q_OBJECT

	public:
		// Constructor and Destructor:
		explicit QGMDialogSliderHD(QWidget *parent = nullptr);
		~QGMDialogSliderHD() override;

		int    setIdx( int    setIdx    );
		double setMin( double setMinVal );
		double setMax( double setMaxVal );
		int    setSteps( int rMaxSteps  );
		double setPos( double setPos    );
		double setFactor( double rFactor );
		bool   setInverted( bool setTo  );
		bool   setLogarithmic( bool rSetTo );
		void   suppressPreview();

		// Value access
		bool getValue( double* rValue );

	public slots:
		void accept() override;
		void reject() override;

	private slots:
		void previewChecked( bool state );
		void valueChangedRel(const QString& rValRel );
		void valueChangedAbs(const QString& rValAbs );
		void valueChanged( int sliderPos );

	signals:
		void valuePreviewInt(int);             //!< Emitted, when preview is selected and the slider is moved - emits round(float).
		void valuePreviewFloat(double);        //!< Emitted, when preview is selected and the slider is moved.
		void valuePreviewIdFloat(int,double);  //!< Emitted, when preview is selected and the slider is moved. Passing thru the index.
		void valueSelected(int);        //!< Emitted, when "close" (initialValue) or "ok" (value from slider) is pressed - emits round(float).
		void valueSelected(double);     //!< Emitted, when "close" (initialValue) or "ok" (value from slider) is pressed.
		void valueSelected(int,double); //!< Emitted, when "close" (initialValue) or "ok" (value from slider) is pressed. Passing thru the index.

	private:
		Ui::QGMDialogSliderHD *ui; //!< Added by the Qt framework.

		int    mIndex;        //!< Index to passed thru.
		double mInitialValue; //!< Initial value in case "cancel" is pressed and preview changed stuff.
		double mMinVal;       //!< Minimum value corresponding to the sliders minimum.
		double mMaxVal;       //!< Maximum value corresponding to the sliders maximum.
		double mFactor;       //!< Factor to be added e.g. to show angles in degree instead of radiant.
		bool   mLogarithmic;  //!< Convert relative (tick mark) values to logarithmic absolut values).

		QDoubleValidator mValidRel; //!< Validator for lineEdit of the relative value.
		QDoubleValidator mValidAbs; //!< Validator for lineEdit of the absolute value.

		void emitValueSelected(double value); //!< Helper function to emit selected values with correct scale
		void emitValuePreview(double value);  //!< Helper function to emit preview values with correct scale

		// QWidget interface
	protected:
		virtual void changeEvent(QEvent* event) override;
};

#endif // QGMDIALOGSLIDERHD_H
