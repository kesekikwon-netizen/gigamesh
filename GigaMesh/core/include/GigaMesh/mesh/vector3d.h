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

#ifndef VECTOR3D_H
#define VECTOR3D_H


#include <iostream>
#include <sstream> // ostringstream

#include <cmath>
#include <cfloat> // FLT_MAX, FLT_MIN, FLT_EPS, etc.

//#include "matrix4d.h"

class Matrix4D;

//! \file vector3d.h
//!
//! \brief Simple 3D Vector for basic operations.
//!
//! Simple 3D Vector for basic operations.
//!

//! \class Vector3D
//!
//! \brief Simple 3D Vector for basic operations.
//!
//! Simple 3D Vector for basic operations.
//!

class Vector3D {

	public:
		Vector3D();
		Vector3D( double scalar );
		Vector3D( double setX, double setY, double setZ, double setH=1.0 );
		Vector3D( Vector3D* vecToCopy );
		Vector3D(const double* coordXYZH );
		Vector3D(const double* coordXYZ, double setH );
		Vector3D( double rPhi, double rTheta, bool rIsRadiant );

		void set( double setX, double setY, double setZ, double setH=1.0 );
        void set( Vector3D* vecToCopy );
		void set( const Vector3D& vecToCopy );
		void setX( double setX );
		void setY( double setY );
		void setZ( double setZ );
		void setH( double setH );

		// Access
		double getX() const;
		double getY() const;
		double getZ() const;
		double getH() const;
		double getLength3() const;
		double getLength3Squared() const;
		double getLength4() const;
		double getAngleToXinXY();
		double getAngleToZinYZ();
		double getAngleToZ();
		bool  get( double* getX, double* getY, double* getZ, double* getH=nullptr );
		bool  get3( double* getXYZ );
		bool  get3( float* rPositionXYZ );
		bool  getString3( std::string* rStr );
		// Spherical coordinates:
		double getSphPhiDeg();
		double getSphThetaDeg();
		double getSphRadius();

		// Manipulation & Computation
		double normalize3();
		double setLength3( double newLen );
		double normalizeW();
		void   pow3( double exponent );
		void   pow4( double exponent );
		double sum3();
		double sum4();
		bool   projectOntoPlane( Vector3D rPlaneHNF );
		double distanceToLine( const Vector3D* rPos1, const Vector3D* rPos2 );
        double distanceToVectorWithouSqrt( const Vector3D* rPos1, const Vector3D* rPos2 );
		double angleInLineCoord( const Vector3D* rPosTop, const Vector3D* rPosBottom );

		// Operators
		void operator += (const Vector3D& addVec);
		void operator -= (const Vector3D& addVec);
		void operator /= (double divVec);
		void operator *= (double multVec);
		void operator *= (const Matrix4D& multMat);
		bool operator== (const Vector3D& compVec);
		bool operator!= (const Vector3D& compVec);

		// Operations
		Vector3D  projectOnto(const Vector3D& someVec);
		Vector3D  projectOntoLine( const Vector3D& rVertA, const Vector3D& rVertB );
		Vector3D& applyTransformation(const Matrix4D& transformationMatrix);

		// Debuging
		void dumpInfo( bool forMatlab=false, char* someNameTag=nullptr );

	private:
		double x; //!< x-coordinate
		double y; //!< y-coordinate
		double z; //!< z-coordinate
		double h; //!< homogenous coordinate
};

// GLOBAL Operators ------------------------------------------------------------

Vector3D  operator% ( const Vector3D& crossVec1, const Vector3D& crossVec2 );
Vector3D  operator+ ( const Vector3D& someVec1, const Vector3D& someVec2 );
Vector3D  operator- ( const Vector3D& someVec );
Vector3D  operator- ( const Vector3D& someVec1, const Vector3D& someVec2 );
double    operator* ( const Vector3D& someVec1, const Vector3D& someVec2 );
Vector3D  operator* ( const Vector3D& someVec, const Matrix4D& someMat );
Vector3D  operator* ( const Matrix4D& someMat, const Vector3D& someVec );
Vector3D  operator* ( const Vector3D& someVec, double scalar );
Vector3D  operator* ( double scalar, const Vector3D& someVec );
Vector3D  operator/ ( const Vector3D& someVec, double scalar );
Vector3D  operator/ ( double scalar, const Vector3D& someVec );
Vector3D  normalize3( const Vector3D& someVec );
Vector3D  normalize4( const Vector3D& someVec );

Vector3D  compMult( const Vector3D& someVec1, const Vector3D& someVec2 );

double     dot3( const Vector3D& someVec1, const Vector3D& someVec2 );
double     dot4( const Vector3D& someVec1, const Vector3D& someVec2 );
double     angle( const Vector3D& vec1, const Vector3D& vec2 );
double     angle( const Vector3D& vec1, const Vector3D& vec2, const Vector3D& vecNormal );
double     angle3( double* vec1, double* vec2 );
double     angle3ToZ( double* vec );
double     abs3( const Vector3D& someVec );
double     abs4( const Vector3D& someVec );

bool lineLineIntersect( const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, const Vector3D& p4, Vector3D& pa, Vector3D& pb );

std::ostream& operator<<(std::ostream& o, const Vector3D& v);

#endif
