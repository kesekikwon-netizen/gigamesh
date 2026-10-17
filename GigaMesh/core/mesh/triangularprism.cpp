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

#include <memory>

#include <GigaMesh/mesh/triangularprism.h>

#include <GigaMesh/mesh/line.h>
#include <GigaMesh/mesh/plane.h>

using namespace std;

TriangularPrism::TriangularPrism(Vector3D* vertices) {
	for(int i=0; i<6; ++i) mVertex[i] = vertices[i];
}

//! helper function for selection: in @param tri are 6 pointers to Vector3D
//! representing 2 triangles, which can be interpreted as a prism,
//! which edges will be returned
//alignment of vertices
//				4/5
//			   /\
//			  /  \
//			 /    \
//			/______\
//			0/1       2/3

Line TriangularPrism::gettriangularPrismEdge(int nr) {

	Line res;

	switch(nr) {
		//front
		case 0:
			res.setData(mVertex[0], mVertex[2]);
			break;
		case 1:
			res.setData(mVertex[2], mVertex[3]);
			break;
		case 2:
			res.setData(mVertex[3], mVertex[1]);
			break;
		case 3:
			res.setData(mVertex[1], mVertex[0]);
			break;

		//right
		case 4:
			res.setData(mVertex[4], mVertex[2]);
			break;
		case 5:
			res.setData(mVertex[3], mVertex[5]);
			break;

		//left
		case 6:
			res.setData(mVertex[0], mVertex[4]);
			break;
		case 7:
			res.setData(mVertex[1], mVertex[5]);
			break;

		//back
		case 8:
			res.setData(mVertex[4], mVertex[5]);
			break;
		default:
			cerr << "[TriangularPrism::     " << __FUNCTION__ << " nr out of range"<<endl;
			break;
	}

	return res;
}


//! same as above, but the planes will be returned
std::unique_ptr<Plane> TriangularPrism::gettriangularPrismPlane(int nr) {


	switch(nr) {
		//the three sides of the triangular prism
		case 0:
			return std::make_unique<Plane> ( mVertex[0], mVertex[2], mVertex[1] );
			break;
		case 1:
			return std::make_unique<Plane> ( mVertex[2], mVertex[4], mVertex[3] );
			break;
		case 2:
			return std::make_unique<Plane> (mVertex[4], mVertex[0], mVertex[5] );
			break;

		//top
		case 3:
			return std::make_unique<Plane> ( mVertex[0], mVertex[2], mVertex[4] );
			break;

		//bottom
		case 4:
			return std::make_unique<Plane> ( mVertex[1], mVertex[3], mVertex[5] );
			break;
		default:
			cerr << "[TriangularPrism::     " << __FUNCTION__ << " nr out of range"<<endl;
            return nullptr;
			break;
	}

}


//check if point is in triangular prism
bool TriangularPrism::PointisintringularPrism(Vector3D &p) {

	double distanceToPlane;
	gettriangularPrismPlane( 0 )->getDistanceToPoint( &p, &distanceToPlane );
	if( distanceToPlane < 0 ) {
		return false;
	}
	gettriangularPrismPlane( 1 )->getDistanceToPoint( &p, &distanceToPlane );
	if( distanceToPlane < 0 ) {
		return false;
	}
	gettriangularPrismPlane( 2 )->getDistanceToPoint( &p, &distanceToPlane );
	if( distanceToPlane < 0 ) {
		return false;
	}
	return true;
#ifdef MORE_INEFFICENT_LEGACY_CODE
//	std::unique_ptr<Plane> f[3];
	//Plane f[3];
	double d[3];
	/*Plane f0 = gettriangularPrismPlane(0);
	Plane f1 = gettriangularPrismPlane(1);
	Plane f2 = gettriangularPrismPlane(2);*/

	for(int i = 0; i < 3; ++i) {
		gettriangularPrismPlane(i)->getDistanceToPoint(&p, &d[i]);
	}


	if( (d[0] + DBL_EPSILON >= 0) && (d[1] + DBL_EPSILON >= 0) && (d[2] + DBL_EPSILON >= 0) ) {
		return true;
	}
	return false;
#endif
}

