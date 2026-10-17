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

#ifndef POLYEDGE_H
#define POLYEDGE_H

#include "vertex.h"

#include "face.h"

//!
//! \brief Edge class for Polygonal lines. (Layer 0)
//!
//! ....
//! ....
//!
//! Layer 0
//!

class PolyEdge {
	friend class PolyLine;

	public:
		PolyEdge( Vertex* rSomeVert, Face* rFromSomeFace, Face::eEdgeNames rFromSomeEdge=Face::EDGE_NONE );
		~PolyEdge();

	// Position:
		bool    getPosition( Vector3D* rPos );
		bool    getPositionTransformed( Vector3D* rPos, const Matrix4D& rTransform );
		bool    getNormal( Vector3D* rNormal );

	// Function value:
		bool    setFuncValue( double  rVal );
		bool    getFuncValue( double* rVal ) const;

	private:
		Vertex* mVertPoly;        //!< Required reference to a Vertex defining the position in 3D.
		Face*   mFromFace;        //!< Optional reference to Face e.g. used to compute vertPoly.
		Face::eEdgeNames  mFromEdge;        //!< Optional reference to the edge of the Face fromFace.
		float   mCurvature;       //!< Optional curvature.
};

#endif
