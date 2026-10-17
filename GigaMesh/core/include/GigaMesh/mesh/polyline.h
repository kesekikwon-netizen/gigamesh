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

#ifndef POLYLINE_H
#define POLYLINE_H

#include <list>

#include "primitive.h"
#include "vertex.h"
#include "vertexofface.h"
#include "polyedge.h"

//!
//! \brief Polygonal line class. (Layer 0)
//!
//! ....
//! ....
//!
//! Layer 0
//!

class PolyLine : public Vertex {
	public:
		// Constructor:
		PolyLine();
		PolyLine( Vector3D vertPos, Vector3D vertNormal );
		PolyLine( Vector3D vertPos, Vector3D vertNormal, uint64_t setLabelNr );
		PolyLine( const Plane& rPlaneIntersecting );

		// Destructor:
		~PolyLine();

		enum ePolyIntInvDirection {
			POLY_INTEGRAL_INV_BOTH,     //!< Compute the run-length integral invariant for both directions.
			POLY_INTEGRAL_INV_FORWARD,  //!< Compute the run-length integral invariant in forward direction.
			POLY_INTEGRAL_INV_BACKWARD  //!< Compute the run-length integral invariant in backward direction.
		};

		// Information retrival:
		virtual double    getX() const;
		virtual double    getY() const;
		virtual double    getZ() const;

		// Information retrival, Specific
		        bool      getLengthAbs( double* rLength );
				bool      getEdgeLengths( std::vector<double>* rEdgeLengths );
				bool      getEdgeFuncVals( std::vector<double>* rFuncVals );
		        bool      getIntersectPlane( Plane** rIntersectPlane );

		// Labeling using Vertex labels - specific for Polylines!
		//        bool      vertLabelGet( int* getLabelNr );

		bool  isClosed();
		bool  getCurvatureSmoothMax( double* rMaxCurv );
		bool  getCurvatureSmoothMin( double* rMinCurv );
		bool  getCurvatureSmoothMinMax( double* rMinCurv, double* rMaxCurv );

		bool closeLine();
		bool estCurvature( bool absolut=false );
		bool estCurvatureSmooth( double gaussWidth );
		bool compIntInv( double rIIRadius, ePolyIntInvDirection rDirection );
		bool compIntInv( int rVertNr, double rIIRadius, ePolyIntInvDirection rDirection );
		bool compIntInvAngle( double rIIRadius );
		bool compIntInvAngle( int rVertNr, double rIIRadius );
		bool getExtrema( std::set<Vertex*>* someVerts, double gaussWidth, bool absolut );
		bool getNeighboursRunLen( int rVertNr, double rDist, std::vector<int>* rNeighVerts, std::vector<double>* rDists, std::vector<double>* rWeights );
		size_t  getSafeIndex( size_t someIdx );

		// Creation of the polyline:
		void  addFront( Vector3D rNewFrontPos, Vector3D rPosNormal, Face* rFromFace=nullptr, Face::eEdgeNames rFromEdge=Face::EDGE_NONE );
		void  addBack(  Vector3D rNewBackPos,  Vector3D rPosNormal, Face* rFromFace=nullptr, Face::eEdgeNames rFromEdge=Face::EDGE_NONE );
		void  addFront( Vertex* rNewFrontVert, Face* rFromFace=nullptr, Face::eEdgeNames rFromEdge=Face::EDGE_NONE );
		void  addBack(  Vertex* rNewBackVert,  Face* rFromFace=nullptr, Face::eEdgeNames rFromEdge=Face::EDGE_NONE );
		bool  addFrontNoDupes( Vertex* rNewFrontVert, Face* rFromFace=nullptr, Face::eEdgeNames rFromEdge=Face::EDGE_NONE );
		bool  addBackNoDupes(  Vertex* rNewBackVert,  Face* rFromFace=nullptr, Face::eEdgeNames rFromEdge=Face::EDGE_NONE );
		size_t   compileLine( std::set<labelLine*>* unsortedLines );
		// Post-creation of the polyline:
		bool  addVerticesTo( std::vector<Vertex*>* rSomeVertList );

		// Extrude
		bool  extrudeAxis( const Vector3D &rAxisTop, const Vector3D &rAxisBottom, std::vector<Vertex*>* rVerticesToAppend, std::vector<Face*>* rFacesToAppend );

		// Modification of the polyline
		PolyLine* advanceFuncValThres( double funcValThres );
		bool      copyNormalToVertices();

		// e.g. for OpenGL:
		int      length();
		int*     getVertexIndices();
		uint*    getVertexIndicesUnsigned();
		bool     getVertexCoords( std::vector<double>* rCoords );
		bool     getVertexCoordsInPlane( std::vector<Vector3D>* rCoords, bool rProjectToPlaneUsingAxis );
		bool     getBoundingBox( double* rMinX, double* rMinY, double* rMinZ, \
		                         double* rMaxX, double* rMaxY, double* rMaxZ, \
		                         bool rProjectToPlane, bool rProjectToPlaneUsingAxis );
		double*  getVertexNormals();
		bool     compVertAvgNormal();
		bool     compVertAvgCog();
		bool     getVertexNormalsFuncVal( std::vector<double>* rNormals, double rScale=1.0, bool rShiftMin=true, bool rUsePolyNormal=true );
		Vertex*  getVertexRef( int rIndex );
		Face*    getFaceRef( int rIndex );

		// Common information:
		virtual int getType();

		// Comparison of function value - Heap/sorting related
		static  bool sortByLenAsc( PolyLine* rPoly1, PolyLine* rPoly2 );
		static  bool sortByLenDesc( PolyLine* rPoly1, PolyLine* rPoly2 );

		// Debuging:
		void  dumpCurvMat();
		void  dumpCurvSmoothMat();
		void  dumpRunLenMat();

	private:
		std::vector<PolyEdge*> mEdgeList;     //!< List of Edgels of the polyline organized by a std::vector.

		Plane*            mPlaneUsed;    //!< For intersections i.e. profile lines: Rember the plane used to compute this polygonal line.
};

#endif
