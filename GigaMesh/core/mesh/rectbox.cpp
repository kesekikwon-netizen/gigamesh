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

#include <GigaMesh/mesh/rectbox.h>

using namespace std;

//! Alternate Constructor -requires only coordiantes and radii. Memory will be allocated. Optional a color can be set.
//!
//! This constructor is more convenient as it does not require an array to be handled outside this object.
//!
//! But: e.g. if this object has to be transfomed, this has to be done with its own method.
//!
//! Or in other words: using this Constructor prevents CUDA/OpenCL integration and should therefore only be used
//! for testing or simple visualizations.
RectBox::RectBox( Vector3D vecCenter, Vector3D vec1, Vector3D vec2, Vector3D vec3 ) {
	x = static_cast<double*>(malloc( sizeof( double ) )); *x = vecCenter.getX();
	y = static_cast<double*>(malloc( sizeof( double ) )); *y = vecCenter.getY();
	z = static_cast<double*>(malloc( sizeof( double ) )); *z = vecCenter.getZ();
	nX1 = static_cast<double*>(malloc( sizeof( double ) )); *nX1 = vec1.getX();
	nY1 = static_cast<double*>(malloc( sizeof( double ) )); *nY1 = vec1.getY();
	nZ1 = static_cast<double*>(malloc( sizeof( double ) )); *nZ1 = vec1.getZ();
	nX2 = static_cast<double*>(malloc( sizeof( double ) )); *nX2 = vec2.getX();
	nY2 = static_cast<double*>(malloc( sizeof( double ) )); *nY2 = vec2.getY();
	nZ2 = static_cast<double*>(malloc( sizeof( double ) )); *nZ2 = vec2.getZ();
	nX3 = static_cast<double*>(malloc( sizeof( double ) )); *nX3 = vec3.getX();
	nY3 = static_cast<double*>(malloc( sizeof( double ) )); *nY3 = vec3.getY();
	nZ3 = static_cast<double*>(malloc( sizeof( double ) )); *nZ3 = vec3.getZ();
	freePointers = true;
}

//! Destructor sets properties to not a number or NULL.
//!
//! This is done just in case we referer to an object still in the memory,
//! which is already destroyed.
RectBox::~RectBox() {
	if( freePointers ) {
		free( x ); x = nullptr;
		free( y ); y = nullptr;
		free( z ); z = nullptr;
		free( nX1 ); nX1 = nullptr;
		free( nY1 ); nY1 = nullptr;
		free( nZ1 ); nZ1 = nullptr;
		free( nX2 ); nX2 = nullptr;
		free( nY2 ); nY2 = nullptr;
		free( nZ2 ); nZ2 = nullptr;
		free( nX3 ); nX3 = nullptr;
		free( nY3 ); nY3 = nullptr;
		free( nZ3 ); nZ3 = nullptr;
		freePointers = false;
	}
}

// return the boxes center -----------------------------------------------------

//!
//! Returns the x-coordinate of the Spehere's center.
double RectBox::getX() const {
	return *x;
}

//!
//! Returns the y-coordinate of the Spehere's center.
double RectBox::getY() const {

	return *y;
}

//! Returns the z-coordinate of the Spehere's center.
double RectBox::getZ() const {
	return *z;
}

//!
//! Returns the x-, y- and z-coordinate (in a struct).
Vector3D RectBox::getCoordCenter() {
	Vector3D coordBoxCenter;
	coordBoxCenter.setX( *x );
	coordBoxCenter.setY( *y );
	coordBoxCenter.setZ( *z );
	return coordBoxCenter;
}

//!
//! Return the type (=class) of this object as an id.
int RectBox::getType() {
	return Primitive::IS_RECTBOX;
}

// return the boxes corners (frontside) ----------------------------------------

//!
//! Returns the x,y,z-coordinates of point A at the corner of the Box.
Vector3D RectBox::getCoordA() {
	Vector3D coordBoxCorner;
	coordBoxCorner.setX( *x + *nX1 - *nX2 - *nX3 ); 
	coordBoxCorner.setY( *y + *nY1 - *nY2 - *nY3 );
	coordBoxCorner.setZ( *z + *nZ1 - *nZ2 - *nZ3 );
	return coordBoxCorner;
}

//!
//! Returns the x,y,z-coordinates of point B at the corner of the Box.
Vector3D RectBox::getCoordB() {
	Vector3D coordBoxCorner;
	coordBoxCorner.setX( *x + *nX1 - *nX2 + *nX3 );
	coordBoxCorner.setY( *y + *nY1 - *nY2 + *nY3 );
	coordBoxCorner.setZ( *z + *nZ1 - *nZ2 + *nZ3 );
	return coordBoxCorner;
}

//!
//! Returns the x,y,z-coordinates of point C at the corner of the Box.
Vector3D RectBox::getCoordC() {
	Vector3D coordBoxCorner;
	coordBoxCorner.setX( *x + *nX1 + *nX2 + *nX3 );
	coordBoxCorner.setY( *y + *nY1 + *nY2 + *nY3 );
	coordBoxCorner.setZ( *z + *nZ1 + *nZ2 + *nZ3 );
	return coordBoxCorner;
}

//!
//! Returns the x,y,z-coordinates of point D at the corner of the Box.
Vector3D RectBox::getCoordD() {
	Vector3D coordBoxCorner;
	coordBoxCorner.setX( *x + *nX1 + *nX2 - *nX3 );
	coordBoxCorner.setY( *y + *nY1 + *nY2 - *nY3 );
	coordBoxCorner.setZ( *z + *nZ1 + *nZ2 - *nZ3 );
	return coordBoxCorner;
}

// return the boxes corners (backside) -----------------------------------------

//!
//! Returns the x,y,z-coordinates of point E at the corner of the Box.
Vector3D RectBox::getCoordE() {
	Vector3D coordBoxCorner;
	coordBoxCorner.setX( *x - *nX1 - *nX2 - *nX3 );
	coordBoxCorner.setY( *y - *nY1 - *nY2 - *nY3 );
	coordBoxCorner.setZ( *z - *nZ1 - *nZ2 - *nZ3 );
	return coordBoxCorner;
}

//!
//! Returns the x,y,z-coordinates of point F at the corner of the Box.
Vector3D RectBox::getCoordF() {
	Vector3D coordBoxCorner;
	coordBoxCorner.setX( *x - *nX1 - *nX2 + *nX3 );
	coordBoxCorner.setY( *y - *nY1 - *nY2 + *nY3 );
	coordBoxCorner.setZ( *z - *nZ1 - *nZ2 + *nZ3 );
	return coordBoxCorner;
}

//!
//! Returns the x,y,z-coordinates of point G at the corner of the Box.
Vector3D RectBox::getCoordG() {
	Vector3D coordBoxCorner;
	coordBoxCorner.setX( *x - *nX1 + *nX2 + *nX3 );
	coordBoxCorner.setY( *y - *nY1 + *nY2 + *nY3 );
	coordBoxCorner.setZ( *z - *nZ1 + *nZ2 + *nZ3 );
	return coordBoxCorner;
}

//!
//! Returns the x,y,z-coordinates of point H at the corner of the Box.
Vector3D RectBox::getCoordH() {
	Vector3D coordBoxCorner;
	coordBoxCorner.setX( *x - *nX1 + *nX2 - *nX3 );
	coordBoxCorner.setY( *y - *nY1 + *nY2 - *nY3 );
	coordBoxCorner.setZ( *z - *nZ1 + *nZ2 - *nZ3 );
	return coordBoxCorner;
}

















