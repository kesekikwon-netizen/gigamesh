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

#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "vertex.h"
#include "face.h"
#include "geodentry.h"

// Circular dependencies:
class Vertex;
class Face;

//!
//! \brief Class handling edges having a geodesic distance class. (Layer -0.5)
//!
//! ...
//!
//! Layer -0.5
//!

class EdgeGeodesic {
	//friend class ...;

	public:
		// Constructor and deconstructor:
		EdgeGeodesic( Face* rFromFace, Face::eEdgeNames rEdgeIdx, GeodEntry* rGeoDistA, GeodEntry* rGeoDistB );
		~EdgeGeodesic() = default;

		// Information retrival
		Face*       getFace();
		bool        getGeoDistA( double* rGeodDist );
		bool        getGeoDistB( double* rGeodDist );
		bool        getGeodAngleA( double* rGeodAngle );
		bool        getGeodAngleB( double* rGeodAngle );
		GeodEntry*  getGeoDistARef();
		GeodEntry*  getGeoDistBRef();
		double      getGeoDistMin();
		double      getGeoDistMax();
		Face::eEdgeNames  getEdgeIdx();

		Vertex*     getVertA();
		Vertex*     getVertB();
		bool        getPositionA( Vector3D* vertAPos );
		bool        getPositionB( Vector3D* vertBPos );
		bool        getFuncValueA( double* funcValA );
		bool        getFuncValueB( double* funcValB );

		// Comparison - Heap related
		static bool shorterThan( EdgeGeodesic* edge1, EdgeGeodesic* edge2 );

		// Debuging:
		void        dumpInfo();

	private:
		Face*       mFromFace;
		Face::eEdgeNames mEdgeIdx; // Face::eEdgeNames
		GeodEntry*  mGeoDistA;
		GeodEntry*  mGeoDistB;
};

#endif
