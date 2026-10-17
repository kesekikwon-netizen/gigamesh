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

#ifndef MARCHINGFRONT_H
#define MARCHINGFRONT_H

#include <map>
#include <list>
#include <set>
#include <sys/types.h>

#include "vertexofface.h"
#include "face.h"

//!
//! \brief Class handling a marching front. (Layer -1)
//!
//! Either a Face or a Vertex can be used as a seed primitve.
//!
//! Typically called by a Mesh. 
//! 

class MarchingFront {

	public:
		// constructor and deconstructor:
// 		MarchingFront( set<Edge*> seedEdges );
		MarchingFront( Vertex*    useSeedVertex );
		MarchingFront( Face*      useSeedFace );
		~MarchingFront();

		// information retrival:
		std::set<Face*> getFrontFaces();
		std::set<Face*> getVisitedFaces();
		int getVisitedFaces( std::set<Face*>* someFaceList );

		// manipulation by n-rings:
		bool advanceOne();
		bool advanceMulti( int nrOfAdvances = 0 );
		// manipulation by n-rings and euclidian distance:
		bool advanceOneDistToCoord( float maxDist, float x, float y, float z, bool advanceOnParts );
		bool advanceMultiDistToCoord( float maxDist, float x, float y, float z, bool advanceOnParts=true );

	private:
		std::set<Face*>   facesVisited;  //!< Faces already visited.
		std::set<Face*>   frontFaces;    //!< Faces right before the marching front.
		Vertex*      seedVertex;    //!< Pointer to the seed Vertex, will be set when created with MarchingFront( Vertex* seedVertex ).
		Face*        seedFace;      //!< Pointer to the seed Face, will be set when created with MarchingFront( Face* seedFace ).

};

#endif
