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

#include <GigaMesh/mesh/primitive.h>

#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/face.h>

#define _PRIMITIVE_DEFAULT_GRAYVALUE_           0.5

#define _VERTEX_DEFAULT_COLOR_COMPONENT_RED_    128
#define _VERTEX_DEFAULT_COLOR_COMPONENT_GREEN_  128
#define _VERTEX_DEFAULT_COLOR_COMPONENT_BLUE_   128
#define _VERTEX_DEFAULT_GRAYSCALE_              0.5

using namespace std;

//#define PRIMITVE_INITDEFAULTS
//	mBitFlags( 0x00000000 | FLAG_LABEL_NOT )

//! Constructor setting up the common properties of geometric primitives.
Primitive::Primitive () {
	//: PRIMITVE_INITDEFAULTS {
	// Set flag:
	setFlag( Primitive::FLAG_LABEL_NOT );
}

// Indexing --------------------------------------------------------------------

bool Primitive::setIndex( int setIdx ) {
	//! Setes the index of a Primitive. Additionally it preserves the very first
	//! index set. Returns false in case the index has not been set.
	//! Will return false, when not overwritten.
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: can not set index to: " << setIdx << "!" << endl;
	return false;
}

int Primitive::getIndex() const {
	//! Retrieves the primitives index. Typcally used to write a mesh to a file.
	//!
	//! Attention: the returned index might be wrong due to alterations to the 
	//! the meshs structure or not set at all.
	return -1;
}

string Primitive::getIndexStr() {
	//! Returns the primitives index as string.
	//!
	//! Attention: the returned index might be wrong due to alterations to the
	//! the meshs structure or not set at all.
	char buffer[255];
	sprintf( buffer, "%i", getIndex() );
	return string( buffer );
}

int Primitive::getIndexOriginal() {
	//! Retrieves the primitives original index, when a Mesh file was read.
	return -1;
}

// Color managment -------------------------------------------------------------

bool Primitive::setRGB( unsigned char setTexR, unsigned char setTexG, unsigned char setTexB ) {
	//! Sets R,G and B of a primitves texture color. 
	//!
	//! Remark: will return false and show an error message, when the primitives
	//! pointers to the texture map array are NULL.
	//!
	//! Returns false in case of an error.
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: can not set color to: " << setTexR << " " << setTexG << " " << setTexB << "!" << endl;
	return false;
}

bool Primitive::getRGBInverted( double* texRGBInv ) {
	//! Inverts the color using 1-val in HSV-color spaces and
	//! sets the (inverted) RGB values into the given array double[3].
	//! IMPORTANT: texRGBInv has to of length 3!

	//! *) Fetch RGB.
	unsigned char colorRGB[3];
	if( !copyRGBTo( colorRGB ) ) {
		cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: unknown!" << endl;
		return false;
	}

	//! *) Convert RGB to HSV 
	double texRGB[3] = { (static_cast<double>(colorRGB[0]))/255.0,
	                     (static_cast<double>(colorRGB[1]))/255.0,
	                     (static_cast<double>(colorRGB[2]))/255.0 };
	double texHSV[3];
	if( !convertRGBtoHSV( texRGB, texHSV ) ) {
		cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: unknown!" << endl;
		return false;
	}

	//! *) Invert color by inverting val.
	texHSV[2] = 1.0-texHSV[2];

	//! *) Convert inverted HSV to RGB.
	if( !convertHSVtoRGB( texHSV, texRGBInv ) ) {
		cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: unknown!" << endl;
		return false;
	}

	return true;
}

//! Converts HSV to RGB, but does NOT alter the stored texture-map!
//!
//! Returns false, when an error occured, which may also caused by setRGB.
//!
//! All three parameters are expected to be within [0...1], even hue is
//! sometimes considered as degree [0°...360°].
//!
//! See also: http://en.wikipedia.org/wiki/HSL_and_HSV#Conversion_from_HSV_to_RGB
bool Primitive::convertHSVtoRGB( double* texHSVin, double* texRGBout ) {
	int   hueIndex = static_cast<int>(floor( 6 * texHSVin[0])) % 6;
	double f = ( 6 * texHSVin[0]) - floor( 6* texHSVin[0]);
	double p = texHSVin[2] * ( 1.0 - texHSVin[1] );
	double q = texHSVin[2] * ( 1.0 - ( f * texHSVin[1] ) );
	double t = texHSVin[2] * ( 1.0 - ( 1.0 - f ) * texHSVin[1] );

	switch( hueIndex ) {
		case 0:
				texRGBout[0] = texHSVin[2]; texRGBout[1] = t; texRGBout[2] = p;
			break;
		case 1:
				texRGBout[0] = q; texRGBout[1] = texHSVin[2]; texRGBout[2] = p;
			break;
		case 2:
				texRGBout[0] = p; texRGBout[1] = texHSVin[2]; texRGBout[2] = t;
			break;
		case 3:
				texRGBout[0] = p; texRGBout[1] = q; texRGBout[2] = texHSVin[2];
			break;
		case 4:
				texRGBout[0] = t; texRGBout[1] = p; texRGBout[2] = texHSVin[2];
			break;
		case 5:
				texRGBout[0] = texHSVin[2]; texRGBout[1] = p; texRGBout[2] = q;
			break;
		default:
			cerr << getCoutPref() << getIndex() << " intexHSVin[2]id hueIndex " << hueIndex << " estimated! Hue was: " << texHSVin[0]<< endl;
			return false;
	}

	return true;
}

//! Convert RGB to HSV - both arrays given have to be of size double * 3.
//! \todo IMPLEMENT!
bool Primitive::convertRGBtoHSV( double* texRGBin, double* texHSVout ) {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: not implemented. Values at addresses " << texRGBin << "," << texHSVout << "were not changed!" << endl;
// 	double maxRGB = getMinRGB();
// 	double minRGB = getMaxRGB();
// 	double diffRGB = maxRGB - minRGB;

	// hue
// 	double hue;
// 	if( maxRGB == minRGB ) {
// 		hue = 0.0;
// 	} else if ( maxRGB == TEX_RED ) {
// 		hue = fmod( ( 360.0 + 60.0 * ( TEX_GREEN - TEX_BLUE ) / diffRGB ), 360 );
// 	} else if ( maxRGB == TEX_GREEN ) {
// 		hue = ( 120.0 + 60.0 * ( TEX_BLUE - TEX_RED ) / diffRGB );
// 	} else if ( maxRGB == TEX_BLUE ) {
// 		hue = ( 240.0 + 60.0 * ( TEX_RED - TEX_GREEN ) / diffRGB );
// 	} else {
		// error
// 		return false;
// 	}

	// sat
// 	double sat;
// 	if( maxRGB == 0.0 ) {
// 		sat = 0.0;
// 	} else {
// 		sat = 1.0 - ( minRGB / maxRGB );
// 	}

// 	texHSV[0] = hue / 360.0;
// 	texHSV[1] = sat;
// 	texHSV[2] = maxRGB; // val = maxRGB
	return false;
}

//! Returns Hue-Sat-Val representation of a Primitves RGB values.
//! Returns false in case of an error - e.g. Primitive without a RGB-value.
bool Primitive::getHSV( double* rHSV ) {
	if( rHSV == nullptr ) {
		cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}

	unsigned char valRGB[3];
	if( !copyRGBTo( valRGB ) ) {
		return false;
	}

	// max( red, green, blue )
	int maxRGB = valRGB[0];
	if( maxRGB < valRGB[1] ) {
		maxRGB = valRGB[1];
	}
	if( maxRGB < valRGB[2] ) {
		maxRGB = valRGB[2];
	}

	// min( red, green, blue )
	int minRGB = valRGB[0];
	if( minRGB > valRGB[1] ) {
		minRGB = valRGB[1];
	}
	if( minRGB > valRGB[2] ) {
		minRGB = valRGB[2];
	}
	int diffRGB = maxRGB - minRGB;

	// Hue
 	double hue;
 	if( maxRGB == minRGB ) {
 		hue = 0.0;
 	} else if ( maxRGB == valRGB[0] ) {
 		hue = fmod( ( 360.0 + 60.0 * ( valRGB[1] - valRGB[2] ) / diffRGB ), 360 );
 	} else if ( maxRGB == valRGB[1] ) {
 		hue = ( 120.0 + 60.0 * ( valRGB[2] - valRGB[0] ) / diffRGB );
 	} else if ( maxRGB == valRGB[2] ) {
 		hue = ( 240.0 + 60.0 * ( valRGB[0] - valRGB[1] ) / diffRGB );
	} else {
		// error
		cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: maxRGB does not match!" << endl;
 		return false;
 	}

	// Saturation
 	double sat;
 	if( maxRGB == 0 ) {
 		sat = 0.0;
 	} else {
 		sat = 1.0 - ( static_cast<double>(minRGB) / static_cast<double>(maxRGB) );
 	}

 	rHSV[0] = hue;
 	rHSV[1] = sat;
 	rHSV[2] = static_cast<double>(maxRGB); // Value == maxRGB
	return true;
}

//! Stub to set alpha/transparency.
//! Returns false in case of an error.
bool Primitive::setAlpha( double rVal ) {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not set alpha to '" << rVal << "' (" << getTypeStr() << ")!" << endl;
	return false;
}

// Color mapping -------------------------------------------------------------------------------

//! Sets the texture color from red (0.0) to green (1.0) via yellow using
//! HSV. Therefore sat(uration) and val(ue) are optional arguments.
//!
//! Returns true when phase is within the interval [0...1]. False otherwise.
bool Primitive::setTrafficLight( double phase, double sat, double val ) {
	double texHSV[3] = { 0.0, sat, val }; // hue, sat, val
	double texRGB[3] = { 0.0, 0.0, 0.0 };
	if( phase <= 0.0 ) {
		convertHSVtoRGB( texHSV, texRGB );
		setRGB( static_cast<unsigned char>(round( texRGB[0]*255.0 )), static_cast<unsigned char>(round( texRGB[1]*255.0 )), static_cast<unsigned char>(round( texRGB[2]*255.0 )) );
		return false;
	}
	if( phase >= 1.0 ) {
		texHSV[0] = 120.0/360.0;
		convertHSVtoRGB( texHSV, texRGB );
		setRGB( static_cast<unsigned char>(round( texRGB[0]*255.0 )), static_cast<unsigned char>(round( texRGB[1]*255.0 )), static_cast<unsigned char>(round( texRGB[2]*255.0 )) );
		return false;
	}
	texHSV[0] = phase*120.0/360.0;
	convertHSVtoRGB( texHSV, texRGB );
	setRGB( static_cast<unsigned char>(round( texRGB[0]*255.0 )), static_cast<unsigned char>(round( texRGB[1]*255.0 )), static_cast<unsigned char>(round( texRGB[2]*255.0 )) );
	return true;
}

bool Primitive::setBlackWhiteBlack( double phase ) {
	//! Sets the texture color from black (0.0) via white (0.5) to black (1.0).
	//! 
	//! Returns true when phase is within the interval [0...1]. False otherwise.

	if( phase <= 0.0 ) {
		setRGB( 0, 0, 0 );
		return false;
	}
	if( phase >= 1.0 ) {
		setRGB( 0, 0, 0 );
		return false;
	}
	phase *= 2.0;
	if( phase > 1.0 ) {
		phase = 2.0-phase;
	}
	setRGB( static_cast<unsigned char>(255.0 * phase),
	        static_cast<unsigned char>(255.0 * phase),
	        static_cast<unsigned char>(255.0 * phase ));
	return true;
}

//! Sets the texture color from white (0.0) via black (0.5) to white (1.0).
//!
//! Returns true when phase is within the interval [0...1]. False otherwise.
bool Primitive::setWhiteBlackWhite( double phase ) {

	if( phase <= 0.0 ) {
		setRGB( 255, 255, 255 );
		return false;
	}
	if( phase >= 1.0 ) {
		setRGB( 255, 255, 255 );
		return false;
	}
	phase *= 2.0;
	if( phase > 1.0 ) {
		phase = 2.0-phase;
	}
	setRGB( static_cast<unsigned char>(255.0 * (1.0-phase)),
	        static_cast<unsigned char>(255.0 * (1.0-phase)),
	        static_cast<unsigned char>(255.0 * (1.0-phase )));
	return true;
}

//! Sets the texture color from white (0.0) via black (0.5) to white (1.0).
//!
//! Returns true when phase is within the interval [0...1]. False otherwise.
bool Primitive::setBlackWhite( double phase ) {

	if( phase < 0.0 ) {
		setRGB( 0, 0, 0 );
		return false;
	}
	if( phase > 1.0 ) {
		setRGB( 255, 255, 255 );
		return false;
	}
	setRGB( static_cast<unsigned char>(255.0 * phase),
	        static_cast<unsigned char>(255.0 * phase),
	        static_cast<unsigned char>(255.0 * phase) );
	return true;
}

//! Sets the texture color from white (0.0) via black (0.5) to white (1.0).
//!
//! Returns true when phase is within the interval [0...1]. False otherwise.
bool Primitive::setWhiteBlack( double phase ) {

	if( phase < 0.0 ) {
		setRGB( 255, 255, 255 );
		return false;
	}
	if( phase > 1.0 ) {
		setRGB( 0, 0, 0 );
		return false;
	}
	setRGB( static_cast<unsigned char>( 255.0 * (1.0-phase)),
	        static_cast<unsigned char>( 255.0 * (1.0-phase)),
	        static_cast<unsigned char>( 255.0 * (1.0-phase)) );
	return true;
}

// --- Function Values --------------------------------------------------------

//! Stub for setting the function value.
//! Returns false in case of an error (or not implemented).
bool Primitive::setFuncValue( [[maybe_unused]] double setVal  ) {
	return false;
}

//! Stub for retrieving the function value.
//! Returns false in case of an error (or not implemented).s
bool Primitive::getFuncValue( [[maybe_unused]] double* rGetVal  ) const {
	return false;
}

// --- Transformation --------------------------------------------------------

//! Applies (multiplies) a given homogenous transformation matrix.
//! Returns false, when the application fails.
bool Primitive::applyTransfrom( Matrix4D* rTransMat ) {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Not implemented for " << getTypeStr() << ". No transformation performed using [ " << rTransMat->get( 0, 0 ) << ", ... ] !" << endl;
	return false;
}

//! Applies melting with sqrt(r^2-x^2-y^2).
//! Returns false, when the application fails, when
//! the method is not implemented or
//! the Primitve is outside the radius.
bool Primitive::applyMeltingSphere( double rRadius, double rRel ) {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Not implemented for " << getTypeStr() << ". Radius = " << rRadius << ", rel = " << rRel << " !" << endl;
	return false;
}

// --- Labeling --------------------------------------------------------------

//! Stub for setting a label id.
//!
//! Zero is not used for labels as it is meant for the background label.
//! Setting a label to zero will be interpreted as error.
//!
//! @returns false in case of an error e.g. not implemented.
bool Primitive::setLabel( uint64_t rSetLabelNr ) {
	if( rSetLabelNr == 0 ) {
		return( false );
	}
	clearFlag( FLAG_LABEL_NOT );
	clearFlag( FLAG_LABEL_BACKGR );
	return( true );
}

//! Sets the flag for having no label.
//! Returns false in case of an error (or not implemented).
bool Primitive::setLabelNone() {
	setFlag( FLAG_LABEL_NOT );
	clearFlag( FLAG_LABEL_BACKGR );
	return true;
}

//! Sets the flag for having no label.
//! Returns false in case of an error (or not implemented).
bool Primitive::setLabelBackGround() {
	clearFlag( FLAG_LABEL_NOT );
	setFlag( FLAG_LABEL_BACKGR );
	return true;
}

//! Returns the label nr.
//! Returns false, when no label ist set or in case of an error (or not implemented).
//! When false is returned labelNr is not changed.
bool Primitive::getLabel( uint64_t& rGetLabelNr ) const {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not write label Nr to " << rGetLabelNr << "!" << endl;
	return( false );
}

//! Returns true, when a label is set.
//! This means, that the Primitive is neither background nor unlabled.
bool Primitive::isLabled() const {
	if( getFlag( FLAG_LABEL_BACKGR ) ) {
		return false;
	}
	if( getFlag( FLAG_LABEL_NOT ) ) {
		return false;
	}
	return true;
}

//! Returns true, when the Primitive is tagged as part of the background.
bool Primitive::isLabelBackGround() {
	return getFlag( FLAG_LABEL_BACKGR );
}

//! Returns the color corresponding to the label nr. (modulo amout of colors) to colRGB.
//! Returns false in case the Primitive is not label or an error occured.
//! This method has hard-coded color values to maintain speed.
bool Primitive::getLabelColor( unsigned char* colRGB, int colorMap ) {
	uint64_t myLabel = 0;
	// We have to get the label no. thru the (overloaded!) method:
	if( !getLabel( myLabel ) ) {
		return false;
	}
	int idxCol = MODULO_INT( myLabel, 11 );
	return getColor( colRGB, idxCol, colorMap );
}

//! Returns the color corresponding to the label nr. (modulo amout of colors) to colRGB.
//! Returns false in case the Primitive is not label or an error occured.
//! This method has hard-coded color values to maintain speed.
bool Primitive::getColor( unsigned char* colRGB, int idxCol, int colorMap ) {

	switch( colorMap ) {
		case COLMAP_BREWER_PAIRED:
			switch( idxCol ) {
				case 0:
					colRGB[0] = 166; colRGB[1] = 206; colRGB[2] = 227;
					return true;
				case 1:
					colRGB[0] =  31; colRGB[1] = 120; colRGB[2] = 180;
					return true;
				case 2:
					colRGB[0] = 178; colRGB[1] = 223; colRGB[2] = 138;
					return true;
				case 3:
					colRGB[0] =  51; colRGB[1] = 160; colRGB[2] =  44;
					return true;
				case 4:
					colRGB[0] = 251; colRGB[1] = 154; colRGB[2] = 153;
					return true;
				case 5:
					colRGB[0] = 227; colRGB[1] =  26; colRGB[2] =  28;
					return true;
				case 6:
					colRGB[0] = 253; colRGB[1] = 191; colRGB[2] = 111;
					return true;
				case 7:
					colRGB[0] = 255; colRGB[1] = 127; colRGB[2] =   0;
					return true;
				case 8:
					colRGB[0] = 202; colRGB[1] = 178; colRGB[2] = 214;
					return true;
				case 9:
					colRGB[0] = 106; colRGB[1] =  61; colRGB[2] = 154;
					return true;
				case 10:
					colRGB[0] = 255; colRGB[1] = 255; colRGB[2] = 153;
					return true;
				default:
					cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Index out of range: " << idxCol << "!" << endl;
					return false;
			}
			break;
		case COLMAP_BREWER_SET1:
			switch( idxCol ) {
				case 0:
					colRGB[0] = 228; colRGB[1] =  26; colRGB[2] =  28;
					return true;
				case 1:
					colRGB[0] =  55; colRGB[1] = 126; colRGB[2] = 184;
					return true;
				case 2:
					colRGB[0] =  77; colRGB[1] = 175; colRGB[2] =  74;
					return true;
				case 3:
					colRGB[0] = 152; colRGB[1] =  78; colRGB[2] = 163;
					return true;
				case 4:
					colRGB[0] = 255; colRGB[1] = 127; colRGB[2] =   0;
					return true;
				default:
					cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Index out of range: " << idxCol << "!" << endl;
					return false;
			}
			break;
		default:
			cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Unknown color map: " << colorMap << "!" << endl;
			return false;
	}
	// We should never reach this point:
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Unknown error!" << endl;
	return false;
}

// --- information retrival ---------------------------------------------------

//! Returns the x-coordinate of the Primitive's center of gravity.
//! Return not-a-number in case of an error.
double Primitive::getX() const {
	return _NOT_A_NUMBER_;
}

//! Returns the y-coordinate of the Primitive's center of gravity.
//! Return not-a-number in case of an error.
double Primitive::getY() const {
	return _NOT_A_NUMBER_;
}

//! Returns the z-coordinate of the Primitive's center of gravity.
//! Return not-a-number in case of an error.
double Primitive::getZ() const {
	return _NOT_A_NUMBER_;
}

//! Returns the x-coordinate of the Primitive's normal.
//! Return not-a-number in case of an error.
double Primitive::getNormalX() {
	return _NOT_A_NUMBER_;
}

//! Returns the y-coordinate of the Primitive's normal.
//! Return not-a-number in case of an error.
double Primitive::getNormalY() {
	return _NOT_A_NUMBER_;
}

//! Returns the z-coordinate of the Primitive's normal.
//! Return not-a-number in case of an error.
double Primitive::getNormalZ() {
	return _NOT_A_NUMBER_;
}

//! Returns the center of gravity, which is stored in an external array.
//! When the reference to this array is not set Vector3D( nan, nan, nan, 1.0 )
//! will be returned!
Vector3D Primitive::getCenterOfGravity() {
	return Vector3D( getX(), getY(), getZ(), 1.0 );
}

//! Returns the center of gravity having the given transformation matrix applied.
//!
//! @returns false in case of an error. True otherwise.
bool Primitive::getTransformedCenterOfGravity(
		const Matrix4D& rTransMat,   //!< Input: Matrix to be applied.
		Vector3D& rCoGTrans          //!< Output: transformed center of gravity.
) {
	//! \todo properly test!
	rCoGTrans = getCenterOfGravity() * rTransMat;
	return( true );
}

Vector3D Primitive::getNormal( bool normalized ) {
	//! Returns the normal vector.
	//! Returns (nan,nan,nan,0.0) in case of an error.
	Vector3D normalVec( getNormalX(), getNormalY(), getNormalZ(), 0.0 );
	if( normalized ) {
		normalVec.normalize3();
	}
	return normalVec;
}

//! Returns the hessian normal form of the primitive, when a position/centerofgravity and a normal is set.
//! May return not-a-number, in case of no normal or no position.
//! @returns False in case of an error. True otherwise.
bool Primitive::getPlaneHNF( Vector3D* rPlaneHNF ) {
	if( rPlaneHNF == nullptr ) {
		return false;
	}
	// This will set A, B, C, but not D
	rPlaneHNF->set( getNormal( true ) );
	// d = -p0 * n
	rPlaneHNF->setH( dot3( -getCenterOfGravity(), (*rPlaneHNF) ) );
	return true;
}

//! Returns the hessian normal form of the primitive, when a position/centerofgravity and a normal is set.
//! May return not-a-number, in case of no normal or no position.
//! @param rPlaneHNF has to be of length 4.
//! @returns False in case of an error. True otherwise.
bool Primitive::getPlaneHNF( double* rPlaneHNF ) {
	if( rPlaneHNF == nullptr ) {
		return false;
	}
	// This will set A, B, C, but not D
	copyNormalXYZTo( rPlaneHNF, true );
	// d = -p0 * n
	rPlaneHNF[3] = dot3( -getCenterOfGravity(), Vector3D( rPlaneHNF ) );
	return true;
}

//! Copies the x-, y- and z-coordinates to a given array, which has to be at least of size 3.
//! Returns false in case of an error.
bool Primitive::copyXYZTo( float* rCoordXYZ ) {
	rCoordXYZ[0] = static_cast<float>(getX());
	rCoordXYZ[1] = static_cast<float>(getY());
	rCoordXYZ[2] = static_cast<float>(getZ());
	return true;
}

//! Copies the x-, y- and z-coordinates to a given array, which has to be at least of size 3.
//! Returns false in case of an error.s
bool Primitive::copyXYZTo( double* rCoordXYZ ) const {
	rCoordXYZ[0] = getX();
	rCoordXYZ[1] = getY();
	rCoordXYZ[2] = getZ();
	return true;
}

//! Copies the double precision x-, y- and z-elements of the normal vector to a given float array, which has to be at least of size 3.
bool Primitive::copyNormalXYZTo( float* rNormalXYZ, bool rNormalized ) {
	if( rNormalized ) {
		double normalLen = getNormalLen();
		rNormalXYZ[0] = static_cast<float>(getNormalX()/normalLen);
		rNormalXYZ[1] = static_cast<float>(getNormalY()/normalLen);
		rNormalXYZ[2] = static_cast<float>(getNormalZ()/normalLen);
		return true;
	}
	rNormalXYZ[0] = static_cast<float>(getNormalX());
	rNormalXYZ[1] = static_cast<float>(getNormalY());
	rNormalXYZ[2] = static_cast<float>(getNormalZ());
	return true;
}

//! Copies the x-, y- and z-elements of the normal vector to a given array, which has to be at least of size 3.
bool Primitive::copyNormalXYZTo( double* rNormalXYZ, bool rNormalized ) {
	if( rNormalized ) {
		double normalLen = getNormalLen();
		rNormalXYZ[0] = getNormalX()/normalLen;
		rNormalXYZ[1] = getNormalY()/normalLen;
		rNormalXYZ[2] = getNormalZ()/normalLen;
		return true;
	}
	rNormalXYZ[0] = getNormalX();
	rNormalXYZ[1] = getNormalY();
	rNormalXYZ[2] = getNormalZ();
	return true;
}

//! Copies the x-, y- and z-elements of the normal vector to a given array, which has to be at least of size 3.
//! Sets the normals to the given length.
bool Primitive::copyNormalXYZTo( double* rNormalXYZ, double rSetLength ) {
	double newLength = getNormalLen()/rSetLength;
	rNormalXYZ[0] = getNormalX()/newLength;
	rNormalXYZ[1] = getNormalY()/newLength;
	rNormalXYZ[2] = getNormalZ()/newLength;
	return true;
}

//! Returns the normal vector in spherical coordinates (phi,theta,radius).
//! Remark: |radius| == area of the Face.
double Primitive::getNormalSpherical( double* phi, double* theta, double* radius ) {
	*phi    = atan2( getNormalY(), getNormalX() );
	*theta  = atan2( getNormalZ(), sqrt( pow( getNormalX(), 2 ) + pow( getNormalY(), 2 ) ) );
	*radius = sqrt( pow( getNormalX(), 2 ) + pow( getNormalY(), 2 ) + pow( getNormalZ(), 2 ) );
	return (*radius);
}

// Colors --------------------------------------------------------------------

//! Copies the red, green and blue value as unsigned char to a given array, which has to be at least of size 3.
//! Returns false in case of an error.
bool Primitive::copyRGBTo([[maybe_unused]] unsigned char* rColorRGB ) {
	//cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not copy RGB to '" << rColorRGB << "' (" << getTypeStr() << ")!" << endl;
	return false;
}

//! Copies the red, green, blue and alpha value as unsigned char to a given array, which has to be at least of size 4.
//! Returns false in case of an error.
bool Primitive::copyRGBATo( unsigned char* rColorRGBA ) {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not copy RGBA to '" << rColorRGBA << "' (" << getTypeStr() << ")!" << endl;
	return false;
}

//! Return the type (=class) of this object as an id.
int Primitive::getType() {
	return Primitive::IS_UNDEFINED;
}

//! Return the type (=class) of this object as string.
//!
//! Typically used for debugging.
string Primitive::getTypeStr() {
	return getTypeStrFor( getType() );
}

//! Returns the type (=class) of an object type as string.
//!
//! Typically used for debugging.
string Primitive::getTypeStrFor( int primitiveType ) {
	string typeStr( "Primitive" );
	switch( primitiveType ) {
		case IS_UNDEFINED:
				typeStr = "Undefined";
			break;
		case IS_VERTEX:
				typeStr = "Vertex";
			break;
		case IS_FACE:
				typeStr = "Face";
			break;
		case IS_SPHERE:
				typeStr = "Sphere";
			break;
		case IS_RECTBOX:
				typeStr = "RectBox";
			break;
		case IS_POLYLINE:
				typeStr = "Polyline";
			break;
		case IS_MESH:
				typeStr = "Mesh";
			break;
		default:
			cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: of unknow type: " << primitiveType << endl;
	}
	return typeStr;
}

//! Return a standarized string for console output. e.g. "[Vertex] "
//!
//! Typically used for debugging on stdout and messages sent to stderr;
string Primitive::getCoutPref() {
	stringstream coutPrefStr;
	coutPrefStr << "[" << getTypeStr() << "] ";
	return coutPrefStr.str();
}

// normals ----------------------------------------------------------------------

//! Adds the normal to the given vector, which has to be of double[3].
//! Used as more efficent method for feature estimation - see:
//! Mesh::averageNormal.
void Primitive::addNormalTo( double* someVec ) {
	someVec[0] += getNormalX();
	someVec[1] += getNormalY();
	someVec[2] += getNormalZ();
}

//! Add the normal to a given array of size 3.
//!
//! @returns false in case of an error. True otherwise.
bool Primitive::addNormalXYZTo( double (&rNormalXYZ)[3], bool rNormalized ) {
	if( rNormalized ) {
		double normalLen = getNormalLen();
		rNormalXYZ[0] += getNormalX()/normalLen;
		rNormalXYZ[1] += getNormalY()/normalLen;
		rNormalXYZ[2] += getNormalZ()/normalLen;
		return( true );
	}
	rNormalXYZ[0] += getNormalX();
	rNormalXYZ[1] += getNormalY();
	rNormalXYZ[2] += getNormalZ();
	return( true );
}

//! Returns the length of the normal - e.g. for normalization.
//! Face normals have 2x the length of the area of the Face.
double Primitive::getNormalLen() {
	double normX = getNormalX();
	double normY = getNormalY();
	double normZ = getNormalZ();
	return sqrt( ( normX * normX ) + ( normY * normY ) + ( normZ * normZ ) );
}

//! Returns the angle between the normal and the given vector in radiant.
//! Might return not-a-number, when the normal is not set.
double Primitive::angleToNormal( Vector3D someVec ) {
	return angle( getNormal( false ), someVec );
}

// feature vector -------------------------------------------------------------

//! When overwritten: Assign a feature vectore stored in an external table.
//!
//! @returns false as no vector can be attached for the primitive (base) class.
bool Primitive::assignFeatureVec(const double*         rAttachFeatureVec,
                unsigned int rSetFeatureVecLen
) {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not attach feature vector [ " << rAttachFeatureVec[0] << ", ... ] of length " << rSetFeatureVecLen << "!" << endl;
	return( false );
}

//! When overwritten: Copies the feature vector to a given array, which has to be of proper length!
//! Returns false.
bool Primitive::copyFeatureVecTo( double* fetchFeatureVec ) const {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not copy feature vector to " << fetchFeatureVec << "!" << endl;
	return false;
}

//! When overwritten: Copies the feature vector to a given vector.
//! @returns false (in case of an error and when not applicable i.e. implemented).
bool Primitive::getFeatureVectorElements( vector<double>& rFeatVec ) const {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not copy feature vector to " << rFeatVec << "!" << endl;
	return false;
}

//! When overwritten: writes the wavelet decomposited feature vector to featureDecomp having length featureDecompLen.
//! Returns false.
bool Primitive::getFeatureVecWavletDecompTo( double** featureDecomp, int* featureDecompLen ) {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not decomposited feature vector at address " << (*featureDecomp) << " to length " << featureDecompLen << "!" << endl;
	return false;
}

//! Distance between this and the given feature vector.
//! Returns NaN (not a number) unless the method has been overwritten.
double Primitive::getFeatureDistTo( double* someFeatureVec ) {
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: Could not compute feature distance. Value at address " <<  someFeatureVec << "not changed!" << endl;
	return _NOT_A_NUMBER_;
}

//! Retrieves an elemment of the feature vector.
//! In case of an error the returned value is set to NaN.
//!
//! @returns false in case of an error e.g. elementNr > featureVecLen. True otherwise.
bool Primitive::getFeatureElement( unsigned int rElementNr, double* rElementValue ) {
	(*rElementValue) = _NOT_A_NUMBER_;
	cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: No feature element No. " <<  rElementNr << "!" << endl;
	return false;
}

//! Returns NaN or the length of the feature vector, when the method has been overwritten.
unsigned int Primitive::getFeatureVectorLen() {
	return _NOT_A_NUMBER_UINT_;
}

// simple geometric/vector functionality ---------------------------------------

//! Estimates the angle between two vectors in 2D.
//!
//! Returns the angle in radiant.
//!
//! See also: http://www.mathe-online.at/materialien/Andreas.Pester/files/Vectors/winkel_zwischen_vektoren.htm
double Primitive::estAngleBetweenVectors( double nx1, double ny1, double nx2, double ny2 ) {
	double numerator   = nx1*nx2 + ny1*ny2;
	double denominator = sqrt( pow( nx1, 2 ) + pow( ny1, 2 ) ) * sqrt( pow( nx2, 2 ) + pow( ny2, 2 ) ) ;
	double lambda      = acos( numerator / denominator);
	return lambda;
}

//! Estimates the angle between two vectors in 3D.
//!
//! Returns the angle in radiant.
//!
//! See also: http://www.mathe-online.at/materialien/Andreas.Pester/files/Vectors/winkel_zwischen_vektoren.htm
double Primitive::estAngleBetweenVectors( double nx1, double ny1, double nz1, double nx2, double ny2, double nz2 ) {

	double numerator   = nx1*nx2 + ny1*ny2 + nz1*nz2;
	double denominator = sqrt( pow( nx1, 2 ) + pow( ny1, 2 ) + pow( nz1, 2 ) ) * sqrt( pow( nx2, 2 ) + pow( ny2, 2 ) + pow( nz2, 2 ) ) ;
	double lambda      = acos( numerator / denominator );
	//cerr << getCoutPref() << "Lambda 3D: nx: " << nx1 << "/" << ny1 << "/" << nz1 << " => " << lambda << endl; 
	return lambda;
}

//! Estimates the angle between the normal vector and the z-axis.
//! This relates to - but are not - spherical coordinates.
//!
//! Returns the angle in radiant or FLT_MAX in case of an error.

double Primitive::estSphericalCoordsThetaNormal( Vector3D normalOther ) {
	return estSphericalCoordsThetaNormal( normalOther.getX(), normalOther.getY(), normalOther.getZ() );
}

//! Estimates the angle between the normal vector and the z-axis.
//! This relates to - but are not - spherical coordinates.
//!
//! Returns the angle in radiant or NOT_A_NUMBER in case of an error.
double Primitive::estSphericalCoordsThetaNormal( double normalOtherX, double normalOtherY, double normalOtherZ ) {
	return estAngleBetweenVectors( getNormalX(), getNormalY(), getNormalZ(), normalOtherX, normalOtherY, normalOtherZ );
}

//! Estimates the angle between the normal vector and the x-axis.
//! This relates to - but are not - spherical coordinates.
//!
//! Returns the angle in radiant or FLT_MAX in case of an error.
double Primitive::estSphericalCoordsPhiNormal() {
	return estAngleBetweenVectors( getNormalX(), getNormalY(), 1.0, 0.0 );
}

// Plane related - requires Point+Normal ------------------------------------------------------------------------------------------

//! Intersects a ray given by a position vector and a direction vector with the plane described by the Face.
//! Does not check if the ray hits within the face or outside the Face.
//! Returns false, when the ray does not intersect the faces plane (co-planar).
bool Primitive::getIntersectionFacePlaneLineDir( Vector3D* rRayPos, Vector3D* rRayDir, Vector3D* rRayIntersect ) {
	double denom = getNormalX() * ( rRayDir->getX() )
		     + getNormalY() * ( rRayDir->getY() )
		     + getNormalZ() * ( rRayDir->getZ() );
	if( denom == 0.0 ) {
		// parallel
		return false;
	}
	//double hnfD  = dot3( -vertA->getCenterOfGravity(), Vector3D( getNormalX(), getNormalY(), getNormalZ(), 0.0 ) );
	Vector3D somePointOnPlane = getCenterOfGravity();
	double hnfD  = dot3( -somePointOnPlane, Vector3D( getNormalX(), getNormalY(), getNormalZ(), 0.0 ) );
	double numer = -hnfD - getNormalX() * rRayPos->getX() - getNormalY() * rRayPos->getY() - getNormalZ() * rRayPos->getZ();
	if( numer == 0.0 ) {
		// line in plane
		return false;
	}
	double lambda = numer / denom;
	rRayIntersect->set( (*rRayPos) + ( (*rRayDir) ) * lambda );
	return true;
}

//! Intersects a ray given by two points with the plane described by the Face.
//! Does not check if the ray hits within the face or outside the Face.
//! Returns false, when the ray does not intersect the faces plane (co-planar).
bool Primitive::getIntersectionFacePlaneLinePos(
                const Vector3D& rayTop,
                const Vector3D& rayBot,
                Vector3D& rayIntersect
) {
	double denom = getNormalX() * ( rayTop.getX() - rayBot.getX() )
		     + getNormalY() * ( rayTop.getY() - rayBot.getY() )
		     + getNormalZ() * ( rayTop.getZ() - rayBot.getZ() );
	if( denom == 0.0 ) {
		// parallel
		return false;
	}
	//double hnfD  = dot3( -vertA->getCenterOfGravity(), Vector3D( getNormalX(), getNormalY(), getNormalZ(), 0.0 ) );
	Vector3D somePointOnPlane = getCenterOfGravity();
	double hnfD  = dot3( -somePointOnPlane, Vector3D( getNormalX(), getNormalY(), getNormalZ(), 0.0 ) );
	double numer = -hnfD - getNormalX() * rayBot.getX() - getNormalY() * rayBot.getY() - getNormalZ() * rayBot.getZ();
	if( numer == 0.0 ) {
		// line in plane
		return false;
	}
	double lambda = numer / denom;
	rayIntersect.set( rayBot + ( rayTop - rayBot ) * lambda );
	return true;
}

//! Same as getIntersectionFacePlaneLine, except:
//! the method will return false, when the point of intersection is not between rayTop and rayBot.
bool Primitive::getIntersectionFacePlaneEdge( Vector3D* rayTop, Vector3D* rayBot, Vector3D* rayIntersect ) {
	if( !getIntersectionFacePlaneLinePos( *rayTop, *rayBot, *rayIntersect ) ) {
		// co-planar - so there is no intersection at all.
		return false;
	}
	double distTop;
	double distBot;
	getDistanceToPoint( rayTop, &distTop );
	getDistanceToPoint( rayBot, &distBot );
	if( ( distTop * distBot ) > 0.0 ) {
		return false;
	}
	return true;
}

//! Returns the dist(ance) to a given a point to the plane defined by the center of gravity and the normal vector.
//! Returns false in case of an error.
bool Primitive::getDistanceToPoint( Vector3D* somePos, double* dist ) {
	Vector3D planeHNF;
	if( !getPlaneHNF( &planeHNF ) ) {
		return false;
	}
	double nom   = planeHNF.getX()*somePos->getX() + planeHNF.getY()*somePos->getY() + planeHNF.getZ()*somePos->getZ() + planeHNF.getH();
	double denom = sqrt( pow(planeHNF.getX(),2) + pow(planeHNF.getY(),2) + pow(planeHNF.getZ(),2) );
	(*dist) = nom/denom;
	return true;
}

// Center of gravity related -----------------------------------------------------------------------------------------------------------------------------------

//! Compute the distance between the center of gravity and a given position.
//! Utilizes Primitive::getX(), Primitive::getY() and Primitive::getZ() -- respectivly the overloaded methods.
bool Primitive::getDistanceFromCenterOfGravityTo( double const rSomePos[3], double* rDist ) const {
	double dx = rSomePos[0] - getX();
	double dy = rSomePos[1] - getY();
	double dz = rSomePos[2] - getZ();
	(*rDist) = sqrt( dx*dx + dy*dy + dz*dz );
	return true;
}

// Common usefull functions ------------------------------------------------------------------------------------------------------------------------------------

//! Compute the point with paraPoint on a line defined by vert1 and vert2 having weight1 and weight2.
//! Returns false in case of an error, e.g: equal weights or vertices having no distance.
bool Primitive::getPointOnWeightedLine( Vector3D* vertNew, Vector3D vert1, Vector3D vert2, double paramPoint, double weight1, double weight2 ) {
	return Primitive::getPointOnWeightedLine( vertNew, &vert1, &vert2, paramPoint, weight1, weight2 );
}

//! Compute the point with paraPoint on a line defined by vert1 and vert2 having weight1 and weight2.
//! Returns false in case of an error, e.g: equal weights or vertices having no distance.
bool Primitive::getPointOnWeightedLine( Vector3D* vertNew, Vector3D* vert1, Vector3D* vert2, double paramPoint, double weight1, double weight2 ) {
	if( weight1 == weight2 ) {
		cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: equal weights: " << weight1 << " and " << weight2 << "!" << endl;
		return false;
	}
	if( weight2 < weight1 ) {
		// Reverse, when weights are inverted:
		Vector3D* tmpVec    = vert1;
		double    tmpWeight = weight1;
		vert1   = vert2;
		weight1 = weight2;
		vert2   = tmpVec;
		weight2 = tmpWeight;
	}
	Vector3D dirVec = (*vert2) - (*vert1);
	if( dirVec.getLength3() == 0.0 ) {
		cerr << "[Primitive::" << __FUNCTION__ << "] ERROR: ZERO distance between vert1 and vert2!" << endl;
		return false;
	}
	dirVec *= ( paramPoint - weight1 ) / ( weight2 - weight1 );
	vertNew->set( vert1 + dirVec );
	return true;
}


//! Compute the point with rIsoVal on a line defined by rPrimA and rPrimB both having a function value assigned.
//! Returns false in case of an error, e.g: equal weights or vertices having no distance.
//! See also Primitive::getPointOnWeightedLine
//! For the position Primitive::getCenterOfGravity is used, which e.g. is equal to Vertex::getPositionVector.
bool Primitive::getPointOnIsoLine( Vector3D* rVertNew, Primitive* rPrimA, Primitive* rPrimB, double rIsoVal ) {
	double funcValA;
	double funcValB;
	if( !rPrimA->getFuncValue( &funcValA ) ) {
		return false;
	}
	if( !rPrimB->getFuncValue( &funcValB ) ) {
		return false;
	}
	Vector3D posA = rPrimA->getCenterOfGravity();
	Vector3D posB = rPrimB->getCenterOfGravity();
	return getPointOnWeightedLine( rVertNew, &posA, &posB,rIsoVal, funcValA, funcValB );
}

// help for debugging ----------------------------------------------------------

//! Dumps information (e.g. Index, Label, RGB) about the Primitve to stdout.
void Primitive::dumpInfo() {
	cout << "[" << getTypeStr() << "] === " << getIndex() << " === " << getIndexOriginal() << " =======================================" << endl;
	if( getFeatureVectorLen() <= 0 ) {
		cout << "| No feature vector assigned." << endl;
	} else {
		cout << "| Feature vector (len=" << getFeatureVectorLen() << "):";
		for( unsigned int i=0; i<getFeatureVectorLen(); i++ ) {
			double elementValue;
			getFeatureElement( i, &elementValue );
			cout << " " << elementValue;
		}
		cout << "." << endl;
	}
}

//! Writes a given graph in DOT language to .DOT file (for further processing).
//! A graphical representation of the graph can be estimated with a system call.
//!
//! Future enhancement: check if dot can also be integrated by a library.
//! See: http://graphviz.org/pdf/libguide.pdf
bool Primitive::writeDotFile( stringstream* dotGraph, string* fileSuffix, bool removeDot ) {
	stringstream  fileName;
	filebuf       fileBuffer;

	fileName << _DIRECTORY_DOT_ << getTypeStr() << "_" << getIndex() << (*fileSuffix) << ".dot";
	fileBuffer.open( fileName.str().c_str(), ios::out );
	ostream os(&fileBuffer);
	os << dotGraph->str();
	fileBuffer.close();

	// generate graph as GIF, when we can execute a command:
	if( !system( nullptr ) ) {
		cerr << "[" << getTypeStr() << "] ERROR: system processor not available." << endl;
		return false;
	}
	stringstream systemCommand;
	systemCommand << "dot -Tgif " << fileName.str() << " -o " << _DIRECTORY_DOT_ << getTypeStr() << "_" << getIndex() << (*fileSuffix) << ".gif";
	if( removeDot ) {
		systemCommand << " && rm -f " << fileName.str() << endl;
	}
	//cout << "[" << getTypeStr() << "] system call: " << systemCommand.str();
	if( !system( systemCommand.str().c_str() ) ) {
		return false;
	}
	return true;
}
