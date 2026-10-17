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

#ifndef PLANE_H
#define PLANE_H

#include "vertex.h"
#include "face.h"

class Vertex;

//!
//! \brief Plane line class. (Layer 0)
//!
//! ....
//! ....
//!
//! Layer 0
//!

class Plane : public Primitive {
	public:
		// Constructor and deconstructor:
		Plane( Vector3D rPosA, Vector3D rPosB, Vector3D rPosC );
		Plane( Vector3D rPosA, Vector3D rNormal );
		Plane( Vector3D* rPlaneHNF );
		Plane( Face& f );
		Plane( const Plane& rOther );
		Plane();
		~Plane();

		// Information retrival:
		double   getX() const override;
		double   getY() const override;
		double   getZ() const override;
		double   getNormalX() override;
		double   getNormalY() override;
		double   getNormalZ() override;
		Vector3D         getHNF() const;

		        double   getHNFA() const;
		        double   getHNFB() const;
		        double   getHNFC() const;
		        double   getHNFD() const;

		int      getType() override;

		        bool     isValid();

		// Transformation
		bool     applyTransfrom( Matrix4D* transMat ) override;

		enum ePlaneVerts{ PLANE_VERT_A, PLANE_VERT_B, PLANE_VERT_C };

		enum ePlaneDefinedBy{
			PLANE_UNDEFINED,               //! The plane object was created, but not properly defined.
			THREE_POSITIONS,               //! Plane defined by three points in R3.
			POSITION_AND_NORMAL,           //! Plane defined using a position vector and a normal (direction - not an axis!).
			HESSE_NORMAL_FORM,             //! Plane defined as HNF.
			AXIS_POINTS_AND_POSITION       //! Plane using the tip ("Spitze") and bottom ("Schaft") of an axis and the center of gravity of a Primitive.
		};

		// Retrieve information
		        bool            getVertCoords( double* rCoords ) const;
		        bool            getVertPositions( Vector3D& rPosA, Vector3D& rPosB, Vector3D& rPosC ) const;
		        bool            getVertHNFCOG( Vector3D& rPosHNF, Vector3D& rPosCOG ) const;
		        bool            getFlipped() const;
		        ePlaneDefinedBy getDefinedBy() const;

		// Operations:
		double classifyPoint( Vertex* point );
		bool   setData( ePlaneVerts rPosIdx, Vector3D* rPos );
		bool   setPlaneHNF( Vector3D* rPlaneHNF );
		bool   setPlaneHNF( Vector3D* rPosA, Vector3D* rPlaneHNF );
		bool   setPlaneByAxisAndPosition( const Vector3D& rAxisTop, const Vector3D& rAxisBottom, const Vector3D& rPos );
		bool   flipPlane();

		// Calculations
		bool   getPointProjected( Vector3D rPoint, Vector3D* rProjection );
		bool   getChangeOfBasisTrans( Matrix4D* rTransMat, bool rUseAxisToY );
		bool   getPlaneIntersection( Plane& rOther, Vector3D& rLinePosA, Vector3D& rLinePosB );

	private:
		// Vertices
		Vertex* mVertA;            //!< reference to Vertex A
		Vertex* mVertB;            //!< reference to Vertex B
		Vertex* mVertC;            //!< reference to Vertex C

		        void   setPositions(  const Vector3D& rPosA, const Vector3D& rPosB, const Vector3D& rPosC  );
		virtual double calcNormalX();
		virtual double calcNormalY();
		virtual double calcNormalZ();
		        bool   updateHNFCOG();

		Vector3D mHNF;             //! Normal of plane and distance from origin
		Vector3D mCog;             //! Center of gravity vector

		bool            mWasFlipped;     //!< Rember that this plane had another orientation.
		ePlaneDefinedBy mDefinitionType; //!< Store the type of definition used.
};

bool solve3dimLinearSystem(const Vector3D lg1, const Vector3D lg2, const Vector3D lg3, double& result1, double& result2, double &result3 );
bool solve2dimLinearSystem( const Vector3D lg1, const Vector3D lg2, double& result1, double& result2 );

std::ostream& operator<<( std::ostream& o, const Plane& p );

#endif
