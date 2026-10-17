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

#include <limits>
#include <GigaMesh/mesh/matrix4d.h>

#define MATRIX4DINITDEFAULTS                    \
	x{ 0.0, 0.0, 0.0, 0.0 },                    \
	y{ 0.0, 0.0, 0.0, 0.0 },                    \
	z{ 0.0, 0.0, 0.0, 0.0 },                    \
	h{ 0.0, 0.0, 0.0, 0.0 }

using namespace std;

//! Constructs an empty matrix (all elements are zero).
Matrix4D::Matrix4D()
	: MATRIX4DINITDEFAULTS {
}

Matrix4D::Matrix4D( int mode, double val )
	: MATRIX4DINITDEFAULTS {
	//! Constructs a matrix for a certain transformation with one parameter.
	//! Scaled identity matrix.
	//! Rotation about the x-, y- or z-axis.
	//!
	//! Some Examples: 
	//! *) Rotation about an angle of 90° about the x-axis:
	//!    Matrix4D( _MATRIX4D_INIT_ROTATION_X_, M_PI/2 );
	//! *) Identity matrix scaled by lambda
	//!    Matrix4D( _MATRIX4D_INIT_SCALE_, lambda );
	//!    Not to be confused with (scaled) _MATRIX4D_INIT_IDENTITY_ (which differ only by the homogenous element).

//! \todo Change old constructor (this) to the new variant (below).
	switch( mode ) {
		// http://en.wikipedia.org/wiki/Identity_matrix
		case _MATRIX4D_INIT_IDENTITY_:
				x[0] = val; y[1] = val; z[2] = val; h[3] = val;
			break;
		case _MATRIX4D_INIT_SCALE_:
				x[0] = val; y[1] = val; z[2] = val; h[3] = 1.0;
			break;
		// http://www.fastgraph.com/makegames/3drotation/
		// -> Other Ways to Build a Rotation Matrix
		//
		// To change them for a right handed system, just remember the sine function is an odd function, so
		// sin(-val) = -sin(val)
		// Change the signs of all the sine terms to change the handedness.
		case _MATRIX4D_INIT_ROTATION_Y_:
				// tested.
				x[0] =  +cos( val ); x[2] = -sin( val );
				y[1] = 1.0;
				z[0] =  +sin( val ); z[2] = +cos( val );
				h[3] = 1.0;
			break;
		case _MATRIX4D_INIT_ROTATION_Z_:
				// tested.
				x[0] = +cos( val ); x[1] = +sin( val );
				y[0] = -sin( val ); y[1] = +cos( val );
				z[2] = 1.0;
				h[3] = 1.0;
			break;
		// Should not happen:
		default:
			cerr << "[Matrix4D] unknown initialization mode: " << mode << endl;
	} 
}

//! Constructs a matrix for a certain transformation
//! with the given parameters.
Matrix4D::Matrix4D( eCreateMode rMode, const vector<double>* rValues )
	: MATRIX4DINITDEFAULTS {

	// Check selected mode
	switch( rMode ) {
		case INIT_IDENTITY: { // http://en.wikipedia.org/wiki/Identity_matrix
				double diagVal = 1.0;
				if( ( rValues != nullptr ) && ( rValues->size() > 0 ) ) {
					diagVal = rValues->at( 0 );
				}
				x[0] = diagVal; y[1] = diagVal; z[2] = diagVal; h[3] = diagVal;
			} break;
		case INIT_SCALE: {
				double diagVal = 1.0;
				if( ( rValues != nullptr ) && ( rValues->size() > 0 ) ) {
					diagVal = rValues->at( 0 );
				}
				x[0] = diagVal; y[1] = diagVal; z[2] = diagVal; h[3] = 1.0;
			} break;
		case INIT_SKEW: {
				if( ( rValues != nullptr ) && ( rValues->size() >= 3 ) ) {
					x[0] = rValues->at( 0 );
					y[1] = rValues->at( 1 );
					z[2] = rValues->at( 2 );
					h[3] = 1.0;
				} else {
					cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: to few values for skewing. Three values required for " << rMode << "!" << endl;
				}
			} break;
		case INIT_TRANSLATE: {
				if( ( rValues != nullptr ) && ( rValues->size() >= 3 ) ) {
					x[0] = 1.0; y[1] = 1.0; z[2] = 1.0; h[3] = 1.0;
					h[0] = rValues->at( 0 );
					h[1] = rValues->at( 1 );
					h[2] = rValues->at( 2 );
				} else {
					cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: to few values for translation. Three values required for " << rMode << "!" << endl;
				}
			} break;
		// http://www.fastgraph.com/makegames/3drotation/
		// -> Other Ways to Build a Rotation Matrix
		//
		// To change them for a right handed system, just remember the sine function is an odd function, so
		// sin(-val) = -sin(val)
		// Change the signs of all the sine terms to change the handedness.
		case INIT_ROTATE_ABOUT_X: {
				if( ( rValues != nullptr ) && ( rValues->size() >= 1 ) ) {
					double val = rValues->at( 0 );
					x[0] = 1.0;
					y[1] = +cos( val ); y[2] =  +sin( val );
					z[1] = -sin( val ); z[2] =  +cos( val );
					h[3] = 1.0;
				} else {
					cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: to few values for translation. One value required for " << rMode << "!" << endl;
				}
			} break;
		case INIT_ROTATE_ABOUT_Y: {
				if( ( rValues != nullptr ) && ( rValues->size() >= 1 ) ) {
					double val = rValues->at( 0 );
					x[0] =  +cos( val ); x[2] = -sin( val );
					y[1] = 1.0;
					z[0] =  +sin( val ); z[2] = +cos( val );
					h[3] = 1.0;
				} else {
					cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: to few values for translation. One value required for " << rMode << "!" << endl;
				}
			} break;
		case INIT_ROTATE_ABOUT_Z: {
				if( ( rValues != nullptr ) && ( rValues->size() >= 1 ) ) {
					double val = rValues->at( 0 );
					x[0] = +cos( val ); x[1] = +sin( val );
					y[0] = -sin( val ); y[1] = +cos( val );
					z[2] = 1.0;
					h[3] = 1.0;
				} else {
					cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: to few values for translation. One value required for " << rMode << "!" << endl;
				}
			} break;
		//! \todo add base change to X
		case INIT_BASE_CHANGE_TO_Y: {
				if( ( rValues != nullptr ) && ( rValues->size() >= 6 ) ) {
					Vector3D originPos( rValues->at(0), rValues->at(1), rValues->at(2) );
					Vector3D orientDir( rValues->at(3), rValues->at(4), rValues->at(5) );
					if( !setBaseChange( SET_NEW_BASE_TO_Y, &originPos, &orientDir ) ) {
						cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: initalization failed using " << rValues << "!" << endl;
					}
				} else {
					cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: to few values for base change. Six values required for " << rMode << "!" << endl;
				}
			} break;
		case INIT_BASE_CHANGE_TO_Z: {
				if( ( rValues != nullptr ) && ( rValues->size() >= 6 ) ) {
					Vector3D originPos( rValues->at(0), rValues->at(1), rValues->at(2) );
					Vector3D orientDir( rValues->at(3), rValues->at(4), rValues->at(5) );
					if( !setBaseChange( SET_NEW_BASE_TO_Z, &originPos, &orientDir ) ) {
						cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: initalization failed using " << rValues << "!" << endl;
					}
				} else {
					cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: to few values for base change. Six values required for " << rMode << "!" << endl;
				}
			} break;
		// Should not happen:
		default:
			cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: Unknown initialization mode: " << rMode << endl;
	}
}

//! Constructs a matrix translating the given position into the origin.
Matrix4D::Matrix4D( const Vector3D& posVec )
	: MATRIX4DINITDEFAULTS {
	x[0] = 1.0; y[1] = 1.0; z[2] = 1.0; h[3] = 1.0;
	h[0] = -posVec.getX();
	h[1] = -posVec.getY();
	h[2] = -posVec.getZ();
}

//! Constructer and initalize with a rotation matrix about an arbitrary axis.
//!
//! see: http://inside.mines.edu/~gmurray/ArbitraryAxisRotation/#sec-programming-notes
Matrix4D::Matrix4D(const Vector3D& posVec, const Vector3D& dirVec, double angle )
	: MATRIX4DINITDEFAULTS {
	init( posVec, dirVec, angle );
}

//! Constructs a matrix using the elements of rArr4by4, which is supposed to have 16 elements.
Matrix4D::Matrix4D( const double* rArr4by4 )
	: MATRIX4DINITDEFAULTS {
	memcpy( x, &rArr4by4[0],  4*sizeof(double) );
	memcpy( y, &rArr4by4[4],  4*sizeof(double) );
	memcpy( z, &rArr4by4[8],  4*sizeof(double) );
	memcpy( h, &rArr4by4[12], 4*sizeof(double) );
}

//! Constructs a matrix using the elements of rArr4by4, which is supposed to have 16 elements.
Matrix4D::Matrix4D( const float* rArr4by4 )
	: MATRIX4DINITDEFAULTS {
    x[0] = static_cast<double>(rArr4by4[0]);
    x[1] = static_cast<double>(rArr4by4[1]);
	x[2] = static_cast<double>(rArr4by4[2]);
	x[3] = static_cast<double>(rArr4by4[3]);
	y[0] = static_cast<double>(rArr4by4[4]);
	y[1] = static_cast<double>(rArr4by4[5]);
	y[2] = static_cast<double>(rArr4by4[6]);
	y[3] = static_cast<double>(rArr4by4[7]);
	z[0] = static_cast<double>(rArr4by4[8]);
	z[1] = static_cast<double>(rArr4by4[9]);
	z[2] = static_cast<double>(rArr4by4[10]);
	z[3] = static_cast<double>(rArr4by4[11]);
	h[0] = static_cast<double>(rArr4by4[12]);
	h[1] = static_cast<double>(rArr4by4[13]);
	h[2] = static_cast<double>(rArr4by4[14]);
	h[3] = static_cast<double>(rArr4by4[15]);
}

//! Constructs a matrix using the elements of rVec4by4, which is supposed to have 16 elements.
//! Will show an error message, when rVec4by4 has not 16 elements.
Matrix4D::Matrix4D( vector<double> rVec4by4 )
	: MATRIX4DINITDEFAULTS {
	if( rVec4by4.size() != 16 ) {
		cerr << "[Matrix4D::" << __FUNCTION__ << "] expects a vector with 16 elements and not " << rVec4by4.size() << "!" << endl;
	}
	x[0] = rVec4by4[0];
	x[1] = rVec4by4[1];
	x[2] = rVec4by4[2];
	x[3] = rVec4by4[3];
	y[0] = rVec4by4[4];
	y[1] = rVec4by4[5];
	y[2] = rVec4by4[6];
	y[3] = rVec4by4[7];
	z[0] = rVec4by4[8];
	z[1] = rVec4by4[9];
	z[2] = rVec4by4[10];
	z[3] = rVec4by4[11];
	h[0] = rVec4by4[12];
	h[1] = rVec4by4[13];
	h[2] = rVec4by4[14];
	h[3] = rVec4by4[15];
}

// Initalization ---------------------------------------------------------------

//! Constructs an empty matrix (all elements are zero).
void Matrix4D::setEmpty() {
	x[0] = 0.0; x[1] = 0.0; x[2] = 0.0; x[3] = 0.0;
	y[0] = 0.0; y[1] = 0.0; y[2] = 0.0; y[3] = 0.0;
	z[0] = 0.0; z[1] = 0.0; z[2] = 0.0; z[3] = 0.0;
	h[0] = 0.0; h[1] = 0.0; h[2] = 0.0; h[3] = 0.0;
}

//! Constructs an idenity matrix (all elements are zero, except the diagonal is 1.0).
void Matrix4D::setIdentiy() {
	x[0] = 1.0; x[1] = 0.0; x[2] = 0.0; x[3] = 0.0;
	y[0] = 0.0; y[1] = 1.0; y[2] = 0.0; y[3] = 0.0;
	z[0] = 0.0; z[1] = 0.0; z[2] = 1.0; z[3] = 0.0;
	h[0] = 0.0; h[1] = 0.0; h[2] = 0.0; h[3] = 1.0;
}

//! Initalize with a rotation matrix about an arbitrary axis.
//!
//! see: http://inside.mines.edu/~gmurray/ArbitraryAxisRotation/#sec-programming-notes
void Matrix4D::init( Vector3D posVec, Vector3D dirVec, double angle ) {
	if( posVec.getH() != 1.0 ) {
		cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: requires a position vector for argument 1!" << endl;
		cerr << "[Matrix4D::" << __FUNCTION__ << "] ....... will continue risking wrong results!" << endl;
	}
	if( dirVec.getH() != 0.0 ) {
		cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: requires a direction vector for argument 2!" << endl;
		cerr << "[Matrix4D::" << __FUNCTION__ << "] ....... will continue risking wrong results!" << endl;
	}
	if( angle == 0.0 ) {
		// Do not show these errors as they are actually rather warnings, which are
		// typically caused by rotating the views.
		//
		// cerr << "[Matrix4D::" << __FUNCTION__ << "] Rotation angle of zero was given!" << endl;
		// cerr << "[Matrix4D::" << __FUNCTION__ << "] is initalized using a identity matrix!" << endl;
		//
		// The given rotation angle is zero - nothing to do!
		// So we return an identity matrix.
		x[0] = 1.0; x[1] = 0.0; x[2] = 0.0; x[3] = 0.0;
		y[0] = 0.0; y[1] = 1.0; y[2] = 0.0; y[3] = 0.0;
		z[0] = 0.0; z[1] = 0.0; z[2] = 1.0; z[3] = 0.0;
		h[0] = 0.0; h[1] = 0.0; h[2] = 0.0; h[3] = 1.0;
		return;
	}

	double a = posVec.getX();
	double b = posVec.getY();
	double c = posVec.getZ();

	double u = dirVec.getX();
	double v = dirVec.getY();
	double w = dirVec.getZ();

	// Set some intermediate values.
	double u2 = u*u;
	double v2 = v*v;
	double w2 = w*w;
	double cosT = cos(angle);
	double sinT = sin(angle);
	double l2 = u2 + v2 + w2;
	double l =  sqrt(l2);

	// l2 == 0 means that there the given direction vector has no length and therefore is invalid.
	if( l2 == 0 ) {
		cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: the given direction vector is of length ZERO!" << endl;
		cerr << "[Matrix4D::" << __FUNCTION__ << "] is initalized using a identity matrix!" << endl;
		x[0] = 1.0; x[1] = 0.0; x[2] = 0.0; x[3] = 0.0;
		y[0] = 0.0; y[1] = 1.0; y[2] = 0.0; y[3] = 0.0; 
		z[0] = 0.0; z[1] = 0.0; z[2] = 1.0; z[3] = 0.0;
		h[0] = 0.0; h[1] = 0.0; h[2] = 0.0; h[3] = 1.0;
		return;
	}

	// Build the matrix entries element by element. 
	double m11 = (u2 + (v2 + w2) * cosT)/l2;
	double m12 = (u*v * (1 - cosT) - w*l*sinT)/l2;
	double m13 = (u*w * (1 - cosT) + v*l*sinT)/l2;
	double m14 = (a*(v2 + w2) - u*(b*v + c*w)
	              + (u*(b*v + c*w) - a*(v2 + w2))*cosT + (b*w - c*v)*l*sinT)/l2;

	double m21 = (u*v * (1 - cosT) + w*l*sinT)/l2;
	double m22 = (v2 + (u2 + w2) * cosT)/l2;
	double m23 = (v*w * (1 - cosT) - u*l*sinT)/l2;
	double m24 = (b*(u2 + w2) - v*(a*u + c*w)
	              + (v*(a*u + c*w) - b*(u2 + w2))*cosT + (c*u - a*w)*l*sinT)/l2;

	double m31 = (u*w * (1 - cosT) - v*l*sinT)/l2;
	double m32 = (v*w * (1 - cosT) + u*l*sinT)/l2;
	double m33 = (w2 + (u2 + v2) * cosT)/l2;
	double m34 = (c*(u2 + v2) - w*(a*u + b*v)
	              + (w*(a*u + b*v) - c*(u2 + v2))*cosT + (a*v - b*u)*l*sinT)/l2;

	//! \todo The following transformation matrix conforms with the others (about the axis), but the one used is correect.
	// x[0] = m11; x[1] = m12; x[2] = m13; x[3] = m14;
	// y[0] = m21; y[1] = m22; y[2] = m32; y[3] = m24;
	// z[0] = m31; z[1] = m23; z[2] = m33; z[3] = m34;
	// h[0] = 0.0; h[1] = 0.0; h[2] = 0.0; h[3] = 1.0;

	x[0] = m11; x[1] = m21; x[2] = m31; x[3] = 0.0;
	y[0] = m12; y[1] = m22; y[2] = m32; y[3] = 0.0;
	z[0] = m13; z[1] = m23; z[2] = m33; z[3] = 0.0;
	h[0] = m14; h[1] = m24; h[2] = m34; h[3] = 1.0;

//	x[0] = m11; x[1] = m12; x[2] = m13; x[3] = m14;
//	y[0] = m21; y[1] = m22; y[2] = m23; y[3] = m24;
//	z[0] = m31; z[1] = m32; z[2] = m33; z[3] = m34;
//	h[0] = 0.0; h[1] = 0.0; h[2] = 0.0; h[3] = 1.0;

//	x[0] =  m11; x[1] =  m12; x[2] =  m13; x[3] = 0.0;
//	y[0] =  m21; y[1] =  m22; y[2] =  m23; y[3] = 0.0;
//	z[0] =  m31; z[1] =  m32; z[2] =  m33; z[3] = 0.0;
//	h[0] = -m14; h[1] = -m24; h[2] = -m34; h[3] = 1.0;
}

// Access ----------------------------------------------------------------------

bool Matrix4D::set( int idxRow, int idxCol, double val ) {
	//! Returns the element of position (row,col) or Not-A-Number in case of an
	//! error.
	switch( idxRow ) {
		case 0:
			x[idxCol] = val;
			return true;
			break;
		case 1:
			y[idxCol] = val;
			return true;
			break;
		case 2:
			z[idxCol] = val;
			return true;
			break;
		case 3:
			h[idxCol] = val;
			return true;
			break;
	}
	cerr << "[Matrix4D::" << __FUNCTION__ << "] Wrong row index: " << idxRow << " - Has to be within [0...1]!" << endl;
	return false; 
}

bool Matrix4D::set( Matrix4D rSomeMat ) {
	//! Copy matrix.
	double arr4by4[16];
	rSomeMat.getArray( arr4by4 );
	memcpy( x, &arr4by4[0],  4*sizeof(double) );
	memcpy( y, &arr4by4[4],  4*sizeof(double) );
	memcpy( z, &arr4by4[8],  4*sizeof(double) );
	memcpy( h, &arr4by4[12], 4*sizeof(double) );
	return true;
}

//! Setup a transformation matrix for a base change.
//! Expects a new origin and a directional vector, which defines the new z-axis.
//!
//! @returns false in case of an error. True otherwise.
bool Matrix4D::setBaseChange( eNewBase rBaseOrient, const Vector3D* rOrigin, const Vector3D* rOrientation ) {
	// Sanity check
	if( ( rOrigin == nullptr ) || ( rOrientation == nullptr ) ) {
		cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return( false );
	}

	Vector3D zAxis;
	switch( rBaseOrient ) {
		case SET_NEW_BASE_TO_X:
			zAxis.set( 1.0, 0.0, 0.0, 0.0 );
			break;
		case SET_NEW_BASE_TO_Y:
			zAxis.set( 0.0, 1.0, 0.0, 0.0 );
			break;
		case SET_NEW_BASE_TO_Z:
			zAxis.set( 0.0, 0.0, 1.0, 0.0 );
			break;
		default:
			cerr << "[Matrix4D::" << __FUNCTION__ << "] ERROR: Invalid base id given!" << endl;
			return( false );
	}

	Vector3D rotAxis  = (*rOrientation) % zAxis;
	double   rotAngle = angle( (*rOrientation), zAxis);

	vector<double> translateToOrigin = { -rOrigin->getX(), -rOrigin->getY(), -rOrigin->getZ() };

	Matrix4D rotMat( Matrix4D::INIT_IDENTITY );
	rotMat.init( Vector3D( 0.0, 0.0, 0.0, 1.0 ), rotAxis, rotAngle );
	(*this)  = Matrix4D( INIT_TRANSLATE, &translateToOrigin );
	(*this) *= rotMat;

	return( true );
}

double Matrix4D::get( int idxRow, int idxCol ) const {
	//! Returns the element of position (row,col) or Not-A-Number in case of an
	//! error.
	switch( idxRow ) {
		case 0:
			return x[idxCol];
			break;
		case 1:
			return y[idxCol];
			break;
		case 2:
			return z[idxCol];
			break;
		case 3:
			return h[idxCol];
			break;
		default:
			cerr << "[Matrix4D::" << __FUNCTION__ << "] Wrong row index: " << idxRow << " - Has to be within [0...3]!" << endl;
			return std::numeric_limits<double>::quiet_NaN();
	}
}

double& Matrix4D::get( int idxRow, int idxCol ) {
	//! Returns the element of position (row,col) or Not-A-Number in case of an
	//! error.
	switch( idxRow ) {
		case 0:
			return getX( idxCol );
			break;
		case 1:
			return getY( idxCol );
			break;
		case 2:
			return getZ( idxCol );
			break;
		case 3:
			return getH( idxCol );
			break;
		default:
			cerr << "[Matrix4D::" << __FUNCTION__ << "] Wrong row index: " << idxRow << " - Has to be within [0...3]!" << endl;

			return(x[0] = std::numeric_limits<double>::quiet_NaN()); // this is evil -- but the matrix is broken
			                                                         // anyway if the user requests an index that
			                                                         // is out of range!
	}
}

double Matrix4D::getX(int idx) const
{
	//! Get const value with index idx form the 1st row
	return x[idx];
}

// Eventually add additional checks to ensure that idx is within [0...3]
// May influence performance.

double& Matrix4D::getX( int idx ) {
	//! Get value with index idx from the 1st row.
	return x[idx];
}

double Matrix4D::getY(int idx) const
{
	//! Get const value with index idx from the 2nd row
	return y[idx];
}

double& Matrix4D::getY( int idx ) {
	//! Get value with index idx from the 2nd row.
	return y[idx];
}

double Matrix4D::getZ(int idx) const
{
	//! Get const value with index idx from the 3rd row.
	return z[idx];
}

double& Matrix4D::getZ( int idx ) {
	//! Get value with index idx from the 3rd row.
	return z[idx];
}

double Matrix4D::getH(int idx) const
{
	//! Get const value with index idx from the 4th row.
	return h[idx];
}

double& Matrix4D::getH( int idx ) {
	//! Get value with index idx from the 4th row.
	return h[idx];
}

double Matrix4D::getSumX() {
	//! Returns the sum of the 1st row.
	return  x[0] + x[1] + x[2] + x[3];
}

double Matrix4D::getSumY() {
	//! Returns the sum of the 2nd row.
return  y[0] + y[1] + y[2] + y[3];
}

double Matrix4D::getSumZ() {
	//! Returns the sum of the 3d row.
	return  z[0] + z[1] + z[2] + z[3];
}

double Matrix4D::getSumH() {
	//! Returns the sum of the 4th row.
	return  h[0] + h[1] + h[2] + h[3];
}

void Matrix4D::getArray( double* array4by4 ) {
	//! Copies all elements to a given 4x4 array of double[4][4].
	array4by4[0]  = x[0];
	array4by4[1]  = x[1];
	array4by4[2]  = x[2];
	array4by4[3]  = x[3];

	array4by4[4]  = y[0];
	array4by4[5]  = y[1];
	array4by4[6]  = y[2];
	array4by4[7]  = y[3];

	array4by4[8]  = z[0];
	array4by4[9]  = z[1];
	array4by4[10] = z[2];
	array4by4[11] = z[3];

	array4by4[12] = h[0];
	array4by4[13] = h[1];
	array4by4[14] = h[2];
	array4by4[15] = h[3];
}

//! Calculates determinant and returns its value.
double Matrix4D::getDeterminant() {

	// Without loss of generality: Use expansion of first row
	// of matrix

	mDet = get(0, 0)*getMinorDeterminant(0, 0) -
	       get(0, 1)*getMinorDeterminant(0, 1) +
	       get(0, 2)*getMinorDeterminant(0, 2) -
	       get(0, 3)*getMinorDeterminant(0, 3);

    return(mDet);
}

//! Calculates determinant of minor, i.e. of a matrix where a line and a column
//! have been removed.
double Matrix4D::getMinorDeterminant(int row, int col) {

	// Temporarily store matrix elements; this makes the formula for calculating the
	// determinant easier.

	double a, b, c, d, e, f, g, h, i;
	int val = 0; // value to be set

	for(int iRow = 0; iRow < 4; iRow++) {
		for(int iCol = 0; iCol < 4; iCol++) {
			if(iRow == row || iCol == col) {
				continue;
			}

			// This is a bit ugly: I want to assign the values of the minor (which is a 3x3 matrix)
			// to the variables a,b,...,i.
			double* valPtr;
			if(val == 0)
				valPtr = &a;
			else if(val == 1)
				valPtr = &b;
			else if(val == 2)
				valPtr = &c;
			else if(val == 3)
				valPtr = &d;
			else if(val == 4)
				valPtr = &e;
			else if(val == 5)
				valPtr = &f;
			else if(val == 6)
				valPtr = &g;
			else if(val ==  7)
				valPtr = &h;
			else
				valPtr = &i;

			*valPtr = this->get(iRow, iCol);
			val++;
		}
	}

	// Rule of Sarrus...
	return(a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g);
}

// Operators -------------------------------------------------------------------

void Matrix4D::operator *= (Matrix4D multMat) {
	//! Matrix multiplication.
	//!
	//! see also: http://en.wikipedia.org/wiki/Matrix_multiplication

	double newMat[4][4];

	for( int idxRow = 0; idxRow<4; idxRow++ ) {
		for( int idxCol = 0; idxCol<4; idxCol++ ) {
			newMat[idxRow][idxCol] = get( idxRow, 0 ) * multMat.get( 0, idxCol ) + get( idxRow, 1 ) * multMat.get( 1, idxCol ) + get( idxRow, 2 ) * multMat.get( 2, idxCol ) + get( idxRow, 3 ) * multMat.get( 3, idxCol );
		}
	}
	for( int idxRow = 0; idxRow<4; idxRow++ ) {
		x[idxRow] = newMat[0][idxRow];
		y[idxRow] = newMat[1][idxRow];
		z[idxRow] = newMat[2][idxRow];
		h[idxRow] = newMat[3][idxRow];
	}
}

void Matrix4D::operator *=(double scalar)
{
	for(int x = 0; x<4; ++x)
	{
		for(int y = 0; y < 4; ++y)
		{
			this->get(x,y) *= scalar;
		}
	}
}

void Matrix4D::operator += (Matrix4D addMat) {
	//! Matrix addition.
	//!
	//! see also: http://en.wikipedia.org/wiki/Matrix_addition

	for( int idxCol = 0; idxCol<4; idxCol++ ) {
		x[idxCol] += addMat.getX( idxCol );
		y[idxCol] += addMat.getY( idxCol );
		z[idxCol] += addMat.getZ( idxCol );
		h[idxCol] += addMat.getH( idxCol );
	}
}

//! Allows access to row `row` and column `col` of the matrix. Returns
//! a reference so that the value may be changed.
double& Matrix4D::operator()(int row, int col) {
	return(this->get(row, col));
}

//! Multiplies this Matrix4D to a given array of homogenous vectors.
//! Therefore vectorArr has to be of size arrSize x 4
void Matrix4D::applyTo(
                double* rVectorArr,
                unsigned long rArrSize
) {
	// see: http://www.mathworks.com/access/helpdesk/help/techdoc/ref/mtimes.html
	// for each vector
	//dumpInfo( true, "trans" );
#ifdef OLD_APPLTO
	double* resultArr = (double*) calloc( arrSize*4, sizeof(double) );
	double  matB[4][4];
	getArray( (double*)&matB );
	for( int i=0; i<arrSize; i++ ) {
		// A is an m-by-p matrix => m=1, p=4
		// B is an p-by-n matrix => p=4, n=4
		// => C is m-by-n => 1x4
		// the loop i = [0...m] can be left out as m=1 so we can simplify:
		for( int j=0; j<4; j++ ) {
			for( int k=0; k<3; k++ ) {
				resultArr[i*4+j] += vectorArr[i*4+k] * matB[k][j];
			}
			//cout << vectorArr[i*4+3] << endl;
		}
	}
	memcpy( vectorArr, resultArr, arrSize*4*sizeof(double) );
	free( resultArr );
#else
	for( unsigned long i=0; i<rArrSize; i++ ) {
		// Loop unrolling is faster:
		double vecX = rVectorArr[i*4];
		double vecY = rVectorArr[i*4+1];
		double vecZ = rVectorArr[i*4+2];
		double vecH = rVectorArr[i*4+3];
		//cout << vectorArr[i*4] << " " << vectorArr[i*4+1] << " " << vectorArr[i*4+2] << " " << vectorArr[i*4+3] << endl;
		rVectorArr[i*4]   = vecX * x[0] + vecY * y[0] + vecZ * z[0] + vecH * h[0];
		rVectorArr[i*4+1] = vecX * x[1] + vecY * y[1] + vecZ * z[1] + vecH * h[1];
		rVectorArr[i*4+2] = vecX * x[2] + vecY * y[2] + vecZ * z[2] + vecH * h[2];
		rVectorArr[i*4+3] = vecX * x[3] + vecY * y[3] + vecZ * z[3] + vecH * h[3];
		//cout << vectorArr[i*4] << " " << vectorArr[i*4+1] << " " << vectorArr[i*4+2] << " " << vectorArr[i*4+3] << endl;

	}
#endif
}

//! Invert a matrix given as float array with 16 entries.
//! Implemented according to Intel ftp://download.intel.com/design/PentiumIII/sml/24504301.pdf
//! with Cramer's rule but without Streaming SIMD extension)
//! ATTENTION: works diffrent in contrast to invert()
//! Reason: internal i.e transposed storage of the matrix.
//! @returns false in case of an error. True otherwise.
bool Matrix4D::invert() {
	double tmp[12]; // temp array for pairs
	double mat[16]; // array of the source/input matrix as an array
	double src[16]; // array of the transposed source matrix as an array
	double dst[16]; // array of the resulting matrix as an array

	// prepare matrix as array
	for ( int i=0; i<4; i++ ) {
		mat[i]    = x[i];
		mat[i+4]  = y[i];
		mat[i+8]  = z[i];
		mat[i+12] = h[i];
	}

	// copy and dump:
	//cout << "[Matrix4D::" << __FUNCTION__ << "] ";
	for( int j=0; j<16; j++ ) {
		//cout << mat[j] << " ";
		src[j] = mat[j];
	}
	//cout << endl;

	// transpose matrix - NOT required as Matrix4D appears to be internally transposed.
	// THIS is different to the invert operation below!!!
	//for ( int i=0; i<4; i++ ) {
	//	src[i]    = mat[i*4];
	//	src[i+4]  = mat[i*4+1];
	//	src[i+8]  = mat[i*4+2];
	//	src[i+12] = mat[i*4+3];
	//}

	// calculate pairs for first 8 elements (cofactors)
	tmp[0]  = src[10] * src[15];
	tmp[1]  = src[11] * src[14];
	tmp[2]  = src[9]  * src[15];
	tmp[3]  = src[11] * src[13];
	tmp[4]  = src[9]  * src[14];
	tmp[5]  = src[10] * src[13];
	tmp[6]  = src[8]  * src[15];
	tmp[7]  = src[11] * src[12];
	tmp[8]  = src[8]  * src[14];
	tmp[9]  = src[10] * src[12];
	tmp[10] = src[8]  * src[13];
	tmp[11] = src[9]  * src[12];

	// calculate first 8 elements (cofactors)
	dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];

	// calculate pairs for second 8 elements (cofactors)
	tmp[0] = src[2]*src[7];
	tmp[1] = src[3]*src[6];
	tmp[2] = src[1]*src[7];
	tmp[3] = src[3]*src[5];
	tmp[4] = src[1]*src[6];
	tmp[5] = src[2]*src[5];
	tmp[6] = src[0]*src[7];
	tmp[7] = src[3]*src[4];
	tmp[8] = src[0]*src[6];
	tmp[9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];

	// calculate second 8 elements (cofactors)
	dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];

	// Compute the determinant
	double det; // determinant
	det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

	// Compute the matrix inverse
	det = 1/det;
	for( double& element : dst ) {
		element *= det;
	}

	// Write back the result to the private member values.
	for ( int i=0; i<4; i++ ) {
		x[i] = dst[i*4];
		y[i] = dst[i*4+1];
		z[i] = dst[i*4+2];
		h[i] = dst[i*4+3];
	}

	return true;
}

// Information and debuging --------------------------------------------------------------------

//! Convert matrix to string with line break e.g. for saving as text file.
bool Matrix4D::getTextMatrix( string* rMatrixStr ) {
	if( rMatrixStr == nullptr ) {
		return false;
	}
	string matStr  = to_string( x[0] ) + " " + to_string( y[0] ) + " " + to_string( z[0] ) + " " + to_string( h[0] ) + "\n";
	       matStr += to_string( x[1] ) + " " + to_string( y[1] ) + " " + to_string( z[1] ) + " " + to_string( h[1] ) + "\n";
		   matStr += to_string( x[2] ) + " " + to_string( y[2] ) + " " + to_string( z[2] ) + " " + to_string( h[2] ) + "\n";
		   matStr += to_string( x[3] ) + " " + to_string( y[3] ) + " " + to_string( z[3] ) + " " + to_string( h[3] ) + "\n";
	(*rMatrixStr) = matStr;
	return true;
}

//! Prints the matrix elements to stdout.
//! Optional: Format output for use with Matlab.
void Matrix4D::dumpInfo( bool forMatlab, const string& someNameTag ) {
	string someWhiteSpace;
	someWhiteSpace.resize( someNameTag.length(), ' ' );

	if( forMatlab ) {
		cout << setprecision( 4 ) << fixed;
		cout << someNameTag    << " = [ " << x[0] << ", " << y[0] << ", " << z[0] << ", " << h[0] << ";  ";
		cout << someWhiteSpace << "     " << x[1] << ", " << y[1] << ", " << z[1] << ", " << h[1] << ";  ";
		cout << someWhiteSpace << "     " << x[2] << ", " << y[2] << ", " << z[2] << ", " << h[2] << ";  ";
		cout << someWhiteSpace << "     " << x[3] << ", " << y[3] << ", " << z[3] << ", " << h[3] << " ]; " << endl;
		return;
	}

	cout << "[Matrix4D] " << someNameTag    << " | " << x[0] << ", " << y[0] << ", " << z[0] << ", " << h[0] << endl;
	cout << "[Matrix4D] " << someWhiteSpace << " | " << x[1] << ", " << y[1] << ", " << z[1] << ", " << h[1] << endl;
	cout << "[Matrix4D] " << someWhiteSpace << " | " << x[2] << ", " << y[2] << ", " << z[2] << ", " << h[2] << endl;
	cout << "[Matrix4D] " << someWhiteSpace << " | " << x[3] << ", " << y[3] << ", " << z[3] << ", " << h[3] << endl;
}

//! Invert a matrix given as float array with 16 entries -- according to Intel ftp://download.intel.com/design/PentiumIII/sml/24504301.pdf
//! with Cramer's rule but without Streaming SIMD extension)
//! ATTENTION: works diffrent in contrast to Matrix4D::invert()
//! Reason: expects a non-transposed storage of the matrix.
void invert( const float *mat, float *dst ) {
	float tmp[12]; // temp array for pairs
	float src[16]; // array of transpose source matrix
	float det;     // determinant

	// dump:
	//cout << "[" << __FUNCTION__ << "] Matrix as array: ";
	//for( int j=0; j<16; j++ ) {
	//	cout << mat[j] << " ";
	//}
	//cout << endl;

	// transpose matrix
	for ( int i=0; i<4; i++ ) {
		src[i]    = mat[i*4];
		src[i+4]  = mat[i*4+1];
		src[i+8]  = mat[i*4+2];
		src[i+12] = mat[i*4+3];
	}

	// calculate pairs for first 8 elements (cofactors)
	tmp[0]  = src[10] * src[15];
	tmp[1]  = src[11] * src[14];
	tmp[2]  = src[9]  * src[15];
	tmp[3]  = src[11] * src[13];
	tmp[4]  = src[9]  * src[14];
	tmp[5]  = src[10] * src[13];
	tmp[6]  = src[8]  * src[15];
	tmp[7]  = src[11] * src[12];
	tmp[8]  = src[8]  * src[14];
	tmp[9]  = src[10] * src[12];
	tmp[10] = src[8]  * src[13];
	tmp[11] = src[9]  * src[12];

	// calculate first 8 elements (cofactors)
	dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];

	// calculate pairs for second 8 elements (cofactors)
	tmp[0] = src[2]*src[7];
	tmp[1] = src[3]*src[6];
	tmp[2] = src[1]*src[7];
	tmp[3] = src[3]*src[5];
	tmp[4] = src[1]*src[6];
	tmp[5] = src[2]*src[5];
	tmp[6] = src[0]*src[7];
	tmp[7] = src[3]*src[4];
	tmp[8] = src[0]*src[6];
	tmp[9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];

	// calculate second 8 elements (cofactors)
	dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];

	// calculate determinant
	det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

	// calculate matrix inverse
	det = 1/det;
	for( int j=0; j<16; j++ ) {
		dst[j] *= det;
	}
}

//! Writes data of current matrix to the output stream. The coordinates are
//! separated by spaces.
std::ostream& operator<<(std::ostream& o, const Matrix4D& m) {
	return(o << std::fixed << std::setprecision(8)
	         << m.get( 0, 0 ) << " " << m.get( 1, 0 ) << " " << m.get( 2, 0 ) << " " << m.get( 3, 0 ) << " "
	         << m.get( 0, 1 ) << " " << m.get( 1, 1 ) << " " << m.get( 2, 1 ) << " " << m.get( 3, 1 ) << " "
	         << m.get( 0, 2 ) << " " << m.get( 1, 2 ) << " " << m.get( 2, 2 ) << " " << m.get( 3, 2 ) << " "
	         << m.get( 0, 3 ) << " " << m.get( 1, 3 ) << " " << m.get( 2, 3 ) << " " << m.get( 3, 3 )
	      );
}

Matrix4D operator*(const Matrix4D& matA, const Matrix4D& matB)
{
	Matrix4D mat(matA);
	mat *= matB;

	return mat;
}

Matrix4D operator*(double scalar, const Matrix4D& mat)
{
	Matrix4D m(mat);
	m *= scalar;

	return m;
}

Matrix4D operator*(const Matrix4D& mat, double scalar)
{
	return scalar * mat;
}

Matrix4D operator+(const Matrix4D& matA, const Matrix4D& matB)
{
	Matrix4D mat(matA);
	mat += matB;

	return mat;
}
