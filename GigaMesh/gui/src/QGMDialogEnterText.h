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

#ifndef QGMDialogEnterText_H
#define QGMDialogEnterText_H

// generic C++ includes:
#include <iostream>
#include <set>

// C++ includes:
#include <GigaMesh/mesh/gmcommon.h>

// generic Qt includes:
#include <QtGui>
#include <QDialog>
#include <qstring.h>
//#include <qpalette.h> // for setting colors of e.g. textboxes

// Qt Interface includes:
#include "ui_dialogEnterText.h"

// Qt includes:
#include "QGMMacros.h"
//!
//! \brief Dialog class for simple user input.
//!
//! Emits signal with text entered as string, int and float.
//! Disconnects itself so it can be used for other purposes.
//!
//! Layer 2
//!

class QGMDialogEnterText : public QDialog, private Ui::QGMDialogEnterText {
    Q_OBJECT

public:
	// Constructor and Destructor:
    QGMDialogEnterText( QWidget *parent = nullptr, Qt::WindowFlags flags = {} );

	enum eStringCheck {
		CHECK_DOUBLE_MULTIPLE,
		CHECK_DOUBLE,
		CHECK_INTEGER_MULTIPLE,
		CHECK_INTEGER_UNSIGNED,
		CHECK_NONE
	};

	// Setup -----------------------------------------------------------------------------------------------------------------------------------------------
	void setID( int rID );
	void setText( const std::string& rText );
	void setText( const QString& rText );
	void setText( const std::set<int64_t>& rValues );
	void setText( std::vector<double>& rValues );
	void fetchClipboard( eStringCheck rExpectedType );
	void setDouble( double rValue );
	void setInt( int rValue );
	void setuInt( unsigned int rValue );

	// Retrieve entered information ------------------------------------------------------------------------------------------------------------------------
	bool getText( std::string& rString );
	bool getText( QString* rString );
	bool getText( int* rValue );
	bool getText( unsigned int* rValue );
	bool getText( uint64_t* rValue );
	bool getText( double* rValue );
	bool getText( std::set<int64_t>&   rSet    );
	bool getText( std::vector<long>&   rVector );
	bool getText( std::vector<double>& rVector );
	bool checkAndConvertTextVecDouble( const QString& rEnteredText, std::vector<double>* rValuesReturn=nullptr );

public slots:
	void accept() override;

signals:
	// Simple signals - new Qt5 style: no overloading allowed/possible.
	void textEnteredStr(QString);     //!< emitted when this dialog is successfully closed - see: accept()

	// Simple signals:
	void textEntered(int);         //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(unsigned int);         //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(float);       //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(double);      //!< emitted when this dialog is successfully closed - see: accept()
	// Signals with ID:
	void textEntered(int,QString); //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(int,int);     //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(int,unsigned int);     //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(int,float);   //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(int,double);  //!< emitted when this dialog is successfully closed - see: accept()
	// Array/vector signals
	void textEntered(std::vector<int>);    //!< emitted when this dialog is successfully closed - see: accept()
	void textEntered(std::vector<double>); //!< emitted when this dialog is successfully closed - see: accept()

private:
	int mValueID; //!< Optional ID to identify values to be set, e.g. by MeshGL::setViewParams

	// QWidget interface
	protected:
	virtual void changeEvent(QEvent* event) override;
};

#endif
