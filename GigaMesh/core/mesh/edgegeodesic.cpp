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

#include <GigaMesh/mesh/edgegeodesic.h>

using namespace std;

//! Constructor
EdgeGeodesic::EdgeGeodesic( Face* rFromFace, Face::eEdgeNames rEdgeIdx, GeodEntry* rGeoDistA, GeodEntry* rGeoDistB )
	: mFromFace( rFromFace ),
	  mEdgeIdx( rEdgeIdx ),
	  mGeoDistA( rGeoDistA ),
	  mGeoDistB( rGeoDistB ) {
	double geodDistA;
	rGeoDistA->getGeodDist( &geodDistA );
	if( geodDistA <= 0.0 ) {
		cout << "[EdgeGeodesic::" << __FUNCTION__ << "] Warning: Bad setGeoDistA: " << geodDistA << "!" << endl;
	}
	double geodDistB;
	rGeoDistB->getGeodDist( &geodDistB );
	if( geodDistB <= 0.0 ) {
		cout << "[EdgeGeodesic::" << __FUNCTION__ << "] Warning: Bad setGeoDistA: " << geodDistB << "!" << endl;
	}
	//cout << "[EdgeGeodesic::" << __FUNCTION__ << "] geodDistA: " << geodDistA << " geodDistB: " << geodDistB <<  endl;
}

// Information retrival -------------------------------------------------------

//! Returns a pointer to the Face the Edge belongs to.
Face* EdgeGeodesic::getFace() {
	return mFromFace;
}

//! Returns the geodesic distance of the vertex A.
bool EdgeGeodesic::getGeoDistA( double* rGeodDist ) {
	return mGeoDistA->getGeodDist( rGeodDist );
}

//! Returns the geodesic distance of the vertex B.
bool EdgeGeodesic::getGeoDistB( double* rGeodDist ) {
	return mGeoDistB->getGeodDist( rGeodDist );
}

//! Returns the geodesic angle at the vertex A.
bool EdgeGeodesic::getGeodAngleA( double* rGeodAngle ) {
	return mGeoDistA->getGeodAngle( rGeodAngle );
}

//! Returns the geodesic angle at the vertex B.
bool EdgeGeodesic::getGeodAngleB( double* rGeodAngle ) {
	return mGeoDistB->getGeodAngle( rGeodAngle );
}

//! Returns the reference to the geodesic distance of the vertex A.
GeodEntry* EdgeGeodesic::getGeoDistARef() {
	return mGeoDistA;
}

//! Returns reference to the geodesic distance of the vertex B.
GeodEntry* EdgeGeodesic::getGeoDistBRef() {
	return mGeoDistB;
}

//! Returns the shorter geodesic distance of the vertex A and B.
double EdgeGeodesic::getGeoDistMin() {
	double geodDistA;
	double geodDistB;
	mGeoDistA->getGeodDist( &geodDistA );
	mGeoDistB->getGeodDist( &geodDistB );
	if( geodDistA <= geodDistB ) {
		return geodDistA;
	}
	return geodDistB;
}

//! Returns the longer geodesic distance of the vertex A and B.
double EdgeGeodesic::getGeoDistMax() {
	double geodDistA;
	double geodDistB;
	mGeoDistA->getGeodDist( &geodDistA );
	mGeoDistB->getGeodDist( &geodDistB );
	if( geodDistA > geodDistB ) {
		return geodDistA;
	}
	return geodDistB;
}

Face::eEdgeNames EdgeGeodesic::getEdgeIdx() {
	//! Return the edges index - see Face.
	return mEdgeIdx;
}

//! Returns the reference to 1st vertex of the edge.
Vertex* EdgeGeodesic::getVertA() {
	return mFromFace->getVertexFromEdgeA( mEdgeIdx );
}

//! Returns the reference to 2nd vertex of the edge.
Vertex* EdgeGeodesic::getVertB() {
	return mFromFace->getVertexFromEdgeB( mEdgeIdx );
}

bool EdgeGeodesic::getPositionA( Vector3D* vertAPos ) {
	//! Return the world coordinates of point A.
	Vertex* vertA = mFromFace->getVertexFromEdgeA( mEdgeIdx );
	if( vertA == nullptr ) {
		return false;
	}
	vertAPos->set( vertA->getPositionVector() );
	return true;
}

bool EdgeGeodesic::getPositionB( Vector3D* vertBPos ) {
	//! Return the world coordinates of point B.
	Vertex* vertB = mFromFace->getVertexFromEdgeB( mEdgeIdx );
	if( vertB == nullptr ) {
		return false;
	}
	vertBPos->set( vertB->getPositionVector() );
	return true;
}

bool EdgeGeodesic::getFuncValueA( double* funcValA ) {
	//! Fetch the function value of Vertex A using Vertex::getFuncValue.
	//! Return value of Vertex::getFuncValue or false in case of an error.
	Vertex* vertA = mFromFace->getVertexFromEdgeA( mEdgeIdx );
	if( vertA == nullptr ) {
		return false;
	}
	return vertA->getFuncValue( funcValA );
}

bool EdgeGeodesic::getFuncValueB( double* funcValB ) {
	//! Fetch the function value of Vertex B using Vertex::getFuncValue.
	//! Passes on return value of Vertex::getFuncValue.
	Vertex* vertB = mFromFace->getVertexFromEdgeA( mEdgeIdx );
	if( vertB == nullptr ) {
		return false;
	}
	return vertB->getFuncValue( funcValB );
}

// Comparison ----------------------------------------------------------------

bool EdgeGeodesic::shorterThan( EdgeGeodesic* edge1, EdgeGeodesic* edge2 ) {
	//! Used for sorting the heap. Remark: inverted logic as we wan't shortest first.
	if( edge1->getGeoDistMin() == edge2->getGeoDistMin() ) {
		// when both short distances are of equal length, we have to sort by the longer distance:
		return ( edge1->getGeoDistMax() > edge2->getGeoDistMax() );
	}
	return ( edge1->getGeoDistMin() > edge2->getGeoDistMin() );
}

// Debuging ------------------------------------------------------------------

void EdgeGeodesic::dumpInfo() {
	Vertex* vertA = mFromFace->getVertexFromEdgeA( mEdgeIdx );
	Vertex* vertB = mFromFace->getVertexFromEdgeB( mEdgeIdx );
	double dAB = ( vertB->getPositionVector() - vertA->getPositionVector() ).getLength3();
	double geodDistA;
	double geodDistB;
	mGeoDistA->getGeodDist( &geodDistA );
	mGeoDistB->getGeodDist( &geodDistB );
	cout << "[EdgeGeodesic] Info: " << geodDistA << " " << geodDistB << " Face: " << mFromFace->getIndex() << " Edge: " << getEdgeIdx() << " " << vertA->getIndex() << " -> " << vertB->getIndex() << " " << dAB << endl;
	//cout << "[EdgeGeodesic] Info: " << getGeoDistMin() << " " << getGeoDistMax() << " Edge: " << getEdgeIdx() << endl;
}
