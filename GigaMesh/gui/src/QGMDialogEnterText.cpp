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

#include "QGMDialogEnterText.h"

using namespace std;

//! Constructor
QGMDialogEnterText::QGMDialogEnterText( QWidget *parent, Qt::WindowFlags flags )
    : QDialog( parent, flags ) {
	// Init UI elements:
	setupUi( this );
	// Set logo:
	setWindowIcon( QIcon( _GIGAMESH_LOGO_ ) );
}

//! Set the optional ID used to send signals mapped to e.g: MeshGL::setViewParams
void QGMDialogEnterText::setID( int rID ) {
	mValueID = rID;
}

//! Sets the text field to the given value.
void QGMDialogEnterText::setText( const std::string& rText ) {
	someText->setText( rText.c_str() );
}

//! Sets the text field to the given value.
void QGMDialogEnterText::setText( const QString& rText ) {
	someText->setText( rText );
}

//! Sets the text field with a given set of integer values.
void QGMDialogEnterText::setText(
		const std::set<int64_t> &rValues
) {
	QString valuesAsText;
	for( auto const& currValue: rValues ) {
		valuesAsText += QString( "%1 " ).arg( currValue );
	}
	someText->setText( valuesAsText );
}

//! Sets the text field with a given vector of floating point values.
void QGMDialogEnterText::setText( vector<double>& rValues ) {
	QString valuesAsText;
	for( const double& rValue : rValues ) {
		valuesAsText += QString( "%1 " ).arg( rValue );
	}
	someText->setText( valuesAsText );
}

//! Fetch text from the clipboard.
void QGMDialogEnterText::fetchClipboard(
                eStringCheck rExpectedType
) {
	QClipboard *clipboard = QApplication::clipboard();
	QString clipBoardStr = clipboard->text( QClipboard::Clipboard );
	switch( rExpectedType ) {
		case CHECK_DOUBLE_MULTIPLE:
			if( checkAndConvertTextVecDouble( clipBoardStr ) ){
				someText->setText( clipBoardStr );
			}
			break;
		default:
			someText->setText( clipBoardStr );
			break;
	}
}

void QGMDialogEnterText::setDouble( double rValue ) {
	//! Convert rValue to a string used for the text field.
	someText->setText( QString( "%1" ).arg( rValue ) );
}

void QGMDialogEnterText::setInt( int rValue ) {
	//! Convert rValue to a string used for the text field.
	someText->setText( QString( "%1" ).arg( rValue ) );
}

void QGMDialogEnterText::setuInt( unsigned int rValue ) {
	//! Convert rValue to a string used for the text field.
	someText->setText( QString( "%1" ).arg( rValue ) );
}

// --- Retrieve entered information ----------------------------------------------------------------------------------------------------------------------------

//! Get the string as it is.
//! @returns false if no valid value was entered.
bool QGMDialogEnterText::getText( std::string& rString ) {
	rString = someText->text().toStdString();
	return true;
}

//! Get the string as it is.
//! @returns false if no valid value was entered or a NULL pointer was given.
bool QGMDialogEnterText::getText( QString* rString ) {
	// Sanity check
	if( rString == nullptr ) {
		return false;
	}
	(*rString) = someText->text();
	return true;
}

//! Converts the entered string to a single integer value.
//! @returns false if no valid value was entered or a NULL pointer was given.
bool QGMDialogEnterText::getText( int* rValue ) {
	// Sanity check
	if( rValue == nullptr ) {
		return false;
	}
	bool   convertOk;
	double someDouble = someText->text().simplified().toInt( &convertOk );
	if( convertOk ) {
		(*rValue) = someDouble;
		return true;
	}
	return false;
}

//! Converts the entered string to a single unsigned integer value.
//! @returns false if no valid value was entered or a NULL pointer was given.
bool QGMDialogEnterText::getText( unsigned int* rValue ) {
	// Sanity check
	if( rValue == nullptr ) {
		return false;
	}
	bool   convertOk;
	double someDouble = someText->text().simplified().toUInt( &convertOk );
	if( convertOk ) {
		(*rValue) = someDouble;
		return true;
	}
	return false;
}

//! Converts the entered string to a single uint64_t integer value.
//! @returns false if no valid value was entered or a NULL pointer was given.
bool QGMDialogEnterText::getText( uint64_t* rValue ) {
	// Sanity check
	if( rValue == nullptr ) {
		return false;
	}
	bool   convertOk;
	double someDouble = someText->text().simplified().toULong( &convertOk );
	if( convertOk ) {
		(*rValue) = someDouble;
		return true;
	}
	return false;
}

//! Converts the entered string to a single double value.
//! @returns false if no valid value was entered or a NULL pointer was given.
bool QGMDialogEnterText::getText( double* rValue ) {
	// Sanity check
	if( rValue == nullptr ) {
		return false;
	}
	bool   convertOk;
	double someDouble = someText->text().simplified().toDouble( &convertOk );
	if( convertOk ) {
		(*rValue) = someDouble;
		return true;
	}
	return false;
}

//! Converts the entered string to a vector with integer values.
//!
//! Ranges given by one or two colons are supported.
//!
//! @returns false when invalid values were entered. True otherwise.
bool QGMDialogEnterText::getText(
		std::set<int64_t> &rSet   //!< Set to retreive the user supplied values. Will be cleared!
) {
	vector<double> fetchAsDouble;
	if( !getText( fetchAsDouble ) ) {
		return( false );
	}
	rSet.clear();
	for( auto const& currValueDbl: fetchAsDouble ) {
		int64_t currValueInt = lround( currValueDbl );
		rSet.insert( currValueInt );
	}
	return( true );
}

//! Converts the entered string to a vector with integer values.
//!
//! Ranges given by one or two colons are supported.
//!
//! @returns false when invalid values were entered. True otherwise.
bool QGMDialogEnterText::getText(
                vector<long>& rVector   //!< Vector to retreive the user supplied values. Will be cleared!
) {
	vector<double> fetchAsDouble;
	if( !getText( fetchAsDouble ) ) {
		return( false );
	}
	rVector.clear();
	for( auto const& currValueDbl: fetchAsDouble ) {
		long currValueInt = lround( currValueDbl );
		rVector.push_back( currValueInt );
	}
	return( true );
}

//! Converts the entered string to a vector with double values.
//!
//! Ranges given by colons are supported, e.g:
//! 3:7 will add 3, 4, 5, 6 and 7
//! 4:-0.5:2 will add 4, 3.5, 3, 2.5 and 2
//!
//! @returns false when invalid values were entered. True otherwise.
bool QGMDialogEnterText::getText(
                vector<double>& rVector   //!< Vector to retreive the user supplied values. Will be cleared!
) {
	if( !checkAndConvertTextVecDouble( someText->text(), &rVector ) ) {
		std::cout << "[QGMDialogEnterText::" << __FUNCTION__ << "] Warning: invalid values entered!" << std::endl;
		return( false );
	}
	return( true );
}

//! Check and optionally convert the entered text to a std::vector<double>.
//!
//! Ranges given by colons are supported, e.g:
//! 3:7 will add 3, 4, 5, 6 and 7
//! 4:-0.5:2 will add 4, 3.5, 3, 2.5 and 2
//!
//! @returns false in case of an error or invalid text. True otherwise.
bool QGMDialogEnterText::checkAndConvertTextVecDouble(
                const QString&          rEnteredText,
                std::vector<double>*   rValuesReturn
) {
	bool            someTokensAreNoDouble = false;
    QStringList     someTokens  = rEnteredText.simplified().split( " ", QString::SkipEmptyParts );
	int             someTokenNr = someTokens.size();
	vector<double>  someTokenDouble;
	for( int i=0; i<someTokenNr; i++ ) {
		bool   convertOk;
		double tmpDouble = someTokens.at( i ).toDouble( &convertOk );
		if( !convertOk ) {
			// Check if it is a range
			int countColons = someTokens.at( i ).count( ":" );
			if( countColons == 0 ) {
				// No range
				someTokensAreNoDouble = true;
				continue;
			}
            QStringList colonTokens = someTokens.at( i ).split( ":", QString::SkipEmptyParts );
			double rangeStart = colonTokens.at( 0 ).toDouble( &convertOk );
			if( !convertOk ) {
				// No valid range
				someTokensAreNoDouble = true;
				continue;
			}
			double rangeStep = 1.0;
			double rangeStop;
			if( countColons == 1 ) {
				rangeStop  = colonTokens.at( 1 ).toDouble( &convertOk );
				if( !convertOk ) {
					// No valid range
					someTokensAreNoDouble = true;
					continue;
				}
			} else if( countColons == 2 ) {
				rangeStep = colonTokens.at( 1 ).toDouble( &convertOk );
				if( !convertOk ) {
					// No valid range
					someTokensAreNoDouble = true;
					continue;
				}
				rangeStop  = colonTokens.at( 2 ).toDouble( &convertOk );
				if( !convertOk ) {
					// No valid range
					someTokensAreNoDouble = true;
					continue;
				}
			} else {
				// Otherwise
				std::cout << "[QGMDialogEnterText::" << __FUNCTION__ << "] Warning unspecified number of colon(s): " << countColons << std::endl;
				someTokensAreNoDouble = true;
				continue;
			}
			std::cout << "[QGMDialogEnterText::" << __FUNCTION__ << "] Range entered from " << rangeStart << " to " << rangeStop << " with stepping " << rangeStep << std::endl;
			std::cout << "[QGMDialogEnterText::" << __FUNCTION__ << "] ";

			//https://wiki.sei.cmu.edu/confluence/display/c/FLP30-C.+Do+not+use+floating-point+variables+as+loop+counters

			if( rangeStart < rangeStop ) {
				for( double insertValue=rangeStart; insertValue<=rangeStop; insertValue+=rangeStep ) {
					std::cout << insertValue << " ";
					someTokenDouble.push_back( insertValue );
				}
			} else {
				for( double insertValue=rangeStart; insertValue>=rangeStop; insertValue+=rangeStep ) {
					std::cout << insertValue << " ";
					someTokenDouble.push_back( insertValue );
				}
			}
			std::cout << std::endl;
			continue; // We already have pushed the values.
		}
		someTokenDouble.push_back( tmpDouble );
	}
	if( !someTokensAreNoDouble ) {
		if( rValuesReturn != nullptr ) {
			rValuesReturn->swap( someTokenDouble );
		}
		return( true );
	}
	return( false );
}

// --- SLOTS ---------------------------------------------------------------------------------------------------------------------------------------------------

//! Called when OK is pressed - emits signal with text entered as string, int and float.
//! Disconnects itself so it can be used for other purposes.
//!
//! Supports:
void QGMDialogEnterText::accept() {

	bool convertOk;

	emit textEnteredStr( someText->text() );
	emit textEntered( mValueID, someText->text() );

	//! *) Single Integer
	int someInt = someText->text().toInt( &convertOk );
	if( convertOk ) {
		emit textEntered( someInt );
		emit textEntered( mValueID, someInt );
	}

	//! *) Single unsigned Integer
	unsigned int someuInt = someText->text().toUInt( &convertOk );
	if( convertOk ) {
		emit textEntered( someuInt );
		emit textEntered( mValueID, someuInt );
	}

	//! *) Single Float
	float someFloat = someText->text().toFloat( &convertOk );
	if( convertOk ) {
		emit textEntered( someFloat );
		emit textEntered( mValueID, someFloat );
	}

	//! *) Single Double
	double someDouble = someText->text().toDouble( &convertOk );
	if( convertOk ) {
		emit textEntered( someDouble );
		emit textEntered( mValueID, someDouble );
	}

	//! *) Not a number Double & Float
	if( someText->text().toLower() == "nan" ) {
		emit textEntered( _NOT_A_NUMBER_DBL_ );
		emit textEntered( mValueID, _NOT_A_NUMBER_DBL_ );
		emit textEntered( _NOT_A_NUMBER_ );
		emit textEntered( mValueID, _NOT_A_NUMBER_ );
	}

	//! *) Arrays of
    QStringList someTokens  = someText->text().split( " ", QString::SkipEmptyParts );
	int         someTokenNr = someTokens.size();

	//! -) Vector of Integers
	bool someTokensAreNoInt = false;
	vector<int> someTokenInt;
	for( int i=0; i<someTokenNr; i++ ) {
		bool convertOk;
		int  tmpInteger = someTokens.at( i ).toInt( &convertOk );
		if( !convertOk ) {
			someTokensAreNoInt = true;
			continue;
		}
		someTokenInt.push_back( tmpInteger );
	}
	if( !someTokensAreNoInt ) {
		emit textEntered( someTokenInt );
	}

	//! -) Vector of Doubles
	bool someTokensAreNoDouble = false;
	vector<double> someTokenDouble;
	for( int i=0; i<someTokenNr; i++ ) {
		bool   convertOk;
		double tmpDouble = someTokens.at( i ).toDouble( &convertOk );
		if( !convertOk ) {
			someTokensAreNoDouble = true;
			continue;
		}
		someTokenDouble.push_back( tmpDouble );
	}
	if( !someTokensAreNoDouble ) {
		emit textEntered( someTokenDouble );
	}

	//! Disconnects all connections - so other other objects can reuse an instance of this class.
	QObject::disconnect();
	QDialog::accept();
}


void QGMDialogEnterText::changeEvent(QEvent*event)
{
	if(event->type() == QEvent::LanguageChange)
	{
		retranslateUi(this);
	}

	QDialog::changeEvent(event);
}
