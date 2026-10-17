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

#ifndef MATRIX4D_H
#define MATRIX4D_H

#define _MATRIX4D_INIT_IDENTITY_     1

#define _MATRIX4D_INIT_SCALE_        5
#define _MATRIX4D_INIT_ROTATION_Y_  12
#define _MATRIX4D_INIT_ROTATION_Z_  13

#include <iostream>
#include <vector>

#include <cstdlib> // uint, calloc, free
#include <cstring> // memcpy, strncmp
#include <iomanip>  // for setiosflags( ios_base::fixed ) - see: http://www.cplusplus.com/reference/iostream/ios_base/fmtflags/
#include <cmath>

#include "vector3d.h"

class Vector3D;

//!
//! \brief 4x4 Matrix for manipulation of Vector3D.
//!
//! Homogenos matrix class for manipulation Vector3D, which is already extended
//! by 4th coordinate - see Vector3D::getH(). 
//!
//! see: http://en.wikipedia.org/wiki/Transformation_matrix#Other_kinds_of_transformations
//! http://en.wikipedia.org/wiki/Transformation_matrix
//! rotation in 3D: http://en.wikipedia.org/wiki/Rotation_matrix#Dimension_three
//! also nice: http://www.cs.trinity.edu/~jhowland/cs3353/intro/intro/
//! 
//! Remark: maybe using a double[4][4] will be better in the future instead of
//! x[], y[], z[] and h[] for internal storage and handling.
//!
//! By default the matrix elements will be set to 0.0 at construction time.
//! However, there are several constructers allowing the initalizing of different
//! types of (e.g. transformation) matrices.
//!

class Matrix4D {
	public:	
		// Types used during construction
		enum eCreateMode {
			INIT_IDENTITY,           //!< Initalization as identity matrix.
			INIT_SCALE,              //!< Initalization using one value for uniform scaling in x-, y- and z-direction.
			INIT_SKEW,               //!< Initalization using three values for skew along the x-, y- and z-axis.
			INIT_TRANSLATE,          //!< Initalization using three values for translation in x-, y- and z-direction.
			INIT_ROTATE_ABOUT_X,     //!< Initalization using one values - angle in radiant - for rotation about the x-axis.
			INIT_ROTATE_ABOUT_Y,     //!< Initalization using one values - angle in radiant - for rotation about the y-axis.
			INIT_ROTATE_ABOUT_Z,     //!< Initalization using one values - angle in radiant - for rotation about the z-axis.
			INIT_BASE_CHANGE_TO_Y,   //!< Initalization using three values for translation in x-, y- and z-direction and three values for the direction becoming the new y-axis.
			INIT_BASE_CHANGE_TO_Z,   //!< Initalization using three values for translation in x-, y- and z-direction and three values for the direction becoming the new z-axis.
		};

		enum eNewBase {
			SET_NEW_BASE_TO_X, //!< see Matrix4D::setBaseChangeZ
			SET_NEW_BASE_TO_Y, //!< see Matrix4D::setBaseChangeZ
			SET_NEW_BASE_TO_Z, //!< see Matrix4D::setBaseChangeZ
		};

		// Constructors
		Matrix4D();
		Matrix4D( int mode, double val=1.0 );
		Matrix4D( eCreateMode rMode, const std::vector<double>* rValues=nullptr );
		Matrix4D( const Vector3D& posVec );
		Matrix4D( const Vector3D& posVec, const Vector3D& dirVec, double angle );
		Matrix4D( const double* rArr4by4 );
		Matrix4D( const float*  rArr4by4 );
		Matrix4D( std::vector<double> rVec4by4 );

		// Initalization
		void setEmpty();
		void setIdentiy();
		void init( Vector3D posVec, Vector3D dirVec, double angle );

		// Access -- Set Matrix
		bool    set( int idxRow, int idxCol, double val );
		bool    set( Matrix4D rSomeMat );
		bool    setBaseChange( eNewBase rBaseOrient, const Vector3D* rOrigin, const Vector3D* rOrientation );

		// Access -- Get values
		double  get( int idxRow, int idxCol ) const;
		double& get( int idxRow, int idxCol );
		double  getX( int idx) const;
		double& getX( int idx );
		double  getY( int idx) const;
		double& getY( int idx );
		double  getZ(int idx) const;
		double& getZ( int idx );
		double  getH(int idx) const;
		double& getH( int idx );
		double  getSumX();
		double  getSumY();
        double  getSumZ();
		double  getSumH();
		void    getArray( double* array4by4 );

		// Calculation
		double getDeterminant();

		// Operators
		void operator *= (Matrix4D multMat);
		void operator *= (double scalar);
		void operator += (Matrix4D multMat);
		double& operator()(int row, int col);

		// Operations:
		void applyTo( double* rVectorArr, unsigned long rArrSize );
		bool invert();

		// Information and debuging
		bool getTextMatrix( std::string* rMatrixStr );
		void dumpInfo(bool forMatlab=false, const std::string& someNameTag="" );

	private:
		double x[4]; //!< 1st column elements
		double y[4]; //!< 2nd column elements
		double z[4]; //!< 3rd column elements
		double h[4]; //!< 4th column elements

		double getMinorDeterminant(int row, int col); //! Auxiliary function for computing minor determinants
		double mDet;                                  //! Determinant (set to NaN when not initialized)
};

// GLOBAL Operators
Matrix4D operator* (const Matrix4D& matA, const Matrix4D& matB);
Matrix4D operator* (       double scalar, const Matrix4D& mat );
Matrix4D operator* (const Matrix4D&  mat, double scalar       );
Matrix4D operator+ (const Matrix4D& matA, const Matrix4D& matB);

//! \todo change to Matrix4D::invert
void invert( const float *mat, float *rdst );

std::ostream& operator<<(std::ostream& o, const Matrix4D& m);

#endif
