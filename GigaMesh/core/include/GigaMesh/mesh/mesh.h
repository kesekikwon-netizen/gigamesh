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

#ifndef MESH_H
#define MESH_H

#include <cstdlib>
#include <cstring>
#include <cfloat>
#include <cmath>
#include <chrono>

#include <iostream>
#include <array>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <functional>

#include "bitflagarray.h"
#include "vertex.h"
#include "face.h"
#include "plane.h"
#include "octree.h"

#include "polyline.h"
#include "sphere.h"
#include "rectbox.h"

#include "edgegeodesic.h"
#include "geodentry.h"

#include "voxelfilter25d.h"

#ifdef THREADS
    // Multithreading (CPU):
    #include <thread>
#endif

#include "meshinfodata.h"
#include "meshio.h"
#include "mesh_params.h"

// Helper classes
#include "showprogress.h"
#include "userinteraction.h"

//!
//! \brief Central class for mesh manipulation. (Layer 0)
//!
//! Central class organizing a mesh described by vertices and faces.
//! Supplies additional classes/parts of the mesh like edges and methods for
//! manipulating the mesh.
//!
//! One should always call Mesh::establishStructure after creation.
//! For performance and test reasons it is not integrated into Mesh::Mesh.
//!
//! Layer 0
//!

class Mesh : public Primitive, public MeshIO, public MeshParams,
             protected ShowProgress, protected UserInteraction {

	public:
		Mesh();
		Mesh( const std::filesystem::path& rFileName, bool& rReadSuccess );
		Mesh( std::set<Face*>* someFaces );
		~Mesh();

		// Menu handling
		virtual bool   setParamFloatMesh( MeshParams::eParamFlt rParam, double rValue );
		virtual bool   callFunction( MeshParams::eFunctionCall rFunctionID, bool rFlagOptional=false );

	private:
		// to be called after READING a file
		void establishStructure( std::vector<sVertexProperties>& rVertexProps,
		                         std::vector<sFaceProperties>& rFaceProps );

	public:
		// Octree
        virtual void generateOctree( int vertexmaxnr);

		// Information retrival - overloaded from Primitive:
		virtual double   getX() const;
		virtual double   getY() const;
		virtual double   getZ() const;

        virtual double   getMinX() const;
        virtual double   getMinY() const;
        virtual double   getMinZ() const;
        virtual double   getMaxX() const;
        virtual double   getMaxY() const;
        virtual double   getMaxZ() const;

		// IO Operations - overloaded from MeshIO and MeshSeedExt
		virtual bool     writeFile( const std::filesystem::path& rFileName );
		        bool     writeFilesForConnectedComponents();
		virtual bool     importFeatureVectorsFromFile( const std::filesystem::path& rFileName );
		virtual bool     exportFeatureVectors( const std::filesystem::path& rFileName );
	private:
		        bool     assignFeatureVectors( const std::vector<double>& rFeatureVecs, const uint64_t& rMaxFeatVecLen );

	public:
		// IO Operations - overloaded from MeshSeed
		virtual bool     getVertNormal( int rVertIdx, double* rNormal );

		// Vertex navigation:
				uint64_t     getVertexNr() const;
				Vertex*      getVertexPos( uint64_t rPosIdx ) const;
				bool         orderVertsByIndex();
				bool         orderVertsByFuncVal();
                bool         orderVertsByRGB();
				bool         setVertexPosToIndex();
				bool         setVertexFlagForAll( ePrimitiveFlags rFlag );
				bool         clearVertexFlagForAll( ePrimitiveFlags rFlag );
				bool         getVertexList( std::set<Vertex*>* rVertices );
				bool         getVertexList( std::vector<Vertex*>* rVertices );
				bool         getVertexListValidFuncVal( std::vector<Vertex*>* rVertices );

				const std::vector<Vertex*>* getPrimitiveListVertices();
				const std::vector<Face*>*   getPrimitiveListFaces();

		// Face navigation:
		uint64_t     getFaceNr() const;
		Face*        getFacePos( uint64_t posIdx ) const;
		bool         getFaceList( std::set<Face*>* rFaces );
		bool         getFaceList( std::vector<Face*>* rFaces );

		// Polyline navigation -- partially overloaded from MeshSeedExt
		virtual unsigned int  getPolyLineNr();
		virtual unsigned int  getPolyLineLength( unsigned int rPolyIdx );
		virtual int           getPolyLineVertIdx( unsigned int rPolyIdx, unsigned int rElementIdx );
		virtual uint64_t getPolyLineLabel( unsigned int rPolyIdx );
		virtual PrimitiveInfo getPolyLinePrimInfo( unsigned int rPolyIdx );
		        PolyLine*     getPolyLinePos( unsigned int rPosIdx );
		        bool          getPolyLineBoundingBoxFromAll( double* rMinX, double* rMinY, double* rMinZ, \
		                                                     double* rMaxX, double* rMaxY, double* rMaxZ, \
		                                                     bool rProjectToPlane );

		// Data retrival:
				Vertex* getVertexByIdxOriginal( int findIdx ); // overloaded from Primitive
				Vertex* getVertexByIdx( int findIdx );
				bool    getVertexNextTo( Vector3D rVertPos, Vertex** rVertexNext );
				bool    getVerticesInBeam( Vector3D rVertAPos, Vector3D rVertBPos, float rBeamPerimeterRadius, std::set<Vertex*>* rVertsInBeam );
				bool    getFacesInBeam( Vector3D rVertAPos, Vector3D rVertBPos, float rBeamPerimeterRadius, std::set<Face*>* rFacesInBeam );
				double  getEdgeLenMin();
				double  getEdgeLenMax();
				double  getAltitudeMin();
		virtual int     getType();
				bool    getVerticesSelectedIndicesStr( std::string& rVertIdx );

		// De-Selection --------------------------------------------------------------------------------------------------------------------------------
		virtual bool    deSelMVertsAll();
		virtual bool    deSelMVertsNoLabel();
		// Selection - Changes -------------------------------------------------------------------------------------------------------------------------
		virtual unsigned int  selectedMVertsChanged();
		virtual unsigned int  selectedMFacesChanged();
		virtual unsigned int  selectedMPolysChanged();
		virtual unsigned int  selectedMPositionsChanged();
		// Selection -----------------------------------------------------------------------------------------------------------------------------------
				bool          selectVertsByIdxShow();
				bool          selectVertsByIdxUser();
				bool          selectVertsRandomUser();
				bool          selectVertsByIdx( const std::vector<long>& rVertIndices );
				bool          selectVertsByIdx( const std::set<uint64_t>& rVertIndices );
				bool          selectVertsRandom( const double rPercent, const bool rAreaWeight );
		virtual bool          addToSelection( Vertex* const rVertToAdd );
		virtual bool          addToSelection( std::vector<Vertex*>* const rVertsToAdd );
		virtual bool          addToSelection( Face* const rFaceToAdd );
		virtual bool          addToSelection( const std::set<Vertex*>& rVertsToAdd );
		virtual bool          addToSelection( const std::set<Face*>& rFaceToAdd );
		//.
        //Deselection ----------------------------------------------------------------------------------------------------------------------------------
        virtual bool          removeFromSelection( const std::set<Vertex*>& rVertsToRemove );
        virtual bool          removeFromSelection( std::vector<Vertex*>* const rVertsToRemove );
        //.
		virtual int    selectVertFuncValLowerThan( double rVal );
		virtual int    selectVertFuncValGreatThan( double rVal );
		//.

		//TODO: is this the right place for these structs?
		struct vertexNeighbourhoodHelper{
			Vertex* vert = nullptr;
			int     type = 0;
			bool visited = false;

			vertexNeighbourhoodHelper(Vertex* aVert, int anInt, bool aBool) : vert(aVert), type(anInt), visited(aBool) { }

			bool operator < (const vertexNeighbourhoodHelper& other) const {
				return (type < other.type);
			}
		};

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Non-Max Suppression and (Skeleton) line determination
	//------------------------------------------------------------------------------------------------------------------------------------------------------
		virtual bool   traverseSkeletonLine(std::vector<vertexNeighbourhoodHelper> &neighbourhood,
		                                    vertexNeighbourhoodHelper &x, vertexNeighbourhoodHelper &start, PolyLine* &line);
		virtual bool   createSkeletonLineInit(int connectTriangles);
	//------------------------------------------------------------------------------------------------------------------------------------------------------
		virtual bool   findVerticesOnSecondaryDirection(Vertex* &curVertex, Vector3D principalDirection,
		                                                std::vector<double> &comparisonFunctionValues, Vertex* &VertexMAXnormal, bool printDebug);
		virtual bool   temporaryMeshSimplification(std::list<Vertex*> &adjacentVertsInOrder, double eps, bool printDebug);
		virtual bool   PlaneSegmentIntersectionAndInterpolation(Vertex* &planeVertex, Vector3D planeNormal, Vertex* &segmentPoint1, Vertex* &segmentPoint2, Vector3D &resIntersectionPoint, double &resInterpolatedFunctionValue, bool debugPrint);
		virtual bool   selectVertNonMaximum( double eps );
		virtual bool   nonMaximumCorrectnessCheck( );
		virtual bool   getSurroundingVerticesInOrder (std::list<Vertex*> &adjacentVertsInOrder, Vertex* &pi, bool printDebug);
		virtual bool   organizeGroupList(std::list<std::pair<Vertex*, int>> &Lgroup, std::list<Vertex*> &adjacentVertsInOrder, bool printDebug);
		virtual bool   groupListMerge(std::list<std::pair<Vertex*, int>> &helperList, std::list<Vertex*> &adjacentVertsInOrder, bool printDebug);
	//------------------------------------------------------------------------------------------------------------------------------------------------------

		virtual bool   selectVertLocalMin();
		virtual bool   selectVertLocalMax();
		//.
		virtual bool   selectVertSolo();
		virtual bool   selectVertNonManifoldFaces();
		virtual bool   selectVertDoubleCone();
		virtual bool   selectVertLabelAreaLT( double rAreaMax );
		virtual bool   selectVertLabelAreaRelativeLT( double rPercent );
		virtual bool   selectVertBorder();
		virtual bool   selectVertFaceMinAngleLT( double rMaxAngle );
		virtual bool   selectVertFaceMaxAngleGT( double rMinAngle );
		virtual bool   selectVertLabelNo();
		        bool   selectVertLabelBackGrd();
		virtual bool   selectVertLabelNo( std::set<int64_t> &rLabelNrs );
		virtual bool   selectVertLabelNo( std::set<uint64_t> &rLabelNrs );
		virtual bool   selVertLabeledNot();
		virtual bool   selVertByFlag( ePrimitiveFlags rFlag );
		virtual bool   selectVertInvert();
		virtual bool   selectVertFromFaces();
		virtual bool   selectVertFromFacesRidges();
		//.
		virtual bool   selectFaceNone();
		virtual bool   selectFaceSticky();
		virtual bool   selectFaceNonManifold();
		virtual bool   selectFacesOppositeToPlane(double nx, double ny, double nz, double d, bool reverseDirection = false);
		virtual bool   selectFaceZeroArea();
		virtual bool   selectFaceInSphere( Vertex* rSeed, double rRadius );
		virtual bool   selectFaceRandom( double rRatio );
		virtual bool   selectFaceWithSyntheticVertices();
				bool   selectFaceBorderThreeVertices();
				bool   selectFaceThreeVerticesSelected();
				bool   selectFaceBorderBridgeTriConnect();
				bool   selectFaceBorderBridge();
				bool   selectFaceBorderDangling();
				bool   selectFaceLabeledVerticesCorner();
		virtual bool   selectFaceSelfIntersecting();

		//.
		virtual bool   selectPolyNoLabel();
		virtual bool   selectPolyRunLenGT( double rValue );
		virtual bool   selectPolyRunLenLT( double rValue );
		virtual bool   selectPolyLongest();
		virtual bool   selectPolyShortest();
		virtual bool   selectPolyLabelNo(const std::vector<int>& rLabelNrs );
		virtual bool   selectPolyNotLabeled();
		virtual bool   selectPolyVertexCount( unsigned int rMinNr, unsigned int rMaxNr );
		//.
		virtual bool   getSelectedVerts( std::set<Vertex*>* rSomeVerts );

		// Single selection - SelPrim i.e. SelVert and SelFace
				Primitive* getPrimitiveSelected();
				bool       selectPrimitiveVertexUser();

		//-------------------------------------------------------------------------------------------
		// Plane definition:
				bool   isPlanePosCSet();
		virtual bool   getPlanePosToSet( int* rPosID );
		virtual bool   setPlaneVPos( std::vector<double>* rPositions );
		virtual bool   setPlanePos( Vector3D* rSomePos );
				bool   setPlaneAxisPos( const Vector3D& axisTop, const Vector3D& axisBottom, const Vector3D& pos);
				bool   getPlaneHNF( Vector3D* rPlaneHNF );
				bool   getPlaneHNF( double* rPlaneHNF );
		virtual bool   setPlaneHNF( Vector3D* rPlaneHNF );
				bool   setPlaneHNFbyAxisSelPrim();
				bool   setPlaneHNFbyAxisAndLastPosition();
				bool   orientPlaneHNFbyAxis();
				bool   flipPlane();
				bool   getPlaneIntersectEdge( Vector3D* rayTop, Vector3D* rayBot, Vector3D* rayIntersect );
				bool   getPlaneIntersectLine( Vector3D* ptA, Vector3D* ptB, Vector3D* lineIntersect );
				bool   getPlaneIntersectLineDir( Vector3D* rRayPos, Vector3D* rRayDir, Vector3D* rRayIntersect );
				bool   getPlanePositions( double* rThreePos );
				bool   getMeshPlaneFan( Vector3D* rCentralPoint, std::vector<Vector3D>* rFanVertices );
		virtual bool   applyTransfromToPlane( Matrix4D rTransMat );
		virtual bool   splitByPlane( Vector3D planeHNF, bool duplicateVertices = false, bool noRedraw = false );
		virtual bool   splitByIsoLine( double rIsoVal, bool duplicateVertices = false, bool noRedraw = false, Vector3D rUniformOffset=Vector3D( 0.0, 0.0, 0.0, 0.0 ) );
		virtual bool   splitMesh(const std::function<bool(Face*)>& intersectTest, const std::function<double(VertexOfFace*)>& signedDistanceFunction, const std::function<void(VertexOfFace*, VertexOfFace*, Vector3D&)>& getIntersectionVector, bool duplicateVertices = false, bool noRedraw = false, Vector3D rUniformOffset=Vector3D( 0.0, 0.0, 0.0, 0.0 ));
		virtual bool   calcIntersectionPolylineWithPlane( const Vector3D& planeHNF, std::vector<Vector3D>* rIntersectionPoints ); //!< Calculate intersection polyline with plane (for archaeological section cutting). Does not modify mesh.

				Plane::ePlaneDefinedBy getPlaneDefinition();
	private:
		virtual bool   triangulateSplitFace(std::vector<VertexOfFace*>& faceVertices, std::set<Face*>* newFaces = nullptr, std::vector<float>* newUVS = nullptr, unsigned char textureID = 0);

	public:
		// Cone stuff:
		//---------------------------------------------------------------------------------------------------------------------------------------------
		// Status flags for drawing partial results of the cone selection:
		enum coneStates {
			CONE_UNDEFINED,
			CONE_DEFINED_AXIS,
			CONE_DEFINED_UPPER_RADIUS,
			CONE_DEFINED_LOWER_RADIUS,
			CONE_UNROLLED
		};

		virtual bool setConeAxis(const Vector3D& upper, const Vector3D& lower);
		virtual bool setConeRadius(Vector3D& p);
		virtual bool setConeRadius(double upperRadius, double lowerRadius);
		        bool getConeAxis( Vector3D* rAxisTop, Vector3D* rAxisBottom );
				bool getConeAxisPosDir( Vector3D* rAxisBottom, Vector3D* rAxisOrient );
				bool getConeAxisDefined();
				bool getConeMeridianZero( Vector3D* rMeridianTop, Vector3D* rMeridianBottom, bool rOppsoite );
				bool getConeRadii( double* rUpperRadius, double* rLowerRadius );
		//! \todo Source revision: Move cone status to MeshParams
				void setConeStatus( coneStates rConeStatus );
				void setConeOuterPoint( double rRadius );
				bool setConeCoverMesh();
				bool setConeConsistentPointOrder();
				bool getConeInfoDump(const std::string& rCallingFunc );
		virtual coneStates getConeStatus();
	private:
				bool calcConeTip( Vector3D* rConeTip );
				bool calcConeHeigth( double* rHeight );
				bool calcConeAngle( double* rAngle );
		//---------------------------------------------------------------------------------------------------------------------------------------------
	public:
		// Sphere stuff:
		//---------------------------------------------------------------------------------------------------------------------------------------------
		virtual bool	setSpherePoint(Vector3D& p, Vector3D &normal);
		virtual bool	getSphereData(Vector3D* center, double* radius);
		virtual bool	getSphereData(double* spherePoints, double* sphereNormals = nullptr);
		virtual int	    getSpherePointIdx();
				double	getSphereRadius();
				bool	isUnrolledAroundSphere();
		//---------------------------------------------------------------------------------------------------------------------------------------------

		// texture map per vertex
		virtual bool    assignImportedTexture( int rLineCount, uint64_t* rRefToPrimitves, unsigned char* rTexMap );
		virtual bool    assignImportedNormalsToVertices( const std::vector<grVector3ID>& rNormals );
		virtual bool    multiplyColorWithFuncVal();
		virtual bool    multiplyColorWithFuncVal( const double rMin, const double rMax );
		virtual bool    assignAlphaToSelectedVertices(unsigned char alpha);

		// feature vectors
	protected:
		virtual int    removeFeatureVectors();

	public:
		// Feature vector functions (NEW)
		        bool   computeMSIIQuick( const double rRadius, const unsigned int rRadiiCount,
		                                 const unsigned int rxyzDim );
		        bool   computeMSIIQuickGUI();
		//! \todo use MeshParams::eFunctionCall to avoid/reduce copy&paste code!
		// Feature vector functions
		 uint64_t getFeatureVecLenMax( int rPrimitiveType );
		        double* getFeatureElementsNr(const int primitiveType, const int elementNr, const double* minVal, const double* maxVal, int* elementsNr );
				int     cutOffFeatureElements( int primitiveType, double minVal, double maxVal, bool setToNotANumber );
		// Norm of the feature vector
				bool    estFeatureVecLenManVertex( double** funcValues, Vertex*** vertices, int* vertCount );
				bool    estFeatureVecLenEucVertex( double** funcValues, Vertex*** vertices, int* vertCount );
				bool    estFeatureVecBVFunc( double** rFuncValues, Vertex*** rVertices, int* rVertCount );
				bool    estFeatureVecTVSeqn( double** rFuncValues, Vertex*** rVertices, int* rVertCount );
		// Norm of the feature vector with a given reference
			bool    estFeatureDistManToVertex( Primitive* rSomePrim, double** funcValues, Vertex*** vertices, int* vertCount );
			    bool    estFeatureDistManToVertex( double* someFeatureVector, double** funcValues, Vertex*** vertices, int* vertCount );
				bool    estFeatureDistEucToVertex( Primitive* rSomePrim, double** rFuncValues, Vertex*** rVertices, int* rVertCount );
				bool    estFeatureDistEucToVertex( double* someFeatureVector, double** funcValues, Vertex*** vertices, int* vertCount );
			bool    estFeatureDistEucNormToVertex( Primitive* rSomePrim, double** rFuncValues, Vertex*** rVertices, int* rVertCount );
			    bool    estFeatureDistEucNormToVertex( double* someFeatureVector, double** funcValues, Vertex*** vertices, int* vertCount );
				bool    estFeatureCosineSimToVertex( Primitive* rSomePrim, double** rFuncValues, Vertex*** rVertices, int* rVertCount );
				bool    estFeatureCosineSimToVertex( double* rSomeFeatureVector, double** rFuncValues, Vertex*** rVertices, int* rVertCount );
				bool    estFeatureTanimotoDistTo( Primitive* rSomePrim, double** rFuncValues, Vertex*** rVertices, int* rVertCount );
				bool    estFeatureTanimotoDistTo( double* rSomeFeatureVector, double** rFuncValues, Vertex*** rVertices, int* rVertCount );
		// Other feature vector related functions
				bool    estFeatureAutoCorrelationVertex( double** funcValues, Vertex*** vertices, int* vertCount );
				bool    estFeatureCorrelationVertex( Primitive* somePrim, double** funcValues, Vertex*** vertices, int* vertCount );
				bool    estFeatureAutoCorrelationVertex( Primitive* somePrim, double** funcValues, Vertex*** vertices, int* vertCount );
		// Feature vector smoothing
				bool    featureVecMedianOneRingUI( bool rPreferMeanOverMedian );
				bool    featureVecMedianOneRing( unsigned int rIterations=1, double rFilterSize=0.0, bool rPreferMeanOverMedian=true );

		//.
				std::set<Vertex*> intersectSphere( Vertex* someVert, float radius );

		// Mesh information:
				bool         getVertNotANumber( std::set<Vertex*>* rSomeVerts );
				bool         getVertPartOfZeroFace( std::set<Vertex*>* rSomeVerts );
				bool         getVertSolo( std::set<Vertex*>* rSomeVerts );
				bool         getVertNonManifoldFaces( std::set<Vertex*>* rSomeVerts );
				bool         getDoubleCones( std::set<Vertex*>* rSomeVerts );
				bool         getVertLabelAreaLargest( uint64_t& rLargestLabelId );
				bool         getVertLabelAreaLT( double rAreaMax, std::set<Vertex*>* rSomeVerts );
				bool         getVertLabelAreaRelativeLT( double rPercent, std::set<Vertex*>* rSomeVerts );
				bool         getVertLabled( std::set<Vertex*>* rSomeVerts );
				bool         getVertBorder( std::set<Vertex*>* rSomeVerts );
				bool         getVertFaceMinAngleLT( double rMaxAngle, std::set<Vertex*>* rSomeVerts );
				bool         getVertFaceMaxAngleGT( double rMinAngle, std::set<Vertex*>* rSomeVerts );
				bool         getVertLabelNoSingle( long rLabelNr, std::set<Vertex*>& rSomeVerts );
				bool         getVertLabelNoMulti( const std::set<int64_t> &rLabelNrs, std::set<Vertex*>& rSomeVerts );
				bool         getVertLabelNoMulti( const std::set<uint64_t> &rLabelNrs, std::set<Vertex*>& rSomeVerts );
				bool         getVertLabelBackGrd( std::set<Vertex*>& rSomeVerts );
				bool         getVertLabeledNot( std::set<Vertex*>* rSomeVerts );
				bool         getVertLabelIdFrom( const std::set<Vertex*>& rSomeVerts, std::set<uint64_t>& rLabelNrs );
				bool         getVertWithFlag( std::set<Vertex*>* rSomeVerts, ePrimitiveFlags rFlag );
				bool         getVertInverted( std::set<Vertex*>& rSomeVerts );
		//.
				bool         getFaceFlag( std::set<Face*>* rSomeFaces, int rFlag );
				bool         getFaceSticky( std::set<Face*>* rSomeFaces );
				bool         getFaceNonManifold( std::set<Face*>* rSomeFaces );
				bool         getFaceBorderThreeVertices( std::set<Face*>& rSomeFaces );
				bool         getFaceThreeVerticesSelected( std::set<Face*>& rSomeFaces );
				bool         getFaceBorderVertsEdges( std::set<Face*>& rSomeFaces, unsigned int rHasBorderVertices, unsigned int rHasBorderEdges );
				bool         getFaceLabeledVerticesCorner( std::set<Face*>& rSomeFaces );
				bool         getFaceZeroArea( std::set<Face*>* rSomeFaces );
				bool         getFaceContainsVert( const std::set<Vertex*>& rSomeVerts, std::set<Face*>& rSomeFaces );
				bool         getFaceHasVertLabelNo( const uint64_t rLabelNr, std::set<Face*>& rSomeFaces );
				bool         getFaceHasVertLabelNo( const std::set<uint64_t>& rLabelNrs, std::set<Face*>& rSomeFaces );
		//.
				bool         getPolyNoLabel( std::set<PolyLine*>* rSomePolyLines );
				bool         getPolyRunLenGT( std::set<PolyLine*>* rSomePolyLines, double rValue );
				bool         getPolyRunLenLT( std::set<PolyLine*>* rSomePolyLines, double rValue );
				bool         getPolyLongest( std::set<PolyLine*>* rSomePolyLines );
				bool         getPolyShortest( std::set<PolyLine*>* rSomePolyLines );
				bool         getPolyLabelNo( const std::vector<int>& rLabelNrs, std::set<PolyLine*>* rSomePolyLines );
				bool         getPolyNotLabel( std::set<PolyLine*>* rSomePolyLines );
				bool         getPolyVertexCount( std::set<PolyLine*>* rSomePolyLines, unsigned int rMinNr, unsigned int rMaxNr );

				std::set<Face*>   getNonManifoldFaces();
				bool         getMeshVolumeDivergence( double& rVolumeDX, double& rVolumeDY, double& rVolumeDZ );
				bool         compVolumePlane( double* rVolumePos, double* rVolumeNeg );

		// Histogram:
		virtual bool         getHistogramValues( eHistogramType rHistType, std::vector<unsigned int>* rNumArray, double* rValMin, double* rValMax );

		// Labeling -- Connected Components
		virtual bool labelsChanged();
				bool labelCount( int rPrimitiveType, uint64_t& rLabelMaxNr );
				int  labelCountElements( int primitiveType );
				bool estLabelNormalSizeCenterVert( std::vector<Vector3D>* rLabelCenters, std::vector<Vector3D>* rLabelNormals );
				void labelVerticesNone();
				void labelVerticesBackground();
		virtual int  labelFaces( int facesNrToRemove=0 );
		virtual bool labelVerticesAll();
		virtual bool labelVertices( const std::vector<Vertex*>& rVerticesToLabel, std::set<Vertex*>& rVerticesSeeds );
		virtual bool labelVertices( const std::set<Vertex*>&    rVerticesToLabel, std::set<Vertex*>& rVerticesSeeds );
		virtual void labelSelectionToSeeds();
			bool labelSelectedVerticesBackGrd();
			    bool labelSelectedVerticesUser();
				bool labelSelectedVertices( std::set<Vertex *>& rSelectedVertices, bool rSetNotSelectedtoBackGrd );
		virtual bool labelVerticesEqualFV();
        virtual bool labelVerticesEqualRGB();
		virtual bool labelSelMVertsToBack();

                bool labelKMeansVertPos();
                //kMeans (also used for automatic mesh alignment)
                bool computeVertexPositionKMeans(std::vector<Vector3D> *centroids, bool labeling);
                bool assignVerticesToClusterByPosition(std::vector<Vector3D> *centroids, std::vector<std::set<Vertex*>>*clusterSets, bool labeling);
                Vector3D getCentroidByPosition(std::set<Vertex*> *clusterSet);
                std::vector<std::set<Vertex*>> computeVertexNormalKMeans(std::vector<Vector3D> *centroids, bool labeling);
                bool assignVerticesToClusterByNormal(std::vector<Vector3D> *centroids, std::vector<std::set<Vertex*>>*clusterSets, bool labeling);
                Vector3D getCentroidByNormal(std::set<Vertex*> *clusterSet);

		virtual bool compPolylinesIntInvRunLen( double rIIRadius, PolyLine::ePolyIntInvDirection rDirection );
		virtual bool compPolylinesIntInvAngle( double rIIRadius );
		virtual void getPolylineExtrema( bool absolut );
		virtual bool setPolylinesNormalToVert();
				bool planeIntersectionToPolyline();
				bool planeIntersectionsAxisAndPositions();
				bool isolineToPolylineMultiple();
		virtual bool isolineToPolyline();
		virtual bool isolineToPolyline( double rIsoValue, Plane* rPlaneIntersect=nullptr );
		virtual bool extrudePolylines();
				bool labelVertSurface( uint64_t& rlabelsNr, double** rArea );
				bool labelFacesVert( std::set<Face*>** rLabelFaces, uint64_t& rlabelsNr );
		// -- Polylines --------------------------------------------------------------------------------------------------------------------------------
		virtual void polyLinesChanged();
		virtual bool removePolylinesAll();
		virtual bool removePolylinesSelected();
				void convertLabelBordersToPolylines();
		virtual bool convertBordersToPolylines();
				void convertSelectedVerticesToPolyline();

		// --- Function Values (FV, FuncVal ) ----------------------------------------------------------------------------------------------------------
		// OLD style - Compute function values:
				//! \todo source revision
				bool getVertIndices( double** funcValues, Vertex*** vertices, int* vertCount );
				bool getDistanceVerticesToPosition( Vector3D rPos, double* rDistMin, double* rDistMax ); // <- this is not an OLD style function - to be kept!
				bool getHueValues( double** funcValues, Vertex*** vertices, int* vertCount );
		// NEW style, Faces - Compute and modify function values:
				bool setFaceFuncValSortIdx();
				bool setFaceFuncValMarchRadiusIdx( Primitive* rSeed, double rRadius );
		// NEW style, Vertices - Compute and modify function values:
		virtual bool setVertFuncValSlope();
		virtual bool setVertFuncValAngleToRadial();
		virtual bool setVertFuncValAxisAngleToRadial();
		virtual bool setVertFuncValOrthogonalAxisAngleToRadial();
		virtual bool setVertFuncValGraylevel( Vertex::eGrayLevelConversion rGrayLevelConvMethod );
		virtual bool setVertFuncValDistanceToLinePosDir();
		virtual bool setVertFuncValDistanceToLinePosDir( const Vector3D* rPos, const Vector3D* rDir );
		virtual bool setVertFuncValDistanceToAxis();
		virtual bool setVertFuncValAngleBasedOnAxis( Vector3D* rPosBottom, Vector3D* rPosTop );
		virtual bool setVertFuncVal( double rVal );
		virtual bool setVertFuncValCutOff( double minVal, double maxVal, bool setToNotANumber );
		virtual bool setVertFuncValNormalize();
		virtual bool setVertFuncValAbs();
		virtual bool setVertFuncValAdd( double rVal );
		virtual bool setVertFuncValMult();
		virtual bool setVertFuncValMult( double rVal );
		virtual bool setVertFuncValToOrder();
				bool setVertFuncValDistanceToCone( bool rAbsDist );
				bool setVertFuncValDistanceToSphere();
				bool setVertFuncVal1RSumAngles();
				bool setVertFuncValOctreeIdx( double rEdgeLen );
				bool setVertFuncValFaceSphereAngleMax( double rRadius );
				bool setVertFuncValFaceSphereMeanAngleMax( double rRadius );
				bool setVertFuncVal1RingArea();
		// NEW style and NEW naming convention:
		enum eFuncFeatureVecPNormWeigth {
			FEATURE_VECTOR_PNORM_WEIGTH_LINEAR,
			FEATURE_VECTOR_PNORM_WEIGTH_QUADRATIC,
			FEATURE_VECTOR_PNORM_WEIGTH_CUBIC
		};
		        bool funcVertMedianOneRingUI( bool rPreferMeanOverMedian );
				bool funcVertMedianOneRing( unsigned int rIterations=1, double rFilterSize=0.0, bool rPreferMeanOverMedian=true, bool rStoreDiffAsFeatureVec=false );
				bool funcVertAdjacentFaces();
		virtual bool funcVert1RingRMin();
		virtual bool funcVert1RingVolInt();
		virtual bool funcVertDistancesMax();
		virtual bool funcVertFeatureElementsStdDev();
		virtual bool funcVertFeatureVecMin();
		virtual bool funcVertFeatureVecMax();
		virtual bool funcVertFeatureVecMinSigned();
		virtual bool funcVertFeatureVecMaxSigned();
		virtual bool funcVertFeatureVecMahalDist();
		virtual bool funcVertFeatureVecPNorm();
		virtual bool funcVertFeatureVecPNorm( const std::vector<double>& rReferenceVector, const double& rpNorm, eFuncFeatureVecPNormWeigth rWeigthType );
		virtual bool funcVertFeatureVecElementByIndex( unsigned int rElementNr );
		virtual bool funcVertDistanceToPlane( Vector3D rPlaneHNF, bool rAbsDist, bool rSilent=false );
				bool funcVertAddLight( Matrix4D &rTransformMat, unsigned int rArrayWidth, unsigned int rArrayHeight, const std::vector<float>& rDepths, float rZTolerance );
				bool funcVertSphereSurfaceLength();
				bool funcVertSphereVolumeArea();
				bool funcVertSphereSurfaceNumberOfComponents();
				bool funcValToFeatureVector(unsigned int dim);
		// Again some old style function value calls:
				bool setVertFuncValCorrTo( std::vector<double>* rFeatVector );
				bool setVertFuncValDistanceToSelPrim();
				bool setVertFuncValDistanceTo( const Vector3D& rPos );
		// Stubs for notification
		virtual void changedFaceFuncVal();
		virtual void changedVertFuncVal();
		virtual void changedVertFeatureVectors();
		// Information about function values.
				bool getVertFuncValAverage( double* rAverage );
		// Partially old stuff:
		//! \todo source revision
				bool   getFuncValuesMinMax( double& rMinVal, double& rMaxVal );
				bool   getFuncValuesMinMaxQuantil( double rMinQuantil, double rMaxQuantil, double& rMinVal, double& rMaxVal );
				bool   getFuncValuesMinMaxInfNanFail( double& rMinVal, double& rMaxVal, int& rInfCount, int& rNanCount, int& rFailCount );
				bool   getFuncValuesMinMaxInfNanFail( double& rMinVal, double& rMaxVal, Vertex*& rVertMin, Vertex*& rVertMax, int& rInfCount, int& rNanCount, int& rFailCount, uint64_t& rFiniteCount );

		// --- Mesh manipulation - GENERIC -------------------------------------------------------------------------------------------------------------
		virtual bool   changedMesh();
		// --- Mesh manipulation - REMOVAL -------------------------------------------------------------------------------------------------------------
		virtual bool   removeVertices( std::set<Vertex*>* verticesToRemove );    // removal of a list of vertices
		virtual bool   removeVerticesSelected();
		        bool   removeUncleanSmall( const std::filesystem::path& rFileName, double rPercentArea, bool rApplyErosion );
		private:
		        bool   removeUncleanSmallCore( const std::filesystem::path& rFileName, double rPercentArea, bool rApplyErosion, 
		                                       uint64_t& rIterationCount );
		public:
		virtual bool   removeSyntheticComponents( const std::set<Vertex *> &rVerticesSeeds );
		virtual bool   removeFacesSelected();
				bool   removeFaces( std::set<Face*>* facesToRemove );            // removal of a list of faces
		virtual bool   removeFacesZeroArea();
				bool   removeFacesBorderErosion();
		// --- Mesh manipulation - MESH POLISHING ------------------------------------------------------------------------------------------------------
		virtual bool   completeRestore(); // AKA Mesh polishing
		virtual bool   completeRestore( const std::filesystem::path& rFilename, double rPercentArea, bool rApplyErosion,
		                                bool rPrevent, uint64_t rMaxNumberVertices, std::string* rResultMsg, uint64_t& rIterationCount );
		// --- Mesh manipulation - Manuall adding primitives -------------------------------------------------------------------------------------------
		virtual bool   insertVerticesEnterManual();
		virtual bool   insertVerticesCoordTriplets( std::vector<double>* rCoordTriplets );
		virtual bool   insertVertices( std::vector<Vertex*>* rNewVertices );
		// ---------------------------------------------------------------------------------------------------------------------------------------------

		// mainly used to set the initial view (see objwidget)
				void     getCenterOfGravity( float* cog );
				Vector3D getCenterOfGravity();
				Vector3D getBoundingBoxCenter();
				bool     getBoundingBoxSize( Vector3D& rBbSize ) const;
				double   getBoundingBoxRadius();
				float    getPerimeterRadius();
		// Additional geometric helper functions
				bool     getDistanceToPlaneMinMax( const Plane &rPlane, double& rMinDist, double& rMaxDist, bool rAbsDist );
		// Bounding Box Corners:
				Vector3D getBoundingBoxA();
				Vector3D getBoundingBoxB();
				Vector3D getBoundingBoxC();
				Vector3D getBoundingBoxD();
				Vector3D getBoundingBoxE();
				Vector3D getBoundingBoxF();
				Vector3D getBoundingBoxG();
				Vector3D getBoundingBoxH();
		        bool     getBoundingBoxProjected( const Matrix4D& rTransMat,
		                                          double& rMinX, double& rMaxX,
		                                          double& rMinY, double& rMaxY,
		                                          double& rMinZ, double& rMaxZ ) const;

		// Bit arrays for labeling, fetch in sphere, etc.
		enum eBitArrayFlags{
			BIT_ARRAY_MARK_NOTHING      = 0x00000000, //!< Zero
			BIT_ARRAY_MARK_LABEL_BACKGR = 0x00000001, //!< Mark all primitives visited, which are tagged as background.
			BIT_ARRAY_MARK_BORDER       = 0x00000002  //!< Mark all primitives visited, which are at the border of the mesh.
		};
		        uint64_t getBitArrayVerts( uint64_t** rVertBitArray, eBitArrayFlags rInitFlags=BIT_ARRAY_MARK_NOTHING );
				uint64_t getBitArrayFaces( uint64_t** rFaceBitArray, eBitArrayFlags rInitFlags=BIT_ARRAY_MARK_NOTHING );
				uint64_t getBitArrayEdges( uint64_t** rEdgeBitArray );

		// Estimate geodesic neighbourhood.
		virtual bool       geodPatchVertSel(bool rWeightFuncVal, bool rGeodDistToFuncVal );
		virtual bool       geodPatchVertSel( std::set<Vertex*>* rmLabelSeedVerts, bool rWeightFuncVal, bool rGeodDistToFuncVal );
		virtual bool       geodPatchVertSelOrder( bool rWeightFuncVal, bool rGeodDistToFuncVal );
		virtual bool       geodPatchVertSelOrder( std::vector<Vertex*>* rmLabelSeedVerts, bool rWeightFuncVal, bool rGeodDistToFuncVal );
				bool       estGeodesicPatchSelPrim();
		virtual bool       estGeodesicPatchRelabel( std::map<Vertex*,GeodEntry*>* geoDistList );
				bool       estGeodesicPatchFuncVal( std::map<Vertex*,GeodEntry*>* rGeoDistList );
				bool       estGeodesicPatchFuncVal( Vertex* seedVertex, double radius, bool weightFuncVal );
				bool       estGeodesicPatchFuncVal( Face* seedFace,     double radius, bool weightFuncVal );
				bool       estGeodesicPatch( Vertex* seedVertex, double radius, std::map<Vertex*,GeodEntry*>* geoDistList, bool weightFuncVal );
				bool       estGeodesicPatch( Face* seedFace,     double radius, std::map<Vertex*,GeodEntry*>* geoDistList, bool weightFuncVal );
	private:
				bool       estGeodesicPatch( Vertex* seedVertex, double radius, std::map<Vertex*,GeodEntry*>* geoDistList, uint64_t* faceBitArray, int faceNrBlocks, bool weightFuncVal );
				bool       estGeodesicPatch( Face* seedFace,     double radius, std::map<Vertex*,GeodEntry*>* geoDistList, uint64_t* faceBitArray, int faceNrBlocks, bool weightFuncVal );
				bool       estGeodesicPatch( std::map<Vertex*,GeodEntry*>* geoDistList, std::deque<EdgeGeodesic*>* frontEdges, double radius, uint64_t* faceBitArray, bool weightFuncVal ); // , int faceNrBlocks not used

	    public:
		// Estimate neighbourhood within a spherical volume --------------------------------------------------------------------------------------------
				std::set<Face*> fetchSphere( Vertex* seedVertex, float radius, bool noDebugTexture=true );
				bool       fetchSphereMarching( Vertex* seedVertex, std::set<Face*>* facesInSphere, float radius, bool noDebugTexture=true );
				bool       fetchSphereMarchingDualFront( Vertex* seedVertex, std::set<Face*>* facesInSphere, float radius );
				bool       fetchSphereBitArray( Vertex* rSeedVertex, std::vector<Face*>* rFacesInSphere, float rRadius,
				                                int rVertNrLongs, uint64_t* rVertBitArrayVisited,
				                                int rFaceNrLongs, uint64_t* rFaceBitArrayVisited,
				                                bool rOrderToFuncVal=false );
				bool       fetchSphereBitArray1R( Vertex* rSeedVertex, std::vector<Face*>& rFacesInSphere, double rRadius,
				                                  uint64_t rVertNrLongs, uint64_t* rVertBitArrayVisited,
				                                  uint64_t rFaceNrLongs, uint64_t* rFaceBitArrayVisited,
				                                  bool rOrderToFuncVal=false );

			// Compute or estimate Multi-Scale Integral Invariants (MSII) ----------------------------------------------------------------------------------
				double     fetchSphereCubeVolume25D( Vertex* seedVertex, std::set<Face*>*    facesInSphere, double radius, double* rasterArray, int cubeEdgeLengthInVoxels=256 );
				double     fetchSphereCubeVolume25D( Vertex* seedVertex, std::vector<Face*>* facesInSphere, double radius, double* rasterArray, uint cubeEdgeLengthInVoxels=256 );
				bool       fetchSphereArea( Vertex* rSeedPosition, std::vector<Face*>* rFacesInSphere, unsigned int rRadiiNr, double* rRadii, double* rAreas );
				bool       fetchSphereAreaEst( const Vertex* rSeedPosition, std::vector<Face*>* rFacesInSphere, const unsigned int rRadiiNr, const double* rRadii, double* rAreas );
				double*    getTriangleVertices( std::set<Face*>* someFaceList );
				double*    getTriangleVertices( std::vector<Face*>* someFaceList );

				std::set<Face*> getFacesIntersectSphere1( Vector3D positionVec, float radius );
				std::set<Face*> getFacesIntersectSphere2( Vector3D positionVec, float radius );
				std::vector<Vector3D> getPointsIntersectSphere( Vector3D positionVec, float radius );


		// Datum Sphere and Box
		virtual bool     datumAddSphere( Vector3D rPos, double rRadius, unsigned char rRed=255, unsigned char rGreen=127, unsigned char rBlue=0 );
		virtual void     removeAllDatumObjects();
				bool     hasDatumObjects();

		enum eTranslate {
			TRANSLATE_PLACE_ON_XZ_ONLY,      //!< Place mesh on xz-plane with no further transformation.
			TRANSLATE_PLACE_ON_XZ_CENTER,    //!< Place mesh on xz-plane and center the bounding box.
		};

		// mesh transformation
		        Matrix4D rotateToZ( Vector3D directionVec );          // generate rotation matrix to transform mesh so that the given direction vector is paralllel to the z-axis
        virtual bool     applyTransformationToWholeMesh( Matrix4D rTrans, bool rResetNormals = true, bool rSaveTransMat = true );
        virtual bool     applyTransformation( Matrix4D rTrans, std::set<Vertex*>* rSomeVerts, bool rResetNormals = true, bool rSaveTransMat = true );
		virtual bool     applyTransformationPlacement( eTranslate rType, Matrix4D* rAppliedMat=nullptr );
		virtual bool     applyTransformationAxisToY( Matrix4D* rAppliedMat=nullptr );
		virtual bool     applyTransformationDefaultViewMatrix( Matrix4D* rViewMatrix );
		virtual bool     applyMeltingSphere( double rRadius, double rRel );
				bool     applyInvertOrientationFaces();
				bool     applyInvertOrientationFaces( std::vector<Face*> rFacesToInvert );

		// Surface normals
		virtual bool     normalsVerticesChanged();
		        bool     resetFaceNormals( double* rAreaTotal=nullptr );
		        bool     resetVertexNormals();
		        bool     normalsVerticesComputeSphere( double rRadius );

		virtual bool     changedBoundingBox();

				bool     estBoundingBox();

		// normal vector related
				double    averageNormalByArea( std::set<Face*>* someFaces, double* normalVec );
				double    averageNormalByArea( std::vector<Face*>* someFaces, double* normalVec );
				Vector3D  averageNormal( std::set<Face*>* someFaces, float* maxAngleDeviation=nullptr, float* avgAngleDeviation=nullptr );
				Vector3D  averageNormal( float* maxAngleDeviation=nullptr, float* avgAngleDeviation=nullptr );

		// rasterization / voxelization
				// from meshseed:
				// we follow roughly this approach: http://www.3ddrome.com/articles/triangleraster.php
				// new link: https://www.joshbeam.com/articles/triangle_rasterization/
				double*   rasterViewFromZ( int xDim, int yDim );
				void      rasterViewFromZ( double* vertexArr, uint64_t vertexSize, double *rasterArray, long rasterSizeX, long rasterSizeY );
				uint8_t*  rasterToHeightMap( const double* rasterArray, const int xDim, const int yDim, const int zDim, bool scaleZ=false );
				uint8_t*  rasterToVolume( const float* rasterArray, int xDim, int yDim, int zDim );

		// import / export
		virtual bool fillPolyLines( const uint64_t& rMaxNrVertices, uint64_t& rFilled, uint64_t& rFail, uint64_t& rSkipped );
				bool exportPolyLinesCoords( std::filesystem::path rFileName, bool rWithNormals, bool rWithVertIdx );
				bool exportPolyLinesCoordsProjected( std::filesystem::path rFileName, bool rWithVertIdx, double rAngleRot=0.0 );
				bool exportPolyLinesFuncVals( std::filesystem::path rFileName );
				bool exportFuncVals( std::filesystem::path rFileName, bool rWithVertIdx );
				bool importFuncValsFromFile( const std::filesystem::path& rFileName, bool withVertIdx );
                bool importLabelsFromFile( const std::filesystem::path& rFileName, bool withVertIdx);
                bool importPolylinesFromFile( const std::filesystem::path& rFileName );
                bool importApplyTransMatFromFile( const std::filesystem::path& rFileName );

		virtual bool exportFaceNormalAngles( std::filesystem::path filename );

		// Extra menu
		virtual void cuneiformFigureLaTeX();

		// Mesh information - Details
				bool getFaceSurfSum( double* rSurfSum );
		virtual bool getPlaneVPos( std::string* rPlaneInfo );
		virtual bool getPlaneHNF( std::string* rPlaneHNF );
				bool dumpMatlabFaceNormalsSel( std::string* rStrMatlab );
		// .... LaTeX output:
				bool latexFetchFigureInfos( std::vector<std::pair<std::string, std::string>>* rStrings );
		// Mesh information - Display for the console in plain text and html for the GUI
				bool showInfoMeshHTML();
                //! @param[rWithSelfIntersectedFaces] leads to high running time
                bool getMeshInfoData( MeshInfoData& rMeshInfos, const bool rAbsolutePath, bool rWithSelfIntersectedFaces=false );
				void dumpMeshInfo( bool avoidSlow=true );
		virtual bool showInfoSelectionHTML();
		virtual bool showInfoFuncValHTML();
				bool showInfoLabelPropsHTML();
		virtual bool showInfoAxisHTML();

        //getter methods for protected area
               std::set<Vertex*> getVertices();
	protected:
		// Bounding Box:
		double             mMinX = 0.0;               //!< Bounding Box - minimum X
		double             mMaxX = 0.0;               //!< Bounding Box - maximum X
		double             mMinY = 0.0;               //!< Bounding Box - minimum Y
		double             mMaxY = 0.0;               //!< Bounding Box - maximum Y
		double             mMinZ = 0.0;               //!< Bounding Box - minimum Z
		double             mMaxZ = 0.0;               //!< Bounding Box - maximum Z

		// Datums
		std::vector<Sphere*>    mDatumSpheres;        //!< Datum list/set for spheres.
		std::vector<RectBox*>   mDatumBoxes;          //!< Datum list/set for boxes.

		// Labeling, Selections and Polyline:
		std::set<Vertex*>       mLabelSeedVerts;      //!< Vertices selected as seeds for labeling.
		std::set<Vertex*>       mSelectedMVerts;      //!< Vertices selected by user or some function e.g. by borders of labels.
		std::set<Face*>         mFacesSelected;       //!< Face selected by user or some function e.g. non-manifold.
		std::vector<PolyLine*>  mPolyLines;           //!< Polylines with or without labels.
		std::set<PolyLine*>     mPolyLinesSelected;   //!< Selected polylines.
		// Single selection of a primitive:
		Primitive*         mPrimSelected = nullptr;        //!< Pointer corresponding to the element related to the selected pixel. Will be a Face or a Vertex in most cases.

		// Selection of positions e.g. to measure distances:
		std::vector<std::tuple<Vector3D,Primitive*,bool> > mSelectedPositions;  //!< Selected coordinates, which are typically from a face or (solo) vertices.
	public:
		        bool addSelectedPosition( Vector3D rPos, Primitive* rPrimFrom, bool rLast );
		        bool getSelectedPosition( std::vector<std::tuple<Vector3D, Primitive *, bool> > *rVec );
		        bool getSelectedPositionLines( std::vector<std::tuple<Vector3D, Primitive *> > *rVec );
		        bool getSelectedPositionDistancesAll( std::vector<double>& rAllDistances );
		        bool getSelectedPositionDistancesHTML( std::string& rDistanceText );
		        bool getSelectedPositionCircleCenters( std::vector<Vertex*>* rCenterVertices );
		virtual bool getAxisFromCircleCenters();
		virtual bool getAxisFromCircleCenters( Vector3D &rTop, Vector3D &rBottom );
                bool getAxisFromEllipseFit();

		// Binary Space Partitioning -- Octree
	protected:
        Octree*   mOctree     = nullptr;          //! Octree handling the Vertices stored in mParentVertices and the mParentFaces.

		// Primitves describing the Mesh:
		std::vector<Vertex*> mVertices;   //!< Vertices of the Mesh.

	private:
		//----------------------------------------------------------------------
		std::vector<Face*>   mFaces;      //!< Faces of the Mesh.
		// Optional pre-computed information:
		//! \todo these values are only set, when a 3D-model is loaded, but NOT when feature vectors are added at a later time.
		std::vector<double>        mVerticesFeatVecMean;   //!< Mean values of all the elements of feature std::vectors of the vertices.
		std::vector<double>        mVerticesFeatVecStd;    //!< Standard deviation of all the elements of feature vectors of the vertices.

		//----------------------------------------------------------------------
		// Selection of points for a plane:
		Plane::ePlaneVerts    mPlanePosIdx = Plane::PLANE_VERT_A;  //!< Index of the next position vector of a plane to be set.
		Plane    mPlane;             //!< Mesh internal plane, used for various operations like cut, projection, etc.

		//! \todo the mConeAxisPoints and MeshParams::AXIS_PRIMEMERIDIAN are also used for the cylinder and (in the future) the sphere. Therefore the name and handling has to be adapted.
		// Selection of points for a cone:
		Vector3D    mConeAxisPoints[2] =
		        {Vector3D(0.0), Vector3D(0.0)};   //! Stores points on axis of cone (upper & lower point); the points
		                                          //! will be updated after the user has chosen two radii. If this is
		                                          //! done, the points will be the foot of perpendicular of the radius
		                                          //! point with respect to the cone axis.
		double      mConeRadius[2] = {0.0,0.0};   //! Stores radii of cone (upper & lower radius)
		int         mConeRadiusIdx = 0;           //! Stores index of cone radius
		coneStates  mConeStatus = CONE_UNDEFINED; //! Stores current status of cone selection

	protected:
		virtual bool centerAroundCone( bool rResetNormals=true );
		virtual bool unrollAroundCone( bool* rIsCylinderCase );
		virtual bool unrollAroundCylinderRadius();

	private:

		// Selection of points for a sphere:
		int      mSpherePointsIdx = 0;  //! Stores next index for sphere point position storage
		Vector3D mSpherePoints[4];      //! Stores points for sphere
		Vector3D mSpherePointNormals[4];//! Stores normals of the points for the sphere
		Vector3D mSphereCenter;         //! Stores center of sphere; calculated once four points have been selected
		double   mSphereRadius = 0.0;   //! Stores radius of sphere; calculated once four points have been selected
		bool     mCenteredAroundSphere = false; //! Flag signalling that the mesh is centered around the sphere
		bool     mUnrolledAroundSphere = false; //! Flag signalling that the mesh has been unrolled


protected:
		virtual bool centerAroundSphere();
		virtual bool unrollAroundSphere();

};
#endif
