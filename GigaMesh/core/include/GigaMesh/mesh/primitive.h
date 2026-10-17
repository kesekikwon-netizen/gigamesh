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

#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#define _PRIMITIVE_NOT_INDEXED_         -1
#define _PRIMITIVE_FIRST_INDEX_          0

#define _PRIMITIVE_STATE_ERROR_         -1
#define _PRIMITIVE_STATE_SOLO_           0
#define _PRIMITIVE_STATE_BORDER_         1
#define _PRIMITIVE_STATE_MANIFOLD_       2
#define _PRIMITIVE_STATE_NON_MANIFOLD_   3

#define _DIRECTORY_DOT_                  "debug_graphs/"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <set>

#include <cmath>
#include <cstdlib>
#include <cfloat>

// generic C++
#include "gmcommon.h"
#include "bitflagarray.h"
#include "vector3d.h"
#include "matrix4d.h"

class GeodEntry;

//!
//! \brief generic class for general mesh functionality. Mother of all classes. (Layer 0)
//! 
//! This is the generic class for mesh handling with general methods like 
//! setting/getting indices or RGB-color.
//! 
//! Layer 0
//! 

class Primitive : public BitFlagArray  {

	public:
		// Constructers and Destructor:
		Primitive();
		virtual ~Primitive() = default;

		enum primitiveType{ IS_DESTROYED,
		                    IS_UNDEFINED,
		                    IS_VERTEX,
		                    IS_FACE,
		                    IS_SPHERE,
		                    IS_RECTBOX,
		                    IS_POLYLINE,
		                    IS_PLANE,
		                    IS_MESH
		};

		// Indexing:
		virtual bool   setIndex( int setIdx );
		virtual int    getIndex() const;
		virtual std::string getIndexStr();
		virtual int    getIndexOriginal();

		// Color managment:
		virtual bool   setRGB( unsigned char setTexR, unsigned char setTexG, unsigned char setTexB );
		        bool   getRGBInverted( double* texRGBInv );
		        bool   convertHSVtoRGB( double* texHSVin, double* texRGBout );
		        bool   convertRGBtoHSV( double* texRGBin, double* texHSVout );
		        bool   getHSV( double* rHSV );
		virtual bool   setAlpha( double rVal );
		// Color mapping:
		        bool   setTrafficLight( double phase, double sat=1.0, double val=1.0 );
		        bool   setBlackWhiteBlack( double phase );
		        bool   setWhiteBlackWhite( double phase );
		        bool   setBlackWhite( double phase );
		        bool   setWhiteBlack( double phase );

		// Function value
		virtual bool   setFuncValue( double  setVal );
		virtual bool   getFuncValue( double* rGetVal ) const;

		// Transformation
		virtual bool   applyTransfrom( Matrix4D* rTransMat );
		virtual bool   applyMeltingSphere( double rRadius, double rRel=1.0 );

	public:
		// labeling:
		virtual bool   setLabel( uint64_t rSetLabelNr );
		virtual bool   setLabelNone();
		virtual bool   setLabelBackGround();
		virtual bool   getLabel( uint64_t& rGetLabelNr ) const;
		virtual bool   isLabled() const;
		virtual bool   isLabelBackGround();
		        bool   getLabelColor( unsigned char* colRGB, int colorMap=Primitive::COLMAP_BREWER_PAIRED );
	protected:      bool   getColor( unsigned char* colRGB, int idxCol, int colorMap=Primitive::COLMAP_BREWER_PAIRED );
	public:
		enum labelColors{ COLMAP_BREWER_PAIRED,  //!< Paired qualitative colormap for 11 classes from http://colorbrewer.org
		                  COLMAP_BREWER_SET1     //!< Qualitative colormap for 5 classes with dark colors.
		                };

		// Information retrival:
		virtual double   getX() const;
		virtual double   getY() const;
		virtual double   getZ() const;
		virtual double   getNormalX();
		virtual double   getNormalY();
		virtual double   getNormalZ();
		        Vector3D getCenterOfGravity();
		        bool     getTransformedCenterOfGravity( const Matrix4D& rTransMat, Vector3D& rCoGTrans );
		        Vector3D getNormal( bool normalized=true );
		        bool     getPlaneHNF( Vector3D* rPlaneHNF );
		        bool     getPlaneHNF( double* rPlaneHNF );
		virtual bool     copyXYZTo( float*  rCoordXYZ );
		virtual bool     copyXYZTo( double* rCoordXYZ ) const;
		virtual bool     copyNormalXYZTo( float*  rNormalXYZ, bool rNormalized=true );
		virtual bool     copyNormalXYZTo( double* rNormalXYZ, bool rNormalized=true );
		virtual bool     copyNormalXYZTo( double* rNormalXYZ, double rSetLength );
		        double   getNormalSpherical( double* phi, double* theta, double* radius );
		// Colors:
		virtual bool     copyRGBTo( unsigned char* rColorRGB );
		virtual bool     copyRGBATo( unsigned char* rColorRGBA );

	virtual	int      getType();
		std::string   getTypeStr();
		std::string   getTypeStrFor( int primitiveType );
		std::string   getCoutPref();

		// Normal
		void     addNormalTo( double* someVec );
		bool     addNormalXYZTo( double (&rNormalXYZ)[3], bool rNormalized=false );
		double   getNormalLen();
		double   angleToNormal( Vector3D someVec );

		// feature vector
		virtual bool   assignFeatureVec( const double *rAttachFeatureVec, unsigned int rSetFeatureVecLen );
		virtual bool   copyFeatureVecTo( double* fetchFeatureVec ) const;
		virtual bool   getFeatureVectorElements( std::vector<double>& rFeatVec ) const;
		virtual bool   getFeatureVecWavletDecompTo( double** featureDecomp, int* featureDecompLen );
		virtual double getFeatureDistTo( double* someFeatureVec );
		virtual bool         getFeatureElement( unsigned int rElementNr, double* rElementValue );
		virtual unsigned int getFeatureVectorLen();

		// simple geometric/vector functionality:
		double estAngleBetweenVectors( double nx1, double ny1, double nx2, double ny2 );
		double estAngleBetweenVectors( double nx1, double ny1, double nz1, double nx2, double ny2, double nz2 );
		double estSphericalCoordsThetaNormal( Vector3D normalOther );
		double estSphericalCoordsThetaNormal( double normalOtherX=0.0, double normalOtherY=0.0, double normalOtherZ=1.0 );
		double estSphericalCoordsPhiNormal();

		// Plane related - requires Point+Normal
		        bool     getIntersectionFacePlaneLineDir( Vector3D* rRayPos, Vector3D* rRayDir, Vector3D* rRayIntersect );
		        bool     getIntersectionFacePlaneLinePos( const Vector3D& rayTop, const Vector3D& rayBot, Vector3D& rayIntersect );
		        bool     getIntersectionFacePlaneEdge( Vector3D* rayTop, Vector3D* rayBot, Vector3D* rayIntersect );
		        bool     getDistanceToPoint( Vector3D* somePos, double* dist );
		// Center of gravity related
		        bool     getDistanceFromCenterOfGravityTo( double const rSomePos[3], double* rDist ) const;

		//! \todo rethink the idea of using the same flags for all primitives, which is not very efficent. e.g. FLAG_FACE* will never be set for anything else than a face or maybe a vertex.
		enum ePrimitiveFlags {
			FLAG_BELONGS_TO_POLYLINE  = BitFlagArray::FLAG_00,  //!< Flag indicating that this Primitive belongs to a polyline.
			FLAG_LABEL_NOT            = BitFlagArray::FLAG_01,  //!< for Primitives, which have not been labeled, but will get a label ID assigned.
			FLAG_LABEL_BACKGR         = BitFlagArray::FLAG_02,  //!< for Primitives, which do not belong to a label at all (AKA background label).
			FLAG_NORMAL_SET           = BitFlagArray::FLAG_03,  //!< Flag for pre-calculated normals.
			FLAG_FACE_STICKY          = BitFlagArray::FLAG_04,  //!< Flag for faces sticking together.
			FLAG_FACE_ZERO_AREA       = BitFlagArray::FLAG_05,  //!< Flag for faces having no area.
			FLAG_LOCAL_MAX            = BitFlagArray::FLAG_06,  //!< Flag for local maximum - typically function value.
			FLAG_LOCAL_MIN            = BitFlagArray::FLAG_07,  //!< Flag for local minimum - typically function value.
			FLAG_SYNTHETIC            = BitFlagArray::FLAG_08,  //!< Flag for synthetic primitives, e.g. estimated by filling a hole.
			FLAG_MARCHING_FRONT_ABORT = BitFlagArray::FLAG_09,  //!< Flag used to tag Primitve's as abort criteria for a marching front.
			FLAG_SELECTED             = BitFlagArray::FLAG_10,  //!< Flag used to tag Primitve's selected either by the user or a method.
			FLAG_MANUAL               = BitFlagArray::FLAG_11,  //!< Flag used to tag a primitive added manually by an user.
			FLAG_CIRCLE_CENTER        = BitFlagArray::FLAG_12,  //!< Flag used to tag a vertex, which was computed from a circle matching method.
			FLAG_CORTEX               = BitFlagArray::FLAG_13   //!< Flag for cortex regions in archaeological lithic illustration (Raczynski-Henk 2017)
		};

		// Common usefull functions
		bool getPointOnWeightedLine( Vector3D* vertNew, Vector3D  vert1, Vector3D  vert2, double paramPoint, double weight1, double weight2 );
		bool getPointOnWeightedLine( Vector3D* vertNew, Vector3D* vert1, Vector3D* vert2, double paramPoint, double weight1, double weight2 );
		bool getPointOnIsoLine( Vector3D* rVertNew, Primitive* rPrimA, Primitive* rPrimB, double rIsoVal );

	protected:
		// help for debugging:
		        void  dumpInfo();
				bool  writeDotFile( std::stringstream* dotGraph, std::string* fileSuffix, bool removeDot=true );
};

#endif
