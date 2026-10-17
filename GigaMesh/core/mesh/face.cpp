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

#include <iostream>
#include <cmath>
#include <sys/types.h>
#include <set>
#include <utility>
#include <array>


#include <GigaMesh/mesh/face.h>
#include <GigaMesh/mesh/vertexofface.h>
#include <GigaMesh/mesh/edgegeodesic.h>
#include <GigaMesh/mesh/plane.h>

#include <GigaMesh/logging/Logging.h>

#define FACE_NEIGHBOUR_AB mNeighbourFaces[0]
#define FACE_NEIGHBOUR_BC mNeighbourFaces[1]
#define FACE_NEIGHBOUR_CA mNeighbourFaces[2]

// proper access to the normal:
#define NORMAL_X        mNormalXYZ[0]
#define NORMAL_Y        mNormalXYZ[1]
#define NORMAL_Z        mNormalXYZ[2]

// Sets default values - to be used by all contructors!
// ----------------------------------------------------
#define FACEINITDEFAULTS \
	vertA( NULL ),                      \
	vertB( NULL ),                      \
	vertC( NULL ),                      \
	mFuncValue( 0.0 ),                  \
	mNeighbourFacesNonManifold( 0 ),    \
	mNeighbourFaces( new Face*[3] ),    \
	mUVs{0.0F,0.0F,0.0F,0.0F,0.0F,0.0F}

using namespace std;

//! Constructor - see also Primitive::Primitive
Face::Face( unsigned int rIndex, VertexOfFace* setA, VertexOfFace* setB, VertexOfFace* setC )
     : FACEINITDEFAULTS {

	// Check for valid references.
	if( setA == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: NULL pointer for Vertex A given!" << endl;
	}
	if( setB == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: NULL pointer for Vertex B given!" << endl;
	}
	if( setC == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: NULL pointer for Vertex C given!" << endl;
	}

	// Check for valid class type.
    if( setA != nullptr && !setA->belongsToFace() ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: The class of vertex A is NOT VertexOfFace!" << endl;
	}
    if( setB != nullptr && !setB->belongsToFace() ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: The class of vertex B is NOT VertexOfFace!" << endl;
	}
    if( setC != nullptr && !setC->belongsToFace() ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: The class of vertex C is NOT VertexOfFace!" << endl;
	}

	// defaults:
	NORMAL_X = _NOT_A_NUMBER_DBL_;
	NORMAL_Y = _NOT_A_NUMBER_DBL_;
	NORMAL_Z = _NOT_A_NUMBER_DBL_;

	// neighbourfaces:
	FACE_NEIGHBOUR_AB = nullptr; // for edge A-B
	FACE_NEIGHBOUR_BC = nullptr; // for edge B-C
	FACE_NEIGHBOUR_CA = nullptr; // for edge C-A

	// Index
	mIndex = rIndex;

	// Vertices
	vertA = setA;
	vertB = setB;
	vertC = setC;

	// Check if vertices are equal => bad structure.
	if( ( vertA == vertB ) || ( vertB == vertC ) || ( vertC == vertA ) ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: degenerated face having identical vertices!" << endl;
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: " << vertA->getIndex() << " - " << vertB->getIndex() << "  " << vertC->getIndex() << "!" << endl;
	}

	// now we have to tell the vertices whom they belong to:
	vertA->connectToFace( this );
	vertB->connectToFace( this );
	vertC->connectToFace( this );

	// normal vector:
	getAreaNormal(); // will also set FLAG_NORMAL_SET
}

//! Destructor sets properties to not a number or NULL.
Face::~Face() {
	// disconnect from related or neighbouring primitives:
	//cout << "[Face] destroyed => disconnect from vertices." << endl;
	//! Removes all connections to adjacent vertices
	vertA->disconnectFace( this );
	vertB->disconnectFace( this );
	vertC->disconnectFace( this );
	// just in case we referer to an object still in the memory, but already destroyed:
	vertA = nullptr;
	vertB = nullptr;
	vertC = nullptr;

	//! Removes all connections to adjacent faces
	if( mNeighbourFaces == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: unexpected: mNeighbourFaces is NULL!" << endl;
		return;
	}
	for( unsigned short i=0; i<(mNeighbourFacesNonManifold+3); i++ ) {
		if( mNeighbourFaces[i] == nullptr ) {
			continue;
		}
		mNeighbourFaces[i]->disconnectFace( this );
		mNeighbourFaces[i] = nullptr;
	}
	// just in case:
	mNeighbourFacesNonManifold = 0;
	delete[] mNeighbourFaces;
	mNeighbourFaces = nullptr;
}

//! Returns the pointer to Vertex A.
Vertex* Face::getVertA() const {
	return vertA;
}

//! Returns the pointer to Vertex B.
Vertex* Face::getVertB() const {
	return vertB;
}

//! Returns the pointer to Vertex C.
Vertex* Face::getVertC() const {
	return vertC;
}

//! Returns the pointer to Vertex A.
VertexOfFace* Face::getVertAoF() {
	return vertA;
}

//! Returns the pointer to Vertex B.
VertexOfFace* Face::getVertBoF() {
	return vertB;
}

//! Returns the pointer to Vertex C.
VertexOfFace* Face::getVertCoF() {
	return vertC;
}

//! Returns the index of Vertex A.
unsigned int Face::getVertAIndex() {
	return vertA->getIndex();
}

//! Returns the index of Vertex B.
unsigned int Face::getVertBIndex() {
	return vertB->getIndex();
}

//! Returns the index of Vertex C.
unsigned int Face::getVertCIndex() {
	return vertC->getIndex();
}


//! Writes the (current) indices of the Vertices A, B and C to the given array, which has to be of size int[3] (or larger).
bool Face::copyFacePropsTo( sFaceProperties& faceProps ) const {
	faceProps.vertexIndices.resize(3);
	faceProps.vertexIndices.shrink_to_fit();
	faceProps.vertexIndices[0] = vertA->getIndex();
	faceProps.vertexIndices[1] = vertB->getIndex();
	faceProps.vertexIndices[2] = vertC->getIndex();

	faceProps.textureCoordinates.resize(6);
	faceProps.textureCoordinates.shrink_to_fit();

	std::copy(mUVs.begin(), mUVs.end(), faceProps.textureCoordinates.begin());

	faceProps.textureId = mTextureId;

	return( true );
}

//! Adds the mid-points of a specified edge as Vertex to a given list.
//! This method is used for sub-division of triangles. Especially for computing a (ico)sphere based on an Icosahedron.
//! @returns false in case of an error.
bool Face::getMidPoint( eEdgeNames rEdge, vector<VertexOfFace*>* rVertexList ) {
	Vertex* vert1;
	Vertex* vert2;
	switch( rEdge ) {
		case EDGE_NONE: // Nothing to do - even this call is strange, there is no error.
			return true;
			break;
		case EDGE_AB:
			vert1 = vertA;
			vert2 = vertB;
			break;
		case EDGE_BC:
			vert1 = vertB;
			vert2 = vertC;
			break;
		case EDGE_CA:
			vert1 = vertC;
			vert2 = vertA;
			break;
		case EDGE_NON_MANIFOLD: //! \todo implement for manifold edges, when required.
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: Not implemented!" << endl;
			return false;
			break;
		default: // We should never ever reach this point.
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: Unknown!" << endl;
			return false;
			break;
	}
	Vector3D midPoint = ( vert1->getPositionVector() + vert2->getPositionVector() ) / 2.0;
	rVertexList->push_back( new VertexOfFace( midPoint ) );
	return false;
}

//! Adds all vertices of this face to a given set.
//! @returns false in case of an error. True otherwise.
bool Face::getVertABC( set<VertexOfFace*>* rSomeVerts ) {
	bool retVal = true;
	rSomeVerts->insert( vertA );
	rSomeVerts->insert( vertB );
	rSomeVerts->insert( vertC );
	return retVal;
}

//! Adds all vertices of this face to a given set.
//! @returns false in case of an error. True otherwise.
bool Face::getVertABC( set<Vertex*>* rSomeVerts ) {
	bool retVal = true;
	rSomeVerts->insert( vertA );
	rSomeVerts->insert( vertB );
	rSomeVerts->insert( vertC );
	return retVal;
}

// Indexing -------------------------------------------------------------------

bool Face::setIndex( int rIndex ) {
	//! Setes the index of a Primitive. Additionally it preserves the very first
	//! index set. Returns false in case the index has not been set.
	mIndex = rIndex;
	return true;
}

int Face::getIndex() const {
	//! Retrieves the primitives index. Typcally used to write a mesh to a file.
	//!
	//! Attention: the returned index might be wrong due to alterations to the 
	//! the meshs structure or not set at all.
	return mIndex;
}

//! Returns the index of the Face for a bit array using an array of 64-bit long integers.
//! relates to Mesh::fetchSphereBitArray
void Face::getIndexOffsetBit(
                uint64_t* rBitOffset, //! Offset i.e. block of bits.
                uint64_t* rBitNr      //! Bit number within block.
) {
	*rBitOffset = mIndex / ( 8*sizeof( uint64_t ) );
	*rBitNr     = mIndex - (*rBitOffset)*8*sizeof( uint64_t );
}

void Face::getIndexOffsetBitEdge( int* bitOffset, int* bitNr, int edgeIdx ) {
	//! Returns the index of the edge of this Face for a bit array using an array of 64-bit long integers.
	//! relates to the estimation of geodesic distances
	*bitOffset = 3 * ( mIndex / ( 8*sizeof( uint64_t ) ) );
	*bitNr     = 3 * ( mIndex - (*bitOffset)*8*sizeof( uint64_t ) );
	switch( edgeIdx ) {
		case EDGE_AB:
			// nothing to do.
			break;
		case EDGE_BC:
			(*bitNr)++;
			break;
		case EDGE_CA:
			(*bitNr) += 2;
			break;
		default:
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: invalid edgeIdx: " << edgeIdx << "!" << endl;
	}
}

bool Face::markVertsVisited( uint64_t* rVertBitArrayVisited ) {
	//! Sets the bit within a bit array for the faces vertices.
	vertA->markVisited( rVertBitArrayVisited );
	vertB->markVisited( rVertBitArrayVisited );
	vertC->markVisited( rVertBitArrayVisited );
	return true;
}

bool Face::addAndTagUntaggedVerts( vector<Vertex*>* rSomeVerts, uint64_t* rVertBitArrayVisited ) {
	//! Sets the bit of the faces vertices to visited and adds those not visited before to rSomeVerts.
	if( !vertA->markVisited( rVertBitArrayVisited ) ) {
		rSomeVerts->push_back( vertA );
	}
	if( !vertB->markVisited( rVertBitArrayVisited ) ) {
		rSomeVerts->push_back( vertB );
	}
	if( !vertC->markVisited( rVertBitArrayVisited ) ) {
		rSomeVerts->push_back( vertC );
	}
	return true;
}

// Information retrival ---------------------------------------------------------------------------

double Face::getX() const {
	//! Returns the x-coordinate of the Primitive's center of gravity.
	//! Return not-a-number in case of an error.
	double localCogX = vertA->getX();
	localCogX += vertB->getX();
	localCogX += vertC->getX();
	localCogX /= 3.0;
	return localCogX;
}

double Face::getY() const {
	//! Returns the y-coordinate of the Primitive's center of gravity.
	//! Return not-a-number in case of an error.
	double localCogY = vertA->getY();
	localCogY += vertB->getY();
	localCogY += vertC->getY();
	localCogY /= 3.0;
	return localCogY;
}

double Face::getZ() const {
	//! Returns the z-coordinate of the Primitive's center of gravity.
	//! Return not-a-number in case of an error.
	double localCogZ = vertA->getZ();
	localCogZ += vertB->getZ();
	localCogZ += vertC->getZ();
	localCogZ /= 3.0;
	return localCogZ;
}

double Face::getNormalX() {
	//! Returns the x-coordinate of the Primitive's normal.
	//! Return not-a-number in case of an error.
	return NORMAL_X;
}

double Face::getNormalY() {
	//! Returns the y-coordinate of the Primitive's normal.
	//! Return not-a-number in case of an error.
	return NORMAL_Y;
}

double Face::getNormalZ() {
	//! Returns the z-coordinate of the Primitive's normal.
	//! Return not-a-number in case of an error.
	return NORMAL_Z;
}

bool Face::copyNormalXYZTo( float* rNormalXYZ, bool rNormalized ) {
	//! Copies the x-, y- and z-elements of the normal vector to a given array, which has to be at least of size 3.
	//! Normal vector is either taken from an array, when set or from the 1-ring neighbourhood.
	if( !getFlag( FLAG_NORMAL_SET ) ) {
		getAreaNormal();
	}
	return Primitive::copyNormalXYZTo( rNormalXYZ, rNormalized );
}

bool Face::copyNormalXYZTo( double* rNormalXYZ, bool rNormalized ) {
	//! Copies the x-, y- and z-elements of the normal vector to a given array and its length set to setLen.
	//! The given array has to be at least of size 3.
	//! Returns false in case of an error.
	if( !getFlag( FLAG_NORMAL_SET ) ) {
		getAreaNormal();
	}
	return Primitive::copyNormalXYZTo( rNormalXYZ, rNormalized );
}

// Colors -------------------------------------------------------------------------------------------------------------------

bool Face::copyRGBTo( unsigned char* rColorRGB ) {
	//! Stub for future use of color per Face, which will increase memory usage.
	//! When removed, Primitive::copyRGBTo will returned undefined values and lots of error messages.
	rColorRGB[0] = 187;
	rColorRGB[1] = 187;
	rColorRGB[2] = 187;
	//! Still returns false as it is kind of an error calling this method in its current state.
	return false;
}

bool Face::copyRGBATo( unsigned char* rColorRGBA ) {
	//! Stub for future use of color per Face, which will increase memory usage.
	//! When removed, Primitive::copyRGBTo will returned undefined values and lots of error messages.
	rColorRGBA[0] = 187;
	rColorRGBA[1] = 187;
	rColorRGBA[2] = 187;
	rColorRGBA[3] = 255;
	//! Still returns false as it is kind of an error calling this method in its current state.
	return false;
}

bool Face::getMaxAngle( double* rMaxAngle ) {
	//! Returns the maximum angle of the triangle (stumpfwinkliges Dreieck).
	//! Returns false in case of an error.
	double lengthEdgeA = getLengthBC();
	double lengthEdgeB = getLengthCA();
	double lengthEdgeC = getLengthAB();
	double alpha = acos( ( lengthEdgeB*lengthEdgeB + lengthEdgeC*lengthEdgeC - lengthEdgeA*lengthEdgeA ) / ( 2.0*lengthEdgeB*lengthEdgeC ) );
	double beta  = acos( ( lengthEdgeA*lengthEdgeA + lengthEdgeC*lengthEdgeC - lengthEdgeB*lengthEdgeB ) / ( 2.0*lengthEdgeA*lengthEdgeC ) );
	double gamma = acos( ( lengthEdgeA*lengthEdgeA + lengthEdgeB*lengthEdgeB - lengthEdgeC*lengthEdgeC ) / ( 2.0*lengthEdgeA*lengthEdgeB ) );
	(*rMaxAngle) = alpha;
	if( (*rMaxAngle) < beta ) {
		(*rMaxAngle) = beta;
	}
	if( (*rMaxAngle) < gamma ) {
		(*rMaxAngle) = gamma;
	}
	return true;
}

bool Face::getMinAngle( double* rMaxAngle ) {
	//! Returns the minimum angle of the triangle (spitzwinkliges Dreieck).
	//! Returns false in case of an error.
	double lengthEdgeA = getLengthBC();
	double lengthEdgeB = getLengthCA();
	double lengthEdgeC = getLengthAB();
	double alpha = acos( ( lengthEdgeB*lengthEdgeB + lengthEdgeC*lengthEdgeC - lengthEdgeA*lengthEdgeA ) / ( 2.0*lengthEdgeB*lengthEdgeC ) );
	double beta  = acos( ( lengthEdgeA*lengthEdgeA + lengthEdgeC*lengthEdgeC - lengthEdgeB*lengthEdgeB ) / ( 2.0*lengthEdgeA*lengthEdgeC ) );
	double gamma = acos( ( lengthEdgeA*lengthEdgeA + lengthEdgeB*lengthEdgeB - lengthEdgeC*lengthEdgeC ) / ( 2.0*lengthEdgeA*lengthEdgeB ) );
	(*rMaxAngle) = alpha;
	if( (*rMaxAngle) > beta ) {
		(*rMaxAngle) = beta;
	}
	if( (*rMaxAngle) > gamma ) {
		(*rMaxAngle) = gamma;
	}
	return true;
}

// Function value -------------------------------------------------------------------

bool Face::setFuncValue( double setVal ) {
	//! Method to set the function value.
	//! Returns false in case of an error (or not implemented).
	mFuncValue = setVal;
	return true;
}

//! Method to retrieve the function value of this Face.
//! Note: There is another method to retrieve the average function value of the face's vertices.
//! See: Face::getFuncValVertAvg
//!
//! @returns false in case of an error (or not implemented).
inline bool Face::getFuncValue( double* rSetVal ) const {
	*rSetVal = mFuncValue;
	return true;
}

// Comparison of function value - Heap/sorting related -------------------------------

bool Face::funcValLower( Face* rPrim1, Face* rPrim2 ) {
	//! Used for sorting e.g. a heap.
	//! Returns false, when one of the vertices has no function value.
	double funcVal1;
	if( !rPrim1->getFuncValue( &funcVal1 ) ) {
		cerr << "[Vertex::funcValLower] no function value!" << endl;
		return false;
	}
	double funcVal2;
	if( !rPrim2->getFuncValue( &funcVal2 ) ) {
		cerr << "[Vertex::funcValLower] no function value!" << endl;
		return false;
	}

	if( funcVal1 < funcVal2 ) {
		// when both short distances are of equal length, we have to sort by the longer distance:
		return true;
	}
	return false;
}

bool Face::funcValLowerLabled( Face* rPrim1, Face* rPrim2 ) {
	//! Used for sorting e.g. a heap.
	//! Sorts only labled.
	//! Returns false, when one of the vertices has no function value.
	if( (rPrim1->isLabled()) && (!rPrim2->isLabled()) ){
		return true;
	}

	double funcVal1;
	if( !rPrim1->getFuncValue( &funcVal1 ) ) {
		cerr << "[Vertex::funcValLower] no function value!" << endl;
		return false;
	}
	double funcVal2;
	if( !rPrim2->getFuncValue( &funcVal2 ) ) {
		cerr << "[Vertex::funcValLower] no function value!" << endl;
		return false;
	}

	if( funcVal1 < funcVal2 ) {
		// when both short distances are of equal length, we have to sort by the longer distance:
		return true;
	}
	return false;
}

bool Face::sortLabelsFirst( Face* rPrim1, Face* rPrim2 ) {
	//! Used for sorting e.g. a heap.
	if( ( rPrim1->isLabled() ) && ( !(rPrim2->isLabled()) ) ) {
		return true;
	}
	return false;
}

bool Face::sortByIndex( Face* rPrim1, Face* rPrim2 ) {
	//! Used for sorting by Index.
	if( rPrim1->getIndex() < rPrim2->getIndex() ) {
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------------------

//! Compute the angle next to a given Vertex reference
//! in radiant.
//!
//! @returns zero if the vertex is not part of the triangle.
//!          Otherwise the angle is returned in radiant.
double Face::getAngleAtVertex( const Vertex* vertABC ) const {
	// Determine the vertex
	if( vertABC == vertA ) {
		return estimateAlpha();
	}
	if( vertABC == vertB ) {
		return estimateBeta();
	}
	if( vertABC == vertC ) {
		return estimateGamma();
	}

	// No matching vertex found:
	cerr << "[Face::" << __FUNCTION__ << "] Vertex " << vertABC->getIndex()
	     << " is not part of face [" << getIndex() << "]." << endl;
	dumpFaceInfo();
	return( 0.0 );
}

//! Determine if the given vertex is part of this face.
//!
//! @returns true if vertexRequired matches A, B or C.
bool Face::requiresVertex(
		const Vertex *vertexRequired
) const {
	return vertA == vertexRequired ||
	       vertB == vertexRequired ||
	       vertC == vertexRequired;
}

//! Determine if one or more of the given vertices is part of this face.
//!
//! @returns true if A, B or C is within the list (set).
bool Face::requiresOneOrMoreVerticesOf(
		const std::set<Vertex*>& rVertexList
) const {
	if( rVertexList.find( vertA ) != rVertexList.end() ) {
		return( true );
	}
	if( rVertexList.find( vertB ) != rVertexList.end() ) {
		return( true );
	}
	if( rVertexList.find( vertC ) != rVertexList.end() ) {
		return( true );
	}
	return( false );
}

//! Minimum Distance of the Distances of A, B and C to a given position.
bool Face::getMinDistTo( double const rSomePos[3], double* rDist ) {
	vertA->getDistanceFromCenterOfGravityTo( rSomePos, rDist );
	double tmpDist;
	vertB->getDistanceFromCenterOfGravityTo( rSomePos, &tmpDist );
	if( tmpDist < (*rDist ) ) {
		(*rDist ) = tmpDist;
	}
	vertC->getDistanceFromCenterOfGravityTo( rSomePos, &tmpDist );
	if( tmpDist < (*rDist ) ) {
		(*rDist ) = tmpDist;
	}
	return true;
}

//! Minimum Distance of the Distances of A, B and C to (x,y,z).
double Face::getMinDistTo( double x, double y, double z ) {
	double dVert = 0.0;
	double dMin = vertA->estDistanceTo( x, y, z );

	dVert = vertB->estDistanceTo( x, y, z );
	if( dVert < dMin ) {
		dMin = dVert;
	}
	dVert = vertC->estDistanceTo( x, y, z );
	if( dVert < dMin ) {
		dMin = dVert;
	}

	return dMin;
}
double Face::getMaxDistTo( double x, double y, double z ) {
	//! Maximum Distance of the Distances of A, B and C to (x,y,z).
	double dVert = 0.0;
	double dMax = vertA->estDistanceTo( x, y, z );

	dVert = vertB->estDistanceTo( x, y, z );
	if( dVert > dMax ) {
		dMax = dVert;
	}
	dVert = vertC->estDistanceTo( x, y, z );
	if( dVert > dMax ) {
		dMax = dVert;
	}

	return dMax;
}

double Face::isInRange( double maxDist, double x, double y, double z ) {
	//! Checks if A, B and C are ALL in range (maxDist) of (x,y,z)
	if( maxDist > getMaxDistTo( x, y, z ) ) {
		return true;
	}
	return false;
}

double Face::isInRangeParts( double maxDist, double x, double y, double z ) {
	//! Checks if A, B or C are in range (maxDist) of (x,y,z)
	if( maxDist > getMinDistTo( x, y, z ) ) {
		return true;
	}
	return false;
}

double Face::isInRangeParts( double maxDist, double* someXYZ ) {
	//! Checks if A, B or C are in range (maxDist) of someXYZ[0], someXYZ[1], someXYZ[2].
	//! someXYZ has to be of length 3.
	//!
	//! This method should be the most efficent one - compared to its overloaded brothers.
	double distToA = vertA->distanceToCoord( someXYZ );
	//cout << __PRETTY_FUNCTION__ << " distToA: " << distToA << endl;
	if( distToA <= maxDist ) {
		return true;
	}
	double distToB = vertB->distanceToCoord( someXYZ );
	//cout << __PRETTY_FUNCTION__ << " distToB: " << distToB << endl;
	if( distToB <= maxDist ) {
		return true;
	}
	double distToC = vertC->distanceToCoord( someXYZ );
	//cout << __PRETTY_FUNCTION__ << " distToC: " << distToC << endl;
	if( distToC <= maxDist ) {
		return true;
	}
	return false;
}

int Face::verticesInRange( double maxDist, double* someXYZ ) {
	//! Counts if A, B or C are in range (maxDist) of someXYZ[0], someXYZ[1], someXYZ[2].
	//! someXYZ has to be of length 3.
	int inRange = 0;
	double distToABC = vertA->distanceToCoord( someXYZ );
	//cout << __PRETTY_FUNCTION__ << " distToA: " << distToABC << endl;
	if( distToABC <= maxDist ) {
		inRange++;
	}
	distToABC = vertB->distanceToCoord( someXYZ );
	//cout << __PRETTY_FUNCTION__ << " distToB: " << distToABC << endl;
	if( distToABC <= maxDist ) {
		inRange++;
	}
	distToABC = vertC->distanceToCoord( someXYZ );
	//cout << __PRETTY_FUNCTION__ << " distToC: " << distToABC << endl;
	if( distToABC <= maxDist ) {
		inRange++;
	}
	return inRange;
}

//! Returns the edge lengths of A-B, B-C and C-A to rLenABC, which has to be of length 3.
//!
//! @returns false in case of an error. True otherwise.
bool Face::getEdgeLengths( double* rLenABC ) {
	// Sanity check:
	if( rLenABC == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return( false );
	}
	rLenABC[0] = ( vertA->getPositionVector() - vertB->getPositionVector() ).getLength3();
	rLenABC[1] = ( vertB->getPositionVector() - vertC->getPositionVector() ).getLength3();
	rLenABC[2] = ( vertC->getPositionVector() - vertA->getPositionVector() ).getLength3();
	return( true );
}

//! Returns the altitudes A-BC, B-CA and C-AB to rAlitudesABC, which has to be of length 3.
//!
//! @returns false in case of an error. True otherwise.
bool Face::getAltitudes( double* rAlitudesABC ) {
	// Sanity check:
	if( rAlitudesABC == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return( false );
	}
	rAlitudesABC[0] = getAltitudeToA();
	rAlitudesABC[1] = getAltitudeToB();
	rAlitudesABC[2] = getAltitudeToC();
	return( true );
}

//! Returns the angles at A, B and C to rAnglesABC, which has to be of length 3.
//!
//! @returns false in case of an error. True otherwise.
bool Face::getAngles( double* rAnglesABC ) {
	// Sanity check:
	if( rAnglesABC == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return( false );
	}
	rAnglesABC[0] = estimateAlpha();
	rAnglesABC[1] = estimateBeta();
	rAnglesABC[2] = estimateGamma();
	return( true );
}

// Bounding box ------------------------------------------------------------------------------------------------------------------------------------------------

double Face::getMaxX() {
	//! bounding-box related: maximum x
	double maxX =  vertA->getX();
	if( maxX < vertB->getX() ) {
		maxX = vertB->getX();
	}
	if( maxX < vertC->getX() ) {
		maxX = vertC->getX();
	}
	return maxX;
}

double Face::getMinX() {
	//! bounding-box related: minimum x
	double minX =  vertA->getX();
	if( minX > vertB->getX() ) {
		minX = vertB->getX();
	}
	if( minX > vertC->getX() ) {
		minX = vertC->getX();
	}
	return minX;
}

double Face::getMaxY() {
	//! bounding-box related: maximum y
	double maxY =  vertA->getY();
	if( maxY < vertB->getY() ) {
		maxY = vertB->getY();
	}
	if( maxY < vertC->getY() ) {
		maxY = vertC->getY();
	}
	return maxY;
}

double Face::getMinY() {
	//! bounding-box related: minimum y
	double minY =  vertA->getY();
	if( minY > vertB->getY() ) {
		minY = vertB->getY();
	}
	if( minY > vertC->getY() ) {
		minY = vertC->getY();
	}
	return minY;
}

double Face::getMaxZ() {
	//! bounding-box related: maximum z
	double maxZ =  vertA->getZ();
	if( maxZ < vertB->getZ() ) {
		maxZ = vertB->getZ();
	}
	if( maxZ < vertC->getZ() ) {
		maxZ = vertC->getZ();
	}
	return maxZ;
}

double Face::getMinZ() {
	//! bounding-box related: minimum z
	double minZ =  vertA->getZ();
	if( minZ > vertB->getZ() ) {
		minZ = vertB->getZ();
	}
	if( minZ > vertC->getZ() ) {
		minZ = vertC->getZ();
	}
	return minZ;
}

Vertex* Face::getVertWithMinX() {
	//! Returns the reference to a Vertex with the minimum distance to the 
	//! yz-plane.
	Vertex* getVertMinX = vertA;
	if( getVertMinX->getX() > vertB->getX() ) {
		getVertMinX = vertB;
	} 
	if( getVertMinX->getX() > vertC->getX() ) {
		getVertMinX = vertC;
	} 
	return getVertMinX;
}

Vertex* Face::getVertWithMaxX() {
	//! Returns the reference to a Vertex with the maximum distance to the 
	//! yz-plane.
	Vertex* getVertMaxX = vertA;
	if( getVertMaxX->getX() < vertB->getX() ) {
		getVertMaxX = vertB;
	} 
	if( getVertMaxX->getX() < vertC->getX() ) {
		getVertMaxX = vertC;
	} 
	return getVertMaxX;
}

Vertex* Face::getVertWithMinY() {
	//! Returns the reference to a Vertex with the minimum distance to the 
	//! xz-plane.
	Vertex* getVertMinY = vertA;
	if( getVertMinY->getY() > vertB->getY() ) {
		getVertMinY = vertB;
	} 
	if( getVertMinY->getY() > vertC->getY() ) {
		getVertMinY = vertC;
	} 
	return getVertMinY;
}

Vertex* Face::getVertWithMaxY() {
	//! Returns the reference to a Vertex with the maximum distance to the 
	//! xz-plane.
	Vertex* getVertMaxY = vertA;
	if( getVertMaxY->getY() < vertB->getY() ) {
		getVertMaxY = vertB;
	} 
	if( getVertMaxY->getY() < vertC->getY() ) {
		getVertMaxY = vertC;
	} 
	return getVertMaxY;
}

Vertex* Face::getVertWithMinZ() {
	//! Returns the reference to a Vertex with the minimum distance to the 
	//! xy-plane.
	Vertex* getVertMinZ = vertA;
	if( getVertMinZ->getZ() > vertB->getZ() ) {
		getVertMinZ = vertB;
	} 
	if( getVertMinZ->getZ() > vertC->getZ() ) {
		getVertMinZ = vertC;
	} 
	return getVertMinZ;
}

Vertex* Face::getVertWithMaxZ() {
	//! Returns the reference to a Vertex with the maximum distance to the 
	//! xy-plane.
	Vertex* getVertMaxZ = vertA;
	if( getVertMaxZ->getZ() < vertB->getZ() ) {
		getVertMaxZ = vertB;
	} 
	if( getVertMaxZ->getZ() < vertC->getZ() ) {
		getVertMaxZ = vertC;
	} 
	return getVertMaxZ;
}

//! Determine the longest edge length of this triangle.
//! @returns the length of the longest edge.
double Face::getEdgeLenMax() {
	//! \todo improve - as done in Face::getEdgeLenMin
	double edgeLenMax = getLengthAB();
	if( getLengthBC() > edgeLenMax ) {
		edgeLenMax = getLengthBC();
	}
	if( getLengthCA() > edgeLenMax ) {
		edgeLenMax = getLengthCA();
	}
	return edgeLenMax;
}

//! Determine the shortest edge length of this triangle.
//!
//! @returns false in case of an error i.e. null-pointer given
//!          or degenerated triangle. True otherwise.
bool Face::getEdgeLenMin( double* rLenMin ) {
	// Sanity check:
	if( rLenMin == nullptr ) {
		return( false );
	}

	// Compute shortest edge:
	double edgeLenMin = getLengthAB();
	if( getLengthBC() < edgeLenMin ) {
		edgeLenMin = getLengthBC();
	}
	if( getLengthCA() < edgeLenMin ) {
		edgeLenMin = getLengthCA();
	}

	// Do not return values from degenerated triangles:
	if( !isnormal( edgeLenMin ) ) {
		return( false );
	}

	// Everything checks out - return the value:
	(*rLenMin) = edgeLenMin;
	return( true );
}

//! Determine the minimal altitude of this triangle.
//!
//! @returns false in case of an error i.e. null-pointer given
//!          or degenerated triangle. True otherwise.
bool Face::getAltitudeMin( double* rAltitudeMin ) {
	// Sanity check:
	if( rAltitudeMin == nullptr ) {
		return( false );
	}

	// Compute shortest edge:
	double altMin = getAltitudeToA();
	if( getAltitudeToB() < altMin ) {
		altMin = getAltitudeToB();
	}
	if( getAltitudeToC() < altMin ) {
		altMin = getAltitudeToC();
	}

	// Do not return values from degenerated triangles:
	if( !isnormal( altMin ) ) {
		return( false );
	}

	// Everything checks out - return the value:
	(*rAltitudeMin) = altMin;
	return( true );
}


//! Retrieve the type (=class) of this object as an id.
//!
//! @returns the class id.
int Face::getType() {
	return Primitive::IS_FACE;
}



//compute triangle area
double triarea(Vector3D* a, Vector3D* b, Vector3D* c ){

	//area of triangle (algorithm copied from Gigamesh code)
	double vBAx, vBAy, vBAz; // Vector AB
	double vCAx, vCAy, vCAz; // Vector CA

	vBAx =	b->getX()-a->getX();	//vertB->getX() - vertA->getX();
	vBAy =	b->getY()-a->getY();	//vertB->getY() - vertA->getY();
	vBAz =	b->getZ()-a->getZ();	//vertB->getZ() - vertA->getZ();

	vCAx =	c->getX()-a->getX();	//vertC->getX() - vertA->getX();
	vCAy =	c->getY()-a->getY();	//vertC->getY() - vertA->getY();
	vCAz =	c->getZ()-a->getZ();	//vertC->getZ() - vertA->getZ();

	double NORMALarea_X, NORMALarea_Y, NORMALarea_Z;
	NORMALarea_X = ( ( vBAy * vCAz ) - ( vBAz * vCAy ) );
	NORMALarea_Y = ( ( vBAz * vCAx ) - ( vBAx * vCAz ) );
	NORMALarea_Z = ( ( vBAx * vCAy ) - ( vBAy * vCAx ) );

	// we return the estimated area:
	return sqrt( ( NORMALarea_X * NORMALarea_X ) + ( NORMALarea_Y * NORMALarea_Y ) + ( NORMALarea_Z * NORMALarea_Z ) ) / 2.0;

}


//! Line-Sphere Intersection:
//! @returns lineSphereCases -- case type of intersection, e.g. tangent, line only, etc.
//! and the line coordinates of intersection in s1,s2
//! quadratic equation from "numerical recipes in c"
eLineSphereCases lineSphereIntersect( double    rRadius, //!< Radius of the sphere B(r).
                                      Vector3D& rCenter, //!< Center of the sphere B(r).
                                      Vector3D& rEA,     //!< Edge's position of vertex e_A.
                                      Vector3D& rEB,     //!< Edge's position of vertex e_B.
                                      Vector3D* s1,      //!< Point of intersection, only set when it exists.
                                      Vector3D* s2       //!< Point of intersection, only set when it exists.
	) {
	// see http://paulbourke.net/geometry/sphereline/

	double aq = (rEB.getX()-rEA.getX())*(rEB.getX()-rEA.getX())+(rEB.getY()-rEA.getY())*(rEB.getY()-rEA.getY()) + (rEB.getZ()-rEA.getZ())*(rEB.getZ()-rEA.getZ());
	double bq = 2.0 * ((rEB.getX()-rEA.getX())*(rEA.getX()-rCenter.getX())+(rEB.getY()-rEA.getY())*(rEA.getY()-rCenter.getY())+(rEB.getZ()-rEA.getZ())*(rEA.getZ()-rCenter.getZ()));
	double cq = -(rRadius*rRadius) + rCenter.getX()*rCenter.getX() + rCenter.getY()*rCenter.getY() + rCenter.getZ()*rCenter.getZ() + rEA.getX()*rEA.getX() + rEA.getY()*rEA.getY() + rEA.getZ()*rEA.getZ() - 2.0*(rCenter.getX()*rEA.getX() + rCenter.getY()*rEA.getY() + rCenter.getZ()*rEA.getZ());

	// Term to compute the root:
	double h = bq*bq-4*aq*cq;
	// The line does not intersect the sphere - so the edge has also no intersection:
	if( h<0 ) {
		return LSI_NO_INTERSECT_LINE;
	}
	// Tangent - means that t1 == t2, so we have to treat only one case:
	if( h==0 ) {
		double t12 = -0.5*bq/aq;
		// Compute tangent point:
		s1->set( ( ( rEB -rEA ) * t12 ) + rEA );
		s2->set( (*s1) );
		if( ( 0.0 <= t12 ) && ( t12 <= 1.0 ) ) {
			// Tangent point is within the edge's intervall of [0,1]:
			return LSI_TANGENT_EDGE;
		}
		// Tangent point is not in [0,1]:
		return LSI_TANGENT_LINE;
	}
	// The remaining cases:
	double t1 = ( 0.5*( -bq -sqrt(h) ) ) / aq;
	double t2 = ( 0.5*( -bq +sqrt(h) ) ) / aq;
	//cout << "t1: " << t1 << endl;
	//cout << "t2: " << t2 << endl;
	s1->set( ( ( rEB -rEA ) * t1 ) + rEA );
	s2->set( ( ( rEB -rEA ) * t2 ) + rEA );
	bool s1int = ( 0.0 <= t1 ) && ( t1 <= 1.0 );
	bool s2int = ( 0.0 <= t2 ) && ( t2 <= 1.0 );
	if( s1int && s2int ) {
		return LSI_TWO_INTERSECT;
	}
	if( s1int ) {
		return LSI_ONE_INTERSECT_P1;
	}
	if( s2int ) {
		return LSI_ONE_INTERSECT_P2;
	}
	return LSI_NO_INTERSECT_EDGE;
/*
	double qq = -0.5*(bq+ sign(bq)*sqrt(h));

	t1 = qq/aq;
	t2 = cq/qq;

	if ((t1<=1) && (t1>=0) && (t2>=0) && (t2<=1)){
		if(t1==t2){
			s1->set( a->getX()+ t1*(b->getX()-a->getX()),
				 a->getY()+ t1*(b->getY()-a->getY()),
				 a->getZ()+ t1*(b->getZ()-a->getZ()),
			         1.0);
			return LSI_ONE_INTERSECT;
		}

		if(t1>t2){
			#ifdef DEBUG1
			cout<<"Function "<<__FUNCTION__<<" has reordered "<<endl;
			#endif
			s2->set( a->getX()+ t1*(b->getX()-a->getX()),
				 a->getY()+ t1*(b->getY()-a->getY()),
				 a->getZ()+ t1*(b->getZ()-a->getZ()),
			        1.0);
			s1->set( a->getX()+ t2*(b->getX()-a->getX()),
				 a->getY()+ t2*(b->getY()-a->getY()),
				 a->getZ()+ t2*(b->getZ()-a->getZ()),
			        1.0 );
			return LSI_TWO_INTERSECT;
		}
		s1->set( a->getX()+ t1*(b->getX()-a->getX()),
			 a->getY()+ t1*(b->getY()-a->getY()),
			 a->getZ()+ t1*(b->getZ()-a->getZ()),
		        1.0);
		s2->set( a->getX()+ t2*(b->getX()-a->getX()),
			 a->getY()+ t2*(b->getY()-a->getY()),
			 a->getZ()+ t2*(b->getZ()-a->getZ()),
		         1.0 );
		return LSI_TWO_INTERSECT;
		}
	if((t1<=1) && (t1>=0)){
		s1->set( a->getX()+ t1*(b->getX()-a->getX()),
			 a->getY()+ t1*(b->getY()-a->getY()),
			 a->getZ()+ t1*(b->getZ()-a->getZ()),
			 1.0);
		return LSI_ONE_INTERSECT;
	}
	if((t2>=0) && (t2<=1)){
		s1->set( a->getX()+ t2*(b->getX()-a->getX()),
			 a->getY()+ t2*(b->getY()-a->getY()),
			 a->getZ()+ t2*(b->getZ()-a->getZ()),
		         1.0);
		return LSI_ONE_INTERSECT;
	}
	 return LSI_NO_INTERSECT;
*/
}


/*
double pointtoplane(Face* tri, Vertex* rseed){
    double dist1;

 //   Vector3D n = tri->getNormal(true);
    tri->getDistanceToPoint(&(rseed->getPositionVector()), &dist1);

    return dist1;
}
*/

//http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
bool AlmostEqual2sComplement(double A, double B, int maxUlps){

	int aInt = *reinterpret_cast<int*>(&A);

    // Make aInt lexicographically ordered as a twos-complement int
    if (aInt < 0){
	    aInt = 0x8000000000000000 - aInt;
    }
    // Make bInt lexicographically ordered as a twos-complement int
	int bInt = *reinterpret_cast<int*>(&B);

    if (bInt < 0){
        bInt = 0x8000000000000000 - bInt;
     }

    int intDiff = abs(aInt - bInt);

    if (intDiff <= maxUlps){
        return true;
    }
    return false;
}

//calculate angle alpha which is spanned by the 2 lines from the center  to s1 and s2
double anglearea(double r, Vector3D* s1, Vector3D* s2){
	//checks to avoid NAN from asin
	double h = distanceVV( s1, s2 ) / (2.0*r);
	if(h>1.0){
		if( AlmostEqual2sComplement(h, 1.0, 5)){
			h = 1.0;
		}
	}
	if(h<-1.0){
		if( AlmostEqual2sComplement(h, -1.0, 5)){
			h = -1.0;
		}
	}
	return( 2.0 * asin(h) );
}

//compute area of circle segment
double Face::csecarea(double r, Vector3D* s1, Vector3D* s2, Vertex* rseed){
    double r2;
    Vector3D temp = rseed->getPositionVector();
    getDistanceToPoint(&temp, &r2);

    if(r>r2){
        r2 = sqrt(r*r - r2*r2);
//        cout<<"radius: "<<r <<"radius2: "<<r2<<endl;
        return fabs( 0.5*r2*r2*(anglearea(r2, s1, s2) - sin(anglearea(r2, s1, s2))) );
    }
	// Error case:
	cerr << "[Face::" << __FUNCTION__ << "] ERROR: negative sqrt in csecarea in Face No. " << getIndex() << endl;
	return( 0.0 );
}


//compute area of circle (resulting from the intersection plane of a triangle with a sphere)
/*double carea(Vertex* rseed, double r){
	double r2;
	getDistanceToPoint(&(rseed->getPositionVector()), &r2);
	if(r>r2){
	    r2 = sqrt(r*r - r2*r2);
    //        cout<<"radius: "<<r <<"radius2: "<<r2<<endl;
	    return M_PI * r2 * r2;
	}
	cout<<"ERROR negative sqrt in csecarea"<<endl;
	return 0.0;
}*/


//test if point is in triangle
//http://www.blackpawn.com/texts/pointinpoly/default.html
bool pointintriangle(Vector3D* p, Vector3D* a, Vector3D* b, Vector3D* c){

	// Compute vectors
	Vector3D v0 = (*c) -(*a);
	Vector3D v1 = (*b) -(*a);
	Vector3D v2 = (*p) -(*a);

	// Compute dot products
	double dot00 = dot3(v0, v0);
	double dot01 = dot3(v0, v1);
	double dot02 = dot3(v0, v2);
	double dot11 = dot3(v1, v1);
	double dot12 = dot3(v1, v2);

	double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;


	return ((u > 0) && (v > 0) && (u + v < 1));
}





//check if p is in triangle or on border of triangle
bool Face::pointontriangle(const Vector3D* p) const {

	// Compute vectors
	Vector3D v0 = (vertC->getPositionVector()) -(vertA->getPositionVector());
	Vector3D v1 = (vertB->getPositionVector()) -(vertA->getPositionVector());
	Vector3D v2 = (*p) -(vertA->getPositionVector());

	// Compute dot products
	double dot00 = dot3(v0, v0);
	double dot01 = dot3(v0, v1);
	double dot02 = dot3(v0, v2);
	double dot11 = dot3(v1, v1);
	double dot12 = dot3(v1, v2);

	double invDenom = 1. / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;


	return ((u >= 0) && (v >= 0) && (u + v <= 1));
}





//check if p is in triangle
bool Face::pointintriangle(Vector3D* p){

	// Compute vectors
	Vector3D v0 = (vertC->getPositionVector()) -(vertA->getPositionVector());
	Vector3D v1 = (vertB->getPositionVector()) -(vertA->getPositionVector());
	Vector3D v2 = (*p) -(vertA->getPositionVector());

	// Compute dot products
	double dot00 = dot3(v0, v0);
	double dot01 = dot3(v0, v1);
	double dot02 = dot3(v0, v2);
	double dot11 = dot3(v1, v1);
	double dot12 = dot3(v1, v2);

	double invDenom = 1. / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;


	return ((u > 0) && (v > 0) && (u + v < 1));
}

//check if p is inside triangle and has a small distance to the border
bool Face::pointininnertriangle(Vector3D* p){

	// Compute vectors
	Vector3D v0 = (vertC->getPositionVector()) -(vertA->getPositionVector());
	Vector3D v1 = (vertB->getPositionVector()) -(vertA->getPositionVector());
	Vector3D v2 = (*p) -(vertA->getPositionVector());

	// Compute dot products
	double dot00 = dot3(v0, v0);
	double dot01 = dot3(v0, v1);
	double dot02 = dot3(v0, v2);
	double dot11 = dot3(v1, v1);
	double dot12 = dot3(v1, v2);

	double invDenom = 1. / (dot00 * dot11 - dot01 * dot01);
	double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	double v = (dot00 * dot12 - dot01 * dot02) * invDenom;


	return ((u > 3*DBL_EPSILON) && (v > 3*DBL_EPSILON) && (u + v + 6*DBL_EPSILON < 1. ));
}




//! Computes the Surface Integral Invariant type A (II.SA)
bool Face::surfaceintegralinvariant( int nradii, double* radii, double* area, Vertex* rseed1 ) {

	Vector3D rseed = rseed1->getPositionVector();
	Vector3D A = vertA->getPositionVector();
	Vector3D B = vertB->getPositionVector();
	Vector3D C = vertC->getPositionVector();
	double dista = distanceVV( &A, &rseed );
	double distb = distanceVV( &B, &rseed );
	double distc = distanceVV( &C, &rseed );
	double dist1;
	bool flagexecution=true;
	int i;
	//for debug
	//double temparea=area[0];
	for( i=0; i < nradii; i++ ) {
		//! maybe a better compare operator/function can be used here
		if ( ( dista <= radii[i] ) && ( distb <= radii[i] ) && ( distc <= radii[i] ) ) {
			area[i] += this->getAreaNormal();
			continue;
		}
		//getDistanceToPoint computation needs only executed once
		if ( flagexecution == true ) {
			getDistanceToPoint(&rseed, &dist1);
			dist1 = fabs(dist1);
			flagexecution=false;
		}
		if ( dist1 > radii[i] ) {
			continue;
		}
		//compute points of intersection
		Vector3D s1, s2, s3, s4, s5, s6;
		eLineSphereCases iab = lineSphereIntersect( radii[i], rseed, A, B, &s1, &s2 );
		eLineSphereCases ibc = lineSphereIntersect( radii[i], rseed, B, C, &s3, &s4 );
		eLineSphereCases ica = lineSphereIntersect( radii[i], rseed, C, A, &s5, &s6 );

		//no intersecion with edges, but intersection with triangle
		if ( (iab == LSI_NO_INTERSECT_LINE && ibc==LSI_NO_INTERSECT_LINE && ica==LSI_NO_INTERSECT_LINE) || (iab == LSI_NO_INTERSECT_LINE && ibc==LSI_NO_INTERSECT_LINE && ica==LSI_TANGENT_EDGE) || (iab == LSI_NO_INTERSECT_LINE && ibc==LSI_TANGENT_EDGE && ica==LSI_NO_INTERSECT_LINE) || (iab == LSI_TANGENT_EDGE && ibc==LSI_NO_INTERSECT_LINE && ica==LSI_NO_INTERSECT_LINE)
		                || (iab == LSI_TANGENT_EDGE && ibc==LSI_TANGENT_EDGE && ica==LSI_NO_INTERSECT_LINE) || (iab == LSI_NO_INTERSECT_LINE && ibc==LSI_TANGENT_EDGE && ica==LSI_TANGENT_EDGE) || (iab == LSI_TANGENT_EDGE && ibc==LSI_NO_INTERSECT_LINE && ica==LSI_TANGENT_EDGE) ) {
			double distn;

			Vector3D n = getNormal(true);
			Vector3D temp = rseed1->getPositionVector();
			getDistanceToPoint(&temp, &distn);
			Vector3D M= distn * n + &temp;

			//cout<<"radius : "<< r << " abstand:  "<<dist1 << endl;
			//plane intersect && pointintriangle(Vector3D* p, Vector3D* a, Vector3D* b, Vector3D* c){
			if( (fabs(distn) < radii[i]) && (pointintriangle(&M)) ){
				//cout<< "pointintri:  "<<pointintriangle(&M, &a, &b, &c)<<endl;
				double r2;
				r2 = sqrt(radii[i] * radii[i] - distn*distn);
				//cout<<"SecondRadius:  "<<r2<<endl;
				#ifdef DEBUG1
				cout<<__FUNCTION__<<"   a"<<M_PI*r2*r2<< "triarea  \t"<<this->estimateAreaNormal()<<endl;
				#endif
				area[i] += M_PI*r2*r2;
				continue;
			}
		}

		vector< pair<Vector3D*, eEdgeNames> > vec;

		if (dista <= radii[i]) {
			pair <Vector3D*, eEdgeNames> spA (&A, EDGE_AB);
			vec.push_back(spA);
		}
		#ifdef DEBUG1
		cout<<"vectors size a "<< vec.size()<<endl;
		#endif
		if( iab==LSI_ONE_INTERSECT_P1 || iab==LSI_ONE_INTERSECT_P2 || iab==LSI_TANGENT_EDGE ) {
			if( iab==LSI_ONE_INTERSECT_P1 || iab== LSI_TANGENT_EDGE) {
				pair <Vector3D*, eEdgeNames> sp1 (&s1, EDGE_AB);
				if( distanceVV( &s1, &A ) > DBL_EPSILON ) {
					vec.push_back(sp1);
				}
			}
			if( iab==LSI_ONE_INTERSECT_P2 ) {
				pair <Vector3D*, eEdgeNames> sp1 (&s2, EDGE_AB);
				if( distanceVV( &s2, &A ) > DBL_EPSILON ) {
					vec.push_back(sp1);
				}
			}
		}


		if( iab==LSI_TWO_INTERSECT ) {
			pair <Vector3D*, eEdgeNames> sp1 (&s1, EDGE_AB);
			pair <Vector3D*, eEdgeNames> sp2 (&s2, EDGE_AB);
			if( ( distanceVV( &s1, &A ) > DBL_EPSILON ) ) {
				vec.push_back(sp1);
				vec.push_back(sp2);
			}
			else {
				vec.push_back(sp2);
			}
		}
		if (distb <= radii[i]) {
			pair <Vector3D*, eEdgeNames> spB (&B, EDGE_AB);
				vec.push_back(spB);
		}
		#ifdef DEBUG1
		cout<<"vectors size ab "<< vec.size()<<endl;
		#endif
		if (distb <= radii[i]) {
			pair <Vector3D*, eEdgeNames> spB (&B, EDGE_BC);
				vec.push_back(spB);
		}

		if( ibc==LSI_ONE_INTERSECT_P1 || ibc==LSI_ONE_INTERSECT_P2 || ibc==LSI_TANGENT_EDGE ) {
			if( ibc==LSI_ONE_INTERSECT_P1 || ibc == LSI_TANGENT_EDGE) {
				pair <Vector3D*, eEdgeNames> sp3 (&s3, EDGE_BC);
				if( distanceVV( &B, &s3 ) > DBL_EPSILON ) {
					vec.push_back(sp3);
				}
			}
			if( ibc==LSI_ONE_INTERSECT_P2 ) {
				pair <Vector3D*, eEdgeNames> sp3 (&s4, EDGE_BC);
				if( distanceVV( &B, &s4 ) > DBL_EPSILON ) {
					vec.push_back(sp3);
				}
			}
		}


		#ifdef DEBUG1
		cout<<"vectors size bc "<< vec.size()<<endl;
		#endif
		if( ibc==LSI_TWO_INTERSECT ) {
			pair <Vector3D*, eEdgeNames> sp3 (&s3, EDGE_BC);
			pair <Vector3D*, eEdgeNames> sp4 (&s4, EDGE_BC);
			if( ( distanceVV( &s3, &B ) > DBL_EPSILON ) ) {
				vec.push_back(sp3);
				vec.push_back(sp4);
			}
			else {
				vec.push_back(sp4);
			}
		}
		if (distc <= radii[i]) {
			pair <Vector3D*, eEdgeNames> spC (&C, EDGE_BC);
			vec.push_back(spC);
		}
		if (distc <= radii[i]) {
			pair <Vector3D*, eEdgeNames> spC (&C, EDGE_CA);
			vec.push_back(spC);
		}
		if( ica==LSI_ONE_INTERSECT_P1 || ica==LSI_ONE_INTERSECT_P2 || ica==LSI_TANGENT_EDGE ) {
			if( ica==LSI_ONE_INTERSECT_P1 || ica == LSI_TANGENT_EDGE) {
				pair <Vector3D*, eEdgeNames> sp5 (&s5, EDGE_CA);
				if( distanceVV( &C, &s5 ) > DBL_EPSILON ) {
					vec.push_back(sp5);
				}
			}
			if( ica==LSI_ONE_INTERSECT_P2 ) {
				pair <Vector3D*, eEdgeNames> sp5 (&s6, EDGE_CA);
				if( distanceVV( &C, &s6 ) > DBL_EPSILON ) {
					vec.push_back(sp5);
				}
			}
		}
		if( ica==LSI_TWO_INTERSECT ) {
			pair <Vector3D*, eEdgeNames> sp5 (&s5, EDGE_CA);
			pair <Vector3D*, eEdgeNames> sp6 (&s6, EDGE_CA);
			if( ( distanceVV( &s5, &C ) > DBL_EPSILON) ) {
				vec.push_back(sp5);
				vec.push_back(sp6);
			}
			else {
				vec.push_back(sp6);
			}
		}
		if (dista <= radii[i]) {
			pair <Vector3D*, eEdgeNames> spA (&A, EDGE_CA);
			vec.push_back(spA);
		}
		#ifdef DEBUG1
		cout<<"vectors size ca"<< vec.size()<<endl;
		#endif

		Vector3D cog(0.0, 0.0, 0.0);
//		for (vector< pair<Vector3D*, eEdgeNames> >::iterator it = vec.begin(); it!=vec.end(); ++it) {
//			cog = cog + it->first;
//		}

		for(const std::pair<Vector3D*, eEdgeNames>& edgeVector : vec)
		{
			cog = cog + edgeVector.first;
		}

		cog = cog / vec.size();

		/*cout<<"vectors size "<< vec.size()<<endl;
		cout<<"------------------------------------- "<<endl;
		for( int k = 0; k<vec.size(); k++ ) {
			vec[k].first->dumpInfo();
			cout<<vec[k].second<<endl;
		}
		cout<<"------------------------------------- "<<endl;*/
		if ( vec.size()<1 ) {
			continue;
		}
		if ( vec.size()<2 ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: wrong vertex list for SURFACE INTEGRAL INVARIANT in Face No. " << getIndex() << endl;
			continue;
		}
		if ( vec.size()==2 ) {
			area[i] += csecarea(radii[i], vec[0].first, vec[1].first, rseed1);
			continue;
		}

		unsigned int j;
		for( j=1; j<vec.size(); j++ ) {
			if( distanceVV( vec[j-1].first, vec[j].first ) > DBL_EPSILON ) {
				area[i] += triarea(vec[j-1].first, vec[j].first, &cog);
				#ifdef DEBUG1
				cout<<"for triarea: "<<j<<endl;
				#endif
				if ( vec[j-1].second != vec[j].second ) {
					area[i] += csecarea(radii[i], vec[j-1].first, vec[j].first, rseed1);
					#ifdef DEBUG1
					cout<<"for csec: "<<j<<endl;
					#endif
				}
			}
		}
		j=vec.size()-1;
		if( distanceVV( vec[0].first, vec[j].first ) > DBL_EPSILON ) {
			area[i] += triarea(vec[0].first, vec[j].first, &cog);
			if ( vec[0].second !=  vec[j].second ) {
				area[i] += csecarea(radii[i], vec[0].first, vec[j].first, rseed1);
				#ifdef DEBUG1
					cout<<"for csec:  +++"<<endl;
				#endif
			}
		}
	}
	//for debug
	/*if ( (area[0]-temparea)>0 ) {
		cout<<"Area: "<<area[0]-temparea<<"mIndex: "<<mIndex<<endl;
	}*/

	return true;
}

// Transformation ----------------------------------------------------------------------

//! Applies a given homogenous transformation matrix.
//!
//! @returns false in case of an error. True otherwise.
bool Face::applyTransfrom( Matrix4D* transMat ) {
	vertA->applyTransfrom( transMat );
	vertB->applyTransfrom( transMat );
	vertC->applyTransfrom( transMat );
	clearFlag( FLAG_NORMAL_SET );
	getAreaNormal();
	return true;
}

//! Changes the orientation of a face.
//!
//! @returns false in case of an error. True otherwise.
bool Face::applyReOrient() {
	VertexOfFace* tmpVert = vertA;
	vertA = vertB;
	vertB = tmpVert;
	clearFlag( FLAG_NORMAL_SET );
	getAreaNormal();
	return( true );
}

// Labeling ----------------------------------------------------------------------------

//! Sets the label of all Vertex A, B and C. Except for badly structured Meshs all three vertices
//! should always have the same label Nr.
bool Face::setLabel( uint64_t rSetLabelNr ) {
	bool labelingOK = true;
	if( !vertA->isLabled() ) {
		if( !vertA->setLabel( rSetLabelNr ) ) {
			labelingOK = false;
		}
	}
	if( !vertB->isLabled() ) {
		if( !vertB->setLabel( rSetLabelNr ) ) {
			labelingOK = false;
		}
	}
	if( !vertC->isLabled() ) {
		if( !vertC->setLabel( rSetLabelNr ) ) {
			labelingOK = false;
		}
	}
	bool retVal = false;
	if( labelingOK ) {
		retVal = Primitive::setLabel( rSetLabelNr );
	}
	return retVal;
}

bool Face::setLabelNone() {
	//! Sets the labling flag for the vertices to not labled.
	//! Returns false in case of an error (or not implemented).
	if( !vertA->setLabelNone() ) {
		return false;
	}
	if( !vertB->setLabelNone() ) {
		return false;
	}
	if( !vertC->setLabelNone() ) {
		return false;
	}
	return true;
}

//! Returns the label of Vertex A, which should generally be identically with Vertices B and C.
//! When no label is set or when the labels of A, B and C differ false is returned.
bool Face::getLabel( uint64_t& rGetLabelNr ) const {
	uint64_t labelA;
	uint64_t labelB;
	uint64_t labelC;
	if( !vertA->getLabel( labelA ) ) {
		return false;
	}
	if( !vertB->getLabel( labelB ) ) {
		return false;
	}
	if( !vertC->getLabel( labelC ) ) {
		return false;
	}
	if( ( labelA != labelB ) || ( labelA != labelC ) ) {
		return false;
	}
	rGetLabelNr = labelA;
	return true;
}

//! Returns true ONLY when all vertices have the same label!
bool Face::isLabled() const {
	uint64_t labelA;
	uint64_t labelB;
	uint64_t labelC;
	if( !vertA->getLabel( labelA ) ) {
		return false;
	}
	if( !vertB->getLabel( labelB ) ) {
		return false;
	}
	if( !vertC->getLabel( labelC ) ) {
		return false;
	}
	if( ( labelA != labelB ) || ( labelA != labelC ) ) {
		return false;
	}
	return true;
}

bool Face::isLabelBackGround() {
	//! Returns true, when of the vertices Vertex::isNoLabel returns true.
	if( vertA->isLabelBackGround() ) {
		return true;
	}
	if( vertB->isLabelBackGround() ) {
		return true;
	}
	if( vertC->isLabelBackGround() ) {
		return true;
	}
	return false;
}

//! Returns true, when two vertices have different labels.
//! Vertices with no label are ignored.
bool Face::isLabelMix() {
	uint64_t labelA;
	uint64_t labelB;
	uint64_t labelC;
	if( ( vertA->getLabel( labelA ) ) && ( !vertB->getLabel( labelB ) ) ) {
		if( labelA != labelB ) {
			return true;
		}
	}
	if( ( vertB->getLabel( labelB ) ) && ( !vertC->getLabel( labelC ) ) ) {
		if( labelB != labelC ) {
			return true;
		}
	}
	if( ( vertC->getLabel( labelC ) ) && ( !vertA->getLabel( labelA ) ) ) {
		if( labelC != labelA ) {
			return true;
		}
	}
	return false;
}

//! Test face for being at a corner between three voronoi cells.
//!
//! Corners between two labels and the background label will
//! be added.
//!
//! @returns true, when three vertices have different labels.
bool Face::isLabelVoronoiCorner() {
    uint64_t labelA = 0;
    uint64_t labelB = 0;
    uint64_t labelC = 0;
	bool isLabeldA = vertA->getLabel( labelA );
	bool isLabeldB = vertB->getLabel( labelB );
	bool isLabeldC = vertC->getLabel( labelC );

	// Three unlabled is no corner
	if( !isLabeldA && !isLabeldB && !isLabeldC ) {
		return( false );
	}
	// Two unlabled is no corner
	if( !isLabeldA && !isLabeldB ) {
		return( false );
	}
	if( !isLabeldB && !isLabeldC ) {
		return( false );
	}
	if( !isLabeldC && !isLabeldA ) {
		return( false );
	}

	// All three have to be different. One label can be 0 i.e. not labled.
	// This will add corners between two labels and the background label.
	if( ( labelA != labelB ) &&
	    ( labelB != labelC ) &&
	    ( labelC != labelA ) ) {
		return( true );
	}

	return( false );
}


bool Face::getLabelLines( set<labelLine*>* labelLineCollection ) {
	//! Adds edges (AB, BC, CA) to the given collection, when the neighbouring face is of
	//! another label or the edge is a border in general (no neighbour).
	//! Does ignore non-manifold Faces!
	uint64_t currentLabel;
	if( labelLineCollection == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: labelLineCollection is NULL!" << endl;
		return false;
	}
	if( !getLabel( currentLabel ) ) {
		cerr << "[Face::" << __FUNCTION__ << "] no label set!" << endl;
		return false;
	}
	uint64_t nextLabel;

    if( ( FACE_NEIGHBOUR_AB == nullptr ) || ( !FACE_NEIGHBOUR_AB->getLabel( nextLabel ) ) || ( nextLabel != currentLabel ) ) {
        labelLine* lineAB = new labelLine();
        //labelLine lineAB;
        lineAB->vertA      = vertA;
        lineAB->vertB      = vertB;
        lineAB->mFromFace  = this;
        lineAB->mFromEdge  = EDGE_AB;
        lineAB->labelNr    = currentLabel;
        labelLineCollection->insert( lineAB );
	}

    if( ( FACE_NEIGHBOUR_BC == nullptr ) || ( !FACE_NEIGHBOUR_BC->getLabel( nextLabel ) ) || ( nextLabel != currentLabel ) ) {
        labelLine* lineBC = new labelLine();
        lineBC->vertA      = vertB;
        lineBC->vertB      = vertC;
        lineBC->mFromFace  = this;
        lineBC->mFromEdge  = EDGE_BC;
        lineBC->labelNr    = currentLabel;
        labelLineCollection->insert( lineBC );
	}
    if( ( FACE_NEIGHBOUR_CA == nullptr ) || ( !FACE_NEIGHBOUR_CA->getLabel( nextLabel ) ) || ( nextLabel != currentLabel )) {
        //labelLine* lineCA = new labelLine();
        labelLine *lineCA = new labelLine();
        lineCA->vertA      = vertC;
        lineCA->vertB      = vertA;
        lineCA->mFromFace  = this;
        lineCA->mFromEdge  = EDGE_CA;
        lineCA->labelNr    = currentLabel;
        labelLineCollection->insert(lineCA );
	}

    return true;
}

// Labeling using Vertex labels - specific for Faces! -----------------------------------------------------

//! Returns true, when all vertices of the face belong to the same label.
//! In this case the label id will be written to getLabelNr.
//! Returns false otherwise - also when all vertices are tagged as "no labled".
//! To use this method to detect label bordes, you will have to use vertLabelNoLabel().
//!
//! @returns true, when all vertices of the face belong to the same label.
bool Face::vertLabelGet( uint64_t& rGetLabelNr ) const {
	uint64_t labelA;
	uint64_t labelB;
	uint64_t labelC;
	if( !vertA->getLabel( labelA ) ) {
		// no label =>
		return( false );
	}
	if( !vertB->getLabel( labelB ) ) {
		// no label =>
		return( false );
	}
	if( !vertC->getLabel( labelC ) ) {
		// no label =>
		return( false );
	}
	if( ( labelA != labelB ) || ( labelB != labelC ) || ( labelC != labelA ) ) {
		// mixed labels =>
		return( false );
	}
	rGetLabelNr = labelA;
	return( true );
}

bool Face::vertLabelBackGround() {
	//! Returns true, when all vertices of this face are tagged "no label".
	//! Returns true otherwise.
	//! Typically used to find and visualize "no label" areas.
	if( !vertA->isLabelBackGround() ) {
		return false;
	}
	if( !vertB->isLabelBackGround() ) {
		return false;
	}
	if( !vertC->isLabelBackGround() ) {
		return false;
	}
	return true;
}

bool Face::vertLabelLabelBorder( bool* isLabelLabelBorder, int* labelFromBorder ) {
	//! Returns true, when the Face is on a border.
	//! In case the face is on the border the flag 
	//! isLabelLabelBorder will be set to true, when it is a border between two labels.
	//! isLabelLabelBorder will be set to false, when it is a border between a label and a "no label" area.
	//! AND: labelFromBorder will be set.
	//! Remark: This method does NOT cover the case of three different labels!
	uint64_t labelA = _NOT_A_NUMBER_UINT_;
	uint64_t labelB = _NOT_A_NUMBER_UINT_;
	uint64_t labelC = _NOT_A_NUMBER_UINT_;
	uint64_t labelABC = _NOT_A_NUMBER_UINT_;
	uint64_t noLabelCount = 0;
	if( !vertA->getLabel( labelA ) ) {
		noLabelCount++;
	} else {
		// we make sure that we fetch some prober label for labelFromBorder - in case we need it.
		labelABC = labelA;
	}
	if( !vertB->getLabel( labelB ) ) {
		noLabelCount++;
	} else {
		// we make sure that we fetch some prober label for labelFromBorder - in case we need it.
		labelABC = labelB;
	}
	if( !vertC->getLabel( labelC ) ) {
		noLabelCount++;
	} else {
		// we make sure that we fetch some prober label for labelFromBorder - in case we need it.
		labelABC = labelC;
	}
	if( noLabelCount == 3 ) {
		// The face is completly part of a "no label" area.
		// equal vertLabelNoLabel
		return false;
	}
	if( ( labelA != labelB ) || ( labelB != labelC ) || ( labelC != labelA ) ) {
		// Definitly on a border, but we have to decide on what kind of border:
		if( noLabelCount == 0 ) {
			(*isLabelLabelBorder) = true;
			// Note: here we do not destinquish between the case of a | or Y type of border.
			// To do so, we have to check if all three vertex labels are different.
		} else {
			(*isLabelLabelBorder) = false;
			// Note: here we do not destinquish between the case of a | or Y type of border.
			// if isLabelLabelBorder == 2 then we have a | border.
			// if isLabelLabelBorder == 1 we have to check if the other 2 vertices have the same label.
			(*labelFromBorder) = labelABC;
		}
		return true;
	}
	// at this point we know for sure, that all three vertices belong to the same label
	return false;
}

// Function value related ---------------------------------------------------------------------------------

//! Computes the average of all three function values.
//! @returns false in case of an error. True otherwise.
bool Face::getFuncValVertAvg( double* rFuncValVertAvg ) {
	double funcValA;
	double funcValB;
	double funcValC;
	if( !vertA->getFuncValue( &funcValA ) ) {
		return false;
	}
	if( !vertB->getFuncValue( &funcValB ) ) {
		return false;
	}
	if( !vertC->getFuncValue( &funcValC ) ) {
		return false;
	}
	(*rFuncValVertAvg) = ( funcValA + funcValB + funcValC ) / 3.0;
	return true;
}

//! Computes the function values for the 1/3 of the face next to
//! the 1-ring center defined by the given Vertex.
//!
//! @returns false in case of an error. True otherwise.
bool Face::getFuncVal1RingThird( Vertex* rVert1RingCenter, double rNormDist, double* rFuncVal1RingThird ) {
	//! \todo remove this method as it is deprecated i.e. replaced by Face::getFuncVal1RingSector
	double funcValA;
	double funcValB;
	double funcValC;
	if( !vertA->getFuncValue( &funcValA ) ) {
		return false;
	}
	if( !vertB->getFuncValue( &funcValB ) ) {
		return false;
	}
	if( !vertC->getFuncValue( &funcValC ) ) {
		return false;
	}
	double funcVal1RingCenter;
	rVert1RingCenter->getFuncValue( &funcVal1RingCenter );
	if( vertA != rVert1RingCenter ) {
		funcValA = rNormDist * ( funcValA - funcVal1RingCenter ) / ( distanceVV( rVert1RingCenter, vertA ) );
		funcValA += funcVal1RingCenter;
	}
	if( vertB != rVert1RingCenter ) {
		funcValB = rNormDist * ( funcValB - funcVal1RingCenter ) / ( distanceVV( rVert1RingCenter, vertB ) );
		funcValB += funcVal1RingCenter;
	}
	if( vertC != rVert1RingCenter ) {
		funcValC = rNormDist * ( funcValC - funcVal1RingCenter ) / ( distanceVV( rVert1RingCenter, vertC ) );
		funcValC += funcVal1RingCenter;
	}
	(*rFuncVal1RingThird) = ( funcValA + funcValB + funcValC ) / 3.0;
	return true;
}

//! Pre-Computes values for a circle sector to compute a mean
//! or weigthed median for e.g. the function value or feature vector elements.
//!
//! See also: VertexOfFace::funcValMeanOneRing (calling method)
//!           Mesh::funcVertMedianOneRing (superior calling method)
//!
//! @returns false in case of an error. True otherwise.
bool Face::get1RingSectorConst(
                const Vertex*        rVert1RingCenter,
                const double&        rNormDist,
                s1RingSectorPrecomp& r1RingSecPre
) {
	// Sanity check I
	if( rVert1RingCenter == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return( false );
	}

	// Fetch angle
	double alpha = getAngleAtVertex( rVert1RingCenter );
	// Sanity check II
	if( !isfinite( alpha ) || abs( alpha ) >= M_PI ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Invalid angle of " << alpha << "!" << endl;
		return( false );
	}

	// Area - https://en.wikipedia.org/wiki/Circular_sector#Area
	r1RingSecPre.mSectorArea = rNormDist * rNormDist * alpha / 2.0; // As alpha is already in radiant.

	// Truncated prism - function value equals the height
	if( !getOposingVertices( rVert1RingCenter, r1RingSecPre.mVertOppA, r1RingSecPre.mVertOppB ) ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Finding opposing vertices!" << endl;
		return( false );
	}

	// Function values interpolated f'_i and f'_{i+1}
	// Compute the third angle using alpha/2.0 and 90°:
	double beta = ( M_PI - alpha ) / 2.0;
	// Law of sines
	double diameterCircum = rNormDist / sin( beta ); // Constant ratio equal longest edge
	// Distances for interpolation
	double lenCenterToA = distanceVV( rVert1RingCenter, r1RingSecPre.mVertOppA );
	double lenCenterToB = distanceVV( rVert1RingCenter, r1RingSecPre.mVertOppB );
	r1RingSecPre.mRatioCA = diameterCircum / lenCenterToA;
	r1RingSecPre.mRatioCB = diameterCircum / lenCenterToB;
	// Circle segment, center of gravity - https://de.wikipedia.org/wiki/Geometrischer_Schwerpunkt#Kreisausschnitt
	r1RingSecPre.mCenterOfGravityDist = ( 2.0 * sin( alpha ) ) / ( 3.0 * alpha );

	return( true );
}

//! Computes a circle sector and a mean function value of
//! the corresponding prism at the center of gravity.
//!
//! See also: VertexOfFace::funcValMeanOneRing (calling method)
//!           Mesh::funcVertMedianOneRing (superior calling method)
//!
//! @returns false in case of an error. True otherwise.
bool Face::getFuncVal1RingSector(
                const Vertex* rVert1RingCenter, //!< Center of the 1-ring (p_0)
                const double& rNormDist,        //!< Filtersize/-radius typically the shortest edge within the mesh (Delta_r_min)
                double&       rSectorArea,      //!< Return value: size of the geodesic sector, which is a circle sector at p_0.
                double&       rSectorFuncVal    //!< Function value at the center of gravity of the sector
) {
	// Sanity check
	if( rVert1RingCenter == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return( false );
	}

	// Values to be pre-computed
	s1RingSectorPrecomp oneRingSecPre;
	if( !get1RingSectorConst( rVert1RingCenter, rNormDist,
	                          oneRingSecPre ) ) {
		return( false );
	}

	// Fetch function values
	double funcValCenter;
	double funcValA;
	double funcValB;
	rVert1RingCenter->getFuncValue( &funcValCenter );
	oneRingSecPre.mVertOppA->getFuncValue( &funcValA );
	oneRingSecPre.mVertOppB->getFuncValue( &funcValB );

	// Interpolate
	double funcValInterpolA = funcValCenter*(1.0-oneRingSecPre.mRatioCA) + funcValA*oneRingSecPre.mRatioCA;
	double funcValInterpolB = funcValCenter*(1.0-oneRingSecPre.mRatioCB) + funcValB*oneRingSecPre.mRatioCB;

	// Compute average function value at the center of gravity of the circle sector
	rSectorFuncVal = funcValCenter*( 1.0 - oneRingSecPre.mCenterOfGravityDist ) +
	                 ( funcValInterpolA + funcValInterpolB ) * oneRingSecPre.mCenterOfGravityDist / 2.0;
	// Pass thru
	rSectorArea = oneRingSecPre.mSectorArea;
	return( true );
}

//! Vague: Assuming the Face lies on the threshold for the Vertices function values,
//! the vertex between the edge and the opposite vertex is estimated.
//! Returns true if the vertices could be estimated.
bool Face::getInterpolVertFuncVal( Vertex** rNewVert, Face** rFromFace, eEdgeNames rEdgeIdx, double rFuncValThres ) {
	(*rNewVert) = nullptr;
	
	Face* neighbourFace = getNeighbourFace( rEdgeIdx );
	if( neighbourFace == nullptr ) {
		return false;
	}
	(*rFromFace) = neighbourFace;

	Vertex* vert1;
	Vertex* vert2;
	switch( rEdgeIdx ) {
		case EDGE_NONE:
		case EDGE_NON_MANIFOLD:
			cerr << "[Face::getInterpolVertFuncVal] invalid edge!" << endl;
			return false;
			break;
		case EDGE_AB:
			vert1 = vertA;
			vert2 = vertB;
			break;
		case EDGE_BC:
			vert1 = vertB;
			vert2 = vertC;
			break;
		case EDGE_CA:
			vert1 = vertC;
			vert2 = vertA;
			break;
		default:
			cerr << "[Face::getInterpolVertFuncVal] undefined edge!" << endl;
			return false;
			break;
	}
	Vertex* vertOpposing = neighbourFace->getOposingVertex( vert1, vert2 );

	double funcVal1;
	double funcVal2;
	double funcValOpp;
	vert1->getFuncValue( &funcVal1 );
	vert2->getFuncValue( &funcVal2 );
	vertOpposing->getFuncValue( &funcValOpp );
	// Average - Middle of the edge: Function Value and coordinates
	double funcVal12 = ( funcVal1 + funcVal2 ) / 2.0;
	Vector3D vert12 = ( vert1->getPositionVector() + vert2->getPositionVector() ) / 2.0;
	// Edge/Directionvector acrros the Face:
	Vector3D dirV12Opp = vertOpposing->getPositionVector() - vert12;
	double scaleV12Opp = ( rFuncValThres - funcVal12 ) / ( funcValOpp - funcVal12 );
	// Interpolated position:
	Vector3D pos12 = vert12 + ( dirV12Opp * scaleV12Opp );
	// Vertex to be returned:
	(*rNewVert) = new Vertex( pos12, getNormal( true ) );

	return true;
}

//! Vague: Assuming the Face lies on the threshold for the Vertices function values,
//! the vertices along the edges are interpolated.
//! Returns true if the vertices could be estimated.
bool Face::getInterpolVertFuncVal( Vertex** rNewVertA, Vertex** rNewVertB, eEdgeNames rEdgeIdx, double rFuncValThres ) {

	(*rNewVertA) = nullptr;
	(*rNewVertB) = nullptr;
	
	Face* neighbourFace = getNeighbourFace( rEdgeIdx );
	if( neighbourFace == nullptr ) {
		return false;
	}

	Vertex* vert1;
	Vertex* vert2;
	switch( rEdgeIdx ) {
		case EDGE_NONE:
		case EDGE_NON_MANIFOLD:
			cerr << "[Face::getInterpolVertFuncVal] invalid edge!" << endl;
			return false;
			break;
		case EDGE_AB:
			vert1 = vertA;
			vert2 = vertB;
			break;
		case EDGE_BC:
			vert1 = vertB;
			vert2 = vertC;
			break;
		case EDGE_CA:
			vert1 = vertC;
			vert2 = vertA;
			break;
		default:
			cerr << "[Face::getInterpolVertFuncVal] undefined edge!" << endl;
			return false;
			break;
	}

	Vertex* vertOpposing = neighbourFace->getOposingVertex( vert1, vert2 );

	double funcVal1;
	double funcVal2;
	double funcValOpp;
	vert1->getFuncValue( &funcVal1 );
	vert2->getFuncValue( &funcVal2 );
	vertOpposing->getFuncValue( &funcValOpp );

	//cout << "[Face::getInterpolVertFuncVal] vertOpposingl:      " << vertOpposing->getIndex() << endl;
	//cout << "[Face::getInterpolVertFuncVal] funcValOpp funcVal: " << funcValOpp << endl;
	//cout << "[Face::getInterpolVertFuncVal] funcVal1 funcVal:   " << funcVal1 << endl;
	//cout << "[Face::getInterpolVertFuncVal] funcVal2 funcVal:   " << funcVal2 << endl;

	Vector3D dirV1Opp = vertOpposing->getPositionVector() - vert1->getPositionVector();
	Vector3D dirV2Opp = vertOpposing->getPositionVector() - vert2->getPositionVector();
	double scaleV1Opp = ( rFuncValThres - funcVal1 ) / ( funcValOpp - funcVal1 );
	double scaleV2Opp = ( rFuncValThres - funcVal2 ) / ( funcValOpp - funcVal2 );

	//cout << "[Face::getInterpolVertFuncVal] scale1 funcVal:   " << scaleV1Opp << endl;
	//cout << "[Face::getInterpolVertFuncVal] scale2 funcVal:   " << scaleV2Opp << endl;

	Vector3D pos1 = vert1->getPositionVector() + ( dirV1Opp * scaleV1Opp );
	Vector3D pos2 = vert2->getPositionVector() + ( dirV2Opp * scaleV2Opp );

	//getNormal( false ).dumpInfo( true, "nf" );
	(*rNewVertA) = new Vertex( pos2, getNormal( true ) );
	(*rNewVertB) = new Vertex( pos1, getNormal( true ) );

	return true;
}

double Face::getFuncValMinExcluding( Vertex* someVert ) {
	//! Returns the minimum function value of the vertices excluding a given vertex.
	//! Used to get minimum along a 1-ring excluding the center vertex.
	double minVal = +DBL_MAX;
	double valVert;
	if( vertA != someVert ) {
		vertA->getFuncValue( &valVert );
		if( valVert < minVal ) {
			minVal = valVert;
		}
	}
	if( vertB != someVert ) {
		vertB->getFuncValue( &valVert );
		if( valVert < minVal ) {
			minVal = valVert;
		}
	}
	if( vertC != someVert ) {
		vertC->getFuncValue( &valVert );
		if( valVert < minVal ) {
			minVal = valVert;
		}
	}
	return minVal;
}

double Face::getFuncValMaxExcluding( Vertex* someVert ) {
	//! Returns the minimum function value of the vertices excluding a given vertex.
	//! Used to get maxima along a 1-ring excluding the center vertex.
	//! Returns false in case of an error.
	double maxVal = -DBL_MAX;
	double valVert;
	if( vertA != someVert ) {
		vertA->getFuncValue( &valVert );
		if( valVert > maxVal ) {
			maxVal = valVert;
		}
	}
	if( vertB != someVert ) {
		vertB->getFuncValue( &valVert );
		if( valVert > maxVal ) {
			maxVal = valVert;
		}
	}
	if( vertC != someVert ) {
		vertC->getFuncValue( &valVert );
		if( valVert > maxVal ) {
			maxVal = valVert;
		}
	}
	return maxVal;
}

//! Adds vertices of the triangle to a given set, which belong to an edge of a ridge
//! of the vertices' function values.
//! @returns false in case of an error. True otherwise.
bool Face::getFuncValVertRidge( set<Vertex*>* rRidgeVerts ) {
	//cerr << "[Face::" << __FUNCTION__ << "] ERROR: Not yet implemented!" << endl;
	double epsThres = 2000.0*std::numeric_limits<double>::epsilon();
	double angleAlpha = estimateAlpha();
	double angleBeta  = estimateBeta();
	double angleGamma = estimateGamma();
	Face*   neighbFace  = nullptr;
	Vertex* oposingVert = nullptr;
	double funcVal1;
	double funcVal2;
	double funcValOpposingNeigh;
	double funcValOpposingThis;
	// Check edge A-B
	neighbFace = getNeighbourFace( EDGE_AB );
	if( neighbFace != nullptr ) { // A-B is not a border
		double neighAngle1 = neighbFace->getAngleAtVertex( vertA );
		double neighAngle2 = neighbFace->getAngleAtVertex( vertB );
		bool largeAngle = max( max( angleAlpha,angleBeta  ), max( neighAngle1, neighAngle2 ) ) > M_PI/2.0;
		//largeAngle = false;
		if( !largeAngle )  {
		oposingVert = neighbFace->getOposingVertex( vertA, vertB );
		vertA->getFuncValue( &funcVal1 );
		vertB->getFuncValue( &funcVal2 );
		vertC->getFuncValue( &funcValOpposingThis );
		oposingVert->getFuncValue( &funcValOpposingNeigh );
		if( min( funcValOpposingNeigh, funcValOpposingThis ) > ( epsThres + max( funcVal1, funcVal2 ) ) ) {
			rRidgeVerts->insert( vertA );
			rRidgeVerts->insert( vertB );
			//cout << "[Face::" << __FUNCTION__ << "] Added A+B" << endl;
		}
		}
	}
	/**/
	// Check edge B-C
	neighbFace = getNeighbourFace( EDGE_BC );
	if( neighbFace != nullptr ) { // B-C is not a border
		double neighAngle1 = neighbFace->getAngleAtVertex( vertB );
		double neighAngle2 = neighbFace->getAngleAtVertex( vertC );
		bool largeAngle = max( max( angleGamma,angleBeta  ), max( neighAngle1, neighAngle2 ) ) > M_PI/2.0;
		//largeAngle = false;
		if( !largeAngle )  {
		oposingVert = neighbFace->getOposingVertex( vertB, vertC );
		vertB->getFuncValue( &funcVal1 );
		vertC->getFuncValue( &funcVal2 );
		vertA->getFuncValue( &funcValOpposingThis );
		oposingVert->getFuncValue( &funcValOpposingNeigh );
		if( min( funcValOpposingNeigh, funcValOpposingThis ) > ( epsThres + max( funcVal1, funcVal2 ) ) ) {
			rRidgeVerts->insert( vertB );
			rRidgeVerts->insert( vertC );
			//cout << "[Face::" << __FUNCTION__ << "] Added B+C" << endl;
		}
		}
	}
	// Check edge C-A
	neighbFace = getNeighbourFace( EDGE_CA );
	if( neighbFace != nullptr ) { // C-A is not a border
		double neighAngle1 = neighbFace->getAngleAtVertex( vertC );
		double neighAngle2 = neighbFace->getAngleAtVertex( vertA );
		bool largeAngle = max( max( angleAlpha,angleGamma  ), max( neighAngle1, neighAngle2 ) ) > M_PI/2.0;
		//largeAngle = false;
		if( !largeAngle )  {
		oposingVert = neighbFace->getOposingVertex( vertC, vertA );
		vertC->getFuncValue( &funcVal1 );
		vertA->getFuncValue( &funcVal2 );
		vertB->getFuncValue( &funcValOpposingThis );
		oposingVert->getFuncValue( &funcValOpposingNeigh );
		if( min( funcValOpposingNeigh, funcValOpposingThis ) > ( epsThres + max( funcVal1, funcVal2 ) ) ) {
			rRidgeVerts->insert( vertC );
			rRidgeVerts->insert( vertA );
			//cout << "[Face::" << __FUNCTION__ << "] Added B+C" << endl;
		}
		}
	}
	/**/
	return true;
}

//! Checks if the Face lies on the isoline.
//! Show a warning, when the isoline intersects thru a vertex.
bool Face::isOnFuncValIsoLine( double isoThres ) {
	double funcValA = _NOT_A_NUMBER_DBL_;
	double funcValB = _NOT_A_NUMBER_DBL_;
	double funcValC = _NOT_A_NUMBER_DBL_;
	vertA->getFuncValue( &funcValA );
	funcValA -= isoThres;
	if( funcValA == 0.0 ) {
		LOG::debug() << "[Face::" << __FUNCTION__ << "] Warning: Iso Line intersects Vertex A.\n";
		return true;
	}
	vertB->getFuncValue( &funcValB );
	funcValB -= isoThres;
	if( funcValB == 0.0 ) {
		LOG::debug() << "[Face::" << __FUNCTION__ << "] Warning: Iso Line intersects Vertex B.\n";
		return true;
	}
	vertC->getFuncValue( &funcValC );
	funcValC -= isoThres;
	if( funcValC == 0.0 ) {
		LOG::debug() << "[Face::" << __FUNCTION__ << "] Warning: Iso Line intersects Vertex C.\n";
		return true;
	}

	return funcValA * funcValB <= 0.0 ||
	       funcValB * funcValC <= 0.0 ||
	       funcValC * funcValA <= 0.0;
}

//! Get next point to trace IsoLine
//! @param Isoline function value
//! @param isoPoint return value of the next point on the Polyline
//! @param faceNext return value of the next face to check. May be nullptr if there is no singular next face
//! @param searchForward determines the search direction
//! @param faceVisited If Isoline is on an edge, we set faceVisited to the other face on the edge to avoid duplicated polylines
//! @return true, if a valid next point was found. false otherwise
bool Face::getFuncValIsoPoint( double    isoThres,
	                       Vector3D* isoPoint,
	                       Face**    faceNext,
						   bool      searchForward,
						   Face**    faceVisited
) {
	//! Estimates the centroid shifted by function values - returned as isoPoint.
	double funcValA = _NOT_A_NUMBER_DBL_;
	double funcValB = _NOT_A_NUMBER_DBL_;
	double funcValC = _NOT_A_NUMBER_DBL_;
	vertA->getFuncValue( &funcValA );
	vertB->getFuncValue( &funcValB );
	vertC->getFuncValue( &funcValC );
	funcValA -= isoThres;
	funcValB -= isoThres;
	funcValC -= isoThres;

	*faceNext    = nullptr;
	*faceVisited = nullptr;

	//cases, where only the isoline touches the face in one or all points
	if((funcValA == 0.0 && funcValB == 0.0 && funcValC == 0.0) || //case all points are on isoValue
	   (funcValA == 0.0 && funcValB * funcValC > 0.0)          || //case A is on, but B and C have same sign
	   (funcValB == 0.0 && funcValC * funcValA > 0.0)          || //case B is on, but C and A have same sign
	   (funcValC == 0.0 && funcValA * funcValB > 0.0)             //case C is on, but A and B have same sign
	  )
	{
		LOG::debug() << "[Face::" << __FUNCTION__ << "] Isoline only touches face! Ignore this one.\n";
		return false;
	}

	//helper function to handle the cases, where one or two vertices are on the isovalue
	auto selectPointAndFace = [isoPoint, faceNext, faceVisited, this] (VertexOfFace* A, VertexOfFace* B, VertexOfFace* C, double fValB, double fValC, Face* nextFaceBC, Face* visitFaceAB, Face* visitFaceAC, bool searchForward) -> bool {
		if(searchForward)
		{
			isoPoint->set(A->getPositionVector());
			if(fValB != 0.0 && fValC != 0.0)	//if the other point is on a edge, we know the next face to check
				*faceNext = nextFaceBC;
			return true;
		}

		//backwards search cases, other point is on vertex
		if(fValB == 0.0)
		{
			isoPoint->set(B->getPositionVector());
			*faceVisited = visitFaceAB;
			return true;
		}
		if(fValC == 0.0)
		{
			isoPoint->set(C->getPositionVector());
			*faceVisited = visitFaceAC;
			return true;
		}

		//backwards search case, other point is on edge
		return this->getPointOnWeightedLine(isoPoint, B, C, 0.0, fValB, fValC);
	};

	if( funcValA == 0.0)
	{
		return selectPointAndFace(vertA, vertB, vertC, funcValB, funcValC, FACE_NEIGHBOUR_BC, FACE_NEIGHBOUR_AB, FACE_NEIGHBOUR_CA, searchForward);
	}

	if( funcValB == 0.0)
	{
		return selectPointAndFace(vertB, vertC, vertA, funcValC, funcValA, FACE_NEIGHBOUR_CA, FACE_NEIGHBOUR_BC, FACE_NEIGHBOUR_AB, searchForward);
	}

	if( funcValC == 0.0)
	{
		return selectPointAndFace(vertC, vertA, vertB, funcValA, funcValB, FACE_NEIGHBOUR_AB, FACE_NEIGHBOUR_CA, FACE_NEIGHBOUR_BC, !searchForward); //we need to invert searchForward here, because the AB-edge is the forwardintersection in this case, not the point
	}

	// Find the edge not intersected by the isoline:
	bool     isForwardIntersect;

	if( ( funcValA * funcValB ) < 0.0 ) {
		isForwardIntersect = ( funcValA < funcValB );
		if( searchForward == isForwardIntersect ) {
			(*faceNext) = FACE_NEIGHBOUR_AB;
			return getPointOnWeightedLine( isoPoint, vertA, vertB, 0.0, funcValA, funcValB );
		}
	}

	if( ( funcValB * funcValC ) < 0.0 ) {
		isForwardIntersect = ( funcValB < funcValC );
		if( searchForward == isForwardIntersect ) {
			(*faceNext) = FACE_NEIGHBOUR_BC;
			return getPointOnWeightedLine( isoPoint, vertB, vertC, 0.0, funcValB, funcValC );
		}
	}

	if( ( funcValC * funcValA ) < 0.0 ) {
		isForwardIntersect = ( funcValC < funcValA );
		if(  searchForward == isForwardIntersect ) {
			(*faceNext) = FACE_NEIGHBOUR_CA;
			return getPointOnWeightedLine( isoPoint, vertC, vertA, 0.0, funcValC, funcValA );
		}
	}

	// we should never reach this point, when the face is on the isoline
	LOG::debug() << "[Face::" << __FUNCTION__ << "] ERROR: Face NOT on an isoline!\n";
	return false;
}

//! Computes the feature vector's element values for the 1-ring sector
//! of the face.
//!
//! @returns false in case of an error. True otherwise.
bool Face::getFeatureVec1RingSector(
                const Vertex*                rVert1RingCenter,
                const s1RingSectorPrecomp&   r1RingSecPre,
                vector<double>&              rFeatureVec1RingSector
) const {
	// Sanity check
	if( rVert1RingCenter == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return( false );
	}

	// Fetch feature vectors of the face's vertices:
	uint64_t featureVecLength = rFeatureVec1RingSector.size();
	vector<double> featureVecA(featureVecLength,_NOT_A_NUMBER_DBL_);
	vector<double> featureVecB(featureVecLength,_NOT_A_NUMBER_DBL_);
	vector<double> featureVecCenter(featureVecLength,_NOT_A_NUMBER_DBL_); // Central point
	if( !r1RingSecPre.mVertOppA->copyFeatureVecTo( featureVecA.data() ) ) {
		return( false );
	}
	if( !r1RingSecPre.mVertOppB->copyFeatureVecTo( featureVecB.data() ) ) {
		return( false );
	}
	if( !rVert1RingCenter->copyFeatureVecTo( featureVecCenter.data() ) ) {
		return( false );
	}

	for( uint64_t i=0; i<featureVecLength; i++ ) {
		// Interpolate
		double featureVecInterpolA = featureVecCenter[i]*(1.0-r1RingSecPre.mRatioCA) + featureVecA[i]*r1RingSecPre.mRatioCA;
		double featureVecInterpolB = featureVecCenter[i]*(1.0-r1RingSecPre.mRatioCB) + featureVecB[i]*r1RingSecPre.mRatioCB;
		// Compute average function value at the center of gravity of the circle sector
		rFeatureVec1RingSector[i] = featureVecCenter[i]*( 1.0 - r1RingSecPre.mCenterOfGravityDist ) +
		                 ( featureVecInterpolA + featureVecInterpolB ) * r1RingSecPre.mCenterOfGravityDist / 2.0;
	}

	return( true );
}

// used for some mesh-checking -------------------------------------------------

//! Count the number of synthetic vertices of this face.
//! 
//! This function is for a face very usefull as face flags are not stored like those of the vertices.
//! Therefore holes filled with GigaMesh can be determined e.g. for removal.
//! 
//! @returns false in case of an error. True otherwise.
bool Face::hasSyntheticVertex(
                unsigned int& rIsSynthetic   //!< Number of vertices having the flag FLAG_SYNTHETIC set (return value)
) {
	// Reset counter
	rIsSynthetic = 0;
	// Check
	if( vertA->getFlag( FLAG_SYNTHETIC ) ) {
		rIsSynthetic++;
	}
	if( vertB->getFlag( FLAG_SYNTHETIC ) ) {
		rIsSynthetic++;
	}
	if( vertC->getFlag( FLAG_SYNTHETIC ) ) {
		rIsSynthetic++;
	}
	// Done
	return( true );
}

//! Count the number of border vertices of this face.
//!
//! Typically used for erosion of the mesh border before filling.
//!
//! @returns false in case of an error. True otherwise.
bool Face::hasBorderVertex(
                unsigned int& rIsBorder   //!< Number of vertices along a border (return value)
) {
	// Reset counter
	rIsBorder = 0;
	// Check
	if( vertA->isBorder() ) {
		rIsBorder++;
	}
	if( vertB->isBorder() ) {
		rIsBorder++;
	}
	if( vertC->isBorder() ) {
		rIsBorder++;
	}
	// Done
	return( true );
}

//! Count the number of border edges of this face.
//!
//! Typically used for erosion of the mesh border before filling.
//!
//! @returns false in case of an error. True otherwise.
bool Face::hasBorderEdges(
                unsigned int& rIsBorder   //!< Number of vertices along a border (return value)
) {
	// Reset counter
	rIsBorder = 0;
	if( FACE_NEIGHBOUR_AB == nullptr ) {
		rIsBorder++;
	}
	if( FACE_NEIGHBOUR_BC == nullptr ) {
		rIsBorder++;
	}
	if( FACE_NEIGHBOUR_CA == nullptr ) {
		rIsBorder++;
	}
	// Done
	return( true );
}


bool Face::isNonManifold() {	
	//! A Face is not manifold, when there are more than two neighbours for any Edge of the Face.
	//! Internally this is indicated by mNeighbourFacesNonManifold.
	if( mNeighbourFacesNonManifold > 0 ) {
		return true;
	}
	return false;
}

bool Face::isManifold() {
	//! Inverse of isNonManifold().
	return !isNonManifold();
}

bool Face::isBorder() {
	//! Returns true, when one or more sides of the Face is not neighbour to any other Face.
	//!
	//! In case we use the memory more efficent we might check for negative values of neighbourFaceCountNonManifold.
	if( FACE_NEIGHBOUR_AB == nullptr ) {
		return true;
	}
	if( FACE_NEIGHBOUR_BC == nullptr ) {
		return true;
	}
	if( FACE_NEIGHBOUR_CA == nullptr ) {
		return true;
	}
	return false;
}

bool Face::isSolo() {
	//! Returns true, when the Face is not neighbour to any other Face.
	if( ( FACE_NEIGHBOUR_AB == nullptr ) && ( FACE_NEIGHBOUR_BC == nullptr ) && ( FACE_NEIGHBOUR_CA == nullptr ) ) {
		return true;
	}
	return false;
}

//! Check if the face has an inverted orientation in comparison to
//! its adjacent neighbours.
//!
//! Faces having non-manifold egdes always match this criteria.
//!
//! @returns true, when one or more borders do not match the required orientation of a proper mesh.
//!          This method will also return true in case the face is not manifold!
bool Face::isInverse() {
	// Non-manifold faces always match the criteria.
	if( !isManifold() ) {
		return( true );
	}
	// Check the adjacent neighbours one by one, when present
	if( FACE_NEIGHBOUR_AB != nullptr ) {
		if( FACE_NEIGHBOUR_AB->isEdgeOrientationMatching( vertA, vertB ) ) {
			return( true );
		}
	}
	if( FACE_NEIGHBOUR_BC != nullptr ) {
		if( FACE_NEIGHBOUR_BC->isEdgeOrientationMatching( vertB, vertC ) ) {
			return( true );
		}
	}
	if( FACE_NEIGHBOUR_CA != nullptr ) {
		if( FACE_NEIGHBOUR_CA->isEdgeOrientationMatching( vertC, vertA ) ) {
			return( true );
		}
	}
	// Nothing found - so the face orientation is fine.
	return( false );
}

//! Check if the given vertex is part of an edge connecting faces with
//! an inverted orientation.
//!
//! This method will count some(!!!) of the non-manifold edges, depending
//! on the order of adjacent faces stored.
//!
//! \todo implement a method to determine a non-manifold edge to avoid
//! counting these edges.
//!
//! @returns true, when the criteria is matched. False otherwise.
bool Face::isInverseOnEdge( const VertexOfFace* rVert ) {
	// Check the adjacent neighbours one by one, when present
	if( ( rVert ==  vertA ) || ( rVert ==  vertB ) ) {
		if( FACE_NEIGHBOUR_AB != nullptr ) {
			if( FACE_NEIGHBOUR_AB->isEdgeOrientationMatching( vertA, vertB ) ) {
				return( true );
			}
		}
	}
	if( ( rVert ==  vertB ) || ( rVert ==  vertC ) ) {
		if( FACE_NEIGHBOUR_BC != nullptr ) {
			if( FACE_NEIGHBOUR_BC->isEdgeOrientationMatching( vertB, vertC ) ) {
				return( true );
			}
		}
	}
	if( ( rVert ==  vertC ) || ( rVert ==  vertA ) ) {
		if( FACE_NEIGHBOUR_CA != nullptr ) {
			if( FACE_NEIGHBOUR_CA->isEdgeOrientationMatching( vertC, vertA ) ) {
				return( true );
			}
		}
	}
	// Nothing found - so the face orientation is fine.
	return( false );
}

//! Check if one the face's edges has an edge with the same vertices and the same orientation.
//!
//! This method is used to determine inverted faces.
//! See Face::isInverse()
//!
//! @returns true, when an edge with the same vertices and the same orientation exists. False otherwise.
bool Face::isEdgeOrientationMatching(
                const VertexOfFace* rVert1, //!< The first vertex of the orientated edge.
                const VertexOfFace* rVert2  //!< The second vertex of the orientated edge.
) {
	// Sanity check
	if( ( rVert1 == nullptr ) || ( rVert2 == nullptr ) ) {
		cerr << "[Face" << __FUNCTION__ << "] ERROR: nullptr given!" << endl;
		return( false );
	}
	// Check edge A-B
	if( ( rVert1 == vertA ) && ( rVert2 == vertB ) ) {
		return( true );
	}
	// Check edge B-C
	if( ( rVert1 == vertB ) && ( rVert2 == vertC ) ) {
		return( true );
	}
	// Check edge C-A
	if( ( rVert1 == vertC ) && ( rVert2 == vertA ) ) {
		return( true );
	}
	// No edge of the same orientation found.
	return( false );
}

int Face::getState() {
	//! Returns the state (solo,border,manifold,non-manifold) of the Face.
	//!
	//! Attention: Double Cone is NOT considered by this method!
	if( isSolo() ) {
		return  _PRIMITIVE_STATE_SOLO_;
	}
	if( isBorder() ) {
		return  _PRIMITIVE_STATE_BORDER_;
	}
	if( isManifold() ) {
		return  _PRIMITIVE_STATE_MANIFOLD_;
	}
	if( isNonManifold() ) {
		return  _PRIMITIVE_STATE_NON_MANIFOLD_;
	}
	// we should never reach this point:
	cerr << "[Face] ERROR - unknown state for face " << getIndex() << "." << endl;
	return _PRIMITIVE_STATE_ERROR_;
}

// MESH-SETUP ------------------------------------------------------------------

void Face::reconnectToFaces() {
	//! Refreshs the connection of this face to its neighbouring faces via 1-ring neighbourhood.
	if( mNeighbourFaces != nullptr ) {
		delete[] mNeighbourFaces;
		mNeighbourFaces = new Face*[3];
		FACE_NEIGHBOUR_AB = nullptr; // for edge A-B
		FACE_NEIGHBOUR_BC = nullptr; // for edge B-C
		FACE_NEIGHBOUR_CA = nullptr; // for edge C-A
	}
	mNeighbourFacesNonManifold = 0;
	clearFlag( FLAG_FACE_STICKY );
	connectToFaces();
}

//! Connects this face to its neighbouring faces via 1-ring neighbourhood.
void Face::connectToFaces() {
	std::vector<Face*> neighbourCandidates;
	neighbourCandidates.reserve(vertA->get1RingFaceCount() + vertB->get1RingFaceCount() + vertC->get1RingFaceCount());
	vertA->getFaces( &neighbourCandidates );
	vertB->getFaces( &neighbourCandidates );
	vertC->getFaces( &neighbourCandidates );
	// Add face - either regular or non-manifold

	std::sort(neighbourCandidates.begin(), neighbourCandidates.end());

	neighbourCandidates.erase(
	            std::unique(neighbourCandidates.begin(), neighbourCandidates.end()) ,
	            neighbourCandidates.end()
	            );

	for(auto itFace=neighbourCandidates.begin(); itFace!=neighbourCandidates.end(); ++itFace ) {
		if( (*itFace) == this ) {
			continue;
		}
		bool faceIsConnected = false;
		bool faceAddedMultipleTimes = false; //! \todo pass on information
		const bool requireA = (*itFace)->requiresVertex(vertA);
		const bool requireB = (*itFace)->requiresVertex(vertB);
		const bool requireC = (*itFace)->requiresVertex(vertC);
		if( requireA && requireB ) {
			if( FACE_NEIGHBOUR_AB == nullptr ) {
				FACE_NEIGHBOUR_AB = (*itFace);
			} else {
				addNonManifold( (*itFace), &faceAddedMultipleTimes );
			}
			faceIsConnected = true;
		}
		if( requireB && requireC ) {
			if( FACE_NEIGHBOUR_BC == nullptr ) {
				FACE_NEIGHBOUR_BC = (*itFace);
			} else {
				addNonManifold( (*itFace), &faceAddedMultipleTimes );
			}
			faceIsConnected = true;
		}
		if( requireC && requireA ) {
			if( FACE_NEIGHBOUR_CA == nullptr ) {
				FACE_NEIGHBOUR_CA = (*itFace);
			} else {
				addNonManifold( (*itFace), &faceAddedMultipleTimes );
			}
			faceIsConnected = true;
		}
		// As we fetched definitly more faces, we skip the rest (otherwise neighCount will be 0)
		if( !faceIsConnected ) {
			continue;
		}
		// Check if this is already a sticky face, than we can skip the rest.
		if( getFlag( FLAG_FACE_STICKY ) ) {
			//cout << "[Face::" << __FUNCTION__ << "] Face is already tagged as sticky face." << endl;
			continue;
		}
		// Check if the face is already a neighbour.
		// If so, the two faces stick together and FLAG_FACE_STICKY will be set.
		int neighCount = 0;
		for( unsigned short i=0; i<(mNeighbourFacesNonManifold+3); ++i ) {
			if( mNeighbourFaces[i] == (*itFace) ) {
				++neighCount;
			}
		}
		if( neighCount == 0 ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: UNEXPECTED face " << (mNeighbourFacesNonManifold) << " should be added, but it is not!" << endl;
		} else if( neighCount > 1 ) {
			setFlag( FLAG_FACE_STICKY );
			(*itFace)->setFlag( FLAG_FACE_STICKY );
			//cout << "[Face::" << __FUNCTION__ << "] sticky faces found - sticks on " << neighCount  << " edges." << endl;
		}
		if( faceAddedMultipleTimes ) {
			std::cerr << "[Face::" << __FUNCTION__ << "] ERROR: Face no. " << getIndex()
			          << " multiple times added!" << std::endl; // pr (*itFace)->getIndex()
		}
	}
	//vertA->getFaces( vertB, &neighbourFaces, this );
	//vertB->getFaces( vertC, &neighbourFaces, this );
	//vertC->getFaces( vertA, &neighbourFaces, this );
}

//! Used during setup of the Mesh: adds non-manifold neighbour to mNeighbourFaces.
//! Additionally used when holes are filled e.g. during mesh polishing.
void Face::addNonManifold( Face* rExtraFace,           //!< Face to be added.
                           bool* rFaceWasAlreadyAdded  //!< Indicates the chance of a sticky face, when set to true.
                          ) {
	// Check for NULL pointer
	if( mNeighbourFaces == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: mNeighbourFaces being NULL is unexpected!" << endl; 
		return;
	}
	// Check for counter overflow
	if( mNeighbourFacesNonManifold == (USHRT_MAX-3) ) { // We have to substract three as our array is of length 3 PLUS mNeighbourFacesNonManifold
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: can not add extra face, because mNeighbourFacesNonManifold is already maxed out!" << endl;
		return;
	}
	// Check if the face was already added
	(*rFaceWasAlreadyAdded) = false;
	for( unsigned short i=0; i<(mNeighbourFacesNonManifold+3); i++ ) {
		if( mNeighbourFaces[i] == rExtraFace ) {
			// The following error occurs a lot after filling holes and slows down the process of cleaning a mesh. => Error message disabled.
			//cerr << "[Face::" << __FUNCTION__ << "] ERROR: face " << rExtraFace->getIndex() << " was already added at pos " << i << "!" << endl;
			// we do not return as it would be inappropriate and may break things.
			(*rFaceWasAlreadyAdded) = true;
		}
	}
	// Increase counter
	mNeighbourFacesNonManifold++;
	// Allocate the new array with increased size
	Face** newNeighbourFaces = new Face*[3+mNeighbourFacesNonManifold];
	// Copy old entries to the new array
	for( unsigned short i=0; i<(mNeighbourFacesNonManifold+2); i++ ) {
		newNeighbourFaces[i] = mNeighbourFaces[i];
	}
	// Add new entry to the last position of the new array
	newNeighbourFaces[mNeighbourFacesNonManifold+2] = rExtraFace;
	// Remove old array
	delete[] mNeighbourFaces;
	// Set new array to member pointer
	mNeighbourFaces = newNeighbourFaces;
}

//! Compute the normal vector stored in Primitive::normalXYZ
//!
//! As the half length of the non-normalized normal vector is the area
//! see: http://en.wikipedia.org/wiki/Triangle#Using_vectors
//! n* and area are pointer to (e.g.) an array allocated by the mesh, which
//! can easily be used for CUDA/OpenCL operations (memcopy!).
//!
//! Returns the area of the Face or Not-A-Number in case of an error.
//!
//! Does not compute the area, when FLAG_NORMAL_SET is set!
//! But the method will still return the area of the face.
double Face::getAreaNormal() {
	if( getFlag( FLAG_NORMAL_SET ) ) {
		return sqrt( ( NORMAL_X * NORMAL_X ) + ( NORMAL_Y * NORMAL_Y ) + ( NORMAL_Z * NORMAL_Z ) ) / 2.0;
	}

	double vBAx, vBAy, vBAz; // Vector AB
	double vCAx, vCAy, vCAz; // Vector CA

	vBAx = vertB->getX() - vertA->getX();
	vBAy = vertB->getY() - vertA->getY();
	vBAz = vertB->getZ() - vertA->getZ();

	vCAx = vertC->getX() - vertA->getX();
	vCAy = vertC->getY() - vertA->getY();
	vCAz = vertC->getZ() - vertA->getZ();

	// cross product using xyzzy - see: http://en.wikipedia.org/wiki/Cross_product#Mnemonic
	// divided by half => | NORMAL | = sqrt( x^2 + y^2 + z^2 ) == area
	NORMAL_X = ( ( vBAy * vCAz ) - ( vBAz * vCAy ) );
	NORMAL_Y = ( ( vBAz * vCAx ) - ( vBAx * vCAz ) );
	NORMAL_Z = ( ( vBAx * vCAy ) - ( vBAy * vCAx ) );
	// Set flag to avoid extra computation:
	setFlag( FLAG_NORMAL_SET );
	// we return the estimated area:
	double faceArea = sqrt( ( NORMAL_X * NORMAL_X ) + ( NORMAL_Y * NORMAL_Y ) + ( NORMAL_Z * NORMAL_Z ) ) / 2.0;
	if( faceArea <= DBL_EPSILON ) {
		setFlag( FLAG_FACE_ZERO_AREA );
		//cerr << "[Face::" << __FUNCTION__ << "] ERROR: Area < DBL_EPSILON => ZERO!" << endl;
	}
	return faceArea;
}

//! Computes the volume contribution of this face to the 3D-models volume using the divergence theorem
//! and increments  rVolumeDX, rVolumeDY and rVolumeDZ.
//!
//! @returns false in case of an error,
//!          when the area of this face is not a finite number,
//!          or in case the face area is zero.
bool Face::getVolumeDivergence(
                double& rVolumeDX,
                double& rVolumeDY,
                double& rVolumeDZ
) {
	double area     = getAreaNormal();
	if( !isfinite( area ) | ( area == 0.0 ) ) {
		return( false );
	}
	Vector3D cog    = getCenterOfGravity();
	Vector3D normal = getNormal();
	rVolumeDX += area * cog.getX() * normal.getX();
	rVolumeDY += area * cog.getY() * normal.getY();
	rVolumeDZ += area * cog.getZ() * normal.getZ();
	// faster alternative for X:
	//Vector3D normalArea = getNormal( false );
	//rVolumeDX[0] += ( cog.getX() * normalArea.getX() )/ 2.0;
	return( true );
}

//! Compute the prism shaped volume between the face and a given plane.
//! The volume is added to rVolume.
//! @returns false in case of an error, when the area of this face is not a finite number or zero.
bool Face::getVolumeToPlane( double* rVolume, bool* rPlanePos, Plane* rPlane ) {
	double area     = getAreaNormal();
	if( !isfinite( area ) | ( area == 0.0 ) ) {
		return false;
	}
	Vector3D planeNormal = rPlane->getNormal();
	double angleToPlane = angleToNormal( planeNormal );
	double cogDistToPlane;
	Vector3D cog = getCenterOfGravity();
	if( !rPlane->getDistanceToPoint( &cog, &cogDistToPlane ) ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: plane->getDistanceToPoint failed!" << endl;
		return false;
	}
	(*rVolume) += cos( angleToPlane ) * area * cogDistToPlane;
	(*rPlanePos) = cogDistToPlane < 0.0;
	/*
	cout << "NormalF: " << getNormal() << endl;
	cout << "NormalP: " << planeNormal << endl;
	cout << "COG:     " << cog << endl;
	cout << "Phi:     " << angleToPlane << " " << angleToPlane*180.0/M_PI << endl;
	cout << "Dist:    " << cogDistToPlane << endl;
	*/
	return true;
}

// mesh manipulation -----------------------------------------------------------

void Face::disconnectFace( Face* rBelonged2Face ) {
	//! ATTENTION: this method overrides the primitives method and someone has
	//! to remove this and rename neighbourFaces with faceList. But NOT BEFORE
	//! PROPERLY checked functionality!

	if( mNeighbourFaces == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: mNeighbourFaces is NULL!" << endl;
		return;
	}
	if( FACE_NEIGHBOUR_AB == rBelonged2Face ) {
		FACE_NEIGHBOUR_AB = nullptr;
		// We do not return here, as sticky faces are connected multiple times!
	}
	if( FACE_NEIGHBOUR_BC == rBelonged2Face ) {
		FACE_NEIGHBOUR_BC = nullptr;
		// We do not return here, as sticky faces are connected multiple times!
	}
	if( FACE_NEIGHBOUR_CA == rBelonged2Face ) {
		FACE_NEIGHBOUR_CA = nullptr;
		// We do not return here, as sticky faces are connected multiple times!
	}
	if( mNeighbourFacesNonManifold == 0 ) {
		// Nothing else to do - bail out, which is safer than sorry.
		return;
	}
	//cout << "[Face::" << __FUNCTION__ << "] removing non-manifold." << endl;
	// Check if the face is actually connected (and how often).
	unsigned int isConnected = 0;
	for( unsigned short i=3; i<(mNeighbourFacesNonManifold+3); i++ ) {
		if( mNeighbourFaces[i] == rBelonged2Face ) {
			isConnected++;
		}
	}
	// isConnected should be ONE.
	// isConnected can TWO or THREE for sticky or sticky+non-manifold faces.
	// isConnected can be ZERO for the 2nd/3rd time a sticky face tries to disconnect.
	// When the isConnected is already ZERO, we can skip the rest:
	if( isConnected == 0 ) {
		return;
	}
	// --- Show a warning, when we encounter an unexpected situation.
	//if( isConnected != 1 ) {
	//	cerr << "[Face::" << __FUNCTION__ << "] ERROR: unexpected number of connections: " << isConnected << "!" << endl;
	//}
	// Keep old array length to copy values to the new array (see below).
	unsigned oldArrayLen = mNeighbourFacesNonManifold+3;
	// As we always expect newNeighbourFaces to be of length 3, mNeighbourFacesNonManifold can not be negative!
	if( isConnected > mNeighbourFacesNonManifold ) {
		mNeighbourFacesNonManifold = 0;
	} else {
		// Decrease counter by connections, which should be ONE
		mNeighbourFacesNonManifold -= isConnected;
	}
	// Allocate new (shorter) array
	Face** newNeighbourFaces = new Face*[mNeighbourFacesNonManifold+3];
	// To be safe than sorry, we initalize the array:
	for( unsigned short i=0; i<(mNeighbourFacesNonManifold+3); i++ ) {
		newNeighbourFaces[i] = nullptr;
	}
	// Copy all elements except rBelonged2Face from the old to the new array
	unsigned short iNew = 0;
	for( unsigned short i=0; i<oldArrayLen; i++ ) {
		if( mNeighbourFaces[i] == rBelonged2Face ) {
			continue;
		}
		newNeighbourFaces[iNew] = mNeighbourFaces[i];
		iNew++;
	}
	// Remove old array
	delete[] mNeighbourFaces;
	// Set new array
	mNeighbourFaces = newNeighbourFaces;
}

//! Inverts the orientation of the face by exchanging Vertex A with Vertex B.
//! @returns false in case of an error. True otherwise.
bool Face::invertFaceOrientation() {
	// Switch vertices:
	VertexOfFace* tempVert = vertA;
	vertA = vertB;
	vertB = tempVert;
	// Update normal, when necessary:
	if( getFlag( FLAG_NORMAL_SET ) ) {
		NORMAL_X = -NORMAL_X;
		NORMAL_Y = -NORMAL_Y;
		NORMAL_Z = -NORMAL_Z;
	}
	return true;
}

// Navigation ------------------------------------------------------------------

//! Returns the opposing edge index as well as pointers to its vertices of vertFrom.
//! Returns false in case of an error: vertFrom not part of this Face.
bool Face::getOposingEdgeAndVertices( Vertex* rVertFrom, eEdgeNames* rEdgeIdx, Vertex** rVert1, Vertex** rVert2 ) {
	if( rVertFrom == vertA ) {
		(*rEdgeIdx) = EDGE_BC;
		(*rVert1) = vertB;
		(*rVert2) = vertC;
		return true;
	}
	if( rVertFrom == vertB ) {
		(*rEdgeIdx) = EDGE_CA;
		(*rVert1) = vertC;
		(*rVert2) = vertA;
		return true;
	}
	if( rVertFrom == vertC ) {
		(*rEdgeIdx) = EDGE_AB;
		(*rVert1) = vertA;
		(*rVert2) = vertB;
		return true;
	}
	// When we reach this point something has gone wrong - vertFrom not part of this Face.
	return false;
}

//! Determine the two opposing vertices.
//!
//! @returns false in case of an error including that the given reference vertex was not found.
//!          True otherwise.
bool Face::getOposingVertices(
                const Vertex*  rVertRef,
                      Vertex*& rVertOpp1,
                      Vertex*& rVertOpp2
) const {
	if( rVertRef == vertA ) {
		rVertOpp1 = vertB;
		rVertOpp2 = vertC;
		return( true );
	}
	if( rVertRef == vertB ) {
		rVertOpp1 = vertC;
		rVertOpp2 = vertA;
		return( true );
	}
	if( rVertRef == vertC ) {
		rVertOpp1 = vertA;
		rVertOpp2 = vertB;
		return( true );
	}
	// When we reach this point something has gone wrong - rVertRef not part of this Face.
	return( false );
}

//! Returns the opposing Vertex of two Vertices defining an edge. This means
//! if you supply a pointer to e.g. C and A of an Edge C-A, the Vertex B
//! will be returned. The method returns NULL when the given Edge is not
//! part of the Face.
Vertex* Face::getOposingVertex( Vertex* vert1, Vertex* vert2 ) {
	if( ( ( vert1 == vertA ) && ( vert2 == vertB ) ) || ( ( vert2 == vertA ) && ( vert1 == vertB ) ) ) {
		return getVertC();
	}
	if( ( ( vert1 == vertB ) && ( vert2 == vertC ) ) || ( ( vert2 == vertB ) && ( vert1 == vertC ) ) ) {
		return getVertA();
	}
	if( ( ( vert1 == vertC ) && ( vert2 == vertA ) ) || ( ( vert2 == vertC ) && ( vert1 == vertA ) ) ) {
		return getVertB();
	}
	return nullptr;
}

//! Returns the opposing Vertex of a neighbour Face on an edge.
//! The method returns NULL when the given Face is not a
//! neighbour of this Face.
//! Ignore non-manifold edge.
Vertex* Face::getOposingVertex( Face* neighFace ) {
	if( neighFace == nullptr ) {
		return nullptr;
	}
	if( FACE_NEIGHBOUR_AB == neighFace ) {
		return getVertC();
	}
	if( FACE_NEIGHBOUR_BC == neighFace ) {
		return getVertA();
	}
	if( FACE_NEIGHBOUR_CA == neighFace ) {
		return getVertB();
	}
	return nullptr;
}

//! Returns the opposing Vertex of a neighbour Face on an edge
//! and the edge indices leading from and to the Vertex (manifold only).
//! The method returns NULL when the given Face is not a
//! neighbour of this Face.
//! Ignore non-manifold edge.
Vertex* Face::getOposingVertex( Face* rNeighFace, eEdgeNames* rEdgeIdxAC, eEdgeNames* rEdgeIdxCB ) {
	if( rNeighFace == nullptr ) {
		(*rEdgeIdxAC) = EDGE_NONE;
		(*rEdgeIdxCB) = EDGE_NONE;
		return nullptr;
	}
	if( FACE_NEIGHBOUR_AB == rNeighFace ) {
		(*rEdgeIdxAC) = EDGE_BC;
		(*rEdgeIdxCB) = EDGE_CA;
		return getVertC();
	}
	if( FACE_NEIGHBOUR_BC == rNeighFace ) {
		(*rEdgeIdxAC) = EDGE_CA;
		(*rEdgeIdxCB) = EDGE_AB;
		return getVertA();
	}
	if( FACE_NEIGHBOUR_CA == rNeighFace ) {
		(*rEdgeIdxAC) = EDGE_AB;
		(*rEdgeIdxCB) = EDGE_BC;
		return getVertB();
	}
	return nullptr;
}

//! Returns the neighbouring Face at Edge AB, BC or CA.
//! Non-manifold edges are ignored.
//! In case of a border or an error NULL is returned.
Face* Face::getNeighbourFace( eEdgeNames rEdgeIdx ) {
	switch( rEdgeIdx ) {
		case EDGE_NONE:
			cerr << "[Face::" << __FUNCTION__ << "] invalid edge: EDGE_NONE!" << endl;
			return nullptr;
			break;
		case EDGE_NON_MANIFOLD:
			cerr << "[Face::" << __FUNCTION__ << "] invalid edge: EDGE_NON_MANIFOLD!" << endl;
			return nullptr;
			break;
		case EDGE_AB:
			return FACE_NEIGHBOUR_AB;
			break;
		case EDGE_BC:
			return FACE_NEIGHBOUR_BC;
			break;
		case EDGE_CA:
			return FACE_NEIGHBOUR_CA;
			break;
	}
	cerr << "[Face::" << __FUNCTION__ << "] unknown edge index: " << rEdgeIdx << "!" << endl;
	return nullptr;
}

//! Returns the neighbouring Face at Edge AB, BC or CA and its edge indices excluding the connecting edge.
//! Non-manifold edges are ignored.
//! In case of a border or an error NULL is returned and adjaEdge1/2 are set to Face::EDGE_NONE.
Face* Face::getNeighbourFace( eEdgeNames rEdgeIdx, eEdgeNames* rAdjaEdge1, eEdgeNames* rAdjaEdge2 ) {
	Face* nextFace = getNeighbourFace( rEdgeIdx );
	if( nextFace == nullptr ) {
		(*rAdjaEdge1) = EDGE_NONE;
		(*rAdjaEdge2) = EDGE_NONE;
		return nullptr;
	}
	getEdgesNotAtFace( this, rAdjaEdge1, rAdjaEdge2 );
	return nextFace;
}

//! Returns the edge indicies of the manifold edges not connected to someFace.
//! Assumes that someFace is related to one of the Faces edges!
void Face::getEdgesNotAtFace( Face* rSomeFace, eEdgeNames* rAdjaEdge1, eEdgeNames* rAdjaEdge2 ) {
	bool adjaEdge1set = false;
	if( FACE_NEIGHBOUR_AB != rSomeFace ) {
		(*rAdjaEdge1) = EDGE_AB;
		adjaEdge1set = true;
	}
	if( FACE_NEIGHBOUR_BC != rSomeFace ) {
		if( !adjaEdge1set ) {
			(*rAdjaEdge1) = EDGE_BC;
		} else {
			(*rAdjaEdge2) = EDGE_BC;
		}
	}
	if( FACE_NEIGHBOUR_CA != rSomeFace ) {
		(*rAdjaEdge2) = EDGE_BC;
	}
}

//! Estimates and returns a set of neighbourfaces.
//! Returns false in case of an error.
bool Face::getNeighbourFaces( set<Face*>* rSomeFaces ) {
	if( rSomeFaces == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	if( mNeighbourFaces == nullptr ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: unexpected NULL pointer for neighbour faces!" << endl;
		return false;
	}
	for( unsigned short i=0; i<(mNeighbourFacesNonManifold+3); i++ ) {
		if( mNeighbourFaces[i] == nullptr ) {
			continue;
		}
		rSomeFaces->insert( mNeighbourFaces[i] );
	}
	return true;
}

//! Returns the number of neighbouring faces.
unsigned int Face::getNeighbourFaceCount() {
	unsigned int neighbourFaceCount = mNeighbourFacesNonManifold;
	if( FACE_NEIGHBOUR_AB != nullptr ) {
		neighbourFaceCount++;
	}
	if( FACE_NEIGHBOUR_BC != nullptr ) {
		neighbourFaceCount++;
	}
	if( FACE_NEIGHBOUR_CA != nullptr ) {
		neighbourFaceCount++;
	}
	return neighbourFaceCount;
}

//! Selects a neighbouring Face having someVert and is not within facesToExclude.
//! Typically used by: Vertex::isDoubleCone()
Face* Face::getNextFaceWith( Vertex* someVert, set<Face*>* facesToExclude ) {
	set<Face*>::iterator itFaceFound;
	for( unsigned short i=0; i<(3+mNeighbourFacesNonManifold); i++ ) {
		if( mNeighbourFaces[i] == nullptr ) {
			continue;
		}
		if( mNeighbourFaces[i]->requiresVertex( someVert ) ) {
			//cout << (*itFace)->getIndexOriginal() << " has vertex " << someVert->getIndexOriginal() << endl;
			if( facesToExclude->find( mNeighbourFaces[i] ) == facesToExclude->end() ) {
				return mNeighbourFaces[i];
			}
		}
	}
	return nullptr;
}

//! Adds all neighbouring Faces to neighboursSelected, which are not within facesToExclude.
void Face::getNeighbourFacesExcluding( set<Face*>* neighboursSelected, set<Face*>* facesToExclude ) {
	set<Face*>::iterator itFaceFound;
	for( unsigned short i=0; i<(3+mNeighbourFacesNonManifold); i++ ) {
		if( mNeighbourFaces[i] == nullptr ) {
			continue;
		}
		if( facesToExclude->find( mNeighbourFaces[i] ) != facesToExclude->end() ) {
			continue;
		}
		neighboursSelected->insert( mNeighbourFaces[i] );
	}
}

//! Adds the vertices vertA, vertB and vertC to a given list.
void Face::copyVerticesTo( set<Vertex*>* someVertexList, Vertex* vertToExclude ) {
	if( vertToExclude != vertA ) {
		someVertexList->insert( vertA );
	}
	if( vertToExclude != vertB ) {
		someVertexList->insert( vertB );
	}
	if( vertToExclude != vertC ) {
		someVertexList->insert( vertC );
	}
}

//! Return the reference to next vertex on the border of this face.
//! Considers the bit-array for vertices tagged unvisited.
Vertex* Face::getNextBorderVertex( Vertex* rVertStart, uint64_t* rVertBitArrayUnVisited ) {
	if( rVertStart == vertA ) {
		if( FACE_NEIGHBOUR_AB == nullptr ) { // continue only when A-B is an edge, which means there is no adjacent face at this edge
			if( vertB->isBorder() ) {
				if( vertB->isMarked( rVertBitArrayUnVisited ) ) { // return the vertex reference only, when it is marked within the bit array
					return vertB;
				}
				return nullptr;
			}
		}
	}
	if( rVertStart == vertB ) {
		if( FACE_NEIGHBOUR_BC == nullptr ) { // continue only when B-C is an edge, which means there is no adjacent face at this edge
			if( vertC->isBorder() ) {
				if( vertC->isMarked( rVertBitArrayUnVisited ) ) { // return the vertex reference only, when it is marked within the bit array
					return vertC;
				}
				return nullptr;
			}
		}
	}
	if( rVertStart == vertC ) {
		if( FACE_NEIGHBOUR_CA == nullptr ) { // continue only when C-A is an edge, which means there is no adjacent face at this edge
			if( vertA->isBorder() ) {
				if( vertA->isMarked( rVertBitArrayUnVisited ) ) {  // return the vertex reference only, when it is marked within the bit array
					return vertA;
				}
				return nullptr;
			}
		}
	}
	return nullptr;
}

//! Tests if one of the vertices in the given list is next to the fromVert.
//!
//! Reverse selects the search in order or reverse order of the faces vertices.
//!
//! Returns this "next" vertex, when found and removes it from the list.
//! If no "next" vertex is found NULL is returned.
Vertex* Face::getConnection( Vertex* fromVert, set<Vertex*>* someVertList, bool reverse ) {
	Vertex* nextVert = nullptr;

	if( reverse ) {
		if( fromVert == vertA ) {
			nextVert = vertC;
		} else if( fromVert == vertB ) {
			nextVert = vertA;
		} else if( fromVert == vertC ) {
			nextVert = vertB;
		} else {
			// will happen when fromVert is not even part of the Face.
			// which basically means an unnessary call of this function!
			cerr << "[Face::getConnection] bad call (reverse)!" << endl;
			return nullptr;
		}
	} else {
		if( fromVert == vertA ) {
			nextVert = vertB;
		} else if( fromVert == vertB ) {
			nextVert = vertC;
		} else if( fromVert == vertC ) {
			nextVert = vertA;
		} else {
			// will happen when fromVert is not even part of the Face.
			// which basically means an unnessary call of this function!
			cerr << "[Face::getConnection] bad call (forward)!" << endl;
			return nullptr;
		}
	}

	set<Vertex*>::iterator itVertex = someVertList->find( nextVert );
 	if( itVertex != someVertList->end() ) {
 		someVertList->erase( itVertex );
 		return nextVert;
 	}
	// not in list - so:
	return nullptr;
}

//! Returns the 1st Vertex from the given edge - ignore non-manifold edges.
Vertex* Face::getVertexFromEdgeA( eEdgeNames rEdgeIdx ) {
	if( rEdgeIdx == EDGE_AB ) {
		return getVertA();
	}
	if( rEdgeIdx == EDGE_BC ) {
		return getVertB();
	}
	if( rEdgeIdx == EDGE_CA ) {
		return getVertC();
	}
	return nullptr;
}

//! Returns the 2nd Vertex from the given edge - ignore non-manifold edges.
Vertex* Face::getVertexFromEdgeB( eEdgeNames rEdgeIdx ) {
	if( rEdgeIdx == EDGE_AB ) {
		return getVertB();
	}
	if( rEdgeIdx == EDGE_BC ) {
		return getVertC();
	}
	if( rEdgeIdx == EDGE_CA ) {
		return getVertA();
	}
	return nullptr;
}

// Geodesics -------------------------------------------------------------------------------------

bool Face::getGeodesicEdgesSeed( deque<EdgeGeodesic*>* frontEdges, map<Vertex*,GeodEntry*>* geoDistList, uint64_t* faceBitArray, int faceNrBlocks, bool weightFuncVal ) {
	//! Assumes this Face as seed and adds its edges to the front.
	//! Ignores any additional (non-manifold) edges.
	//!
	//! Returns false in case of an error.

	// Check for invalid settings:
	if( ( frontEdges == nullptr ) || ( geoDistList == nullptr ) ) {
		cerr << "[Face::" << __FUNCTION__ << "] ERROR: frontEdges and/or geoDistList is/are NULL!" << endl;
		return false;
	}

	// Estimate distances:
	Vector3D vecCOG = getCenterOfGravity();
	Vector3D vecGeodA = vertA->getPositionVector() - vecCOG;
	Vector3D vecGeodB = vertB->getPositionVector() - vecCOG;
	Vector3D vecGeodC = vertC->getPositionVector() - vecCOG;
	double geodDistA = vecGeodA.getLength3();
	double geodDistB = vecGeodB.getLength3();
	double geodDistC = vecGeodC.getLength3();
	double geodAngleA = 0.0;
	double geodAngleB = angle( vecGeodB, vecGeodA, getNormal() );
	double geodAngleC = angle( vecGeodC, vecGeodA, getNormal() );
	// Apply optional weight:
	if( weightFuncVal ) {
		double funcValCent;
		double funcValA;
		double funcValB;
		double funcValC;
		if( !getFuncValue( &funcValCent ) ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: can not get function value for this face!" << endl;
			return false;
		}
		if( !vertA->getFuncValue( &funcValA ) ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: can not get function value for vertex A!" << endl;
			return false;
		}
		if( !vertB->getFuncValue( &funcValB ) ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: can not get function value for vertex B!" << endl;
			return false;
		}
		if( !vertC->getFuncValue( &funcValC ) ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: can not get function value for vertex C!" << endl;
			return false;
		}
		geodDistA *= ( funcValA - funcValCent ) + 0.5;
		geodDistB *= ( funcValB - funcValCent ) + 0.5;
		geodDistC *= ( funcValC - funcValCent ) + 0.5;
		if( geodDistA <= 0.0 ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: negative/zero geodesic distance for A!" << endl;
			return false;
		}
		if( geodDistB <= 0.0 ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: negative/zero geodesic distance for B!" << endl;
			return false;
		}
		if( geodDistC <= 0.0 ) {
			cerr << "[Face::" << __FUNCTION__ << "] ERROR: negative/zero geodesic distance for C!" << endl;
			return false;
		}
	}
	GeodEntry* geodA = new GeodEntry( geodDistA, geodAngleA, this );
	GeodEntry* geodB = new GeodEntry( geodDistB, geodAngleB, this );
	GeodEntry* geodC = new GeodEntry( geodDistC, geodAngleC, this );
	//cout << "[Face::getGeodesicEdgesSeed] geod. distances: " << (*geodA) << "  " << (*geodB) << "  " << (*geodC) << endl;
	// Add them to the list of geodesic distances
	geoDistList->insert( pair<Vertex*,GeodEntry*>( vertA, geodA ) );
	geoDistList->insert( pair<Vertex*,GeodEntry*>( vertB, geodB ) );
	geoDistList->insert( pair<Vertex*,GeodEntry*>( vertC, geodC ) );
	EdgeGeodesic* edgeAB = new EdgeGeodesic( this, EDGE_AB, geodA, geodB );
	EdgeGeodesic* edgeBC = new EdgeGeodesic( this, EDGE_BC, geodB, geodC );
	EdgeGeodesic* edgeCA = new EdgeGeodesic( this, EDGE_CA, geodC, geodA );
	// Insert the edges to the front:
	//make_heap( frontEdges->begin(), frontEdges->end() );
	frontEdges->push_back( edgeAB );
	push_heap( frontEdges->begin(), frontEdges->end(), EdgeGeodesic::shorterThan );
	frontEdges->push_back( edgeBC );
	push_heap( frontEdges->begin(), frontEdges->end(), EdgeGeodesic::shorterThan );
	frontEdges->push_back( edgeCA );
	push_heap( frontEdges->begin(), frontEdges->end(), EdgeGeodesic::shorterThan );
	//sort_heap( frontEdges->begin(), frontEdges->end() );

	// Mark this (seed( face visited:
	uint64_t bitOffset;
	uint64_t bitNr;
	getIndexOffsetBit( &bitOffset, &bitNr );
	faceBitArray[bitOffset] = (1UL << bitNr);

	return true;
}

// Surface integrals ------------------------------------------------------------------------------

bool Face::intersectsSphere1( Vector3D positionVec, double radius ) {
	//! Test if the Face is intersected by a sphere.
	//! Using Edge::intersectsSphere1

	// A face can be intersected at two or three edges, but if
	// find that one edge is intersected we don't have to 
	// search further.
	if( intersectsSphere1( vertA, vertB, positionVec, radius ) ) {
		return true;
	}
	if( intersectsSphere1( vertB, vertC, positionVec, radius ) ) {
		return true;
	}
	if( intersectsSphere1( vertC, vertA, positionVec, radius ) ) {
		return true;
	}

	return false;
}

bool Face::intersectsSphere1( Vertex* vert1, Vertex* vert2, Vector3D positionVec, double radius ) {
	//! Tests if the Edge intersects with a given sphere.
	//! Method (1) by exclusion.
	//!
	//! Consider the three cases shown here:
	//! http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

	// Shift our vertices A, B and P (=positionVec), so that the 
	// sphere's center P equals the origin, which makes some estimations
	// simpler and faster:
	Vector3D vecA0Pos = vert1->getPositionVector() - positionVec;
	Vector3D vecB0Pos = vert2->getPositionVector() - positionVec;

	double vecALen3 = vecA0Pos.getLength3();
	double vecBLen3 = vecB0Pos.getLength3();

	// A on the spheres surface:
	if( vecALen3 == radius ) {
		return true;
	}
	// B on the spheres surface: 
	if( vecBLen3 == radius ) {
		return true;
	}

	// A inside and B outside the sphere:
	if( ( vecALen3 < radius ) && ( vecBLen3 > radius ) ) {
		return true;
	}

	// B inside and A outside the sphere:
	if( ( vecBLen3 < radius ) && ( vecALen3 > radius ) ) {
		return true;
	}

	// A and B inside the sphere:
	if( ( vecALen3 < radius ) && ( vecBLen3 < radius ) ) {
		return false;
	}

	// still there is one more case, where the Edges points are outside
	// the sphere, but the edge intersects it - we can find these edges by:

	// 1. estimation of the minimum distance between the line (defined 
	// by the edge) and the spheres center:

	// as we shifted everytthing, so that positionVec = ( 0,0,0 ) we can simplify
	// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
	// to:
	double lenAB   = abs3( ( vecB0Pos - vecA0Pos ) );
	double distMin = abs3( ( ( vecB0Pos - vecA0Pos ) % ( vecA0Pos ) ) ) / lenAB;
	if( distMin > radius ) {
		return false;
	}
	//cout << "[Edge] intersectsSphere distMin: " << distMin << " < radius: " << radius << endl;

	// 2. find the point P' on the line closest to the sphere has to be between
	// the vertices of the edges. inspired by:
	// http://planetmath.org/encyclopedia/Project.html
	double angleABP = angle( vecB0Pos - vecA0Pos, -vecA0Pos ); // remember: P = ( 0,0,0 )
	if( abs( angleABP ) > M_PI/2.0 ) { // > 90° => P' (P projected on line AB) is outside the edge segment A-B 
		return false;
	}
	//cout << "[Edge] intersectsSphere angleABP: " << angleABP*180.0/M_PI << "°" << endl;
	//cout << "[Edge] intersectsSphere " << vecALen3 * cos( angleABP ) << " < " << lenAB << endl;
	if( vecALen3 * cos( angleABP ) < lenAB ) {
		cout << "[Edge] intersectsSphere INTERSECT."<< endl;
		return true;
	}

	// otherwise no intersection:
	return false;
}

bool Face::intersectsSphere2( Vector3D positionVec, double radius ) {
	//! Test if the Face is intersected by a sphere.
	//! Using Edge::intersectsSphere2

	// A face can be intersected at two or three edges, but if
	// find that one edge is intersected we don't have to 
	// search further.
	if( intersectsSphere2( vertA, vertB, positionVec, radius ) ) {
		return true;
	}
	if( intersectsSphere2( vertB, vertC, positionVec, radius ) ) {
		return true;
	}
	if( intersectsSphere2( vertC, vertA, positionVec, radius ) ) {
		return true;
	}

	return false;
}

bool Face::intersectsSphere2( Vertex* vert1, Vertex* vert2, Vector3D positionVec, double radius ) {
	//! Tests if the Edge intersects with a given sphere.
	//! Method (2) by estimation of intersection points and 
	//! assuring that one of the intersections is on the egde
	//! between A and B (= line segment).

	Vector3D interSec1;
	Vector3D interSec2;
	int noIntersectionAtAll;
	
	noIntersectionAtAll = pointOfSphereEdgeIntersection( vert1, vert2, positionVec, radius, &interSec1, &interSec2 );
	if( noIntersectionAtAll == 0 ) {
		return false;
	}

	return true;
}

int Face::intersectSphereSpecial( Vector3D positionVec, double radius, Vector3D** interSections ) {
	//! Estimate intersection between this Face and a given Sphere.

	cout << "%-------------" << endl;
	Vector3D interSec1;
	Vector3D interSec2;
	int noIntersectionAtAll;
	int totalIntersections = 0;

	noIntersectionAtAll = pointOfSphereEdgeIntersection( vertA, vertB, positionVec, radius, &interSec1, &interSec2 );
	totalIntersections += noIntersectionAtAll;
	if( noIntersectionAtAll == 1 ) {
		interSec1.dumpInfo();
	}
	if( noIntersectionAtAll == 2 ) {
		interSec1.dumpInfo();
		interSec2.dumpInfo();
	}

	double dirAB_x = vertB->getX() - vertA->getX();
	double dirAB_y = vertB->getY() - vertA->getY();
	double dirAB_z = vertB->getZ() - vertA->getZ();

	double lenAB = sqrt( pow(dirAB_x,2) + pow(dirAB_y,2) + pow(dirAB_z,2) );

	double D_x = dirAB_x / lenAB;
	double D_y = dirAB_y / lenAB;
	double D_z = dirAB_z / lenAB;

	double C_x = -vertA->getX() + positionVec.getX();
	double C_y = -vertA->getY() + positionVec.getY();
	double C_z = -vertA->getZ() + positionVec.getZ();

	double d = D_x * C_x + D_y * C_y + D_z * C_z;
	double underSQRT = pow( d, 2 ) - ( C_x * C_x + C_y * C_y + C_z * C_z ) + pow( radius, 2 );
	if( underSQRT < 0 ) {
		cout << "% NO intersection A-B" << endl;
	} else if( underSQRT == 0 ) {
		cout << "% Tangent A-B - d=" << d << endl;
	} else {
		double d1 = d-sqrt(underSQRT);
		double d2 = d+sqrt(underSQRT);
		cout << "% Intersection A-B d: " << d1 << " " << d2 << endl;
		double intersect1_x = d1*D_x + vertA->getX();
		double intersect1_y = d1*D_y + vertA->getY();
		double intersect1_z = d1*D_z + vertA->getZ();
		double intersect2_x = d2*D_x + vertA->getX();
		double intersect2_y = d2*D_y + vertA->getY();
		double intersect2_z = d2*D_z + vertA->getZ();
		cout << "         p(end+1,:) = [ " << intersect1_x << " , " << intersect1_y << " , " << intersect1_z << " ]; " << endl;
		cout << "         p(end+1,:) = [ " << intersect2_x << " , " << intersect2_y << " , " << intersect2_z << " ]; " << endl;
		cout << "%        radius = " << sqrt( pow( intersect1_x - positionVec.getX(), 2 ) + pow( intersect1_y - positionVec.getY(), 2 ) + pow( intersect1_z - positionVec.getZ(), 2 ) ) << endl;
		cout << "%        radius = " << sqrt( pow( intersect2_x - positionVec.getX(), 2 ) + pow( intersect2_y - positionVec.getY(), 2 ) + pow( intersect2_z - positionVec.getZ(), 2 ) ) << endl;
	}
	cout << "%-------------" << endl;

/*
	noIntersectionAtAll = pointOfSphereEdgeIntersection( vertB, vertC, positionVec, radius, &interSec1, &interSec2 );
	totalIntersections += noIntersectionAtAll;
	if( noIntersectionAtAll == 1 ) {
		interSec1.dumpInfo();
	}
	if( noIntersectionAtAll == 2 ) {
		interSec1.dumpInfo();
		interSec2.dumpInfo();
	}
	noIntersectionAtAll = pointOfSphereEdgeIntersection( vertC, vertA, positionVec, radius, &interSec1, &interSec2 );
	totalIntersections += noIntersectionAtAll;
	if( noIntersectionAtAll == 1 ) {
		interSec1.dumpInfo();
	}
	if( noIntersectionAtAll == 2 ) {
		interSec1.dumpInfo();
		interSec2.dumpInfo();
	}
*/
	//cout << "[Face::intersectSphereSpecial] " << totalIntersections << endl;
	return totalIntersections;
}

int Face::getSphereIntersections( Vector3D positionVec, double radius, vector<Vector3D>* pointsOfIntersection ) {
	//! Pushs the points of intersection between a sphere and this Face int the given vector.
	//! Maintains the direction given by the Face.
	//!
	//! Returns the number of segments added.
	//!
	//! ATTENTION: by current state this method covers only the following case(s):
	//! The sphere intersects two edges at one point per edge.
	//! 
	//! It does not cover all the other rarer cases like a sphere intersecting 
	//! an edge at two points, etc.

	int nrIntersecAB;
	int nrIntersecBC;
	int nrIntersecCA;
	Vector3D intersecAB1;
	Vector3D intersecAB2;
	Vector3D intersecBC1;
	Vector3D intersecBC2;
	Vector3D intersecCA1;
	Vector3D intersecCA2;
	vector<Vector3D> vecInterSect;

	nrIntersecAB = pointOfSphereEdgeIntersection( vertA, vertB, positionVec, radius, &intersecAB1, &intersecAB2 );
	nrIntersecBC = pointOfSphereEdgeIntersection( vertB, vertC, positionVec, radius, &intersecBC1, &intersecBC2 );
	nrIntersecCA = pointOfSphereEdgeIntersection( vertC, vertA, positionVec, radius, &intersecCA1, &intersecCA2 );

	//if( nrIntersecAB == 2 ) {
	//	intersecAB1.dumpInfo();
	//	intersecAB2.dumpInfo();
	//}

	// no intersection
	if( nrIntersecAB + nrIntersecBC + nrIntersecCA == 0 ) {
		return 0;
	}

	// intersection AB -> BC 
	if( ( nrIntersecAB == 1 ) && ( nrIntersecBC == 1 ) ) {
		pointsOfIntersection->push_back( intersecAB1 );
		pointsOfIntersection->push_back( intersecBC1 );
		return 1;
	}
	// intersection BC -> CA
	if( ( nrIntersecBC == 1 ) && ( nrIntersecCA == 1 ) ) {
		pointsOfIntersection->push_back( intersecBC1 );
		pointsOfIntersection->push_back( intersecCA1 );
		return 1;
	}
	// intersection CA -> AB
	if( ( nrIntersecCA == 1 ) && ( nrIntersecAB == 1 ) ) {
		pointsOfIntersection->push_back( intersecCA1 );
		pointsOfIntersection->push_back( intersecAB1 );
		return 1;
	}

	if( nrIntersecAB == 1 ) {
		pointsOfIntersection->push_back( intersecAB1 );
	}
	if( nrIntersecAB == 2 ) {
		pointsOfIntersection->push_back( intersecAB1 );
		pointsOfIntersection->push_back( intersecAB2 );
	}

	if( nrIntersecBC == 1 ) {
		pointsOfIntersection->push_back( intersecBC1 );
	}
	if( nrIntersecBC == 2 ) {
		pointsOfIntersection->push_back( intersecBC1 );
		pointsOfIntersection->push_back( intersecBC2 );
	}

	if( nrIntersecCA == 1 ) {
		pointsOfIntersection->push_back( intersecCA1 );
	}
	if( nrIntersecCA == 2 ) {
		pointsOfIntersection->push_back( intersecCA1 );
		pointsOfIntersection->push_back( intersecCA2 );
	}

	cerr << "[Face] getSphereIntersections unsupported intersection! " << nrIntersecAB << " " << nrIntersecBC << " " << nrIntersecCA << endl;

	return 0;
}

int Face::pointOfSphereEdgeIntersection( Vertex* vert1, Vertex* vert2, Vector3D positionVec, double radius, Vector3D* interSec1, Vector3D* interSec2 ) {
	//! Estimates the points of intersection of the line defined by the edge with a given sphere.
	//! Returns number of intersections (0,1,2 - everything else is bogus!).
	//!
	//! See: http://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

	// Shift our vertices A, B and P (=positionVec), so that A equals the origin, 
	// which makes some estimations simpler and faster. 
	// According to wiki: "Equation for a line starting at (0,0,0)"
	Vector3D vecLineUnit  = normalize3( vert2->getPositionVector() - vert1->getPositionVector() );
	Vector3D sphereCenter = positionVec - vert1->getPositionVector();

	double b = pow( dot3( vecLineUnit, sphereCenter ), 2 ) - dot3( sphereCenter, sphereCenter ) + pow( radius, 2 );
	// There is no solution if sqrt( b ) is imaginary, which is the case when:
	if( b < 0.0 ) {
		//cout << "[Edge] pointOfSphereIntersection: NO intersection." << endl;
		return 0;
	}

	double a = dot3( vecLineUnit, sphereCenter );
	double d1 = a + sqrt( b );
	double d2 = a - sqrt( b );
	double dMax = abs3( vert2->getPositionVector() - vert1->getPositionVector() );
	Vector3D interSection1 = vecLineUnit * d1 + vert1->getPositionVector();
	Vector3D interSection2 = vecLineUnit * d2 + vert1->getPositionVector();

	// only d1 is probably valid, so we have only one or no intersection:
	if( ( d2 < 0.0 ) || ( d2 > dMax ) ) {
		// check if d1 is in a valid range or not:
		if( ( d1 < 0.0 ) || ( d1 > dMax ) ) {
			return 0;
		} else {
			interSec1->set( &interSection1 );
			return 1;
		}
	} else {
		// d2 is valid, so we have to check about d1 as well
		if( ( d1 < 0.0 ) || ( d1 > dMax ) ) {
			interSec1->set( &interSection2 );
			return 1;
		} else {
			interSec1->set( &interSection1 );
			interSec2->set( &interSection2 );
			return 2;
		}
	}
	cerr << "[Edge] pointOfSphereEdgeIntersection IMPOSSIBLE SOLUTION." << endl;
	return -1;
}

Vertex* Face::advanceInSphere( Vertex* fromVert, double* sphereCenter, double radius, uint64_t* bitArrayVisited, set<Vertex*>* nextArray ) {
	//! relates to Mesh::fetchSphereBitArray
	//! Remark: fromVert has to be NOT NULL and should be one of the faces vertices - we don't check this for performance reasons.
	int bitOffset;
	int bitNr;
	Vertex* nextVert = nullptr;
	if( vertA != fromVert ) {
		// only when the vertex is within the sphere and it has not been visited, we can add it to the bitArrayNext
		if( vertA->distanceToCoord( sphereCenter ) <= radius ) {
			// estimate the position within the bit array
			vertA->getIndexOffsetBit( &bitOffset, &bitNr );
			// check if bit is set:
			if( ! ((1UL << bitNr) & bitArrayVisited[bitOffset] ) ) {
				// if not add to visited and next:
				bitArrayVisited[bitOffset] |= (1UL << bitNr);
				nextArray->insert( vertA );
				nextVert = vertA;
			}
		}
	}
	if( vertB != fromVert ) {
		// only when the vertex is within the sphere and it has not been visited, we can add it to the bitArrayNext
		if( vertB->distanceToCoord( sphereCenter ) <= radius ) {
			// estimate the position within the bit array
			vertB->getIndexOffsetBit( &bitOffset, &bitNr );
			// check if bit is set:
			if( ! ((1UL << bitNr) & bitArrayVisited[bitOffset] ) ) {
				// if not add to visited and next:
				bitArrayVisited[bitOffset] |= 1UL <<bitNr;
				nextArray->insert( vertB );
				nextVert = vertB;
			}
		}
	}
	if( vertC != fromVert ) {
		// only when the vertex is within the sphere and it has not been visited, we can add it to the bitArrayNext
		if( vertC->distanceToCoord( sphereCenter ) <= radius ) {
			// estimate the position within the bit array
			vertC->getIndexOffsetBit( &bitOffset, &bitNr );
			// check if bit is set:
			if( ! ((1UL << bitNr) & bitArrayVisited[bitOffset] ) ) {
				// if not add to visited and next:
				bitArrayVisited[bitOffset] |= 1UL << bitNr;
				nextArray->insert( vertC );
				nextVert = vertC;
			}
		}
	}
	return nextVert;
}

// rasterization / voxelization => volume integral ------------------------------------------------

void Face::rasterViewFromZ( double *rasterArray, //!< array to write z-values
	                    int    rasterSizeX, //!< array size 
	                    int    rasterSizeY  //!< array size 
	) {
	//! Rasters (renders) the top-view of a Face for use in a height map.
	//!
	//! Will be called typically from Mesh::rasterViewFromZ

	// we follow roughly this approach: http://www.3ddrome.com/articles/triangleraster.php

	// 0. initalize raster:
	if( rasterArray == nullptr ) {
		cerr << "[Face] rasterViewFromZ memory not allocated!" << endl;
		return;
	}
	//cout << "[Face] rasterViewFromZ rastersize: " << rasterSizeX << ", " << rasterSizeY << endl;

	// 1. find edge with longest y-span:
	Vertex* vertMinY = getVertWithMinY();
	Vertex* vertMaxY = getVertWithMaxY();
	Vertex* VertMedY = getOposingVertex( vertMinY, vertMaxY );
	if( VertMedY == nullptr ) {
		cerr << "[Face] rasterViewFromZ no VertMedY found!" << endl;
	}
	int     minY = floor( vertMinY->getY() + 0.5 );
	int     maxY = floor( vertMaxY->getY() + 0.5 );
	int     medY = floor( VertMedY->getY() + 0.5 );
	//cout << "[Face] rasterViewFromZ spans in Y from: " << minY << " to " << medY << " to " << maxY << "." << endl;

	double zCurrLong = vertMinY->getZ();
	double zStepLong = ( vertMaxY->getZ() - vertMinY->getZ() ) / ( maxY - minY );
	double xCurrLong = vertMinY->getX();
	double xStepLong = ( vertMaxY->getX() - vertMinY->getX() ) / ( maxY - minY );

	double zCurrShort1 = vertMinY->getZ();
	double zStepShort1 = ( VertMedY->getZ() - vertMinY->getZ() ) / ( medY - minY );
	double xCurrShort1 = vertMinY->getX();
	double xStepShort1 = ( VertMedY->getX() - vertMinY->getX() ) / ( medY - minY );

	double zCurrShort2 = VertMedY->getZ();
	double zStepShort2 = ( vertMaxY->getZ() - VertMedY->getZ() ) / ( maxY - medY );
	double xCurrShort2 = VertMedY->getX();
	double xStepShort2 = ( vertMaxY->getX() - VertMedY->getX() ) / ( maxY - medY );

	xCurrLong -= xStepLong;
	zCurrLong -= zStepLong;
	xCurrShort1 -= xStepShort1;
	zCurrShort1 -= zStepShort1;
	xCurrShort2 -= xStepShort2;
	zCurrShort2 -= zStepShort2;
	int rasterShortX;
	int rasterLongX;
	int scanLineX0;
	int scanLineX1;
	double scanLineZ0;
	double scanLineZ1;
	double zCurrShort;
	for( int rY=minY; rY<maxY; rY++ ) {
		xCurrLong += xStepLong;
		zCurrLong += zStepLong;
		rasterLongX = floor( xCurrLong + 0.5 );
		if( rY < medY ) {
			// short1 
			xCurrShort1 += xStepShort1;
			zCurrShort1 += zStepShort1;
			zCurrShort   = zCurrShort1;
			rasterShortX = floor( xCurrShort1 + 0.5 );
		} else {
			// short2
			xCurrShort2 += xStepShort2;
			zCurrShort2 += zStepShort2;
			zCurrShort   = zCurrShort2;
			rasterShortX = floor( xCurrShort2 + 0.5 );
		}
		if( ( rY < 0 ) || ( rY >= rasterSizeY ) ) {
			//cout << "[Face] rasterViewFromZ ignore scanline at " << rY << endl;
			continue;
		}
		//cout << "[Face] rasterViewFromZ Scanline: " << rasterShortX << " to " << rasterLongX << endl;
		if( rasterShortX > rasterLongX ) {
			scanLineX0 = rasterLongX;
			scanLineX1 = rasterShortX;
			scanLineZ0 = zCurrLong;
			scanLineZ1 = zCurrShort;
		} else {
			scanLineX0 = rasterShortX;
			scanLineX1 = rasterLongX;
			scanLineZ0 = zCurrShort;
			scanLineZ1 = zCurrLong;
		}
		if( ( scanLineX1 - scanLineX0 ) == 0 ) {
			continue;
		}
		double zCurrStep = ( scanLineZ1 - scanLineZ0 ) / ( scanLineX1 - scanLineX0 );
		double zCurr = scanLineZ0;
		for( int scanX = scanLineX0; scanX < scanLineX1; scanX++ ) {
			if( ( scanX < 0 ) || ( scanX >= rasterSizeX ) ) {
				continue;
			}
			//cout << "[Face] rasterViewFromZ set " << zCurrLong << " at " << scanX << ", " << rY << endl;
			rasterArray[scanX+rY*rasterSizeX] = zCurr;
			zCurr += zCurrStep;
		}
	}

	//cout << "[Face] rasterViewFromZ END." << endl;

}
// Plane functions ---------------------------------------------------------------------------------------------

//! Checks whether the face intersects a plane given in HNF.
bool Face::intersectsPlane(const Vector3D *planeHNF) {

	bool sign1 = (dot3(vertA->getPositionVector(), *planeHNF) + planeHNF->getH() > 0);
	bool sign2 = (dot3(vertB->getPositionVector(), *planeHNF) + planeHNF->getH() > 0);
	bool sign3 = (dot3(vertC->getPositionVector(), *planeHNF) + planeHNF->getH() > 0);

	bool onSameSide =  (sign1 && sign2 && sign3) || (!sign1 && !sign2 && !sign3);
	return(!onSameSide);
}

//! Checks for intersection with another face as outlined by Tomas Moeller in "A Fast Triangle-Triangle Intersection Test".
//! @returns true if intersection found.
bool Face::intersectsFace( const Face* face1 ) const {
	Plane plane0( getVertA()->getPositionVector(), getVertB()->getPositionVector(), getVertC()->getPositionVector() );
	Plane plane1( face1->getVertA()->getPositionVector(), face1->getVertB()->getPositionVector(), face1->getVertC()->getPositionVector() );
	
	std::set<Vertex*> face0_verts{ getVertA(), getVertB(), getVertC() };
	std::set<Vertex*> face1_verts{ face1->getVertA(), face1->getVertB(), face1->getVertC() };
	
	//return false if faces are neighbors
	for( Vertex* v_ptr : face0_verts ) {
		if( face1_verts.count( v_ptr ) == 1 ) {
			return false;
		}
	}
	
	//Check if all vertices of second face are on same side of plane through first face.
	double distance_vertA_to_plane0 = 0.0;
	Vector3D projected_vertA_to_plane0 = face1->getVertA()->getPositionVector();
	if( projected_vertA_to_plane0.projectOntoPlane( plane0.getHNF() ) ) {
		distance_vertA_to_plane0 = dot3( face1->getVertA()->getPositionVector() - projected_vertA_to_plane0, plane0.getNormal() );
	}
	
	double distance_vertB_to_plane0 = 0.0;
	Vector3D projected_vertB_to_plane0 = face1->getVertB()->getPositionVector();
	if( projected_vertB_to_plane0.projectOntoPlane( plane0.getHNF() ) ) {
		distance_vertB_to_plane0 = dot3( face1->getVertB()->getPositionVector() - projected_vertB_to_plane0, plane0.getNormal() );
	}
	
	double distance_vertC_to_plane0 = 0.0;
	Vector3D projected_vertC_to_plane0 = face1->getVertC()->getPositionVector();
	if( projected_vertC_to_plane0.projectOntoPlane( plane0.getHNF() ) ) {
		distance_vertC_to_plane0 = dot3( face1->getVertC()->getPositionVector() - projected_vertC_to_plane0, plane0.getNormal() );
	}
	
	//return false if all vertices on same side of plane through other face
	if( ( distance_vertA_to_plane0 < 0 ) and ( distance_vertB_to_plane0 < 0 ) and ( distance_vertC_to_plane0 < 0 ) ) {
		return false;
	}
	else if( ( distance_vertA_to_plane0 > 0 ) and ( distance_vertB_to_plane0 > 0 ) and ( distance_vertC_to_plane0 > 0 ) ) {
		return false;
	}
	
	//Check if all vertices of first face are on same side of plane through second face.
	double distance_vertA_to_plane1 = 0.0;
	Vector3D projected_vertA_to_plane1 = getVertA()->getPositionVector();
	if( projected_vertA_to_plane1.projectOntoPlane( plane1.getHNF() ) ) {
		distance_vertA_to_plane1 = dot3( getVertA()->getPositionVector() - projected_vertA_to_plane1, plane0.getNormal() );
	}
	
	double distance_vertB_to_plane1 = 0.0;
	Vector3D projected_vertB_to_plane1 = getVertB()->getPositionVector();
	if( projected_vertB_to_plane1.projectOntoPlane( plane1.getHNF() ) ) {
		distance_vertB_to_plane1 = dot3( getVertB()->getPositionVector() - projected_vertB_to_plane1, plane0.getNormal() );
	}
	
	double distance_vertC_to_plane1 = 0.0;
	Vector3D projected_vertC_to_plane1 = getVertC()->getPositionVector();
	if( projected_vertC_to_plane1.projectOntoPlane( plane1.getHNF() ) ) {
		distance_vertC_to_plane1 = dot3( getVertC()->getPositionVector() - projected_vertC_to_plane1, plane0.getNormal() );
	}
	
	//return false if all vertices on same side of plane through other face
	if( ( distance_vertA_to_plane1 < 0 ) and ( distance_vertB_to_plane1 < 0 ) and ( distance_vertC_to_plane1 < 0 ) ) {
		return false;
	}
	else if( ( distance_vertA_to_plane1 > 0 ) and ( distance_vertB_to_plane1 > 0 ) and ( distance_vertC_to_plane1 > 0 ) ) {
		return false;
	}
	
	//Compute intersection line of planes through faces.
	Vector3D interSectLinePosA;
	Vector3D interSectLinePosB;
	if( !plane0.getPlaneIntersection( plane1, interSectLinePosA, interSectLinePosB ) ) {
		return( false );
	}
	
	//Compute intervals on which faces intersect with intersection line.
	Vector3D face0_intersection0;
	Vector3D face0_intersection1;
	Vector3D face1_intersection0;
	Vector3D face1_intersection1;
	
	//Check if faces intersect with plane intersection line, if yes return intersections.
	if( !borderIntersectionWithLine( interSectLinePosA, interSectLinePosB,
	                                 face0_intersection0, face0_intersection1 ) ) {
		return( false );
	}
	
	if( !face1->borderIntersectionWithLine( interSectLinePosA, interSectLinePosB,
	                                        face1_intersection0, face1_intersection1 ) ) {
		return( false );
	}
	
	//Check if intervals overlap.
	//const Vector3D face0_intersection{face0_intersection1 - face0_intersection0};
	//const Vector3D face0_0_to_face1_0{face1_intersection0 - face0_intersection0};
	//const Vector3D face0_0_to_face1_1{face1_intersection1 - face0_intersection0};
	
	//sort intersections. Use if, else if because intersection line might be parallel to one coordinate axis.
	std::array<std::pair<double,int>,4> intersections;
	if( face0_intersection0.getX() != face0_intersection1.getX() ) {
		intersections = { { { face0_intersection0.getX(), 0 }, { face0_intersection1.getX(), 0 }, { face1_intersection0.getX(), 1 }, { face1_intersection1.getX(), 1 } } };
	}
	else if( face0_intersection0.getY() != face0_intersection1.getY() ) {
		intersections = { { { face0_intersection0.getY(), 0 }, { face0_intersection1.getY(), 0 }, { face1_intersection0.getY(), 1 }, { face1_intersection1.getY(), 1 } } };
	}
	else if( face0_intersection0.getZ() != face0_intersection1.getZ() ) {
		intersections = { { { face0_intersection0.getZ(), 0 }, { face0_intersection1.getZ(), 0 }, { face1_intersection0.getZ(), 1 }, { face1_intersection1.getZ(), 1 } } };
	}
	
	std::sort( intersections.begin(), intersections.end() );

	if( intersections.size() == 4 ) {
		std::array<int,4> intersection_order{ intersections[0].second, intersections[1].second, intersections[2].second, intersections[3].second };
		std::array<int,4> intersection1{ 0, 1, 0, 1 };
		std::array<int,4> intersection2{ 0, 1, 1, 0 };
		std::array<int,4> intersection3{ 1, 0, 0, 1 };
		std::array<int,4> intersection4{ 1, 0, 1, 0 };
		if( ( intersection_order == intersection1 ) or ( intersection_order == intersection2 ) or ( intersection_order == intersection3 ) or ( intersection_order == intersection4 ) ) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

// Common usefull functions ------------------------------------------------------------------------------------

//! Finds the intersections of the face with a line.
//! @returns true if successful.
//! Parameters are the line and two references to vectors where the intersections points should be stored.
bool Face::borderIntersectionWithLine(
                const Vector3D& rLinePointA,
                const Vector3D& rLinePointB,
                Vector3D& rIntersection_point_1,
                Vector3D& rIntersection_point_2
) const {
	Vector3D a_projected = getVertA()->getPositionVector().projectOntoLine( rLinePointA, rLinePointB );
	Vector3D b_projected = getVertB()->getPositionVector().projectOntoLine( rLinePointA, rLinePointB );
	Vector3D c_projected = getVertC()->getPositionVector().projectOntoLine( rLinePointA, rLinePointB );
	const Vector3D a_projection_vector{ a_projected - getVertA()->getPositionVector() };
	const Vector3D b_projection_vector{ b_projected - getVertB()->getPositionVector() };
	const Vector3D c_projection_vector{ c_projected - getVertC()->getPositionVector() };
	
	double scalar_product_ab = dot3( a_projection_vector, b_projection_vector );
	double scalar_product_ac = dot3( a_projection_vector, c_projection_vector );
	double scalar_product_bc = dot3( b_projection_vector, c_projection_vector );
	Vertex* vertex_other_side;
	Vertex* vertex_1;
	Vertex* vertex_2;
	
	//sort vertices according to on which side of the line they are
	if( ( scalar_product_ac < 0 ) and ( scalar_product_bc < 0 ) ) {
		vertex_1 = getVertA();
		vertex_2 = getVertB();
		vertex_other_side = getVertC();
	}
	else if( ( scalar_product_ab < 0 ) and ( scalar_product_bc < 0 ) ) {
		vertex_1 = getVertA();
		vertex_2 = getVertC();
		vertex_other_side = getVertB();
	}
	else if( ( scalar_product_ab < 0 ) and ( scalar_product_ac < 0 ) ) {
		vertex_1 = getVertB();
		vertex_2 = getVertC();
		vertex_other_side = getVertA();
	}
	else {
		return( false );
	}

	Vector3D interSectLine1A, interSectLine1B;
	Vector3D interSectLine2A, interSectLine2B;
	if( !lineLineIntersect( rLinePointA, rLinePointB,
	                        vertex_1->getCenterOfGravity(), vertex_other_side->getCenterOfGravity(),
	                        interSectLine1A, interSectLine1B ) ) {
		return( false );
	}
	if( !lineLineIntersect( rLinePointA, rLinePointB,
	                        vertex_2->getCenterOfGravity(), vertex_other_side->getCenterOfGravity(),
	                        interSectLine2A, interSectLine2B ) ) {
		return( false );
	}

	rIntersection_point_1 = interSectLine1A;
	rIntersection_point_2 = interSectLine2A;

	return( true );
}

bool Face::getPointOnWeightedLine( Vector3D* vertNew, Vertex* vert1, Vertex* vert2, double paramPoint, double weight1, double weight2 ) {
	//! Estimates the point with paraPoint on a line defined by vert1 and vert2 having weight1 and weight2.
	//! Returns false in case of an error, e.g: equal weights or vertices having no distance.
	return Primitive::getPointOnWeightedLine( vertNew, vert1->getPositionVector(), vert2->getPositionVector(), paramPoint, weight1, weight2 );
}

// CALCULATIONS ----------------------------------------------------------------

double Face::getLengthAB() const {
	//! Get the length of the edge A->B
	Vector3D dirVec = vertB->getPositionVector() - vertA->getPositionVector();
	double lenVec = dirVec.getLength3();
	return lenVec;
}

double Face::getLengthBC() const {
	//! Get the length of the edge B->C
	Vector3D dirVec = vertC->getPositionVector() - vertB->getPositionVector();
	double lenVec = dirVec.getLength3();
	return lenVec;
}

double Face::getLengthCA() const {
	//! Get the length of the edge C->A
	Vector3D dirVec = vertA->getPositionVector() - vertC->getPositionVector();
	double lenVec = dirVec.getLength3();
	return lenVec;
}

//! Compute the altitude of the line BC to A.
//! @returns the altitude.
double Face::getAltitudeToA() const {
	return vertA->estDistanceToLine( vertB, vertC );
}

//! Compute the altitude of the line CA to B.
//! @returns the altitude.
double Face::getAltitudeToB() const {
	return vertB->estDistanceToLine( vertC, vertA );
}

//! Compute the altitude of the line AB to C.
//! @returns the altitude.
double Face::getAltitudeToC() const {
	return vertC->estDistanceToLine( vertA, vertB );
}

double Face::estimateAlpha() const {
	//! Estimates the angle alpha between C-A-B in radiant.
	//!
	//! http://de.wikipedia.org/wiki/Dreieck#Berechnung_eines_beliebigen_Dreiecks
	double alpha;
	double lengthEdgeA = getLengthBC();
	double lengthEdgeB = getLengthCA();
	double lengthEdgeC = getLengthAB();
	alpha = acos( ( lengthEdgeB*lengthEdgeB + lengthEdgeC*lengthEdgeC - lengthEdgeA*lengthEdgeA ) / ( 2*lengthEdgeB*lengthEdgeC ) );
	return alpha;
}

double Face::estimateBeta() const {
	//! Estimates the angle beta between A-B-C in radiant.
	double beta;
	double lengthEdgeA = getLengthBC();
	double lengthEdgeB = getLengthCA();
	double lengthEdgeC = getLengthAB();
	beta = acos( ( lengthEdgeA*lengthEdgeA + lengthEdgeC*lengthEdgeC - lengthEdgeB*lengthEdgeB ) / ( 2*lengthEdgeA*lengthEdgeC ) );
	return beta;
}

double Face::estimateGamma() const {
	//! Estimates the angle gamma between B-C-A in radiant.
	double gamma;
	double lengthEdgeA = getLengthBC();
	double lengthEdgeB = getLengthCA();
	double lengthEdgeC = getLengthAB();
	gamma = acos( ( lengthEdgeA*lengthEdgeA + lengthEdgeB*lengthEdgeB - lengthEdgeC*lengthEdgeC ) / ( 2*lengthEdgeA*lengthEdgeB ) );
	return gamma;
}

// DEBUGING --------------------------------------------------------------------

void Face::dumpFaceInfo() const {
	//! Dumps information about the Face to stdout.
	cout << "[Face] idx: " << getIndex() << " -> " << vertA->getIndex() << " - " << vertB->getIndex() << " - " << vertC->getIndex() << endl;
	cout << endl;
}

void Face::dumpInfoAsDOT( string fileSuffix ) {
	//! Dumps a graph using the DOT language to an image.
	stringstream dotGraph (stringstream::in | stringstream::out);
/*
	// Edge A-B
	dotGraph << "strict digraph Face_" << getIndex() << " {" << endl;
	dotGraph << "  ";
	dotGraph << edgeAB->getVertA()->getIndex() << " -> ";
	dotGraph << edgeAB->getVertB()->getIndex();
// 	if( edgeAB->isBorder() ) {
// 		dotGraph << " [label=\"" << edgeAB->getIndex() << "\",color=red,weight=2] ";  
// 	} else {
 		dotGraph << " [label=\"" << edgeAB->getIndex() << "\"] ";  
// 	}
	dotGraph << endl;

	// Edge B-C
	dotGraph << "  ";
	dotGraph << edgeBC->getVertA()->getIndex() << " -> ";
	dotGraph << edgeBC->getVertB()->getIndex();
// 	if( edgeBC->isBorder() ) {
// 		dotGraph << " [label=\"" << edgeBC->getIndex() << "\",color=red,weight=2] ";  
// 	} else {
		dotGraph << " [label=\"" << edgeBC->getIndex() << "\",weight=2] ";  
// 	}
	dotGraph << endl;

	// Edge C-A
	dotGraph << "  ";
	dotGraph << edgeCA->getVertA()->getIndex() << " -> ";
	dotGraph << edgeCA->getVertB()->getIndex();
// 	if( edgeCA->isBorder() ) {
// 		dotGraph << " [label=\"" << edgeCA->getIndex() << "\",color=red,weight=2] ";  
// 	} else {
 		dotGraph << " [label=\"" << edgeCA->getIndex() << "\",weight=2] ";  
// 	}
	dotGraph << endl;
*/
	// index -> A
	dotGraph << "  ";
	dotGraph << getIndex() << " -> ";
	dotGraph << vertA->getIndex();
	dotGraph << " [group=\"" << getIndex() << "\"] ";  
	dotGraph << endl;

	// index -> B
	dotGraph << "  ";
	dotGraph << getIndex() << " -> ";
	dotGraph << vertB->getIndex();
	dotGraph << " [group=\"" << getIndex() << "\"] ";  
	dotGraph << endl;

	// index -> C
	dotGraph << "  ";
	dotGraph << getIndex() << " -> ";
	dotGraph << vertC->getIndex();
	dotGraph << " [group=\"" << getIndex() << "\"] ";  
	dotGraph << endl;

	dotGraph << "  ";
	dotGraph << getIndex() << " [shape=triangle]";
	dotGraph << endl;

	dotGraph << "}" << endl;

	writeDotFile( &dotGraph, &fileSuffix );
}

std::array<float, 6> Face::getUVs() const
{
	return mUVs;
}

void Face::setUVs(const std::array<float, 6>& uVs)
{
	mUVs = uVs;
}

unsigned char Face::getTextureId() const
{
    return mTextureId;
}

void Face::setTextureId(unsigned char textureId)
{
    mTextureId = textureId;
}
