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

#ifndef VERTEX_H
#define VERTEX_H

#include "primitive.h"

#include <deque>
#include <map>
#include <initializer_list>
#include <array>

// Circular dependencies:
class Face;
struct s1RingSectorPrecomp;
class EdgeGeodesic;

//!
//! \brief Class for the most basic element of a mesh. (Layer 0)
//!
//! Vertex class for the most basic element of a mesh.
//!
//! Layer 0
//!

class Vertex : public Primitive {

	public:
		enum eGrayLevelConversion {
			RGB_TO_GRAY_AVERAGE,
			RGB_TO_GRAY_AVERAGE_WEIGHTED,
			RGB_TO_GRAY_SATURATION_REMOVAL,
			RGB_TO_GRAY_HSV_DECOMPOSITION,
		};

		// Const- & destructor:
		Vertex();
		Vertex( const int rSetIdx, const sVertexProperties& rSetProps );
		Vertex( unsigned int rSetIdx, double rPosX, double rPosY, double rPosZ );
		Vertex( Vector3D vertPos );
		Vertex( Vector3D vertPos, Vector3D vertNormal );
		Vertex( Vector3D vertPos, Vector3D vertNormal, uint64_t setLabelTo );
		Vertex( Vertex* vertA, Vertex* vertB, double weightA=-1.0, double weightB=+1.0, double weightPos=0.0 );
		~Vertex();

		// Indexing:
		        bool     setIndex( int setIdx ) override;
				int      getIndex() const override;
				int      getIndexOriginal() override;
		virtual void     getIndexOffsetBit( int* bitOffset, int* bitNr );
		        bool     markVisited( uint64_t* rVertBitArrayVisited );
				bool     unmarkVisited( uint64_t* rVertBitArrayVisited );
				bool     isMarked(const uint64_t* rVertBitArrayVisited );

		// Color managment:
		bool     setRGB( unsigned char setTexR, unsigned char setTexG, unsigned char setTexB ) override;
		bool     setAlpha( double rVal ) override;
virtual bool     setAlpha(unsigned char alpha);

		// Information retrival:

		//! Sets the index value and the x-,y-, and z-coordinates of the index/position vector pair to the
		//! index/position vector of the Vertex
		inline void getIndexAndCoordinates(std::pair<int, std::array<double, 3>>& indexAndPositionVectorPair) const
#if defined(__GNUC__) || defined(__GNUG__)
		__attribute__((always_inline))
#endif
		{
			
			indexAndPositionVectorPair.first = mIdx;
			indexAndPositionVectorPair.second.at(0) = mPosition[0];
			indexAndPositionVectorPair.second.at(1) = mPosition[1];
			indexAndPositionVectorPair.second.at(2) = mPosition[2];
		}
		                                                                             
		        double   getX() const override;
				double   getY() const override;
				double   getZ() const override;
				double   getNormalX() override;
				double   getNormalY() override;
				double   getNormalZ() override;
		        double   getNormalLen() const;
		        Vector3D getCenterOfGravity();
		        Vector3D getNormal( bool normalized=true );
				bool     copyNormalXYZTo( float*  rNormalXYZ, bool rNormalized=true ) override;
				bool     copyNormalXYZTo( double* rNormalXYZ, bool rNormalized=true ) override;
				bool     copyNormalXYZTo( double* rNormalXYZ, double rSetLength ) override;
				bool     copyRGBTo( unsigned char* rColor ) override;
				bool     copyRGBATo( unsigned char* rColor ) override;
		virtual unsigned char getR();
		virtual unsigned char getG();
		virtual unsigned char getB();
		virtual unsigned char getA();
		virtual bool     getGraylevel( double* rGraylevel, eGrayLevelConversion rGrayLevelConvMethod=RGB_TO_GRAY_HSV_DECOMPOSITION );

		// Position management
		virtual bool     setPosition( double rPosX, double rPosY, double rPosZ );

		// Value access:
		        bool     getPositionVector( Vector3D* rPosVec ) const;
		        Vector3D getPositionVector() const;
		        double   distanceToHNF( Vector3D HNF_Position, Vector3D HNF_Normal );
		        double   distanceToLine( const Vector3D* rPos1, const Vector3D* rPos2 );
		        double   angleInLineCoord( const Vector3D* rPosTop, const Vector3D* rPosBottom );
		        double   distanceToCoord(const double* someXYZ );
		        bool     copyCoordsTo( float* rCoordArr );
		        bool     copyCoordsTo( double* rCoordArr );
		        bool     copyVertexPropsTo( sVertexProperties& rVertexProps ) const;
		virtual bool     estNormalAvgAdjacentFaces(); // ***
		        bool     setNormal( double rNormX, double rNormY, double rNormZ );
		        bool     setNormal( Vector3D* rNormal );
		        bool     unsetNormal();
		virtual double   get1RingArea(); // ***
		        double   get1RingEdgeLenAvg();
		        double   get1RingEdgeLenMin();
		        double   get1RingEdgeLenMax();
		virtual uint64_t    get1RingFaceCount() const; // ***
		virtual double   get1RingSumAngles(); // ***
		        unsigned int getOctreeIndex( Vector3D rCubeTopLeft, double rEdgeLen, unsigned int rXyzCubes );
				int      getType() override;

		// Fetch sphere:
		virtual Vertex*  advanceInSphere( double* sphereCenter, double radius, // ***
				                          uint64_t* vertBitArrayVisited, std::set<Vertex*>* nextArray,
				                          uint64_t* faceBitArrayVisited, bool rOrderToFuncVal=false, double* rSeqNr=nullptr );
		virtual bool     mark1RingVisited( uint64_t* rVertBitArrayVisited, uint64_t* rFaceBitArrayVisited, // ***
		                                   bool           rOrderToFuncVal,      double*        rSeqNr );
		// Function value:
				bool     setFuncValue( double  setVal ) override;
				bool     getFuncValue( double* rGetVal ) const override;
		virtual bool     isFuncValFinite() const; // ***
		virtual bool     isFuncValLocalMinimum(); // ***
		virtual bool     isFuncValLocalMaximum(); // ***
		// Function value smoothing:
		virtual bool     funcValMedianOneRing( double* rMedianValue, double rMinDist );  // ***
		virtual bool     funcValMeanOneRing( double* rMeanValue, double rMinDist );  // ***
		// Comparison of function value - Heap/sorting related
		static  bool     funcValDescendingLabel( Vertex* vert1, Vertex* vert2 );
		static  bool     funcValDescending( Vertex* vert1, Vertex* vert2 );
		static  bool     funcValLower( Vertex* vert1, Vertex* vert2 );
		static  bool     funcValLowerLabled( Vertex* vert1, Vertex* vert2 );
		static  bool     sortLabelsFirst( Vertex* vert1, Vertex* vert2 );
		static  bool     sortByIndex( Vertex* vert1, Vertex* vert2 );
        static  bool     RGBLower( Vertex* vert1, Vertex* vert2 );
		
		
		// Coordinate Setter:
		
		//! If the index of the passed index/position vector pair match, sets the x-,y-, and z-coordinates
		//! of the position vector of the Vertex to the position vector of the passed index/position vector pair
		//! @return true if the passed index matched with the actual index, false if not
		inline void setCoordinates(const std::array<double, 3>& positionVector)
		{
			mPosition[0] = positionVector.at(0);
			mPosition[1] = positionVector.at(1);
			mPosition[2] = positionVector.at(2);
		}
		
		inline void setFunctionValue(const double functionValue)
		{
			mFuncValue = functionValue;
		}
		
		// Transformation:
		        bool     applyTransfrom( Matrix4D* transMat ) override;
		        bool     applyMeltingSphere( double rRadius, double rRel=1.0 ) override;
        virtual Vertex*  applyNormalShift(float offsetDistance,int index);

		// Labeling:
		bool     setLabel( uint64_t rSetLabelNr ) override;
		bool     getLabel( uint64_t& rGetLabelNr ) const override;

		// Neighbourhood - related to labeling!
		virtual void     getNeighbourVertices( std::set<Vertex*>* someVertList ); // ***
		        bool     getNeighbourVerticesExcluding( std::set<Vertex*>* neighboursSelected, std::set<Vertex*>* verticesToExclude );
		virtual bool     getAdjacentVerticesExcluding( std::vector<Vertex*>* rSomeVerts, uint64_t* rVertBitArrayVisited ); // ***
		virtual Vertex*  getAdjacentNextBorderVertex( uint64_t* rVertBitArrayUnVisited ); // ***
		virtual Vertex*  getConnection( std::set<Vertex*>* someVertList, bool reverse ); // ***

		// Mesh setup:
		//virtual void     connectToFace( Face* someFace ); // ***
		//virtual void     disconnectFace( Face* someFace ); // ***
		virtual bool     isAdjacent( Face* someFace ); // ***
		virtual void     getFaces( Vertex* otherVert, std::set<Face*>* neighbourFaces, Face* callingFace ); // ***
		virtual void     getFaces( std::set<Face*>* someFaceList ); // ***
		virtual void     getFaces( std::vector<Face*>* someFaceList ); // ***

		// Mesh checking:
		virtual bool     belongsToFace(); // ***
		virtual int      connectedToFacesCount(); // returns how many faces are connected with this vertex // ***
		virtual bool     isNotANumber();          // true, when at least one coordinate is not a number
		virtual bool     isSolo();                // true, when connected to ZERO faces and ZERO edges // ***
		virtual bool     isBorder();              // true, when the vertex is part of one or more border edges // ***
		virtual bool     isManifold(); // ***
		virtual bool     isNonManifold(); // ***
		virtual bool     isDoubleCone();          // checks for a vertex as tip of two cones (as this annoys labeling) // ***
		virtual bool     isPartOfZeroFace();      // checks if the vertex belongs to face having an area of zero // ***
		virtual bool     isInverse();             // check if this vertex is part of an edge connecting faces with improper orientation // ***
		        int      getState();

		// Distances
		        double   estDistanceTo( Vertex* vert );
		        double   estDistanceTo( double x, double y, double z );
			double   estDistanceToLine( const Vertex* rPosTip, const Vertex* rPosBottom );
		        double   estDistanceToLineDir( const Vector3D* rPos, const Vector3D* rDir );
		        double   estDistanceToPlane( double* planeHNF, bool absDist );
				double   estDistanceToPlane( const Vector3D& rPlaneHNF, bool rAbsDist = false ) const;
		        double   estDistanceToCone(Vector3D* axisTop,
		                                   Vector3D* axisBot,
		                                   Vector3D* coneTip,
		                                   const double& semiAngle,
		                                   const double& coneHeight,
		                                   const double& upperRadius,
		                                   const double& lowerRadius,
		                                   bool absDist=false );
		        double   estDistanceToSphere(Vector3D* center,
		                                     const double& radius,
		                                     bool absDist=false );

		// Angles
		        double   getAngleToRadial(const Vector3D& rAxisTop, const Vector3D& rAxisBottom);
		        double   getAxisAngleToRadial( const Vector3D &rAxisTop, const Vector3D &raxisBottom );
		        double   getOrthogonalAxisAngleToRadial( const Vector3D &rAxisTop, const Vector3D &rAxisBottom );

		// Geodesics:
				bool     getGeodesicEdgesSeed( std::deque<EdgeGeodesic*>* frontEdges,
				                               std::map<Vertex*,GeodEntry*>* geoDistList,
				                               uint64_t* faceBitArray, int faceNrBlocks, bool weightFuncVal );

		// Feature vector functions
				bool     assignFeatureVec( const double *rAttachFeatureVec, unsigned int rSetFeatureVecLen ) override;
		virtual bool     assignFeatureVec( const std::initializer_list<double> featureVectorValues );
				bool     assignFeatureVecValues( const std::vector<double>& newFeatureVec );
				bool     copyFeatureVecTo( double* rFetchFeatureVec ) const override;
				bool     getFeatureVectorElements( std::vector<double>& rFeatVec ) const override;
		// Norm of the feature vector inlcuding related functions
		virtual bool     getFeatureVecMeanStdDev( double* rFeatureVecMean, double* rFeatureVecStdDev ) const;
		virtual bool     getFeatureVecLenMan( double* rFeatureVecLenMan );
		virtual bool     getFeatureVecLenEuc( double* rFeatureVecLenEuc );
		virtual bool     getFeatureVecMin( double* rFeatureVecMin );
		virtual bool     getFeatureVecMax( double* rFeatureVecMax );
		virtual bool     getFeatureVecMinSigned( double* rFeatureVecMinSigned );
		virtual bool     getFeatureVecMaxSigned( double* rFeatureVecMaxSigned );
		virtual bool     getFeatureVecBVFunc( double* rFeatureVecBVFunc );
		virtual bool     getFeatureVecTVSeqn( double* rFeatureVecTVSeqn );
		// Norm of the feature vector with a given reference
		virtual double   getFeatureDistManTo( double* rSomeFeatureVec );
		virtual double   getFeatureDistEucTo( double* rSomeFeatureVec );
		virtual double   getFeatureDistEucNormTo( double* rSomeFeatureVec, std::vector<double>* rFeatureVecStdDev );
		virtual double   getFeatureVecCosSim( double* rSomeFeatureVec, bool rApplyACos=true );
		virtual double   getFeatureVecTanimotoDist( double* rSomeFeatureVec );
		// NEW feature vector p-Norm
		virtual bool     getFeatureVecPNorm( double* rResult, double rPNorm,
				                             const std::vector<double>* rReferenceVec=nullptr, const std::vector<double>* rOffsetVec=nullptr,
				                             const std::vector<double>* rWeightNum=nullptr, const std::vector<double>* rWeightDenom=nullptr );
		// Other feature vector related functions
				bool         getFeatureElement( unsigned int rElementNr, double* rElementValue ) override;
		virtual bool         setFeatureElement( unsigned int elementNr, double value);
				unsigned int getFeatureVectorLen() override;
				int          cutOffFeatureElements( double rMinVal, double rMaxVal, bool rSetToNotANumber );
		virtual void         resizeFeatureVector(unsigned int size);
		// Feature vector smoothing:
		virtual bool     getFeatureVecMedianOneRing( std::vector<double>& rMedianValues, double rMinDist );
		virtual bool     getFeatureVecMeanOneRing(   std::vector<double>& rMeanValues,   double rMinDist );
		virtual bool     getFeatureVecMeanOneRing(   const double& rMinDist,
				                                     const std::vector<s1RingSectorPrecomp>& r1RingSecPrecomp,
				                                     std::vector<double>& rMeanValues );

		// Rollouts:
		        bool     unrollAroundCone( double rCutHeight1,
		                                   double rCutHeight2,
		                                   double rConeAngle,
		                                   double rPrimeMeridian );
		        bool     unrollAroundCylinderRadius( double rRadius, double rPrimeMeridian );
		        bool     unrollAroundSphere(double primeMeridian, double sphereRadius);

		// Debuging:
		virtual void     dumpInfo(); // ***
		        void     dumpInfoAsDOT( std::string fileSuffix="" );

private:
		// Position, Normal and Color:
		double        mPosition[3];      //!< Position vector of the Vertex.
		double        mNormalXYZ[3];     //!< Normal vector for the vertex - has to be initalized, e.g. by the average normal of adjacent faces.
		unsigned char mTexRGBA[4];       //!< Texture per Vertex: Red, Green, Blue and Alpha
		// Function Value:
		double        mFuncValue;        //!< Function value.
		// Indexing:
		int           mIdx;              //!< Stores the actual index, which may change due to manipulation, while idxOri stores the original index.
		int           mIdxOri;           //!< Original or first given index. Typically as read from a file.
		// Labeling:
		uint64_t      mLabelNr;          //!< Number of a label of connected mesh part. Label 0 means background. Default: _PRIMITIVE_NOT_LABLED_
		// Feature vector:
		unsigned int  mFeatureVecLen;    //!< Length of the Feature vector (e.g. from multi-scale volume integral)
		double*       mFeatureVec;       //!< Feature vector (e.g. from multi-scale volume integral)
};

// GLOBAL Operators ------------------------------------------------------------
void corrr1d(const double* signal, const int n, const double* pattern, const int m, double* r);
double distanceVV( const Vertex* rVertA, const Vertex* rVertB );
double distanceVV( Vertex* rVert, const Vector3D& rPos );
double distanceVV( Vector3D* rPos, Vertex* rVert  );
double distanceVV( Vector3D* rPos1, Vector3D* rPos2 );

#endif
