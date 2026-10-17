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

#include <GigaMesh/mesh/line.h>

#include <GigaMesh/mesh/plane.h>

Line::Line( const Vector3D& rPosA, const Vector3D& rPosB) {
	mVecA = rPosA;
	mVecB = rPosB;
}

Line::Line( Vertex& rPosA, Vertex& rPosB) {
	mVertA = &rPosA;
	mVertB = &rPosB;
	mVecA = rPosA.getPositionVector();
	mVecB = rPosB.getPositionVector();
}


bool Line::operator==(Line& l) const {
	Line res;
	getLineIntersection(l, res);
	if(res.getLength() <= DBL_EPSILON) {
		if( abs3( (mVecA-res.getA()) % (l.getA()-res.getA()) ) <= DBL_EPSILON ) {
			return true;
		}
	}
	return false;
}



// Information retrieval:

void Line::getA(Vector3D &p) const {
	p = mVecA;
}

void Line::getB(Vector3D &p) const {
	p = mVecB;
}


Vector3D Line::getA() const {
	return mVecA;
}

Vector3D Line::getB() const {
	return mVecB;
}


void Line::getA(Vertex &p) const {
	p = *mVertA;
}

void Line::getB(Vertex &p) const {
	p = *mVertB;
}

void Line::getData(Vector3D& a, Vector3D& b) const {
	a = mVecA;
	b = mVecB;
}

double Line::getLength() const {
	return sqrt( (mVecA.getX() - mVecB.getX()) * (mVecA.getX() - mVecB.getX()) +
				 (mVecA.getY() - mVecB.getY()) * (mVecA.getY() - mVecB.getY()) +
				 (mVecA.getZ() - mVecB.getZ()) * (mVecA.getZ() - mVecB.getZ()) );
}


bool Line::getDistanceToPoint(Vector3D& p, double& dist) const {

		double t,h1,h2;
		Vector3D x;
		h1 = (p.getX()-mVecA.getX())*(mVecB.getX()-mVecA.getX()) + (p.getY()-mVecA.getY())*(mVecB.getY()-mVecA.getY()) + (p.getZ()-mVecA.getZ())*(mVecB.getZ()-mVecA.getZ());
		h2 = (mVecB.getX()-mVecA.getX())*(mVecB.getX()-mVecA.getX()) + (mVecB.getY()-mVecA.getY())*(mVecB.getY()-mVecA.getY()) + (mVecB.getZ()-mVecA.getZ())*(mVecB.getZ()-mVecA.getZ());
		t = h1/h2;

		//check if shortest line is in line segment
		if((t<=1) && (t>=0)) {
			x.setX( mVecA.getX() + (mVecB.getX()-mVecA.getX())*t );
			x.setY( mVecA.getY() + (mVecB.getY()-mVecA.getY())*t );
			x.setZ( mVecA.getZ() + (mVecB.getZ()-mVecA.getZ())*t );
			dist = abs3(p-x);
			return true;
		}
		dist = abs3(p-x);
		return false;
}


bool Line::getDistanceToPoint(Vector3D& p, double& dist, Line& shortest) const {

		double t,h1,h2;
		Vector3D x;
		h1 = (p.getX()-mVecA.getX())*(mVecB.getX()-mVecA.getX()) + (p.getY()-mVecA.getY())*(mVecB.getY()-mVecA.getY()) + (p.getZ()-mVecA.getZ())*(mVecB.getZ()-mVecA.getZ());
		h2 = (mVecB.getX()-mVecA.getX())*(mVecB.getX()-mVecA.getX()) + (mVecB.getY()-mVecA.getY())*(mVecB.getY()-mVecA.getY()) + (mVecB.getZ()-mVecA.getZ())*(mVecB.getZ()-mVecA.getZ());
		t = h1/h2;

		//check if shortest line is in line segment
		if((t<=1) && (t>=0)) {
			x.setX( mVecA.getX() + (mVecB.getX()-mVecA.getX())*t );
			x.setY( mVecA.getY() + (mVecB.getY()-mVecA.getY())*t );
			x.setZ( mVecA.getZ() + (mVecB.getZ()-mVecA.getZ())*t );
			dist = abs3(p-x);
			shortest.setData(p,x);
			return true;
		}
		dist = abs3(p-x);
		shortest.setData(p,x);
		return false;
}

// Operations:
bool Line::setA(Vector3D& p) {
	mVecA = p;
	return true;
}

bool Line::setB(Vector3D &p) {
	mVecB = p;
	return true;
}

bool Line::setA(Vertex& p) {
	mVertA = &p;
	return true;
}

bool Line::setB(Vertex& p) {
	mVertB = &p;
	return true;
}

void Line::setData(Vector3D& a, Vector3D& b) {
	mVecA = a;
	mVecB = b;
}


/* DEAD: http://paulbourke.net/geometry/lineline3d/
   NEW: http://paulbourke.net/geometry/pointlineplane/lineline.c
   Calculate the line segment PaPb that is the shortest route between
   two lines P1P2 and P3P4. Calculate also the values of mua and mub where
      Pa = P1 + mua (P2 - P1)
      Pb = P3 + mub (P4 - P3)
   Return false if no solution exists.
*/
//! See Vector3D::lineLineIntersect
bool Line::getLineIntersection( Line& other, Line& result ) const {
	Vector3D p1,p2,p3,p4,pa,pb;
	this->getA(p1);
	this->getB(p2);
	other.getA(p3);
	other.getB(p4);

	bool retVal = lineLineIntersect( p1, p2, p3, p4, pa, pb );
	result.setA(pa);
	result.setB(pb);
	return( retVal );
}


// returns -1 if normal to the plane is perpendicular to the line. no result
// returns 0 if parameter u is between 0 and 1. result in res
// returns 1 if parameter u is not between 0 and 1. result in res
int Line::getLinePlaneIntersection(Face& F, Vector3D& res) {


	Vector3D HNF;
	Plane P(F);
	P.getPlaneHNF(&HNF);

	double u, A,B,C,D;
	double x1,x2,y1,y2,z1,z2;
	A = HNF.getX();
	B = HNF.getY();
	C = HNF.getZ();
	D = HNF.getH();
	x1 = mVecA.getX();
	y1 = mVecA.getY();
	z1 = mVecA.getZ();
	x2 = mVecB.getX();
	y2 = mVecB.getY();
	z2 = mVecB.getZ();

	//the denominator is 0 then the normal to the plane is perpendicular to the line. Thus the line is either parallel to the plane and there
	//are no solutions or the line is on the plane in which case are infinite solutions
	if ( fabs( A*(x1-x2) + B*(y1-y2) + C*(z1-z2) ) <= DBL_EPSILON ) {
		return -1;
	}

	u = (A*x1 + B*y1 + C*z1 + D ) /
		(A*(x1-x2) + B*(y1-y2) + C*(z1-z2));

	res = mVecA + u * (mVecB-mVecA);

	if(u>=0 && 1>=u) {
		return 0;
	}

	return 1;

}


// returns -1 if normal to the plane is perpendicular to the line. no result
// returns 0 if parameter u is between 0 and 1. result in res
// returns 1 if parameter u is not between 0 and 1. result in res
int Line::getLinePlaneIntersection(Plane& P, Vector3D& res) {


	Vector3D HNF;
	P.getPlaneHNF(&HNF);

	double u, A,B,C,D;
	double x1,x2,y1,y2,z1,z2;
	A = HNF.getX();
	B = HNF.getY();
	C = HNF.getZ();
	D = HNF.getH();
	x1 = mVecA.getX();
	y1 = mVecA.getY();
	z1 = mVecA.getZ();
	x2 = mVecB.getX();
	y2 = mVecB.getY();
	z2 = mVecB.getZ();

	//the denominator is 0 then the normal to the plane is perpendicular to the line. Thus the line is either parallel to the plane and there
	//are no solutions or the line is on the plane in which case are infinite solutions
	if ( fabs( A*(x1-x2) + B*(y1-y2) + C*(z1-z2) ) <= DBL_EPSILON ) {
		return -1;
	}

	u = (A*x1 + B*y1 + C*z1 + D ) /
		(A*(x1-x2) + B*(y1-y2) + C*(z1-z2));

	res = mVecA + u * (mVecB-mVecA);

	if(u>=0 && 1>=u) {
		return 0;
	}

	return 1;

}


//! get t from equation p = a + t *(b-a)
bool Line::getParameter(Vector3D& p, double& t) {

	if( fabs( mVecB.getX() - mVecA.getX() )  > DBL_EPSILON ) {
		t = (p.getX()-mVecA.getX()) / (mVecB.getX() - mVecA.getX()) ;
		return true;
	}
	if( fabs( mVecB.getY() - mVecA.getY() )  > DBL_EPSILON ) {
		t = (p.getY()-mVecA.getY()) / (mVecB.getY() - mVecA.getY()) ;
		return true;
	}
	if( fabs( mVecB.getZ() - mVecA.getZ() )  > DBL_EPSILON ) {
		t = (p.getZ()-mVecA.getZ()) / (mVecB.getZ() - mVecA.getZ()) ;
		return true;
	}
	return false;
}

//! project @this line to tar plane , resulting line is in res
void Line::projectonPlane(Plane& tar, Line& res) {
	Vector3D N;
	tar.getPlaneHNF(&N); // normal of the plane
	Vector3D vStart, vEnd;
	this->getA(vStart);
	this->getB(vEnd);

	double d = N.getH();

	float sn = dot3( vStart, N ) + d;
	float en = dot3( vEnd, N ) + d;
	float n2 = dot3(N,N); // square length of normal

	vStart = vStart - (sn / n2) * N; // projection on plane
	vEnd = vEnd - (en / n2) * N; // projection on plane

	res.setData(vStart, vEnd);
}
