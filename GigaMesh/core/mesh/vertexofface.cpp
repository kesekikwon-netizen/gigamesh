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

#include <GigaMesh/mesh/vertexofface.h>

#include <GigaMesh/mesh/face.h>

#include <GigaMesh/logging/Logging.h>

#define VERTEXOFFACEINITDEFAULTS \
	mAdjacentFacesNr( 0 ),   \
	mAdjacentFaces( NULL )

using namespace std;

//! Constructor
VertexOfFace::VertexOfFace()
    : VERTEXOFFACEINITDEFAULTS {
	// Pass through.
}

//! Constructor e.g. to be used when a Vertex list is read from a file.
VertexOfFace::VertexOfFace( const int rSetIdx, const sVertexProperties& rSetProps )
	: Vertex( rSetIdx, rSetProps ), VERTEXOFFACEINITDEFAULTS  {
	// Pass through.
}

//! Constuctor using a position Vector3D.
VertexOfFace::VertexOfFace( Vector3D vertPos )
	: Vertex( vertPos ), VERTEXOFFACEINITDEFAULTS {
	 // Pass through.
}

//! Constructor using three coordinates.
VertexOfFace::VertexOfFace( unsigned int rSetIdx, double rPosX, double rPosY, double rPosZ )
	: Vertex( rSetIdx, rPosX, rPosY, rPosZ ), VERTEXOFFACEINITDEFAULTS {
	// Pass through.
}

//! Destructor
VertexOfFace::~VertexOfFace() {
	if( mAdjacentFacesNr > 0 ) {
		LOG::debug() << "[VertexOfFace::" << __FUNCTION__ << "] ERROR: still connected to " << mAdjacentFacesNr << " faces!\n";
	}
	if( mAdjacentFaces != nullptr ) {
		delete[] mAdjacentFaces;
		mAdjacentFacesNr  = 0;
		mAdjacentFaces = nullptr;
	}
}

//! Estimates the average normal of the adjacent faces and stores them into
//! the vertice's normal vector.
//! This function displays an error on the console in case degenerated faces are encountered.
//! These are typically faces having an area of zero.
//!
//! @returns false in case of an error - e.g. no adjacent faces or an invalid normal. True otherwise.
bool VertexOfFace::estNormalAvgAdjacentFaces() {
	// We can stop if it is a solo vertex.
	if( mAdjacentFacesNr == 0 ) {
		unsetNormal();
		return( false );
	}
	// Compute a weigthed average:
	Vector3D faceNormal;
	for( int i=0; i<mAdjacentFacesNr; ++i ) {
		// We use the area of the face as weight by using getNormal( false )
		Vector3D adjacentFaceNormal = mAdjacentFaces[i]->getNormal( false );
		// Neglect degenerated faces:
		if( !isnormal( adjacentFaceNormal.getLength3() ) ) {
			LOG::warn() << "[VertexOfFace::" << __FUNCTION__ << "] ERROR: No valid normal for face No. " << mAdjacentFaces[i]->getIndex() << "!\n";
			LOG::warn() << "[VertexOfFace::" << __FUNCTION__ << "]        adjacentFaceNormal: " << adjacentFaceNormal << "\n";
			LOG::warn() << "[VertexOfFace::" << __FUNCTION__ << "]        Check for zero-area faces!\n";
			continue;
		}
		// We apply a counterweight using the angle of the face for this vertex.
		double angleAtThisVertex = mAdjacentFaces[i]->getAngleAtVertex( this );
		faceNormal += adjacentFaceNormal * angleAtThisVertex;
	}
	// Invalid normal
	if( !isnormal( faceNormal.getLength3() ) ) {
		// This error will be shown often for defective meshs - to be treated by calling method:
		// cerr << "[VertexOfFace::" << __FUNCTION__ << "] ERROR: no valid normal set for Vertex " << this->getIndex() << " !" << endl;
		unsetNormal();
		return( false );
	}
	// Final step: normalize the valid normal vector.
	faceNormal.normalize3();
	if( !setNormal( faceNormal.getX(), faceNormal.getY(), faceNormal.getZ() ) ) {
		LOG::warn() << "[VertexOfFace::" << __FUNCTION__ << "] ERROR: setNormal failed!\n";
		return( false );
	}
	return( true );
}

//! Estimates the total area of the adjacent faces.
double VertexOfFace::get1RingArea() {
	Vector3D faceNormal;
	for( int i=0; i<mAdjacentFacesNr; ++i ) {
		if( mAdjacentFaces[i] == nullptr )  {
			continue;
		}
		faceNormal += mAdjacentFaces[i]->getNormal( false );
	}
	return faceNormal.getLength3();
}

//! Number of adjacent Faces, which have to be pre-determined
//! and stored in VertexOfFace::mAdjacentFacesNr!
//!
//! @returns the number of adjacent Faces (when pre-determined).
inline uint64_t VertexOfFace::get1RingFaceCount() const {
	return( mAdjacentFacesNr );
}

//! Returns the sum of the angles of the adjacent faces enclosing this Vertex.
double VertexOfFace::get1RingSumAngles() {
	double angleSum = 0.0;
	for( int i=0; i<mAdjacentFacesNr; ++i ) {
		angleSum += mAdjacentFaces[i]->getAngleAtVertex( this );
	}
	return angleSum;
}

//! Relates to Mesh::fetchSphereBitArray passes thru to neighbourfaces to reach the vertices in 1-ring distance.
Vertex* VertexOfFace::advanceInSphere( double*        sphereCenter,        //!< X, y and z-coordinate of the sphere's center
									   double         radius,              //!< Radius of the sphere.
                                       uint64_t* vertBitArrayVisited, //!< Bit array tagging visited vertices.
									   set<Vertex*>*  nextArray,           //!< Vertices along the marching front.
                                       uint64_t* faceBitArrayVisited, //!< Bit array tagging visited faces.
									   bool           rOrderToFuncVal,     //!< Flag for visual debuging: will set the faces function value to the sequenze nr.
									   double*        rSeqNr               //!< Sequenze number - only used when rOrderToFuncVal is set.
	) {
	Vertex* nextVert = nullptr;
	Vertex* tmpVert;
	uint64_t  bitOffset;
	uint64_t  bitNr;
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		// sanity check;
		if( mAdjacentFaces[i] == nullptr ) {
			continue;
		}

		// check if already visited:
		mAdjacentFaces[i]->getIndexOffsetBit( &bitOffset, &bitNr );
		if( ((static_cast<uint64_t>(1)<<bitNr) & faceBitArrayVisited[bitOffset] ) ) {
			continue;
		}
		tmpVert = mAdjacentFaces[i]->advanceInSphere( this, sphereCenter, radius, vertBitArrayVisited, nextArray );

		// Set sequenze number, when requested:
		if( rOrderToFuncVal ) {
			if( not( faceBitArrayVisited[bitOffset] & static_cast<uint64_t>(1)<<bitNr ) ) {
				mAdjacentFaces[i]->setFuncValue( (*rSeqNr) );
				(*rSeqNr) += 1.0;
			}
		}

		// Add face as visited
		faceBitArrayVisited[bitOffset] |= static_cast<uint64_t>(1)<<bitNr;

		if( tmpVert != nullptr ) {
			nextVert = tmpVert;
		}
	}
	return nextVert;
}

//! Relates to Mesh::fetchSphereBitArray1R adds vertices and faces in 1-ring distance to the bit arrays.
bool VertexOfFace::mark1RingVisited(
                uint64_t* rVertBitArrayVisited, //!< Bit array tagging visited vertices.
                uint64_t* rFaceBitArrayVisited, //!< Bit array tagging visited faces.
                bool           rOrderToFuncVal,      //!< Flag for visual debuging: will set the faces function value to the sequenze nr.
                double*        rSeqNr                //!< Sequenze number - only used when rOrderToFuncVal is set.
) {
	uint64_t  bitOffset;
	uint64_t  bitNr;
	for( int i=0; i<mAdjacentFacesNr; ++i ) {
		// Sanity check;
		if( mAdjacentFaces[i] == nullptr ) {
			continue;
		}
		// Check if already visited:
		mAdjacentFaces[i]->getIndexOffsetBit( &bitOffset, &bitNr );
		if( ((static_cast<uint64_t>(1)<<bitNr) & rFaceBitArrayVisited[bitOffset] ) ) {
			continue;
		}
		// When not add vertices as visited too:
		mAdjacentFaces[i]->markVertsVisited( rVertBitArrayVisited );
		// Add face as visited
		rFaceBitArrayVisited[bitOffset] |= static_cast<uint64_t>(1)<<bitNr;
		// Set sequenze number, when requested:
		if( rOrderToFuncVal ) {
			mAdjacentFaces[i]->setFuncValue( (*rSeqNr) );
			(*rSeqNr) += 1.0;
		}
	}
	return true;
}

//! Checks if this Vertex is a local Minimum using the 1-ring
//! stored in mAdjacentFaces.
//! 
//! The according flag will be set.
bool VertexOfFace::isFuncValLocalMinimum() {
	this->clearFlag( FLAG_LOCAL_MIN );
	double funcValue;
	if( !getFuncValue( &funcValue ) ) {
		return( false );
	}
	if( !std::isfinite( funcValue ) ) {
		return( false );
	}
	double neighbourFuncVal;
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		neighbourFuncVal = mAdjacentFaces[i]->getFuncValMinExcluding( this );
		//cout << "[VertexOfFace::isFuncValLocalMinimum] " << neighbourFuncVal << " < " << FUNCTION_VALUE << endl;
		if( neighbourFuncVal <= funcValue ) {
			return( false );
		}
	}
	this->setFlag( FLAG_LOCAL_MIN );
	return( true );
}

//! Checks if this Vertex is a local Maximumg using the 1-ring
//! stored in mAdjacentFaces.
//! 
//! The according flag will be set.
bool VertexOfFace::isFuncValLocalMaximum() {
	this->clearFlag( FLAG_LOCAL_MAX );
	double funcValue;
	if( !getFuncValue( &funcValue ) ) {
		return( false );
	}
	if( !std::isfinite( funcValue ) ) {
		return( false );
	}
	double neighbourFuncVal;
	for( int i=0; i<mAdjacentFacesNr; ++i ) {
		neighbourFuncVal = mAdjacentFaces[i]->getFuncValMaxExcluding( this );
		//cout << "[VertexOfFace::isFuncValLocalMaximum] " << neighbourFuncVal << " < " << FUNCTION_VALUE << endl;
		if( neighbourFuncVal >= funcValue ) {
			return( false );
		}
	}
	this->setFlag( FLAG_LOCAL_MAX );
	return( true );
}

//! Applies an 1-ring median filter to the function value.
//! Note: Applying weights like 1-ring area or distance to
//!       1-ring neighbours does not improve the results.
//! @returns false in case of an error. True otherwise.
bool VertexOfFace::funcValMedianOneRing(
                double* rMedianValue,  //!< to return the computed mean value
                double  rMinDist       //!< smallest edge length to be expected
) {
	set<Face*> oneRingFaces;
	getFaces( &oneRingFaces );
	double accuArea = 0.0;

	// Fetch values
	vector<pair<double,double>> oneRingValues;
	for( auto const& currFace: oneRingFaces ) {
		double currFuncVal;
#ifdef OLD1RINGSMOOTH
		currFace->getFuncVal1RingThird( this, rMinDist, &currFuncVal );
		double currArea = sin( currFace->getAngleAtVertex( this ) );
#else
		double currArea = _NOT_A_NUMBER_DBL_;
		currFace->getFuncVal1RingSector( this, rMinDist, currArea, currFuncVal );
#endif
		oneRingValues.emplace_back( make_pair(currFuncVal,currArea) );
		accuArea += currArea;
	}
	sort( oneRingValues.begin(), oneRingValues.end() );

	double startRange = 0.0;
	double endRange = 0.0;
	for( auto const& currPair: oneRingValues ) {
		endRange += currPair.second;
		if( ( startRange/accuArea < 0.5 ) && ( endRange/accuArea >= 0.5 ) ) {
			//cout << currPair.first << " " << currPair.second << " | " << startRange/accuArea << " - " << endRange/accuArea << endl;
			(*rMedianValue) = currPair.first;
			return( true );
		}
		startRange = endRange;
	}

	// At this point we have encountered an error:
	return( false );
}

//! Applies an 1-ring mean filter to the function value.
//!
//! See also: Face::getFuncVal1RingSector (used)
//!           Mesh::funcVertMedianOneRing (calling method)
//!
//! @returns false in case of an error. True otherwise.
bool VertexOfFace::funcValMeanOneRing(
                double* rMeanValue,    //!< to return the computed mean value
                double  rMinDist       //!< smallest edge length to be expected
) {
	set<Face*> oneRingFaces;
	getFaces( &oneRingFaces );

	double accuFuncVals = 0.0;
	double accuArea = 0.0;

	// Fetch values
	for( auto const& currFace: oneRingFaces ) {
		double currFuncVal;
#ifdef OLD1RINGSMOOTH
		currFace->getFuncVal1RingThird( this, rMinDist, &currFuncVal );
		// SWS - Seite Winkel Seite
		// Two edges (b,c) and the enclosed angle: A=b*c*sin(alpha). Because of a=b=const. skipped.
		double currArea = sin( currFace->getAngleAtVertex( this ) );
#else
		double currArea = _NOT_A_NUMBER_DBL_;
		currFace->getFuncVal1RingSector( this, rMinDist, currArea, currFuncVal );
#endif
		accuFuncVals += currFuncVal * currArea;
		accuArea += currArea;
	}

	// Finally compue the mean value:
	accuFuncVals /= accuArea;

	// Write back
	(*rMeanValue) = accuFuncVals;
	return( true );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Feature vector smoothing --------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Median value for each element of the feature vector's elements.
//!
//! @returns false in case of an error. True otherwise.
bool VertexOfFace::getFeatureVecMedianOneRing( vector<double>& rMedianValues, double rMinDist ) {
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] ERROR: NOT yet IMPLEMENTED!\n";
	return( false );
}

//! Mean value for each element of the feature vector's elements.
//!
//! @returns false in case of an error. True otherwise.
bool VertexOfFace::getFeatureVecMeanOneRing(
                vector<double>& rMeanValues, //!< used to return the mean values.
                double rMinDist              //!< typically the minmal edge length of all faces in the mesh.
) {
	vector<Face*> oneRingFaces;
	getFaces( &oneRingFaces );

	uint64_t featureVecLen =  rMeanValues.size(); // has to be the same value as mFeatureVecLen.
	vector<double> accuFeatureVec(featureVecLen,0.0);
	double accuArea = 0.0;

//	for( uint64_t i=0; i<oneRingFaces.size(); i++ ) {
//		Face* currFace = oneRingFaces[i];
//		// Values to be pre-computed
//		s1RingSectorPrecomp oneRingSecPre;
//		if( !currFace->get1RingSectorConst( this, rMinDist,
//		                          oneRingSecPre ) ) {
//			return( false );
//		}
//		// Compute new feature vector
//		vector<double> currFeatureVec(featureVecLen,_NOT_A_NUMBER_DBL_);
//		currFace->getFeatureVec1RingSector( this, oneRingSecPre, currFeatureVec );
//		for( uint64_t i=0; i<featureVecLen; i++ ) {
//			accuFeatureVec.at( i ) += currFeatureVec.at( i ) * oneRingSecPre.mSectorArea;
//		}
//		accuArea += oneRingSecPre.mSectorArea;
//	}

	// Fetch values
	for( Face* oneRingFace : oneRingFaces )
	{
		Face* currFace = oneRingFace;
		// Values to be pre-computed
		s1RingSectorPrecomp oneRingSecPre;
		if( !currFace->get1RingSectorConst( this, rMinDist,
		                          oneRingSecPre ) )
		{
			return( false );
		}

		// Compute new feature vector
		vector<double> currFeatureVec(featureVecLen, _NOT_A_NUMBER_DBL_);
		currFace->getFeatureVec1RingSector( this, oneRingSecPre, currFeatureVec );

//		for( uint64_t i = 0; i < featureVecLen; i++ )
//		{
//			accuFeatureVec.at( i ) += currFeatureVec.at( i ) * oneRingSecPre.mSectorArea;
//		}

		std::transform(currFeatureVec.begin(),
		                currFeatureVec.end(),
		                accuFeatureVec.begin(),
		                [oneRingSecPre](double currentValue) {
			                return currentValue*oneRingSecPre.mSectorArea;
		});

		accuArea += oneRingSecPre.mSectorArea;
	}

//	// Finally compue the mean value and write back
//	for( uint64_t i=0; i<featureVecLen; ++i ) {
//		rMeanValues.at( i ) = accuFeatureVec.at( i ) / accuArea;
//	}

	std::transform(accuFeatureVec.begin(),
	                accuFeatureVec.end(),
	                rMeanValues.begin(),
	                [accuArea](double currentValue) {
		                return currentValue / accuArea;
	});

	return( true );
}

bool VertexOfFace::getFeatureVecMeanOneRing(
                const double& rMinDist,
                const vector<s1RingSectorPrecomp>& r1RingSecPrecomp,
                vector<double>& rMeanValues
) {
	return( true );
}

//! Pre-compute 1-ring sector i.e. geodesic circle segment.
//!
//! @returns false in case of an error. True otherwise.
bool VertexOfFace::get1RingSectors(
                const double& rNormDist,
                vector<s1RingSectorPrecomp>& r1RingSectorPrecomp
) {
	set<Face*> oneRingFaces;
	getFaces( &oneRingFaces );

	for( auto const& currFace: oneRingFaces ) {
		s1RingSectorPrecomp oneRingSecPre;
		currFace->get1RingSectorConst( this, rNormDist, oneRingSecPre );
		r1RingSectorPrecomp.push_back( oneRingSecPre );
	}

	return( true );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- "Transformation" for shelling ---------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Move the vertex by the offset value
Vertex *VertexOfFace::applyNormalShift(float offsetDistance, int index) {
    Vertex *offsetVertex = Vertex::applyNormalShift(offsetDistance, index);
    VertexOfFace* ret = new VertexOfFace(offsetVertex->getPositionVector());
    Vector3D normal = offsetVertex->getNormal(true);
    ret->setNormal(normal.getX(),normal.getY(),normal.getZ());
    return ret;
}

//! Retrieves all Vertices in 1-ring distance and adds them to the given list.
//! Will exclude the calling Vertex.
void VertexOfFace::getNeighbourVertices( set<Vertex*>* someVertList ) {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		mAdjacentFaces[i]->copyVerticesTo( someVertList, this );
	}
}

//! Adds the adjacent vertices of the 1-ring to rSomeVerts, when they have not been tagged in rVertBitArrayVisited.
//! Will also tag the vertices added.
bool VertexOfFace::getAdjacentVerticesExcluding( vector<Vertex*>* rSomeVerts, uint64_t* rVertBitArrayVisited ) {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		mAdjacentFaces[i]->addAndTagUntaggedVerts( rSomeVerts, rVertBitArrayVisited );
	}
	return true;
}

//! Return the reference to next vertex on the border within the 1-ring neighbourhood.
//! Considers the bit-array for vertices tagged unvisited.
Vertex* VertexOfFace::getAdjacentNextBorderVertex( uint64_t* rVertBitArrayUnVisited ) {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		Vertex* nextVert = mAdjacentFaces[i]->getNextBorderVertex( this, rVertBitArrayUnVisited );
		if( nextVert != nullptr ) {
			return nextVert;
		}
	}
	return nullptr;
}

//! When one of the next Vertex within orientated 1-ring neighbourhood
//! is part of the list, the Vertex is returned - NULL otherwise.
Vertex* VertexOfFace::getConnection( set<Vertex*>* someVertList, bool reverse ) {
	Vertex* nextVert = nullptr;
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		nextVert = mAdjacentFaces[i]->getConnection( this, someVertList, reverse );
		if( nextVert != nullptr ) {
			return nextVert;
		}
	}
	return nextVert;
}

// MESH-SETUP ------------------------------------------------------------------

//! Add an adjacent Face.
void VertexOfFace::connectToFace( Face* someFace ) {
	if( someFace == nullptr ) {
		LOG::debug() << "[VertexOfFace::" << __FUNCTION__ << "] can not connect - NO Face given (NULL).\n";
		return;
	}
	// when empty:
	if( mAdjacentFacesNr == 0 ) {
		mAdjacentFacesNr = 1;
		mAdjacentFaces = new Face*[mAdjacentFacesNr]();
		mAdjacentFaces[0] = someFace;
		return;
	}
	// check if already connected:
	if( isAdjacent( someFace ) ) {
		LOG::debug() << "[VertexOfFace::" << __FUNCTION__ << "] can not re-connect - the Face is already adjacent.\n";
		return;
	}
	// creat new array, size+1:
	Face** newmAdjacentFaces = new Face*[mAdjacentFacesNr+1]();
	// copy:
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		newmAdjacentFaces[i] = mAdjacentFaces[i];
	}
	// add at last position:
	newmAdjacentFaces[mAdjacentFacesNr] = someFace;
	// remove old:
	delete[] mAdjacentFaces;
	// set new:
	mAdjacentFaces = newmAdjacentFaces;
	mAdjacentFacesNr++;
}

//! Remove an adjacent Face (e.g. when removed).
void VertexOfFace::disconnectFace( Face* someFace ) {
	if( someFace == nullptr ) {
		LOG::debug() << "[VertexOfFace::" << __FUNCTION__ << "] can not disconnect - NO Face given (NULL).\n";
		return;
	}
	if( mAdjacentFacesNr <= 0 ) {
		LOG::debug() << "[VertexOfFace::" << __FUNCTION__ << "] can not disconnect - NO Faces adjacent.\n";
		return;
	}
	if( !isAdjacent( someFace ) ) {
		LOG::debug() << "[VertexOfFace::" << __FUNCTION__ << "] can not disconnect - the Face is NOT adjacent.\n";
		return;
	}
	if( mAdjacentFacesNr == 1 ) {
		// We reach this point, when the last face is removed!
		mAdjacentFacesNr = 0;
		delete[] mAdjacentFaces;
		mAdjacentFaces = nullptr;
		return;
	}
	//! \todo properly test this
	Face** newmAdjacentFaces = new Face*[mAdjacentFacesNr-1]();
	int newIdx = 0;
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		if( mAdjacentFaces[i] == someFace ) {
			continue;
		}
		newmAdjacentFaces[newIdx] = mAdjacentFaces[i];
		newIdx++;
	}
	delete[] mAdjacentFaces;
	mAdjacentFaces = newmAdjacentFaces;
	mAdjacentFacesNr--;
}

//! Test if a given face is adjacent to this Vertex.
bool VertexOfFace::isAdjacent( Face* someFace ) {
	if( mAdjacentFacesNr == 0 ) {
		return false;
	}

	if( mAdjacentFaces ) {
		for( int i=0; i<mAdjacentFacesNr; i++ ) {
			if( mAdjacentFaces[i] == someFace ) {
				return true;
			}
		}
	}

	else
	{
		LOG::error() << "[VertexOfFace::" << __FUNCTION__ << "] ERROR: unexpected state: mAdjacentFacesNr: " << mAdjacentFacesNr << " while mAdjacentFaces is NULL!\n";
	}

	return false;
}

//! Typically called when a Face is initalized. Adds all Faces
//! having this Vertex and the otherVert, but is not the
//! calling Face.
void VertexOfFace::getFaces( Vertex* otherVert, set<Face*>* neighbourFaces, Face* callingFace ) {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		if( mAdjacentFaces[i] == callingFace ) {
			continue;
		}
		if( mAdjacentFaces[i]->getOposingVertex( this, otherVert ) == nullptr ) {
			continue;
		}
		neighbourFaces->insert( mAdjacentFaces[i] );
	}
}

//! Adds all adjacent Faces of this Vertex to the given list/set of Faces.
//! Because we use a set, there will be NO duplicate faces.
//!
//! Typically called by Mesh::removeVertices()
void VertexOfFace::getFaces( set<Face*>* someFaceList ) {
	for( int i=0; i<mAdjacentFacesNr; ++i ) {
		someFaceList->insert( mAdjacentFaces[i] );
	}
}

//! Adds all adjacent Faces of this Vertex to the given list/set of Faces.
//! Because we use a vector, there will can be duplicate faces, which is a rather odd case.
void VertexOfFace::getFaces( vector<Face*>* someFaceList ) {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		someFaceList->push_back( mAdjacentFaces[i] );
	}
}

// Mesh checking ----------------------------------------------------------------------------

//! For sanity checks
bool VertexOfFace::belongsToFace() {
	return true;
}

//! Returns the number of faces connected/related to this Vertex.
int VertexOfFace::connectedToFacesCount() {
	return mAdjacentFacesNr;
}

//! Returns true, when related/connected to ZERO faces and ZERO edges.
bool VertexOfFace::isSolo() {
	if( mAdjacentFacesNr > 0 ) {
		return false;
	}
	return true;
}

//! Returns true, when the Vertex is part of one or more border edges and
//! therefore on the border of a mesh.
//!
//! Solo vertices are NOT considered as BORDER vertices.
//!
//! @returns true in case the vertex is along the mesh's border. False otherwise as well as for solo vertices.
bool VertexOfFace::isBorder() {
	// No adjacent faces means solo vertex.
	if( mAdjacentFacesNr == 0 ) {
		return( false );
	}
	// When there are less than three faces adjacent, the vertex has to be part of the mesh border.
	if( mAdjacentFacesNr < 3 ) {
		return( true );
	}
	// Check all other cases.
	int countBorderFaces = 0;
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		if( mAdjacentFaces[i]->isBorder() ) {
			countBorderFaces++;
		}
	}
	if( countBorderFaces >= 2 ) {
		if( mAdjacentFacesNr >= 3 ) {
			// do a dance around the Vertex:
			set<Face*> facesVisited;
			Face* nextFace    = nullptr;
			Face* currentFace = mAdjacentFaces[0];
			Face* firstFace   = currentFace;
			//cout << "[VertexOfFace::isBorder] currentFace " << currentFace->getIndexOriginal() << endl;
			facesVisited.insert( currentFace );
			nextFace = currentFace->getNextFaceWith( this, &facesVisited );
			while( nextFace != nullptr ) {
				//cout << "[VertexOfFace::isBorder] nextFace " << nextFace->getIndexOriginal() << endl;
				currentFace = nextFace;
				nextFace = currentFace->getNextFaceWith( this, &facesVisited );
				facesVisited.insert( currentFace );
			}
			// if we could dance thru around all adjacent faces and the face we startet is the last visitied, then we are not on a border!
			facesVisited.erase( firstFace );
			nextFace = currentFace->getNextFaceWith( this, &facesVisited );
			if( nextFace == firstFace ) {
				if( mAdjacentFacesNr == static_cast<int>(facesVisited.size())+1 ) {
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

//! Returns true, when the Vertex is part of manifold faces (only).
bool VertexOfFace::isManifold() {
	return not( isNonManifold() );
}

//! Returns true, when the Vertex is part of one or more non-manifold
//! edges/faces.
bool VertexOfFace::isNonManifold() {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		if( mAdjacentFaces[i]->isNonManifold() ) {
			return true;
		}
	}
	return false;
}

//! Determines if this vertex is the only connection between different parts
//! of the mesh. This (rare) connection is kind of generic double cone (in
//! respec to the meshs graph not actual geometry).
//! It is kind of a non-manifold construct with a single Vertex instead of
//! and Edge.
bool VertexOfFace::isDoubleCone() {
	// prevent some nasty messages and unnecessary checks
	if( mAdjacentFacesNr <= 1 ) {
		return( false );
	}

	// do a dance around the Vertex:
	set<Face*> facesVisited;
	Face* nextFace = nullptr;
	Face* currentFace = mAdjacentFaces[0];
	//cout << "[VertexOfFace::isDoubleCone] currentFace " << currentFace->getIndexOriginal() << endl;
	facesVisited.insert( currentFace );
	nextFace = currentFace->getNextFaceWith( this, &facesVisited );
	while( nextFace != nullptr ) {
		//cout << "[VertexOfFace::isDoubleCone] nextFace " << nextFace->getIndexOriginal() << endl;
		currentFace = nextFace;
		nextFace = currentFace->getNextFaceWith( this, &facesVisited );
		facesVisited.insert( currentFace );
	}
	// in case of a border vertex we also have to dance around in the other direction:
	currentFace = mAdjacentFaces[0];
	nextFace = currentFace->getNextFaceWith( this, &facesVisited );
	while( nextFace != nullptr ) {
		//cout << "[VertexOfFace::isDoubleCone] nextFace " << nextFace->getIndexOriginal() << endl;
		currentFace = nextFace;
		nextFace = currentFace->getNextFaceWith( this, &facesVisited );
		facesVisited.insert( currentFace );
	}
	//cout << "[VertexOfFace::isDoubleCone] facesVisited.size = " << facesVisited.size() << endl;
	//cout << "[VertexOfFace::isDoubleCone] faceList.size     = " << faceList.size() << endl;
	//cout << "[VertexOfFace::isDoubleCone] ------------------------------------------------------------" << endl;
	// during our dance we should have visitied all neighbouring faces:
	if( mAdjacentFacesNr == static_cast<int>(facesVisited.size()) ) {
		return( false );
	}
	// if not we have a very very bad structure and
	return( true );
}

//! Determines if this vertex belongs to at least one face
//! having an area of zero.
bool VertexOfFace::isPartOfZeroFace() {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		if( mAdjacentFaces[i]->getFlag( FLAG_FACE_ZERO_AREA ) ) {
			return( true );
		}
	}
	return( false );
}

//! Determines if this vertex belongs to an edge connecting
//! faces with improper i.e. inverted orientation.
bool VertexOfFace::isInverse() {
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		if( mAdjacentFaces[i]->isInverseOnEdge( this ) ) {
			return( true );
		}
	}
	return( false );
}

// DEBUGING --------------------------------------------------------------------

//! Dumps information about the Vertex to stdout.
void VertexOfFace::dumpInfo() {
	Vertex::dumpInfo();
	for( int i=0; i<mAdjacentFacesNr; i++ ) {
		cout << " " << mAdjacentFaces[i];
	}
	cout << endl;
}
