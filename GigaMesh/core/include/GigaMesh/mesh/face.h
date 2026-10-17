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

#ifndef FACES_H
#define FACES_H

#include <map>
#include <algorithm>
#include <deque>

#include "primitive.h"

class Plane;
class EdgeGeodesic;

using uint = unsigned int;

// Circular dependencies:
class Vertex;
class VertexOfFace;
class Face;
class Plane;
struct labelLine; // see below class Face

struct s1RingSectorPrecomp { //! Used to store pre-computed values for 1-ring circle sectors.
	Vertex* mVertOppA;
	Vertex* mVertOppB;
	double  mRatioCA;
	double  mRatioCB;
	double  mCenterOfGravityDist;
	double  mSectorArea;
};

//!
//! \brief Faces (or Triangles). (Layer 0)
//!
//! Faces (or Triangles). (Layer 0)
//!
//! Layer 0
//!

class Face : public Primitive {

	public:
		// constructor and deconstructor:
		Face( unsigned int rIndex, VertexOfFace *setA, VertexOfFace *setB, VertexOfFace *setC );
		virtual ~Face();

		// Element enumeration
		enum eEdgeNames : unsigned int {
			EDGE_NONE,          //!< No edge specified.
			EDGE_AB,            //!< Edge between Vertex A and B.
			EDGE_BC,            //!< Edge between Vertex B and C.
			EDGE_CA,            //!< Edge between Vertex C and A.
			EDGE_NON_MANIFOLD   //!< Edge(s) not being manifold.
		};

		// functions to retrive information:
		        Vertex*  getVertA() const;
		        Vertex*  getVertB() const;
		        Vertex*  getVertC() const;
		        VertexOfFace*  getVertAoF();
		        VertexOfFace*  getVertBoF();
		        VertexOfFace*  getVertCoF();
		        unsigned int getVertAIndex();
		        unsigned int getVertBIndex();
		        unsigned int getVertCIndex();
		        bool         copyFacePropsTo( sFaceProperties& faceProps ) const;

				bool     getMidPoint( eEdgeNames rEdge, std::vector<VertexOfFace*>* rVertexList );
				bool     getVertABC( std::set<VertexOfFace*>* rSomeVerts );
				bool     getVertABC( std::set<Vertex*>* rSomeVerts );

		// Indexing:
				bool     setIndex( int rIndex ) override;
				int      getIndex() const override;
		virtual void     getIndexOffsetBit( uint64_t* rBitOffset, uint64_t* rBitNr );
		        void     getIndexOffsetBitEdge( int* bitOffset, int* bitNr, int edgeIdx );
				bool     markVertsVisited( uint64_t* rVertBitArrayVisited );
				bool     addAndTagUntaggedVerts( std::vector<Vertex*>* rSomeVerts, uint64_t* rVertBitArrayVisited );

		// Information retrival:
				double   getX() const override;
				double   getY() const override;
				double   getZ() const override;
				double   getNormalX() override;
				double   getNormalY() override;
				double   getNormalZ() override;
				bool     copyNormalXYZTo( float*  rNormalXYZ, bool rNormalized=true ) override;
				bool     copyNormalXYZTo( double* rNormalXYZ, bool rNormalized=true ) override;
		// Colors:
				bool     copyRGBTo( unsigned char* rColorRGB )   override;
				bool     copyRGBATo( unsigned char* rColorRGBA ) override;

		        bool     getMaxAngle( double* rMaxAngle );
		        bool     getMinAngle( double* rMaxAngle );

		// Function value
				bool     setFuncValue( double  setVal ) override;
				bool     getFuncValue( double* rSetVal ) const override;
		// Comparison of function value - Heap/sorting related
		static  bool     funcValLower( Face* rPrim1, Face* rPrim2 );
		static  bool     funcValLowerLabled( Face* rPrim1, Face* rPrim2 );
		static  bool     sortLabelsFirst( Face* rPrim1, Face* rPrim2 );
		static  bool     sortByIndex( Face* rPrim1, Face* rPrim2 );

		        double   getAngleAtVertex( const Vertex* vertABC ) const;            // retrieve the angle next to an arbitrary vertex
		        bool     requiresVertex( const Vertex* vertexRequired ) const;                   // returns true if vertexRequired matches A, B or C
		        bool     requiresOneOrMoreVerticesOf( const std::set<Vertex*>& rVertexList ) const;    // returns true if A,B or C is within the list (set)
		        bool     getMinDistTo( double const rSomePos[3], double* rDist );
		        double   getMinDistTo( double x, double y, double z );                  // minimum Distance of the Distances of A, B and C to (x,y,z)
		        double   getMaxDistTo( double x, double y, double z );                  // maximum Distance of the Distances of A, B and C to (x,y,z)
		        double   isInRange( double maxDist, double x, double y, double z );      // checks if A, B and C are ALL in range (maxDist) of (x,y,z)
		        double   isInRangeParts( double maxDist, double x, double y, double z ); // checks if A, B or C are in range (maxDist) of (x,y,z)
		        double   isInRangeParts( double maxDist, double* someXYZ );
		        int      verticesInRange( double maxDist, double* someXYZ );
		        bool     getEdgeLengths( double* rLenABC );
		        bool     getAltitudes( double* rAlitudesABC );
		        bool     getAngles( double* rAnglesABC );

		// Bounding box
		        double   getMaxX(); // bounding-box related: maximum x
		        double   getMinX(); // bounding-box related: minimum x
		        double   getMaxY(); // bounding-box related: maximum y
		        double   getMinY(); // bounding-box related: minimum y
		        double   getMaxZ(); // bounding-box related: maximum z
		        double   getMinZ(); // bounding-box related: minimum z
		        Vertex*  getVertWithMinX();
		        Vertex*  getVertWithMaxX();
		        Vertex*  getVertWithMinY();
		        Vertex*  getVertWithMaxY();
		        Vertex*  getVertWithMinZ();
		        Vertex*  getVertWithMaxZ();

		// Other geometric properties
		        double   getEdgeLenMax();
		        bool     getEdgeLenMin( double* rLenMin );
		        bool     getAltitudeMin( double* rAltitudeMin );

		// ...
			int      getType() override;
			bool     surfaceintegralinvariant( int nradii, double* radii, double* area, Vertex* rseed1 );
			//check if p is in triangle or on border of triangle
			bool     pointontriangle(const Vector3D* p) const;
			bool     pointintriangle(Vector3D* p);
			bool     pointininnertriangle(Vector3D* p);
			double   csecarea(double r, Vector3D* s1, Vector3D* s2, Vertex* rseed);

		// Transformation
		virtual bool     applyTransfrom( Matrix4D* transMat ) override;
		        bool     applyReOrient();

		// Labeling - common functions, inherited from Primitive.
				bool     setLabel( uint64_t rSetLabelNr ) override;
				bool     setLabelNone() override;
				bool     getLabel( uint64_t& rGetLabelNr ) const override;
				bool     isLabled() const override;
				bool     isLabelBackGround() override;
		        bool     isLabelMix();
		        bool     isLabelVoronoiCorner();
				bool     getLabelLines( std::set<labelLine*>* labelLineCollection );
		// Labeling using Vertex labels - specific for Faces!
		        bool     vertLabelGet( uint64_t& rGetLabelNr ) const;
		        bool     vertLabelBackGround();
		        bool     vertLabelLabelBorder( bool* isLabelLabelBorder, int* labelFromBorder );

		// function value related
		        bool     getFuncValVertAvg( double* rFuncValVertAvg );
		        bool     getFuncVal1RingThird( Vertex* rVert1RingCenter, double rNormDist, double* rFuncVal1RingThird );
		        bool     get1RingSectorConst( const Vertex* rVert1RingCenter, const double& rNormDist, s1RingSectorPrecomp& r1RingSecPre );
		        bool     getFuncVal1RingSector( const Vertex* rVert1RingCenter, const double& rNormDist, double& rSectorArea, double& rSectorFuncVal );
		        bool     getInterpolVertFuncVal( Vertex** rNewVert, Face** rFromFace, eEdgeNames rEdgeIdx, double rFuncValThres );
		        bool     getInterpolVertFuncVal( Vertex** rNewVertA, Vertex** rNewVertB, eEdgeNames rEdgeIdx, double rFuncValThres );
		        double   getFuncValMinExcluding( Vertex* someVert );
		        double   getFuncValMaxExcluding( Vertex* someVert );
				bool     getFuncValVertRidge( std::set<Vertex*>* rRidgeVerts );
		        bool     isOnFuncValIsoLine( double isoThres );
				bool     getFuncValIsoPoint( double isoThres, Vector3D* isoPoint, Face** faceNext, bool searchForward, Face** faceVisited );
		// Feature vector related
				bool     getFeatureVec1RingSector( const Vertex* rVert1RingCenter, const s1RingSectorPrecomp& r1RingSecPre,
				                                    std::vector<double>& rFeatureVec1RingSector ) const;

		// used for some mesh-checking:
		        bool     hasSyntheticVertex( unsigned int& rIsSynthetic );   // number of vertices having the FLAG_SYNTHETIC set
		        bool     hasBorderVertex(    unsigned int& rIsBorder );      // number of vertices being part of a border
		        bool     hasBorderEdges(     unsigned int& rIsBorder );      // number of edges being part of a border
		        bool     isNonManifold(); // when connected to ONE or MORE non-manifold edges OR has to border edges with an vertex connected to other faces
		        bool     isManifold();    // when connected to manifold edges only
		        bool     isBorder();      // when connected to ONE or more border edges
		        bool     isSolo();        // when connected to THREE border EDGES
		        bool     isInverse();     // when the orientation of the edges do not match the adjacent faces
		        bool     isInverseOnEdge( const VertexOfFace* rVert );
		        bool     isEdgeOrientationMatching( const VertexOfFace* rVert1, const VertexOfFace* rVert2 );
		        int      getState();      // returns _PRIMITIVE_STATE_*

		// mesh setup:
		        void     reconnectToFaces();  // re-connects this face to its neighbouring faces
		        void     connectToFaces();    // connects this face to its neighbouring faces
		        double   getAreaNormal();
		        bool     getVolumeDivergence( double& rVolumeDX, double& rVolumeDY, double& rVolumeDZ );
		        bool     getVolumeToPlane( double* rVolume, bool* rPlanePos, Plane* rPlane );

		// mesh manipulation:
		        void     disconnectFace( Face* rBelonged2Face );
		        bool     invertFaceOrientation();

		// Navigation:
		        bool         getOposingEdgeAndVertices( Vertex* rVertFrom, eEdgeNames* rEdgeIdx, Vertex** rVert1, Vertex** rVert2 );
		        bool         getOposingVertices( const Vertex* rVertRef, Vertex*& rVertOpp1, Vertex*& rVertOpp2 ) const;
		        Vertex*      getOposingVertex( Vertex* vert1, Vertex* vert2 );
		        Vertex*      getOposingVertex( Face* neighFace );
		        Vertex*      getOposingVertex( Face* rNeighFace, eEdgeNames* rEdgeIdxAC, eEdgeNames* rEdgeIdxCB );
		        Face*        getNeighbourFace( eEdgeNames rEdgeIdx );
		        Face*        getNeighbourFace( eEdgeNames rEdgeIdx, eEdgeNames* rAdjaEdge1, eEdgeNames* rAdjaEdge2 );
		        void         getEdgesNotAtFace( Face* rSomeFace, eEdgeNames* rAdjaEdge1, eEdgeNames* rAdjaEdge2 );
				bool         getNeighbourFaces( std::set<Face*>* rSomeFaces );
		        unsigned int getNeighbourFaceCount();
				Face*        getNextFaceWith( Vertex* someVert, std::set<Face*>* facesToExclude );
				void         getNeighbourFacesExcluding( std::set<Face*>* neighboursSelected, std::set<Face*>* facesToExclude );
				void         copyVerticesTo( std::set<Vertex*>* someVertexList, Vertex* vertToExclude=NULL );
				Vertex*      getNextBorderVertex( Vertex* rVertStart, uint64_t* rVertBitArrayUnVisited );
				Vertex*      getConnection( Vertex* fromVert, std::set<Vertex*>* someVertList, bool reverse );
		        Vertex*      getVertexFromEdgeA( eEdgeNames rEdgeIdx );
		        Vertex*      getVertexFromEdgeB( eEdgeNames rEdgeIdx );

		// Geodesics:
				bool        getGeodesicEdgesSeed( std::deque<EdgeGeodesic*>* frontEdges,
				                                  std::map<Vertex*,GeodEntry*>* geoDistList,
				                                  uint64_t* faceBitArray, int faceNrBlocks, bool weightFuncVal );

		// Surface integrals:
		        bool intersectsSphere1( Vector3D positionVec, double radius );
		        bool intersectsSphere1( Vertex* vert1, Vertex* vert2, Vector3D positionVec, double radius ); //! \todo make private?
		        bool intersectsSphere2( Vector3D positionVec, double radius );
		        bool intersectsSphere2( Vertex* vert1, Vertex* vert2, Vector3D positionVec, double radius ); //! \todo make private?
		        int  intersectSphereSpecial( Vector3D positionVec, double radius, Vector3D** interSections );
				int  getSphereIntersections( Vector3D positionVec, double radius, std::vector<Vector3D>* pointsOfIntersection );
		        int  pointOfSphereEdgeIntersection( Vertex* vert1, Vertex* vert2, Vector3D positionVec, double radius, Vector3D* interSec1, Vector3D* interSec2 ); //! \todo make private?

				Vertex* advanceInSphere( Vertex* fromVert, double* sphereCenter, double radius, uint64_t* bitArrayVisited, std::set<Vertex*>* nextArray );

		// plane
				bool intersectsPlane(const Vector3D* planeHNF);
		        bool intersectsFace( const Face* face2 ) const;

		// rasterization / voxelization => volume integral
		        void rasterViewFromZ( double *rasterArray, int rasterSizeX, int rasterSizeY );

		// Common usefull functions
		bool borderIntersectionWithLine( const Vector3D& rLinePointA, const Vector3D& rLinePointB, Vector3D & rIntersection_point_1, Vector3D & rIntersection_point_2 ) const;

		        bool getPointOnWeightedLine( Vector3D* vertNew, Vertex*   vert1, Vertex*   vert2, double paramPoint, double weight1, double weight2 );

		// debugging:
		        void    dumpFaceInfo() const;
				void    dumpInfoAsDOT( std::string fileSuffix="" );

				[[nodiscard]] std::array<float, 6> getUVs() const;
				void setUVs(const std::array<float, 6>& uVs);

				[[nodiscard]] unsigned char getTextureId() const;
				void setTextureId(unsigned char textureId);

	private:
		// Vertices - Connectivity:
		VertexOfFace* vertA;            //!< reference to index of Vertex A
		VertexOfFace* vertB;            //!< reference to index of Vertex B
		VertexOfFace* vertC;            //!< reference to index of Vertex C

		// Normal
		double  mNormalXYZ[3];    //!< Normal vector for the face - has to be initalized.

		// UV-Coordinates
		std::array<float,6>   mUVs;          //!< UV coordinates of the three vertices of the face

		// Texture-Id
		unsigned char mTextureId = 0;        //!< id of the texture-file assiciated to this face

		// Indexing - required for bit arrays and to be maintained properly!
		int mIndex;      //!< Stores the actual index, which may change due to manipulation, while idxOri stores the original index.
		// Function Value:
		double       mFuncValue;         //!< Function value.

		// Neighbourhood:
		unsigned short mNeighbourFacesNonManifold; //!< for any extra faces one encounters.
		Face**         mNeighbourFaces;            //!< is minimum size of 3 for each neighbour per edge. no neighbour == NULL. any extra (=non-manifold) face will be added at index > 3.

		void    addNonManifold( Face* rExtraFace, bool* rFaceWasAlreadyAdded );

		// Compute edge lengths:
		double  getLengthAB() const;
		double  getLengthBC() const;
		double  getLengthCA() const;
		// Compute altitudes:
		double  getAltitudeToA() const;
		double  getAltitudeToB() const;
		double  getAltitudeToC() const;
		// Compute angles:
		double  estimateAlpha() const;  //!< angle between C-A-B
		double  estimateBeta()  const;   //!< angle between A-B-C
		double  estimateGamma() const;  //!< angle between B-C-A
};

//! \todo MOVE to a more suitable location - probably: vector3d.h/.cpp
enum eLineSphereCases {
	LSI_NO_INTERSECT_LINE, //!< No intersection of the line.
	LSI_NO_INTERSECT_EDGE, //!< No intersection of the edge AKA line segment.
	LSI_ONE_INTERSECT_P1,  //!< One intersection within the line segment -- position is stored in the position vector 1.
	LSI_ONE_INTERSECT_P2,  //!< One intersection within the line segment -- position is stored in the position vector 2.
	LSI_TWO_INTERSECT,     //!< Both intersections within the line segment.
	LSI_TANGENT_EDGE,      //!< The edge is tangent to the sphere.
	LSI_TANGENT_LINE       //!< Only the line is tangent, but the tangent point is OUTSIDE the edge -- this case might be treated equal to LSI_NO_INTERSECT.
};
eLineSphereCases lineSphereIntersect( double rRadius, Vector3D& pos, Vector3D& a, Vector3D& b, Vector3D* s1, Vector3D* s2 );

struct labelLine { // Introduced April 2010 ... to be improved
	Vertex*           vertA;
	Vertex*           vertB;
	Face*             mFromFace;
	Face::eEdgeNames  mFromEdge;
	int               labelNr;
};

#endif
