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

#include <GigaMesh/mesh/cube.h>
#include <GigaMesh/mesh/plane.h>


//<<<<<<< HEAD:mesh/cube.cpp

//=======

/*
// Sets default values
// ----------------------------------------------------
#define OCTNODEDEFAULTS                     \
	mparent( NULL ),                     \
	misleaf( false )


Octnode::Octnode() : OCTNODEDEFAULTS {
	for ( int i=0; i <8; i++) {
		mchildren[i] = NULL;
	}
>>>>>>> f0f3f7c7b19d0576c7c33d0d861b78b50f39032a:mesh/octnode.cpp
}
*/

using namespace std;

Cube::Cube(const Vector3D & center, double scale): mscale(scale) {
	mcenter = center;
}

Cube::Cube(double x, double y, double z, double scale): mscale(scale) {
	mcenter.setX( x );
	mcenter.setY( y );
	mcenter.setZ( z );
}


//alignment of vertices
//								1							0
//								 _________________________
//								/ _____________________  /|
//							   / / ___________________/ / |
//							  / / /| |               / /  |
//							 / / / | |              / / . |
//							/ / /| | |             / / /| |
//						   / / / | | |            / / / | |
//						  / / /  | | |         4 / / /| | |
//						 / /_/__________________/ / / | | |
//				  6 	/________________________/ /  | | |
//						| ______________________ | |  | | |
//						| | |    | | |_________| | |__| | |
//						| | |  3 | |___________| | |____| | 2
//						| | |   / / ___________| | |_  / /
//						| | |  / / /           | | |/ / /
//						| | | / / /            | | | / /
//						| | |/ / /             | | |/ /
//						| | | / /              | | ' /
//						| | |/_/_______________| |  /
//						| |____________________| | /
//						|________________________|/
//					7							5
Vector3D Cube::getVertex(int nr) {

	Vector3D res;

	switch(nr) {
		case 0:
			res.setX( mcenter.getX() + mscale );
			res.setY( mcenter.getY() + mscale );
			res.setZ( mcenter.getZ() + mscale );
			break;
		case 1:
			res.setX( mcenter.getX() - mscale );
			res.setY( mcenter.getY() + mscale );
			res.setZ( mcenter.getZ() + mscale );
			break;
		case 2:
			res.setX( mcenter.getX() + mscale );
			res.setY( mcenter.getY() - mscale );
			res.setZ( mcenter.getZ() + mscale );
			break;
		case 3:
			res.setX( mcenter.getX() - mscale );
			res.setY( mcenter.getY() - mscale );
			res.setZ( mcenter.getZ() + mscale );
			break;
		case 4:
			res.setX( mcenter.getX() + mscale );
			res.setY( mcenter.getY() + mscale );
			res.setZ( mcenter.getZ() - mscale );
			break;
		case 5:
			res.setX( mcenter.getX() + mscale );
			res.setY( mcenter.getY() - mscale );
			res.setZ( mcenter.getZ() - mscale );
			break;
		case 6:
			res.setX( mcenter.getX() - mscale );
			res.setY( mcenter.getY() + mscale );
			res.setZ( mcenter.getZ() - mscale );
			break;
		case 7:
			res.setX( mcenter.getX() - mscale );
			res.setY( mcenter.getY() - mscale );
			res.setZ( mcenter.getZ() - mscale );
			break;
		default:
			cerr << "[octnode::     " << __FUNCTION__ << " nr out of range"<<endl;
	}

	return res;
}

Line Cube::getEdge(int nr) {

	Line res;
	Vector3D v1,v2;

	switch(nr) {
		//back of cube
		case 0:
			v1=getVertex(0);
			v2=getVertex(1);
			res.setData(v1, v2);
			break;
		case 1:
			v1=getVertex(3);
			v2=getVertex(2);
			res.setData(v1, v2);
			break;
		case 2:
			v1=getVertex(0);
			v2=getVertex(2);
			res.setData(v1, v2);
			break;
		case 3:
			v1=getVertex(1);
			v2=getVertex(3);
			res.setData(v1, v2);
			break;
		//front of cube
		case 4:
			v1=getVertex(7);
			v2=getVertex(5);
			res.setData(v1, v2);
			break;
		case 5:
			v1=getVertex(4);
			v2=getVertex(6);
			res.setData(v1, v2);
			break;
		case 6:
			v1=getVertex(4);
			v2=getVertex(5);
			res.setData(v1, v2);
			break;
		case 7:
			v1=getVertex(7);
			v2=getVertex(6);
			res.setData(v1, v2);
			break;

		//left of cube
		case 8:
			v1=getVertex(6);
			v2=getVertex(1);
			res.setData(v1, v2);
			break;
		case 9:
			v1=getVertex(7);
			v2=getVertex(3);
			res.setData(v1, v2);
			break;

		//right of cube
		case 10:
			v1=getVertex(4);
			v2=getVertex(0);
			res.setData(v1, v2);
			break;
		case 11:
			v1=getVertex(5);
			v2=getVertex(2);
			res.setData(v1, v2);
			break;
		default:
			cerr << "[octnode::     " << __FUNCTION__ << " nr out of range"<<endl;
	}


	return res;
}

std::unique_ptr<Plane> Cube::getPlane(int nr) {

	//Plane res;
	Vector3D a,b,c;

	switch(nr) {
		//front and back
		case 0:
			a= getVertex(0);
			b= getVertex(2);
			c= getVertex(1);
			//res = new Plane( a, b, c );
			//res.setData( a, b, c );
			return std::make_unique<Plane>( a, b, c );
			break;
		case 1:
			a= getVertex(7);
			b= getVertex(5);
			c= getVertex(4);
			//res.setData( a, b, c );
			return std::make_unique<Plane>( a, b, c );
			break;

		//left and right
		case 2:
			a= getVertex(1);
			b= getVertex(3);
			c= getVertex(7);
			//res.setData( a, b, c );
			return std::make_unique<Plane>( a, b, c );
			break;
		case 3:
			a= getVertex(5);
			b= getVertex(0);
			c= getVertex(4);
			//res.setData( a, b, c );
			return std::make_unique<Plane>( a, b, c );
			break;

		//top and bottom
		case 4:
			a= getVertex(1);
			b= getVertex(0);
			c= getVertex(4);
			//res.setData( a, b, c );
			return std::make_unique<Plane>( a, b, c );
			break;
		case 5:
			a= getVertex(7);
			b= getVertex(5);
			c= getVertex(2);
			//res.setData( a, b, c );
			return std::make_unique<Plane>( a, b, c );
			break;
		default:
			cerr << "[octnode::     " << __FUNCTION__ << " nr out of range"<<endl;
            return nullptr;
			break;
	}

	//return res;
}

//returns true if point is inside or on  border of the cube
bool Cube::PointisinCube(Vector3D& p) {
	if (    (fabs( p.getX() - mcenter.getX() ) <= mscale+DBL_EPSILON)
		 && (fabs( p.getY() - mcenter.getY() ) <= mscale+DBL_EPSILON)
		 && (fabs( p.getZ() - mcenter.getZ() ) <= mscale+DBL_EPSILON) ) {
        return true;
    }
    return false;
}

bool Cube::isFaceCompletelyInCube(Face *face){
    Vector3D posA;
    Vector3D posB;
    Vector3D posC;
    face->getVertA()->getPositionVector(&posA);
    face->getVertB()->getPositionVector(&posB);
    face->getVertC()->getPositionVector(&posC);
    if( PointisinCube(posA)
        && PointisinCube(posB)
        && PointisinCube(posC)){
        return true;
    }
    return false;
}


//!  linecubeintersection function
// computes distance of line ab to the center and tests if line segment containing this nearest point is inside the cube
bool Cube::linecubeintersection(Vector3D* a, Vector3D* b) {
	double t = ( (mcenter.getX()-a->getX()) * (b->getX()-a->getX()) + (mcenter.getY()-a->getY()) * (b->getY()-a->getY()) + (mcenter.getZ()-a->getZ()) * (b->getZ()-a->getZ()) )
		 / ( (a->getX()-b->getX())*(a->getX()-b->getX()) + (a->getY()-b->getY()) * (a->getY()-b->getY()) + (a->getZ()-b->getZ()) * (a->getZ()-b->getZ()) ) ;
	Vector3D ba = (*b) -(*a);
	Vector3D i = (*a) + t*ba;

	if ( fabs( mcenter.getX()-i.getX() )<= mscale && fabs( mcenter.getY()-i.getY() )<= mscale && fabs( mcenter.getZ()-i.getZ() )<= mscale ) {
		return true;
	}
	return false;
}


bool Cube::trianglecubeintersection(Face* face) {


	Vector3D A( face->getVertA()->getX(),face->getVertA()->getY(),face->getVertA()->getZ() );
	Vector3D B( face->getVertB()->getX(),face->getVertB()->getY(),face->getVertB()->getZ() );
	Vector3D C( face->getVertC()->getX(),face->getVertC()->getY(),face->getVertC()->getZ() );

	//check if a vertex of the triangle is in the cube
	if ( PointisinCube(A) ||  PointisinCube(B) ||  PointisinCube(C) ) {
		return true;
	}

	Line TriangleEdge[3];
	TriangleEdge[0] = Line(A,B);
	TriangleEdge[1] = Line(B,C);
	TriangleEdge[2] = Line(C,A);

	std::unique_ptr<Plane> cp[6];
	for(int i=0; i<6; ++i) {
		cp[i] = getPlane(i);
	}

	Vector3D tmp;

	Line Edge[12];
	for(int i=0; i<12; ++i) {
		Edge[i] = getEdge(i);
	}

	Plane triplane ( A, B, C );

//	//get intersections of cube edges with triangular plane
//		for(int k=0; k<12; ++k) {
//			if( Edge[k].getLinePlaneIntersection(triplane, tmp) == 0) {
//				if(face->pointintriangle(&tmp)) {
//					if(PointisinCube(tmp)) {
//						return true;
//					}
//				}
//			}
//		}

	//get intersections of cube edges with triangular plane
	for(Line& line : Edge)
	{
		if( line.getLinePlaneIntersection(triplane, tmp) == 0)
		{
			if(face->pointintriangle(&tmp))
			{
				if(PointisinCube(tmp))
				{
					return true;
				}
			}
		}
	}

//	//get intersections of triangular edges with cube planes
//	for(int j=0; j<6; ++j) {
//		for(int k=0; k<3; ++k) {
//			if( TriangleEdge[k].getLinePlaneIntersection(*cp[j], tmp) == 0 ) {
//				if(PointisinCube(tmp)) {
//					if( face->pointintriangle(&tmp) ) {
//						//cout << "[octnode::     " << __FUNCTION__ << "TEST TRIANGLE TRUE "<<endl;

//						//return INTERSECTION;
//						return true;
//					}
//				}
//			}
//		}
//	}

	for(unique_ptr<Plane>& plane : cp)
	{
		for(Line& triangleLine : TriangleEdge)
		{
			if( triangleLine.getLinePlaneIntersection(*plane, tmp) == 0 )
			{
				if(PointisinCube(tmp))
				{
					if( face->pointintriangle(&tmp) )
					{
						//cout << "[octnode::     " << __FUNCTION__ << "TEST TRIANGLE TRUE "<<endl;
						//return INTERSECTION;
						return true;
					}
				}
			}
		}
	}

	return false;
}



//! check if theres an intersection between triangular prism and octnode cube
//test if there is intersection of this cube with triangular prism,
//tri is a vector containing the 6 points which describe 5 planes of the triangular prism
enum relativePosition Cube::CubeisintriangularPrism(TriangularPrism& tri, vector<Line> &drawlines) {

	//return true if octnode center is in triangular prism
	if( tri.PointisintringularPrism(mcenter) ) {
			//check if cube is completely contained in triangular prism, i.e. all vertices are contained
			bool containedflag=false;
			Vector3D vertices[8];
			for(int i=0; i<8; ++i) {
				vertices[i] = getVertex(i);
				containedflag = tri.PointisintringularPrism(vertices[i]);
				if (containedflag) {
					continue;
				}
				else {
					break;
				}
			}
			if(containedflag) {
				return CONTAINED;
			}
			//else there is intersection, because center of cube is contained
			return INTERSECTION;
	}

	std::unique_ptr<Plane> f[3];
	for(int i=0; i<3; ++i) {
		f[i] = tri.gettriangularPrismPlane(i);
	}
	Line PrismEdge[9];
	for(int i=0; i<9; ++i) {
		PrismEdge[i] = tri.gettriangularPrismEdge(i);
	}
/*
	Vector3D vertices[8];
	for(int i=0; i<8; ++i) {
		vertices[i] = getVertex(i);
	}*/
	Line Edge[12];
	for(int i=0; i<12; ++i) {
		Edge[i] = getEdge(i);
	}
	std::unique_ptr<Plane> cp[6];
	for(int i=0; i<6; ++i) {
		cp[i] = getPlane(i);
	}

	for(Line& line : PrismEdge)
	{
		drawlines.push_back(line);
	}

	//get intersections of triangular prism edges with cube planes
	Vector3D tmp;

//	for(int j=0; j<6; ++j) {
//		for(int k=0; k<9; ++k) {
//			if( PrismEdge[k].getLinePlaneIntersection(*cp[j], tmp) == 0 ) {
//				if(PointisinCube(tmp)) {
//					if( tri.PointisintringularPrism(tmp) ) {
//						//cout << "[octnode::     " << __FUNCTION__ << "TEST TRIANGLE TRUE "<<endl;

//						return INTERSECTION;
//					}
//				}
//			}
//		}
//	}

	for(std::unique_ptr<Plane>& plane : cp)
	{
		for(Line& line : PrismEdge)
		{
			if( line.getLinePlaneIntersection(*plane, tmp) == 0 )
			{
				if(PointisinCube(tmp))
				{
					if( tri.PointisintringularPrism(tmp) )
					{
						//cout << "[octnode::     " << __FUNCTION__ << "TEST TRIANGLE TRUE "<<endl;
						return INTERSECTION;
					}
				}
			}
		}
	}

//	//get intersections of cube edges with triangular prism planes
//	for(int j=0; j<3; ++j) {
//		for(int k=0; k<12; ++k) {
//			if( Edge[k].getLinePlaneIntersection(*f[j], tmp) == 0) {
//				if(tri.PointisintringularPrism(tmp)) {

//					if(PointisinCube(tmp)) {
//						return INTERSECTION;
//					}
//				}
//			}
//		}
//	}

	for(std::unique_ptr<Plane>& plane : f)
	{
		for(Line& line : Edge)
		{
			if( line.getLinePlaneIntersection(*plane, tmp) == 0)
			{
				if(tri.PointisintringularPrism(tmp))
				{
					if(PointisinCube(tmp))
					{
						return INTERSECTION;
					}
				}
			}
		}
	}

	//cout << "[octnode::     " << __FUNCTION__ << "TEST TRIANGLE FALSE "<<endl;
/*	for(int o=0; o<3; ++o) {
		delete f[o];
	}
	for(int o=0; o<6; ++o) {
		delete cp[o];
	}*/
	return DISJOINT;
}
