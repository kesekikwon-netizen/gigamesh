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
#include <cstdlib>

#include <GigaMesh/mesh/marchingfront.h>

using namespace std;

// constructor and deconstructor -----------------------------------------------

/*
MarchingFront::MarchingFront( set<Edge*> seedEdges ) {
	//! Constructor using a list/set of Edges as seed, initalizing the marching 
	//! front.
	//! 
	//! Example of usage - advance from all borders 3 edge counts into the mesh:
	//!   MarchingFront marchingFront( someMesh.getBorderEdges() );
	//!   marchingFront.advanceMulti( 3 );

	set<Face*> adjacentFaces;	
	set<Edge*>::iterator itSeedEdges;
	set<Face*>::iterator itFaces;
	for( itSeedEdges=seedEdges.begin(); itSeedEdges!=seedEdges.end(); itSeedEdges++ ) {
		adjacentFaces = (*itSeedEdges)->getFaceList();
		for( itFaces=adjacentFaces.begin(); itFaces!=adjacentFaces.end(); itFaces++ ) {
			frontFaces.insert( (*itFaces) );
		}
	}
	seedFace   = NULL;
	seedVertex = NULL;
}
*/

MarchingFront::MarchingFront( Vertex* useSeedVertex ) {
	//! Constructor using a Vertex as seed, initalizing the marching front.
	frontFaces.clear();
	useSeedVertex->getFaces( &frontFaces );
	seedFace   = nullptr;
	seedVertex = useSeedVertex;
}

MarchingFront::MarchingFront( Face* useSeedFace ) {
	//! Constructor using a Face as seed, initalizing the marching front.
	//! 
	//! \todo Test this, becaus it is UNTESTED!
	facesVisited.insert( useSeedFace );
	frontFaces.clear();
	useSeedFace->getNeighbourFaces( &frontFaces );
	seedFace   = useSeedFace;
	seedVertex = nullptr;
}

MarchingFront::~MarchingFront() {
	//! Destructor of the marching front.
	facesVisited.clear();
	frontFaces.clear();
	seedFace   = nullptr;
	seedVertex = nullptr;
}

// Information retrival --------------------------------------------------------

set<Face*> MarchingFront::getFrontFaces() {
	//! Returns all the faces in front of the marching front.
	return frontFaces;
}

set<Face*> MarchingFront::getVisitedFaces() {
	//! Returns all the faces visited by the marching front.
	return facesVisited;
}

int MarchingFront::getVisitedFaces( set<Face*>* someFaceList ) {
	//! Adds all visited faces to a given face list.
	//! Returns number of faces added.
	set<Face*>::iterator itFace;
	for( itFace=facesVisited.begin(); itFace!=facesVisited.end(); itFace++ ) {
		someFaceList->insert( (*itFace) );
	}
	return facesVisited.size();
}

// manipulation by n-rings -----------------------------------------------------

bool MarchingFront::advanceOne() {
	//! Advances the marching front by 1-ring.
	//!
	//! Returns true, when an advance was performed.

	// find new faces along the front faces, but not towards visited faces.
	bool advanced = false;
	set<Face*> newFront;
	set<Face*> newFrontCandidates;

	set<Face*>::iterator itFrontFaces;
	set<Face*>::iterator itFrontCandidateFaces;
	for( itFrontFaces=frontFaces.begin(); itFrontFaces!=frontFaces.end(); itFrontFaces++ ) {
		// fetch faces around the front faces
		newFrontCandidates.clear();
		(*itFrontFaces)->getNeighbourFaces( &newFrontCandidates );
		//cout << "[MarchingFront] fetched " << newFrontCandidates.size() << " Candidates." << endl;

		// check these faces if they have been visited - when not they'll become 
		// the new front, while the old/current front faces have to be added
		// to the visited faces.
		for( itFrontCandidateFaces=newFrontCandidates.begin(); itFrontCandidateFaces!=newFrontCandidates.end(); itFrontCandidateFaces++ ) {
			if( facesVisited.find( (*itFrontCandidateFaces) ) == facesVisited.end() ) {
				newFront.insert( (*itFrontCandidateFaces) );
				advanced = true;
				//cout << "[MarchingFront] advance added a Candidate." << endl;
			} else {
				//cout << "[MarchingFront] advance skipped a Candidate." << endl;
			}
		}
		facesVisited.insert( (*itFrontFaces) );
	}
	frontFaces.swap( newFront );

	return advanced;
}

bool MarchingFront::advanceMulti( int nrOfAdvances ) {
	//! Advances the marching front by n-rings.
	//! If nrOfAdvances <= 0 we advance as far as possible.
	//!
	//! Returns true, when an advance was performed.
	bool advanced = false;

	if( nrOfAdvances > 0 ) {
		for( int i=0; i<nrOfAdvances; i++ ) {
			if( advanceOne() ) {
				advanced = true;
			}
		}
		return advanced;
	}

	bool noAdvanceAnyMore = false;
	while( not( noAdvanceAnyMore )  ) {
		if( advanceOne() ) {
			advanced = true;
		} else {
			noAdvanceAnyMore = true;
		}
	}
	return advanced;
}

// manipulation by n-rings and euclidian distance ------------------------------

bool MarchingFront::advanceOneDistToCoord( float maxDist,       //!< Radius of the sphere (abort criterion)
	                                   float x,             //!< x-coordinate of the sphere (typically: identical to the seed Primitive)
	                                   float y,             //!< y-coordinate of the sphere (typically: identical to the seed Primitive)
	                                   float z,             //!< z-coordinate of the sphere (typically: identical to the seed Primitive)
	                                   bool  advanceOnParts //!< When true, the front advances on to faces, which are partially inside the sphere.
	) {
	//! Advances the marching front by 1-ring only if the new faces are within 
	//! the range (maxDist) of a given coordinate (x,y,z).
	//! 
	//! advanceOnParts determines if Faces are FULLY or PARTIALLY in range.
	//!
	//! Returns true, when an advance was performed.
	//!
	//! Remark: this will be called typcally/only by advanceMultiDistToCoord.

	// find new faces along the front faces, but not towards visited faces.
	bool advanced = false;
	bool considerFace;
	set<Face*> newFront;
	set<Face*> newFrontCandidates;

	set<Face*>::iterator itFrontFaces;
	set<Face*>::iterator itFrontCandidateFaces;
	for( itFrontFaces=frontFaces.begin(); itFrontFaces!=frontFaces.end(); itFrontFaces++ ) {
		// fetch faces around the front faces
		newFrontCandidates.clear();
		(*itFrontFaces)->getNeighbourFaces( &newFrontCandidates );
		//cout << "[MarchingFront] fetched " << newFrontCandidates.size() << " Candidates." << endl;

		// check these faces if they meet the criteria for advancement:
		// 1. the face candidates have not been visited 
		// 2. no Vertex of the face candidate is too far away (> maxDist)
		for( itFrontCandidateFaces=newFrontCandidates.begin(); itFrontCandidateFaces!=newFrontCandidates.end(); itFrontCandidateFaces++ ) {
			if( facesVisited.find( (*itFrontCandidateFaces) ) == facesVisited.end() ) {
				if( advanceOnParts ) {
					considerFace = (*itFrontCandidateFaces)->isInRangeParts( maxDist, x, y, z );
				} else {
					considerFace = (*itFrontCandidateFaces)->isInRange( maxDist, x, y, z );
				}
				if( considerFace ) {
					newFront.insert( (*itFrontCandidateFaces) );
					advanced = true;
					//cout << "[MarchingFront] advance added a Candidate." << endl;
				} else {
					//cout << "[MarchingFront] advance skipped a Candidate (out of range)." << endl;
				}
			} else {
				//cout << "[MarchingFront] advance skipped a Candidate (already visited)." << endl;
			}
		}
		facesVisited.insert( (*itFrontFaces) );
	}
	frontFaces.swap( newFront );

	return advanced;
}

bool MarchingFront::advanceMultiDistToCoord( float maxDist,       //!< Radius of the sphere (abort criterion)
	                                     float x,             //!< x-coordinate of the sphere (typically: identical to the seed Primitive)
	                                     float y,             //!< y-coordinate of the sphere (typically: identical to the seed Primitive)
	                                     float z,             //!< z-coordinate of the sphere (typically: identical to the seed Primitive)
	                                     bool  advanceOnParts //!< When true, the front advances on to faces, which are partially inside the sphere.
	) {
	//! Advances the marching front until no face is within the range (maxDist)
	//! to (x,y,z).
	//!
	//! Returns true, when an advance was performed.
	bool advanced = false;
	bool noAdvanceAnyMore = false;

	while( not( noAdvanceAnyMore )  ) {
		if( advanceOneDistToCoord( maxDist, x, y, z, advanceOnParts ) ) {
			advanced = true;
		} else {
			noAdvanceAnyMore = true;
		}
	}
	return advanced;
}



























