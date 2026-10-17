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
#include <ostream>
#include <vector>
#include <set>
#include <iterator>
#include <string>
#include <algorithm>
#include <tuple>
#include <random>
#include <algorithm> // std::find_if
#include <iomanip>
#include <regex>

#include <cstdlib>

#ifdef _MSC_VER	//windows version for hostname and login
#include <winsock.h>
#include <WinBase.h>
#include <lmcons.h>
#elif defined(__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#include <lmcons.h>
#else
#include <unistd.h> // gethostname, getlogin_r
#endif
#include <climits>

#ifdef MACXPORT
    #include <sys/malloc.h> // mac os x
#endif

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/mesh/vertex.h>
#include <GigaMesh/mesh/marchingfront.h>

#include <GigaMesh/mesh/compfeaturevecs.h>

#include <GigaMesh/mesh/ellipsedisc.h>
#include <GigaMesh/logging/Logging.h>


extern "C"
{
    #include <triangle/triangle.h>
}
#include <triangle/triangleTriangleIntersection.h>

#ifdef LIBPSALM
	#include <libpsalm/libpsalm.h>
#endif

#ifdef ALGLIB
    #include <alg/corr.h>
    #include <alg/ablas.h>
#endif

#ifdef LIBSPHERICAL_INTERSECTION
    #include <spherical_intersection/algorithm/component_count.h>
    #include <spherical_intersection/algorithm/sphere_surface_msii.h>
    #include <spherical_intersection/algorithm/sphere_volume_msii.h>
    #include <spherical_intersection/graph.h>
    #include <spherical_intersection/mesh_spherical.h>
#endif

using namespace std;

#ifdef THREADS
const auto NUM_THREADS = std::thread::hardware_concurrency() * 2;

	struct faceDataStruct {
		int     mThreadID; //!< ID of the posix thread
		Mesh*   mMesh;     //!< Mesh to be processed.
		double  mAreaProc; //!< Processed area.
	};

	void* estMultiFaceConnection( faceDataStruct* rFaceData ) {
		const int   threadID = rFaceData->mThreadID;
		Mesh* const myMesh   = rFaceData->mMesh;

		LOG::info() << "[Thread " << threadID+1 << "] START one out of " << NUM_THREADS << " threads.\n";

		// Show only for one thread
		ShowProgress myThreadProgress( "[Thread 1]" );
		if( threadID == 0 ) {
			myThreadProgress.showProgressStart( "estMultiFaceConnection" );
		}

		double areaProc = 0.0;

		const uint64_t faceCount = myMesh->getFaceNr();
		for( uint64_t faceIdx=static_cast<uint64_t>(threadID); faceIdx<faceCount; faceIdx+=NUM_THREADS ) {
			Face* const currFace = myMesh->getFacePos( faceIdx );
			areaProc += currFace->getAreaNormal();
			currFace->connectToFaces();
			if( threadID == 0 ) {
				myThreadProgress.showProgress( static_cast<double>(faceIdx)/static_cast<double>(faceCount) ,
				                      "estMultiFaceConnection" );
			}
		}

		if( threadID == 0 ) {
			myThreadProgress.showProgressStop( "estMultiFaceConnection" );
		}

		rFaceData->mAreaProc = areaProc;
		LOG::info() << "[Thread " << threadID+1 << "] Processed area: " << areaProc << "\n";
		return nullptr;
	}
#endif

#define FUNCTION_VALUES_STASH \
	uint64_t nrOfVertices = getVertexNr(); \
	vector<double> funcValStash; \
	funcValStash.assign( nrOfVertices, _NOT_A_NUMBER_DBL_ ); \
	for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) { \
	    Vertex* currVertex = getVertexPos( vertIdx ); \
	    double currFuncValue; \
	    currVertex->getFuncValue( &currFuncValue ); \
	    funcValStash.at( vertIdx ) = currFuncValue; \
	}

#define FUNCTION_VALUES_STASH_RETRIEVE \
	for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) { \
	    Vertex* currVertex = getVertexPos( vertIdx ); \
	    currVertex->setFuncValue( funcValStash.at( vertIdx ) ); \
	} \
	funcValStash.clear();

#define MESHINITDEFAULTS                        \
	ShowProgress( "[Mesh]" )

//! Minimalistic constructur initalizing variables and pointers.
Mesh::Mesh()
    : MESHINITDEFAULTS {
#ifdef THREADS
    LOG::debug() << "[Mesh::" << __FUNCTION__ << "] constructed Multi-THREAD-ing Version.\n";
#else
    LOG::debug() << "[Mesh::" << __FUNCTION__ << "] constructed.\n";
#endif

}

Mesh::Mesh( const std::filesystem::path& rFileName, bool& rReadSuccess )
        : MESHINITDEFAULTS {
    showProgressStart( string( "Construct Mesh" ) );
#ifdef THREADS
    LOG::debug() << "[Mesh::" << __FUNCTION__ << "] constructed from Faces Multi-THREAD-ing Version.\n";
#else
    LOG::debug() << "[Mesh::" << __FUNCTION__ << "] constructed from Faces - NOT THREAD SAFE.\n";
#endif
	std::vector<sVertexProperties> vertexProps;
	std::vector<sFaceProperties> faceProps;
	rReadSuccess = readFile( rFileName, vertexProps, faceProps );

    if(faceProps.size()>30000000){
        std::cout << "[Mesh::" << __FUNCTION__ << "] You are processing a large mesh." << std::endl;
        std::cout << "[Mesh::" << __FUNCTION__ << "] This can cause a crash." << std::endl;
        std::cout << "[Mesh::" << __FUNCTION__ << "] Make sure your system has enough RAM or increase the swap memory!" << std::endl;
    }
	establishStructure( vertexProps, faceProps );
	showProgressStop( string( "Construct Mesh" ) );
}

//! Minimalistic constructur initalizing variables and pointers using a given face list.
Mesh::Mesh( std::set<Face*>* someFaces )
    : MESHINITDEFAULTS {
    showProgressStart( string( "Construct Mesh" ) );
#ifdef THREADS
    LOG::debug() << "[Mesh::" << __FUNCTION__ << "] constructed from Faces Multi-THREAD-ing Version.\n";
#else
    LOG::debug() << "[Mesh::" << __FUNCTION__ << "] constructed from Faces - NOT THREAD SAFE.\n";
#endif
	//! Copies structure from faces:
	//!---------------------------
	//! 1. get vertex references
	set<Vertex*> vertexTemp;
	for( auto itFace=someFaces->begin(); itFace != someFaces->end(); ++itFace ) {
		vertexTemp.insert( (*itFace)->getVertA() );
		vertexTemp.insert( (*itFace)->getVertB() );
		vertexTemp.insert( (*itFace)->getVertC() );
	}
	//! 2. set temp vertex index
	size_t tempVertIdx = 0;
	for( auto itVertex=vertexTemp.begin(); itVertex != vertexTemp.end(); ++itVertex ) {
		LOG::debug() << "Ori " << (*itVertex)->getIndexOriginal() << " will map to " << tempVertIdx << "\n";
		(*itVertex)->setIndex( tempVertIdx++ );
	}
	//! 3. allocate memory for vertices
	std::vector<sVertexProperties> vertexProps;
	vertexProps.resize( tempVertIdx );
	//! 4. copy vertex coordinates
	tempVertIdx = 0;
	for( auto itVertex=vertexTemp.begin(); itVertex != vertexTemp.end(); ++itVertex ) {
		(*itVertex)->copyVertexPropsTo( vertexProps[tempVertIdx] );
		tempVertIdx++;
	}
	//! 5. allocate face array
	std::vector<sFaceProperties> faceProps;
	faceProps.resize( someFaces->size() );
	//! 6. fill face array
	int tempFaceIdx = 0;
	for( auto itFace=someFaces->begin(); itFace != someFaces->end(); ++itFace ) {
		(*itFace)->copyFacePropsTo( faceProps[tempFaceIdx] );
		tempFaceIdx++;
	}
	//! 7. establish structure
	establishStructure( vertexProps, faceProps );

	showProgressStop( string( "Construct Mesh" ) );
}

//! Destructor. Destroys all primitives referenced by lists.
//! Does a lot of freeing memory in the following steps:
Mesh::~Mesh() {
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Destruct ...\n";
	showProgressStart( string( "Destruct Mesh" ) );
	unsigned int primCtr = 0;
	auto primTotal = mFaces.size() + mVertices.size();

	//! 1a. remove Faces.
	for(auto itFace = mFaces.begin(); itFace != mFaces.end(); ++itFace ) {
		if( primCtr % 100000 == 0 ) {
			showProgress( (1.0+static_cast<double>(primCtr))/static_cast<double>(primTotal), string( "Destruct Mesh" ) );
		}
		++primCtr;
		delete (*itFace);
	}
	mFaces.clear();
	//! 1b. remove Vertices.
	for(auto itVertex = mVertices.begin(); itVertex != mVertices.end(); ++itVertex ) {
		if( primCtr % 100000 == 0 ) {
			showProgress( (1.0+static_cast<double>(primCtr))/static_cast<double>(primTotal), string( "Destruct Mesh" ) );
		}
		++primCtr;
		delete (*itVertex);
	}
	mVertices.clear();

	//! 2. remove Datums
	//! \bug parsing thru sphereList and boxList and removing its items causes a crash, when elements are present.
	//vector<Sphere*>::iterator itSphere;
	//for( itSphere=sphereList.begin(); itSphere != sphereList.end(); itSphere++ ) {
	//	delete (*itSphere);
	//}

	for(Sphere*& spherePtr : mDatumSpheres)
	{
		delete spherePtr;
	}

	mDatumSpheres.clear();
	//vector<RectBox*>::iterator itBox;
	//for( itBox=boxList.begin(); itBox != boxList.end(); itBox++ ) {
	//	delete (*itBox);
	//}

	for(RectBox*& rectBoxPtr : mDatumBoxes)
	{
		delete rectBoxPtr;
	}

	mDatumBoxes.clear();

	//! 3. clean selected vertices
	mSelectedMVerts.clear();

	//! 4. clean polygonal lines
	removePolylinesAll();

	if( mOctree != nullptr ) {
		delete mOctree;
		mOctree = nullptr;
	}
    /**
	if(mOctreeface != nullptr) {
		delete mOctreeface;
		mOctreeface = nullptr;
	}
    **/
	showProgressStop( string( "Destruct Mesh" ) );
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Done.\n";
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Set a floating parameter (double) of the Mesh class.
//! @returns: true, when the value was changed. false otherwise.
bool Mesh::setParamFloatMesh( MeshParams::eParamFlt rParam, double rValue ) {
	return MeshParams::setParamFloatMesh( rParam, rValue );
}

//! Call requested method/function.
//! See MeshParams::eFunctionCall
bool Mesh::callFunction(
		MeshParams::eFunctionCall rFunctionID,
		bool rFlagOptional // [[maybe_unused]] should be here, but causes troubles with qt creator finding references
) {
	bool retVal = false;
	switch( rFunctionID ) {
		case FILE_SAVE_AS:
			retVal = writeFileUserInteract();
			break;
        case EXPORT_AS_LEGACY:
            retVal = writeFileUserInteract(true);
            break;
		case EXPORT_CONNECTED_COMPONENTS:
			retVal = writeFilesForConnectedComponents();
			break;
		case EXPORT_METADATA_HTML: {
				MeshInfoData metaInfo;
				getMeshInfoData( metaInfo, true );
				std::filesystem::path htmlFileName = getFullName().replace_extension( ".html" );
				if( metaInfo.writeMeshInfo( htmlFileName ) ) {
					showInformation( "Meta-Data HTML page", std::string("Sidecar file saved as:<br />")+htmlFileName.string() );
				}
			} break;
		case EXPORT_METADATA_JSON: {
				MeshInfoData metaInfo;
				getMeshInfoData( metaInfo, true );
				std::filesystem::path jsonFileName = getFullName().replace_extension( ".json" );
				if( metaInfo.writeMeshInfo( jsonFileName ) ) {
					showInformation( "Meta-Data JSON", std::string("Sidecar file saved as:<br />")+jsonFileName.string() );
				}
			} break;
		case EXPORT_METADATA_TTL: {
				MeshInfoData metaInfo;
				getMeshInfoData( metaInfo, true );
				std::filesystem::path ttlFileName = getFullName().replace_extension( ".ttl" );
				if( metaInfo.writeMeshInfo( ttlFileName ) ) {
					showInformation( "Meta-Data TTL (Turtle, RDF)", std::string("Sidecar file saved as:<br />")+ttlFileName.string() );
				}
			} break;
		case EXPORT_METADATA_XML: {
				MeshInfoData metaInfo;
				getMeshInfoData( metaInfo, true );
				std::filesystem::path xmlFileName = getFullName().replace_extension( ".xml" );
				if( metaInfo.writeMeshInfo( xmlFileName ) ) {
					showInformation( "Meta-Data XML", std::string("Sidecar file saved as:<br />")+xmlFileName.string() );
				}
			} break;
		case EXPORT_METADATA_ALL: {
				MeshInfoData metaInfo;
				getMeshInfoData( metaInfo, true );
				std::filesystem::path htmlFileName = getFullName().replace_extension( ".html" );
				std::filesystem::path jsonFileName = getFullName().replace_extension( ".json" );
				std::filesystem::path ttlFileName = getFullName().replace_extension( ".ttl" );
				std::filesystem::path xmlFileName = getFullName().replace_extension( ".xml" );
				if( !metaInfo.writeMeshInfo( htmlFileName ) ) {
					showWarning( "Meta-Data HTML page", std::string("Could NOT be saved!") );
					break;
				}
				if( !metaInfo.writeMeshInfo( jsonFileName ) ) {
					showWarning( "Meta-Data JSON", std::string("Could NOT be saved!") );
					break;
				}
				if( !metaInfo.writeMeshInfo( ttlFileName ) ) {
					showWarning( "Meta-Data TTL (Turtle, RDF)", std::string("Could NOT be saved!") );
					break;
				}
				if( !metaInfo.writeMeshInfo( xmlFileName ) ) {
					showWarning( "Meta-Data XML", std::string("Could NOT be saved!") );
					break;
				}
				showInformation( "Meta-Data", std::string("Sidecar files saved as:<br />") +
				                              htmlFileName.string() + "<br />" +
				                              jsonFileName.string() + "<br />" +
				                              ttlFileName.string() + "<br />" +
				                              xmlFileName.string() + "<br />" );
			} break;
		case PLANE_FLIP:
			retVal = flipPlane();
			break;
		case SELPRIM_VERTEX_BY_INDEX:
			retVal = selectPrimitiveVertexUser();
			break;
		case SELMVERTS_FLAG_SYNTHETIC:
			retVal = selVertByFlag( FLAG_SYNTHETIC );
			break;
		case SELMVERTS_FLAG_CIRCLE_CENTER:
			retVal = selVertByFlag( FLAG_CIRCLE_CENTER );
			break;
		case SELMVERTS_FLAG_CORTEX:
			retVal = selVertByFlag( FLAG_CORTEX );
			break;
		case SELMVERTS_INVERT:
			retVal = selectVertInvert();
			break;
		case SELMVERTS_LABEL_IDS:
			retVal = selectVertLabelNo();
			break;
		case SELMVERTS_LABEL_BACKGROUND:
			retVal = selectVertLabelBackGrd();
			break;
		case SELMVERTS_FROMSELMFACES:
			retVal = selectVertFromFaces();
			break;
		case SELMVERTS_RIDGES:
			retVal = selectVertFromFacesRidges();
			break;
		case SELMVERTS_SHOW_INDICES:
			retVal = selectVertsByIdxShow();
			break;
		case SELMVERTS_SELECT_INDICES:
			retVal = selectVertsByIdxUser();
			break;
		case SELMVERTS_RANDOM:
			retVal = selectVertsRandomUser();
			break;
		case SELMFACES_WITH_SYNTHETIC_VERTICES:
			retVal = selectFaceWithSyntheticVertices();
			break;
		case SELMFACES_WITH_THREE_BORDER_VERTICES:
			retVal = selectFaceBorderThreeVertices();
			break;
		case SELMFACES_WITH_THREE_SELECTED_VERTICES:
			retVal = selectFaceThreeVerticesSelected();
			break;
		case SELMFACES_BORDER_BRIDGE_TRICONN:
			retVal = selectFaceBorderBridgeTriConnect();
			break;
		case SELMFACES_BORDER_BRIDGE:
			retVal = selectFaceBorderBridge();
			break;
		case SELMFACES_BORDER_DANGLING:
			retVal = selectFaceBorderDangling();
			break;
		case SELMFACES_LABEL_CORNER:
			retVal = selectFaceLabeledVerticesCorner();
			break;
		case SELMPOLY_BY_VERTEX_COUNT: {
			vector<long> valuesMinMax;
			if( !showEnterText( valuesMinMax, "Select PolyLine by Vertex count" ) ) {
				break;
			}
			if( valuesMinMax.size() == 1 ) { // Single Value
				uint64_t valMinMax = valuesMinMax.at(0);
				retVal = selectPolyVertexCount( valMinMax, valMinMax );
			} else if( valuesMinMax.size() == 2 ) { // Value Range
				uint64_t valMin = valuesMinMax.at(0);
				uint64_t valMax = valuesMinMax.at(1);
				retVal = selectPolyVertexCount( valMin, valMax );
			}
			retVal = true;
			} break;
		case SELECT_MESH_PLANE_AXIS_SELPRIM:
			retVal = setPlaneHNFbyAxisSelPrim();
			break;
		case SELECT_MESH_PLANE_AXIS_SELPOS:
			retVal = setPlaneHNFbyAxisAndLastPosition();
			break;
		case ORIENT_MESH_PLANE_TO_AXIS:
			retVal = orientPlaneHNFbyAxis();
			break;
		case FEATUREVEC_MEAN_ONE_RING_REPEAT:
			retVal = featureVecMedianOneRingUI( true );
			break;
		case FEATUREVEC_MEDIAN_ONE_RING_REPEAT:
			retVal = featureVecMedianOneRingUI( false );
			break;
		case FEATUREVEC_UNLOAD_ALL:
			removeFeatureVectors();
			break;
		case FUNCVAL_FEATUREVECTOR_STDDEV_ELEMENTS:
			retVal = funcVertFeatureElementsStdDev();
			break;
		case FUNCVAL_FEATUREVECTOR_CORRELATE_WITH: {
			vector<double> refernceVector;
			if( !showEnterText( refernceVector, "Reference vector" ) ) {
				break;
			}
			retVal |= setVertFuncValCorrTo( &refernceVector );
			} break;
		case FUNCVAL_FEATUREVECTOR_APPLY_PNORM:
			retVal = funcVertFeatureVecPNorm();
			break;
		case FUNCVAL_FEATUREVECTOR_APPLY_MAHALANOBIS:
			retVal = funcVertFeatureVecMahalDist();
			break;
		case FUNCVAL_FEATUREVECTOR_MIN_ELEMENT:
			retVal = funcVertFeatureVecMin();
			break;
		case FUNCVAL_FEATUREVECTOR_MAX_ELEMENT:
			retVal = funcVertFeatureVecMax();
			break;
		case FUNCVAL_FEATUREVECTOR_MIN_ELEMENT_SIGNED:
			retVal = funcVertFeatureVecMinSigned();
			break;
		case FUNCVAL_FEATUREVECTOR_MAX_ELEMENT_SIGNED:
			retVal = funcVertFeatureVecMaxSigned();
			break;
		case FUNCVAL_FEATUREVECTOR_ELEMENT_BY_INDEX: {
			uint64_t valueIndex = _NOT_A_NUMBER_UINT_;
			if( !showEnterText( valueIndex, "Select Featurevector Element No." ) ) {
				break;
			}
			retVal = funcVertFeatureVecElementByIndex( valueIndex );
			} break;
		case FUNCVAL_DISTANCE_TO_SELPRIM:
			retVal = setVertFuncValDistanceToSelPrim();
			break;
		case FUNCVAL_PLANE_ANGLE:
			retVal = setVertFuncValSlope();
			break;
		case FUNCVAL_ANGLE_TO_RADIAL:
			retVal = setVertFuncValAngleToRadial();
			break;
		case FUNCVAL_AXIS_ANGLE_TO_RADIAL:
			retVal = setVertFuncValAxisAngleToRadial();
			break;
		case FUNCVAL_ORTHOGONAL_AXIS_ANGLE_TO_RADIAL:
			retVal = setVertFuncValOrthogonalAxisAngleToRadial();
			break;
		// Color - Gralevel to Function Value
		case FUNCVAL_SET_GRAY_RGB_AVERAGE:
			retVal = setVertFuncValGraylevel( Vertex::RGB_TO_GRAY_AVERAGE );
			break;
		case FUNCVAL_SET_GRAY_RGB_AVERAGE_WEIGHTED:
			retVal = setVertFuncValGraylevel( Vertex::RGB_TO_GRAY_AVERAGE_WEIGHTED );
			break;
		case FUNCVAL_SET_GRAY_SATURATION_REMOVAL:
			retVal = setVertFuncValGraylevel( Vertex::RGB_TO_GRAY_SATURATION_REMOVAL);
			break;
		case FUNCVAL_SET_GRAY_HSV_DECOMPOSITION:
			retVal = setVertFuncValGraylevel( Vertex::RGB_TO_GRAY_HSV_DECOMPOSITION );
			break;
		case FUNCVAL_SET_DISTANCE_TO_LINE:
			retVal = setVertFuncValDistanceToLinePosDir();
			break;
		case FUNCVAL_SET_DISTANCE_TO_SPHERE:
			retVal = setVertFuncValDistanceToSphere();
			break;
		case FUNCVAL_SET_DISTANCE_TO_AXIS:
			retVal = setVertFuncValDistanceToAxis();
			break;
		case FUNCVAL_SET_ANGLE_USING_AXIS: {
			Vector3D axisTop;
			Vector3D axisBottom;
			retVal = getConeAxis( &axisTop, &axisBottom );
			if( retVal ) {
				retVal = setVertFuncValAngleBasedOnAxis( &axisBottom, &axisTop );
			}
		    } break;
		case FUNCVAL_MULTIPLY_SCALAR:
			retVal = setVertFuncValMult();
			break;
		case FUNCVAL_TO_ORDER:
			retVal = setVertFuncValToOrder();
			break;
		case FUNCVAL_MEAN_ONE_RING_REPEAT:
			retVal = funcVertMedianOneRingUI( true );
			break;
		case FUNCVAL_MEDIAN_ONE_RING_REPEAT:
			retVal = funcVertMedianOneRingUI( false );
			break;
		case FUNCVAL_ADJACENT_FACES:
			retVal = funcVertAdjacentFaces();
			break;
		case FUNCVAL_DISTANCE_TO_SEED_MARCHING: {
			double radiusStop;
			if( !showEnterText( radiusStop, "Enter a radius as stop criteria for the marching front" ) ) {
				break;
			}
			retVal = setFaceFuncValMarchRadiusIdx( mPrimSelected, radiusStop );
			} break;
		case FUNCVAL_VERT_ONE_RING_AREA:
			retVal = setVertFuncVal1RingArea();
			break;
		case FUNCVAL_VERT_ONE_RING_ANGLE_SUM:
			retVal = setVertFuncVal1RSumAngles();
			break;
		case FUNCVAL_VERT_MAX_DISTANCE:
			retVal = funcVertDistancesMax();
			break;
		case FUNCVAL_FACE_SORT_INDEX:
			retVal = setFaceFuncValSortIdx();
			break;
		case FUNCVAL_SPHERE_SURFACE_LENGTH:
			retVal = funcVertSphereSurfaceLength();
			break;
		case FUNCVAL_SPHERE_VOLUME_AREA:
			retVal = funcVertSphereVolumeArea();
			break;
		case FUNCVAL_SPHERE_SURFACE_NUMBER_OF_COMPONENTS:
			retVal = funcVertSphereSurfaceNumberOfComponents();
			break;
		// Edit
		case EDIT_REMOVE_SELMFACES:
			retVal &= removeFacesSelected();
			break;
		case EDIT_REMOVE_FACESZERO:
			retVal = removeFacesZeroArea();
			break;
		case EDIT_REMOVE_FACES_BORDER_EROSION:
			retVal = removeFacesBorderErosion();
			break;
		case EDIT_AUTOMATIC_POLISHING:
			retVal = completeRestore();
			break;
		case EDIT_REMOVE_SEEDED_SYNTHETIC_COMPONENTS:
			retVal = removeSyntheticComponents( mSelectedMVerts );
			break;
		case EDIT_VERTICES_RECOMPUTE_NORMALS: {
			double radiusNormals{0.0};
			if( !showEnterText( radiusNormals, "Enter a radius to compute the normals" ) ) {
				break;
			}
			retVal &= resetFaceNormals();
			if( radiusNormals <= 0.0 ) {
				retVal &= resetVertexNormals();
			} else {
				retVal &= normalsVerticesComputeSphere( radiusNormals );
			}
			} break;
		case EDIT_VERTICES_ADD:
			retVal = insertVerticesEnterManual();
			break;
		case EDIT_SPLIT_BY_PLANE: {
			Vector3D planeHNF = mPlane.getHNF();
			retVal &= Mesh::splitByPlane( planeHNF, true, false );
			} break;
		case EDIT_FACES_INVERT_ORIENTATION:
			retVal &= applyInvertOrientationFaces();
			break;
		case APPLY_TRANSMAT_ALL: {
			Matrix4D valuesMatrix4x4;
			if( !showEnterText( &valuesMatrix4x4 ) ) {
				break;
			}
			retVal = applyTransformationToWholeMesh( valuesMatrix4x4 );
			} break;
		case APPLY_TRANSMAT_ALL_SCALE: {
			std::vector<double> valuesScaleSkew;
			if( !showEnterText( valuesScaleSkew, "One value for uniform scale. Three values for skewed scaling:" ) ) {
				break;
			}
			if( valuesScaleSkew.size() == 1 ) {
				Matrix4D valuesMatrix4x4( Matrix4D::INIT_SCALE, &valuesScaleSkew );
				retVal = applyTransformationToWholeMesh( valuesMatrix4x4 );
				break;
			}
			if( valuesScaleSkew.size() == 3 ) {
				Matrix4D valuesMatrix4x4( Matrix4D::INIT_SKEW, &valuesScaleSkew );
				retVal = applyTransformationToWholeMesh( valuesMatrix4x4 );
				break;
			}
			LOG::error() << "[Mesh::" << __FUNCTION__ << "] ERROR: Bad number of values given. Expecting one or three values, but "<< valuesScaleSkew.size() << " were given!\n";
			retVal = false;
			} break;
		case APPLY_TRANSMAT_SELMVERT: {
			if(hasDatumObjects())
			{
				bool proceed = true;
				if(!showQuestion(&proceed, "Warning", "Warning: There are datum-objects in the scene that will be deleted by this operation.\nDo you want to continue?"))
					break;

				if(!proceed)
					break;
			}

			Matrix4D valuesMatrix4x4;
			if( !showEnterText( &valuesMatrix4x4, true ) ) {
				break;
			}
			retVal = applyTransformation( valuesMatrix4x4, &mSelectedMVerts );
			} break;

		case SELMPRIMS_POS_DESELECT_ALL:
			mSelectedPositions.clear();
			selectedMPositionsChanged();
			retVal = true;
			break;
		case SELMPRIMS_POS_DISTANCES: {
			std::string msgInfo;
			retVal = getSelectedPositionDistancesHTML( msgInfo );
			showInformation( "Selected Positions (SelPos) - Distances", msgInfo );
			} break;
		case SELMPRIMS_POS_CIRCLE_CENTERS:
			if( mSelectedPositions.size() < 3 ) {
				showInformation( "Positions - Compute Circle Centers", "Too few Selected." );
			} else {
				vector<Vertex*> circleCenterVertices;
				retVal  = getSelectedPositionCircleCenters( &circleCenterVertices );
				retVal &= insertVertices( &circleCenterVertices );
				// Ask user to continue:
				if( getAxisFromCircleCenters() ) {
					retVal &= callFunction( AXIS_FROM_CIRCLE_CENTERS );
				}
			}
			break;
		case COMPUTE_FEATUREVECTORS_QUICK:
			retVal = computeMSIIQuickGUI();
			break;
		case GEODESIC_DISTANCE_TO_SELPRIM:
			retVal = estGeodesicPatchSelPrim();
			break;
		case POLYLINES_FROM_MULTIPLE_FUNCTION_VALUES:
			retVal = isolineToPolylineMultiple();
			break;
		case POLYLINES_FROM_FUNCTION_VALUE:
			retVal = isolineToPolyline();
			break;
		case POLYLINES_FROM_PLANE_INTERSECTIONS:
			retVal = planeIntersectionToPolyline();
			break;
		case POLYLINES_FROM_AXIS_AND_POSTIONS:
			retVal = planeIntersectionsAxisAndPositions();
			break;
		case POLYLINES_REMOVE_SELECTED:
			retVal &= removePolylinesSelected();
			break;
		case POLYLINES_REMOVE_ALL:
			retVal &= removePolylinesAll();
			break;
		case AXIS_FROM_CIRCLE_CENTERS: {
			Vector3D topPoint;
			Vector3D bottomPoint;
			retVal  = getAxisFromCircleCenters( topPoint, bottomPoint );
			retVal &= setConeAxis( &topPoint, &bottomPoint );
			} break;
		case AXIS_ENTER_PRIMEMERIDIAN_ROTATION: {
			// IMPLEMENTED on GUI level due to user interaction.
			} break;
		case AXIS_SET_PRIMEMERIDIAN_SELPRIM:
			if( mPrimSelected != nullptr ) {
				Vector3D posSel = mPrimSelected->getCenterOfGravity();
				double angle = posSel.angleInLineCoord( &mConeAxisPoints[0], &mConeAxisPoints[1] );
				retVal = setParamFloatMesh( MeshParams::AXIS_PRIMEMERIDIAN, angle + M_PI );
			}
			break;
		case AXIS_SET_CUTTINGMERIDIAN_SELPRIM:
			if( mPrimSelected != nullptr ) {
				Vector3D posSel = mPrimSelected->getCenterOfGravity();
				double angle = posSel.angleInLineCoord( &mConeAxisPoints[0], &mConeAxisPoints[1] );
				retVal = setParamFloatMesh( MeshParams::AXIS_PRIMEMERIDIAN, angle );
			}
			break;
		case UNROLL_AROUND_CONE:
			if(hasDatumObjects())
			{
				bool proceed = true;
				if(!showQuestion(&proceed, "Warning", "Warning: There are datum-objects in the scene that will be deleted by this operation.\nDo you want to continue?"))
					break;

				if(!proceed)
					break;
			}

			removeAllDatumObjects();
			bool isCylinderCase;
			retVal = unrollAroundCone( &isCylinderCase );
			break;
		case UNROLL_AROUNG_CYLINDER:
			if(hasDatumObjects())
			{
				bool proceed = true;
				if(!showQuestion(&proceed, "Warning", "Warning: There are datum-objects in the scene that will be deleted by this operation.\nDo you want to continue?"))
					break;

				if(!proceed)
					break;
			}

			removeAllDatumObjects();
			retVal = unrollAroundCylinderRadius();
			break;
		case CONE_COVER_MESH:
			retVal = setConeCoverMesh();
			break;
		case EXTRUDE_POLYLINES:
			retVal = extrudePolylines();
			break;
		case LABELING_LABEL_ALL: {
			labelVerticesNone();
			retVal = Mesh::labelVertices( mVertices, mLabelSeedVerts );
			} break;
		case LABELING_LABEL_SELMVERTS:
			retVal = Mesh::labelSelectedVerticesUser();
			break;
        case LABELING_KMEANS_VERT_POS:
            labelKMeansVertPos();
            break;
		case REFRESH_SELECTION_DISPLAY:
			selectedMVertsChanged();
			selectedMFacesChanged();
			selectedMPolysChanged();
			selectedMPositionsChanged();
			break;
		case SHOW_INFO_MESH:
			showInfoMeshHTML(); // for plain text use: dumpMeshInfo( false );
			break;
		case SHOW_INFO_SELECTION:
			showInfoSelectionHTML();
			break;
		case SHOW_INFO_FUNCVAL:
			showInfoFuncValHTML();
			break;
		case SHOW_INFO_LABEL_PROPS:
			showInfoLabelPropsHTML();
			break;
		case SHOW_INFO_AXIS:
			showInfoAxisHTML();
			break;
		case LATEX_TEMPLATE:
			cuneiformFigureLaTeX();
			break;
		case METADATA_EDIT_MODEL_ID: {
			string metaData = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
			if( showEnterText( metaData, "Edit the Id of the model" ) ) {
				getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_ID, metaData );
			}
			} break;
		case METADATA_EDIT_MODEL_MATERIAL: {
			string metaData = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
			if( showEnterText( metaData, "Edit the material of the object" ) ) {
				getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_MATERIAL, metaData );
			}
			} break;
		case METADATA_EDIT_REFERENCE_WEB: {
			string metaData = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_REFERENCE_WEB );
			if( showEnterText( metaData, "Edit web reference of the model" ) ) {
				getModelMetaDataRef().setModelMetaString( ModelMetaData::META_REFERENCE_WEB, metaData );
			}
			} break;
		case ELLIPSENFIT_EXPERIMENTAL:
            if( mSelectedMVerts.size() < 5){
                showInformation("To few selected vertices!","For the ellipsefit are 5 selected vertices needed!");
            }
            else{
                getAxisFromEllipseFit();
            }
			break;
		case DRAW_SELF_INTERSECTIONS:
			selectFaceSelfIntersecting();
			break;
	    case SELMVERTS_SET_ALPHA: {
			uint64_t alphaVal = 255;
			if(!showEnterText(alphaVal, "Enter Value [0-255]"))
			{
				return false;
			}
			assignAlphaToSelectedVertices(static_cast<unsigned char>(alphaVal));
	        } break;
		default:
			LOG::error() << "[Mesh::" << __FUNCTION__ << "] ERROR: Unknown rFunctionID "<< rFunctionID << " !\n";
		    return( false );
	}

	return( retVal );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

#define SHOW_MALLOC_STATS( nr ) { \
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] +++ MALLOC "<< (nr) << " ++++++++++++++++++++++++++++++++++++++\n"; \
	malloc_stats(); \
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] +++++++++++++++++++++++++++++++++++++++++++++++++++\n"; \
}
#undef SHOW_MALLOC_STATS // comment for dumping memory usage to stdout

//! Establish our Mesh structure as linked objects -- to be called right
//! after a file is read!!!
//!
//! Requires the following attributes to be set properly:
//! vertexCoords, vertexCoordsNr, textureRGB, textureRGBNr, facesMeshed and facesMeshedNr
void Mesh::establishStructure(
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	int timeStart = clock(); // for performance mesurement

    #ifdef SHOW_MALLOC_STATS
	    SHOW_MALLOC_STATS( 1 );
    #endif

	// Prepare array for faces:
	for(auto & face : mFaces) {
		delete face;
	}
	mFaces.clear();

	// Prepare array for vertices:
	for(auto & vertex : mVertices) {
		delete vertex;
	}
	mVertices.clear();

    #ifdef SHOW_MALLOC_STATS
	    SHOW_MALLOC_STATS( 2 );
    #endif

	// Bounding Box:
	mMinX = +DBL_MAX;
	mMaxX = -DBL_MAX;
	mMinY = +DBL_MAX;
	mMaxY = -DBL_MAX;
	mMinZ = +DBL_MAX;
	mMaxZ = -DBL_MAX;
	double* featureVecVerticesPtr = mFeatureVecVertices.data();
	for(size_t i=0; i<rVertexProps.size(); ++i ) {
		VertexOfFace* newVert = new VertexOfFace( i, rVertexProps[i] );
		// Assign feature vectors, when present:
		if( mFeatureVecVerticesLen > 0 ) {
			newVert->assignFeatureVec( featureVecVerticesPtr+(i*mFeatureVecVerticesLen), mFeatureVecVerticesLen );
		}
		// Bounding Box:
		if( mMinX > rVertexProps.at( i ).mCoordX ) {
			mMinX = rVertexProps.at( i ).mCoordX;
		}
		if( mMaxX < rVertexProps.at( i ).mCoordX ) {
			mMaxX = rVertexProps.at( i ).mCoordX;
		}
		if( mMinY > rVertexProps.at( i ).mCoordY ) {
			mMinY = rVertexProps.at( i ).mCoordY;
		}
		if( mMaxY < rVertexProps.at( i ).mCoordY ) {
			mMaxY = rVertexProps.at( i ).mCoordY;
		}
		if( mMinZ > rVertexProps.at( i ).mCoordZ ) {
			mMinZ = rVertexProps.at( i ).mCoordZ;
		}
		if( mMaxZ < rVertexProps.at( i ).mCoordZ ) {
			mMaxZ = rVertexProps.at( i ).mCoordZ;
		}
		mVertices.push_back( newVert );
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------------
	// Initalize storage for precomuted information about the feature vectors
	changedVertFeatureVectors();
	mFeatureVecVertices.clear(); // Can be clearad, because of newVert->assignFeatureVec (see above)
	//------------------------------------------------------------------------------------------------------------------------------------------------------

    #ifdef SHOW_MALLOC_STATS
	    SHOW_MALLOC_STATS( 3 );
    #endif

	Face *myFace;
	mFaces.resize( rFaceProps.size() );
	uint64_t facesAddedToMesh = 0;
	for( uint64_t i=0; i<rFaceProps.size(); ++i ) {
		//cout << "Face: " << i << " " << facesMeshed[i*3]-1 << ", " << facesMeshed[i*3+1]-1 << ", " << facesMeshed[i*3+2]-1 << endl;

		if(rFaceProps[i].vertexIndices.size() < 3)
		{
			continue;
		}

		//!TODO: handle case, where rFaceProps[i] contains an ngon => currently handled in MeshIO
		uint64_t vertAIdx = rFaceProps[i].vertexIndices[0];
		uint64_t vertBIdx = rFaceProps[i].vertexIndices[1];
		uint64_t vertCIdx = rFaceProps[i].vertexIndices[2];
		if( vertAIdx >= rVertexProps.size() ) {
			LOG::warn() << "[Mesh::" << __FUNCTION__ << "] Vertex A index out of range: " << vertAIdx <<
						 " ... ignoring Face no. " << i << "!\n";
			continue;
		}
		if( vertBIdx >= rVertexProps.size() ) {
			LOG::warn() << "[Mesh::" << __FUNCTION__ << "] Vertex B index out of range: " << vertBIdx <<
						 " ... ignoring Face no. " << i << "!\n";
			continue;
		}
		if( vertCIdx >= rVertexProps.size() ) {
			LOG::warn() << "[Mesh::" << __FUNCTION__ << "] Vertex C index out of range: " << vertCIdx <<
						 " ... ignoring Face no. " << i << "!\n";
			continue;
		}
		// Create new face
		try {
			myFace = new Face( facesAddedToMesh,
			           static_cast<VertexOfFace*>(mVertices[vertAIdx]),
			           static_cast<VertexOfFace*>(mVertices[vertBIdx]),
			           static_cast<VertexOfFace*>(mVertices[vertCIdx])
			         );
		} catch ( std::bad_alloc& errBadAlloc ) {
			LOG::error() << "[Mesh::" << __FUNCTION__ << "] ERROR: bad_alloc caught at index " << i << ": " << errBadAlloc.what() << "\n";
			continue;
		}

		//!TODO: handle case, where rFaceProps[i] contains an ngon => currently handled in MeshIO
		std::array<float,6> textureCoordinates{0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F};

		if(rFaceProps[i].textureCoordinates.size() >= 6)
		{
			for(size_t j = 0; j<6;++j)
			{
				textureCoordinates[j] = rFaceProps[i].textureCoordinates[j];
			}
		}

		myFace->setUVs(textureCoordinates);
		myFace->setTextureId(rFaceProps[i].textureId);
		// Add face to the list
		try {
			mFaces[facesAddedToMesh] = myFace;
		} catch ( std::bad_alloc& errBadAlloc ) {
			LOG::error() << "[Mesh::" << __FUNCTION__ << "] ERROR: bad_alloc caught when pushing index " << i << ": " << errBadAlloc.what() << "\n";
			continue;
		}
		// Count sucessfully added faces
		++facesAddedToMesh;

		// Don't do this as it is extremly slow (approx. >>50x than using the temporary vector:
		//Face *myFace = new Face( faceIdx, getVertexByIdx(...), getVertexByIdx(...), getVertexByIdx(...) );

		// For debugging:
		//if( i % 1000 == 0 ) {
		//	cout << "[Mesh::" << __FUNCTION__ << "] Faces set: " << mFaces.size() << "(" << (i*100.0/facesMeshedNr) << "%)" << endl;
		//}
	}
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] Faces set: " << mFaces.size() << "\n";
	if( rFaceProps.size() != facesAddedToMesh ) {
		LOG::warn() << "[Mesh::" << __FUNCTION__ << "] ERROR: Number of faces created: " <<
					 facesAddedToMesh << " is smaller than number of faces given " << rFaceProps.size() << "\n";
		LOG::warn() << "[Mesh::" << __FUNCTION__ << "]        Therefore " << rFaceProps.size() - facesAddedToMesh <<
					 " faces were ignored!\n";
		// Shrinking is required:
		mFaces.resize( facesAddedToMesh );
		// Inform user
		showWarning( "Mesh import incomplete", "Not all faces could be imported due to out-of-range indices." );
	}

	if( rFaceProps.empty() ) {
		LOG::info() << "[Mesh::" << __FUNCTION__ << "] Point cloud dedected - no further Mesh-setup possible!\n";
		return;
	}

    #ifdef SHOW_MALLOC_STATS
	    SHOW_MALLOC_STATS( 4 );
    #endif

	// Polylines:
	removePolylinesAll();
	for( unsigned int i=0; i<MeshSeedExt::getPolyLineNr(); ++i ) {
		//cout << "[Mesh::" << __FUNCTION__ << "] Polyline: ";
		PolyLine* tmpPolyLine = new PolyLine( Vector3D( _NOT_A_NUMBER_, _NOT_A_NUMBER_, _NOT_A_NUMBER_, 1.0 ),
		                                      Vector3D( _NOT_A_NUMBER_, _NOT_A_NUMBER_, _NOT_A_NUMBER_, 0.0 ) );
		// The references to the vertices:
		for( unsigned int j=0; j<MeshSeedExt::getPolyLineLength( i ); ++j ) {
			int vertIdxPoly = MeshSeedExt::getPolyLineVertIdx( i, j );
			tmpPolyLine->addBack( mVertices.at( vertIdxPoly ) );
			//cout << vertIdxPoly << " ";
		}
		// if we have basic information:
		if( static_cast<unsigned int>(mPolyPrimInfo.size()) == MeshSeedExt::getPolyLineNr() ) { // TODO: Check whether it is correct to use
			                                                           // integers for the numbers; using something
			                                                           // such as `size_t` may be more suited.
			PrimitiveInfo primInfo = mPolyPrimInfo.at( i );
			if( !tmpPolyLine->setPosition( primInfo.mPosX, primInfo.mPosY, primInfo.mPosZ ) ) {
				LOG::warn() << "[Mesh::" << __FUNCTION__ << "] ERROR: PolyLine::setPosition failed!\n";
			}
			if( !tmpPolyLine->setNormal( primInfo.mNormalX, primInfo.mNormalY, primInfo.mNormalZ ) ) {
				LOG::warn() << "[Mesh::" << __FUNCTION__ << "] ERROR: PolyLine::setNormal failed!\n";
			}
		}
		// if we have a label ID:
		if( static_cast<unsigned int>(mPolyLabelID.size()) == MeshSeedExt::getPolyLineNr() ) { // TODO: Check whether it is correct to use
			                                                           // integers for the numbers; using something
			                                                           // such as `size_t` may be more suited.
			if( !tmpPolyLine->setLabel( mPolyLabelID.at( i ) ) ) {
				LOG::warn() << "[Mesh::" << __FUNCTION__ << "] ERROR: PolyLine::setLabel failed!\n";
			//} else {
			//	cout << "[Mesh::" << __FUNCTION__ << "] PolyLine::setLabel( " << mPolyLabelID.at( i ) << " )!" << endl;
			}
		}
		mPolyLines.push_back( tmpPolyLine );
		//cout << endl;
	}
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] Polygonal Lines: " << mPolyLines.size() << "\n";

	LOG::info() << "[Mesh::" << __FUNCTION__ << "] done in " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds.\n";

    #ifdef SHOW_MALLOC_STATS
	    SHOW_MALLOC_STATS( 6 );
    #endif

	//------------------------------------------------------------------------------------------------------------------------------------------------
	// BASIC MESH SETUP (for non-Point Clouds)
	//------------------------------------------------------------------------------------------------------------------------------------------------

	//#ifdef SHOW_MALLOC_STATS
	//	SHOW_MALLOC_STATS( 8 );
	//#endif

#ifdef THREADS
	time_t timeStampParallel = time( nullptr );
	std::vector<std::thread> threads(NUM_THREADS);
	/* Initialize and set thread detached attribute */
	//pthread_mutex_init( &mutexVertexPtr, NULL );

	std::vector<faceDataStruct> setFaceData(NUM_THREADS);
	for( unsigned int t=0; t<NUM_THREADS; ++t ) {
		//cout << "[Mesh::" << __FUNCTION__ << "] Preparing data for thread " << t << endl;
		setFaceData[t].mThreadID               = t;
		setFaceData[t].mMesh                   = this;
		setFaceData[t].mAreaProc               = 0.0;
	}

	for( unsigned int t=0; t<NUM_THREADS; t++ ) {
		threads[t] = std::thread(estMultiFaceConnection, &setFaceData[t]);
	}

	/* wait for the other threads */
	double areaTotal = 0.0;
	for( unsigned int t=0; t<NUM_THREADS; t++ ) {
		threads[t].join();
		//cout << "[Mesh::" << __FUNCTION__ << "] Thread " << t << " processed faces with an area of: " << setMeshData[t].mAreaProc << " mm² (unit assumed)." << endl;
		areaTotal += setFaceData[t].mAreaProc;
	}
	//pthread_mutex_destroy( &mutexVertexPtr );
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] Total surface area: " << areaTotal << " mm² (unit assumed).\n";
	LOG::info() << "[Mesh::" << __FUNCTION__ << "]                     " << getVertexNr()/areaTotal << " dots/mm² (unit assumed).\n";
	LOG::info() << "[Mesh::" << __FUNCTION__ << "]                     " << 25.4*sqrt(getVertexNr()/areaTotal) << " DPI (unit assumed).\n";
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] PARALLEL: Face neighbourhood and normal estimation time: " << static_cast<int>( time( nullptr ) - timeStampParallel )  << " seconds.\n";
#else
	time_t timeStampSerial = time( NULL );
	double areaTotal = 0.0;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		areaTotal += currFace->getAreaNormal();
	}
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] Total surface area: " << areaTotal << " mm² (unit assumed).\n";
	LOG::info() << "[Mesh::" << __FUNCTION__ << "]                     " << getVertexNr()/areaTotal << " dots/mm² (unit assumed).\n";
	LOG::info() << "[Mesh::" << __FUNCTION__ << "]                     " << 25.4*sqrt(getVertexNr()/areaTotal) << " DPI (unit assumed).\n";
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] SERIAL: Face normal estimation time:        " << (int) ( time( NULL ) - timeStampSerial ) << " seconds.\n";
    #ifdef SHOW_MALLOC_STATS
	    SHOW_MALLOC_STATS( 9 );
    #endif

	timeStampSerial = time( NULL );
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		currFace->reconnectToFaces();
	}
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] SERIAL: Face neighbourhood estimation time: " << (int) ( time( NULL ) - timeStampSerial ) << " seconds.\n";
#endif

	int totalNeighbourCount = 0;
	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		totalNeighbourCount += curVertex->get1RingFaceCount();
	}
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] Vertex No. of Neighbours: " << totalNeighbourCount << " => " << totalNeighbourCount*sizeof(Face*)/(1024*1024) << " MBytes allocated.\n";

    #ifdef SHOW_MALLOC_STATS
	    SHOW_MALLOC_STATS( 10 );
    #endif

	// Empty seed and extended seed.
	MeshSeedExt::clear();

    #ifdef SHOW_MALLOC_STATS
	    SHOW_MALLOC_STATS( 11 );
    #endif

	//! Vertices, which are tagged with FLAG_SELECTED are added to the according set:
	deSelMVertsAll();
	for( auto const& currVertex: mVertices ) {
		if( currVertex->getFlag( FLAG_SELECTED ) ) {
			mSelectedMVerts.insert( currVertex );
		}
		//! Set local min/max flag
		currVertex->isFuncValLocalMinimum();
		currVertex->isFuncValLocalMaximum();
	}

	//! Set plane as a default to the center of the mesh
	Vector3D zAxis( 0.0, 0.0, 1.0, -getZ() );
	mPlane.setPlaneHNF( &zAxis );

	//! Show information regarding the mesh.
	dumpMeshInfo( true );

	// generate octree -- and remove in case one already exists

    delete mOctree;
	mOctree = nullptr;

	//timeStart = clock(); // for performance mesurement
	// TODO: provide better estimate for small meshes
	//generateOctree( 0.05*mVertices.size(), 0.05*mFaces.size() );
	//generateOctree( 0.05*mVertices.size(), 0 ); // Face octrees take quite some time and are buggy.
	//cout << "[Mesh::" << __FUNCTION__ << "] OCTREE done in " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
}

//! Generates Octree(s)
//! @param[in] vertexmaxnr maximum number of vertices per cube
//!            if vertexmaxnr==-1 no new octree vertex will be constructed
void Mesh::generateOctree(int vertexmaxnr) {

	//if bad estimate for a small mesh suggests to choose 0 elements per cube
	if(vertexmaxnr == 0) vertexmaxnr = 5;
    //if(facemaxnr == 0) facemaxnr = 7;

	//get boundingbox size for largest cube of octree
	double h=getEdgeLenMax();
	Vector3D center = getBoundingBoxCenter();
	Vector3D size;
	getBoundingBoxSize( size );
	double edgelen = max( size.getX(), max( size.getY(), size.getZ() ) );

	if (vertexmaxnr > 0) {
        if (vertexmaxnr > mVertices.size()){
            vertexmaxnr = round(mVertices.size()/4);
        }
        delete mOctree;
        mOctree = new Octree(mVertices, mFaces, &center, vertexmaxnr, edgelen, h, false);
		mOctree->dumpInfo();


	}
    /**
	if (facemaxnr > 0) {
        delete mOctreeface;
		mOctreeface = new Octree<Face*>(mFaces, &center, facemaxnr, edgelen, h);

		mOctreeface->dumpInfo();
	}
    **/
}


//! Returns the x-coordinate of the position vector of the Vertex
//! Return not-a-number in case of an error regarding to Primitive::getX()
double Mesh::getX() const {
	return ( mMaxX + mMinX ) / 2.0;
}

//! Returns the x-coordinate of the position vector of the Vertex
//! Return not-a-number in case of an error regarding to Primitive::getY()
double Mesh::getY() const {
	return ( mMaxY + mMinY ) / 2.0;
}

//! Returns the x-coordinate of the position vector of the Vertex
//! Return not-a-number in case of an error regarding to Primitive::getZ()
double Mesh::getZ() const {
    return ( mMaxZ + mMinZ ) / 2.0;
}
//! the following getter methods return the min and max coordinates of the mesh vertices
double Mesh::getMinX() const {
    return mMinX;
}

double Mesh::getMinY() const {
    return mMinY;
}

double Mesh::getMinZ() const {
    return mMinZ;
}

double Mesh::getMaxX() const {
    return mMaxX;
}

double Mesh::getMaxY() const {
    return mMaxY;
}

double Mesh::getMaxZ() const {
    return mMaxZ;
}


//! Write Mesh to file - overloaded from MeshIO.
bool Mesh::writeFile(
                const filesystem::path& rFileName
) {
	//! 1. Clear anr Re-Create arrays
	MeshSeedExt::clear();

	std::vector<sVertexProperties> vertexProps;
	vertexProps.resize( getVertexNr() );

	std::vector<sFaceProperties> faceProps;
	faceProps.resize( getFaceNr() );

	//! 2. Fill arrays - some will stay empty! \todo try to store as much data as possible.
	uint64_t vertCount = getVertexNr();
	uint64_t featVecLenMax = getFeatureVecLenMax( Primitive::IS_VERTEX );
	mFeatureVecVertices.clear();
	if( featVecLenMax > 0 ) {
		mFeatureVecVertices.resize(vertCount*featVecLenMax,_NOT_A_NUMBER_DBL_);
	}
	for( uint64_t vertIdx=0; vertIdx<vertCount; vertIdx++ ) {
		Vertex* curVertex = getVertexPos( vertIdx );
		curVertex->setIndex( vertIdx );
		curVertex->copyVertexPropsTo( vertexProps[vertIdx] );
	}

	if(!mFeatureVecVertices.empty())
	{
		for(size_t vertIdx = 0; vertIdx < vertCount; vertIdx++)
		{
			Vertex* curVertex = getVertexPos(vertIdx);
			curVertex->copyFeatureVecTo(&mFeatureVecVertices.at(vertIdx*featVecLenMax));
		}
	}

	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		currFace->copyFacePropsTo( faceProps[faceIdx] );
	}
	//! 3. Write arrays to file.
	bool retVal = MeshIO::writeFilePrimProps( rFileName, vertexProps, faceProps );
	//! 4. Remove arrays.
	MeshSeedExt::clear();
	return retVal;
}

//! Write connected components into one file per component.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::writeFilesForConnectedComponents() {
	// Check if vertices have been selected
	if( mSelectedMVerts.size() == 0 ) {
		showWarning( "No selection", "No selection of multiple vertices (SelMVerts) present!" );
		return( false );
	}

	bool userChoice;
	if( !showQuestion( &userChoice,
	                   "Apply labeling",
	                   "Do you want to compute the connected components (lables)?<br /><br />Recommended: YES" ) ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: User cancel!" << std::endl;
		return( false );
	}
	if( userChoice ) {
		if( !labelVerticesAll() ) {
			std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: labeling failed!" << std::endl;
			return( false );
		}
	}

	std::set<uint64_t> labelNrs;
	if( !getVertLabelIdFrom( mSelectedMVerts, labelNrs ) ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] ERROR: getVertLabelIdFrom failed!" << std::endl;
		return( false );
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] Amount of labels: " << labelNrs.size() << std::endl;

	// Inform the user if there are no connected components defined
	if( labelNrs.size() == 0 ) {
		showWarning( "Lables missing", "No connected components (labels) defined!" );
		return( false );
	}

	uint fileError = 0;
	uint fileOkay = 0;
	for( auto const& labelNr: labelNrs ) {
		std::set<Face*> facesWithLabel;
		bool retVal = getFaceHasVertLabelNo( labelNr, facesWithLabel );
		if( retVal ) {
			filesystem::path fileName = getFullName();

			// The following looks wired due to Windows build
			std::string oriExtension = fileName.extension().string();
			std::string suffixExtension = "comp.";
			suffixExtension += std::to_string( labelNr );
			suffixExtension += oriExtension;

			fileName.replace_extension( filesystem::path( suffixExtension ) );
			Mesh meshToWrite( &facesWithLabel );
			meshToWrite.getModelMetaDataRef().setModelMeta( getModelMetaDataRef() );
			if( meshToWrite.writeFile( fileName ) ) {
				std::cout << "[Mesh::" << __FUNCTION__ << "] Connected component written to file: " << fileName.string() << std::endl;
				fileOkay++;
			} else {
				std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Connected component NOT written to file " << fileName.string() << "!" << std::endl;
				fileError++;
			}
		}
	}

	// Notify the user about errors.
	if( fileError > 0 ) {
		showWarning( "File write error(s)", "Error writing files occured!" );
		return( false );
	}

	// Notify user about success.
	showInformation( "Files written", "Selected connected components written to separate files." );
	std::cout << "[Mesh::" << __FUNCTION__ << "] Done." << std::endl;
	return( true );
}

//! Import AND assign feature vectors - overloaded from MeshSeedExt.
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool Mesh::importFeatureVectorsFromFile(
    const filesystem::path& rFileName //!< Name of the file for import.
) {
	// Ask for vertex index within the first colum
	bool hasVertexIndex = true;
	if( !showQuestion( &hasVertexIndex, "First Column", "Does the first column contain the vertex index?<br /><br />"
	                   "Recommendation: YES for files computed with gigamesh-featurevectors" ) ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] User cancled." << std::endl;
		return( false );
	}

	// Fetch vectors into array.
	uint64_t maxFeatVecLen;
	vector<double> featureVecs;
	if( !MeshSeedExt::importFeatureVectors( rFileName, getVertexNr(), featureVecs,
	                                        maxFeatVecLen, hasVertexIndex ) ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Import failed!" << std::endl;
		return( false );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] mFeatureVecVerticesLen: " << maxFeatVecLen << endl;

	if( !assignFeatureVectors( featureVecs, maxFeatVecLen ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: during assignment of the feature vectors!" << endl;
		return( false );
	}

	return( true );
}

//! Exports feature vectors to a file
//! @returns false in case of an error or user cancel
bool Mesh::exportFeatureVectors(const filesystem::path& rFileName)
{
	// Ask for vertex index within the first colum
	bool hasVertexIndex = true;
	if( !showQuestion( &hasVertexIndex, "First Column", "Should the first column contain the vertex index?<br /><br />" ) ) {
		LOG::debug() << "[Mesh::" << __FUNCTION__ << "] User cancled.\n";
		return( false );
	}

	//! Exports feature vectors as ASCII file.
	std::ofstream filestr(rFileName);

	if(!filestr.is_open())
	{
		LOG::error() << "[Mesh::" << __FUNCTION__ << "] file " << rFileName << " unable to open for writing!\n";
		return false;
	}

	filestr.imbue(std::locale("C"));


	time_t rawtime;
	struct tm * timeinfo;

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	std::string timeInfoStr( asctime( timeinfo ) );
	timeInfoStr = timeInfoStr.substr( 0, timeInfoStr.length()-1 );

	std::stringstream strHeader;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | MAT file with feature vectors computed by the GigaMesh Software Framework     |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | WebSite: https://gigamesh.eu                                                  |" << std::endl;
	strHeader << "# | EMail:   info@gigamesh.eu                                                     |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Contact: Hubert MARA <hubert.mara@iwr.uni-heidelberg.de>                      |" << std::endl;
	strHeader << "# |          FCGL - Forensic Computational Geometry Laboratory                    |" << std::endl;
	strHeader << "# |          IWR - Heidelberg University, Germany                                 |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Mesh:       " << getBaseName() << std::endl;
	strHeader << "# | - Vertices: " << getVertexNr() << std::endl;
	strHeader << "# | - Faces:    " << getFaceNr() << std::endl;
	strHeader << "# | Timestamp:  " << timeInfoStr << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	filestr << strHeader.str();

	uint64_t currIndex = 0;
	for(const auto& currVert : mVertices)
	{
		auto vecSize = currVert->getFeatureVectorLen();
		if(hasVertexIndex)
		{
			filestr << (currIndex++) << " ";
		}

		for(unsigned int i = 0; i<vecSize; ++i)
		{
			double elem;
			currVert->getFeatureElement(i, &elem);
			filestr << elem;
			filestr << (i == vecSize - 1 ? "\n" : " ");
		}
	}

	filestr.close();

	return true;
}

//! Assigns the feature vectors within Mesh::mFeatureVecVertices
//! to the vertices.
//!
//! Not to be confused with the method having a similar name defined in the Primitive class.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::assignFeatureVectors(
        const vector<double>&   rFeatureVecs,
        const uint64_t&    rMaxFeatVecLen
) {
	// Assign array to vertices, when the file was successfully read.
    if( rFeatureVecs.empty() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: No feature vectors found!" << endl;
		return( false );
	}
	const double* featureVecVerticesPtr = rFeatureVecs.data();
	if( featureVecVerticesPtr == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: No pointer to feature vectors!" << endl;
		return( false );
	}
	Vertex* curVertex;
	bool assignOk = true;
	uint64_t vertexCount = getVertexNr();
	for( uint64_t vertIdx=0; vertIdx<vertexCount; vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		//cout << featureVecVertices[featureVecVerticesLen*vertIdx] << endl;
		const double* nextFeatureVecPtr = &(featureVecVerticesPtr[rMaxFeatVecLen*vertIdx]);
		if( !curVertex->assignFeatureVec( nextFeatureVecPtr, rMaxFeatVecLen ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Could not assign feature vector!" << endl;
			assignOk = false;
		}
	}
	return( assignOk );
}

//! Fetchs the normal into a given array of double values, which has to be of size 3.
//! @returns false in case of an error.
bool Mesh::getVertNormal( int rVertIdx, double* rNormal ) {
	Vertex* currVert = getVertexPos( rVertIdx );
	if( currVert == nullptr ) {
		return false;
	}
	bool retVal = currVert->copyNormalXYZTo( rNormal, false );
	return retVal;
}

// --- Vertex Navigation -------------------------------------------------------------

//! Returns the actual number of vertices, excluding vertices marked for removal, including new vertices.
//! See also Mesh::getVertexPos
uint64_t Mesh::getVertexNr() const {
	return mVertices.size();
}

//! Retrieves the n-th Vertex, excluding vertices marked for removal, including new vertices.
//! @returns the nullptr in case of an error.
Vertex* Mesh::getVertexPos( uint64_t rPosIdx ) const {
	if( rPosIdx >= getVertexNr() ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] Index too large!" << std::endl;
		return( nullptr );
	}
	return( mVertices.at( rPosIdx ) );
}

//! Order Vertices by Index.
bool Mesh::orderVertsByIndex() {
	int timeStart = clock();
	sort( mVertices.begin(), mVertices.end(), Vertex::sortByIndex );
	cout << "[Mesh::" << __FUNCTION__ << "] time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return true;
}

//! Order Vertices ascending by Function Value.
bool Mesh::orderVertsByFuncVal() {
	int timeStart = clock();
	sort( mVertices.begin(), mVertices.end(), Vertex::funcValLower );
	cout << "[Mesh::" << __FUNCTION__ << "] time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return true;
}

//! Order Vertices ascending by Function Value.
bool Mesh::orderVertsByRGB() {
    int timeStart = clock();
    sort( mVertices.begin(), mVertices.end(), Vertex::RGBLower );
    cout << "[Mesh::" << __FUNCTION__ << "] time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
    return true;
}

//! Set a given flag for all vertices.
//! @returns false in case an error occured.
bool Mesh::setVertexFlagForAll( ePrimitiveFlags rFlag ) {
	bool retVal = true;
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		if( !((*itVertex)->setFlag( rFlag )) ) {
			retVal = false;
		}
	}
	return retVal;
}

//! Clears a given flag for all vertices.
//! @returns false in case an error occured.
bool Mesh::clearVertexFlagForAll( ePrimitiveFlags rFlag ) {
	bool retVal = true;
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		if( !((*itVertex)->clearFlag( rFlag )) ) {
			retVal = false;
		}
	}
	return retVal;
}

//! Sets the vertices indicies to their position.
bool Mesh::setVertexPosToIndex() {
	orderVertsByIndex();
	int i = 0;
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		(*itVertex)->setIndex( i );
		i++;
	}
	return true;
}

bool Mesh::getVertexList( set<Vertex*>* rVertices ) {
	//! Adds all vertices to a std::set.
	int timeStart = clock();
	//! Returns false in case of an error.
	if( rVertices == nullptr ) {
		return false;
	}
	//! *) Clears the given vertexVector.
	rVertices->clear();

	std::copy(mVertices.begin(), mVertices.end(), std::inserter(*rVertices, rVertices->end()));
	/*
	//! *) Prepare list of vertices.
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		rVertices->insert( (*itVertex) );
	}
	*/
	cout << "[Mesh::" << __FUNCTION__ << "] make vertex set time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return true;
}

//! Adds all vertices to a std::vector.
//! @returns false in case of an error.
bool Mesh::getVertexList( vector<Vertex*>* rVertices ) {
	int timeStart = clock();
	if( rVertices == nullptr ) {
		return false;
	}

	*rVertices = mVertices;
	/*
	//! *) Clears the given vertexVector.
	rVertices->clear();
	//! *) Prepare list of vertices.
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		rVertices->push_back( (*itVertex) );
	}
	*/
	cout << "[Mesh::" << __FUNCTION__ << "] make vertex vector time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return true;
}

//! Adds all vertices to a std::vector having a FINITE function value!
//! i.e. vertices with NAN, +INF and -INF are NOT added!
//! @returns false in case of an error.
bool Mesh::getVertexListValidFuncVal( vector<Vertex*>* rVertices ) {
	//! Adds all vertices to a std::vector.
	int timeStart = clock();
	//! Returns false in case of an error.
	if( rVertices == nullptr ) {
		return false;
	}
	//! *) Clears the given vertexVector.
	rVertices->clear();
	//! *) Prepare list of vertices.
	unsigned int noFiniteValues = 0;
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		double currVal = _NOT_A_NUMBER_DBL_;
		(*itVertex)->getFuncValue( &currVal );
		if( isfinite( currVal ) ) {
			rVertices->push_back( (*itVertex) );
		} else {
			noFiniteValues++;
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Vertices with finite values:   " << rVertices->size() << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] Vertices with infinite values: " << noFiniteValues << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] make vertex vector time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return true;
}

//! Fetch point to primitve vectors for generic operations.
//! e.g. Building a VBO with indices of selected primitives.
const vector<Vertex*>* Mesh::getPrimitiveListVertices() {
	return &mVertices;
}

//! Fetch point to primitve vectors for generic operations.
//! e.g. Building a VBO with indices of selected primitives.
const vector<Face*>* Mesh::getPrimitiveListFaces() {
	return &mFaces;
}

// --- Face Navigation -------------------------------------------------------------

//! Determine the actual number of faces, excluding faces marked for removal, including new faces.
//!
//! See also Mesh::getFacePos
//!
//! @returns the actual number of faces.
uint64_t Mesh::getFaceNr() const {
	return mFaces.size();
}

//! Retrieves the n-th Face, excluding faces marked for removal, including new faces.
//!
//! @returns NULL in case of an error.
Face* Mesh::getFacePos( uint64_t posIdx ) const {
	if( posIdx >= getFaceNr() ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] Index (" << posIdx << ") too large - Max: " << getFaceNr() << "!" << std::endl;
		return nullptr;
	}
	return mFaces.at( posIdx );
}

bool Mesh::getFaceList( set<Face*>* rFaces ) {
	//! Adds all faces to a std::set.
	int timeStart = clock();
	//! Returns false in case of an error.
	if( rFaces == nullptr ) {
		return false;
	}
	//! *) Clears the given vertexVector.
	rFaces->clear();
	//! *) Prepare list of vertices.
	vector<Face*>::iterator itFace;
	for( itFace=mFaces.begin(); itFace!=mFaces.end(); itFace++ ) {
		rFaces->insert( (*itFace) );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] make vertex set time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return true;
}

bool Mesh::getFaceList( vector<Face*>* rFaces ) {
	//! Adds all vertices to a std::vector.
	int timeStart = clock();
	//! Returns false in case of an error.
	if( rFaces == nullptr ) {
		return false;
	}

	*rFaces = mFaces;
	/*
	//! *) Clears the given vertexVector.
	rFaces->clear();
	rFaces->reserve(mFaces.size());

	//! *) Prepare list of vertices.
	vector<Face*>::iterator itFace;
	for( itFace=mFaces.begin(); itFace!=mFaces.end(); itFace++ ) {
		rFaces->push_back( (*itFace) );
	}
	*/
	cout << "[Mesh::" << __FUNCTION__ << "] make vertex vector time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return true;
}

// --- Face Navigation -------------------------------------------------------------

//! Returns the actual number of PolyLine objects.
//! See also Mesh::getPolyLinePos
unsigned int Mesh::getPolyLineNr() {
	return mPolyLines.size();
}

//! Returns the actual number of elements of a PolyLine object.
unsigned int Mesh::getPolyLineLength( unsigned int rPolyIdx ) {
	PolyLine* somePolyLine = mPolyLines.at( rPolyIdx );
	return somePolyLine->length();
}

//! Returns the current index of the vertex of an element of a Polyline object.
int Mesh::getPolyLineVertIdx( unsigned int rPolyIdx, unsigned int rElementIdx ) {
	PolyLine* somePolyLine = mPolyLines.at( rPolyIdx );
	Vertex* someVert = somePolyLine->getVertexRef( rElementIdx );
	return someVert->getIndex();
}

//! Returns the label ID of a polyine.
uint64_t Mesh::getPolyLineLabel( unsigned int rPolyIdx ) {
	PolyLine* somePolyLine = mPolyLines.at( rPolyIdx );
	uint64_t labelID;
	somePolyLine->getLabel( labelID );
	return labelID;
}

//! Returns the primitive information of a polyline, like position vector and normal.
PrimitiveInfo Mesh::getPolyLinePrimInfo( unsigned int rPolyIdx ) {
	PrimitiveInfo primInfo;
	PolyLine* somePolyLine = mPolyLines.at( rPolyIdx );
	primInfo.mPosX = somePolyLine->getX();
	primInfo.mPosY = somePolyLine->getY();
	primInfo.mPosZ = somePolyLine->getZ();
	primInfo.mNormalX = somePolyLine->getNormalX();
	primInfo.mNormalY = somePolyLine->getNormalY();
	primInfo.mNormalZ = somePolyLine->getNormalZ();
	return primInfo;
}

//! Retrieves the n-th Face, excluding faces marked for removal, including new faces.
//! @returns NULL in case of an error.
PolyLine* Mesh::getPolyLinePos( unsigned int rPosIdx ) {
	if( rPosIdx >= getPolyLineNr() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Index too large!" << endl;
		return nullptr;
	}
	if( mPolyLines.at( rPosIdx ) == nullptr ) {
		// This actually should never happen:
		cerr << "[Mesh::" << __FUNCTION__ << "] unexpected NULL pointer at " << rPosIdx << "!" << endl;
		return nullptr;
	}
	return mPolyLines.at( rPosIdx );
}

//! Retrieve the bounding box of all polylines
//!
//! Optional: project the vertices to the plane.
//!           This option will only work if a valid plane is present.
//!           If rProjectToPlane is true only polyines with planes are considered.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getPolyLineBoundingBoxFromAll(
                double* rMinX, double* rMinY, double* rMinZ, \
                double* rMaxX, double* rMaxY, double* rMaxZ, \
                bool rProjectToPlane ) {
	//! \todo add checks.

	// Set inital values:
	*rMinX = +_INFINITE_DBL_;
	*rMinY = +_INFINITE_DBL_;
	*rMinZ = +_INFINITE_DBL_;
	*rMaxX = -_INFINITE_DBL_;
	*rMaxY = -_INFINITE_DBL_;
	*rMaxZ = -_INFINITE_DBL_;

	bool valueSet = false; // To check if there are any qualified polyines.
	vector<PolyLine*>::iterator itPolyLinePtr;
	for( itPolyLinePtr=mPolyLines.begin(); itPolyLinePtr!=mPolyLines.end(); itPolyLinePtr++ ) {
		double Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
		if( (*itPolyLinePtr)->getBoundingBox( &Xmin, &Ymin, &Zmin, &Xmax, &Ymax, &Zmax, rProjectToPlane, true ) ) {
			valueSet =  true;
			// Minimum values
			*rMinX = min( *rMinX, Xmin );
			*rMinY = min( *rMinY, Ymin );
			*rMinZ = min( *rMinZ, Zmin );
			// Maximum values
			*rMaxX = max( *rMaxX, Xmax );
			*rMaxY = max( *rMaxY, Ymax );
			*rMaxZ = max( *rMaxZ, Zmax );
			// cout << "[Mesh::" << __FUNCTION__ << "] Bounding Box (X): " << Xmin << " - " << Xmax << endl;
			// cout << "[Mesh::" << __FUNCTION__ << "] Bounding Box (Y): " << Ymin << " - " << Ymax << endl;
			// cout << "[Mesh::" << __FUNCTION__ << "] Bounding Box (Z): " << Zmin << " - " << Zmax << endl;
		}
	}
	// No qualified polylines.
	if( !valueSet ) {
		return( false );
	}
	// Done.
	return( true );
}

// data retrival ---------------------------------------------------------------------

Vertex* Mesh::getVertexByIdxOriginal( int findIdx ) {
	//! Overloads Primitive::getVertexByIdxOriginal. Uses a look-up table
	//! for the Vertex classes - which is much faster.
	//!
	//! ATTENTION: The verticesInOriginalOrder has to be properly maintained -
	//! especially when a Vertex is deleted.
	// In case a too large texture map or feature vector array is attached:
	if( ( findIdx < 0 ) || ( findIdx >= static_cast<int>(mVertices.size()) ) ) {
		return nullptr;
	}
	//cerr << "[Mesh::" << __FUNCTION__ << "] DANGEROUS: DEPRECATED FUNCTION!" << endl;
	//! \bug will return wrong results, when vertices have been deleted.
	return mVertices[findIdx];
}

//! Finds a Vertex with a certain index, which might be the same as the original index.
//! This method can slow down the programm, when called often.
Vertex* Mesh::getVertexByIdx( int findIdx ) {
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		if( (*itVertex)->getIndex() == findIdx ) {
			return (*itVertex);
		}
	}
	//cout << "[Mesh::" << __FUNCTION__ << "] " << findIdx << endl;
	return nullptr;
}

//! Returns the reference to a vertex next to given position.
bool Mesh::getVertexNextTo( Vector3D rVertPos,    //!< Position vector in world coordinates.
                            Vertex** rVertexNext  //!< Pointer to be returned of the vertex next to rVertPos.
                          ) {
	double coord[3];
	rVertPos.get3( coord );

	float vertexDistMin = _INFINITE_DBL_;
	for( auto const& currVertex: mVertices ) {
		double vertexDist = currVertex->distanceToCoord( coord );
		if( vertexDist < vertexDistMin ) {
			vertexDistMin = vertexDist;
			(*rVertexNext) = currVertex;
		}
	}
	return true;
}

//! Returns all Vertices along the line defined by the position of Vertex A and B, closer than the beamPerimeterRadius.
//! Returns false, when no vertices were added or an error occured (e.g. NULL pointer given).
//! See also Vertex::distanceToLine
bool Mesh::getVerticesInBeam( Vector3D rVertAPos, Vector3D rVertBPos, float rBeamPerimeterRadius, set<Vertex*>* rVertsInBeam ) {

	if( rVertsInBeam == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	//rVertsInBeam->clear();

	bool    verticesInBeam = false;
	float   vertexDist;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		vertexDist = currVertex->distanceToLine( &rVertAPos, &rVertBPos );
		if( vertexDist < rBeamPerimeterRadius ) {
			rVertsInBeam->insert( currVertex );
			verticesInBeam = true;
		}
	}
	return verticesInBeam;
}

bool Mesh::getFacesInBeam( Vector3D rVertAPos, Vector3D rVertBPos, float rBeamPerimeterRadius, set<Face*>* rFacesInBeam ) {
	//! Returns all Faces along the line defined by the position of Vertex A and B, closer than the beamPerimeterRadius.
	//! Returns false, when no face were added or an error occured (e.g. NULL pointer given).
	//! See also Mesh::getVerticesInBeam and Vertex::distanceToLine

	if( rFacesInBeam == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	//rFacesInBeam->clear();

	set<Vertex*> verticesInBeam;
	if( !getVerticesInBeam( rVertAPos, rVertBPos, rBeamPerimeterRadius, &verticesInBeam ) ) {
		// No vertices add - no chance to add a face
		return false;
	}

	unsigned int facesInBeamCount = rFacesInBeam->size();
	//cout << "[Mesh::" << __FUNCTION__ << "] verticesInBeam.size: " << verticesInBeam.size() << endl;
	set<Vertex*>::iterator itVertex;
	for ( itVertex=verticesInBeam.begin(); itVertex != verticesInBeam.end(); itVertex++ ) {
		(*itVertex)->getFaces( rFacesInBeam );
		//cout << "[Mesh::" << __FUNCTION__ << "] facesInBeam.size: " << facesInBeam.size() << endl;
	}
	return ( facesInBeamCount != rFacesInBeam->size() );
}

//! Compute the minimum edge length of all faces.
//!
//! Remark 1: Calling this method often will result in low performance.
//! Remark 2: Edge lengths of polyines are ignored.
//!
//! @returns minimum edge length of all triangles within the mesh.
double Mesh::getEdgeLenMin() {
	double edgeLenMin = _INFINITE_DBL_;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		double edgeLen = _INFINITE_DBL_;
		currFace->getEdgeLenMin( &edgeLen );
		if( edgeLen < edgeLenMin ) {
			edgeLenMin = edgeLen;
		}
	}
	return edgeLenMin;
}

//! Compute the maximum edge length of all faces.
//!
//! Remark 1: Calling this method often will result in low performance.
//! Remark 2: Edge lengths of polyines are ignored.
//!
//! @returns maximum edge length of all triangles within the mesh.
double Mesh::getEdgeLenMax() {
	double edgeLenMax = 0.0;
	double edgeLen;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		edgeLen = currFace->getEdgeLenMax();
		if( edgeLen > edgeLenMax ) {
			edgeLenMax = edgeLen;
		}
	}
	return edgeLenMax;
}

//! Compute the minimum edge length of all faces.
//!
//! Remark 1: Calling this method often will result in low performance.
//! Remark 2: Edge lengths of polyines are ignored.
//!
//! @returns minimum edge length of all triangles within the mesh.
double Mesh::getAltitudeMin() {
	double altitudeMin = _INFINITE_DBL_;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		double altLen = _INFINITE_DBL_;
		currFace->getAltitudeMin( &altLen );
		if( altLen < altitudeMin ) {
			altitudeMin = altLen;
		}
	}
	return altitudeMin;
}

int Mesh::getType() {
	//! Return the type (=class) of this object as an id.
	return Primitive::IS_MESH;
}

//! Returns a concatenated string with the indices of the selected vertices.
//!
//! @returns false, when no vertex was selected or an error occured.
bool Mesh::getVerticesSelectedIndicesStr(
                std::string& rVertIdx
) {
	if( mSelectedMVerts.empty()) {
		return( false );
	}

	set<Vertex*>::iterator itVertex;
	for( itVertex=mSelectedMVerts.begin(); itVertex!=mSelectedMVerts.end(); itVertex++ ) {
		rVertIdx += (*itVertex)->getIndexStr() + " ";
	}

	return( true );
}

// De-Selection ------------------------------------------------------------------------------------------------------------------------------------------------

//! Clears mSelectedMVerts -- SelMVerts.
//! @returns true, when vertices were deselected.
bool Mesh::deSelMVertsAll() {
	if( mSelectedMVerts.empty() ) {
		return( false );
	}
	mSelectedMVerts.clear();
	selectedMVertsChanged();
	return true;
}

//! Removes vertices without a label from mSelectedMVerts -- SelMVerts.
//! @returns true, when vertices were deselected.
bool Mesh::deSelMVertsNoLabel() {
    if( mSelectedMVerts.empty() ) {
		return false;
	}
	set<Vertex*> mVertsSelNew;
	set<Vertex*>::iterator itVert;
	for( itVert=mSelectedMVerts.begin(); itVert!=mSelectedMVerts.end(); itVert++ ) {
		Vertex* currVertex = (*itVert);
		if( !currVertex->isLabled() ) {
			continue;
		}
		mVertsSelNew.insert( currVertex );
	}
	if( mVertsSelNew.size() == mLabelSeedVerts.size() ) {
		return false;
	}
	mSelectedMVerts.swap( mVertsSelNew );
	selectedMVertsChanged();
	return true;
}

// Selection ---------------------------------------------------------------------------------------------------------------------------------------------------

//! Stub to be overwritten by UI for user notification about changes to the selected vertices.
//! @returns the number of selected vertices.
unsigned int Mesh::selectedMVertsChanged() {
	cout << "[Mesh::" << __FUNCTION__ << "] SelMVerts has changed. |SelMVerts| is now " <<  mSelectedMVerts.size() << endl;
	int timeStart = clock();
	for( auto const& currVertex: mVertices ) {
		currVertex->clearFlag( FLAG_SELECTED );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Clear vertex flags: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	timeStart = clock();
	for( auto const& selVertex: mSelectedMVerts ) {
		selVertex->setFlag( FLAG_SELECTED );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Set vertex flags: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return mSelectedMVerts.size();
}

//! Stub to be overwritten by UI for user notification about changes to the selected faces.
//! @returns the number of selected faces.
unsigned int Mesh::selectedMFacesChanged() {
	cout << "[Mesh::" << __FUNCTION__ << "] SelMFaces has changed. |SelMFaces| is now " <<  mFacesSelected.size() << endl;
	int timeStart = clock();
	for( auto const& currFace: mFaces ) {
		currFace->clearFlag( FLAG_SELECTED );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Clear vertex flags: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	timeStart = clock();
	for( auto const& selFace: mFacesSelected ) {
		selFace->setFlag( FLAG_SELECTED );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Set vertex flags: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return mFacesSelected.size();
}

//! Stub to be overwritten by UI for user notification about changes to the selected polylines.
//! @returns the number of selected polylines.
unsigned int Mesh::selectedMPolysChanged() {
	cout << "[Mesh::" << __FUNCTION__ << "] SelMPolylines has changed. |SelMPolylines| is now " <<  mPolyLinesSelected.size() << endl;
	int timeStart = clock();
	for( auto const& currVertex: mPolyLines ) {
		currVertex->clearFlag( FLAG_SELECTED );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Clear vertex flags: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	timeStart = clock();
	for( auto const& selVertex: mPolyLinesSelected ) {
		selVertex->setFlag( FLAG_SELECTED );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Set vertex flags: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return mPolyLinesSelected.size();
}

//! Stub to be overwritten by UI for user notification about changes to the selected positions.
unsigned int Mesh::selectedMPositionsChanged() {
	cout << "[Mesh::" << __FUNCTION__ << "] SelMPositions has changed. |SelMPositions| is now " <<  mSelectedPositions.size() << endl;
	return mSelectedPositions.size();
}

//! Show the indices of the selected vertices.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectVertsByIdxShow() {
	std::string vertIndicesStr;
	if( !getVerticesSelectedIndicesStr( vertIndicesStr ) ) {
		showInformation( "Indices of the currently selected vertices", "None." );
		return( false );
	}

	showInformation( "Indices of the currently selected vertices: ",
	                 vertIndicesStr, vertIndicesStr );
	return( true );
}

//! Ask the user for indices to select vertices.
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool Mesh::selectVertsByIdxUser() {
	vector<long> vertexIndices;
	if( !showEnterText( vertexIndices, "Enter vertex indices for selection" ) ) {
		return( false );
	}
	selectVertsByIdx( vertexIndices );
	return( true );
}

//! Ask the user for indices to select vertices.
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool Mesh::selectVertsRandomUser() {
	double vertexAmount;
	if( !showEnterText( vertexAmount, "Enter amount of vertices for selection in percent ]0.0...100.0[" ) ) {
		return( false );
	}
	if( ( vertexAmount <= 0.0 ) || ( vertexAmount >= 100.0 ) ) {
		showWarning( "Value out of range", "The value has to be larger than zero and lower than 100.0.");
		return( false );
	}
	bool useRingArea = false;
	if( !showQuestion( &useRingArea, "Weight with local neighborhood",
	                   "Use the area of the 1-ring neighborhood as weights "
	                   "for the random selection?<br /><br />Recommended: YES for meshes") ) {
		return( false ); // User cancel
	}
	bool retVal = selectVertsRandom( vertexAmount, useRingArea );
	return( retVal );
}


//! Adds vertices by current index to the selection given as std::vector.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectVertsByIdx(
                const std::vector<long>& rVertIndices
) {
	// Order first:
	orderVertsByIndex();
	// Mark matching indicies
	for( auto const& itIndex: rVertIndices ) {
		Vertex* currVert = getVertexPos( itIndex );
		if( currVert == nullptr ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Vertex with Index " << itIndex << " not found!" << endl;
			continue;
		}
		// cout << "[Mesh::" << __FUNCTION__ << "] adding " << itIndex << endl;
		// SLOW! -> addToSelection( currVert );
		mSelectedMVerts.insert( currVert );
	}
	selectedMVertsChanged();
	return( true );
}

//! Adds vertices by current index to the selection given as std::set.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectVertsByIdx(
				const std::set<uint64_t>& rVertIndices
) {
	// Order first:
	orderVertsByIndex();
	// Mark matching indicies
	for( auto const& itIndex: rVertIndices ) {
		Vertex* currVert = getVertexPos( itIndex );
		if( currVert == nullptr ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Vertex with Index " << itIndex << " not found!" << endl;
			continue;
		}
		// cout << "[Mesh::" << __FUNCTION__ << "] adding " << itIndex << endl;
		// SLOW! -> addToSelection( currVert );
		mSelectedMVerts.insert( currVert );
	}
	selectedMVertsChanged();
	return( true );
}

//! Adds ramdom vertices by given amount in percent.
//!
//! Note: As the indices are used the distribution won't
//! be equally distributed along the manifold.
//!
//! \todo take 1-ring into account for an equal distribution.
//!
//! @param rPercent expected to be in ]0.0...100.0[
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectVertsRandom(
                const double rPercent,
                const bool rAreaWeight
) {
	if( rPercent <= 0.0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Value equal or below zero given!" << std::endl;
		return( false );
	}
	if( rPercent >= 100.0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Value equal or above 100.0 given!" << std::endl;
		return( false );
	}
	uint64_t vertexCount = getVertexNr();
	uint64_t verticesToSelect = vertexCount * (rPercent/100.0);

	std::default_random_engine generator;
	std::set<uint64_t> verticesRandomlySelected;
	if( rAreaWeight ) {
		showProgressStart( "Random vertex selection, weighted" );
		// Prepare areas as weights
		std::vector<double> accumulatedArea;
		accumulatedArea.resize( vertexCount );
		double totalArea = 0.0;
		for( uint64_t i=0; i<vertexCount; i++ ) {
			Vertex* currVert = getVertexPos( i );
			totalArea += currVert->get1RingArea();
			accumulatedArea.at( i ) = totalArea;
			showProgress( 0.5*(i/vertexCount), "Random vertex selection, weighted" );
		}
		std::cout << "[Mesh::" << __FUNCTION__ << "] accumulatedArea: " << totalArea << std::endl;
		// Select randomly using the area
		std::uniform_real_distribution<double> distribution( 0.0, totalArea );
		while( verticesRandomlySelected.size() <= verticesToSelect ) {
			double newRandomAreaPos = distribution( generator );
			std::vector<double>::iterator it = std::find_if(
			      accumulatedArea.begin(),
			      accumulatedArea.end(),
			      [&]( const double & areaAccu ) {
				return( areaAccu >= newRandomAreaPos );
			});
			uint64_t newRandomIndex = static_cast<uint64_t>( std::distance( accumulatedArea.begin(), it ) );
			verticesRandomlySelected.insert( newRandomIndex-1 );
			showProgress( 0.5+0.5*(verticesToSelect/verticesRandomlySelected.size()), "Random vertex selection, weighted" );
		}
		showProgressStop( "Random vertex selection, weighted" );
	} else { // Simple and fast. Best for point clouds or very regular grids
		showProgressStart( "Random vertex selection" );
		std::uniform_int_distribution<uint64_t> distribution( 0, vertexCount ); // ULONG_MAX
		while( verticesRandomlySelected.size() < verticesToSelect ) {
			uint64_t newRandomIndex = distribution( generator );
			verticesRandomlySelected.insert( newRandomIndex );
		}
		showProgressStop( "Random vertex selection" );
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] Indices choosen: " << verticesRandomlySelected.size() << std::endl;

	bool retVal = selectVertsByIdx( verticesRandomlySelected );
	return( retVal );
}

//--- Vertex and Face selection --------------------------------------------------------------------------------------------------------------------------------

//! Adds a given pointer to a Vertex to mSelectedMVerts.
//!
//! IMPORTANT: This function is slow, when often called i.e. by inserting vertices from a list|vector.
//! Better use another addToSelection for vectors/sets of vertices.
//!
//! @returns false in case of an error, e.g. NULL pointer given.
bool Mesh::addToSelection( Vertex* const rVertToAdd ) {
	if( rVertToAdd == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	mSelectedMVerts.insert( rVertToAdd );
	selectedMVertsChanged();
	return true;
}

//! Adds a given Pointer to a Face to mFacesSelected
//!
//! IMPORTANT: This function is slow, when often called i.e. by inserting vertices from a list|vector.
//! Better use another addToSelection for vectors/sets of vertices.
//!
//! @returns false in case of an error, e.g. NULL pointer given.
bool Mesh::addToSelection( Face* const rFaceToAdd ) {
	if( rFaceToAdd == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	mFacesSelected.insert( rFaceToAdd );
	selectedMFacesChanged();
	return true;
}

//! Adds a given vector of Vertices to mSelectedMVerts.
//! Returns false in case of an error, e.g. NULL pointer given.
bool Mesh::addToSelection( vector<Vertex*>* const rVertsToAdd ) {
	if( rVertsToAdd == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	if( rVertsToAdd->empty() ) {
		cout << "[Mesh::" << __FUNCTION__ << "] No vertices added." << endl;
		return true;
	}
	vector<Vertex*>::iterator itVertex;
	for( itVertex=rVertsToAdd->begin(); itVertex!=rVertsToAdd->end(); itVertex++ ) {
		mSelectedMVerts.insert( (*itVertex) );
	}
	selectedMVertsChanged();
	return true;
}

//! Adds a given set of Vertices to mSelectedMVerts.
//! @returns false in case of an error. True otherwise.
bool Mesh::addToSelection( const set<Vertex*>& rVertsToAdd ) {
	mSelectedMVerts.insert( rVertsToAdd.begin(), rVertsToAdd.end() );
	selectedMVertsChanged();
	return( true );
}

//! Adds a given set of Vertices to mFacesSelected.
//! @returns false in case of an error. True otherwise.
bool Mesh::addToSelection( const set<Face*>& rFaceToAdd ) {
	mFacesSelected.insert( rFaceToAdd.begin(), rFaceToAdd.end() );
	selectedMFacesChanged();
	return( true );
}

//--- Vertex deselection ---------------------------------------------------------------------------------------------------------------------------------------
//! Removes a given set of Vertices from mSelectedMVerts.
//! @returns false in case of an error. True otherwise.
bool Mesh::removeFromSelection( const set<Vertex*>& rVertsToRemove ) {
    //delete all selected vertices which are deselected

    for(  set<Vertex*>::iterator itVertex = rVertsToRemove.begin(); itVertex != rVertsToRemove.end(); itVertex++ ) {
        //! Find each selected vertex which is not in part of the deselection vector
        std::set<Vertex*>::iterator selectedVert = mSelectedMVerts.find(*itVertex);
        if( selectedVert != mSelectedMVerts.end()) {
            mSelectedMVerts.erase( (*selectedVert) );
        }

    }
    selectedMVertsChanged();
    return( true );
}

//! Removes a given set of Vertices from mSelectedMVerts.
//! @returns false in case of an error. True otherwise.
bool Mesh::removeFromSelection( std::vector<Vertex*>* const rVertsToRemove ) {
    if( rVertsToRemove == nullptr ) {
        cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
        return false;
    }
    if( rVertsToRemove->empty() ) {
        cout << "[Mesh::" << __FUNCTION__ << "] No vertices deselected." << endl;
        return true;
    }



    //std::set_difference(mSelectedMVerts.begin(), mSelectedMVerts.end(), rVertsToRemove->begin(), rVertsToRemove->end(),
//                          std::inserter(difference, difference.begin()));

    vector<Vertex*>::iterator itVertex;
    for( itVertex=rVertsToRemove->begin(); itVertex!=rVertsToRemove->end(); itVertex++ ) {
        //! Find each selected vertex which is not in part of the deselection vector
        std::set<Vertex*>::iterator selectedVert = mSelectedMVerts.find(*itVertex);
        if( selectedVert != mSelectedMVerts.end()) {
            mSelectedMVerts.erase( (*selectedVert) );
        }

    }
    selectedMVertsChanged();
    return true;
}

//--- Vertex selection -----------------------------------------------------------------------------------------------------------------------------------------

int Mesh::selectVertFuncValLowerThan( double rVal ) {
	//! Select all vertices with a function value lower than someVal.
	//! Return the number of selected vertices.
	//! Returns a negative value in case of an error.
	Vertex* curVertex;
	double  funcValCurr;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( !curVertex->getFuncValue( &funcValCurr ) ) {
			cout << "[Mesh::" << __FUNCTION__ << "] ERROR: getFuncValue!" << endl;
			continue;
		}
		if( funcValCurr < rVal ) {
			mSelectedMVerts.insert( curVertex );
		}
	}
	selectedMVertsChanged();
	return mSelectedMVerts.size();
}

int Mesh::selectVertFuncValGreatThan( double rVal ) {
	//! Select all vertices with a function value lower than someVal.
	//! Return the number of selected vertices.
	//! Returns a negative value in case of an error.
	Vertex* curVertex;
	double  funcValCurr;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( !curVertex->getFuncValue( &funcValCurr ) ) {
			cout << "[Mesh::" << __FUNCTION__ << "] ERROR: getFuncValue!" << endl;
			continue;
		}
		if( funcValCurr > rVal ) {
			mSelectedMVerts.insert( curVertex );
		}
	}
	selectedMVertsChanged();
	return mSelectedMVerts.size();
}

bool Mesh::selectVertLocalMin() {
	//! Selects all vertices tagged as local minimum.
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( currVertex->getFlag( FLAG_LOCAL_MIN ) ) {
			mSelectedMVerts.insert( currVertex );
		}
	}
	selectedMVertsChanged();
	return true;
}

//! Selects all vertices tagged as local minimum.
bool Mesh::selectVertLocalMax() {
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( currVertex->getFlag( FLAG_LOCAL_MAX ) ) {
			mSelectedMVerts.insert( currVertex );
		}
	}
	selectedMVertsChanged();
	return true;
}

//! Add solo vertices to the selection - see getVertSolo().
//! @returns true, when vertices were added.
bool Mesh::selectVertSolo() {
	bool retVal = getVertSolo( &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Add all vertices from non-manifold faces to the selection - see getVertNonManifoldFaces().
//! @returns true, when vertices were added.
bool Mesh::selectVertNonManifoldFaces() {
	bool retVal = getVertNonManifoldFaces( &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Add all double-cone vertices to the selection - see getDoubleCones().
//! @returns true, when vertices were added.
bool Mesh::selectVertDoubleCone() {
	bool retVal = getDoubleCones( &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Selects all vertices of labels having an area smaller than rAreaMax.
//! Uses vertex based labels - see getVertLabelAreaLT().
//!
//! Remark: this will not select single vertices, even they got their own label no!
//! @returns true, when vertices were added.
bool Mesh::selectVertLabelAreaLT( double rAreaMax ) {
	bool retVal = getVertLabelAreaLT( rAreaMax, &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Selects all vertices of labels having an area smaller than rAreaMax.
//! Uses vertex based labels - see getVertLabelAreaRelativeLT().
//!
//! Remark: this will not select single vertices, even they got their own label no!
//! @returns true, when vertices were added.
bool Mesh::selectVertLabelAreaRelativeLT( double rPercent ) {
	bool retVal = getVertLabelAreaRelativeLT( rPercent, &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Add all border vertices to the selection - see getVertBorder().
//! @returns true, when vertices were added.
bool Mesh::selectVertBorder() {
	bool retVal = getVertBorder( &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Adds all vertices from all faces with a minimum angle lower than rMinAngle to the selection - see getVertFaceMinAngleLT().
//! rMaxAngle has to be in radiant.
//! @returns true, when vertices were added.
bool Mesh::selectVertFaceMinAngleLT( double rMaxAngle ) {
	bool retVal = getVertFaceMinAngleLT( rMaxAngle, &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Adds all vertices from all faces with a maximum angle larger than rMinAngle to the selection - see getVertFaceMaxAngleGT().
//! rMinAngle has to be in radiant.
//! @returns true, when vertices were added.
bool Mesh::selectVertFaceMaxAngleGT( double rMinAngle ) {
	bool retVal = getVertFaceMaxAngleGT( rMinAngle, &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Adds all vertices with the given label numbers to the selection - see getVertLabelNo().
//! @returns true, when vertices were added.
bool Mesh::selectVertLabelNo() {
	std::set<int64_t> selectedLabelIds;
	for( auto const& currVertex: mSelectedMVerts ) {
		uint64_t currLabelId = _NOT_A_NUMBER_INT_;
		if( currVertex->getLabel( currLabelId ) ) {
			selectedLabelIds.insert( currLabelId );
			// std::cout << "[Mesh::" << __FUNCTION__ << "] Label for selection: " << currLabelId << std::endl;
		}
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] Number labels selected: " << selectedLabelIds.size() << std::endl;

	if( !showEnterText( selectedLabelIds, "Enter label numbers - negative for inverting" ) ) {
		return( false );
	}

	bool retVal = selectVertLabelNo( selectedLabelIds );
	return( retVal );
}

//! Adds all vertices with being labeled as background (0) to the selection - see getVertLabelBackGrd().
//! @returns true, when vertices were added.
bool Mesh::selectVertLabelBackGrd() {
	bool retVal = getVertLabelBackGrd( mSelectedMVerts );
	selectedMVertsChanged();
	return( retVal );
}

//! Adds all vertices with the given label numbers to the selection - see getVertLabelNo().
//! Allows negative indicies for inverted selection.
//!
//! @returns true, when vertices were added.
bool Mesh::selectVertLabelNo(
		std::set<int64_t>& rLabelNrs
) {
	bool retVal = getVertLabelNoMulti( rLabelNrs, mSelectedMVerts );
	selectedMVertsChanged();
	return( retVal );
}

//! Adds all vertices with the given label numbers to the selection - see getVertLabelNo().
//!
//! @returns true, when vertices were added.
bool Mesh::selectVertLabelNo(
		std::set<uint64_t>& rLabelNrs
) {
	bool retVal = getVertLabelNoMulti( rLabelNrs, mSelectedMVerts );
	selectedMVertsChanged();
	return( retVal );
}

//! Add vertices having no label no. set, which are also not background!.
//! @returns true, when vertices were added.
bool Mesh::selVertLabeledNot() {
	bool retVal = getVertLabeledNot( &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Add vertices having one (or more) flag(s) set.
//! @returns true, when vertices were added.
bool Mesh::selVertByFlag( Primitive::ePrimitiveFlags rFlag ) {
	bool retVal = getVertWithFlag( &mSelectedMVerts, rFlag );
	selectedMVertsChanged();
	return retVal;
}

//! Inverts the selection of multiple vertices.
bool Mesh::selectVertInvert() {
	bool retVal = true;
	for( auto const& currVertex: mVertices ) {
		currVertex->setFlag( FLAG_SELECTED );
	}
	for( auto const& currVertex: mSelectedMVerts ) {
		currVertex->clearFlag( FLAG_SELECTED );
	}
	mSelectedMVerts.clear();
	for( auto const& currVertex: mVertices ) {
		if( currVertex->getFlag( FLAG_SELECTED ) ) {
			mSelectedMVerts.insert( currVertex );
		}
	}
	retVal |= selectedMVertsChanged() != 0;
	return retVal;
}

//! Adds all vertices belonging to selected faces (SelMFaces) to the selction of vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::selectVertFromFaces() {
	bool retVal = true;
	mSelectedMVerts.clear();
	for( auto const& currFace: mFaces ) { // mFacesSelected
		if( currFace->getFlag( FLAG_SELECTED ) ) {
			currFace->getVertABC( &mSelectedMVerts );
		}
	}
	retVal |= selectedMVertsChanged() != 0;
	return retVal;
}

//! Adds all vertices belonging to ridges of edges along faces to the selction of vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::selectVertFromFacesRidges() {
	bool retVal = true;
	mSelectedMVerts.clear();
	for( auto const& currFace: mFaces ) {
		currFace->getFuncValVertRidge( &mSelectedMVerts );
	}
	retVal |= selectedMVertsChanged() != 0;
	return retVal;
}



//--- Ridge Point Extraction with Non-Maximum Suppression -----------------------------------------------------------------------------------------------------------------------------------------

//TODO: could be slimlined and moved to vertex class
bool Mesh::getSurroundingVerticesInOrder (list<Vertex*> &adjacentVertsInOrder, Vertex* &pi, bool printDebug) {

	if(printDebug) {
		cout << endl << "[Mesh::getSurroundingVerticesInOrder] pi: " << pi->getIndex() << endl;
	}

	set<Face*> facesVisited;
	set<Face*> facesAdjacent;
	pi->getFaces(&facesAdjacent);
	Face* nextFace = nullptr;
	Face* currentFace = (*facesAdjacent.begin());
	facesVisited.insert( currentFace );
	nextFace = currentFace->getNextFaceWith( pi, &facesVisited );
	adjacentVertsInOrder.clear();

	if(printDebug) {
		cout << "[Mesh::getSurroundingVerticesInOrder] Vertex A,B,C: " << currentFace->getVertA()->getIndex() << "\t" <<
		        currentFace->getVertB()->getIndex() << "\t" << currentFace->getVertC()->getIndex() << endl;
		cout << "[Mesh::getSurroundingVerticesInOrder] size of facesAdjacent: " << facesAdjacent.size() << endl;
		cout << "[Mesh::getSurroundingVerticesInOrder] curFaceID1: " << currentFace->getIndex() << endl;
	}

	if(pi == currentFace->getVertA()) {
		adjacentVertsInOrder.push_back(currentFace->getVertA());
		adjacentVertsInOrder.push_back(currentFace->getVertC());
		adjacentVertsInOrder.push_back(currentFace->getVertB());
	}
	if(pi== currentFace->getVertB()) {
		adjacentVertsInOrder.push_back(currentFace->getVertB());
		adjacentVertsInOrder.push_back(currentFace->getVertC());
		adjacentVertsInOrder.push_back(currentFace->getVertA());
	}
	if(pi == currentFace->getVertC()) {
		adjacentVertsInOrder.push_back(currentFace->getVertC());
		adjacentVertsInOrder.push_back(currentFace->getVertA());
		adjacentVertsInOrder.push_back(currentFace->getVertB());
	}

	while( nextFace != nullptr ) {
		currentFace = nextFace;
		nextFace = currentFace->getNextFaceWith( pi, &facesVisited );
		facesVisited.insert( currentFace );

		if(printDebug) {
			cout << "[Mesh::getSurroundingVerticesInOrder] curFaceID2: " << currentFace->getIndex() << endl;
		}

		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertA()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_back(currentFace->getVertA());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertB()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_back(currentFace->getVertB());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertC()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_back(currentFace->getVertC());
		}
	}

	// in case of a border vertex we also have to dance around in the other direction:
	// in this case, the vertices are pushed to the beginning, to ensure the correct order in the list
	currentFace = (*facesAdjacent.begin());
	nextFace = currentFace->getNextFaceWith( pi, &facesVisited );
	    if(printDebug) {
			cout << "[Mesh::getSurroundingVerticesInOrder] curFaceID3: " << currentFace->getIndex() << endl;
		}

		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertA()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertA());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertB()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertB());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertC()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertC());
		}

		while( nextFace != nullptr ) {
		currentFace = nextFace;
		nextFace = currentFace->getNextFaceWith( pi, &facesVisited );
		facesVisited.insert( currentFace );

		if(printDebug) {
			cout << "[Mesh::getSurroundingVerticesInOrder] curFaceID4: " << currentFace->getIndex() << endl;
		}

		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertA()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertA());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertB()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertB());
		}
		if(find(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), currentFace->getVertC()) == adjacentVertsInOrder.end()) {
			adjacentVertsInOrder.push_front(currentFace->getVertC());
		}
	}

	adjacentVertsInOrder.erase(std::remove(adjacentVertsInOrder.begin(), adjacentVertsInOrder.end(), pi), adjacentVertsInOrder.end());

	if(printDebug) {
		cout << "[Mesh::getSurroundingVerticesInOrder] size of adjacentVertsInOrder: " << adjacentVertsInOrder.size() << endl;
		list<Vertex*>::iterator iter;
		for(iter=adjacentVertsInOrder.begin(); iter!=adjacentVertsInOrder.end();++iter) {
			cout << (*iter)->getIndex() << endl;
		}
	}

	return true;
}

//groups the current helperList and writes the vertices directly into adjacentVertsInOrder
bool Mesh::groupListMerge(list<pair<Vertex*, int>> &helperList, list<Vertex*> &adjacentVertsInOrder, bool printDebug) {

	int count = std::distance( helperList.begin(), helperList.end() );

	if(printDebug) {
		cout << "[Mesh::groupListMerge] count: " << count << endl;
	}

	Vector3D posVec;
	Vector3D normVec;
	double funcValue = 0;
	double temp = 0;
	for(auto it : helperList) {
		posVec += it.first->getPositionVector();
		normVec += it.first->getNormal();
		it.first->getFuncValue(&temp);
		funcValue += temp;
	}
	posVec /= count;
	normVec /= count;
	funcValue /= count;

	Vertex *tempVertPointer = new Vertex( posVec, normVec );
	tempVertPointer->setFuncValue(funcValue);

	int uniqueVertexIndexCount = getVertexNr();
	tempVertPointer->setIndex(++uniqueVertexIndexCount);

	adjacentVertsInOrder.push_back(tempVertPointer);

	return true;
}


//merges the first and last vertex groups if the last two elements contain the same vertex
bool Mesh::organizeGroupList(list<pair<Vertex*, int>> &Lgroup, list<Vertex*> &adjacentVertsInOrder, bool printDebug) {

	int groupToMerge = 0;
	if((*Lgroup.rbegin()).first == (*next(Lgroup.rbegin(),1)).first) {
		groupToMerge = (*next(Lgroup.rbegin(),1)).second;
		Lgroup.pop_back();
	}

	if(groupToMerge != 0) {
		for(auto it : Lgroup) {
			if(it.second == groupToMerge) it.second = 0;
		}
	}

	//search predicate to search the list of pairs dependent on the second entry (groupValue - int)
	struct sort_pred {
		bool operator()(const std::pair<Vertex*,int> &left, const std::pair<Vertex*,int> &right) {
			return left.second < right.second;
		}
	};

	Lgroup.sort( sort_pred() );

	list<pair<Vertex*, int>> helperList;
	int curGroup = 0;
	for(auto it : Lgroup) {
		if(it.second != curGroup) {
			groupListMerge(helperList, adjacentVertsInOrder, printDebug);

			if(printDebug) {
				cout << "*** current helper List ***" << endl;
				for(auto tmpit : helperList) {
					cout << tmpit.first->getIndex() << endl;
				}
			}

			helperList.clear();
			curGroup = it.second;
		}
		helperList.push_back(it);
	}

	groupListMerge(helperList, adjacentVertsInOrder, printDebug);

	if(printDebug) {
		cout << "*** current helper List ***" << endl;
		for(auto tmpit : helperList) {
			cout << tmpit.first->getIndex() << endl;
		}
	}

	return true;
}

//calculates the Intersection of a plane and an edge (as line segment, not as infinite line) and gives the intersection point back
//also interpolates between the function values of the vertices spanning the line segment
//TODO: could be slimlined and moved to plane class (without interpolation functionality)
bool Mesh::PlaneSegmentIntersectionAndInterpolation(Vertex* &planeVertex, Vector3D planeNormal, Vertex* &segmentPoint1, Vertex* &segmentPoint2,
                                                    Vector3D &resIntersectionPoint, double &resInterpolatedFunctionValue, bool debugPrint) {

	Vector3D u = segmentPoint2->getPositionVector() - segmentPoint1->getPositionVector();
	Vector3D w = segmentPoint1->getPositionVector() - planeVertex->getPositionVector();

	float D = dot3(planeNormal, u);
	float N = -dot3(planeNormal, w);

	if (abs(D) < 0.001) return false; //segment parallel to plane (warning: can also lie IN plane)

	//if not parallel, compute intersect param
	float sI = N/D;
	if(sI < 0 || sI > 1) return false;  //no intersection

	resIntersectionPoint = segmentPoint1->getPositionVector() + sI * u; //compute segment intersect point

	double dist1 = (segmentPoint1->getPositionVector() - resIntersectionPoint).getLength3();
	double dist2 = (segmentPoint2->getPositionVector() - resIntersectionPoint).getLength4();

	if(debugPrint) {
		double distcheck = (segmentPoint1->getPositionVector() - segmentPoint2->getPositionVector()).getLength3();
		cout << "dist1: " << dist1 << endl;
		cout << "dist2: " << dist2 << endl;
		cout << "dist1+2: " << dist1+dist2 << endl;
		cout << "dist check: " << distcheck << endl;
	}

	double funcValue1;
	double funcValue2;
	segmentPoint1->getFuncValue(&funcValue1);
	segmentPoint2->getFuncValue(&funcValue2);

	//linear interpolation with distances to get new function value
	resInterpolatedFunctionValue = funcValue1 + dist1 * ((funcValue2-funcValue1) / (dist1 + dist2) );

	if(debugPrint) {
		cout << "function Value 1: " << funcValue1 << endl;
		cout << "function Value 2: " << funcValue2 << endl;
		cout << "interpolated Function Value: " << resInterpolatedFunctionValue << endl;
	}

	return true;
}

//As subsets of consecutive vertices p_j can be on a plane the 1-ring has to be simplified to provide a robust tracing of ridge points
bool Mesh::temporaryMeshSimplification(list<Vertex*> &adjacentVertsInOrder, double eps, bool printDebug) {

	int groupLabel = 0;
	list<pair<Vertex*, int>> Lgroup;

	auto pj = adjacentVertsInOrder.begin();

	Lgroup.emplace_back((*pj), groupLabel);
	Vector3D nj = (*pj)->getNormal( true );

	for(auto pjnext = next(pj, 1); pjnext != adjacentVertsInOrder.end(); pjnext++) {
		Vector3D njnext = (*pjnext)->getNormal( true );
		if((1-dot3(nj, njnext)) <= eps) {
			Lgroup.emplace_back((*pjnext), groupLabel);
		}
		else {
			groupLabel++;
			Lgroup.emplace_back((*pjnext), groupLabel);
		}
		pj = pjnext;
		nj = njnext;
	}

	if((1-dot3(nj, (*adjacentVertsInOrder.begin())->getNormal( true ))) <= eps) {
		Lgroup.emplace_back((*pj), 0);
	}

	adjacentVertsInOrder.clear();
	organizeGroupList(Lgroup, adjacentVertsInOrder, false);
	Lgroup.clear();

	if(printDebug) {
		cout << "adjacentVertsInOrder: " << endl;
		for(auto vert : adjacentVertsInOrder) {
			double functionValue;
			vert->getFuncValue(&functionValue);
			cout << vert->getIndex() << "\t" << functionValue << "\t" << vert->getPositionVector() << endl;
		}
	}

	return true;
}

//find all vertices which lie on the secondary direction
bool Mesh::findVerticesOnSecondaryDirection(Vertex* &curVertex, Vector3D principalDirection, vector<double> &comparisonFunctionValues, Vertex* &VertexMAXnormal, bool printDebug) {

	Vertex* vert1;
	Vertex* vert2;
	Face::eEdgeNames edgeIdx = Face::EDGE_NONE;
	set<Face*> faces;
	curVertex->getFaces(&faces);

	//debug output
	if(printDebug) {
		cout << "curVertex: " << curVertex->getIndex() << endl
		     << "Position: " << curVertex->getPositionVector() << endl
		     << "principal direction vertex: " << VertexMAXnormal->getPositionVector() << endl
		     << "#################################################################" << endl << endl;
	}

	for(auto& face : faces) {
		face->getOposingEdgeAndVertices( curVertex, &edgeIdx, &vert1, &vert2 );
		Vector3D intersectionPoint;
		double intersectionFunctionValue;
		bool isIntersection = PlaneSegmentIntersectionAndInterpolation(curVertex, principalDirection, vert1, vert2, intersectionPoint, intersectionFunctionValue, false);

		//Note: comparisonFunctionValues normally contains 2 values, because 2 edges are intersected by the plane
		//if an edge is parallel and on the plane, it can actually intersect more edges
		//due to interpolation the found edges which share a vertex should be identical though
		if(isIntersection) comparisonFunctionValues.push_back(intersectionFunctionValue);

		//debug output
		if(printDebug) {
			cout << "from: " << vert1->getIndex() << endl << "to " << vert2->getIndex() << endl;
			cout << "from: " << vert1->getPositionVector() << endl << "to " << vert2->getPositionVector() << endl;
			cout << "isIntersection: " << isIntersection << endl;
			cout << "intersectionPoint: " << intersectionPoint << endl << endl;
		}

	}

	//debug output
	if(printDebug && comparisonFunctionValues.size() > 2) {
		cout << "curVertex: " << curVertex->getIndex() << endl
		     << "Position: " << curVertex->getPositionVector() << endl
		     << "principal direction vertex: " << VertexMAXnormal->getPositionVector() << endl
		     << "#################################################################" << endl << endl;
	}

	return true;
}

//non-maximum selection main function
//based on AAPR16 Paper "Schönpflug, Mara: Ridge Point Extraction with Non-Maximum Suppression on Irregular Grids"
//http://archiv.ub.uni-heidelberg.de/volltextserver/20734/1/aapr16_richard_schonpflug_hubert_mara_FINAL_20160430.pdf
bool Mesh::selectVertNonMaximum(double eps) {

	    Vertex* pi;
		vector<Vertex*> Lmax;
		list<Vertex*> adjacentVertsInOrder;

		clock_t timerBegin = clock();

	//loop over all vertices
	for( uint64_t vertIdx = 0; vertIdx<getVertexNr(); vertIdx++ ) {

		        pi = getVertexPos( vertIdx );

				//calculate function value of current vertex pi
				double fpi = 0;
				pi->getFuncValue(&fpi);

				//if the current vertex FuncValue or Normal is NaN, skip it
				if( std::isnan(fpi) || std::isnan(pi->getNormalX())) {
					continue;
				}

				//getting the sourrounding vertices in order, saved in list (because set orders randomly and vector iterators are invalidated by deletion)
				getSurroundingVerticesInOrder(adjacentVertsInOrder, pi, false);

				//mesh gets simplified here
				temporaryMeshSimplification(adjacentVertsInOrder, eps, false);

				//the 1-ring needs to contain at least 3 vertices to be considered a manifold (2 if on a mesh border), it is skipped otherwise
				if(pi->isBorder()) {
					if(adjacentVertsInOrder.size() < 2) continue;
				}
				else {
					if(adjacentVertsInOrder.size() < 3) continue;
				}

				//Find the maximum gradient aka the principal direction by calculating the dot product between vert pi and all verts pj
				double dotmax = -DBL_MAX;
				Vertex* pj = nullptr;
				Vector3D nj = pi->getNormal( true );
				for (auto& vert : adjacentVertsInOrder) {
					double dot = dot3(vert->getNormal (true), nj);

					if(dot > dotmax) {
						dotmax = dot;
						pj = vert;
					}
				}

				Vector3D principalDirection;

				if(pj)
				{
					principalDirection = pj->getPositionVector() - pi->getPositionVector();
				}

				//find all vertices which lie on the secondary direction
				vector<double> vertsSecDir;
				findVerticesOnSecondaryDirection(pi, principalDirection, vertsSecDir, pj, false);

				//Perform actual non-max suppression
				if(vertsSecDir.size() > 0) {
					if(fpi > *max_element(vertsSecDir.begin(), vertsSecDir.end()))
						Lmax.push_back(pi);
				}
				vertsSecDir.clear();

				for(auto vert : adjacentVertsInOrder) {
					delete vert;
				}

				adjacentVertsInOrder.clear();

	    }

	    //add found maxima to the selection which is going to be displayed
	    addToSelection(&Lmax);
		Lmax.clear();

		clock_t timerEnd = clock();
		double elapsed_secs = double(timerEnd - timerBegin) / CLOCKS_PER_SEC;
		cout << "nonMaximum computation took: " << elapsed_secs << " seconds for " << getVertexNr() << " vertices." << endl;
		cout << "________________DONE______________" << endl;


		return true;
}

//checks if there are two maximum vertices next to each other
//this is not possible if the non-maxima are suppressed correctly
//TODO: when the vertex is on a tip of the mesh with outgoing skeleton lines, it is possible, leading to a number of false positives
bool Mesh::nonMaximumCorrectnessCheck() {

	set<Face*> faces;
	Face::eEdgeNames edgeIdx = Face::EDGE_NONE;
	Vertex* vert1;
	Vertex* vert2;
	set<Vertex*> errorVerts;

	//iterate over all selected vertices (results of non-maximum suppression)
	for(auto& vert : mSelectedMVerts) {
		//all adjacent Faces of the current vertex are added to a set
		vert->getFaces(&faces);
		for(auto& face : faces) {
			//outbound of the current vertex, the vertices forming the opposing edge for the current face are fetched
			face->getOposingEdgeAndVertices( vert, &edgeIdx, &vert1, &vert2 );
			//if both faces are also non-maxima, we have found an erroneous vertex
			if((mSelectedMVerts.find(vert1) != mSelectedMVerts.end()) && (mSelectedMVerts.find(vert2) != mSelectedMVerts.end())) {
				errorVerts.insert(vert);
			}
		}
	}
	cout << "Number of erroneous vertices: " << errorVerts.size() << endl;
	for(auto& i : errorVerts) {
		cout << i->getIndex() << endl;
	}
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//Line Tracing Component
bool Mesh::traverseSkeletonLine(vector<vertexNeighbourhoodHelper> &neighbourhood, vertexNeighbourhoodHelper &x, vertexNeighbourhoodHelper &start, PolyLine* &line) {
	set<Vertex*> adjacentVerts;
	x.vert->getNeighbourVertices(&adjacentVerts);

	//if no PolyLine is active, create a new one and add the current vertex x to it
	if(line == nullptr) {
		line = new PolyLine();
		line->addBack(x.vert);
		start = x;
	}

	    //loop over all adjacent vertices
	    for(auto& vert : adjacentVerts) {

			//look if current vertex vert is in the active selection
			auto y = find_if(neighbourhood.begin(), neighbourhood.end(),
			    [&vert](const vertexNeighbourhoodHelper& helper)
			    {
				    return helper.vert == vert;
			    }
			);

		//if the vertice is in the active selection AND not yet visited AND not the start vertex AND there is an active line, continue
		if(y != neighbourhood.end() && !y->visited && y->vert != start.vert && line != nullptr) {

			//in case the vertex is a split vertex, we add it and end the line
			if(y->type == 0) {
				line->addBack(y->vert);
				mPolyLines.push_back(line);
				line = nullptr;
			}
			//in case the vertex is an end vertex, we add it, mark it visited and end the line
			else if(y->type == 1) {
				y->visited = true;
				line->addBack(y->vert);
				mPolyLines.push_back(line);
				line = nullptr;
			}
			//in case the vertex is a line vertex, we add it and traverse to it's neighbours
			else if(y->type == 2) {
				y->visited = true;
				line->addBack(y->vert);
				traverseSkeletonLine(neighbourhood, (*y), start, line);
			}
		}
		}
		//when all vertices outgoing of the current start vertex x are visited, set itself to visited aswell
		x.visited = true;
	return true;
}

bool Mesh::createSkeletonLineInit(int connectTriangles) {
	//check if there are currently any vertices selected, abort if there are not
	if( mSelectedMVerts.size() == 0 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] no vertices given." << endl;
		return false;
	}

	//delete possibly existing polylines
	mPolyLines.clear();

	//create the initial vertex neigbourhood where:
	// - vertices on a split point have type 0
	// - vertices at the end of a line have type 1
	// - vertices on a line have type 2
	vector<vertexNeighbourhoodHelper> vertexNeighbourhood;
	for(auto& vert : mSelectedMVerts) {
		set<Vertex*> adjacentVerts;
		vert->getNeighbourVertices(&adjacentVerts);
		int counter = 0;
		for(auto adjvert : adjacentVerts) {
			bool is_in = mSelectedMVerts.find(adjvert) != mSelectedMVerts.end();
			if(is_in) counter++;
		}
		//filter all vertices with no neighbors in the same set
		if(counter > 0) {
			//vertices with 2 or more neighbors are split vertices, and have type 0
			if(counter > 2) counter = 0;
			vertexNeighbourhood.emplace_back(vertexNeighbourhoodHelper(vert, counter, false));
		}
	}

	//sort the vertex neighbourhood by type, from split to end to line vertex
	sort(vertexNeighbourhood.begin(), vertexNeighbourhood.end());

	//iterate over the whole neighbourhood
	for(auto it : vertexNeighbourhood) {

		//create a new pointer to a polyline, which is passed through the entire traversion
		PolyLine* line = nullptr;

		if(!it.visited) {
			traverseSkeletonLine(vertexNeighbourhood, it, it, line);
		}

		delete line;
	}

	/*****************************************************experimental*******************************************************/

	//method to repair missing polylines:
	// - search for faces with all 3 vertices in mSelectedMVerts
	// - these should have a missing polyline connection
	// - connect all 3 vertices with polylines

	//an alternative method would be to merge these 3 vertices into one and
	//connect it somehow to its old neighbors and do the skeleton line computation afterwards

	if(connectTriangles == 1) {

		set<vector<Vertex*>> fullFaces;
		for(auto& vert : mSelectedMVerts) {
			set<Face*> faces;
			vert->getFaces(&faces);
			for(auto face : faces) {
				set<Vertex*> faceVerts;
				face->getVertABC(&faceVerts);
				bool fullFace = true;
				for(auto faceVert : faceVerts) {
					bool is_in = mSelectedMVerts.find(faceVert) != mSelectedMVerts.end();
					if(!is_in) fullFace = false;
				}
				if(fullFace) {
					vector<Vertex*> tmpVec(faceVerts.begin(),
					                            faceVerts.end());
					fullFaces.emplace(tmpVec);
					tmpVec.clear();
				}
			}
		}

		//print full faces
		//add vertices in full faces to a polyline
		//close the line to get a full triangle
		//note that this method is unpolished and can have overlap with existing polylines
		for(const vector<Vertex*>& setIt : fullFaces) {
			std::cout << "Face spanned by vertices: ";

			PolyLine* fixline = new PolyLine;
			//fixline->addVerticesTo(&setIt);
			for(Vertex* vecIt : setIt) {
				std::cout << vecIt->getIndex() << " ";
				fixline->addBack(vecIt);
			}
			fixline->closeLine();
			mPolyLines.push_back(fixline);
			fixline = nullptr;

			std::cout << "is FULL." << std::endl;
		}

	}

	/****************************************************************************************************************/

	cout << "[Mesh::" << __FUNCTION__ << "]: " << mPolyLines.size() << " PolyLines extracted." << endl;
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Empties the selection of faces.
bool Mesh::selectFaceNone() {
	mFacesSelected.clear();
	selectedMFacesChanged();
	return true;
}

//! Adds all sticky faces to the selection.
bool Mesh::selectFaceSticky() {
	bool retVal = getFaceSticky( &mFacesSelected );
	selectedMFacesChanged();
	return retVal;
}

//! Adds all sticky faces to the selection.
bool Mesh::selectFaceNonManifold() {
	bool retVal = getFaceNonManifold( &mFacesSelected );
	selectedMFacesChanged();
	return retVal;
}

//! Selects all faces that are opposite to a given plane, i.e. in the direction of the
//! plane normal. Selection is stored in `mFacesSelected` as usual.
bool Mesh::selectFacesOppositeToPlane(double nx, double ny, double nz, double d, bool reverseDirection) {

	Vector3D planeHNF( nx, ny, nz, d );
	funcVertDistanceToPlane( planeHNF, false );

	if(reverseDirection) {
		selectVertFuncValGreatThan(0.0);
	}
	else {
		selectVertFuncValLowerThan(0.0);
	}

	// Select all faces that are adjacent to the selected vertices
	mFacesSelected.clear();
//	for(std::set<Vertex*>::iterator vIt = mSelectedMVerts.begin(); vIt != mSelectedMVerts.end(); vIt++) {
//		(*vIt)->getFaces(&mFacesSelected);
//	}
	for( Vertex* vertex : mSelectedMVerts) {
		vertex->getFaces(&mFacesSelected);
	}
	selectedMFacesChanged();
	return(true);
}

//! Adds all faces with zero area to the selection.
bool Mesh::selectFaceZeroArea() {
	bool retVal = getFaceZeroArea( &mFacesSelected );
	selectedMFacesChanged();
	return retVal;
}

bool Mesh::selectFaceInSphere( Vertex* rSeed, double rRadius ) {
	//! Select all faces within a sphere using Mesh::fetchSphereBitArray()
	vector<Face*> facesInSphere;
	// Allocate the bit arrays for vertices:
	uint64_t* vertBitArrayVisited;
	int vertNrLongs = getBitArrayVerts( &vertBitArrayVisited );
	// Allocate the bit arrays for faces:
	uint64_t* faceBitArrayVisited;
	uint64_t faceNrLongs = getBitArrayFaces( &faceBitArrayVisited );
	if( !fetchSphereBitArray( rSeed, &facesInSphere, rRadius,  vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: fetchSphereBitArray failed!" << endl;
		return false;
	}
	// Add faces to selection:
	vector<Face*>::iterator itFace;
	for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
		mFacesSelected.insert( (*itFace) );
	}
	selectedMFacesChanged();
	// Free bit arrays:
	delete vertBitArrayVisited;
	delete faceBitArrayVisited;
	return true;
}

//! Selects a random number of faces using a given ratio.
//! @returns false in case of an error. True otherwise.
bool Mesh::selectFaceRandom( double rRatio ) {
	Face* currFace = nullptr;
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		double randVal = static_cast<double>( rand() % 100 );
		if( randVal < rRatio ) {
			mFacesSelected.insert( currFace );
		}
	}
	selectedMFacesChanged();
	return true;
}

//! Selects self-intersecting faces.
//! @returns true if successful.
bool Mesh::selectFaceSelfIntersecting() {
	// Progress - Start
    // Octree required - also time consuming
    if( mOctree == nullptr ) {
        generateOctree( 1000 );
    }
    vector<Face*> intersectedFaces;
    mOctree->detectselfintersections(intersectedFaces);
    mFacesSelected.insert(intersectedFaces.begin(), intersectedFaces.end());
    selectedMFacesChanged();
	return( true );
}

//! Selects all face having one or more vertices with the FLAG_SYNTHETIC set.
//!
//! Attention: Faces also have this flag, BUT only those of the vertices are stored.
//! Therefore holes filled with GigaMesh can only be determined using this approach.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectFaceWithSyntheticVertices() {
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		unsigned int numberSynthVert;
		currFace->hasSyntheticVertex( numberSynthVert );
		if( numberSynthVert > 0 ) {
			mFacesSelected.insert( currFace );
		}
	}
	selectedMFacesChanged();
	return true;
}

//! Selects all faces having all three vertices along a border.
//!
//! Typically used for erosion of the mesh border before filling.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectFaceBorderThreeVertices() {
	bool retVal = getFaceBorderThreeVertices( mFacesSelected );
	selectedMFacesChanged();
	return( retVal );
}

//! Selects all faces having all three vertices selected i.e.
//! are in SelMVerts.
//!
//! Typically used for erosion of the mesh border before filling.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectFaceThreeVerticesSelected() {
	bool retVal = getFaceThreeVerticesSelected( mFacesSelected );
	selectedMFacesChanged();
	return( retVal );
}

//! Selects all faces along a border connecting three bridges.
//!
//! Related to erosion of the mesh border before filling.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectFaceBorderBridgeTriConnect() {
	bool retVal = getFaceBorderVertsEdges( mFacesSelected, 3, 0 );
	selectedMFacesChanged();
	return( retVal );
}

//! Selects all faces along a border being part of a bridge.
//!
//! Related to erosion of the mesh border before filling.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectFaceBorderBridge() {
	bool retVal = getFaceBorderVertsEdges( mFacesSelected, 3, 1 );
	selectedMFacesChanged();
	return( retVal );
}

//! Selects all dangling faces along a border.
//!
//! Typically used for erosion of the mesh border before filling.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectFaceBorderDangling() {
	bool retVal = getFaceBorderVertsEdges( mFacesSelected, 3, 2 );
	selectedMFacesChanged();
	return( retVal );
}

//! Selects all faces having three vertices with different label ids
//! i.e. corner of a voronoi cell.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::selectFaceLabeledVerticesCorner() {
	bool retVal = getFaceLabeledVerticesCorner( mFacesSelected );
	selectedMFacesChanged();
	return( retVal );
}

//--- Polylines ------------------------------------------------------------------------------------------------------------------------------------------------

//! Selects all polylines not being part/border of a label.
bool Mesh::selectPolyNoLabel() {
	bool retVal = getPolyNoLabel( &mPolyLinesSelected );
	selectedMPolysChanged();
	return retVal;
}

//! Selects the longest polyline with a run length minimum of rValue.
bool Mesh::selectPolyRunLenGT( double rValue ) {
	bool retVal = getPolyRunLenGT( &mPolyLinesSelected, rValue );
	selectedMPolysChanged();
	return retVal;
}

//! Selects the longest polyline with a run length maximum of rValue.
bool Mesh::selectPolyRunLenLT( double rValue ) {
	bool retVal = getPolyRunLenLT( &mPolyLinesSelected, rValue );
	selectedMPolysChanged();
	return retVal;
}

//! Selects the longest polyline ...
bool Mesh::selectPolyLongest() {
	bool retVal = getPolyLongest( &mPolyLinesSelected );
	selectedMPolysChanged();
	return retVal;
}

//! Selects the longest polyline ...
bool Mesh::selectPolyShortest() {
	bool retVal = getPolyShortest( &mPolyLinesSelected );
	selectedMPolysChanged();
	return retVal;
}

//! Adds all polylines with the given label numbers to the selection - see getVertLabelNo().
bool Mesh::selectPolyLabelNo( const vector<int>& rLabelNrs ) {
	bool retVal = getPolyLabelNo( rLabelNrs, &mPolyLinesSelected );
	selectedMPolysChanged();
	return retVal;
}

//! Adds all polylines not belonging to a label to the selection - see getPolyNotLabel().
bool Mesh::selectPolyNotLabeled() {
	bool retVal = getPolyNotLabel( &mPolyLinesSelected );
	selectedMPolysChanged();
	return retVal;
}

//! Adds all polylines with a given range of numbers.
bool Mesh::selectPolyVertexCount( unsigned int rMinNr, unsigned int rMaxNr ) {
	bool retVal = getPolyVertexCount( &mPolyLinesSelected, rMinNr, rMaxNr );
	selectedMPolysChanged();
	return retVal;
}

//--- Vertices -------------------------------------------------------------------------------------------------------------------------------------------------

//! Adds the selected vertices' references to the given set.
//! @returns false in case of an error.
bool Mesh::getSelectedVerts( set<Vertex*>* rSomeVerts ) {
	set<Vertex*>::iterator itVert;
	for( itVert=mSelectedMVerts.begin(); itVert!=mSelectedMVerts.end(); itVert++ ) {
		rSomeVerts->insert( (*itVert) );
	}
	return true;
}

//--- Single Selection SelPrim i.e. SelVert and SelFace --------------------------------------------------------------------------------------------------------

//! Returns the primitive next to the selected pixel.
//! Returns NULL, when nothing was (or could be) selected.
//!
//! Remark: Use Primitive::getType to determine the type of class.
Primitive* Mesh::getPrimitiveSelected() {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
#endif
	return mPrimSelected;
}

//! Select the single primitive/vertex using an index given by the user.
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool Mesh::selectPrimitiveVertexUser() {
	uint64_t vertexIndex;
	if( !showEnterText( vertexIndex, "Enter vertex index for selection" ) ) {
		showWarning( "Invalid Value",
		             "An invalid value was entered." );
		return( false );
	}
	Vertex* selectedVertex = getVertexByIdx( vertexIndex ) ;
	// when no Vertex was found we can't do anything and return
	if( selectedVertex == nullptr ) {
		showWarning( "Vertex not found",
		             "Vertex with index " + std::to_string( vertexIndex)  + "was not found." );
		return( false );
	}
	// Done
	mPrimSelected = selectedVertex;
	return( true );
}

//--- Plane definition -----------------------------------------------------------------------------------------------------------------------------------------

//! Returns true, when the last(=third) position of the plane is set - so mPlanePosIdx will be PLANE_VERT_A.
bool Mesh::isPlanePosCSet() {
	return ( mPlanePosIdx == Plane::PLANE_VERT_A );
}

//! Returns the ID of the next vertex to be set, which defines the mesh's plane.
bool Mesh::getPlanePosToSet( int* rPosID ) {
	(*rPosID) = mPlanePosIdx;
	return true;
}

bool Mesh::setPlaneVPos( vector<double>* rPositions ) {
	//! Set all three vertices defining a plane. 3x3 coordinates required.
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	if( rPositions == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	if( rPositions->size() != 9 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] Wrong number of values: " << rPositions->size() << " - expected: 9!" << endl;
		return false;
	}
	// Set to first position:
	mPlanePosIdx = Plane::PLANE_VERT_A;
	// Set the three vertices:
	Vector3D vertA( rPositions->at(0), rPositions->at(1), rPositions->at(2), 1.0 );
	Vector3D vertB( rPositions->at(3), rPositions->at(4), rPositions->at(5), 1.0 );
	Vector3D vertC( rPositions->at(6), rPositions->at(7), rPositions->at(8), 1.0 );
	setPlanePos( &vertA );
	setPlanePos( &vertB );
	setPlanePos( &vertC );
	return true;
}

//! Sets the next point defining a plane - rotates A->B->C->A->....
//! Returns false in case of an error.
bool Mesh::setPlanePos( Vector3D*  rSomePos ) {
	if( rSomePos == nullptr ) {
		return false;
	}
	mPlane.setData( mPlanePosIdx, rSomePos );
	switch( mPlanePosIdx ) {
		case Plane::PLANE_VERT_A:
			mPlanePosIdx = Plane::PLANE_VERT_B;
			break;
		case Plane::PLANE_VERT_B:
			mPlanePosIdx = Plane::PLANE_VERT_C;
			break;
		case Plane::PLANE_VERT_C:
			mPlanePosIdx = Plane::PLANE_VERT_A;
			break;
		default:
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Invalid mPlanePosIdx!" << endl;
			return false;
	}
	return true;
}

bool Mesh::setPlaneAxisPos(const Vector3D& axisTop, const Vector3D& axisBottom, const Vector3D& pos)
{
	return mPlane.setPlaneByAxisAndPosition(axisTop, axisBottom, pos);
}

//! Retrieve the HNF of the plane.
//! Returns false in case of an error.
bool Mesh::getPlaneHNF( Vector3D* rPlaneHNF ) {
	if( rPlaneHNF == nullptr ) {
		return false;
	}
	mPlane.getPlaneHNF( rPlaneHNF );
	if( mPlane.getNormalLen() <= 0.0 ) {
		// Plane has no normal => this can not be a plane at all.
		return false;
	}
	return true;
}

//! Retrieve the HNF of the plane.
//! Returns false in case of an error.
bool Mesh::getPlaneHNF( double* rPlaneHNF ) {
	if( rPlaneHNF == nullptr ) {
		return false;
	}

	mPlane.getPlaneHNF( rPlaneHNF );
	if( mPlane.getNormalLen() <= 0.0 ) {
		// Plane has no normal => this can not be a plane at all.
		return false;
	}
	return true;
}

//! Set the plane using the HNF
bool Mesh::setPlaneHNF( Vector3D* rPlaneHNF ) {
	mPlanePosIdx = Plane::PLANE_VERT_A;
	mPlane.setPlaneHNF( rPlaneHNF );
	return true;
}

//! Define the plane using the (cone) axis and the selected primitive. This will change the Mesh-Plane to an axis-plane
//!
//! See Mesh::mPlane and Mesh::mPrimSelected
//!
//! @returns false in case of an error e.g. degenarted positions or missing axis. True otherwise.
bool Mesh::setPlaneHNFbyAxisSelPrim() {
	if(!mPlane.isValid() || mPrimSelected == nullptr)
		return false;

	Vector3D axisTop, axisBottom;
	if( !getConeAxis( &axisTop, &axisBottom ) ) {
		return( false );
	}

	mPlane.setPlaneByAxisAndPosition( axisTop, axisBottom, mPrimSelected->getCenterOfGravity() );
	return( true );
}

//! Define the plane using the (cone) axis and the last selected position. This will change the Mesh-Plane to an axis-plane
//!
//! See Mesh::mPlane and Mesh::mSelectedPositions
//!
//! @returns false in case of an error, true otherwise
bool Mesh::setPlaneHNFbyAxisAndLastPosition()
{
	if(mSelectedPositions.empty() || !mPlane.isValid())
		return false;

	Vector3D axisTop, axisBottom, selectedPosition;

	if(!getConeAxis(&axisTop, &axisBottom))
	{
		return false;
	}

	selectedPosition = std::get<0>(mSelectedPositions.back());

	return mPlane.setPlaneByAxisAndPosition(axisTop, axisBottom, selectedPosition);
}

//! Orients the plane using the (cone) axis
//! //! See: https://math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d/476311#476311
//! @returns false in case of an error, true otherwise
bool Mesh::orientPlaneHNFbyAxis()
{
	if(!mPlane.isValid())
		return false;

	auto planeNormal = mPlane.getNormal(true);

	Vector3D axisTop, axisBottom;
	if(!getConeAxis(&axisTop, &axisBottom))
	{
		return false;
	}

	auto axisDir = axisTop - axisBottom;
	axisDir.normalize3();

	if(planeNormal == axisDir || -planeNormal == axisDir)
		return true;

	auto V = planeNormal % axisDir;
	auto c = planeNormal * axisDir;

	Matrix4D skew( std::vector<double>{      0.0,  V.getZ(), -V.getY(), 0.0,
	                                   -V.getZ(),       0.0,  V.getX(), 0.0,
	                                    V.getY(), -V.getX(),       0.0, 0.0,
	                                         0.0,       0.0,       0.0, 1.0} );

	Matrix4D transmatrix = Matrix4D(Matrix4D::INIT_IDENTITY) + skew + skew*skew * (1.0 / (1.0 + c));

	transmatrix.get(3,3) = 1.0;

	return applyTransfromToPlane(transmatrix);
}


//! Change the orientation of the plane's normal vector -- see Plane::flipPlane()
//! This is used e.g. for an OpenGL clipping plane.
bool Mesh::flipPlane() {
	return mPlane.flipPlane();
}

//! Returns the intersection of a given edge (not line!) to the plane.
bool Mesh::getPlaneIntersectEdge( Vector3D* rayTop, Vector3D* rayBot, Vector3D* rayIntersect ) {
	return(mPlane.getIntersectionFacePlaneEdge( rayTop, rayBot, rayIntersect ));
}

//! Calculates intersection of a line given by `ptA` and `ptB` with the plane. Result is
//! stored in `lineIntersect`.
bool Mesh::getPlaneIntersectLine( Vector3D* ptA, Vector3D* ptB, Vector3D* lineIntersect ) {
	return( mPlane.getIntersectionFacePlaneLinePos( *ptA, *ptB, *lineIntersect) );
}

//! Returns the intersection of a line with the plane.
bool Mesh::getPlaneIntersectLineDir( Vector3D* rRayPos, Vector3D* rRayDir, Vector3D* rRayIntersect ) {
	return mPlane.getIntersectionFacePlaneLineDir( rRayPos, rRayDir, rRayIntersect );
}

//! Returns the three points defining the plane.
//! threePos is expected to be of size 3x3*float.
//! Returns false in case of an error.
bool Mesh::getPlanePositions( double* rThreePos ) {
	if( rThreePos == nullptr ) {
		return false;
	}
	if( !(mPlane.isValid()) ) {
		return false;
	}
	mPlane.getVertCoords( rThreePos );
	return true;
}

//! Compute the triangle fan for rendering the mesh plane.
//! @returns false in case of an error or an undefined plane.
bool Mesh::getMeshPlaneFan( Vector3D*         rCentralPoint, //!< Central and first point of the triangle fan.
                            vector<Vector3D>* rFanVertices   //!< All subsequent vertices.
                          ) {
	if( !(mPlane.isValid()) ) {
		return( false );
	}

	Vector3D rayBot;
	Vector3D rayTop;
	Vector3D rayIntersect;
	Vector3D centralPointPlane( 0.0, 0.0, 0.0, 0.0 );
	vector<Vector3D> planeIntersections;
	// Bottom plane
	rayBot = getBoundingBoxA();
	rayTop = getBoundingBoxB();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxB();
	rayTop = getBoundingBoxC();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxC();
	rayTop = getBoundingBoxD();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxD();
	rayTop = getBoundingBoxA();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	// Side planes
	rayBot = getBoundingBoxA();
	rayTop = getBoundingBoxE();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxB();
	rayTop = getBoundingBoxF();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxC();
	rayTop = getBoundingBoxG();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxD();
	rayTop = getBoundingBoxH();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	// Top plane
	rayBot = getBoundingBoxE();
	rayTop = getBoundingBoxH();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxH();
	rayTop = getBoundingBoxG();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxG();
	rayTop = getBoundingBoxF();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	rayBot = getBoundingBoxF();
	rayTop = getBoundingBoxE();
	if( getPlaneIntersectEdge( &rayBot, &rayTop, &rayIntersect ) ) {
		planeIntersections.push_back( rayIntersect );
		centralPointPlane += rayIntersect;
	}
	if( planeIntersections.size() < 3 ) {
		// Something wrong
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Bad number of vertices: " << planeIntersections.size() << endl;
		return false;
	}

	// Get central point and draw points
	centralPointPlane /= planeIntersections.size();

	// Fetch plane normal as reference for signed angle estimation:
	Vector3D planeHNF;
	getPlaneHNF( &planeHNF );
	// Now we sort things to vertices in order:
	// Define planeIntersections[0] - centralPointPlane as 0°
	Vector3D refDirection = planeIntersections[0] - centralPointPlane;

	// LEGACY code - working.
	//vector<pair<double,Vector3D>> anglesTmp;
	//for( const auto& currPoint: planeIntersections ) {
	//	anglesTmp.push_back( pair<double,Vector3D>( angle( currPoint - centralPointPlane, refDirection, planeHNF ), currPoint ) );
	//}
	// New and improved code
	vector<pair<double,Vector3D>> anglesTmp(planeIntersections.size());
	std::transform( planeIntersections.begin(), planeIntersections.end(), anglesTmp.begin(),
	                [&centralPointPlane, &refDirection, &planeHNF](const Vector3D& currPoint) {
			return std::pair<double,Vector3D>( angle( currPoint - centralPointPlane, refDirection, planeHNF ), currPoint) ;
		}
	);

	// C++11 rocks, because of lambdas/closures - inspired by http://stackoverflow.com/a/4325449
	sort( begin(anglesTmp), end(anglesTmp),
		[] ( const pair<double,Vector3D> &lhs, const pair<double,Vector3D> &rhs ) {
			return ( lhs.first < rhs.first );
		}
	);

	// LEGACY code - working.
	//vector<Vector3D> planeIntersectionsSorted;
	//for( const auto& currAngle: anglesTmp ) {
	//	planeIntersectionsSorted.push_back( currAngle.second );
	//}
	//rFanVertices->clear();
	//planeIntersectionsSorted.swap( (*rFanVertices) );

	// New and improved code
	rFanVertices->resize(anglesTmp.size());
	std::transform( anglesTmp.begin(), anglesTmp.end(),
	                rFanVertices->begin(), [](pair<double, Vector3D>& angle ) {
			return angle.second;
		}
	);

	(*rCentralPoint) = centralPointPlane;

	return( true );
}

//! Applies transformation to the mPlanePos
bool Mesh::applyTransfromToPlane( Matrix4D rTransMat ) {
	return mPlane.applyTransfrom( &rTransMat );
}

//! Splits the mesh by the currently selected plane. This function creates
//! new vertices and faces. If faces are currently selected, the function
//! will only split selected faces.
//! @param planeHNF Hesse Normal Form of a plane
//! @param duplicateVertices If set, vertices will be duplicated along the
//!        splitting plane.
//! @param noRedraw If set, does not force a redraw after each splitting
//!        operation. Increases speed.
bool Mesh::splitByPlane( Vector3D planeHNF, bool duplicateVertices, bool noRedraw ) {

	Plane cutPlane( &planeHNF );

	//! \todo sanity checks for plane, because the following is deprecated:
	//if(!isPlanePosCSet()) {
	//	return(false);
	//}

	auto interSectFun = [&planeHNF](Face* face) { return face->intersectsPlane(&planeHNF);};

	auto distFun = [&planeHNF](VertexOfFace* vertex) { return vertex->estDistanceToPlane(&planeHNF);};

	auto getIntersectionVectorFun = [&cutPlane]( VertexOfFace* vertX, VertexOfFace* vertY, Vector3D& rVecIntersection)  {
		cutPlane.getIntersectionFacePlaneLinePos(vertX->getPositionVector(), vertY->getPositionVector(), rVecIntersection);
	};

	return splitMesh( interSectFun, distFun, getIntersectionVectorFun, duplicateVertices, noRedraw, 1000.0*numeric_limits<double>::epsilon() * planeHNF);
}

//! Calculate intersection polyline between mesh and plane (for archaeological section cutting).
//! This method does NOT modify the mesh - it only calculates intersection points.
//! Used for real-time section preview in archaeological illustration mode.
//! @param planeHNF - Plane in Hesse Normal Form (nx, ny, nz, d)
//! @param rIntersectionPoints - Output vector of intersection points (ordered to form a polyline)
//! @return true on success, false on failure
bool Mesh::calcIntersectionPolylineWithPlane( const Vector3D& planeHNF, std::vector<Vector3D>* rIntersectionPoints ) {
	if( rIntersectionPoints == nullptr ) {
		return false;
	}

	rIntersectionPoints->clear();

	// Create plane object from HNF
	Plane cutPlane( const_cast<Vector3D*>(&planeHNF) );

	// Iterate through all faces to find intersections
	for( uint64_t i = 0; i < getFaceNr(); i++ ) {
		Face* face = getFacePos( i );

		// Skip if face doesn't intersect plane
		if( !face->intersectsPlane( &planeHNF ) ) {
			continue;
		}

		// Get face vertices
		Vertex* v0 = face->getVertA();
		Vertex* v1 = face->getVertB();
		Vertex* v2 = face->getVertC();

		if( v0 == nullptr || v1 == nullptr || v2 == nullptr ) {
			continue;
		}

		Vector3D pos0 = v0->getPositionVector();
		Vector3D pos1 = v1->getPositionVector();
		Vector3D pos2 = v2->getPositionVector();

		// Check each of the three edges for intersection
		// A triangle can intersect a plane at exactly 2 points (entry and exit)
		Vector3D intersection;
		int intersectionCount = 0;

		// Edge 0-1
		if( cutPlane.getIntersectionFacePlaneLinePos( pos0, pos1, intersection ) ) {
			rIntersectionPoints->push_back( intersection );
			intersectionCount++;
		}

		// Edge 1-2
		if( cutPlane.getIntersectionFacePlaneLinePos( pos1, pos2, intersection ) ) {
			rIntersectionPoints->push_back( intersection );
			intersectionCount++;
		}

		// Edge 2-0 (only if we haven't found 2 intersections yet)
		if( intersectionCount < 2 && cutPlane.getIntersectionFacePlaneLinePos( pos2, pos0, intersection ) ) {
			rIntersectionPoints->push_back( intersection );
		}
	}

	std::cout << "[Mesh::" << __FUNCTION__ << "] Found " << rIntersectionPoints->size()
	          << " intersection points with plane" << std::endl;

	return ( rIntersectionPoints->size() >= 2 );
}

//! Splits the mesh using a threshold (Iso Value) for the vertice's function value.
//! This function creates new vertices and faces. If faces are currently selected,
//! the function will only split selected faces.
//! It is an advanced version of Mesh::splitByPlane by Bastian Rieck.
//! @param rIsoVal - Threshold.
//! @param duplicateVertices If set, vertices will be duplicated along the
//!        splitting plane.
//! @param noRedraw If set, does not force a redraw after each splitting
//!        operation. Increases speed.
bool Mesh::splitByIsoLine( double rIsoVal, bool duplicateVertices, bool noRedraw, Vector3D rUniformOffset ) {

	auto intersectFun = [rIsoVal](Face* face) { return face->isOnFuncValIsoLine( rIsoVal);};

	auto distFun = [rIsoVal](VertexOfFace* vertex) {
		double retVal;
		vertex->getFuncValue(&retVal);
		return retVal - rIsoVal;
	};

	auto getIntersectionVectorFun = [rIsoVal, this] (VertexOfFace* vertX, VertexOfFace* vertY, Vector3D& rVecIntersection)  {
		this->getPointOnIsoLine(&rVecIntersection, vertX, vertY, rIsoVal);
	};

	return splitMesh(intersectFun, distFun, getIntersectionVectorFun, duplicateVertices, noRedraw, rUniformOffset);
}

//----------------------------------
//helper functions for splitMesh
//returns relatve distance between vertA and vect in relation to the distance between vertA and vertB
double getRelativeDistance(VertexOfFace* vertA, VertexOfFace* vertB, Vector3D* vect)
{
	Vector3D a;
	vertA->getPositionVector(&a);
	Vector3D b;
	vertB->getPositionVector(&b);

	return (*vect - a).getLength3() / (b - a).getLength3();
}

void linearInterpolateUV(float s1, float t1, float s2, float t2, float factor, float& sOut, float& tOut)
{
	sOut = (1.0f - factor) * s1 + factor * s2;
	tOut = (1.0f - factor) * t1 + factor * t2;
}
//----------------------------------


bool Mesh::splitMesh(const std::function<bool(Face*)>& intersectTest , const std::function<double(VertexOfFace*)>& signedDistanceFunction, const std::function<void(VertexOfFace*, VertexOfFace*, Vector3D&)>& getIntersectionVector, bool duplicateVertices, bool noRedraw, Vector3D rUniformOffset)
{
	// TODO: Should become a unique function <--- is this TODO still relevant?
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currVertex->setIndex( vertIdx );
	}

	// If `duplicateVertices` is set, the vertex position will be
	// changed by this vector
	Vector3D vecOffset = rUniformOffset;
	vecOffset.setH( 1.0 );

	// Work with _all_ faces of the mesh and populate selected faces
		// with _all_ faces that actually intersect the plane
		Face* currFace = nullptr;
		if( mFacesSelected.size() == 0 ) {
			cout << "[Mesh::" << __FUNCTION__ << "] Processing ALL faces." << endl;
			// select all faces that are to be deleted afterwards -- they are not required anymore
			// as they will be replaced by the faces created by the splitting procedure
			for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
				currFace = getFacePos(faceIdx);
				if(intersectTest(currFace))
				{
					mFacesSelected.insert( currFace );
				}
			}
		}
		else {
			cout << "[Mesh::" << __FUNCTION__ << "] Processing only selected faces: " << mFacesSelected.size() << endl;
			// remove all non-intersecting faces from the preselected faces; this step is necessary
			// because the code below does not actually check for an intersection
			for(std::set<Face*>::iterator faceIt = mFacesSelected.begin(); faceIt != mFacesSelected.end();) {
				if(!intersectTest(*faceIt))
				{
					mFacesSelected.erase( faceIt++ );
				}
				else {
					faceIt++;
				}
			}
		}

		// required for reconnecting the _new_ faces created by
		// the splitting procedure
		std::set<Face*> adjacentAndNewFaces;

		// store edges for which an intersection vertex has already
		// been created
		std::map< std::pair<int, int>, std::pair<VertexOfFace*, VertexOfFace*> > edgeMap;

		cout << "[Mesh::" << __FUNCTION__ << "] Remaining faces: " << mFacesSelected.size() << endl;
		for(Face* face : mFacesSelected) {

			// Classify face vertices with respect to the plane; split face into
			// `front` part and `back` part

			std::vector<VertexOfFace*> front;
			std::vector<VertexOfFace*> back;
			front.reserve(4);
			back.reserve(4);

			std::vector<float> frontUVs;
			frontUVs.reserve(8);
			std::vector<float> backUVs;
			backUVs.reserve(8);

			// required so that we may loop over the vertices
			VertexOfFace* vertsToCheck[3] = { static_cast<VertexOfFace*>(face->getVertB()),
											  static_cast<VertexOfFace*>(face->getVertC()),
											  static_cast<VertexOfFace*>(face->getVertA()) };

			auto uvs = face->getUVs();

			//x index of the current uv-coordinate
			size_t uvX = 0;
			size_t uvY = 2;

			VertexOfFace* vertX  = static_cast<VertexOfFace*>(face->getVertA());
			VertexOfFace* vertY = nullptr;

			double sideX = signedDistanceFunction(vertX);
			double sideY;

			Vector3D vecIntersection;
			for(VertexOfFace*& vertexOfFace : vertsToCheck) {

				// These vertices store the position of the points _on_ the plane. If `duplicateVertices`
				// is set to true, `vertIntersection2` will contain a slightly offset version of
				// `vertIntersection1`. Else, this parameter will _not_ be used.
				VertexOfFace* vertIntersection1 = nullptr;
				VertexOfFace* vertIntersection2 = nullptr;

				// required for reconnecting the faces later on
				vertexOfFace->getFaces(&adjacentAndNewFaces);

				vertY = vertexOfFace;
				sideY = signedDistanceFunction(vertexOfFace);

				// Check whether edge exists; if this is the case, use the stored vertex as
				// the intersection vertex and do _not_ store the vertex twice.

				bool edgeExists = false;

				int id1 = vertX->getIndex();
				int id2 = vertY->getIndex();
				if(id1 > id2) {
					std::swap(id1, id2);
				}

				std::map< std::pair<int, int>, std::pair<VertexOfFace*, VertexOfFace*> >::iterator edgeIt;
				if((edgeIt = edgeMap.find(std::make_pair(id1, id2))) != edgeMap.end()) {
					vertIntersection1 = edgeIt->second.first; // Cool notation ;)
					vertIntersection2 = edgeIt->second.second;
					edgeExists = true;
				}

				unsigned char averageColor[3];
				averageColor[0] = static_cast<unsigned char>(0.5*(vertX->getR() + vertY->getR()));
				averageColor[1] = static_cast<unsigned char>(0.5*(vertX->getG() + vertY->getG()));
				averageColor[2] = static_cast<unsigned char>(0.5*(vertX->getB() + vertY->getB()));


				//the vertices are on the other sides
				if( sideY != 0)
				{
					if(sideY > 0 != sideX > 0)
					{
						getIntersectionVector(vertX, vertY, vecIntersection);

						if(duplicateVertices) {
							if(!vertIntersection1 && !vertIntersection2) {
								vertIntersection1 = new VertexOfFace(vecIntersection + vecOffset);
								vertIntersection2 = new VertexOfFace(vecIntersection - vecOffset);

								edgeMap[std::make_pair(id1, id2)] = std::make_pair(vertIntersection1, vertIntersection2);
							}

							front.push_back(vertIntersection1);
							back.push_back( vertIntersection2);
						}
						else {
							// Vertex at intersection may have already been found;
							// do _not_ create vertex twice
							if(!vertIntersection1) {
								vertIntersection1 = new VertexOfFace(vecIntersection);
								edgeMap[std::make_pair(id1, id2)] = std::make_pair(vertIntersection1, vertIntersection1);
							}

							front.push_back(vertIntersection1);
							back.push_back( vertIntersection1);
						}

						//push interpolated uvs to front and back
						float relDist = getRelativeDistance(vertY, vertX, &vecIntersection);
						float s;
						float t;

						linearInterpolateUV(uvs[uvY], uvs[uvY + 1], uvs[uvX], uvs[uvX + 1], relDist, s, t);

						frontUVs.push_back(s); frontUVs.push_back(t);
						backUVs.push_back( s); backUVs.push_back( t);
					}
					//y is in front
					if(sideY > 0)
					{
						front.push_back(vertY);
						frontUVs.push_back(uvs[uvY]); frontUVs.push_back(uvs[uvY + 1]);
					}
					//y is in back
					else
					{
						back.push_back(vertY);
						backUVs.push_back( uvs[uvY]); backUVs.push_back( uvs[uvY + 1]);
					}
				}

				else
				{
					std::cerr << "[Mesh::" << __FUNCTION__ << "] Handling vertex _on_ plane" << std::endl;

					// slightly shift vertex; variable names are not entirely correct, but will not
					// be changed as code afterwards depends on it
					if(duplicateVertices) {
						vertIntersection1 = new VertexOfFace(vertY->getPositionVector() + vecOffset);
						vertIntersection2 = new VertexOfFace(vertY->getPositionVector() - vecOffset);

						front.push_back(vertIntersection1);
						back. push_back(vertIntersection2);
					}
					// if no duplicates are required, store vertex in _both_ sets
					else {
						front.push_back(vertY);
						back. push_back(vertY);
					}

					//push uvs of y to front and back
					frontUVs.push_back(uvs[uvY]); frontUVs.push_back(uvs[uvY + 1]);
					backUVs. push_back(uvs[uvY]); backUVs. push_back(uvs[uvY + 1]);
				}

				// Set attributes for new vertices

				if(vertIntersection1 && !edgeExists) {
					vertIntersection1->setRGB(averageColor[0], averageColor[1], averageColor[2]);
					mVertices.push_back(vertIntersection1);
				}
				if(vertIntersection2 && !edgeExists) {
					vertIntersection2->setRGB(averageColor[0], averageColor[1], averageColor[2]);
					mVertices.push_back(vertIntersection2);
				}

				vertX = vertY;
				uvX = uvY;
				uvY = (uvY + 2) % 6;
				sideX = sideY;
			}

			triangulateSplitFace(front, &adjacentAndNewFaces, &frontUVs, face->getTextureId());
			triangulateSplitFace(back , &adjacentAndNewFaces, &backUVs , face->getTextureId());
		}

		for(Face* face : mFacesSelected) {
			adjacentAndNewFaces.erase(face);
		}

		if(noRedraw) {
			Mesh::removeFacesSelected();
		}
		else {
			this->removeFacesSelected();
		}

		// Re-establish mesh
		for(Face* adjacentAndNewFace : adjacentAndNewFaces) {
			adjacentAndNewFace->reconnectToFaces();
		}

		// Apply other nessary methods
		bool retVal = true;
		retVal &= changedMesh();

		return( retVal );
}

Plane::ePlaneDefinedBy Mesh::getPlaneDefinition()
{
	return mPlane.getDefinedBy();
}

//! Triangulates a face that has been split by a plane. The new faces are then inserted into a
//! set if the parameter is not NULL.
//! @warning This function assumes that points are added to the vector of vertices in the correct
//! order. There is no way for this function to determine/change the order.
bool Mesh::triangulateSplitFace(std::vector<VertexOfFace*>& faceVertices, std::set<Face*>* newFaces, std::vector<float>* newUVS, unsigned char textureID) {

	size_t n = faceVertices.size();
	if(n != 3 && n !=4) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] Encountered result of non-triangular split. Unable to triangulate. n=" << n
		          << std::endl;
		return(false);
	}

	if(newUVS)
	{
		if(faceVertices.size() * 2 != newUVS->size())
		{
			std::cerr << "[Mesh::" << __FUNCTION__ << "] The number of UV's does not match with the number of vertices * 2: " << newUVS->size() << " vs " << faceVertices.size() << std::endl;
			return false;
		}
	}

	int faceId = getFaceNr();

	Face* newFace1 = nullptr;
	Face* newFace2 = nullptr;

	// this face is created in any case
	newFace1 = new Face(faceId, faceVertices[0], faceVertices[1], faceVertices[2]);
	if(newUVS)
	{
		std::array<float,6> uvs;
		for(int i = 0; i<6; ++i)
			uvs[i] = (*newUVS)[i];

		newFace1->setUVs(uvs);
		newFace1->setTextureId(textureID);
	}

	mFaces.push_back(newFace1);

	// this face can only be created if four points are the result of
	// the splitting plane operation
	if(n == 4) {
		newFace2 = new Face(faceId + 1, faceVertices[2], faceVertices[3], faceVertices[0]);
		if(newUVS)
		{
			std::array<float,6> uvs;
			for(int i = 0; i<6; ++i)
				uvs[i] = (*newUVS)[(4 + i) % 8];

			newFace2->setUVs(uvs);
			newFace2->setTextureId(textureID);
		}
		mFaces.push_back(newFace2);
	}

	if(newFaces) {
		newFaces->insert(newFace1);
		if(newFace2) {
			newFaces->insert(newFace2);
		}
	}

	return(true);
}

// Cone stuff ---------------------------------------------------------------------------------------------

//! Sets axis of cone (specified by upper and lower point).
bool Mesh::setConeAxis( const Vector3D& upper, const Vector3D& lower ) {
// o:	mCenteredAroundCone = false;

	mConeAxisPoints[0].set(upper);
	mConeAxisPoints[1].set(lower);

	setConeStatus(CONE_DEFINED_AXIS);
	return(true);
}

//! Sets next radius of cone (either upper or lower)
bool Mesh::setConeRadius(Vector3D& p)
{
	if(getConeStatus() == CONE_UNDEFINED)
		return(false);

	// Find point on axis that is perpendicular to the selected point

	Vector3D a = mConeAxisPoints[0];
	Vector3D b = mConeAxisPoints[1];

	double l = (b-a).getLength3();   // axis length
	double t = dot3(p-a, b-a)/(l*l); // yields parameter for line between a and b that specifies the requested point

	// Calculate point on axis and _change_ current axis; store outer point
	// for visualization
	Vector3D pointOnAxis             = a + (b-a)*t;
	mConeAxisPoints[mConeRadiusIdx]  = pointOnAxis;

	// Cone radius is simply the distance of p from the _current_ axis
	mConeRadius[mConeRadiusIdx] = p.distanceToLine( &mConeAxisPoints[0], &mConeAxisPoints[1] );

	if(mConeRadiusIdx == 0)
		setConeStatus(CONE_DEFINED_UPPER_RADIUS);
	else {
		setConeStatus(CONE_DEFINED_LOWER_RADIUS);
	}

	mConeRadiusIdx++;
	mConeRadiusIdx %= 2;

	return(true);
}

//! Sets radii of the cone. Uses the currently selected axis to compute points on the
//! outer side of the cone.
bool Mesh::setConeRadius(double upperRadius, double lowerRadius) {

	coneStates coneStatus = getConeStatus();
	if(coneStatus < CONE_DEFINED_AXIS || coneStatus == CONE_UNROLLED) {
		return(false);
	}

	setConeOuterPoint( upperRadius );
	setConeOuterPoint( lowerRadius );

	return(true);
}

//! Sets an outer point of the cone. The point is placed perpendicular to the cone
//! axis and will have a distance of `radius` to the axis. The `referencePoint` is
//! to place the calculated point at the proper height relative to the cone axis.
void Mesh::setConeOuterPoint( double rRadius ) {
	mConeRadius[mConeRadiusIdx]      = rRadius;

	if(mConeRadiusIdx == 0) {
		setConeStatus(CONE_DEFINED_UPPER_RADIUS);
	} else {
		setConeStatus(CONE_DEFINED_LOWER_RADIUS);
	}

	mConeRadiusIdx++;
	mConeRadiusIdx %= 2;
}

//! Extend the cone to contain the whole mesh.
//!
//! @returns false in case of an error. True otherwise
bool Mesh::setConeCoverMesh() {
	setConeConsistentPointOrder();

	Vector3D coneTip;
	this->calcConeTip( &coneTip );

	Vector3D coneAxis = mConeAxisPoints[0] - mConeAxisPoints[1];
	coneAxis.normalize3();

	Plane coneTipPlane( coneTip, coneAxis );

	double minDist = +INFINITY;
	double maxDist = -INFINITY;
	bool   absDist = true; // Works with the order of points ensured (setConeConsistentPointOrder).
	getDistanceToPlaneMinMax( coneTipPlane, minDist, maxDist, absDist );

	double angleCone;
	this->calcConeAngle( &angleCone );

	mConeAxisPoints[0] = coneTip + coneAxis * -minDist;
	mConeAxisPoints[1] = coneTip + coneAxis * -maxDist;
	mConeRadius[0] =     minDist * tan( angleCone );
	mConeRadius[1] =     maxDist * tan( angleCone );

	setConeConsistentPointOrder();

	// Provide some (debug) information:
	getConeInfoDump( __FUNCTION__ );

	return( true );
}


//! Ensures that the cone axis point #0 has the smallest radius i.e. is closest to the cone's tip.
//!
//! @returns false in case of an error. True otherwise
bool Mesh::setConeConsistentPointOrder() {
	if( mConeRadius[0] > mConeRadius[1] ) {
		cout << "[Mesh::" << __FUNCTION__ << "] WARNING: Order of cone points will be swaped!" << endl;
		// Swap points and radii:
		std::swap( mConeRadius[0],     mConeRadius[1]     );
		std::swap( mConeAxisPoints[0], mConeAxisPoints[1] );
	}
	return( true );
}

//! Dump cone information.
//!
//! @returns false in case of an error. True otherwise
bool Mesh::getConeInfoDump( const string& rCallingFunc ) {
	double coneAngle;
	if( !this->calcConeAngle( &coneAngle ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: calcConeAngle - Called by " << rCallingFunc << "!" << endl;
		return( false );
	}

	Vector3D coneTip;
	if( !this->calcConeTip( &coneTip ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: calcConeTip - Called by " << rCallingFunc << "!" << endl;
		return( false );
	}

	double distTipTo0 = ( mConeAxisPoints[0] - coneTip ).getLength3();
	double distTipTo1 = ( mConeAxisPoints[1] - coneTip ).getLength3();

	cout << "[Mesh::" << rCallingFunc << "] ----------------------------------------------- " << endl;
	cout << "[Mesh::" << rCallingFunc << "] Cone Angle: " << coneAngle << " in degree " << coneAngle*180.0/M_PI << endl;
	cout << "[Mesh::" << rCallingFunc << "] Cone Tip: " << mConeAxisPoints[0] << endl;
	cout << "[Mesh::" << rCallingFunc << "] Axis point #0: " << mConeAxisPoints[0] << endl;
	cout << "[Mesh::" << rCallingFunc << "] Axis point #1: " << mConeAxisPoints[1] << endl;
	cout << "[Mesh::" << rCallingFunc << "] Radius #0: " << mConeRadius[0] << endl;
	cout << "[Mesh::" << rCallingFunc << "] Radius #1: " << mConeRadius[1] << endl;
	cout << "[Mesh::" << rCallingFunc << "] Distance #0: " << distTipTo0 << endl;
	cout << "[Mesh::" << rCallingFunc << "] Distance #1: " << distTipTo1 << endl;
	cout << "[Mesh::" << rCallingFunc << "] ----------------------------------------------- " << endl;

	return( true );
}

//! Sets current status of cone.
//! Asks the user if the cone shall be extended to cover the whole mesh.
void Mesh::setConeStatus( coneStates rConeStatus ) {
	mConeStatus = rConeStatus;
	if( rConeStatus == CONE_DEFINED_LOWER_RADIUS ) {
		setConeConsistentPointOrder();
		// Provide some (debug) information:
		getConeInfoDump( __FUNCTION__ );
		// Extend the cone, when nessary.
		bool coverMesh;
		if( showQuestion( &coverMesh, "Cone - Cover mesh", "Extend the cone to cover the whole mesh?" ) ) {
			if( coverMesh ) {
				callFunction( CONE_COVER_MESH );
			}
		} else {
			// User cancel
			return;
		}
	}
}

//! Returns current status of cone. Depending on the status, different objects
//! will be drawn in the main window of GigaMesh.
Mesh::coneStates Mesh::getConeStatus()
{
	return(mConeStatus);
}

//! Returns the two position vectors defining the (cone) axis.
//!
//! @returns true only, when the axis is properly defined by two different position vectors. Returns also false in case of an error occured.
bool Mesh::getConeAxis( Vector3D* rAxisTop, Vector3D* rAxisBottom ) {
	// Sanity check
	if( ( rAxisTop == nullptr ) || ( rAxisBottom == nullptr ) ) {
		return( false );
	}
	// The axis is only defined if the length of the orientation is a normal value (non-zero, finite, etc.)
	Vector3D orient = mConeAxisPoints[0] - mConeAxisPoints[1];
	if( !isnormal( orient.getLength3() ) ) {
		return( false );
	}
	// Return the position vectors:
	*rAxisTop    = mConeAxisPoints[0];
	*rAxisBottom = mConeAxisPoints[1];
	return( true );
}

//! Returns the bottom position vector and the orientation vector defining the (cone) axis.
//!
//! @returns true only, when the axis is properly defined by two different position vectors. Returns also false in case of an error occured.
bool Mesh::getConeAxisPosDir( Vector3D* rAxisBottom, Vector3D* rAxisOrient ) {
	// Sanity check
	if( ( rAxisBottom == nullptr ) || ( rAxisOrient == nullptr ) ) {
		return( false );
	}
	// The axis is only defined if the length of the orientation is a normal value (non-zero, finite, etc.)
	Vector3D orient = mConeAxisPoints[0] - mConeAxisPoints[1];
	if( !isnormal( orient.getLength3() ) ) {
		return( false );
	}
	// Return the position vectors:
	*rAxisBottom = mConeAxisPoints[0];
	*rAxisOrient = orient;
	return( true );
}

//! Check if the (cone) axis is defined.
//!
//! @returns true, if the cone axis is properly defined by two positon vectors.
bool Mesh::getConeAxisDefined() {
	// The axis is only defined if the length of the orientation is a normal value (non-zero, finite, etc.)
	Vector3D orient = mConeAxisPoints[0] - mConeAxisPoints[1];
	return( isnormal( orient.getLength3() ) );
}

//! Returns the two points defining the cone' axis's zero meridian.
//! When rOppsoite is true, than cone' axis's 180° meridian is returned.
//!
//! @returns true only, when both points were assigned.
bool Mesh::getConeMeridianZero( Vector3D* rMeridianTop, Vector3D* rMeridianBottom, bool rOppsoite ) {
	if( ( rMeridianTop == nullptr ) || ( rMeridianBottom == nullptr ) ) {
		return false;
	}
	if( getConeStatus() != CONE_DEFINED_LOWER_RADIUS ) {
		return false;
	}

	// Setup transformation matrix
	Vector3D coneAxis = mConeAxisPoints[0] - mConeAxisPoints[1];
	Matrix4D matTransform( Matrix4D::INIT_IDENTITY );
	matTransform.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Y, &mConeAxisPoints[1], &coneAxis );

	// Compute outer points:
	Vector3D meridianTop    = mConeAxisPoints[0] * matTransform;
	Vector3D meridianBottom = mConeAxisPoints[1] * matTransform;

	// Fetch rotation
	double primeMeridian;
	getParamFloatMesh( AXIS_PRIMEMERIDIAN, &primeMeridian );
	// Opposite i.e. 180° meridian:
	if( rOppsoite ) {
		primeMeridian += M_PI;
	}
	vector<double> angle { -primeMeridian };
	Matrix4D rotPrime( Matrix4D::INIT_ROTATE_ABOUT_Y, &angle );
	Vector3D vecUpperPoint( mConeRadius[0], 0.0, 0.0, 0.0 );
	Vector3D vecLowerPoint( mConeRadius[1], 0.0, 0.0, 0.0 );

	// Rotate using prime meridian angle and move back:
	matTransform.invert();
	*rMeridianTop    = ( meridianTop +    ( vecUpperPoint * rotPrime ) ) * matTransform;
	*rMeridianBottom = ( meridianBottom + ( vecLowerPoint * rotPrime ) ) * matTransform;
	return true;
}

//! Returns the two radii defining the cone.
//! @returns true only, when radii points were assigned.
bool Mesh::getConeRadii( double* rUpperRadius, double* rLowerRadius ) {
	if( ( rUpperRadius == nullptr ) || ( rLowerRadius == nullptr ) ) {
		return false;
	}
	if( getConeStatus() != CONE_DEFINED_LOWER_RADIUS ) {
		return false;
	}

	*rUpperRadius = mConeRadius[0];
	*rLowerRadius = mConeRadius[1];
	return true;
}

//! Centers all vertices around the cone described by the user. The cone is
//! made the new y axis.
//! @param resetNormals Forces reset of normals. Default behaviour.
bool Mesh::centerAroundCone( bool rResetNormals ) {
	if( this->getConeStatus() != CONE_DEFINED_LOWER_RADIUS ) {
		return( false );
	}

	// Setup transformation matrix to align the cone
	Vector3D coneAxis = mConeAxisPoints[0] - mConeAxisPoints[1];
	Matrix4D matTransform( Matrix4D::INIT_IDENTITY );
	matTransform.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Y, &mConeAxisPoints[1], &coneAxis );
	this->applyTransformationToWholeMesh( matTransform, rResetNormals );

	// Rotate the prime meridian into place
	double primeMeridian;
	getParamFloatMesh( AXIS_PRIMEMERIDIAN, &primeMeridian );
	vector<double> angle { primeMeridian };
	Matrix4D rotPrime( Matrix4D::INIT_ROTATE_ABOUT_Y, &angle );
	this->applyTransformationToWholeMesh( rotPrime, rResetNormals );
	setParamFloatMesh( AXIS_PRIMEMERIDIAN, 0.0 );

	// Create a translation matrix so that the cone tip is translated into the origin.
	Vector3D coneTip;
	this->calcConeTip( &coneTip );
	vector<double> translate { coneTip.getX(), -coneTip.getY(), coneTip.getZ() };
	Matrix4D matTransLate( Matrix4D::INIT_TRANSLATE, &translate );
	bool retVal;
	retVal = this->applyTransformationToWholeMesh( matTransLate, rResetNormals );

	// Check if the cone is properly positioned i.e. with the tip down!
	if( mMinY < 0.0 ) { // Tip Up
		// showWarning( "Tip Up", "Tip Up" );
		vector<double> axisFlip { -1.0, -1.0, 1.0 }; // Flip mesh
		Matrix4D matFlip( Matrix4D::INIT_SKEW, &axisFlip );
		retVal &= this->applyTransformationToWholeMesh( matFlip, rResetNormals );
	} else { // quick and dirty fix to set the meridian
		vector<double> angle180 { M_PI };
		Matrix4D rot180( Matrix4D::INIT_ROTATE_ABOUT_Y, &angle180 );
		retVal &= this->applyTransformationToWholeMesh( rot180, rResetNormals );
	}

	return( retVal );
}

//! Compute the position of the tip of the cone.
bool Mesh::calcConeTip( Vector3D* rConeTip ) {
	bool coneTipDown = (mConeRadius[0] > mConeRadius[1]);
	Vector3D coneAxis = mConeAxisPoints[0] - mConeAxisPoints[1]; // _direction_ of axis
	coneAxis.normalize3(); // otherwise `coneHeight` would not be used correctly
	                       // in the equation below
	double coneHeight;
	calcConeHeigth( &coneHeight );
	if( coneTipDown ) {
		// start from upper axis point and go _downwards_, i.e. in the negative
		// direction of the axis vector determined above
		rConeTip->set( mConeAxisPoints[0] - coneAxis*coneHeight );
	} else {
		// start from lower axis point and go _upwards_, i.e. in the positive
		// direction of the axis vector determined above
		rConeTip->set( mConeAxisPoints[1] + coneAxis*coneHeight );
	}
	return true;
}

//! Compute the height of the cone frustum.
bool Mesh::calcConeHeigth( double* rHeight ) {
	Vector3D coneAxis = mConeAxisPoints[0] - mConeAxisPoints[1]; // _direction_ of axis
	double coneAngle = 0.0;
	calcConeAngle( &coneAngle );
	if( coneAngle != 0 ) {	// proper cone
		(*rHeight) = std::max( mConeRadius[0], mConeRadius[1] ) / tan( coneAngle );
	}
	else {			// degenerated cone = cylinder
		(*rHeight) = coneAxis.getLength3();
	}
	return true;
}

//! Compute the angle of the cone.
bool Mesh::calcConeAngle( double* rAngle ) {
	Vector3D coneAxis = mConeAxisPoints[0] - mConeAxisPoints[1]; // _direction_ of axis
	(*rAngle)   = fabs(atan(fabs(mConeRadius[0] - mConeRadius[1]) / coneAxis.getLength3()));
	//cout << "[Mesh::" << __FUNCTION__ << "] Cone angle is: " << mConeAngle*180.0/M_PI << endl;
	return true;
}

//! Unrolls the mesh around the cone. Requires that the mesh is already centered
//! around the cone. If this is not the case, the mesh is centered.
//!
//! Presets the function value for profile line extraction.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::unrollAroundCone( bool* rIsCylinderCase ) {
	// Sanity checks
	if( this->getConeStatus() != CONE_DEFINED_LOWER_RADIUS ) {
		return( false );
	}
	if( rIsCylinderCase == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!"  << endl;
		return( false );
	}

	// Determine processing time - START
	using namespace std::chrono;
	high_resolution_clock::time_point tStart = high_resolution_clock::now();

	bool resetNormals = false;
	this->centerAroundCone( resetNormals );

	// The rotation is already fixed in centerAroundCone.
	double primeMeridian = 0.0;

	// height of the truncated cone with respect to the
	// y axis -- this uses the fact that centreAroundCone()
	// transforms the axis points
	double y0 = mConeAxisPoints[0].getY();
	double y1 = mConeAxisPoints[1].getY();

	bool noRedraw          = true;
	bool duplicateVertices = true;
	// Split at upper and lower plane -- this ensures that the
	// cut does not contain any rough edges.
	this->splitByPlane( Vector3D( 0.0, 1.0, 0.0, -y0 ), duplicateVertices, noRedraw);
	this->splitByPlane( Vector3D( 0.0, 1.0, 0.0, -y1 ), duplicateVertices, noRedraw);
	// "Open" the mesh at the side where the cone will be unrolled later on. This is done
	// in two steps:
	//
	// 1) Select "left" side of the mesh (when being cut along the yz-plane)
	// 2) Split  "left" side of the mesh along xy-plane
	this->selectFacesOppositeToPlane( cos(primeMeridian), 0.0, -sin(primeMeridian), 0.0 );
	this->splitByPlane( Vector3D( sin(primeMeridian), 0.0, cos(primeMeridian), 0.0 ), duplicateVertices, noRedraw );

	// Fetch the cone's angle
	double coneAngle = 0.0;
	calcConeAngle( &coneAngle );
	// and return the info about cone or cylinder
	(*rIsCylinderCase) = ( coneAngle == 0.0 );

	// project all vertices
	Vertex* currVertex;
	std::set<Vertex*> notUnrolled;
	// cout << "[Mesh::" << __FUNCTION__ << "] Cone Angle: " << mConeAngle * 180.0 / M_PI << endl;
	for( uint64_t vertIdx = 0; vertIdx < getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos(vertIdx);
		bool res = currVertex->unrollAroundCone( y0, y1, coneAngle, primeMeridian );
		// stores vertices that could not be unrolled
		if(!res) {
			notUnrolled.insert(currVertex);
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Vertices not unrolled: " << notUnrolled.size() << endl;

	// Clean-Up: remove intermediate selection.
	mFacesSelected.clear();
	selectedMFacesChanged();
	deSelMVertsAll();

	Mesh::removeVertices(&notUnrolled);
	setConeStatus(CONE_UNROLLED); // this disables cone selection and stops drawing the cone

	// Determine processing time - STOP
	high_resolution_clock::time_point tEnd = high_resolution_clock::now();
	duration<double> time_span = duration_cast<duration<double>>( tEnd - tStart );
	cout << "[Mesh::" << __FUNCTION__ << "] unrollAroundCone:" << time_span.count() << " seconds." << std::endl;

	// Position the mesh plane to compute the distance to the cone:
	Vector3D newMeshPlane( 0.0, 0.0, 1.0, 0.0 );
	setPlaneHNF( &newMeshPlane );

	// Remove the axis and set the function value for profile extraction.
	if( (*rIsCylinderCase) ) {
		// Degenerated case i.e. cylinder
		mConeAxisPoints[0].set( _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_ );
		mConeAxisPoints[1].set( _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_ );
		// Preset the function value for profile extraction.
		Vector3D profileCutPlane( 1.0, 0.0, 0.0, 0.0 );
		funcVertDistanceToPlane( profileCutPlane, false );
	} else {
		mConeAxisPoints[0].set( 0.0, 0.0, +1.0, 1.0 );
		mConeAxisPoints[1].set( 0.0, 0.0, -1.0, 1.0 );
		// Compute average distance to prepare additional cylinder rollout to
		// optionally straigthen the arc-shape of the rollout.
		setVertFuncValDistanceToLinePosDir( &mConeAxisPoints[0], &mConeAxisPoints[1] );
		double cylinderRadius = _NOT_A_NUMBER_DBL_;
		getVertFuncValAverage( &cylinderRadius );
		//Vector3D cogPos = getCenterOfGravity();
		//double cylinderRadius = cogPos.distanceToLine( &mConeAxisPoints[0], &mConeAxisPoints[1] );
		setParamFloatMesh( CYLINDER_RADIUS, cylinderRadius );
		// Preset the function value for profile extraction.
		setVertFuncValAngleBasedOnAxis( &mConeAxisPoints[0], &mConeAxisPoints[1] );
	}

	// In the case of cylinder, the object will already be rotated correctly. In the case
	// of a cone, we need to rotate it.
	if( !(*rIsCylinderCase) ) {
		Matrix4D rotMatrix(Vector3D(0.0, 0.0, 0.0, 1.0), Vector3D(0.0, 0.0, 1.0, 0.0), 0.5*M_PI);
		this->applyTransformationToWholeMesh( rotMatrix, resetNormals );
	}

	// Else, object is drawn facing in wrong direction
	else {
		Matrix4D rotMatrix(Vector3D(0.0, 0.0, 0.0, 1.0), Vector3D(0.0, 1.0, 0.0, 0.0), M_PI);
		this->applyTransformationToWholeMesh( rotMatrix, resetNormals );
	}

	// Re-compute the normal vectors:
	resetFaceNormals();
	resetVertexNormals();

	// Re-compute the bounding box:
	estBoundingBox();

	return( true );
}

//! Unrolls the mesh around an infinite cylinder with a given radius.
//! Requires that the mesh is already centered around the axis.
//! If this is not the case, the mesh is centered.
bool Mesh::unrollAroundCylinderRadius() {
	bool noRedraw          = true;
	bool duplicateVertices = true;
	bool resetNormals      = false;

	Vector3D topAxis = mConeAxisPoints[0];
	Vector3D botAxis = mConeAxisPoints[1];
	Vector3D axisDir = topAxis-botAxis;

	double primeMeridian;
	getParamFloatMesh( AXIS_PRIMEMERIDIAN, &primeMeridian );

	Vector3D origin( 0.0, 0.0, 0.0, 1.0 );
	Vector3D yAxis( 0.0, 1.0, 0.0, 0.0 );

	// Change the base so that the rotational axis equals the y-axis
	// as expected by Vertex::unrollAroundCylinderRadius.
	Matrix4D baseChange;
	baseChange.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Y, &botAxis, &axisDir );
	Matrix4D rotMeridian( origin, yAxis, -primeMeridian-M_PI ); //  + M_PI
	baseChange *= rotMeridian;
	this->applyTransformationToWholeMesh( baseChange, resetNormals );
	// Prime Merdian becomes now 0.0 - set for consistency:
	primeMeridian = 0.0;
	setParamFloatMesh( AXIS_PRIMEMERIDIAN, primeMeridian );

	Vertex* currVertex;
	mFacesSelected.clear();
	for( uint64_t vertIdx = 0; vertIdx < getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos(vertIdx);
		currVertex->setFuncValue( currVertex->getX() );
		if( currVertex->getZ() <= 0.0 ) {
			currVertex->getFaces( &mFacesSelected );
		}
	}
	double uniformOffset = 100.0*numeric_limits<double>::epsilon();
	splitByIsoLine( 0.0, duplicateVertices, noRedraw, Vector3D( uniformOffset, 0.0, 0.0, 0.0 ) );

	// Fetch given radius
	double cylinderRadius;
	getParamFloatMesh( CYLINDER_RADIUS, &cylinderRadius );

	// transform all vertices
	for( uint64_t vertIdx = 0; vertIdx < getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos(vertIdx);
		currVertex->unrollAroundCylinderRadius( cylinderRadius, primeMeridian ); // bool res = ... | primeMeridian IS 0.0
	}

	// Position the mesh plane to compute the distance to the cone:
	Vector3D newMeshPlane( 0.0, 0.0, 1.0, 0.0 );
	setPlaneHNF( &newMeshPlane );

	// Remove the axis
	mConeAxisPoints[0].set( _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_ );
	mConeAxisPoints[1].set( _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_, _NOT_A_NUMBER_DBL_ );

	// Sst the function value for profile extraction.
	Vector3D profileCutPlane( 1.0, 0.0, 0.0, 0.0 );
	bool absoluteDistance = false;
	funcVertDistanceToPlane( profileCutPlane, absoluteDistance );

	// Reset selections
	mFacesSelected.clear();
	mSelectedMVerts.clear();
	selectedMFacesChanged();
	selectedMVertsChanged();
	changedVertFuncVal();

	// Re-compute the normal vectors:
	resetFaceNormals();
	resetVertexNormals();

	// Re-compute the bounding box:
	estBoundingBox();

	return true;
}

// Sphere stuff -------------------------------------------------------------------------------------------

//! Sets next point for describing the sphere. Once all four points
//! have been set, sphere data is calculated.
bool Mesh::setSpherePoint(Vector3D &p, Vector3D &normal) {

	mSphereRadius                   = 0.0; // always reset so that no sphere is drawn during the selection process
	mSpherePoints[mSpherePointsIdx] = p;
	mSpherePointNormals[mSpherePointsIdx] = normal;

	if(mSpherePointsIdx == 3) {

		// Calculate sphere data -- this is not aesthetic

		double x[4];
		double y[4];
		double z[4];
		double l[4]; // length of vector describing ith sphere point

		for(int i = 0; i < 4; i++) {
			x[i] = mSpherePoints[i].getX();
			y[i] = mSpherePoints[i].getY();
			z[i] = mSpherePoints[i].getZ();
			l[i] = mSpherePoints[i].getLength3()*mSpherePoints[i].getLength3();
		}

		// Prepare appropriate matrices; notation follows Paul Bourke, see
		// http://paulbourke.net/geometry/spherefrom4.

		Matrix4D M11;
		Matrix4D M12;
		Matrix4D M13;
		Matrix4D M14;
		Matrix4D M15;

		for(int i = 0; i < 4; i++) {
			M11(i, 0) = x[i];
			M11(i, 1) = y[i];
			M11(i, 2) = z[i];
			M11(i, 3) = 1.0;

			M12(i, 0) = l[i];
			M12(i, 1) = y[i];
			M12(i, 2) = z[i];
			M12(i, 3) = 1.0;

			M13(i, 0) = l[i];
			M13(i, 1) = x[i];
			M13(i, 2) = z[i];
			M13(i, 3) = 1.0;

			M14(i, 0) = l[i];
			M14(i, 1) = x[i];
			M14(i, 2) = y[i];
			M14(i, 3) = 1.0;

			M15(i, 0) = l[i];
			M15(i, 1) = x[i];
			M15(i, 2) = y[i];
			M15(i, 3) = z[i];
		}

		double det11 = M11.getDeterminant();
		double det12 = M12.getDeterminant();
		double det13 = M13.getDeterminant();
		double det14 = M14.getDeterminant();
		double det15 = M15.getDeterminant();

		// Coplanar or three points are colinear...
		if(det11 == 0.0) {
			return(false);
		}

		mSphereCenter.setX( 0.5*det12/det11);
		mSphereCenter.setY(-0.5*det13/det11);
		mSphereCenter.setZ( 0.5*det14/det11);
		mSphereRadius = sqrt(mSphereCenter.getLength3()*mSphereCenter.getLength3() - det15/det11);

		// Reset index; allows selection of another sphere afterwards
		mSpherePointsIdx = 0;
	}

	// Normal case; store next point
	else {
		// delete _old_ points by collapsing them to the
		// first selected point
		if(mSpherePointsIdx == 0) {
			mSpherePoints[1] = mSpherePoints[2] = mSpherePoints[3] = mSpherePoints[0];
			mSpherePointNormals[1] = mSpherePointNormals[2] = mSpherePointNormals[3] = mSpherePointNormals[0];
		}

		mSpherePointsIdx++;
	}

	return(true);
}

//! Returns relevant sphere data, i.e. a vector describing the center
//! of the sphere and the sphere radius.
bool Mesh::getSphereData(Vector3D *center, double *radius) {
	if(!center || !radius || mSphereRadius == 0.0) {
		return(false);
	}

	*center = this->mSphereCenter;
	*radius = this->mSphereRadius;

	return(true);
}

//! Returns the index of the current point to be set for a sphere defined by four points and used for unwrapping.
int Mesh::getSpherePointIdx() {
	return mSpherePointsIdx;
}

//! Returns the radius of the sphere used for unwrapping.
double Mesh::getSphereRadius() {
	return mSphereRadius;
}

//! Returns points _on_ sphere. This is used for visualization
//! purposes.
//! @warning Function does not care about uninitialized or old
//!          values!
bool Mesh::getSphereData(double* spherePoints, double *sphereNormals) {
	if(!spherePoints) {
		return(false);
	}

	mSpherePoints[0].get3(&spherePoints[0]);
	mSpherePoints[1].get3(&spherePoints[3]);
	mSpherePoints[2].get3(&spherePoints[6]);
	mSpherePoints[3].get3(&spherePoints[9]);

	if(sphereNormals != nullptr)
	{
		mSpherePointNormals[0].get3(&sphereNormals[0]);
		mSpherePointNormals[1].get3(&sphereNormals[3]);
		mSpherePointNormals[2].get3(&sphereNormals[6]);
		mSpherePointNormals[3].get3(&sphereNormals[9]);
	}

	return(true);
}

//! Returns status of mesh unrolling.
bool Mesh::isUnrolledAroundSphere() {
	return(mUnrolledAroundSphere);
}

//! Centers the mesh around the sphere. This is simply done by
//! setting the new origin to be the center of the sphere.
bool Mesh::centerAroundSphere() {
	if(mSphereRadius == 0.0) {
		return(false);
	}

	Matrix4D matTransformation(mSphereCenter);
	if( !applyTransformationToWholeMesh( matTransformation, true ) ) {
		return( false );
	}

	// Apply transformation to points _on_ the sphere, too. This ensures
	// that they are still drawn correctly.

	for(Vector3D& mSpherePoint : mSpherePoints) {
		mSpherePoint.applyTransformation(matTransformation);
	}

	mSphereCenter.applyTransformation(matTransformation);
	mCenteredAroundSphere = true;
	return(true);
}

//! Unrolls the mesh around the user-specified sphere.
bool Mesh::unrollAroundSphere() {
	if(mSphereRadius == 0.0) {
		return(false);
	}

	if(!mCenteredAroundSphere) {
		this->centerAroundSphere();
	}

	// Split mesh before unrolling it
	bool noRedraw          = true;
	bool duplicateVertices = true;
	double primeMeridian;
	getParamFloatMesh( AXIS_PRIMEMERIDIAN, &primeMeridian );
	this->selectFacesOppositeToPlane( cos(primeMeridian), 0.0, sin(primeMeridian), 0.0 );
	this->splitByPlane( Vector3D( sin(primeMeridian), 0.0, -cos(primeMeridian), 0.0 ), duplicateVertices, noRedraw );

	Vertex* currVertex;
	for( uint64_t vertIdx = 0; vertIdx < getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos(vertIdx);
		currVertex->unrollAroundSphere( primeMeridian, mSphereRadius );
	}

	// Re-compute the normal vectors:
	resetFaceNormals();
	resetVertexNormals();

	// Re-compute the bounding box:
	estBoundingBox();
	mUnrolledAroundSphere = true;

	//! \todo stash selections before the rollout and unstash it here.
	// Clean-Up step 1: remove the intermediate selection of vertices.
	deSelMVertsAll();
	// Clean-Up step 2: Compute the distance to the plane for profile lines.
	Vector3D profileCutPlane( 1.0, 0.0, 0.0, 0.0 );
	funcVertDistanceToPlane( profileCutPlane, false );

	// Done.
	return( true );
}

// TEXTURE MAP PER VERTEX --------------------------------------------------------------------------------------------------------------------------------------

//! Assigns an imported texture map (typically generated by external tools, e.g. Matlab).
//!
//! @returns true when the texture map was assigned and stored.
//!
//! see also: MeshIO::importTEXMap and MeshQt::importTexMapFromFile
bool Mesh::assignImportedTexture( int rLineCount, uint64_t* rRefToPrimitves, unsigned char* rTexMap ) {
	Vertex* vert = nullptr;
	//! \todo add vertexTextureSetMonoHSV
	//vertexTextureSetMonoHSV( 0.0/360.0, 1.0, 0.3 );
	for( int i=0; i<rLineCount; i++ ) {
		vert = getVertexByIdxOriginal( rRefToPrimitves[i] );
		if( vert != nullptr ) {
			//cout << "[Mesh::" << __FUNCTION__ << "] " << refToPrimitves[i] << endl;
			//cout << "[Mesh::" << __FUNCTION__ << "] " << vert->getIndexOriginal() << endl;
			//cout << "[Mesh::" << __FUNCTION__ << "]---------------------------------" << endl;
			vert->setRGB( rTexMap[i*3], rTexMap[i*3+1], rTexMap[i*3+2] );
		} else {
			//! \todo Remove this debug message, when tested:
			if( rRefToPrimitves[i] < getVertexNr() ) {
				cout << "[Mesh::" << __FUNCTION__ << "] Vertex " << rRefToPrimitves[i] << " does not exist anymore." << endl;
			}
		}
	}
	return true;
}

//! Assigns normal vectors (typically generated by external tools, e.g. Matlab) to vertices.
//!
//! Returns true when the texture map was assigned and stored.
bool Mesh::assignImportedNormalsToVertices(
                const std::vector<grVector3ID>& rNormals
) {
	bool retVal = true;
	unsigned long assignedNotOk = 0;
	unsigned long badVertexIdx = 0;
	std::vector<grVector3ID>::const_iterator itVector3ID;
	for( itVector3ID = rNormals.begin(); itVector3ID != rNormals.end(); ++itVector3ID ) {
		Vertex* currVert = getVertexByIdxOriginal( (*itVector3ID).mId );
		if( currVert == nullptr ) {
			badVertexIdx++;
			continue;
		}
		if( !currVert->setNormal( (*itVector3ID).mX, (*itVector3ID).mY, (*itVector3ID).mZ ) ) {
			assignedNotOk++;
			std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Normal ( "
			          << (*itVector3ID).mX << ", " << (*itVector3ID).mY << ", " << (*itVector3ID).mZ
			          << " )' not assigned for vertex "
			          << currVert->getIndex() << " could not be assigned!" << std::endl;
		}
	}
	if( badVertexIdx > 0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: " << badVertexIdx
		          << " bad vertex indices!" << std::endl;
		retVal = false;
	}
	if( assignedNotOk > 0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: " << assignedNotOk << " of "
		          << rNormals.size() << " could not be assigned!" << std::endl;
		retVal = false;
	}
	return( retVal );
}

//! For each vertex with valid (i.e. not nan) function value: multiplies red, green and blue component with the current vertex's function value
//! @param rMin lower value for clipping
//! @param rMax higher value for clipping
//! @returns false in case of an error. True otherwise.
bool Mesh::multiplyColorWithFuncVal() {
	Vertex* currVertex = nullptr;

	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {

		double currFuncVal = 0.0;

		currVertex = getVertexPos( vertIdx );

		if( !currVertex->getFuncValue( &currFuncVal )) {
			return false;
		}

		if(std::isfinite(currFuncVal)) {

			unsigned char color[3];
			if ( !currVertex->copyRGBTo( color ) ){
				return false;
			}

			color[0] *= currFuncVal;
			color[1] *= currFuncVal;
			color[2] *= currFuncVal;

			if ( !currVertex->setRGB( color[0], color[1], color[2] ) ) {
				return false;
			}
		}
	}
	return true;
}

//! For each vertex with valid (i.e. not nan) function value: multiplies red, green and blue component with (max(rMin,min(v,rMax))-rMin)/(rMax-rMin) where v denotes the current vertex's function value
//! @param rMin lower value for clipping
//! @param rMax higher value for clipping
//! @returns false in case of an error. True otherwise.
bool Mesh::multiplyColorWithFuncVal( const double rMin, const double rMax ) {
	Vertex* currVertex = nullptr;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );

		double currFuncVal = 0.0;
		if( !currVertex->getFuncValue( &currFuncVal )) {
			return false;
		}

		if ( std::isfinite( currFuncVal )) {
			double factor = (std::max( rMin, std::min( currFuncVal, rMax ) ) -rMin ) / ( rMax - rMin );

			unsigned char color[3];
			if ( !currVertex->copyRGBTo( color ) ){
				return false;
			}

			color[0] *= factor;
			color[1] *= factor;
			color[2] *= factor;

			if ( !currVertex->setRGB( color[0], color[1], color[2] ) ) {
				return false;
			}
		}
	}
	return true;
}


//! Sets alpha values to selected vertices. If none is selected, set alpha to all vertices
bool Mesh::assignAlphaToSelectedVertices(unsigned char alpha)
{
	const unsigned char maxAlpha = 255;
	alpha = std::min(alpha, maxAlpha);

	if(mSelectedMVerts.empty())
	{
		for(auto& vertex : mVertices)
		{
			vertex->setAlpha(alpha);
		}
	}

	else
	{
		for(auto& vertex : mSelectedMVerts)
		{
			vertex->setAlpha(alpha);
		}
	}
	return true;
}

// Feature vectors ---------------------------------------------------------------------------------------------------------------------------------------------

//! Removes feature vectors from the memory and Vertex objects.
//! @returns number vertices, where no pointer could not be removed.
int Mesh::removeFeatureVectors() {
	int  vertNotAssigned  = 0;
	bool assignOK;
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		// de-assign pointer:
		assignOK = (*itVertex)->assignFeatureVec( nullptr, 0 ); // from Primitve
		if( !assignOK ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] assigned failed for Vertex with original index " << (*itVertex)->getIndexOriginal() << "!" << endl;
			vertNotAssigned++;
		}
	}
	return vertNotAssigned;
}

// --- Feature vectors -----------------------------------------------------------------------------------------------------------------------------

//! Apply MSII filtering using default parameters.
//! Only the radius i.e. largest feature size is required.
//! Calls Mesh::funcVertFeatureVecMax which will show the user
//! directly a result.
//!
//! Used for simple user interaction by toolbar button.
//!
//! \todo Implement a full version similar to the command line tool.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::computeMSIIQuick(
                const double       rRadius,       //!< Radius of the largest feature to detect
                const unsigned int rRadiiCount,   //!< Number of radii
                const unsigned int rxyzDim        //!< Raster size
) {
	// Sanity check
	if( !isnormal( rRadius ) || ( rRadius <= 0.0 ) ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Invalid radius of " << rRadius << "given!" << std::endl;
		return( false );
	}

	bool retVal(true);
	showProgressStart( "MSII filtering (Quick)" );

	// Pre-compute relative radii:
	uint64_t multiscaleRadiiSize = std::pow( 2.0, static_cast<double>(rRadiiCount) );
	double*       multiscaleRadii     = new double[multiscaleRadiiSize];
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		multiscaleRadii[i] = 1.0 - static_cast<double>(i) /
		                           static_cast<double>(multiscaleRadiiSize);
	}

	// Pre-compute sparse filte:
	voxelFilter2DElements* sparseFilters;
	generateVoxelFilters2D( multiscaleRadiiSize, multiscaleRadii, rxyzDim, &sparseFilters );

	// Prepare array for (1st) volume integral invariant filter responses
	double* descriptVolume = new double[this->getVertexNr()*multiscaleRadiiSize];

	// Determine number of threads using CPU cores minus one.
	const unsigned int availableConcurrentThreads =  std::thread::hardware_concurrency() - 1;
	std::cout << "[GigaMesh::" << __FUNCTION__ << "] Computing vertex normals using "
	          << availableConcurrentThreads << " threads" << std::endl;

	sMeshDataStruct* setMeshData = new sMeshDataStruct[availableConcurrentThreads];
	for( size_t t = 0; t < availableConcurrentThreads; t++ )
	{
		setMeshData[t].threadID               = t;
		setMeshData[t].meshToAnalyze          = this;
		setMeshData[t].radius                 = rRadius;
		setMeshData[t].xyzDim                 = rxyzDim;
		setMeshData[t].multiscaleRadiiSize    = multiscaleRadiiSize;
		setMeshData[t].multiscaleRadii        = multiscaleRadii;
		setMeshData[t].sparseFilters          = &sparseFilters;
		setMeshData[t].mPatchNormal           = nullptr;
		setMeshData[t].descriptVolume         = descriptVolume;
		setMeshData[t].descriptSurface        = nullptr;
	}

	// Use MSII function for parallel processing and normal estimation
	compFeatureVectorsMain( setMeshData, availableConcurrentThreads );

	// Assing computed feature vectors to vertices
	for( uint64_t i=0; i<this->getVertexNr(); i++ ) {
		Vertex* currVert = this->getVertexPos( i );
		if( !currVert->assignFeatureVec( &descriptVolume[i*multiscaleRadiiSize],
		                                 multiscaleRadiiSize ) ) {
			std::cerr << "[GigaMesh] ERROR: Assignment of volume based feature vectors"
			          << "to vertices failed for Vertex No. " << i << "!" << std::endl;
			retVal |= false;
		}
	}

	// Compute a function value per vertex using the feature vectors
	retVal |= funcVertFeatureVecMax();

	// Cleanup
	delete[] descriptVolume;
	delete[] setMeshData;
	showProgressStop( "MSII filtering (Quick)" );

	return( retVal );
}

//! GUI variant for Mesh::computeMSIIQuick
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::computeMSIIQuickGUI() {
	// Default
	double radius{1.0};
	// Use SelPrims to suggest the largest line segment as radius.
	double maxDist{0.0};
	std::vector<double> distancesSelPrim;
	getSelectedPositionDistancesAll( distancesSelPrim );
	for( auto const& currDist: distancesSelPrim ) {
		if( currDist > maxDist ) {
			maxDist = currDist;
		}
	}
	if( maxDist > 0.0 ) {
		radius = maxDist;
	}
	// Ask the user to check the suggested value
	if( !showEnterText( radius, "Enter radius (largest feature size)" ) ) {
		return( false );
	}
	// Ask to save the file
	bool saveFile( true );
	if( !showQuestion( &saveFile, "Save file",
	                   "<b>Computing MSII feature vectors will take time!</b><br /><br />"
	                   "Do you want to store the file?<br /><br />Recommended: YES" ) ) {
		return( false );
	}

	// Compute
	unsigned int radiiCount{4}; // Default
	unsigned int xyzDim{256};   // Default
	if( !computeMSIIQuick( radius, radiiCount, xyzDim ) ) {
		return( false );
	}

	// Save, if requested
	if( saveFile ) {
		std::wstringstream extension;
		extension.imbue(std::locale("C"));
		extension << L"_r" << std::fixed << std::setprecision(2) << radius << L"_n" << radiiCount << L"_v" << xyzDim << ".volume.ply";

		std::filesystem::path outFile = getFullName().replace_extension( "" );
		outFile += extension.str();
		if( !writeFile( outFile ) ) {
			return( false );
		}
		std::string msgFileSaved = "File was saved as:<br /><br />\n" + outFile.string();
		showInformation( "File saved", msgFileSaved );
	}

	// Done.
	return( true );
}

// --- Feature vectors - import/export -------------------------------------------------------------------------------------------------------------

//! Returns the length of the longest feature vector for a specific type of Primitive.
//! @returns zero in case of an error, e.g. if the Primitive type is not (yet) supported.
uint64_t Mesh::getFeatureVecLenMax( int rPrimitiveType ) {
	if( rPrimitiveType != Primitive::IS_VERTEX ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: only implemented for vertices!" << endl;
		return 0;
	}
	uint64_t currElementNr;
	uint64_t maxElementsNr = 0;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currElementNr = currVertex->getFeatureVectorLen();
		if( currElementNr > maxElementsNr ) {
			maxElementsNr = currElementNr;
		}
	}
	return maxElementsNr;
}

double* Mesh::getFeatureElementsNr( const int primitiveType, const int elementNr, const double* minVal, const double* maxVal, int* elementsNr ) {
	//! Returns the n-th element of all feature vectors as an array of length verticesSize x 1.
	//! Attention: NO NORMALIZATION is performed - The minimum and maximum will be stored in
	//! minVal and maxVal. e.g. to be used for Histogram display!
	//!
	//! Returns NULL in case of an error.
	if( primitiveType != Primitive::IS_VERTEX ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: only implemented for vertices!" << endl;
		return nullptr;
	}
	if( ( minVal == nullptr ) || ( maxVal == nullptr ) || ( elementsNr == nullptr ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return nullptr;
	}
	(*elementsNr) = getVertexNr();
	double* allElements = new double[getVertexNr()];
	Vertex* currVertex;
	for( unsigned int vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currVertex->getFeatureElement( elementNr, &allElements[vertIdx] );
	}
	return allElements;
}

int Mesh::cutOffFeatureElements( int primitiveType, double minVal, double maxVal, bool setToNotANumber ) {
	//! Removes feature elements of a given Primitive type. This is done either by setting values
	//! larger than maxVal to maxVal or to not-a-number. Analog: minVal.
	//! Purpose: to remove/discard outliers.
	//!
	//! Returns negative value in case of an error.
	//! Otherwise: returns the number of elements changed.
	if( primitiveType != Primitive::IS_VERTEX ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: only implemented for vertices!" << endl;
		return -1;
	}
	int elementsChanged = 0;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		elementsChanged += currVertex->cutOffFeatureElements( minVal, maxVal, setToNotANumber );
	}
	return elementsChanged;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Norm of the feature vector ------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Computes the manhattan length of the feature vector i.e. 1-norm.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureVecLenManVertex( double** funcValues, Vertex*** vertices, int* vertCount ) {
	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( !currVertex->getFeatureVecLenMan( &(*funcValues)[vertIdx] ) ) {
			// error - continue - (*funcValues)[vertIdx] will be _NOT_A_NUMBER_DBL_
		}
	}
	return true;
}

//! Computes the euclidean length of the feature vector i.e. 2-norm.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureVecLenEucVertex( double** funcValues, Vertex*** vertices, int* vertCount ) {
	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( !currVertex->getFeatureVecLenEuc( &(*funcValues)[vertIdx] ) ) {
			// error - continue - (*funcValues)[vertIdx] will be _NOT_A_NUMBER_DBL_
		}
	}
	return true;
}

//! Computes the Bounded Variation (BV) of the feature vector.
//! See Vertex::getFeatureVecBVFunc
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureVecBVFunc( double** rFuncValues, Vertex*** rVertices, int* rVertCount ) {
	*rVertices   = new Vertex*[getVertexNr()];
	*rFuncValues = new double[getVertexNr()];
	*rVertCount  = getVertexNr();

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*rVertices)[vertIdx] = currVertex;
		(*rFuncValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( !currVertex->getFeatureVecBVFunc( &(*rFuncValues)[vertIdx] ) ) {
			// error - continue - (*funcValues)[vertIdx] will be _NOT_A_NUMBER_DBL_
		}
	}
	return true;
}

//! Computes the Total Variation (TV) of the feature vector.
//! See Vertex::getFeatureVecTVSeqn
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureVecTVSeqn( double** rFuncValues, Vertex*** rVertices, int* rVertCount ) {
	*rVertices   = new Vertex*[getVertexNr()];
	*rFuncValues = new double[getVertexNr()];
	*rVertCount  = getVertexNr();

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*rVertices)[vertIdx] = currVertex;
		(*rFuncValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( !currVertex->getFeatureVecTVSeqn( &(*rFuncValues)[vertIdx] ) ) {
			// error - continue - (*funcValues)[vertIdx] will be _NOT_A_NUMBER_DBL_
		}
	}
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Norm of the feature vector with a given reference vector ------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Computes the manhattan distances of the feature vector of a given primitive to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureDistManToVertex( Primitive* rSomePrim, double** funcValues, Vertex*** vertices, int* vertCount ) {
	if( rSomePrim == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer passed!" << endl;
		return false;
	}
	int featureVecLenRef = rSomePrim->getFeatureVectorLen();
	if( featureVecLenRef < 1 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Selection has NO feature vector present!" << endl;
		return false;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] Reference feature vector: ";
	double* referenceFeature = new double[featureVecLenRef];
	for( int i=0; i<featureVecLenRef; i++ ) {
		rSomePrim->getFeatureElement( i, &referenceFeature[i] );
		cout << " " << referenceFeature[i];
	}
	cout << endl;
	bool retVal = estFeatureDistManToVertex( referenceFeature, funcValues, vertices, vertCount );
	delete[] referenceFeature;
	return retVal;
}

//! Computes the manhattan distances of a given feature vector to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureDistManToVertex( double* someFeatureVector, double** funcValues, Vertex*** vertices, int* vertCount ) {
	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( currVertex->getFeatureVectorLen() > 0 ) {
			(*funcValues)[vertIdx] = currVertex->getFeatureDistManTo( someFeatureVector );
		}
	}
	return true;
}

//! Computes the distances of a the feature vector of a given primitive to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureDistEucToVertex( Primitive* rSomePrim, double** rFuncValues, Vertex*** rVertices, int* rVertCount ) {
	if( rSomePrim == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer passed!" << endl;
		return false;
	}
	int featureVecLenRef = rSomePrim->getFeatureVectorLen();
	if( featureVecLenRef < 1 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Selection has NO feature vector present!" << endl;
		return false;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] Reference feature vector: ";
	double* referenceFeature = new double[featureVecLenRef];
	for( int i=0; i<featureVecLenRef; i++ ) {
		rSomePrim->getFeatureElement( i, &referenceFeature[i] );
		cout << " " << referenceFeature[i];
	}
	cout << endl;
	bool retVal = estFeatureDistEucToVertex( referenceFeature, rFuncValues, rVertices, rVertCount );
	delete[] referenceFeature;
	return retVal;
}

//! Computes the distances of a the given feature vector to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureDistEucToVertex( double* someFeatureVector, double** funcValues, Vertex*** vertices, int* vertCount ) {
	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( currVertex->getFeatureVectorLen() > 0 ) {
			(*funcValues)[vertIdx] = currVertex->getFeatureDistEucTo( someFeatureVector );
		}
	}
	return true;
}

//! Computes the distances of a the feature vector of a given primitive to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureDistEucNormToVertex( Primitive* rSomePrim, double** rFuncValues, Vertex*** rVertices, int* rVertCount ) {
	if( rSomePrim == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer passed!" << endl;
		return false;
	}
	int featureVecLenRef = rSomePrim->getFeatureVectorLen();
	if( featureVecLenRef < 1 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Selection has NO feature vector present!" << endl;
		return false;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] Reference feature vector: ";
	double* referenceFeature = new double[featureVecLenRef];
	for( int i=0; i<featureVecLenRef; i++ ) {
		rSomePrim->getFeatureElement( i, &referenceFeature[i] );
		cout << " " << referenceFeature[i];
	}
	cout << endl;
	bool retVal = estFeatureDistEucNormToVertex( referenceFeature, rFuncValues, rVertices, rVertCount );
	delete[] referenceFeature;
	return retVal;
}

//! Computes the distances of a the given feature vector to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureDistEucNormToVertex( double* someFeatureVector, double** funcValues, Vertex*** vertices, int* vertCount ) {
	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( currVertex->getFeatureVectorLen() > 0 ) {
			(*funcValues)[vertIdx] = currVertex->getFeatureDistEucNormTo( someFeatureVector, &mVerticesFeatVecStd );
		}
	}
	return true;
}

//! Computes the cosine similarity of a given feature vector of a primitive to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureCosineSimToVertex( Primitive* rSomePrim, double** rFuncValues, Vertex*** rVertices, int* rVertCount ) {
	if( rSomePrim == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer passed!" << endl;
		return false;
	}
	int featureVecLenRef = rSomePrim->getFeatureVectorLen();
	if( featureVecLenRef < 1 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Selection has NO feature vector present!" << endl;
		return false;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] Reference feature vector: ";
	double* referenceFeature = new double[featureVecLenRef];
	for( int i=0; i<featureVecLenRef; i++ ) {
		rSomePrim->getFeatureElement( i, &referenceFeature[i] );
		cout << " " << referenceFeature[i];
	}
	cout << endl;
	bool retVal = estFeatureCosineSimToVertex( referenceFeature, rFuncValues, rVertices, rVertCount );
	delete[] referenceFeature;
	return retVal;
}

//! Computes the cosine similarity of a given feature vector to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureCosineSimToVertex( double* rSomeFeatureVector, double** rFuncValues, Vertex*** rVertices, int* rVertCount ) {
	*rVertices   = new Vertex*[getVertexNr()];
	*rFuncValues = new double[getVertexNr()];
	*rVertCount  = getVertexNr();

	int vertIdx=0;
	vector<Vertex*>::iterator itVertex;
	for( itVertex = mVertices.begin(); itVertex != mVertices.end(); itVertex++ ) {
		(*rVertices)[vertIdx] = (*itVertex);
		(*rFuncValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( (*itVertex)->getFeatureVectorLen() > 0 ) {
			(*rFuncValues)[vertIdx] = (*itVertex)->getFeatureVecCosSim( rSomeFeatureVector, true );
		}
		vertIdx++;
	}
	return true;
}

//! Computes the cosine similarity of a given feature vector of a primitive to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureTanimotoDistTo( Primitive* rSomePrim, double** rFuncValues, Vertex*** rVertices, int* rVertCount ) {
	if( rSomePrim == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer passed!" << endl;
		return false;
	}
	int featureVecLenRef = rSomePrim->getFeatureVectorLen();
	if( featureVecLenRef < 1 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Selection has NO feature vector present!" << endl;
		return false;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] Reference feature vector: ";
	double* referenceFeature = new double[featureVecLenRef];
	for( int i=0; i<featureVecLenRef; i++ ) {
		rSomePrim->getFeatureElement( i, &referenceFeature[i] );
		cout << " " << referenceFeature[i];
	}
	cout << endl;
	bool retVal = estFeatureTanimotoDistTo( referenceFeature, rFuncValues, rVertices, rVertCount );
	delete[] referenceFeature;
	return retVal;
}

//! Computes the cosine similarity of a given feature vector to the feature vectors of all Vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::estFeatureTanimotoDistTo( double* rSomeFeatureVector, double** rFuncValues, Vertex*** rVertices, int* rVertCount ) {
	*rVertices   = new Vertex*[getVertexNr()];
	*rFuncValues = new double[getVertexNr()];
	*rVertCount  = getVertexNr();

	int vertIdx=0;
	vector<Vertex*>::iterator itVertex;
	for( itVertex = mVertices.begin(); itVertex != mVertices.end(); itVertex++ ) {
		(*rVertices)[vertIdx] = (*itVertex);
		(*rFuncValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		if( (*itVertex)->getFeatureVectorLen() > 0 ) {
			(*rFuncValues)[vertIdx] = (*itVertex)->getFeatureVecTanimotoDist( rSomeFeatureVector );
		}
		vertIdx++;
	}
	return true;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------
// --- Other feature vector related functions ------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Computes the auto correlation of Vertex features.
//! @returns false in case of an error (e.g. ALGLIB not defined/included).
bool Mesh::estFeatureAutoCorrelationVertex( double** funcValues, Vertex*** vertices, int* vertCount ) {
	cout << "[Mesh::" << __FUNCTION__ << "] AUTO Correlation." << endl;
	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

#ifndef ALGLIB
	cerr << __PRETTY_FUNCTION__ << " Warning: running untested code without ALGLIB!" << endl;
	//ap::real_1d_array crossCorr;
	//ap::real_1d_array referenceFeature;

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		const int featureVecLen = currVertex->getFeatureVectorLen();
		if( featureVecLen <= 0 ) {
			continue;
		}
		//referenceFeature.setlength( featureVecLen );
		double* referenceFeature = new double[featureVecLen];
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		for( int i=0; i<featureVecLen; i++ ) {
			// Copy each element and scale it to [-1.0...+1.0]
			//referenceFeature( i ) = (2.0*currVertex->getFeatureElement(i))-1.0;
			// Elements are already scaled to [-1.0...+1.0]
			getFeatureElement( i, &referenceFeature[i] );
		}
		double* crossCorr = new double[2*featureVecLen];
		corrr1d( referenceFeature, featureVecLen, referenceFeature, featureVecLen, crossCorr );
		for( int j=0; j<2*featureVecLen-1; j++ ) {
			// Integrate ... while 0 = symmetric
			(*funcValues)[vertIdx] += crossCorr[j];
		}

		delete[] referenceFeature;
		delete[] crossCorr;

	}

	return true;
#else
	ap::real_1d_array crossCorr;
	ap::real_1d_array referenceFeature;

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		unsigned int featureVecLen = currVertex->getFeatureVectorLen();
		if( featureVecLen <= 0 ) {
			continue;
		}
		referenceFeature.setlength( featureVecLen );
		(*funcValues)[vertIdx] = 0.0;
		for( unsigned int i=0; i<featureVecLen; i++ ) {
			// Copy each element and scale it to [-1.0...+1.0]
			//referenceFeature( i ) = (2.0*currVertex->getFeatureElement(i))-1.0;
			// Elements are already scaled to [-1.0...+1.0]
			getFeatureElement( i, &referenceFeature(i) );
		}
		corrr1d( referenceFeature, featureVecLen, referenceFeature, featureVecLen, crossCorr );
		for( uint64_t j=0; j<2*featureVecLen-1; j++ ) {
			// Integrate ... while 0 = symmetric
			(*funcValues)[vertIdx] += crossCorr(j);
		}
	}
	return true;
#endif
}

bool Mesh::estFeatureCorrelationVertex( Primitive* somePrim, double** funcValues, Vertex*** vertices, int* vertCount ) {
	//! Estimates the auto correlation of Vertex features.
	//! Returns false in case of an error (e.g. ALGLIB not defined/included).

	if( somePrim == nullptr ) {
		cerr << __PRETTY_FUNCTION__ << " NULL pointer passed!" << endl;
		return false;
	}
	const int featureVecLenRef = somePrim->getFeatureVectorLen();
	if( featureVecLenRef < 1 ) {
		cerr << __PRETTY_FUNCTION__ << " Selection has NO feature vector present!" << endl;
		return false;
	}

	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

#ifndef ALGLIB
	cerr << __PRETTY_FUNCTION__ << " Warning: running untested code without ALGLIB!" << endl;

	double* referenceFeature = new double[featureVecLenRef]; //ap::real_1d_array referenceFeature; referenceFeature.setlength( featureVecLenRef );


	cout << "[Mesh::estFeatureCorrelationVertex] reference:";
	for( int i=0; i<featureVecLenRef; i++ ) {
		// Copy each element and scale it to [-1.0...+1.0]
		//cout << " " << (2.0*somePrim->getFeatureElement(i))-1.0;
		//referenceFeature( i ) = (2.0*somePrim->getFeatureElement(i))-1.0;
		// Elements are already scaled to [-1.0...+1.0]
		somePrim->getFeatureElement( i, &referenceFeature[i] );
		cout << " " << referenceFeature[i];
	}
	cout << endl;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		const int featureVecLenAll = currVertex->getFeatureVectorLen();
		if( featureVecLenAll <= 0 ) {
			continue;
		}
		double* someFeature = new double[featureVecLenAll]; //someFeature.setlength( featureVecLenAll );
		for( int i=0; i<featureVecLenAll; i++ ) {
			// Copy each element and scale it to [-1.0...+1.0]
			//someFeature( i ) = (2.0*currVertex->getFeatureElement(i))-1.0;
			// Elements are already scaled to [-1.0...+1.0]
			currVertex->getFeatureElement( i, &someFeature[i] );
		}
		double* crossCorr = new double[featureVecLenAll + featureVecLenRef];
		corrr1d( someFeature, featureVecLenAll, referenceFeature, featureVecLenRef, crossCorr );
		(*funcValues)[vertIdx] = 0.0;
		//(*funcValues)[vertIdx] = crossCorr(0);
// 		int maxVal = crossCorr(0);
		for( int j=0; j<(featureVecLenRef+featureVecLenAll)-1; j++ ) {
			// Integrate ... while 0 = symmetric
			(*funcValues)[vertIdx] += crossCorr[j];
// 			if( (*funcValues)[vertIdx] < crossCorr(j) ) {
// 				(*funcValues)[vertIdx] = crossCorr(j);
// 			}
// 			if( maxVal < crossCorr(j) ) {
// 				(*funcValues)[vertIdx] = (float)j;
// 			}
		}

		delete[] someFeature;
		delete[] crossCorr;

	}

	delete[] referenceFeature;

	return true;
#else
	ap::real_1d_array crossCorr;
	ap::real_1d_array referenceFeature;
	ap::real_1d_array someFeature;
	referenceFeature.setlength( featureVecLenRef );
	cout << "[Mesh::estFeatureCorrelationVertex] reference:";
	for( int i=0; i<featureVecLenRef; i++ ) {
		// Copy each element and scale it to [-1.0...+1.0]
		//cout << " " << (2.0*somePrim->getFeatureElement(i))-1.0;
		//referenceFeature( i ) = (2.0*somePrim->getFeatureElement(i))-1.0;
		// Elements are already scaled to [-1.0...+1.0]
		double elementValue;
		somePrim->getFeatureElement( i, &elementValue );
		referenceFeature( i ) = elementValue;
		cout << " " << elementValue;
	}
	cout << endl;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = currVertex;
		(*funcValues)[vertIdx] = _NOT_A_NUMBER_DBL_;
		int featureVecLenAll = currVertex->getFeatureVectorLen();
		if( featureVecLenAll <= 0 ) {
			continue;
		}
		someFeature.setlength( featureVecLenAll );
		for( int i=0; i<featureVecLenAll; i++ ) {
			// Copy each element and scale it to [-1.0...+1.0]
			//someFeature( i ) = (2.0*currVertex->getFeatureElement(i))-1.0;
			// Elements are already scaled to [-1.0...+1.0]
			double elementValue;
			currVertex->getFeatureElement( i, &elementValue );
			someFeature( i ) = elementValue;
		}
		corrr1d( someFeature, featureVecLenAll, referenceFeature, featureVecLenRef, crossCorr );
		(*funcValues)[vertIdx] = 0.0;
		//(*funcValues)[vertIdx] = crossCorr(0);
// 		int maxVal = crossCorr(0);
		for( int j=0; j<(featureVecLenRef+featureVecLenAll)-1; j++ ) {
			// Integrate ... while 0 = symmetric
			(*funcValues)[vertIdx] += crossCorr(j);
// 			if( (*funcValues)[vertIdx] < crossCorr(j) ) {
// 				(*funcValues)[vertIdx] = crossCorr(j);
// 			}
// 			if( maxVal < crossCorr(j) ) {
// 				(*funcValues)[vertIdx] = (float)j;
// 			}
		}
	}
	return true;
#endif
}

bool Mesh::estFeatureAutoCorrelationVertex( Primitive* somePrim, double** funcValues, Vertex*** vertices, int* vertCount ) {
	//! Estimates autocorrelation and correlation to selection: log(|Autocrr|*100000) + |FTcorr| + 0.5 cutting of below 0.0 and above 1.0
	//! Returns false in case of an error (e.g. ALGLIB not defined/included).

	if( somePrim == nullptr ) {
		cerr << __PRETTY_FUNCTION__ << " NULL pointer passed!" << endl;
		return false;
	}
	unsigned int featureVecLenRef = somePrim->getFeatureVectorLen();
	if( featureVecLenRef == 0 ) {
		cerr << __PRETTY_FUNCTION__ << " Selection has NO feature vector present!" << endl;
		return false;
	}

	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

#ifndef ALGLIB
	cerr << __PRETTY_FUNCTION__ << " Warning: running untested code without ALGLIB!" << endl;
	//! *) Prepare list of vertices.
	int timeStart = clock();
	vector<Vertex*> verticesSorted;
	if( !getVertexList( &verticesSorted ) ) {
		return false;
	}

	//! *) Fetch feature vector for correlation from the given Primitive.
	//ap::real_1d_array crossCorr;
	//ap::real_1d_array crossCorrAuto;
	double* referenceFeature = new double[featureVecLenRef];      //ap::real_1d_array referenceFeature; referenceFeature.setlength( featureVecLenRef );
	double* someFeature = nullptr;

	cout << "[Mesh::estFeatureCorrelationVertex] reference:";
	for( uint64_t i=0; i<featureVecLenRef; i++ ) {
		// Copy each element and scale it to [-1.0...+1.0]
		//cout << " " << (2.0*somePrim->getFeatureElement(i))-1.0;
		//referenceFeature( i ) = (2.0*somePrim->getFeatureElement(i))-1.0;
		somePrim->getFeatureElement( i, &referenceFeature[i] );
		cout << " " << referenceFeature[i];
	}
	cout << endl;

	Vertex* currVertex = nullptr;

	//! *) Estimate correlation to selected Primitive.
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currVertex->setFuncValue( _NOT_A_NUMBER_DBL_ );
		int featureVecLenAll = currVertex->getFeatureVectorLen();
		if( featureVecLenAll <= 0 ) {
			continue;
		}
		someFeature = new double[ featureVecLenAll ];
		for( int i=0; i<featureVecLenAll; i++ ) {
			// Copy each element and scale it to [-1.0...+1.0]
			//someFeature( i ) = (2.0*currVertex->getFeatureElement(i))-1.0;
			// Elements are already scaled to [-1.0...+1.0]
			currVertex->getFeatureElement( i, &someFeature[i] );
		}
		double* crossCorr = new double[ featureVecLenAll + featureVecLenRef ];
		corrr1d( someFeature, featureVecLenAll, referenceFeature, featureVecLenRef, crossCorr );
		float funcValuesCorrTmp = 0.0F;
		for( uint64_t j=0; j<(featureVecLenRef+featureVecLenAll)-1; j++ ) {
			funcValuesCorrTmp += crossCorr[ j ];
		}

		delete[] crossCorr;

		currVertex->setFuncValue( funcValuesCorrTmp );
	}

	delete[] referenceFeature;

	//! *) Sort by correlation.
	timeStart = clock();
	sort( verticesSorted.begin(), verticesSorted.end(), Vertex::funcValLower );
	cout << "[Mesh::estFeatureCorrelationVertex] sort time: " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	//! *) Fetch indices of sorted vertices.
	uint64_t funcValOrderIdx = 0;
	vector<Vertex*>::iterator itVert;
	for( itVert = verticesSorted.begin(); itVert != verticesSorted.end(); itVert++ ) {
		// debuging:
		//float funcVal;
		//(*itVert)->getFuncValue( &funcVal );
		//cout << "[Mesh::setVertexFuncValues] Vertex " << (*itVert)->getIndex() << " funcVal: " << funcVal << endl;
		int vertIdx = (*itVert)->getIndex();
		(*vertices)[vertIdx] = (*itVert);
		(*funcValues)[vertIdx] = funcValOrderIdx;
		funcValOrderIdx++;
	}
	cout << "[Mesh::estFeatureCorrelationVertex] fetch indices time: " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	//! *) Estimate auto-correlation
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currVertex->setFuncValue( _NOT_A_NUMBER_DBL_ );
		int featureVecLenAll = currVertex->getFeatureVectorLen();
		if( featureVecLenAll <= 0 ) {
			continue;
		}

		for( int i=0; i<featureVecLenAll; i++ ) {
			// Copy each element and scale it to [-1.0...+1.0]
			//someFeature( i ) = (2.0*currVertex->getFeatureElement(i))-1.0;
			// Elements are already scaled to [-1.0...+1.0]
			currVertex->getFeatureElement( i, &someFeature[i] );
		}
		double* crossCorrAuto = new double[ featureVecLenAll + featureVecLenRef ];
		corrr1d( someFeature, featureVecLenAll, someFeature, featureVecLenRef, crossCorrAuto );
		float funcValuesCorrTmp = 0.0;
		for( uint64_t j=0; j<(featureVecLenRef+featureVecLenAll)-1; j++ ) {
			funcValuesCorrTmp += crossCorrAuto[j];
		}

		delete[] crossCorrAuto;

		currVertex->setFuncValue( funcValuesCorrTmp );
	}

	//! *) Sort by auto-correlation.
	timeStart = clock();
	sort( verticesSorted.begin(), verticesSorted.end(), Vertex::funcValLower );
	cout << "[Mesh::estFeatureCorrelationVertex] sort time: " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	//! *) Add indicies of sorted vertices.
	funcValOrderIdx = 0;
	for( itVert = verticesSorted.begin(); itVert != verticesSorted.end(); itVert++ ) {
		// debuging:
		//float funcVal;
		//(*itVert)->getFuncValue( &funcVal );
		//cout << "[Mesh::setVertexFuncValues] Vertex " << (*itVert)->getIndex() << " funcVal: " << funcVal << endl;
		int vertIdx = (*itVert)->getIndex();
		(*funcValues)[vertIdx] += funcValOrderIdx;
		funcValOrderIdx++;
	}
	cout << "[Mesh::estFeatureCorrelationVertex] fetch indices time: " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	delete[] someFeature;

	return true;
#else
	//! *) Prepare list of vertices.
	int timeStart = clock();
	vector<Vertex*> verticesSorted;
	if( !getVertexList( &verticesSorted ) ) {
		return false;
	}

	//! *) Fetch feature vector for correlation from the given Primitive.
	ap::real_1d_array crossCorr;
	ap::real_1d_array crossCorrAuto;
	ap::real_1d_array referenceFeature;
	ap::real_1d_array someFeature;
	referenceFeature.setlength( featureVecLenRef );
	cout << "[Mesh::estFeatureCorrelationVertex] reference:";
	for( unsigned int i=0; i<featureVecLenRef; i++ ) {
		// Copy each element and scale it to [-1.0...+1.0]
		//cout << " " << (2.0*somePrim->getFeatureElement(i))-1.0;
		//referenceFeature( i ) = (2.0*somePrim->getFeatureElement(i))-1.0;
		double elementValue;
		somePrim->getFeatureElement( i, &elementValue );
		referenceFeature( i ) = elementValue;
	}
	cout << endl;

	Vertex* currVertex;

	//! *) Estimate correlation to selected Primitive.
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currVertex->setFuncValue( _NOT_A_NUMBER_DBL_ );
		unsigned int featureVecLenAll = currVertex->getFeatureVectorLen();
		if( featureVecLenAll <= 0 ) {
			continue;
		}
		someFeature.setlength( featureVecLenAll );
		for( unsigned int i=0; i<featureVecLenAll; i++ ) {
			// Copy each element and scale it to [-1.0...+1.0]
			//someFeature( i ) = (2.0*currVertex->getFeatureElement(i))-1.0;
			// Elements are already scaled to [-1.0...+1.0]
			double elementValue;
			currVertex->getFeatureElement( i, &elementValue );
			someFeature( i ) = elementValue;
		}
		corrr1d( someFeature, featureVecLenAll, referenceFeature, featureVecLenRef, crossCorr );
		float funcValuesCorrTmp = 0.0;
		for( uint64_t j=0; j<(featureVecLenRef+featureVecLenAll)-1; j++ ) {
			funcValuesCorrTmp += crossCorr( j );
		}
		currVertex->setFuncValue( funcValuesCorrTmp );
	}

	//! *) Sort by correlation.
	timeStart = clock();
	sort( verticesSorted.begin(), verticesSorted.end(), Vertex::funcValLower );
	cout << "[Mesh::estFeatureCorrelationVertex] sort time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	//! *) Fetch indices of sorted vertices.
	uint64_t funcValOrderIdx = 0;
	vector<Vertex*>::iterator itVert;
	for( itVert = verticesSorted.begin(); itVert != verticesSorted.end(); itVert++ ) {
		// debuging:
		//float funcVal;
		//(*itVert)->getFuncValue( &funcVal );
		//cout << "[Mesh::setVertexFuncValues] Vertex " << (*itVert)->getIndex() << " funcVal: " << funcVal << endl;
		int vertIdx = (*itVert)->getIndex();
		(*vertices)[vertIdx] = (*itVert);
		(*funcValues)[vertIdx] = funcValOrderIdx;
		funcValOrderIdx++;
	}
	cout << "[Mesh::estFeatureCorrelationVertex] fetch indices time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	//! *) Estimate auto-correlation
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currVertex->setFuncValue( _NOT_A_NUMBER_DBL_ );
		unsigned int featureVecLenAll = currVertex->getFeatureVectorLen();
		if( featureVecLenAll <= 0 ) {
			continue;
		}
		for( unsigned int i=0; i<featureVecLenAll; i++ ) {
			// Copy each element and scale it to [-1.0...+1.0]
			//someFeature( i ) = (2.0*currVertex->getFeatureElement(i))-1.0;
			// Elements are already scaled to [-1.0...+1.0]
			double elementValue;
			currVertex->getFeatureElement( i, &elementValue );
			someFeature( i ) = elementValue;
		}
		corrr1d( someFeature, featureVecLenAll, someFeature, featureVecLenRef, crossCorrAuto );
		float funcValuesCorrTmp = 0.0;
		for( unsigned int j=0; j<(featureVecLenRef+featureVecLenAll)-1; j++ ) {
			funcValuesCorrTmp += crossCorrAuto( j );
		}
		currVertex->setFuncValue( funcValuesCorrTmp );
	}

	//! *) Sort by auto-correlation.
	timeStart = clock();
	sort( verticesSorted.begin(), verticesSorted.end(), Vertex::funcValLower );
	cout << "[Mesh::estFeatureCorrelationVertex] sort time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	//! *) Add indicies of sorted vertices.
	funcValOrderIdx = 0;
	for( itVert = verticesSorted.begin(); itVert != verticesSorted.end(); itVert++ ) {
		// debuging:
		//float funcVal;
		//(*itVert)->getFuncValue( &funcVal );
		//cout << "[Mesh::setVertexFuncValues] Vertex " << (*itVert)->getIndex() << " funcVal: " << funcVal << endl;
		int vertIdx = (*itVert)->getIndex();
		(*funcValues)[vertIdx] += funcValOrderIdx;
		funcValOrderIdx++;
	}
	cout << "[Mesh::estFeatureCorrelationVertex] fetch indices time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	return true;
#endif
}

//! Apply a median or mean filter operation on a vertex's 1-ring neighbourhood
//! at the elements of the feature vector.
//!
//! User interactive.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::featureVecMedianOneRingUI(
    bool         rPreferMeanOverMedian    //!< Compute mean value instead of the median.
) {
	bool retVal = true;
	// Ask for a filter size (radius) in mesh units:
	double filterSize = 0.25;
	if( showEnterText( filterSize, "Filtersize in world units" ) ) {
		if( filterSize <= 0.0 ) {
			// Ask for a number of iterations, when no radius was given:
			uint64_t iterations=1;
			if( showEnterText( iterations, "Number of iterations" ) ) {
				retVal = featureVecMedianOneRing( iterations, 0.0, rPreferMeanOverMedian );
			} else {
				retVal = false; // i.e. user cancel or invalid values.
			}
		} else {
			retVal = featureVecMedianOneRing( 0, filterSize, rPreferMeanOverMedian );
		}
	}

	return( retVal );
}

//! Apply a median or mean filter operation on a vertex's 1-ring neighbourhood
//! at the elements of the feature vector.
//!
//! The filter size can be defined by either
//!     providing a number of iterations by providing rIterations > 0
//! or
//!     a radius in mesh units by providing a rFilterSize > 0.0.
//! In case both valies are set (valid), the number of iterations is used.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::featureVecMedianOneRing(
    unsigned int rIterations,              //!< Number of interations
    double       rFilterSize,              //!< Filter size in units of the mesh (e.g. mm)
    bool         rPreferMeanOverMedian     //!< Compute mean value instead of the median.
) {
	// 0a. Pre-Compute the minimum face altitude length
	// double minDist = getAltitudeMin();
	// cout << "[Mesh::" << __FUNCTION__ << "] Minimal altitude: " << minDist << " mm (unit assumed)." << endl;

	// 0a. Pre-Compute the minimum edge length
	double minDist = getEdgeLenMin();
	cout << "[Mesh::" << __FUNCTION__ << "] Minimal edge length: " << minDist << " mm (unit assumed)." << endl;

	// 0b. Sanity checks
	if( rIterations == 0 ) {
		if( rFilterSize <= 0.0 ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Zero iterations or invalid filter size requested!" << endl;
			return( false );
		}
		// Compute number of iterations using the given filter size:
		rIterations = round( 2.0*rFilterSize/minDist );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] " << rIterations << " iterations corresponds to a filter size (radius) of " << rIterations*minDist << " mm (unit assumed)." << endl;

	string funcName;
	if( rPreferMeanOverMedian ) {
		funcName = "Feature vector elements 1-ring mean, " + to_string( rIterations ) + " iterations";
	} else {
		funcName = "Feature vector elements 1-ring median, " + to_string( rIterations ) + " iterations";
	}
	time_t timeStart = clock();
	bool retVal = true;

	// Number of vertices
	uint64_t nrOfVertices = getVertexNr();

	// Pre-compute values
	showProgressStart( funcName + " Pre-Computation" );
	vector<vector<s1RingSectorPrecomp>> oneRingSectPrecomp;
	for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		VertexOfFace* currVertex = static_cast<VertexOfFace*>( getVertexPos( vertIdx ) );
		vector<s1RingSectorPrecomp> curr1RingSectPrecomp;
		currVertex->get1RingSectors( minDist, curr1RingSectPrecomp );
		oneRingSectPrecomp.push_back( curr1RingSectPrecomp );
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName + " Pre-Computation" );
	}
	showProgressStop( funcName + " Pre-Computation" );

	showProgressStart( funcName );
	// Apply multiple times
	for( unsigned int i=0; i<rIterations; i++ ) {
		vector<vector<double>> newFeatureVectors;
		unsigned int   vertsIgnored = 0;
		// 1. Compute indipendently and write back later!
		for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
			Vertex* currVertex = getVertexPos( vertIdx );
			uint64_t featureVecLenCurr = currVertex->getFeatureVectorLen();
			vector<double> featureVecSmooth(featureVecLenCurr,_NOT_A_NUMBER_DBL_);
			bool retValCurr;
			if( rPreferMeanOverMedian ) {
				retValCurr = currVertex->getFeatureVecMeanOneRing( minDist, oneRingSectPrecomp.at(vertIdx), featureVecSmooth );
				// retValCurr = currVertex->getFeatureVecMeanOneRing( featureVecSmooth, minDist );
			} else {
				retValCurr = currVertex->getFeatureVecMedianOneRing( featureVecSmooth, minDist );
			}
			if( !retValCurr ) {
				vertsIgnored++;
			}
			newFeatureVectors.push_back( featureVecSmooth );
			showProgress( (static_cast<double>(i)/static_cast<double>(rIterations)) + (0.75*((vertIdx+1)/static_cast<double>(nrOfVertices)))/static_cast<double>(rIterations), funcName );
		}

		// 2. Write back the new values:
		for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
			Vertex* currVertex = getVertexPos( vertIdx );
			vector<double> featureVecSmooth = newFeatureVectors.at( vertIdx );
			currVertex->assignFeatureVecValues( featureVecSmooth );
			showProgress( (static_cast<double>(i)/static_cast<double>(rIterations)) + (0.25+0.75*((vertIdx+1)/static_cast<double>(nrOfVertices)))/static_cast<double>(rIterations), funcName );
		}

		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Vertices processed: " << getVertexNr()-vertsIgnored << endl;
		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Vertices ignored:   " << vertsIgnored << endl;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFeatureVectors();
	return( retVal );
}

// ----------------------------------------------------------------------------

set<Vertex*> Mesh::intersectSphere( Vertex* someVert, float radius ) {
	//! Estimate points of intersection between a sphere described by a Vertex and a radius.
	//!
	//! \return Points of interseciont.

	cout << "[Mesh::intersectSphere]" << endl;

	set<Face*> facesInSphere;
	set<Vertex*> intersectionVertices;
	if( !fetchSphereMarchingDualFront( someVert, &facesInSphere, radius ) ) {
		cerr << "[Mesh::intersectSphere] fetchSphereMarchingDualFront failed!" << endl;
		return intersectionVertices;
	}

	set<Face*>::iterator itFace;
	for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
		if( (*itFace)->intersectsSphere2( someVert->getPositionVector(), radius ) ) {
			Vector3D* interSectionsArr = nullptr;
			int       interSectionsNr;
			interSectionsNr = (*itFace)->intersectSphereSpecial( someVert->getPositionVector(), radius, &interSectionsArr );
			cout << "[Mesh::intersectSphere] " << interSectionsNr << " intersection." << endl;
		}
	}

	return intersectionVertices;
}


// labeling ----------------------------------------------------------------------------------------

bool Mesh::labelsChanged() {
	//! Stub to be overloaded to handle events related to changed labels.
	//! e.g.renewal of texture maps.
	return true;
}

//! Returns the number of labels set.
bool Mesh::labelCount(
                int rPrimitiveType,     //!< Type of labled primitive.
                uint64_t& rLabelMaxNr   //!< (return value)
) {
	bool labelFound      = false;
	uint64_t labelMaxNrFound = 0; // Labels start at one - non-labled areas will return false in getLabel/vertLabelGet
	uint64_t currLabel;
	switch( rPrimitiveType ) {
		case Primitive::IS_POLYLINE: {
			vector<PolyLine*>::iterator itPoly;
			for( itPoly=mPolyLines.begin(); itPoly != mPolyLines.end(); itPoly++ ) {
				if( (*itPoly)->getLabel( currLabel ) ) {
					labelFound = true;
					if( labelMaxNrFound < currLabel ) {
						labelMaxNrFound = currLabel;
					}
				}
			}
		} break;
		case Primitive::IS_VERTEX:
		case Primitive::IS_FACE: {
			// we parse vertices also for faces as it will result in the same as
			// face labels are determined by vertex labels and the number of iterations
			// is lower and therefore it is faster:
			Vertex* curVertex;
			for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
				curVertex = getVertexPos( vertIdx );
				if( curVertex->getLabel( currLabel ) ) {
					labelFound = true;
					if( labelMaxNrFound < currLabel ) {
						labelMaxNrFound = currLabel;
					}
				}
			}
		} break;
		default:
			cerr << "[Mesh::" << __FUNCTION__ << "] not defined for primitive type " << rPrimitiveType << "!" << endl;
	}
	if( labelFound ) {
		rLabelMaxNr = labelMaxNrFound ; // Otherwise we will return the maximum index is equal to the number of labels as the first label has id ONE.
		//cerr << "[Mesh::" << __FUNCTION__ << "] labelMaxNrFound: " << labelMaxNrFound << "!" << endl;
	}
	return labelFound;
}

int Mesh::labelCountElements( int primitiveType ) {
	//! Returns the number of elements having a label set.
	int elementsLabled = 0;
	switch( primitiveType ) {
		case Primitive::IS_POLYLINE: {
			    vector<PolyLine*>::iterator itPoly;
				for( itPoly=mPolyLines.begin(); itPoly != mPolyLines.end(); itPoly++ ) {
					if( (*itPoly)->isLabled() ) {
						elementsLabled++;
					}
				}
		    }
			break;
		case Primitive::IS_VERTEX:
		case Primitive::IS_FACE: {
			// we parse vertices also for faces as it will result in the same as
			// face labels are determined by vertex labels and the number of iterations
			// is lower and therefore it is faster:
			    Vertex* curVertex;
				for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
					curVertex = getVertexPos( vertIdx );
					if( curVertex->isLabled() ) {
						elementsLabled++;
					}
				}
		    }
			break;
		default:
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: not defined for primitive type << " << primitiveType << "!" << endl;
	}
	return elementsLabled;
}

bool Mesh::estLabelNormalSizeCenterVert( vector<Vector3D>* rLabelCenters, vector<Vector3D>* rLabelNormals ) {
	//! Estimate the center of gravities the average normals for each label.
	//! The homogenous coordinate of the centers is the number of vertices.
	//! The length of the normal is the label area.
	//! Returns false in case of an error.
	if( ( rLabelCenters == nullptr ) || ( rLabelNormals == nullptr ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	uint64_t labelMaxNr;
	if( !labelCount( Primitive::IS_VERTEX, labelMaxNr ) ) {
		return( false );
	}
	rLabelCenters->resize( labelMaxNr, Vector3D( 0.0, 0.0, 0.0, 0.0 ) );
	rLabelNormals->resize( labelMaxNr, Vector3D( 0.0, 0.0, 0.0, 0.0 ) );
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		uint64_t currLabel;
		if( !currVertex->getLabel( currLabel ) ) {
			// No label set.
			continue;
		}
		// Label numbering starts with index 1. Therefore:
		currLabel -= 1;
		// Sum vertex positions:
		Vector3D vecPos = currVertex->getPositionVector();
		vecPos += rLabelCenters->at( currLabel );
		rLabelCenters->at( currLabel ) = vecPos;
		// Sum normals:
		Vector3D vecNormal = currVertex->getNormal( false );
		vecNormal /= 3.0; // <- As vertices have the sum of the adjacent faces as normal length, we have to compensate the fact that each face area will be added 3 times.
		vecNormal += rLabelNormals->at( currLabel );
		rLabelNormals->at( currLabel ) = vecNormal;
	}
	return true;
}

void Mesh::labelVerticesNone() {
	//! Sets all vertices to not labled.
	//! Typically called before labeling.
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currVertex->setLabelNone();
		//cout << "[Mesh::" << __FUNCTION__ << "] " << curVertex->getIndex() << endl;
	}
	labelsChanged();
}

void Mesh::labelVerticesBackground() {
	//! Sets all vertices to background.
	//! Typically called before labeling selected vertices.
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		curVertex->setLabelBackGround();
		//cout << "[Mesh::" << __FUNCTION__ << "] " << curVertex->getIndex() << endl;
	}
	labelsChanged();
}

int Mesh::labelFaces( int facesNrToRemove ) {
	//! Labeling assings the same index to all connected Faces.
	//!
	//! When facesNrToRemove > 0 than a connected (labeld) areas having a
	//! Face count lower than facesNrToRemove are removed.
	//!
	//! As the used marching front uses connectivity by edges (and not vertices),
	//! faces connected by single vertices may not get the same and label and
	//! errors will be shown.
	//! This can be fixed by calling removeDoubleCones() first.
	cout << "[Mesh::" << __FUNCTION__ << "] facesNrToRemove: " << facesNrToRemove << endl;

	int  setLabel    = 0;
	bool removeLabel = false;

	set<Face*> frontFaces;
	set<Face*> frontFacesNew;
	set<Face*> facesVisited;
	set<Face*> facesToRemove;

	set<Face*>::iterator itFaceFront;
	set<Face*>::iterator itFaceToLabel;

	int   addedToRemove    = 0;
	float areaCurrentLabel = 0.0;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		removeLabel = false;
		if( currFace->isLabelBackGround() ) {
			continue;
		}
		if( currFace->isLabled() ) {
			continue;
		}
		frontFaces.insert( currFace );
		while( frontFaces.size() > 0 ) {
			for( itFaceFront=frontFaces.begin(); itFaceFront != frontFaces.end(); itFaceFront++ ) {
				if( (*itFaceFront)->isLabelBackGround() ) {
					//cout << "[Mesh::labelFaces] (1) Stop at Vertex: " << (*itVertexFront)->getIndexOriginal() << endl;
					continue;
				}
				if( (*itFaceFront)->isLabled() ) {
					//cout << "[Mesh::labelFaces] (2) Stop at Vertex: " << (*itVertexFront)->getIndexOriginal() << endl;
					// This will select vertices in 1-ring neighbourhood around the label:
					//mSelectedMVerts.insert( (*itVertexFront) );
					continue;
				}
				facesVisited.insert( (*itFaceFront) );
				areaCurrentLabel += (*itFaceFront)->getAreaNormal();
				(*itFaceFront)->getNeighbourFacesExcluding( &frontFacesNew, &facesVisited );
			}
			//cout << "facesVisited.size:  " << facesVisited.size() << endl;
			//cout << "frontFacesNew.size: " << frontFacesNew.size() << endl;
			//cout << "frontFaces.size:    " << frontFaces.size() << endl;
			frontFaces.swap( frontFacesNew );
			frontFacesNew.clear();
		}
		if( facesVisited.size() < static_cast<uint>(facesNrToRemove) ) {
			removeLabel = true;
		}
		for( itFaceToLabel=facesVisited.begin(); itFaceToLabel != facesVisited.end(); itFaceToLabel++ ) {
			if( removeLabel ) {
				facesToRemove.insert( (*itFaceToLabel) );
				addedToRemove++;
			} else {
				(*itFaceToLabel)->setLabel( setLabel );
			}
		}
		if( addedToRemove > 0 ) {
			cout << "[Mesh::" << __FUNCTION__ << "] Labeling is going to remove " << addedToRemove << " Faces." << endl;
			addedToRemove = 0;
		} else {
			if( facesVisited.size() < 500 ) {
				for( itFaceToLabel=facesVisited.begin(); itFaceToLabel != facesVisited.end(); itFaceToLabel++ ) {
					(*itFaceToLabel)->setLabelBackGround();
				}
				cout << "[Mesh::" << __FUNCTION__ << "] Labeling set Label |Faces| < 500 to background." << endl;
			} else {
				cout << "[Mesh::" << __FUNCTION__ << "] Label No: " << setLabel << " containes " << facesVisited.size() << " Faces and has " << areaCurrentLabel << " mm² (unit assumed)." << endl;
				setLabel++;
			}
			areaCurrentLabel = 0;
		}
		frontFaces.clear();
		frontFacesNew.clear();
		facesVisited.clear();
		//cout << "--- " << setLabel << " --------------------------------------------" << endl;
	}

	if( facesToRemove.size() > 0 ) {
		removeFaces( &facesToRemove );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Labeling removed " << facesToRemove.size() << " Faces." << endl;

	setLabel -= 1; // Correct for indexing begining at ONE.
	cout << "[Mesh::" << __FUNCTION__ << "] " << setLabel << " Labels set." << endl;
	labelsChanged();
	return setLabel;
}

//! Label all vertices i.e. connected components of the mesh.
//! Wrapping method typically used within gigamesh-clean.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::labelVerticesAll() {
	set<Vertex*> allVerticesToLabel;
	if( !getVertexList( &allVerticesToLabel ) ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: getVertexList failed!" << std::endl;
		return( false );
	}
	labelVerticesNone();
	return Mesh::labelVertices( allVerticesToLabel, allVerticesToLabel );
}

//! Wrapping for the labeling method accepting a vector of vertices.
//! Typically used for Mesh::mVertices, which is a vector.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::labelVertices(
        const vector<Vertex *>& rVerticesToLabel,      //!< Selection of vertices to be labeled.
              set<Vertex *>&    rVerticesSeeds         //!< Seed points for labeling.
) {
	set<Vertex*> verticesToLabelVec;
	verticesToLabelVec.insert( rVerticesToLabel.begin(), rVerticesToLabel.end() );
	return Mesh::labelVertices( verticesToLabelVec, rVerticesSeeds );
}

//! Labeling assings the same index to all connected Faces.
//!
//! Label numbering begins with ONE - otherwise the inverted selction using negative
//! indices will not work properly for the label having the (signless) zero.
//!
//! When verticesNrToRemove > 0 than a connected (labeld) areas having a
//! Vertex count lower than verticesNrToRemove are removed.
//!
//! As the used marching front uses connectivity by edges (and not vertices),
//! faces connected by single vertices may not get the same and label and
//! errors will be shown.
//!
//! This can be fixed by calling removeDoubleCones() first.
bool Mesh::labelVertices(
        const set<Vertex*>&   rVerticesToLabel,        //!< Selection of vertices to be labeled.
              set<Vertex*>&   rVerticesSeeds           //!< Seed vertices
) {
	//cout << "[Mesh::" << __FUNCTION__ << "]"<< endl;

	if( rVerticesSeeds.size() <= 0 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] No seeds stored ...." << endl;
		if( rVerticesToLabel.size() <= 0 ) {
			cout << "[Mesh::" << __FUNCTION__ << "] ... neither are vertices given => all vertices as seed selected." << endl;
			set<Vertex*> allVertices;
			if( !getVertexList( &allVertices ) ) {
				cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: could not fetch vertices as seed!" << endl;
				return false;
			}
			rVerticesSeeds.swap( allVertices );
		} else {
			cout << "[Mesh::" << __FUNCTION__ << "] ... using selection as seed." << endl;
			// Selection to seeds (similar to Mesh::labelSelectionToSeeds)
			rVerticesSeeds.clear();
			rVerticesSeeds.insert( rVerticesToLabel.begin(), rVerticesToLabel.end() );

		}
	}

	// Iteration step as function value.
	bool setLabelStepToFuncVal = false;
	if( !getParamFlagMesh( LABELING_USE_STEP_AS_FUNCVAL, &setLabelStepToFuncVal ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: getParamFlagMesh failed!" << endl;
		return( false );
	}

	// Performance and progress
	clock_t timeStart = clock();
	uint64_t verticesLabeled = 0;
	uint64_t verticesToLabel = rVerticesSeeds.size();
	string funcName = "Labeling";
	showProgressStart( funcName );

	// Initial label
	uint64_t setLabel = 1; // First label is ONE(!!!)
	// Marching front
	set<Vertex*>  frontVertices;
	set<Vertex*>  frontVerticesNew;
	set<Vertex*>::iterator itVertexSeed;
	set<Vertex*>::iterator itVertexFront;

	for( itVertexSeed=rVerticesSeeds.begin(); itVertexSeed != rVerticesSeeds.end(); itVertexSeed++ ) {
		Vertex* currVertex = (*itVertexSeed);
		if( currVertex->isSolo() ) {
			//! Solo vertices are ignored and Vertex::setLabelNone is called.
			currVertex->setLabelNone();
			continue;
		}
		if( currVertex->isLabelBackGround() ) {
			//cout << "[Mesh::" << __FUNCTION__ << "] Vertex can not be labled - already set to background." << endl;
			//cout << "b";
			continue;
		}
		if( currVertex->isLabled() ) {
			//cout << "[Mesh::" << __FUNCTION__ << "] Vertex is already labled." << endl;
			//cout << "l";
			continue;
		}
		//cout << "X";
		// BEGIN NEW LABEL and initalize front
		frontVertices.insert( currVertex );
		double labelStepToFuncVal = 0.0;
		// Advance front
		while( frontVertices.size() > 0 ) {
			for( itVertexFront=frontVertices.begin(); itVertexFront != frontVertices.end(); itVertexFront++ ) {
				if( (*itVertexFront)->isLabelBackGround() ) {
					//cout << "[Mesh::labelVertices] (1) Stop at Vertex: " << (*itVertexFront)->getIndexOriginal() << endl;
					continue;
				}
				if( (*itVertexFront)->isLabled() ) {
					//cout << "[Mesh::labelVertices] (2) Stop at Vertex: " << (*itVertexFront)->getIndexOriginal() << endl;
					continue;
				}
				(*itVertexFront)->getNeighbourVertices( &frontVerticesNew );
				(*itVertexFront)->setLabel( setLabel );
				verticesLabeled++;
				showProgress( static_cast<double>(verticesLabeled)/verticesToLabel, funcName );
				if( setLabelStepToFuncVal ) {
					(*itVertexFront)->setFuncValue( labelStepToFuncVal++ );
				}
			}
			frontVertices.swap( frontVerticesNew );
			frontVerticesNew.clear();
		}
		frontVertices.clear();
		//cout << "[Mesh::" << __FUNCTION__ << "] Label No: " << setLabel << " containes " << verticesVisited->size() << " Vertices." << endl;
		setLabel++;
		//cout << "--- " << setLabel << " --------------------------------------------" << endl;
	}
	showProgressStop( funcName );

	// Remove seeds
	rVerticesSeeds.clear();
	// tell other methods (e.g. OpenGL) that stuff has changed
	labelsChanged();
	setLabel -= 1; // Correct for indexing begining at ONE.
	cout << "[Mesh::" << __FUNCTION__ << "] " << verticesLabeled << " vertices labeld out of " << verticesToLabel << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] " << setLabel << " Labels set." << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	return setLabel;
}

//! Moves vertices from mSelectedMVerts to mLabelSeedVerts.
void Mesh::labelSelectionToSeeds() {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	mLabelSeedVerts.clear(),
	mLabelSeedVerts.swap( mSelectedMVerts );
	selectedMVertsChanged();
}

//! Labels the selection of multiple vertices (SelMVerts).
//! Non-interactive Wrapping method used by gigamesh-clean!
//!
//! @returns false in case of an error, warning or user cancel. true otherwise.
bool Mesh::labelSelectedVerticesBackGrd() {
	if( mSelectedMVerts.size() == 0 ) {
		// Nothing to do.
		return( false );
	}
	return labelSelectedVertices( mSelectedMVerts, true );
}

//! Labels the selection of multiple vertices (SelMVerts).
//!
//! @returns false in case of an error, warning or user cancel. true otherwise.
bool Mesh::labelSelectedVerticesUser() {
	if( mSelectedMVerts.size() == 0 ) {
		showWarning( "Labeling of selected vertices", "No vertices selected i.e. SelMVerts is empty. No labeling performed." );
		return( false );
	}
	bool setNotSelectedtoBackGrd = true;
	if( !showQuestion( &setNotSelectedtoBackGrd, "Vertices NOT selected",
	                   "Keep the labels of the vertices, which are NOT selected?" ) ) {
		// User cancel
		return( false );
	}
	return labelSelectedVertices( mSelectedMVerts, not( setNotSelectedtoBackGrd ) );
}

//! Labels a given set of vertices.
//!
//! @returns false in case of an error or warning. true otherwise.
bool Mesh::labelSelectedVertices(
                set<Vertex*>& rSelectedVertices, //!< Selection to be labeled.
                bool rSetNotSelectedtoBackGrd    //!< Set the vertices, which are not selected as background.
) {
	// check if there is something to label:
	if( rSelectedVertices.size() == 0 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] WARNING: No vertices selected for labeling." << endl;
		return( false );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Vertices selected: " << rSelectedVertices.size() << endl;
	// first we reset the label and set all NOT to be labled.
	if( rSetNotSelectedtoBackGrd ) {
		labelVerticesBackground();
	}
	// set areas to become labled:
	set<Vertex*> selectedVerticesVector;
	set<Vertex*>::iterator itVertex;
	for( itVertex=rSelectedVertices.begin(); itVertex != rSelectedVertices.end(); itVertex++ ) {
		(*itVertex)->setLabelNone();
		selectedVerticesVector.insert( *itVertex );
	}
	return Mesh::labelVertices( selectedVerticesVector, mLabelSeedVerts );
}

//! Labels vertices having the same function value.
//! Nan-values become background label.
bool Mesh::labelVerticesEqualFV() {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	orderVertsByFuncVal();

	uint64_t labelNr = 0; // was -1 in older versions.
	                           // Now the first label id has to be one (for inverted selection)
	double funcValLast = _NOT_A_NUMBER_DBL_;
	double funcVal;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( !currVertex->getFuncValue( &funcVal ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: getFuncValue failed!" << endl;
			continue;
		}
		if( std::isnan( funcVal ) ) {
			currVertex->setLabelBackGround();
			continue;
		}
		// as vertices are order - a new function value means a new label
		if( funcValLast != funcVal ) {
			funcValLast = funcVal;
			labelNr++;
		}
		currVertex->setLabel( labelNr );
	}
	orderVertsByIndex();
	labelsChanged();
	return true;
}

//! Labels vertices having the same color.
//! Nan-values become background label.
bool Mesh::labelVerticesEqualRGB() {
    cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
    orderVertsByRGB();

    uint64_t labelNr = 0; // was -1 in older versions.
                               // Now the first label id has to be one (for inverted selection)
    unsigned char RGBLast[3] = {255,255,255}; // since we order from low to high, this should work
    unsigned char RGB[3];
    Vertex* currVertex;
    for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
        currVertex = getVertexPos( vertIdx );
        if( !currVertex->copyRGBTo( RGB ) ) {
            cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: copyRGBto failed!" << endl;
            continue;
        }

        // as vertices are order - a new function value means a new label
        // RGBLast is set to 255,255,255, so if all vertices have only 255,255,255 as color, all will get label 0 (unlabeld)
        if( RGBLast[0] != RGB[0] || RGBLast[1] != RGB[1] || RGBLast[2] != RGB[2] ) {
            RGBLast[0] = RGB[0];
            RGBLast[1] = RGB[1];
            RGBLast[2] = RGB[2];
            labelNr++;
        }
        currVertex->setLabel( labelNr );
    }
    orderVertsByIndex();
    labelsChanged();
    return true;
}

//! Sets the selected vertices' label to background
bool Mesh::labelSelMVertsToBack() {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	set<Vertex*>::iterator itVertex;
	for( itVertex=mSelectedMVerts.begin(); itVertex != mSelectedMVerts.end(); itVertex++ ) {
		Vertex* currVertex = (*itVertex);
		currVertex->setLabelBackGround();
	}
	labelsChanged();
    return true;
}

/**
 * This method uses selected vertices (SelMVerts) to label the point cloud of the mesh into as many labels as there are selected vertices.
 * The selection sets a preference for the central location of the classes.
 *  However, the algorithm is allowed to shift the location of the classes.
 *  This means that selections that are too close together may result in clusters in different locations than expected.
 *  Also note that highly curved surfaces can cause the labels to be disconnected as they are part of another area of the triangular grid, since only the positions of the vertices are used for clustering.
 */
bool Mesh::labelKMeansVertPos() {
    cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
    // check if there is something to label:
    if( mSelectedMVerts.size() < 2 ) {
        cout << "[Mesh::" << __FUNCTION__ << "] WARNING: No vertices selected for labeling." << endl;
        showWarning( "Not enough selected Vertices!", "You have to select at least 2 vertices!" );
        return( false );
    }
    //use the selected vertices as intial centroids
    std::vector<Vector3D> initCentroids;
    //extract the vertex pos from the selected vertices
    set<Vertex*>::iterator itVertex;
    for( itVertex=mSelectedMVerts.begin(); itVertex != mSelectedMVerts.end(); itVertex++ ) {
            Vertex* currVertex = (*itVertex);
            initCentroids.push_back(currVertex->getPositionVector());
        }
    computeVertexPositionKMeans(&initCentroids,true);
    labelsChanged();
    return( true );
}
//!K-Means clustering algorithm
//!@param centroids as input expected. Contains the start centroids and thus defines the number of the clusters
//!@param clusterSets returns vertex clusters
//! @param labeling if true, then the vertices are labeled with their cluster id
bool Mesh::computeVertexPositionKMeans(std::vector<Vector3D> *centroids, bool labeling)
{
    cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
    const int cMaxIterations = 30;
    int iter = 0;
    bool centroidsChanged = true;
    while(iter < cMaxIterations && centroidsChanged){
        //calculate new cluster sets
        std::vector<std::set<Vertex*>> newClusterSets(centroids->size());
        if( !assignVerticesToClusterByPosition(centroids, &newClusterSets, labeling)){
            return false;
        }
        //calculate new centroids
        centroidsChanged = false;
        for(unsigned int i=0; i<newClusterSets.size(); i++){
            Vector3D newCentroid = getCentroidByPosition(&newClusterSets.at(i));
            if( newCentroid != centroids->at(i)){
                centroids->at(i) = newCentroid;
                centroidsChanged = true;
            }
        }
        iter++;
        cout << "[Mesh::" << __FUNCTION__ << "]" << "iteration:" << iter << endl;
    }

    labelsChanged();
    return true;
}

bool Mesh::assignVerticesToClusterByPosition(std::vector<Vector3D> *centroids, std::vector<std::set<Vertex *>> *clusterSets, bool labeling)
{
    for(Vertex *vert: mVertices){
        double minDist = std::numeric_limits<double>::infinity();
        unsigned int clusterId = 0; //cluster id = index of the centroid or the cluster sets
        //assign the vertex to the cluster with centroid that has the least distance to the vertex
        for(unsigned i=0; i<centroids->size(); i++){
            //I don't use the square root for the distance because it's not necessary for the comparison
            Vector3D vertPos = vert->getPositionVector();
            double dist = centroids->at(i).distanceToVectorWithouSqrt(&centroids->at(i),&vertPos);
            if(minDist > dist){
                minDist = dist;
                clusterId = i;
            }
        }
        clusterSets->at(clusterId).insert(vert);
        if(labeling){
            vert->setLabel(clusterId+1);
        }

    }
    return true;
}


Vector3D Mesh::getCentroidByPosition(std::set<Vertex*> *clusterSet)
{
    std::set<Vertex*>::iterator itr;
    Vector3D newCentroid(0.0,0.0,0.0);
    for (itr = clusterSet->begin(); itr != clusterSet->end(); itr++){
        newCentroid = newCentroid + (*itr)->getPositionVector();
    }
    return newCentroid/clusterSet->size();
}

std::vector<std::set<Vertex*>> Mesh::computeVertexNormalKMeans(std::vector<Vector3D> *centroids, bool labeling)
{
    cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
    const int cMaxIterations = 50;
    int iter = 0;
    bool centroidsChanged = true;
    std::vector<std::set<Vertex*>> newClusterSets(centroids->size());
    while(iter < cMaxIterations && centroidsChanged){
        //calculate new cluster sets
        newClusterSets = std::vector<std::set<Vertex*>>(centroids->size());
        assignVerticesToClusterByNormal(centroids, &newClusterSets, labeling);
        //calculate new centroids
        centroidsChanged = false;
        for(unsigned int i=0; i<newClusterSets.size(); i++){
            Vector3D newCentroid = getCentroidByNormal(&newClusterSets.at(i));
            if( newCentroid != centroids->at(i)){
                centroids->at(i) = newCentroid;
                centroidsChanged = true;
            }
        }
        iter++;
        cout << "[Mesh::" << __FUNCTION__ << "]" << "iteration:" << iter << endl;
    }

    labelsChanged();
    return newClusterSets;
}

bool Mesh::assignVerticesToClusterByNormal(std::vector<Vector3D> *centroids, std::vector<std::set<Vertex *>> *clusterSets, bool labeling)
{
    for(Vertex *vert: mVertices){
        double minDist = std::numeric_limits<double>::infinity();
        unsigned int clusterId = 0; //cluster id = index of the centroid or the cluster sets
        //assign the vertex to the cluster with centroid that has the least distance to the vertex
        for(unsigned i=0; i<centroids->size(); i++){
            //I don't use the square root for the distance because it's not necessary for the comparison
            Vector3D vertNormal = vert->getNormal();
            double dist = centroids->at(i).distanceToVectorWithouSqrt(&centroids->at(i),&vertNormal);
            if(minDist > dist){
                minDist = dist;
                clusterId = i;
            }
        }
        clusterSets->at(clusterId).insert(vert);
        if(labeling){
            vert->setLabel(clusterId+1);
        }

    }
    return true;
}


Vector3D Mesh::getCentroidByNormal(std::set<Vertex*> *clusterSet)
{
    std::set<Vertex*>::iterator itr;
    Vector3D newCentroid(0.0,0.0,0.0);
    int nrOfdefinedNormals = 0;
    for (itr = clusterSet->begin(); itr != clusterSet->end(); itr++){
        Vector3D normal = (*itr)->getNormal();
        //check if normal is defined
        //some vertices return NaN
        if (!isnan(normal.getX())){
            newCentroid = newCentroid + normal;
            nrOfdefinedNormals++;
        }
    }
    return newCentroid/nrOfdefinedNormals;
}



// ---------------------------------------------------------------------------------------------------

//! Compute the integral invariants and their extrema of the polylines using the run-length. See PolyLine.
bool Mesh::compPolylinesIntInvRunLen( double rIIRadius, PolyLine::ePolyIntInvDirection rDirection ) {
	unsigned int ctrError = 0;
	PolyLine* currPoly;
	for( unsigned int i=0; i<getPolyLineNr(); i++ ) {
		currPoly = getPolyLinePos( i );
		if( !currPoly->isClosed() ) {
			cout << "[Mesh::" << __FUNCTION__ << "] Polyline " << i << " ignored as it is not closed." << endl;
			continue;
		}
		if( !currPoly->compIntInv( rIIRadius, rDirection ) ) {
			ctrError++;
		}
		// Compute COG
		currPoly->compVertAvgCog();
		// Compute normal
		currPoly->compVertAvgNormal();
	}
	if( ctrError > 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: " << getPolyLineNr() << " Polylines processed - " << ctrError << " errors occured!" << endl;
		return false;
	}
	cout << "[Mesh::" << __FUNCTION__ << "] " << getPolyLineNr() << " Polylines processed." << endl;
	return true;
}

//! Compute the integral invariants and their extrema of the polylines using the angle. See PolyLine.
bool Mesh::compPolylinesIntInvAngle( double rIIRadius ) {
	cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
	unsigned int ctrError = 0;
	PolyLine* currPoly;
	for( unsigned int i=0; i<getPolyLineNr(); i++ ) {
		currPoly = getPolyLinePos( i );
		if( !currPoly->isClosed() ) {
			cout << "[Mesh::" << __FUNCTION__ << "] Polyline " << i << " ignored as it is not closed." << endl;
			continue;
		}
		if( !currPoly->compIntInvAngle( rIIRadius ) ) {
			ctrError++;
		}
		// Compute COG
		currPoly->compVertAvgCog();
		// Compute normal
		currPoly->compVertAvgNormal();
	}
	if( ctrError > 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: " << getPolyLineNr() << " Polylines processed - " << ctrError << " errors occured!" << endl;
		return false;
	}
	cout << "[Mesh::" << __FUNCTION__ << "] " << getPolyLineNr() << " Polylines processed." << endl;
	return true;
}

//! Compute a curvature estimation of the polylines and its the extrema.
void Mesh::getPolylineExtrema( bool absolut ) {
	double smoothLength;
	getParamFloatMesh( SMOOTH_LENGTH, &smoothLength );
	cout << "[Mesh::" << __FUNCTION__ << "] Smooth length: " << smoothLength << endl;
	PolyLine* currPoly;
	for( unsigned int i=0; i<getPolyLineNr(); i++ ) {
		currPoly = getPolyLinePos( i );
		currPoly->getExtrema( &mSelectedMVerts, smoothLength, absolut );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] mSelectedMVerts: " << mSelectedMVerts.size() << endl;
}

//! Copy the polylines' normals to their vertices-
bool Mesh::setPolylinesNormalToVert() {
	bool retVal = true;
	PolyLine* currPoly;
	for( unsigned int i=0; i<getPolyLineNr(); i++ ) {
		currPoly = getPolyLinePos( i );
		if( !currPoly->copyNormalToVertices() ) {
			// An error has occured.
			retVal = false;
		}
	}
	return retVal;
}

//! Compute polylines using the mesh plane i.e. profile line.
//!
//! Note: Function values are not changed as they are
//! stashed and restored.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::planeIntersectionToPolyline() {
	// Stash function values
	FUNCTION_VALUES_STASH

	bool absolutDistance   = false; // Parameter for the function value computation.
	bool silentComputation = true;  // Parameter for the function value computation.
	// Set function values using distance to plane
	Vector3D rPlaneHNF;
	mPlane.getPlaneHNF( &rPlaneHNF );
	funcVertDistanceToPlane( rPlaneHNF, absolutDistance, silentComputation );
	// Compute polyline using iso-value of zero.
	// The plane will be copied/attached to the polylines.
	isolineToPolyline( 0.0, &mPlane );

	// Retrieve stashed values
	FUNCTION_VALUES_STASH_RETRIEVE

	return( true );
}

//! Compute polylines using the axis and the selected positons
//! i.e. multiple profile line.
//!
//! Note:
//! (i) Function values are not changed as they are stashed and restored.
//! (ii) The mesh plane will not be changed.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::planeIntersectionsAxisAndPositions() {

	// Check and fetch axis.
	Vector3D axisTop;
	Vector3D axisBottom;
	if( !getConeAxis( &axisTop, &axisBottom ) ) {
		showWarning( "Polylines from Axis and Positions", "No axis defined!" );
		cout << "[Mesh::" << __FUNCTION__ << "] No axis defined!" << endl;
		return( false );
	}

	// Check positions.
	if( mSelectedPositions.size() == 0 ) {
		showWarning( "Polylines from Axis and Positions", "No positions (SelPos) selected!" );
		cout << "[Mesh::" << __FUNCTION__ << "] No positions (SelPos) selected!" << endl;
		return( false );
	}

	// Stash function values
	FUNCTION_VALUES_STASH

	bool absolutDistance   = false; // Parameter for the function value computation.
	bool silentComputation = true;  // Parameter for the function value computation.
	for( auto const& currPosTuple: mSelectedPositions ) {
		Vector3D currPos( std::get<0>( currPosTuple ) );
		Plane currPlane;
		currPlane.setPlaneByAxisAndPosition( axisTop, axisBottom, currPos );

		// Set function values using distance to plane
		Vector3D rPlaneHNF;
		currPlane.getPlaneHNF( &rPlaneHNF );
		funcVertDistanceToPlane( rPlaneHNF, absolutDistance, silentComputation );
		// Compute polyline using iso-value of zero.
		// The plane will be copied/attached to the polylines.
		isolineToPolyline( 0.0, &currPlane );
	}

	// Retrieve stashed values
	FUNCTION_VALUES_STASH_RETRIEVE

	return( true );
}

//! Compute isolines using multiple function values entered by user interaction.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::isolineToPolylineMultiple() {
	vector<double> multipleIsoValues;
	if( !showEnterText( multipleIsoValues, "Enter multiple isovalues" ) ) {
		return( false );
	}
	bool retVal = true;
	for( auto const& isoValue: multipleIsoValues ) {
		retVal |= isolineToPolyline( isoValue );
	}
	return( retVal );
}

//! Compute isolines using the function values using the stored threshold.
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool Mesh::isolineToPolyline() {
	double isoValue = _NOT_A_NUMBER_DBL_;
	double isoValueSelPrim = _NOT_A_NUMBER_DBL_;
	getParamFloatMesh( MeshParams::FUNC_VALUE_THRES, &isoValue );

	if( mPrimSelected != nullptr ) {
		if( mPrimSelected->getFuncValue( &isoValueSelPrim ) ) {
			if( isfinite( isoValueSelPrim ) ) {
				bool chooseValFromSelectedPrim = false;
				bool userCancel = false;
				userCancel = showQuestion( &chooseValFromSelectedPrim,
				                           "Value from Selected Primitive",
				                           "Do you want to use the function value of the selected primitive?<br /><br />"
				                           "YES for <b>" + to_string( isoValueSelPrim ) + "</b><br /><br />"
				                           "NO for <b>" + to_string( isoValue ) + "</b><br /><br />"
				                           "CANCEL to abort."
				                          );
				if( !userCancel ) {
					return( false );
				}
				if( chooseValFromSelectedPrim ) {
					isoValue = isoValueSelPrim;
				}
			}
		}
	}

	bool retVal = isolineToPolyline( isoValue );
	return( retVal );
}

//! Compute isolines using the function values and a given threshold.
//!
//! Optional: plane, which is typically stored with isolines based on distances to planes
//!           and used for projection to 2D during SVG export.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::isolineToPolyline(
    double     rIsoValue,          //!< Isovalue to compute the isolines.
    Plane*     rPlaneIntersect     //!< Optional plane for intersections. Will be stored with the isolines.
) {
	// Sanity check
	if( !isfinite( rIsoValue ) ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Given iso-value is not finite!" << std::endl;
		return( false );
	}

	//! \todo Fetching the label related information, makes only sense for outlines of connected components. Therefore optimizations might be possible at this point.
	// Fetch label normals first.
	vector<Vector3D> labelCenters;
	vector<Vector3D> labelNormals;
	estLabelNormalSizeCenterVert( &labelCenters, &labelNormals );

	for( auto& labelCenter : labelCenters) {
		labelCenter /= labelCenter.getH();
	}

	// Bit array:
	      uint64_t* facesVisitedBitArray;
	const uint64_t  faceBlocksNr = getBitArrayFaces( &facesVisitedBitArray );

	auto setFaceVisited = [&facesVisitedBitArray] (Face* face) {
		uint64_t  bOffset;
		uint64_t  bNr;
		face->getIndexOffsetBit(&bOffset, &bNr);
		facesVisitedBitArray[bOffset] |= static_cast<uint64_t>(1) << bNr;
	};

	auto traceIsoLine = [&setFaceVisited, &facesVisitedBitArray] (Face* startFace, bool forward, double rIsoValue, PolyLine* isoLine)
	{
		Vector3D  isoPoint;
		Face*     nextFace = nullptr;
		Face*     excludeFace = nullptr;
		uint64_t  bitOffset;
		uint64_t  bitNr;

		// Trace in forward direction:
		//----------------------------
		if(!startFace->getFuncValIsoPoint( rIsoValue, &isoPoint, &nextFace, forward, &excludeFace ))
		{
			return; //skip face, because it only touches the isoLine on its vertices
		}
		if(excludeFace != nullptr)
		{
			setFaceVisited(excludeFace);
		}
		// ... add to polyline with normal ....
		Vector3D normalPos = startFace->getNormal( true );

		forward ? isoLine->addFront( isoPoint, normalPos, startFace )
		        : isoLine->addBack ( isoPoint, normalPos, startFace );

		Face* checkFace = nextFace;
		while( checkFace != nullptr ) {
			checkFace->getIndexOffsetBit( &bitOffset, &bitNr );

			// check if we have been there to prevent infinite loops:
			if( facesVisitedBitArray[bitOffset] & static_cast<uint64_t>(1)<<bitNr ) {
				break;
			}
			// set visited
			facesVisitedBitArray[bitOffset] |= static_cast<uint64_t>(1)<<bitNr;

			// get the point ...
			if(!checkFace->getFuncValIsoPoint( rIsoValue, &isoPoint, &nextFace, forward, &excludeFace ))
			{
				nextFace = nullptr;
				continue;
			}
			if(excludeFace != nullptr)
			{
				setFaceVisited(excludeFace);
			}
			// ... add to polyline with normal ...
			normalPos = checkFace->getNormal( true );

			forward ? isoLine->addFront( isoPoint, normalPos, checkFace )
			        : isoLine->addBack ( isoPoint, normalPos, checkFace );

			// .... move on:
			checkFace = nextFace;
		}
	};

	const uint64_t numBits = sizeof(uint64_t) * 8;

	for( uint64_t i=0; i<faceBlocksNr; ++i ) {
		if( facesVisitedBitArray[i] == 0xFFFFFFFFFFFFFFFF ) {
			// whole block visited.
			continue;
		}
		for( uint64_t j=0; j<numBits; ++j ) {
			const uint64_t currFaceIndex = i*numBits+j;

			// skip unused bits within the last block:
			if( ( i==faceBlocksNr-1 ) && ( currFaceIndex >= getFaceNr() ) ) {
				break;
			}

			uint64_t currentBit = static_cast<uint64_t>(1) << j;
			if( facesVisitedBitArray[i] & currentBit ) {
				// face already visited.
				continue;
			}
			//cout << "[Mesh::isolineToPolyline] Check Face in Block No. " << i << " Bit No. " << j << endl;
			Face* checkFace = getFacePos( currFaceIndex );
			if( !checkFace->isOnFuncValIsoLine( rIsoValue ) ) {
				// when the face is not along the isoline, we mark it visited and move on:
				facesVisitedBitArray[i] |= currentBit;
				continue;
			}
			// store reference for our starting point
			Face*     checkFaceFirst = checkFace;
			//
			bool isLabelLabelBorder;
			int  labelFromBorder = -1;
			bool isLabelBorder = checkFaceFirst->vertLabelLabelBorder( &isLabelLabelBorder, &labelFromBorder );
			if( isLabelBorder ) {
				if( isLabelLabelBorder ) {
					LOG::debug() << "[Mesh::" << __FUNCTION__ << "] is on a label-label border.\n";
				} else {
					LOG::debug() << "[Mesh::" << __FUNCTION__ << "] is on a nolabel-label border. labelFromBorder: " << labelFromBorder << "\n";
				}
			} else {
				LOG::debug() << "[Mesh::" << __FUNCTION__ << "] is NOT on a label related border.\n";
			}

			PolyLine* isoLine;
			if( isLabelBorder && !isLabelLabelBorder ) {
				// Remeber: Labels start at index 1!
				isoLine = new PolyLine( labelCenters.at( labelFromBorder-1 ),
				                        labelNormals.at( labelFromBorder-1 ),
				                        labelFromBorder );
			} else {
				if( rPlaneIntersect != nullptr ) {
					isoLine = new PolyLine( *rPlaneIntersect );
				} else {
					isoLine = new PolyLine();
				}
			}

			setFaceVisited(checkFaceFirst);

			// Trace in forward and backward direction:
			//----------------------------
			traceIsoLine(checkFaceFirst, true , rIsoValue, isoLine);
			traceIsoLine(checkFaceFirst, false, rIsoValue, isoLine);

			// Add vertices of the polyline:
			isoLine->addVerticesTo( &mVertices );
			// Add polyline:
			mPolyLines.push_back( isoLine );
		}
	}
    polyLinesChanged();
	delete[] facesVisitedBitArray;
	return( true );
}

//! Use the rotational axis to extrude the poylines.
//! @returns false in case of an error. True otherwise.
bool Mesh::extrudePolylines() {
	std::cout << "[Mesh::" << __FUNCTION__ << "] Extruding polylines ..." << std::endl;

	// Fetch axis, if present
	Vector3D axisTop;
	Vector3D axisBottom;
	if( !getConeAxis( &axisTop, &axisBottom ) ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: No axis defined!" << std::endl;
		return( false );
	}

	if( getPolyLineNr() <= 0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: No polylines defined!" << std::endl;
		return( false );
	}

	// Apply to all polylines
	bool retVal = true;
	PolyLine* currPoly;
	vector<Vertex*> verticesToAppend; // actually: VertexOfFace
	vector<Face*>   facesToAppend;
	for( unsigned int i=0; i<getPolyLineNr(); i++ ) {
		currPoly = getPolyLinePos( i );
		if( !currPoly->extrudeAxis( &axisTop, &axisBottom, &verticesToAppend, &facesToAppend ) ) {
			// An error has occured.
			retVal = false;
		}
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] Polylines processed." << std::endl;

	// Insert Faces
	for( auto const& currFace: facesToAppend ) {
		currFace->reconnectToFaces();
		mFaces.push_back( currFace );
	}
	// and vertices
	if( !insertVertices( &verticesToAppend ) ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: insertVertices failed!" << std::endl;
		return( false );
	}

	return retVal;
}

bool Mesh::labelVertSurface( uint64_t& rlabelsNr, double** rArea ) {
	//! Returns an array of information about the labled area using Vertex based labeling.
	//! Returns an empty array (rArea==NULL), when no labels were found.
	//! Returns false in case of an error.
	//!
	//! Remark: The array is order by label no.
	if( rArea == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return false;
	}

	// One set of faces per label:
	set<Face*>* labelFaces;
	if( !labelFacesVert( &labelFaces, rlabelsNr ) ) {
		rlabelsNr = 0;
		(*rArea)  = nullptr;
		return false;
	}
	// Allocate memory for area:
	(*rArea) = new double[rlabelsNr];

	// Estimate area and clear memory.
	for( uint64_t i=0; i<rlabelsNr; i++ ) {
		(*rArea)[i] = 0.0;
		for( Face* face : labelFaces[i] ) {
			(*rArea)[i] += face->getAreaNormal();
		}
		cout << "[Mesh::" << __FUNCTION__ << "] Label " << i << " area: " << (*rArea)[i] << endl;
		labelFaces[i].clear();
	}
	// Clear allocated memory
	//! \todo check why delete causes a segmentation fault, when used.
	//delete labelFaces;

	delete[] labelFaces;

	return true;
}

//! Groups faces based on vertex labels.
//!
//! @returns false in case of an error and when no label was found.
bool Mesh::labelFacesVert(
                set<Face*>** rLabelFaces,   //!< Set of faces (return value)
                uint64_t&  rlabelsNr   //!< Number of the connected component (return value)
) {
	if( !labelCount( Primitive::IS_VERTEX, rlabelsNr ) ) {
		return false;
	}

	// One set of faces per label:
	(*rLabelFaces) = new set<Face*>[rlabelsNr];

	// Fetch faces belonging to the same vertex based label:
	Vertex* currVertex;
	uint64_t currLabel;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( !currVertex->getLabel( currLabel ) ) {
			// Not labeled - or background =>
			continue;
		}
		currVertex->getFaces( &(*rLabelFaces)[currLabel-1] );
	}
	return true;
}

// Polylines ---------------------------------------------------------------------------------------------------------------------------------------------------

//! Stub for higher levels.
//! Has to be called after polylines were changed,
//! because OpenGL has to refresh the display.
void Mesh::polyLinesChanged() {
	// stub -- nothing to do.
}

//! Removes ALL polylines - clears the polyline list.
//!
//! @returns false, when no polyline was removed.
bool Mesh::removePolylinesAll() {
	if( mPolyLines.size() <= 0 ) {
		return( false );
	}
	vector<PolyLine*>::iterator itPolyLines;
	for( itPolyLines=mPolyLines.begin(); itPolyLines!=mPolyLines.end(); itPolyLines++ ) {
		delete (*itPolyLines);
	}
	mPolyLines.clear();
	polyLinesChanged();
	return( true );
}

//! Removes SELECTED polylines.
//!
//! @returns false, when no polyline was removed.
bool Mesh::removePolylinesSelected() {
	bool polyLinesRemoved = false;
	vector<PolyLine*> polyLinesKeep;
	vector<PolyLine*>::iterator itPolyLines;
	set<PolyLine*>::iterator itPolyLinesSel;
	for( itPolyLines=mPolyLines.begin(); itPolyLines!=mPolyLines.end(); itPolyLines++ ) {
		itPolyLinesSel = mPolyLinesSelected.find( (*itPolyLines) );
		if( itPolyLinesSel == mPolyLinesSelected.end() ) {
			// Polyline not selected - keep
			polyLinesKeep.push_back( (*itPolyLines) );
			continue;
		}
		delete (*itPolyLines);
		mPolyLinesSelected.erase( itPolyLinesSel );
		polyLinesRemoved = true;
	}
	if( polyLinesRemoved ) {
		mPolyLines.clear();
		mPolyLines.swap( polyLinesKeep );
		polyLinesChanged();
	}
	return polyLinesRemoved;
}

void Mesh::convertLabelBordersToPolylines() {
	//! Converts the borders of labels into polylines.

	int               labelLineCollectionNr; //!< Size of labelLineCollection
    //set<labelLine*>** labelLineCollection;   //!< Lines/Edges (experimental) see face.h
	PolyLine*         selectedPoy;           //!< temporary pointer
	Vector3D*         labelCOGs;             //!< Center of gravities for the labels.
	Vector3D*         labelNormals;          //!< Average normals for the labels.
	uint64_t*    faceCountPerLabel;     //!< Number of faces per label.

	removePolylinesAll();

	uint64_t labelsNr;
	// We have to determine the maximum number of labels from the VERTICES as there are no polylines (yet)!
	if( !labelCount( Primitive::IS_VERTEX, labelsNr ) ) {
		cout << "[Mesh::" << __FUNCTION__ << "] No labels found. " << endl;
		return;
	}

	// Prepare one array per label:
	labelLineCollectionNr = labelsNr;
    //labelLineCollection   = new set<labelLine*>*[labelsNr];
    vector<set<labelLine*>> labelLineCollection;//(labelsNr+1);
    labelCOGs             = new Vector3D[labelsNr+1];
    labelNormals          = new Vector3D[labelsNr+1];
    faceCountPerLabel     = new uint64_t[labelsNr+1];
    for( uint64_t i=0; i<=labelsNr; i++ ) {
        //labelLineCollection[i] = new set<labelLine*>;
        labelLineCollection.push_back(*new set<labelLine*>);
		// per default we get the origin - a position vector:
		labelNormals[i].setH( 0.0 );
		// init as calloc might not:
		faceCountPerLabel[i] = 0;
	}

	// Sort labelines by label nr
	uint64_t currentLabel = 0;
	Face* currFace = nullptr;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
        if( !currFace->getLabel( currentLabel ) ) {
            continue;
        }
        //currFace->getLabelLines( labelLineCollection[currentLabel] );
        currFace->getLabelLines( &labelLineCollection.at(currentLabel) );
		labelCOGs[currentLabel]    += currFace->getCenterOfGravity();
        labelNormals[currentLabel] += currFace->getNormal( false );
		faceCountPerLabel[currentLabel]++;
	}

	// Estimate polylines per label - when a label has more than one polyline, it has one or more holes.
    for( int i=0; i<=labelLineCollectionNr; i++ ) {
		//cout << "[Mesh::convertSelectedVerticesToPolyline] labelLineCollection[" << i << "]: " << labelLineCollection[i]->size() << endl;
        if( labelLineCollection.at(i).size() <= 0 ) {
			continue;
		}
        //labelCOGs[i].dumpInfo();
		labelCOGs[i] /= static_cast<float>(faceCountPerLabel[i]);
		int linesLeft;
		do {
			selectedPoy = new PolyLine( labelCOGs[i], labelNormals[i], i );
			//selectedPoy->setLabel( i );
            linesLeft = selectedPoy->compileLine( &labelLineCollection.at(i) );
			mPolyLines.push_back( selectedPoy );
		} while( linesLeft > 0 );
	}

	// Clear labellines
    if( labelLineCollection.size() != 0 ) {
		for( int i=0; i<labelLineCollectionNr; i++ ) {
			//! \todo check about cleaning labelLine
            labelLineCollection.at(i).clear();
		}
        labelLineCollection.clear();
	}
    delete[] labelCOGs;
    delete[] labelNormals;
    delete[] faceCountPerLabel;

	polyLinesChanged();

}

//! Convertes triangle edges along mesh border to a polyline, e.g. for hole filling.
bool Mesh::convertBordersToPolylines() {
	// Progress bar
	showProgressStart( "Convert Mesh Borders to Polylines" );
	// Bit array:
	uint64_t* vertBitArrayVisited;
	int vertNrLongs = getBitArrayVerts( &vertBitArrayVisited, BIT_ARRAY_MARK_BORDER );
	// Counters:
	unsigned int ctrContainsSingular = 0;
	unsigned int ctrBorders = 0;
	unsigned int ctrBordersOne  = 0;
	unsigned int ctrBordersTwo  = 0;
	unsigned int ctrBordersTri  = 0;
	unsigned int ctrBordersQuad = 0;
	// Process all vertices on the border:
	for( int i=0; i<vertNrLongs; i++ ) {
		if( vertBitArrayVisited[i] == 0 ) {
			// No border vertex within block
			continue;
		}
		showProgress( static_cast<double>(i+1)/static_cast<double>(vertNrLongs), "Convert Mesh Borders to Polylines" );
		// Some border vertex encountered:
		for( int bitIdx=0; bitIdx<64; bitIdx++ ) {
			if( (static_cast<uint64_t>(1)<<bitIdx) & vertBitArrayVisited[i] ) {
				int currIdx = (i*8*sizeof( uint64_t ))+bitIdx;
				PolyLine* borderLine = new PolyLine();
				Vertex* currVert = getVertexPos( currIdx );
				Vertex* firstVert = currVert;
				bool containsSingular = false;
				while( currVert != nullptr ) {
					currVert->unmarkVisited( vertBitArrayVisited );
					if( currVert->isDoubleCone() ) {
						//cerr << "[Mesh::" << __FUNCTION__  << "] ERROR: Polyline may be wrong due to singularity!" << endl;
						containsSingular = true;
					}
					borderLine->addFront( currVert );
					//cout << "[Mesh::" << __FUNCTION__  << "] nextVert: " << currVert->getIndexStr() << endl;
					currVert = currVert->getAdjacentNextBorderVertex( vertBitArrayVisited );
				}
				if( containsSingular ) {
					ctrContainsSingular++;
				}
				// Close polyline:
				borderLine->addFront( firstVert );
				// Add polyline:
				mPolyLines.push_back( borderLine );

				// Debug info
				if( borderLine->length() == 2 ) {
					ctrBordersOne++;
				}
				if( borderLine->length() == 3 ) {
					ctrBordersTwo++;
				}
				if( borderLine->length() == 4 ) { // +1 because the start/end vertex is added twice for closed lines.
					ctrBordersTri++;
				}
				if( borderLine->length() == 5 ) { // +1 because the start/end vertex is added twice for closed lines.
					ctrBordersQuad++;
				}
				ctrBorders++;
			}
		}

	}
	polyLinesChanged();
	delete[] vertBitArrayVisited;
	showProgressStop( "Convert Mesh Borders to Polylines" );
	cout << "[Mesh::" << __FUNCTION__  << "] Borders added: " << ctrBorders << endl;
	cout << "[Mesh::" << __FUNCTION__  << "] Borders length 1: " << ctrBordersOne << " (>0 indicates an error)" << endl;
	cout << "[Mesh::" << __FUNCTION__  << "] Borders length 2: " << ctrBordersTwo << " (>0 indicates an error)" << endl;
	cout << "[Mesh::" << __FUNCTION__  << "] Borders triangle: " << ctrBordersTri << endl;
	cout << "[Mesh::" << __FUNCTION__  << "] Borders quadtriangle: " << ctrBordersQuad << endl;
	cout << "[Mesh::" << __FUNCTION__  << "] Borders may be wrong due to singularities: " << ctrContainsSingular << endl;
	if( ctrContainsSingular > 0 ) {
		showWarning( "[Mesh::" + string( __FUNCTION__ ) + "]", "Borders may be wrong due to singularities: " + to_string( ctrContainsSingular ) );
	}
	return true;
}

void Mesh::convertSelectedVerticesToPolyline() {
	//! Converts selected vertices to polylines.

	Vertex*   vertA = nullptr;
	Vertex*   vert1 = nullptr;
	Vertex*   vertNext = nullptr;
	bool      reverse = true;
	PolyLine* selectedPoy;

	removePolylinesAll();

	set<Vertex*>::iterator itVertex;
	while( mSelectedMVerts.size() > 0 ) {
		if( vertNext == nullptr ) {
			if( reverse ) {
				// start new polyline
				//cout << "New Polyline: " << mSelectedMVerts.size() << endl;
				selectedPoy = new PolyLine( Vector3D( _NOT_A_NUMBER_, _NOT_A_NUMBER_, _NOT_A_NUMBER_, 1.0 ), Vector3D( _NOT_A_NUMBER_, _NOT_A_NUMBER_, _NOT_A_NUMBER_, 0.0 ) );
				mPolyLines.push_back( selectedPoy );
				itVertex = mSelectedMVerts.begin();
				vertA = (*itVertex);
				vert1 = (*itVertex);
				mSelectedMVerts.erase( itVertex );
				reverse = false;
			} else {
				// go back to the first vertex
				//cout << "--- reverse" << endl;
				vertA = vert1;
				vert1 = nullptr;
				reverse = true;
			}
		} else {
			vertA = vertNext;
			// we do not need to call .erase as this happens in Face::getConnection
			if( reverse ) {
				selectedPoy->addFront( vertA );
			} else {
				selectedPoy->addBack( vertA );
			}
		}
		if( vertA != nullptr ) {
			//cout << "" << vertA->getIndexOriginal() << " - ";
			vertNext = vertA->getConnection( &mSelectedMVerts, reverse );
			//if( vertNext != NULL ) {
			//	cout << "" << vertNext->getIndexOriginal() << " | ";
			//} else {
			//	cout << "NULL | ";
			//}
		} else {
			cerr << "[Mesh::convertSelectedVerticesToPolyline] Error vertA is NULL!" << endl;
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] nrPolyLines: " << mPolyLines.size() << endl;
	selectedMVertsChanged();
}

// Function Value methods (OLD style) Do not copy & paste !!!  -----------------------------------------------------------------

bool Mesh::getVertIndices( double** funcValues, Vertex*** vertices, int* vertCount ) {
	//! Returns an array with vertex indices as float to be used for OpenGL visualization.
	//! Returns false in case of an error.
	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = curVertex;
		(*funcValues)[vertIdx] = static_cast<double>(curVertex->getIndexOriginal());
	}
	return true;
}

//! Computes the distance of each Vertex to a given position vector and returns the minimum and the maximum.
bool Mesh::getDistanceVerticesToPosition( Vector3D rPos, double* rDistMin, double* rDistMax ) {
	(*rDistMin) = +_INFINITE_DBL_;
	(*rDistMax) = -_INFINITE_DBL_;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		double dist = distanceVV( currVertex, &rPos );
		if( dist < (*rDistMin) ) {
			(*rDistMin) = dist;
		}
		if( dist > (*rDistMax) ) {
			(*rDistMax) = dist;
		}
	}
	return true;
}


//! Compute the hue value of the color per vertex to be stored as function value.
bool Mesh::getHueValues( double** funcValues, Vertex*** vertices, int* vertCount ) {
	*vertices   = new Vertex*[getVertexNr()];
	*funcValues = new double[getVertexNr()];
	*vertCount  = getVertexNr();

	double texHSV[3];
	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		(*vertices)[vertIdx] = curVertex;
		if( curVertex->getHSV( texHSV ) ) {
			(*funcValues)[vertIdx] = texHSV[0];
		}
	}
	return true;
}

// Compute function values (new style) -------------------------------------------------------------------------------------------------------------------------

bool Mesh::setFaceFuncValSortIdx() {
	//! Sets the function values of the faces equal to the index of the current sorting order.
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		currFace->setFuncValue( static_cast<double>(faceIdx) );
	}
	changedFaceFuncVal();
	return true;
}

//! Uses the SelPrim/SelVert as seed for the marching front.
//! The function values of the vertices within the given radius
//! will be set to the euclidean distance of the seed. All
//! other function values will be set to not-a-number (NaN).
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::setFaceFuncValMarchRadiusIdx(
                Primitive* rSeed,  //!< Seed primitive, which can only be a vertex in the current implementation.
                double rRadius     //!< Radius i.e. maximum distance to the seed as stop criteria for the marching front.
) {
	// Sanity
	if( rSeed == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: No primitve given!" << endl;
		return( false );
	}
	if( rSeed->getType() != Primitive::IS_VERTEX ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Primitive has to be a vertex!" << endl;
		return( false );
	}

// Can not be visualized
//	Face* currFace;
//	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
//		currFace = getFacePos( faceIdx );
//		currFace->setFuncValue( _NOT_A_NUMBER_DBL_ );
//	}

	Vertex* seedVert = static_cast<Vertex*>(rSeed);
	if( seedVert->isSolo() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Vertex is solo - nothing to compute!" << endl;
		return( false );
	}

	// Allocate the bit arrays for vertices:
	uint64_t* vertBitArrayVisited;
	uint64_t vertNrLongs = getBitArrayVerts( &vertBitArrayVisited );
	// Allocate the bit arrays for faces:
	uint64_t* faceBitArrayVisited;
	uint64_t faceNrLongs = getBitArrayFaces( &faceBitArrayVisited );
	vector<Face*> facesInSphere;
	fetchSphereBitArray1R( seedVert, facesInSphere, rRadius,
	                       vertNrLongs, vertBitArrayVisited,
	                       faceNrLongs, faceBitArrayVisited, true );

// Can not be visualized
//	changedFaceFuncVal();

	// Tag vertices visited
	setVertFuncVal( _NOT_A_NUMBER_DBL_ );
	for( auto const& currFace: facesInSphere ) {
		double distanceToSeed;
		distanceToSeed = currFace->getVertA()->estDistanceTo( seedVert );
		currFace->getVertA()->setFuncValue( distanceToSeed );
		distanceToSeed = currFace->getVertB()->estDistanceTo( seedVert );
		currFace->getVertB()->setFuncValue( distanceToSeed );
		distanceToSeed = currFace->getVertC()->estDistanceTo( seedVert );
		currFace->getVertC()->setFuncValue( distanceToSeed );
	}
	changedVertFuncVal();

	return true;
}

//! Slope: compute the angle between the vertex normal and the normal of the mesh plane.
//! Angle is set in radiant.
//! @returns true, when a new function value was set for all vertices. False otherwise or in case of an error.
bool Mesh::setVertFuncValSlope() {
	Vector3D planeHNF;
	if( !mPlane.getPlaneHNF( &planeHNF ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Fetching mesh plane failed!" << endl;
		return false;
	}
	if( !isnormal( planeHNF.getLength3() ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Normal of the mesh plane undefined!" << endl;
		return false;
	}

	bool allSet = true;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		Vector3D vertexNormal = currVertex->getNormal( true );
		if( !isnormal( vertexNormal.getLength3() ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] Normal of vertex " << vertIdx << " undefined!" << endl;
			allSet = false;
			continue;
		}
		double slopeAngle = angle( planeHNF,vertexNormal );
		if( !currVertex->setFuncValue( slopeAngle ) ) {
			allSet = false;
		}
	}
	changedVertFuncVal();
	return allSet;
}

//! Compute the angle between the vertex normal and the vector between vertex position and its projection onto cone axis.
//! Unsigned angle is set in radian.
//! @returns true, when a new function value was set for all vertices. False otherwise or in case of an error.
bool Mesh::setVertFuncValAngleToRadial() {
	Vector3D axisTop;
	Vector3D axisBottom;
	if( !getConeAxis(&axisTop, &axisBottom ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Fetching cone axis failed!" << endl;
		return false;
	};

	bool allSet = true;
	Vertex* currentVertex;
	for( unsigned int vertIdx = 0; vertIdx < getVertexNr(); ++vertIdx ) {
		currentVertex = getVertexPos( vertIdx );
		Vector3D point( currentVertex->getX(), currentVertex->getY(), currentVertex->getZ() );
		double angle = currentVertex->getAngleToRadial( axisTop, axisBottom );
		if( !currentVertex->setFuncValue( angle ) ) {
			allSet = false;
		};
	};
	changedVertFuncVal();
	return allSet;
}

//! Compute the angle between vertex normal and projection vector of vertex onto cone axis with vertex normal projected onto cone axis.
//! Unsigned angle is set in radian.
//! @returns true, when a new function value was set for all vertices. False otherwise or in case of an error.
bool Mesh::setVertFuncValAxisAngleToRadial() {
	Vector3D axisTop;
	Vector3D axisBottom;
	if( !getConeAxis( &axisTop, &axisBottom ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Fetching cone axis failed!" << endl;
		return false;
	};

	bool allSet = true;
	Vertex* currentVertex;
	for( unsigned int vertIdx = 0; vertIdx < getVertexNr(); ++vertIdx ) {
		currentVertex = getVertexPos( vertIdx );
		Vector3D point( currentVertex->getX(), currentVertex->getY(), currentVertex->getZ() );
		double angle = currentVertex->getAxisAngleToRadial( axisTop, axisBottom );
		if( !currentVertex->setFuncValue( angle ) ) {
			allSet = false;
		};
	};
	changedVertFuncVal();
	return allSet;
}

//! Compute the angle between vertex normal and projection vector of vertex onto cone axis with vertex normal projected onto plane orthogonal to cone axis.
//! Unsigned angle is set in radian.
//! @returns true, when a new function value was set for all vertices. False otherwise or in case of an error.
bool Mesh::setVertFuncValOrthogonalAxisAngleToRadial() {
	Vector3D axisTop;
	Vector3D axisBottom;
	if( !getConeAxis( &axisTop, &axisBottom ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Fetching cone axis failed!" << endl;
		return false;
	};

	bool allSet = true;
	Vertex* currentVertex;
	for( unsigned int vertIdx = 0; vertIdx < getVertexNr(); ++vertIdx ) {
		currentVertex = getVertexPos( vertIdx );
		Vector3D point( currentVertex->getX(), currentVertex->getY(), currentVertex->getZ() );
		double angle = currentVertex->getOrthogonalAxisAngleToRadial( axisTop, axisBottom );
		if( !currentVertex->setFuncValue( angle ) ) {
			allSet = false;
		};
	};
	changedVertFuncVal();
	return allSet;
}

//! Sets the graylevel of the color per vertex as function value.
//! See Vertex::getGraylevel and Vertex::eGrayLevelConversion
//! @returns true, when a new function value was set for all vertices. False otherwise.
bool Mesh::setVertFuncValGraylevel( Vertex::eGrayLevelConversion rGrayLevelConvMethod ) {
	bool allSet = true;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		double grayLevel;
		if( !currVertex->getGraylevel( &grayLevel, rGrayLevelConvMethod ) ) {
			allSet = false;
		}
		if( !currVertex->setFuncValue( grayLevel ) ) {
			allSet = false;
		}
	}
	changedVertFuncVal();
	return allSet;
}

//! Ask user for a line.
//! \todo Provide cone axis as default, when defined.
//! Compute the distance of the vertices to the given line.
//! Line defintion: position vector and directional vector.
//! @returns false in case of an error or user abort. True otherwise.
bool Mesh::setVertFuncValDistanceToLinePosDir() {
	vector<double> posVecAndDirVec;
	if( !showEnterText( posVecAndDirVec, "Line by position and direction (2x3 values)" ) ) {
		return false;
	}
	if( posVecAndDirVec.size() != 6 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Wrong number of elements (" << posVecAndDirVec.size() << ") given 6 expected!" << endl;
		return false;
	}
	Vector3D posVec( posVecAndDirVec[0], posVecAndDirVec[1], posVecAndDirVec[2], 1.0 );
	Vector3D dirVec( posVecAndDirVec[3], posVecAndDirVec[4], posVecAndDirVec[5], 0.0 );

	bool retVal = setVertFuncValDistanceToLinePosDir( &posVec, &dirVec );
	return retVal;
}

//! Compute the distance of the vertices to the given line.
//! Line defintion: position vector and directional vector.
//! @returns false in case of an error. True otherwise.
bool Mesh::setVertFuncValDistanceToLinePosDir(
    const Vector3D* rPos,  //!< Position vector
    const Vector3D* rDir   //!< Direction vector
) {
	bool retVal = true;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double distToLine = currVertex->estDistanceToLineDir( rPos, rDir );
		retVal &= currVertex->setFuncValue( distToLine );
	}
	changedVertFuncVal();
	return retVal;
}

//! Ask user for a line and provide cone axis as default, when defined.
//! Compute the distance of the vertices to the (cone) axis.
//! Axis defintion: two position vectors.
//! @returns false in case of an error or user abort. True otherwise.
bool Mesh::setVertFuncValDistanceToAxis() {
	Vector3D axisTop;
	Vector3D axisBottom;
	vector<double> posVecAndPosVec;
	// If the axis is defined use it to set the default values.
	if( getConeAxis( &axisTop, &axisBottom ) ) {
		posVecAndPosVec.push_back( axisTop.getX() );
		posVecAndPosVec.push_back( axisTop.getY() );
		posVecAndPosVec.push_back( axisTop.getZ() );
		posVecAndPosVec.push_back( axisBottom.getX() );
		posVecAndPosVec.push_back( axisBottom.getY() );
		posVecAndPosVec.push_back( axisBottom.getZ() );
	}
	if( !showEnterText( posVecAndPosVec, "Axis/Line by two positions (2x3 values)" ) ) {
		return false;
	}
	if( posVecAndPosVec.size() != 6 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Wrong number of elements (" << posVecAndPosVec.size() << ") given 6 expected!" << endl;
		return false;
	}
	// Fetch values, which might by changed by user interaction:
	axisTop.set( posVecAndPosVec[0], posVecAndPosVec[1], posVecAndPosVec[2], 1.0 );
	axisBottom.set( posVecAndPosVec[3], posVecAndPosVec[4], posVecAndPosVec[5], 1.0 );

	// Use the line function, which requires a direction instead of a position.
	Vector3D axisDir = axisTop - axisBottom;
	bool retVal = setVertFuncValDistanceToLinePosDir( &axisTop, &axisDir );
	return retVal;
}

//! Accepts an axis (e.g. from the cone) defined by two position vectors.
//! The axis is used as origin of a cylindrical coordinate system.
//! The function value is set to the angle of the cylindrical coordinate system.
//! See Vertex::angleInLineCoord which is a(n inefficent) variant in this case.
//! @returns false in case of an error. True otherwise.
bool Mesh::setVertFuncValAngleBasedOnAxis( Vector3D* rPosBottom, Vector3D* rPosTop ) {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	// Sanity checks
	if( ( rPosBottom == nullptr ) || ( rPosTop == nullptr ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] NULL pointer given!" << endl;
		return false;
	}
	if( std::isnan( rPosBottom->getLength3() ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Invalid rPosBottom vector!" << endl;
		return false;
	}
	if( std::isnan( rPosTop->getLength3() ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Invalid rPosTop vector!" << endl;
		return false;
	}

	Vector3D orientVec = (*rPosTop) - (*rPosBottom); // direction of axis
	Matrix4D originTrans;
	originTrans.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Y, rPosBottom, &orientVec );

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		Vector3D currPosition = currVertex->getPositionVector();
		currPosition.applyTransformation( originTrans );
		double x = currPosition.getX();
		double z = currPosition.getZ();
		double newValue = atan2( z, x ); //currPosition.getAngleToXinXY();
		currVertex->setFuncValue( newValue );
	}
	changedVertFuncVal();
	return true;
}

bool Mesh::setVertFuncVal( double rVal ) {
	//! Sets all vertex function values to a given value.
	//! Returns true, when all vertices function value could be set.
	bool allSet = true;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( !currVertex->setFuncValue( rVal ) ) {
			allSet = false;
		}
	}
	changedVertFuncVal();
	return allSet;
}

bool Mesh::setVertFuncValCutOff( double minVal, double maxVal, bool setToNotANumber ) {
	//! Cuts of function values (of vertices).
	//! When setToNotANumber is false, the values lower than minVal will be set to minVal.
	//! When setToNotANumber is true, the values lower minVal will be set to not-a-number.
	//! Analog: maxVal.

	Vertex* curVertex;
	double  funcValCurr;
	double  funcValSetTo;
	int     countFuncValSet = 0;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( !curVertex->getFuncValue( &funcValCurr ) ) {
			cout << "[Mesh::" << __FUNCTION__ << "] ERROR: getFuncValue!" << endl;
			continue;
		}
		if( funcValCurr < minVal ) {
			if( setToNotANumber ) {
				funcValSetTo = _NOT_A_NUMBER_DBL_;
			} else {
				funcValSetTo = minVal;
			}
			if( !curVertex->setFuncValue( funcValSetTo ) ) {
				cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: setFuncValue( minVal )!" << endl;
			} else {
				countFuncValSet++;
			}
		}
		if( funcValCurr > maxVal ) {
			if( setToNotANumber ) {
				funcValSetTo = _NOT_A_NUMBER_DBL_;
			} else {
				funcValSetTo = maxVal;
			}
			if( !curVertex->setFuncValue( funcValSetTo ) ) {
				cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: setFuncValue( maxVal )!" << endl;
			} else {
				countFuncValSet++;
			}
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] " << countFuncValSet << " values set." << endl;
	changedVertFuncVal();
	return true;
}

bool Mesh::setVertFuncValNormalize() {
	//! Normalization of the function values to [0.0 ... 1.0].
	//! Typically used for weighting other functions like the geodesic distance.

	// Find minimum and maximum first
	double  funcValueMin = +DBL_MAX;
	double  funcValueMax = -DBL_MAX;
	Vertex* curVertex;
	double  funcValCurr;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( !curVertex->getFuncValue( &funcValCurr ) ) {
			cout << "[Mesh::" << __FUNCTION__ << "] ERROR: getFuncValue!" << endl;
			continue;
		}
		if( funcValueMin > funcValCurr ) {
			funcValueMin = funcValCurr;
		}
		if( funcValueMax < funcValCurr ) {
			funcValueMax = funcValCurr;
		}
	}

	double  funcValueRange = funcValueMax - funcValueMin;
	cout << "[Mesh::" << __FUNCTION__ << "] funcVal min: " << funcValueMin << " max: " << funcValueMax << endl;

	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( !curVertex->getFuncValue( &funcValCurr ) ) {
			cout << "[Mesh::" << __FUNCTION__ << "] ERROR: getFuncValue!" << endl;
			continue;
		}
		funcValCurr = ( funcValCurr - funcValueMin ) / funcValueRange;
		if( !curVertex->setFuncValue( funcValCurr ) ) {
			cout << "[Mesh::" << __FUNCTION__ << "] ERROR: setFuncValue!" << endl;
			continue;
		}
	}
	changedVertFuncVal();
	return true;
}

bool Mesh::setVertFuncValAbs() {
	//! Compute and set the absolute function value.
	//! Returns true, when all vertices function value could be set.
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	bool allSet = true;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double  currVal;
		if( !currVertex->getFuncValue( &currVal ) ) {
			allSet = false;
			continue;
		}
		if( !currVertex->setFuncValue( abs( currVal ) ) ) {
			allSet = false;
		}
	}
	changedVertFuncVal();
	return allSet;
}

bool Mesh::setVertFuncValAdd( double rVal ) {
	//! Add a given constant to the vertices function values.
	//! Returns true, when all vertices function value could be set.
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	bool allSet = true;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double  currVal;
		if( !currVertex->getFuncValue( &currVal ) ) {
			allSet = false;
			continue;
		}
		if( !currVertex->setFuncValue( currVal+rVal ) ) {
			allSet = false;
		}
	}
	changedVertFuncVal();
	return allSet;
}

//! Multiple the vertices function values using a scalar value entered by the user.
//! @returns true, when all vertices function value were set.
bool Mesh::setVertFuncValMult() {
	// Stub
	//return setVertFuncValMult( userEnteredValue );
	return false;
}

//! Multiple a given scalar value to the vertices function values.
//! @returns true, when all vertices function value were set.
bool Mesh::setVertFuncValMult( double rVal ) {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	bool allSet = true;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double  currVal;
		if( !currVertex->getFuncValue( &currVal ) ) {
			allSet = false;
			continue;
		}
		if( !currVertex->setFuncValue( currVal*rVal ) ) {
			allSet = false;
		}
	}
	changedVertFuncVal();
	return allSet;
}

//! Orders the vertices' function values. The index of the order list is used to set the function value.
//! So we can achieve an equalized visualizations.
//!
//! @returns true, when all vertices function value were set.
bool Mesh::setVertFuncValToOrder() {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;

	vector<Vertex*> verticesSorted;
	if( !getVertexList( &verticesSorted ) ) {
		return( false );
	}

	int timeStart = clock();
	sort( verticesSorted.begin(), verticesSorted.end(), Vertex::funcValLower );
	cout << "[Mesh::" << __FUNCTION__ << "] sort time: " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	bool allSet = true;
	for( uint64_t vertIdx=0; vertIdx<verticesSorted.size(); vertIdx++ ) {
		Vertex* currVertex = verticesSorted.at( vertIdx );
		if( !currVertex->setFuncValue( static_cast<double>(vertIdx) ) ) {
			allSet = false;
		}
	}

	changedVertFuncVal();
	return( allSet );
}

//! Calculates distance from each vertex to the cone given by an axis and two radii -- an upper radius
//! and a lower radius
bool Mesh::setVertFuncValDistanceToCone( bool rAbsDist ) {

	Vector3D axisTop;
	Vector3D axisBot;
	double upperRadius;
	double lowerRadius;


	if( !( getConeRadii( &upperRadius, &lowerRadius ) && getConeAxis( &axisTop, &axisBot ) ) ) {
		return false;
	}

	bool coneTipDown = (mConeRadius[0] > mConeRadius[1]);
	if(!coneTipDown) {
		std::swap(axisTop, axisBot);
	}

	// Fetch cone data.
	Vector3D coneTip;
	double coneHeight;
	this->calcConeTip( &coneTip );
	this->calcConeHeigth( &coneHeight );
	double coneAngle = 0.0;
	calcConeAngle( &coneAngle );

	for( uint64_t vertIdx = 0; vertIdx < getVertexNr(); vertIdx++ ) {
		Vertex*	currVertex = getVertexPos(vertIdx);
		double currVal = currVertex->estDistanceToCone( &axisTop, &axisBot, &coneTip, coneAngle, coneHeight, upperRadius, lowerRadius, rAbsDist );
		currVertex->setFuncValue( currVal );
	}
	changedVertFuncVal();
	return true;
}

//! Calculates distance from each vertex to the sphere given by a center and a radius
bool Mesh::setVertFuncValDistanceToSphere() {

	Vector3D center;
	double radius;

	if( !getSphereData( &center, &radius ) )  {
		return false;
	}

	bool rAbsDist = false;
	for( uint64_t vertIdx = 0; vertIdx < getVertexNr(); vertIdx++ ) {
		Vertex*	currVertex = getVertexPos(vertIdx);
		double currVal = currVertex->estDistanceToSphere(&center, radius, rAbsDist );
		currVertex->setFuncValue( currVal );
	}
	changedVertFuncVal();
	return true;
}

//! Visualizes the sum of the angles of the adjacent faces.
//! Only the angle at the vertex is added!
//! See Vertex::get1RingSumAngles
bool Mesh::setVertFuncVal1RSumAngles() {
	showProgressStart( "1-ring angle sum" );
	uint64_t vertexCount = getVertexNr();
	for( uint64_t vertIdx=0; vertIdx<vertexCount; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double angleSum = currVertex->get1RingSumAngles();
		currVertex->setFuncValue( angleSum );
		showProgress( (double)vertIdx/(double)vertexCount, "1-ring angle sum" );
	}
	showProgressStop( "1-ring angle sum" );
	changedVertFuncVal();
	return( true );
}

//! Use the function value to store a sequential index for binary space
//! partitioning (BSP) the 3D-model by an edge length of rEdgeLen.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::setVertFuncValOctreeIdx( double rEdgeLen ) {
	if( rEdgeLen <= 0.0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: zero and negative value not allowed!" << std::endl;
		return( false );
	}

	unsigned int xCubes = ceil( ( mMaxX - mMinX ) / rEdgeLen );
	unsigned int yCubes = ceil( ( mMaxY - mMinY ) / rEdgeLen );
	unsigned int zCubes = ceil( ( mMaxZ - mMinZ ) / rEdgeLen );
	unsigned int xyzCubes = max( xCubes, max( yCubes, zCubes ) );
	if( xyzCubes & 0x1 ) {
		// odd number => round to even
		xyzCubes++;
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] Cubes: " << xyzCubes << std::endl;
	Vector3D cubeTopLeft = getBoundingBoxCenter() - Vector3D( xyzCubes*rEdgeLen, xyzCubes*rEdgeLen, xyzCubes*rEdgeLen, 0.0 );
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		unsigned int octInd = currVertex->getOctreeIndex( cubeTopLeft, rEdgeLen, xyzCubes );
		currVertex->setFuncValue( static_cast<double>(octInd) );
	}

	changedVertFuncVal();
	return( true );
}

bool Mesh::setVertFuncValFaceSphereAngleMax( double rRadius ) {
	//! Compute the maximum face angle to the vertex normal within a spherical neighbourhood and store it as function value

	// Allocate the bit arrays for vertices:
	uint64_t* vertBitArrayVisited;
	int vertNrLongs = getBitArrayVerts( &vertBitArrayVisited );
	// Allocate the bit arrays for faces:
	uint64_t* faceBitArrayVisited;
	int faceNrLongs = getBitArrayFaces( &faceBitArrayVisited );

	vector<Face*> facesInSphere;
	vector<Face*>::iterator itFace;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		Vector3D vertNorm = currVertex->getNormal();
		fetchSphereBitArray( currVertex, &facesInSphere, static_cast<float>(rRadius), vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited, false );
		double maxAngle = -DBL_MAX;
		for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
			Vector3D faceNorm = (*itFace)->getNormal();
			double currAngle = angle( faceNorm, vertNorm );
			if( maxAngle < currAngle ) {
				maxAngle = currAngle;
			}
		}
		currVertex->setFuncValue( maxAngle );
		facesInSphere.clear();
	}

	delete[] vertBitArrayVisited;
	delete[] faceBitArrayVisited;

	changedVertFuncVal();
	return true;
}

bool Mesh::setVertFuncValFaceSphereMeanAngleMax( double rRadius ) {
	//! Compute the maximum face angle to the faces mean normal normal within a spherical neighbourhood and store it as function value per vertex.

	// Allocate the bit arrays for vertices:
	uint64_t* vertBitArrayVisited;
	int vertNrLongs = getBitArrayVerts( &vertBitArrayVisited );
	// Allocate the bit arrays for faces:
	uint64_t* faceBitArrayVisited;
	int faceNrLongs = getBitArrayFaces( &faceBitArrayVisited );

	vector<Face*> facesInSphere;
	vector<Face*>::iterator itFace;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		fetchSphereBitArray( currVertex, &facesInSphere, static_cast<float>(rRadius), vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited, false );
		//! .) Compute weighted mean normal of the faces.
		Vector3D vertFaceMean( 0.0, 0.0, 0.0, 0.0 );
		for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
			vertFaceMean += (*itFace)->getNormal();
		}
		//! .) Compute max angle to mean normal.
		double maxAngle = -DBL_MAX;
		for( itFace=facesInSphere.begin(); itFace!=facesInSphere.end(); itFace++ ) {
			Vector3D faceNorm = (*itFace)->getNormal();
			double currAngle = angle( faceNorm, vertFaceMean );
			if( maxAngle < currAngle ) {
				maxAngle = currAngle;
			}
		}
		currVertex->setFuncValue( maxAngle );
		facesInSphere.clear();
	}

	delete[] vertBitArrayVisited;
	delete[] faceBitArrayVisited;

	changedVertFuncVal();
	return true;
}

//! Compute 1-ring area for all vertices.
//! @returns false in case of an error. True otherwise
bool Mesh::setVertFuncVal1RingArea() {
	showProgressStart( "1-ring area" );
	uint64_t vertexCount = getVertexNr();
	for( uint64_t vertIdx=0; vertIdx<vertexCount; vertIdx++ ) {
		Vertex* curVertex = getVertexPos( vertIdx );
		double area = curVertex->get1RingArea();
		curVertex->setFunctionValue( area );
		showProgress( (double)vertIdx/(double)vertexCount, "1-ring area" );
	}
	showProgressStop( "1-ring area" );
	changedVertFuncVal();
	return( true );
}

//! Apply a median or mean filter operation on a vertex's 1-ring neighbourhood.
//! User interactive.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertMedianOneRingUI(
    bool         rPreferMeanOverMedian    //!< Compute mean value instead of the median.
) {
	bool storeDiffAsFeatureVec;
	if( !showQuestion( &storeDiffAsFeatureVec, "Store changes", "as feature vectors" ) ) {
		// User cancel
		return( false );
	}

	bool retVal = true;
	// Ask for a filter size (radius) in mesh units:
	double filterSize = 0.25;
	if( showEnterText( filterSize, "Filtersize in world units" ) ) {
		if( filterSize <= 0.0 ) {
			// Ask for a number of iterations, when no radius was given:
			uint64_t iterations=1;
			if( showEnterText( iterations, "Number of iterations" ) ) {
				retVal = funcVertMedianOneRing( iterations, 0.0, rPreferMeanOverMedian, storeDiffAsFeatureVec );
			} else {
				retVal = false; // i.e. user cancel or invalid values.
			}
		} else {
			retVal = funcVertMedianOneRing( 0, filterSize, rPreferMeanOverMedian, storeDiffAsFeatureVec );
		}
	}

	return( retVal );
}

//! Apply a median or mean filter operation on a vertex's 1-ring neighbourhood.
//!
//! The filter size can be defined by either
//!     providing a number of iterations by providing rIterations > 0
//! or
//!     a radius in mesh units by providing a rFilterSize > 0.0.
//! In case both valies are set (valid), the number of iterations is used.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertMedianOneRing(
    unsigned int rIterations,              //!< Number of interations
    double       rFilterSize,              //!< Filter size in units of the mesh (e.g. mm)
    bool         rPreferMeanOverMedian,    //!< Compute mean value instead of the median.
    bool         rStoreDiffAsFeatureVec    //!< Option to store the changes as feature vectors.
) {
	// 0a. Pre-Compute the minimum face altitude length
	// double minDist = getAltitudeMin();
	// cout << "[Mesh::" << __FUNCTION__ << "] Minimal altitude: " << minDist << " mm (unit assumed)." << endl;

	// 0a. Pre-Compute the minimum edge length
	double minDist = getEdgeLenMin();
	cout << "[Mesh::" << __FUNCTION__ << "] Minimal edge length: " << minDist << " mm (unit assumed)." << endl;

	// 0b. Sanity checks
	if( rIterations == 0 ) {
		if( rFilterSize <= 0.0 ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Zero iterations or invalid filter size requested!" << endl;
			return( false );
		}
		// Compute number of iterations using the given filter size:
		rIterations = round( 2.0*rFilterSize/minDist );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] " << rIterations << " iterations corresponds to a filter size (radius) of " << rIterations*minDist << " mm (unit assumed)." << endl;

	string funcName;
	if( rPreferMeanOverMedian ) {
		funcName = "Vertex function value 1-ring mean, " + to_string( rIterations ) + " iterations";
	} else {
		funcName = "Vertex function value 1-ring median, " + to_string( rIterations ) + " iterations";
	}
	time_t timeStart = clock();
	bool retVal = true;
	Vertex* currVertex;

	// Fetch number of vertices.
	uint64_t nrOfVertices = getVertexNr();

	// Option A: use the difference as feature vector for flow visualization
	// Prepare two-dimensional array as vector
	vector<double> diffFlowFTVec;
	if( rStoreDiffAsFeatureVec ) {
		cout << "[Mesh::" << __FUNCTION__ << "] Allocating memory. which requires a moment." << endl;
		diffFlowFTVec.assign( nrOfVertices*rIterations, _NOT_A_NUMBER_DBL_ );
	}

	// Time has to be counted after the memory assignment. Otherwise the estimated time is way off for larger numbers of iterations.
	showProgressStart( funcName );

	// Apply multiple times
	for( unsigned int i=0; i<rIterations; i++ ) {
		vector<double> newFuncVals;
		unsigned int   vertsIgnored = 0;
		double         akkuTrackDiff = 0.0;
		unsigned int   akkuTrackChangesCount = 0;
		// 1. Compute indipendently and write back later!
		for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
			currVertex = getVertexPos( vertIdx );
			double funcVal;
			double funcValPrev;
			currVertex->getFuncValue( &funcVal );
			funcValPrev = funcVal;
			bool retValCurr;
			if( rPreferMeanOverMedian ) {
				retValCurr = currVertex->funcValMeanOneRing( &funcVal, minDist );
			} else {
				retValCurr = currVertex->funcValMedianOneRing( &funcVal, minDist );
			}
			if( !retValCurr ) {
				vertsIgnored++;
			}
			newFuncVals.push_back( funcVal );
			if( isfinite( funcVal ) && isfinite( funcValPrev ) ) {
				akkuTrackDiff += abs( funcVal - funcValPrev );
				akkuTrackChangesCount++;
			}
			showProgress( (static_cast<double>(i)/static_cast<double>(rIterations)) + (0.75*((vertIdx+1)/static_cast<double>(nrOfVertices)))/static_cast<double>(rIterations), funcName );
		}

		// 2. Write back the new values:
		for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
			currVertex = getVertexPos( vertIdx );
			double newValue = newFuncVals.at( vertIdx );

			// Option A: use the difference as feature vector for flow visualization
			// Compute and store difference
			if( rStoreDiffAsFeatureVec ) {
				double oldValue = _NOT_A_NUMBER_DBL_;
				currVertex->getFuncValue( &oldValue );
				// Rember: Feature vector elements are expected consecutive
				diffFlowFTVec.at( vertIdx*rIterations + i ) = oldValue - newValue;
			}

			currVertex->setFuncValue( newValue );
			showProgress( (static_cast<double>(i)/static_cast<double>(rIterations)) + (0.25+0.75*((vertIdx+1)/static_cast<double>(nrOfVertices)))/static_cast<double>(rIterations), funcName );
		}

		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Vertices processed: " << getVertexNr()-vertsIgnored << endl;
		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Vertices ignored:   " << vertsIgnored << endl;
		cout << "[Mesh::" << __FUNCTION__ << "] Iteration " << (i+1) << " Relative changes to the function vales:   " << akkuTrackDiff / static_cast<double>(akkuTrackChangesCount) << endl;
	}

	// Option A: use the difference as feature vector for flow visualization
	// Store as new feature vector
	if( rStoreDiffAsFeatureVec ) {
		removeFeatureVectors();
		mFeatureVecVerticesLen = rIterations;
		mFeatureVecVertices.swap( diffFlowFTVec );
		if( !assignFeatureVectors( mFeatureVecVertices, mFeatureVecVerticesLen ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: during assignment of the feature vectors!" << endl;
			retVal = false;
		}
	}

	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFuncVal();
	return( retVal );
}

//! Compute the number of adjacent faces and store it as function value.
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertAdjacentFaces() {
	bool retVal = true;
	string funcName = "Number of adjacent faces.";

	showProgressStart( funcName );
	uint64_t nrOfVertices = getVertexNr();

	for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		uint64_t adjacentFaceCount = currVertex->get1RingFaceCount();
		if( !currVertex->setFuncValue( static_cast<double>(adjacentFaceCount) ) ) {
			retVal = false;
		}
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}

	showProgressStop( funcName );
	changedVertFuncVal();
	return( retVal );
}

//! Compute r_min_i for 1-ring neighborhoods.
//! See Vertex::get1RingEdgeLenMin.
bool Mesh::funcVert1RingRMin() {
	string funcName = "r_min for 1-ring";
	time_t timeStart = clock();
	Vertex* currVertex;
	int nrOfVertices = getVertexNr();
	showProgressStart( funcName );
	for( int vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		double radiusMin = currVertex->get1RingEdgeLenMin();
		currVertex->setFuncValue( radiusMin );
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFuncVal();
	return true;
}

//! Compute volume integral for a 1-ring, which is equivalent to V(r->0)
bool Mesh::funcVert1RingVolInt() {
	// Find smallest radius within the mesh:
	double rMin = getAltitudeMin();
	cout << "[Mesh::" << __FUNCTION__ << "] Rmin: " << rMin << endl;

	// Data structures to be (re-)used for integration:
	int xyzDim = 256;
	vector<Face*> faces1Ring;

	// Its is safer to use a vector instead of a plain array.
	vector<double> rasterArrayValues;
	rasterArrayValues.resize( xyzDim*xyzDim, _NOT_A_NUMBER_DBL_ );
	double* rasterArray = rasterArrayValues.data();
	//double* rasterArray = new double[xyzDim*xyzDim];

	voxelFilter2DElements* sparseFilters = nullptr;
	// double** voxelFilters2D = generateVoxelFilters2D( 1, &rMin, xyzDim, &sparseFilters ); // <- the returned values are unused, but sparseFilters will be computed!
	generateVoxelFilters2D( 1, &rMin, xyzDim, &sparseFilters );
	string funcName = "Volume Int. 1-ring";
	time_t timeStart = clock();
	showProgressStart( funcName );
	uint64_t nrOfVertices = getVertexNr();
	for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		currVertex->getFaces( &faces1Ring );
		fetchSphereCubeVolume25D( currVertex, &faces1Ring, rMin, rasterArray, xyzDim );
		faces1Ring.clear();
		double descriptVolume;
		applyVoxelFilters2D( &descriptVolume, rasterArray, &sparseFilters, 1, xyzDim );
		//cout << "[Mesh::" << __FUNCTION__ << "] descriptVolume: " << descriptVolume << endl;
		currVertex->setFuncValue( descriptVolume );
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFuncVal();
	return( true );
}

//! Compute maximum distances between vertices.
//! This implementation has a O(n^2), which is very bad!
//! \todo Faster implementation, e.g, using BSP (Binary Space Partitioning).
bool Mesh::funcVertDistancesMax() {
	std::string funcName = "Max. distances";
	time_t timeStart = clock();
	Vertex* currVertex;
	Vertex* otherVertex;
	uint64_t nrOfVertices = getVertexNr();
	showProgressStart( funcName );
	for( uint64_t vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		double dist = 0.0;
		for( uint64_t vertOtherIdx=0; vertOtherIdx<nrOfVertices; vertOtherIdx++ ) {
			otherVertex = getVertexPos( vertOtherIdx );
			if( otherVertex == currVertex ) {
				continue;
			}
			double currDist = distanceVV( currVertex, otherVertex );
			if( currDist > dist ) {
				dist = currDist;
			}
		}
		currVertex->setFuncValue( dist );
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << std::endl;
	showProgressStop( funcName );
	changedVertFuncVal();
	return true;
}

//! Set the function value to the standard deviation of the feature vector's element.
//!
//! @returns true, when all vertices got a new function value. False otherwise i.e. in case of an error.
bool Mesh::funcVertFeatureElementsStdDev() {
	bool allSet = true;

	string funcName = "Compute standard deviation of the feature vectors' elements.";
	uint64_t nrOfVertices = getVertexNr();
	showProgressStart( funcName );

	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double featureVecMean; 
		double featureVecStdDev;
		if( !currVertex->getFeatureVecMeanStdDev( &featureVecMean, &featureVecStdDev ) ) {
			allSet = false;
		}
		if( !currVertex->setFuncValue( featureVecStdDev ) ) {
			allSet = false;
		}
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	showProgressStop( funcName );

	changedVertFuncVal();
	return( allSet );
}

//! Set the function value to the minimum of the feature vector's element.
//!
//! @returns true, when all vertices got a new function value. False otherwise i.e. in case of an error.
bool Mesh::funcVertFeatureVecMin() {
	bool allSet = true;

	string funcName = "Determine minimal value of the feature vectors' elements.";
	uint64_t nrOfVertices = getVertexNr();
	showProgressStart( funcName );

	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double minFeatureVecElement;
		if( !currVertex->getFeatureVecMin( &minFeatureVecElement ) ) {
			allSet = false;
		}
		if( !currVertex->setFuncValue( minFeatureVecElement ) ) {
			allSet = false;
		}
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	showProgressStop( funcName );

	changedVertFuncVal();
	return( allSet );
}

//! Set the function value to the maximum of the feature vector's element.
//! This is not the same as the maximum norm!
//!
//! @returns true, when all vertices got a new function value. False otherwise i.e. in case of an error.
bool Mesh::funcVertFeatureVecMax() {
	bool allSet = true;

	string funcName = "Determine maximum value of the feature vectors' elements.";
	uint64_t nrOfVertices = getVertexNr();
	showProgressStart( funcName );

	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double maxFeatureVecElement;
		if( !currVertex->getFeatureVecMax( &maxFeatureVecElement ) ) {
			allSet = false;
		}
		if( !currVertex->setFuncValue( maxFeatureVecElement ) ) {
			allSet = false;
		}
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	showProgressStop( funcName );

	changedVertFuncVal();
	return( allSet );
}

//! Set the function value to the minimal positve or negative value of the feature vectors' elements.
//!
//! @returns true, when all vertices got a new function value. False otherwise i.e. in case of an error.
bool Mesh::funcVertFeatureVecMinSigned() {
	bool allSet = true;

	string funcName = "Determine minimal positve or negative value of the feature vectors' elements.";
	uint64_t nrOfVertices = getVertexNr();
	showProgressStart( funcName );

	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double minFeatureVecElement;
		if( !currVertex->getFeatureVecMinSigned( &minFeatureVecElement ) ) {
			allSet = false;
		}
		if( !currVertex->setFuncValue( minFeatureVecElement ) ) {
			allSet = false;
		}
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	showProgressStop( funcName );

	changedVertFuncVal();
	return( allSet );
}

//! Set the function value to the maximum positve or negative value of the feature vectors' elements.
//! This is not the same as the maximum norm!
//!
//! @returns true, when all vertices got a new function value. False otherwise i.e. in case of an error.
bool Mesh::funcVertFeatureVecMaxSigned() {
	bool allSet = true;

	string funcName = "Determine maximal positve or negative value of the feature vectors' elements.";
	uint64_t nrOfVertices = getVertexNr();
	showProgressStart( funcName );

	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double maxFeatureVecElement;
		if( !currVertex->getFeatureVecMaxSigned( &maxFeatureVecElement ) ) {
			allSet = false;
		}
		if( !currVertex->setFuncValue( maxFeatureVecElement ) ) {
			allSet = false;
		}
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	showProgressStop( funcName );

	changedVertFuncVal();
	return( allSet );
}

//! Compute a Mahalanobis distance using the feature vector of the vertices.
//! Note: that this method is only inspired by the Mahalanobis distance
//!       in the current implementation (06/2017).
//! @returns false in case of an error e.g. Alglib not present. True otherwise.
bool Mesh::funcVertFeatureVecMahalDist() {
#ifndef ALGLIB
	cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: ALGLIB is missing and required for this method!" << endl;
	return false;
#else
	// ALGLIB minimal workin example for the matrix multiplication
	// Requires "ablas.h"
/*
	int m = 2; // a - Zeilen
	int n = 4; // b - Spalten
	int k = 3; // a - Spalten == b - Zeilen
	double a[m*k] = { // 2 x 3
		1,2,3,
		4,5,6
	};
	double b[k*n] = { // 3 x 4
		7,8,9,10,
		11,12,13,14,
		15,16,17,18
	};
	ap::real_2d_array aa;
	ap::real_2d_array bb;
	aa.setcontent(0,3,0,2,a);
	bb.setcontent(0,4,0,3,b);
	for( int i = 0; i < 2; i++ ) { // y
		for( int j = 0; j < 3; j++ ) { // x
			cout << aa(i,j) << " ";
		}
		cout << endl;
	}
	cout << "..........." << endl;
	for( int i = 0; i < 3; i++ ) { // y
		for( int j = 0; j < 4; j++ ) { // x
			cout << bb(i,j) << " ";
		}
		cout << endl;
	}
	cout << "----------" << endl;
	ap::real_2d_array cc;
	cc.setlength( m, n );
	rmatrixgemm( m, n, k, 1, aa,0,0,0, bb,0,0,0, 0, cc, 0,0 );
	for( int i = 0; i < 2; i++ ) { // y
		for( int j = 0; j < 4; j++ ) { // x
			cout << cc(i,j) << " ";
		}
		cout << endl;
	}
*/
	string funcName = "Compute Mahalanobis inspired distance for feature vectors";
	time_t timeStart = clock();

	// STUB - Reference Vector
	// 0.611369 0.657464 0.691763 0.7143540.726667 0.727034 0.722448 0.717575 0.713068 0.708751 0.701904 0.68729 0.648809 0.583745 0.475463 0.222697
	//double refVec[] = { 0.611369, 0.657464, 0.691763, 0.714354,
	//                    0.726667, 0.727034, 0.722448, 0.717575,
	//                    0.713068, 0.708751, 0.701904, 0.68729,
	//                    0.648809, 0.583745, 0.475463, 0.222697 };

	// Fetch feature vector from selection, when present.
	vector<double> referenceVector;
	if( mPrimSelected != nullptr ) {
		mPrimSelected->getFeatureVectorElements( referenceVector );
	}
	// Allow user to chance the vector.
	if( !showEnterText( referenceVector, "Reference vector" ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] User cancel or bad values!" << endl;
		return false;
	}
	if( referenceVector.size() == 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] No reference vector given!" << endl;
		return false;
	}

	// Let the user choose between quadric and cubic i.e. for surface or volume integral invariants.
	bool useCubic = true;
	bool useLinear = !showQuestion( &useCubic, string( "Cubic, quadric or linear weights" ), \
	                                string( "YES for cubic weights i.e. volume based integral invariant feature vectors.<br /><br />NO for quadric weights i.e. surface based integral invariant feature vectors.<br /><br />CANCEL for linear weights." ) );

	// Prepare Matrix with weights
	unsigned int scaleCount = referenceVector.size(); // 16;
	ap::real_2d_array weightMat;
	weightMat.setlength( scaleCount, scaleCount );
	// Init with zeros
	for( unsigned int i=0; i<scaleCount; i++ ) {         // y
		for( unsigned int j=0; j<scaleCount; j++ ) { // x
			weightMat(i,j) = 0.0;
		}
	}
	// Fill half of the diagonal
	for( unsigned int ny=1; ny<=scaleCount; ny++ ) {  // y
		double rowSum = 0.0;                                                                                                // For validation / Debug
		for( unsigned int nx=1; nx<=ny; nx++ ) {  // x
			double volumeWeight = 1.0 / ny;
			if( !useLinear ) {
				if( useCubic ) {
					volumeWeight = ( 3.0*pow( nx, 2.0 ) - 3.0*nx + 1.0 ) / ( pow( ny, 3.0 ) );
				} else {
					volumeWeight = ( 2.0*nx - 1.0 ) / ( pow( ny, 2.0 ) );
				}
			}
			rowSum += volumeWeight;                                                                                     // For validation / Debug
			//weightMat( nx-1, ny-1 ) = volumeWeight;
			weightMat( ny-1, nx-1 ) = volumeWeight; // transposed as the vector is also transposed
		}
		cout << "[Mesh::" << __FUNCTION__ << "] Checksum for Row " << ny << " Sum: " << rowSum << endl;                                  // For validation / Debug
	}

	// for( int i=0; i<scaleCount; i++ ) {         // y                                                                            // For validation / Debug
	//	cout << "[Mesh::" << __FUNCTION__ << "] Weight Matrix ";
	//	for( int j=0; j<scaleCount; j++ ) { // x
	//		cout << weightMat(i,j) << " ";
	//	}
	//	cout << endl;
	//}

	ap::real_2d_array featureVec;        // scaleCount x 1
	ap::real_2d_array featureVecMultTmp; // scaleCount x 1
	ap::real_2d_array featureVecTransp;  // 1 x scaleCount
	ap::real_2d_array featureVecResult;  // 1 x 1
//	featureVec.setlength( scaleCount, 1 );
	featureVecMultTmp.setlength( scaleCount, 1 );
	featureVecTransp.setlength( scaleCount, 1 );
	featureVecResult.setlength( 1, 1 );

	int nrOfVertices = getVertexNr();
	double* currFeatVec = new double[scaleCount];
	unsigned int noMatchingDimension = 0;
	showProgressStart( funcName );
	for( int vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		if( currVertex->getFeatureVectorLen() != scaleCount ) {
			currVertex->setFuncValue( _NOT_A_NUMBER_DBL_ );
			noMatchingDimension++;
			continue;
		}
		currVertex->copyFeatureVecTo( currFeatVec );
		for( unsigned int i=0; i<scaleCount; i++ ) {
			//currFeatVec[i] -= refVec[i];
			currFeatVec[i] -= referenceVector.at( i );
			//currFeatVec[i] = pow( currFeatVec[i] - refVec[i], 2.0 );
			//currFeatVec[i] = abs( currFeatVec[i] - refVec[i] );
		}
//		featureVec.setcontent( 0, scaleCount, 0, 1, currFeatVec );
		featureVec.setcontent( 0, scaleCount - 1, 0, 0, currFeatVec );
		rmatrixtranspose( 1, scaleCount, featureVec, 0, 0, featureVecTransp, 0, 0 );
		// cout << "Feature Vector Ori:    ";
		// for( int i=0; i<scaleCount; i++ ) {         // x | Spalte
		//	cout << currFeatVec[i] << " ";
		// }
		// cout << "Feature Vector:        ";
		// for( int i=0; i<scaleCount; i++ ) {         // x | Spalte
		//	cout << featureVec(0,i) << " ";
		// }
		// cout << endl;
		// cout << "Feature Vector Transp: ";
		// for( int i=0; i<scaleCount; i++ ) {         // y | Zeile
		//	cout << featureVecTransp(i,0) << " ";
		// }
		//cout << endl;
		rmatrixgemm( 1, scaleCount, scaleCount, 1, featureVec,0,0,0, weightMat,0,0,0, 0, featureVecMultTmp, 0,0 );
		// cout << "Intermediate result: ";
		// for( int i=0; i<scaleCount; i++ ) {         // y
		//	cout << featureVecMultTmp(0,i) << " ";
		//}
		// cout << endl;
		rmatrixgemm( 1, 1, scaleCount, 1, featureVecMultTmp,0,0,0, featureVecTransp,0,0,0, 0, featureVecResult, 0,0 );
		//cout << "Res: " << featureVecResult(0,0) << endl;
		currVertex->setFuncValue( sqrt( featureVecResult(0,0) ) ); // SQRT as suggest by the original Mahalanobis distance.
		//currVertex->setFuncValue( featureVecResult(0,0) );
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	delete[] currFeatVec;

	if( noMatchingDimension > 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Vertices with non-matching dimension of feature vector: " << noMatchingDimension << "!"  << endl;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFuncVal();

	return true;
#endif
}

//! Compute p-Norm using the feature vector of the vertices.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertFeatureVecPNorm() {
	// Fetch feature vector from selection, when present.
	vector<double> referenceVector;
	if( mPrimSelected != nullptr ) {
		mPrimSelected->getFeatureVectorElements( referenceVector );
	}
	// Allow user to chance the vector.
	if( !showEnterText( referenceVector, "Reference vector" ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] User cancel or invalid values!" << endl;
		return false;
	}
	if( referenceVector.size() == 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] No reference vector given!" << endl;
		return false;
	}

	// Fetch p-Norm by asking the user. Default is maximum-norm.
	double pNorm = std::numeric_limits<double>::infinity(); // Default: maximum-norm
	if( !showEnterText( pNorm, "p-Norm" ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] User cancel or invalid value!" << endl;
		return false;
	}
	if( pNorm < 0.0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Negative p=" << pNorm << " is not defined!" << endl;
		return false;
	}
	cout << "[Mesh::" << __FUNCTION__ << "] p-Norm with p=" << pNorm << endl;

	// Let the user choose between quadric and cubic i.e. for surface or volume integral invariants.
	bool useCubic = true;
	bool useLinear = !showQuestion( &useCubic, string( "Cubic, quadric or linear weights" ), \
	                                string( "YES for cubic weights i.e. volume based integral invariant feature vectors.<br /><br />NO for quadric weights i.e. surface based integral invariant feature vectors.<br /><br />CANCEL for linear weights." ) );

	eFuncFeatureVecPNormWeigth weigthChoosen = FEATURE_VECTOR_PNORM_WEIGTH_LINEAR;
	if( !useLinear ) {
		if( useCubic ) {
			weigthChoosen = FEATURE_VECTOR_PNORM_WEIGTH_CUBIC;
		} else {
			weigthChoosen = FEATURE_VECTOR_PNORM_WEIGTH_QUADRATIC;
		}
	}

	return( funcVertFeatureVecPNorm( referenceVector, pNorm, weigthChoosen ) );
}

//! Compute p-Norm using the feature vector of the vertices.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertFeatureVecPNorm(
                const vector<double>&        rReferenceVector,
                const double&                rpNorm,
                eFuncFeatureVecPNormWeigth   rWeigthType
) {
	// Sanity check
	if( rReferenceVector.size() == 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] No reference vector given!" << endl;
		return( false );
	}

	// Compute weights.
	string funcName = "Compute p-Norm for feature vectors";
	time_t timeStart = clock();

	vector<double> volumeIntInvWeight;
	double numberOfScales = static_cast<double>(rReferenceVector.size()); //16.0;
	double checkSum = 0.0;
	//for( double radiusIndex=1.0; radiusIndex<=numberOfScales; radiusIndex+=1.0 ) {
	for( double radiusIndex=numberOfScales; radiusIndex>=1.0; radiusIndex-=1.0 ) {
		double volumeWeight = 1.0 / numberOfScales; // <- linear
		switch( rWeigthType ) {
			case FEATURE_VECTOR_PNORM_WEIGTH_LINEAR:
				// do nothing
				break;
			case FEATURE_VECTOR_PNORM_WEIGTH_QUADRATIC:
				volumeWeight = ( 2.0*radiusIndex - 1.0 ) / ( pow( numberOfScales, 2.0 ) );
				break;
			case FEATURE_VECTOR_PNORM_WEIGTH_CUBIC:
				volumeWeight = ( 3.0*pow( radiusIndex, 2.0 ) - 3.0*radiusIndex + 1.0 ) / ( pow( numberOfScales, 3.0 ) );
				break;
			default:
				cerr << "[Mesh::" << __FUNCTION__ << "] Unknown weigth!" << endl;
				return( false );
		}
		cout << "[Mesh::" << __FUNCTION__ << "] volumeWeight: " << volumeWeight << endl;
		volumeIntInvWeight.push_back( volumeWeight );
		checkSum += volumeWeight;
	}
	cout << "[Mesh::" << __FUNCTION__ << "] checkSum: " << checkSum << endl;

	int nrOfVertices = getVertexNr();
	showProgressStart( funcName );
	for( int vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double newFuncValue;
		if( currVertex->getFeatureVecPNorm( &newFuncValue, rpNorm, &rReferenceVector, nullptr, &volumeIntInvWeight ) ) {
			currVertex->setFuncValue( newFuncValue );
		} else {
			currVertex->setFuncValue( _NOT_A_NUMBER_DBL_ );
		}
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFuncVal();

	return false;
}

//! Set the function value to an element of the feature vector.
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertFeatureVecElementByIndex( unsigned int rElementNr ) {
	// Sanity checks
	/*
	//TODO: check which should actually be done here. rElementNr cannot be nan, because its an unsigned int
	if( isnan( rElementNr ) ) {
		return false;
	}
	*/
	string funcName = "Fetch an element of the feature vector";
	time_t timeStart = clock();
	bool retVal = true;

	unsigned int nrOfVertices = getVertexNr();
	showProgressStart( funcName );
	for( unsigned int vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double newFuncValue;
		retVal |= currVertex->getFeatureElement( rElementNr, &newFuncValue );
		retVal |= currVertex->setFuncValue( newFuncValue );
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Element Nr: " << rElementNr << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	showProgressStop( funcName );
	changedVertFuncVal();
	return retVal;
}

//! Computes the distance of each Vertex to a plane given in Hesse normal form (HNF) as Vector3D.
//! @returns false in case of an error. True otherwise.
bool Mesh::funcVertDistanceToPlane(
    Vector3D rPlaneHNF,  //!< Hesse normal form
    bool     rAbsDist,   //!< Compute absolut distance
    bool     rSilent     //!< Supress progress and function value update e.g. for temporary use.
) {
	// Sanity checks
	if( !isnormal( rPlaneHNF.getLength3() ) ) {
		return( false );
	}

	string funcName = "Compute distance to plane by HNF";
	time_t timeStart = clock();
	bool retVal = true;

	unsigned int nrOfVertices = getVertexNr();
	if( !rSilent ) {
		showProgressStart( funcName );
	}
	for( unsigned int vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double newFuncValue = currVertex->estDistanceToPlane( &rPlaneHNF, rAbsDist );
		retVal |= currVertex->setFuncValue( newFuncValue );
		if( !rSilent ) {
			showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
		}
	}
	if( !rSilent ) {
		cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		showProgressStop( funcName );
		changedVertFuncVal();
	}
	return( retVal );
}

//! Local brightness values are calculated by casting penetrating light rays parallel to the z-axis onto the transformed mesh.
//! Each ray ends at a z-value given by the array of depths and extends into infinity in negative z-direction.
//! The function values are then incremented by the local brightness values.
//! @param rTransformMat transformation matrix that is applied before casting the light rays
//! @param rArrayWidth horizontal resolution of the depth buffer represented by rDepths
//! @param rArrayHeight vertical resolution of the depth buffer represented by rDepths
//! @param rZTolerance tolerance that is used during comparisons with the values of rDepths (rays are extended by rZTolerance)
//! @warning The function does not notify the mesh about these changes (i.e. Mesh::changedVertFuncVal() is not called)!
//! @returns False in case of an error. True otherwise.
bool Mesh::funcVertAddLight( Matrix4D &rTransformMat, unsigned int rArrayWidth, unsigned int rArrayHeight, const vector<float>& rDepths, float rZTolerance ) {
	unsigned int nrOfVertices = getVertexNr();

	for( unsigned int vertIdx = 0; vertIdx < nrOfVertices; vertIdx++ ) {
		Vertex* vertex = getVertexPos( vertIdx );

		double value;
		if ( !vertex->getFuncValue( &value ) ) {
			return false;
		}

		Vector3D position;
		if ( !vertex->getPositionVector( &position ) ) {
			return false;
		}

		position.applyTransformation( rTransformMat );
		position.normalizeW();

		if ( position.getX() < 1.0 && position.getY() < 1.0 && position.getX() > -1.0 && position.getY() > -1.0 ) {
			int arrayPosX = static_cast<int>((position.getX() + 1.0) * rArrayWidth / 2.0);
			int arrayPosY = static_cast<int>((position.getY() + 1.0) * rArrayHeight / 2.0);

			if (0.5 + 0.5 * position.getZ() <= rDepths.at(arrayPosX + arrayPosY * rArrayWidth) + rZTolerance ) {
				Vector3D transformedNormal = rTransformMat * vertex->getNormal(false);

				double newValue = value + max(0.0, -transformedNormal.getZ() / transformedNormal.getLength3());

				if ( !vertex->setFuncValue( newValue ) ) {
					return false;
				}
			}
		}
	}
	return true;
}

#ifdef LIBSPHERICAL_INTERSECTION
namespace {
//! Converts a Mesh to a spherical_intersecton::Mesh
//! @param original the given Mesh
//! @returns The spherical_intersection::Mesh
spherical_intersection::Mesh convertMesh( Mesh &original ) {
	spherical_intersection::Mesh converted;

	// add all vertices
	auto vertexCount = original.getVertexNr();
	for( uint64_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++ ) {
		const auto *vertex = original.getVertexPos( vertexIndex );
		spherical_intersection::math3d::Vector location{vertex->getX(), vertex->getY(), vertex-> getZ()};
		converted.add_vertex(location);
	}

	// add all faces
	const auto &convertedVertices = converted.get_vertices();
	auto faceCount = original.getFaceNr();
	for ( uint64_t faceIndex = 0; faceIndex < faceCount; faceIndex++ ) {
		auto *face = original.getFacePos( faceIndex );
		const auto &convertedVertex0 = convertedVertices[face->getVertAIndex()];
		const auto &convertedVertex1 = convertedVertices[face->getVertBIndex()];
		const auto &convertedVertex2 = convertedVertices[face->getVertCIndex()];
		converted.add_triangle(convertedVertex0, convertedVertex1, convertedVertex2);
	}

	return converted;
}
#ifdef THREADS
//! Calculates the results obtained from applying a given algorithm to all vertices of a given spherical_intersection::Mesh
//! @param mesh the given spherical_intersection::mesh
//! @param algorithm the given algorithm
//! @param threadCount number of worker threads used
//! @param maximumBatchSize maximum number of vertices processed before updating the progress
//! @param notifyAboutProgress a function that is occasionally called with the faction of processed vertices as its argument
//! @returns The calculation results where the i-th result is the result corresponding to the i-th vertex
vector<double> calculateSphericalIntersectionFuncValues(
	const spherical_intersection::Mesh &mesh,
	function<double(const spherical_intersection::Mesh::Vertex &)> algorithm,
	const size_t threadCount,
	const size_t maximumBatchSize,
	function<void(double)> notifyAboutProgress
) {
	// prepare multithreaded calculation
	auto vertexCount = mesh.get_vertices().size();
	size_t startIndex = 0;
	vector<double> results(vertexCount);
	auto setResults = [&mesh, &algorithm, &results](size_t startIndex, size_t count) {
		for( size_t vertexIndex = startIndex; vertexIndex < startIndex+count; vertexIndex++ ) {
			const auto &vertex = mesh.get_vertices()[vertexIndex];
			results[vertexIndex] = algorithm(vertex);
		}
	};

	// calculate results
	while (startIndex < vertexCount) {
		auto batchSize = min(vertexCount-startIndex, maximumBatchSize);
		auto threadLoad = batchSize / threadCount;
		auto lastThreadLoad = batchSize - (threadCount -1 ) * threadLoad;
		vector<thread> threads;
		threads.reserve(threadCount);
		for( size_t threadIndex = 0; threadIndex < threadCount - 1; threadIndex++ ) {
			threads.emplace_back(setResults, startIndex, threadLoad);
			startIndex += threadLoad;
		}
		threads.emplace_back(setResults, startIndex, lastThreadLoad);
		startIndex += lastThreadLoad;
		for (auto &thread : threads) {
			thread.join();
		}
		notifyAboutProgress( static_cast<double>(startIndex+1)/vertexCount );
	}

	return results;
}
#else
//! Calculates the results obtained from applying a given algorithm to all vertices of a given spherical_intersection::Mesh
//! @param mesh the given spherical_intersection::mesh
//! @param algorithm the given algorithm
//! @param notifyAboutProgress a function that is occasionally called with the faction of processed vertices as its argument
//! @returns The calculation results where the i-th result is the result corresponding to the i-th vertex
vector<double> calculateSphericalIntersectionFuncValues(
	const spherical_intersection::Mesh &mesh,
	function<double(const spherical_intersection::Mesh::Vertex &)> algorithm,
	function<void(double)> notifyAboutProgress
) {
	auto vertexCount = mesh.get_vertices().size();
	vector<double> results(vertexCount);

	// calculate results
	for( size_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++ ) {
		const auto &vertex = mesh.get_vertices()[vertexIndex];
		results[vertexIndex] = algorithm(vertex);
		notifyAboutProgress( static_cast<double>(vertexIndex+1)/vertexCount );
	}

	return results;
}
#endif

//! Assigns the i-th given values to the i-th vertex of the given mesh for every suitable i
//! @param mesh the given mesh
//! @param values the given values
//! @returns False in case of an error. True otherwise.
bool applyFuncValues( Mesh &mesh, vector<double> values ) {
	for( size_t vertexIndex = 0; vertexIndex < min(static_cast<size_t>(mesh.getVertexNr()), values.size()); vertexIndex++ ) {
		if ( !mesh.getVertexPos( vertexIndex )->setFuncValue( values[vertexIndex] ) ) {
			return false;
		}
	}
	return true;
}
}
#endif

//! Sets the function value of every vertex to the normalized arc length of the intersection of a local part of the mesh surface with a sphere with the vertex as its center
//! Asks for the sphere's radius
//! The normalization factor is 1/r where r is the sphere's radius
//! @returns False in case of an error. True otherwise.
bool Mesh::funcVertSphereSurfaceLength() {
#ifdef LIBSPHERICAL_INTERSECTION
#ifdef THREADS
	vector<double> parameters(3,0);
	parameters[0] = 0.1;
	parameters[1] = 7;
	parameters[2] = 1000;

	if( !showEnterText( parameters, "Radius, thread count and maximum batch size (3 values)" ) ) {
		return false;
	}
	if( parameters.size() != 3 ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] Wrong number of elements (" << parameters.size() << ") given. 3 expected!" << endl;
		return false;
	}
	if ( parameters[0] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Radius has to be > 0 (given radius: " << parameters[0] << ")" << endl;
		return false;
	}
	if ( parameters[1] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Thread count has to be > 0 (given thread count: " << parameters[0] << ")" << endl;
		return false;
	}
	if ( parameters[2] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Maximum batch size has to be > 0 (given maximum batch size: " << parameters[0] << ")" << endl;
		return false;
	}

	// fetch values
	auto radius = parameters[0];
	auto threadCount = static_cast<size_t>(parameters[1]);
	auto maximumBatchSize = static_cast<size_t>(parameters[2]);
#else
	vector<double> parameters(1,0.1);
	if( !showEnterText( parameters, "Radius (1 value)" ) ) {
		return false;
	}
	if( parameters.size() != 1 ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] Wrong number of elements (" << parameters.size() << ") given. 1 expected!" << endl;
		return false;
	}
	if ( parameters[0] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Radius has to be > 0 (given radius: " << parameters[0] << ")" << endl;
		return false;
	}

	// fetch value
	auto radius = parameters[0];
#endif
	auto convertedMesh = convertMesh(*this);
	string funcName = "Sphere Surface Length";
	auto notifyAboutProgress = [&funcName, this](double value){
		this->showProgress(value, funcName);
	};
	auto algorithm = [&radius](const spherical_intersection::Mesh::Vertex &vertex) {
		spherical_intersection::math3d::Sphere sphere{vertex.get_location(), radius};
		spherical_intersection::Graph graph{vertex, sphere};
		return spherical_intersection::algorithm::get_sphere_surface_length(graph);
	};
	showProgressStart( funcName );
#ifdef THREADS
	auto values = calculateSphericalIntersectionFuncValues(convertedMesh, algorithm, threadCount, maximumBatchSize, notifyAboutProgress);
#else
	auto values = calculateSphericalIntersectionFuncValues(convertedMesh, algorithm, notifyAboutProgress);
#endif
	showProgressStop( funcName );
	if ( !applyFuncValues(*this, values) ) {
		return false;
	}

	changedVertFuncVal();
	return true;
#else
	cerr << "[Mesh::" << __FUNCTION__ << "] Functionality missing!" << endl;
	return false;
#endif
}

//! Sets the function value of every vertex to the normalized areea of the intersection of a local part of the mesh volume with a sphere with the vertex as its center
//! Asks for the sphere's radius
//! The normalization factor is 1/(r^2) where r is the sphere's radius
//! @returns False in case of an error. True otherwise.
bool Mesh::funcVertSphereVolumeArea() {
#ifdef LIBSPHERICAL_INTERSECTION
#ifdef THREADS
	vector<double> parameters(3,0);
	parameters[0] = 0.1;
	parameters[1] = 7;
	parameters[2] = 1000;

	if( !showEnterText( parameters, "Radius, thread count and maximum batch size (3 values)" ) ) {
		return false;
	}
	if( parameters.size() != 3 ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] Wrong number of elements (" << parameters.size() << ") given. 3 expected!" << endl;
		return false;
	}
	if ( parameters[0] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Radius has to be > 0 (given radius: " << parameters[0] << ")" << endl;
		return false;
	}
	if ( parameters[1] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Thread count has to be > 0 (given thread count: " << parameters[0] << ")" << endl;
		return false;
	}
	if ( parameters[2] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Maximum batch size has to be > 0 (given maximum batch size: " << parameters[0] << ")" << endl;
		return false;
	}

	// fetch values
	auto radius = parameters[0];
	auto threadCount = static_cast<size_t>(parameters[1]);
	auto maximumBatchSize = static_cast<size_t>(parameters[2]);
#else
	vector<double> parameters(1,0.1);
	if( !showEnterText( parameters, "Radius (1 value)" ) ) {
		return false;
	}
	if( parameters.size() != 1 ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] Wrong number of elements (" << parameters.size() << ") given. 1 expected!" << endl;
		return false;
	}
	if ( parameters[0] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Radius has to be > 0 (given radius: " << parameters[0] << ")" << endl;
		return false;
	}

	// fetch value
	auto radius = parameters[0];
#endif
	auto convertedMesh = convertMesh(*this);
	string funcName = "Sphere Volume Area";
	auto notifyAboutProgress = [&funcName, this](double value){
		this->showProgress(value, funcName);
	};
	auto algorithm = [&radius](const spherical_intersection::Mesh::Vertex &vertex) {
		spherical_intersection::math3d::Sphere sphere{vertex.get_location(), radius};
		spherical_intersection::Graph graph{vertex, sphere};
		return spherical_intersection::algorithm::get_sphere_volume_area(graph);
	};
	showProgressStart( funcName );
#ifdef THREADS
	auto values = calculateSphericalIntersectionFuncValues(convertedMesh, algorithm, threadCount, maximumBatchSize, notifyAboutProgress);
#else
	auto values = calculateSphericalIntersectionFuncValues(convertedMesh, algorithm, notifyAboutProgress);
#endif
	showProgressStop( funcName );
	if ( !applyFuncValues(*this, values) ) {
		return false;
	}

	changedVertFuncVal();
	return true;
#else
	cerr << "[Mesh::" << __FUNCTION__ << "] Functionality missing!" << endl;
	return false;
#endif
}

//! Sets the function value of every vertex to the number of components of the intersection of a local part of the mesh surface with a sphere with the vertex as its center
//! Asks for the sphere's radius
//! @returns False in case of an error. True otherwise.
bool Mesh::funcVertSphereSurfaceNumberOfComponents() {
#ifdef LIBSPHERICAL_INTERSECTION
#ifdef THREADS
	vector<double> parameters(3,0);
	parameters[0] = 0.1;
	parameters[1] = 7;
	parameters[2] = 1000;

	if( !showEnterText( parameters, "Radius, thread count and maximum batch size (3 values)" ) ) {
		return false;
	}
	if( parameters.size() != 3 ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] Wrong number of elements (" << parameters.size() << ") given. 3 expected!" << endl;
		return false;
	}
	if ( parameters[0] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Radius has to be > 0 (given radius: " << parameters[0] << ")" << endl;
		return false;
	}
	if ( parameters[1] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Thread count has to be > 0 (given thread count: " << parameters[0] << ")" << endl;
		return false;
	}
	if ( parameters[2] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Maximum batch size has to be > 0 (given maximum batch size: " << parameters[0] << ")" << endl;
		return false;
	}

	// fetch values
	auto radius = parameters[0];
	auto threadCount = static_cast<size_t>(parameters[1]);
	auto maximumBatchSize = static_cast<size_t>(parameters[2]);
#else
	vector<double> parameters(1,0.1);
	if( !showEnterText( parameters, "Radius (1 value)" ) ) {
		return false;
	}
	if( parameters.size() != 1 ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] Wrong number of elements (" << parameters.size() << ") given. 1 expected!" << endl;
		return false;
	}
	if ( parameters[0] <= 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Radius has to be > 0 (given radius: " << parameters[0] << ")" << endl;
		return false;
	}

	// fetch value
	auto radius = parameters[0];
#endif
	auto convertedMesh = convertMesh(*this);
	string funcName = "Sphere Surface Number of Components";
	auto notifyAboutProgress = [&funcName, this](double value){
		this->showProgress(value, funcName);
	};
	auto algorithm = [&radius](const spherical_intersection::Mesh::Vertex &vertex) {
		spherical_intersection::math3d::Sphere sphere{vertex.get_location(), radius};
		spherical_intersection::Graph graph{vertex, sphere};
		return spherical_intersection::algorithm::get_component_count(graph);
	};
	showProgressStart( funcName );
#ifdef THREADS
	auto values = calculateSphericalIntersectionFuncValues(convertedMesh, algorithm, threadCount, maximumBatchSize, notifyAboutProgress);
#else
	auto values = calculateSphericalIntersectionFuncValues(convertedMesh, algorithm, notifyAboutProgress);
#endif
	showProgressStop( funcName );
	if ( !applyFuncValues(*this, values) ) {
		return false;
	}

	changedVertFuncVal();
	return true;
#else
	cerr << "[Mesh::" << __FUNCTION__ << "] Functionality missing!" << endl;
	return false;
#endif
}
//! Copies the function value from each vertex to the nth component of its feature vector
//! @param dim the component of the feature vector, where the function value is written to. If dim > featureVecSize, then the vector gets padded with zeros to fit dim
//! @returns False in case of an error
bool Mesh::funcValToFeatureVector(unsigned int dim)
{
	for(auto pVertex : mVertices)
	{
		if(dim >= pVertex->getFeatureVectorLen())
		{
			pVertex->resizeFeatureVector(dim + 1);
		}
		double funcVal;
		pVertex->getFuncValue(&funcVal);
		pVertex->setFeatureElement(dim, funcVal);
	}

	return true;
}


//! Compute the correlation of the vertices feature vector and store it as their function value.
bool Mesh::setVertFuncValCorrTo( vector<double>* rFeatVector ) {
	cout << "[Mesh::" << __FUNCTION__ << "] Start" << endl;
	// Sanity checks:
	if( rFeatVector == nullptr ) {
		return false;
	}
	const unsigned int featureVecLenRef = rFeatVector->size();
	if( featureVecLenRef == 0 ) {
		return false;
	}
	string funcName = "Vertex feat.vec. corr.";
	showProgressStart( funcName );
#ifndef ALGLIB
	cerr << "[Mesh::" << __FUNCTION__ << "] Warning: running untested code without ALGLIB!" << endl;

	double* referenceFeature = new double[featureVecLenRef]; //ap::real_1d_array referenceFeature; referenceFeature.setlength( featureVecLenRef );

	cout << "[Mesh::" << __FUNCTION__ << "] reference:";
	for( unsigned int i=0; i<rFeatVector->size(); i++ ) {
		cout << " " << rFeatVector->at( i );
		referenceFeature[i] = rFeatVector->at( i );
	}
	cout << endl;

	int nrOfVertices = getVertexNr();
	for( int vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		const unsigned int featureVecLenAll = currVertex->getFeatureVectorLen();
		if( featureVecLenAll <= 0 ) {
			continue;
		}
		double* someFeature = new double[featureVecLenAll];
		for( unsigned int i=0; i<featureVecLenAll; i++ ) {
			double elementValue;
			currVertex->getFeatureElement( i, &elementValue );
			someFeature[i] = elementValue;
		}
		double* crossCorr = new double[ featureVecLenAll + featureVecLenRef ];
		corrr1d( someFeature, featureVecLenAll, referenceFeature, featureVecLenRef, crossCorr );
		double currFuncVal = 0.0;
		for( unsigned int j=0; j<(featureVecLenRef+featureVecLenAll)-1; j++ ) {
			// Integrate ... while 0 = symmetric
			currFuncVal += crossCorr[j];
		}
		if( !currVertex->setFuncValue( currFuncVal ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Setting vertex function value!" << endl;
		}
		showProgress( (double)(vertIdx+1)/(double)nrOfVertices, funcName );

		delete[] someFeature;
		delete[] crossCorr;

	}

	delete[] referenceFeature;
#else
	ap::real_1d_array crossCorr;
	ap::real_1d_array referenceFeature;
	ap::real_1d_array someFeature;
	referenceFeature.setlength( rFeatVector->size() );

	cout << "[Mesh::" << __FUNCTION__ << "] reference:";
	for( unsigned int i=0; i<rFeatVector->size(); i++ ) {
		cout << " " << rFeatVector->at( i );
		referenceFeature( i ) = rFeatVector->at( i );
	}
	cout << endl;

	int nrOfVertices = getVertexNr();
	for( int vertIdx=0; vertIdx<nrOfVertices; vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		unsigned int featureVecLenAll = currVertex->getFeatureVectorLen();
		if( featureVecLenAll <= 0 ) {
			continue;
		}
		someFeature.setlength( featureVecLenAll );
		for( unsigned int i=0; i<featureVecLenAll; i++ ) {
			double elementValue;
			currVertex->getFeatureElement( i, &elementValue );
			someFeature( i ) = elementValue;
		}
		corrr1d( someFeature, featureVecLenAll, referenceFeature, featureVecLenRef, crossCorr );
		double currFuncVal = 0.0;
		for( unsigned int j=0; j<(featureVecLenRef+featureVecLenAll)-1; j++ ) {
			// Integrate ... while 0 = symmetric
			currFuncVal += crossCorr( j );
		}
		if( !currVertex->setFuncValue( currFuncVal ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Setting vertex function value!" << endl;
		}
		showProgress( static_cast<double>(vertIdx+1)/static_cast<double>(nrOfVertices), funcName );
	}
#endif
	showProgressStop( funcName );
	changedVertFuncVal();

	return true;
}

//! Compute the distance to the center of gravity (COG) of a single
//! selected primitive (SelPrim i.e. SelVert or SelFace).
//!
//! See Mesh::setVertFuncValDistanceTo
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::setVertFuncValDistanceToSelPrim() {
	if( mPrimSelected == nullptr ) {
		cout << "[Mesh::" << __FUNCTION__ << "] Warning: NO Primitive selected!" << endl;
		showInformation( "No selected primitive", "Please, select a primitive (SelVert or SelFace)." );
		return( false );
	}

	Vector3D cogSel= mPrimSelected->getCenterOfGravity();
	vector<double> cogSelVals = { cogSel.getX(), cogSel.getY(), cogSel.getZ() };
	if( !showEnterText( cogSelVals, "Primitive position (x,y,z)" ) ) {
		cout << "[Mesh::" << __FUNCTION__ << "] Warning: NO valid values!" << endl;
		return( false );
	}
	if( cogSelVals.size() < 3 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] Warning: less than three values given!" << endl;
		return( false );
	}
	cogSel.set( cogSelVals.at(0), cogSelVals.at(1), cogSelVals.at(2), 1.0 );

	bool retVal = setVertFuncValDistanceTo( cogSel );
	return( retVal );
}

//! Computes the distance of all vertices to the given position.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::setVertFuncValDistanceTo( const Vector3D& rPos ) {
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		double dist = distanceVV( currVertex, rPos );
		currVertex->setFuncValue( dist );
	}

	changedVertFuncVal();
	return( true );
}


//! Stub to be called, when a the function values of the faces were changed,
void Mesh::changedFaceFuncVal() {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
}

//! To be called, when a the function values of the vertices were changed,
void Mesh::changedVertFuncVal() {
	int timeStartSub = clock(); // for performance mesurement
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Begin.\n";
	for( auto const& currVertex: mVertices ) {
		// Set flags by calling the according method.
		currVertex->isFuncValLocalMinimum();
		currVertex->isFuncValLocalMaximum();
	}
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Time: " << static_cast<float>( clock() - timeStartSub ) / CLOCKS_PER_SEC << " seconds.\n";
}

//! To be called, when the feature vectors were manipulated.
void Mesh::changedVertFeatureVectors() {
	//! \todo compute for feature vectors stored within the vertices.
	//! \bug mFeatureVecVertices seems to hold bad values.
	if( mFeatureVecVertices.size() == 0 ) {
		LOG::warn() << "[Mesh::" << __FUNCTION__ << "] ERROR: No feature vectors found!\n";
		return;
	}
	auto timeStartSub = clock(); // for performance mesurement
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Begin.\n";
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] mFeatureVecVertices.size: " << mFeatureVecVertices.size() << "\n";
	mVerticesFeatVecMean.clear();
	mVerticesFeatVecStd.clear();
	mVerticesFeatVecMean.resize(mFeatureVecVerticesLen,0.0);
	mVerticesFeatVecStd.resize(mFeatureVecVerticesLen,0.0);
	vector<uint64_t> verticesFeatVecNormal(mFeatureVecVerticesLen,0); // Number of elements having normal values of the feature vectors of the vertices. See std::isnormal()

	// Accumulate values for the mean values:
	for( uint64_t i=0; i<getVertexNr(); i++ ) {
		Vertex* currVert = getVertexPos( i );
		for( uint64_t j=0; j<currVert->getFeatureVectorLen(); j++ ) {
			double elementValue = _NOT_A_NUMBER_DBL_;
			currVert->getFeatureElement( j, &elementValue );
			if( isnormal( elementValue ) ) {
				mVerticesFeatVecMean[j] += elementValue;
				verticesFeatVecNormal[j]++;
			}
		}
	}

	// Compute and show mean values:
	for( uint64_t j=0; j<mFeatureVecVerticesLen; j++ ) {
		mVerticesFeatVecMean[j] /= static_cast<double>(verticesFeatVecNormal[j]);
		LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Feature vector mean [" << j << "]: " << mVerticesFeatVecMean[j] << '\n';
		LOG::debug() << " using " << verticesFeatVecNormal[j] << " values.\n";
	}
	// Accumulate values for the standard deviations:
	for( uint64_t i=0; i<getVertexNr(); i++ ) {
		for( uint64_t j=0; j<mFeatureVecVerticesLen; j++ ) {
			double val = mFeatureVecVertices.at( i*mFeatureVecVerticesLen+j );
			if( isnormal( val ) ) {
				mVerticesFeatVecStd[j] += pow( val - mVerticesFeatVecMean[j], 2.0 );
			}
		}
	}
	// Compute and show standard deviations:
	for( uint64_t j=0; j<mFeatureVecVerticesLen; j++ ) {
		mVerticesFeatVecStd[j] /= static_cast<double>(verticesFeatVecNormal[j]);
		mVerticesFeatVecStd[j] = sqrt( mVerticesFeatVecStd[j] );
		LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Feature vector standard deviation [" << j << "]: " << mVerticesFeatVecStd[j] << "\n";
	}
	LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Time: " << static_cast<float>( clock() - timeStartSub ) / CLOCKS_PER_SEC << " seconds.\n";
}

// Information about function values. -----------------------------------------------

//! Compute the average of all function values.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertFuncValAverage( double* rAverage ) {
	double valueCount = 0.0;
	double valueAccu  = 0.0;
	for( auto const& currVertex: mVertices ) {
		double funcVal;
		currVertex->getFuncValue( &funcVal );
		// Skip all infinite, nan, etc. values
		if( !isnormal( funcVal ) && ( funcVal != 0.0 ) ) {
			continue;
		}
		valueCount++;
		valueAccu += funcVal;
	}
	double averageTmp = valueAccu / valueCount;

	// Check for overflow or numeric problems
	if( !isnormal( averageTmp ) && ( averageTmp != 0.0 ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: average is not a normal value nor zero: " << valueAccu << " / " << valueCount << endl;
		return( false );
	}

	(*rAverage) = averageTmp;
	return( true );
}

// --- function values --------------------------------------------------------

//! Returns the minimum of the vertices' function value.
//!
//! Remark: Do not use this method too often (e.g. in a loop) as it may slow down the calling method/function.
//!
//! @return false in case of an error (e.g. no values set). True otherwise.
bool Mesh::getFuncValuesMinMax( double& rMinVal, double& rMaxVal ) {
	// Check if there is any valid number
	bool valueSet = false;
	rMinVal = +DBL_MAX;
	rMaxVal = -DBL_MAX;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double  currFuncVal;
		if( !currVertex->getFuncValue( &currFuncVal ) ) {
			continue;
		}
		if( std::isnan( currFuncVal ) ) {
			continue;
		}
		if( std::isinf( currFuncVal ) ) {
			continue;
		}
		valueSet = true;
		if( rMinVal > currFuncVal ) {
			rMinVal = currFuncVal;
		}
		if( rMaxVal < currFuncVal ) {
			rMaxVal = currFuncVal;
		}
	}

	// Typical for meshs without a quality field:
	if( !valueSet ) {
		rMinVal = _NOT_A_NUMBER_DBL_;
		rMaxVal = _NOT_A_NUMBER_DBL_;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] min: " << rMinVal << " max: " << rMaxVal << " valueSet: " << valueSet << endl;
	return( valueSet );
}

//! Returns the vertices' function values for the given quantiles.
//!
//! Remark: Do not use this method too often (e.g. in a loop) as it may slow down the calling method/function.
//!
//! @return false in case of an error (e.g. no values set). True otherwise.
bool Mesh::getFuncValuesMinMaxQuantil( double  rMinQuantil,
                                       double  rMaxQuantil,
                                       double& rMinVal,
                                       double& rMaxVal
) {
	// Sanity checks:
	if( ( rMinQuantil<0.0 ) ||  ( rMinQuantil>1.0 ) ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Min Quantile out of range!" << endl;
		return( false );
	}
	if( ( rMaxQuantil<0.0 ) ||  ( rMaxQuantil>1.0 ) ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: Max Quantile out of range!" << endl;
		return( false );
	}

	// Fetch all vertices with finite values:
	vector<Vertex*> verticesSortedAndSelected;
	getVertexListValidFuncVal( &verticesSortedAndSelected );
	if( verticesSortedAndSelected.empty() ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: No vertices!" << endl;
		return( false );
	}

	// Sort:
	sort( verticesSortedAndSelected.begin(), verticesSortedAndSelected.end(), Vertex::funcValLower );
	// valid numbers for the quantil exclude NaN, +Inf and - Inf
	// because the first vertices will have -Inf after sorting we need to add their number as an offset:
	long quantilIdxLow  = round( rMinQuantil*static_cast<double>(verticesSortedAndSelected.size()-1) );
	long quantilIdxHigh = round( rMaxQuantil*static_cast<double>(verticesSortedAndSelected.size()-1) );

	// Fetch values:
	double valCur;
	if( !verticesSortedAndSelected.at( quantilIdxLow )->getFuncValue( &valCur ) ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: UNEXPECTED minimum Value (Quantile)!" << endl;
		return( false );
	} else if( !std::isfinite(valCur) ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: INFINITE minimum Value (Quantile)!" << endl;
		return( false );
	} else {
		rMinVal = valCur;
	}
	if( !verticesSortedAndSelected.at( quantilIdxHigh )->getFuncValue( &valCur ) ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: UNEXPECTED maximum Value (Quantile)!" << endl;
		return( false );
	} else if( !std::isfinite(valCur) ) {
		cerr << "[MeshGL::" << __FUNCTION__ << "] ERROR: INFINITE maximum Value (Quantile)!" << endl;
		return( false );
	} else {
		rMaxVal = valCur;
	}

	// Done:
	cout << "[Mesh::" << __FUNCTION__ << "] Quantil Min: " << rMinVal << " at position: " << quantilIdxLow << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] Quantil Max: " << rMaxVal << " at position: " << quantilIdxHigh << endl;
	return( true );
}

//! Returns the minimum of the feature function value.
//! Returns false in case of an error (e.g. no values set).
//!
//! Remark: Do not use this method too often (e.g. in a loop) as it may slow down the calling method/function.
bool Mesh::getFuncValuesMinMaxInfNanFail( double& rMinVal, double& rMaxVal, int& rInfCount, int& rNanCount, int& rFailCount ) {
	bool valueSet = false;
	rMinVal = +DBL_MAX;
	rMaxVal = -DBL_MAX;
	rInfCount  = 0;
	rNanCount  = 0;
	rFailCount = 0;
	Vertex* currVertex;
	double  currFuncVal;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( !currVertex->getFuncValue( &currFuncVal ) ) {
			rFailCount++;
			continue;
		}
		if( std::isnan( currFuncVal ) ) {
			rNanCount++;
			continue;
		}
		if( std::isinf( currFuncVal ) ) {
			rInfCount++;
			continue;
		}
		valueSet = true;
		if( rMinVal > currFuncVal ) {
			rMinVal = currFuncVal;
		}
		if( rMaxVal < currFuncVal ) {
			rMaxVal = currFuncVal;
		}
	}
	return valueSet;
}

//! Returns the minimum of the feature function value.
//! Returns false in case of an error (e.g. no values set).
//!
//! Remark: Do not use this method too often (e.g. in a loop) as it may slow down the calling method/function.
bool Mesh::getFuncValuesMinMaxInfNanFail( double& rMinVal,
                                          double& rMaxVal,
                                          Vertex*& rVertMin,
                                          Vertex*& rVertMax,
                                          int& rInfCount,
                                          int& rNanCount,
                                          int& rFailCount,
                                          uint64_t& rFiniteCount
) {
	bool valueSet = false;
	rMinVal    = +std::numeric_limits<double>::infinity();
	rMaxVal    = -std::numeric_limits<double>::infinity();
	rInfCount  = 0;
	rNanCount  = 0;
	rFailCount = 0;
	rFiniteCount = 0;
	rVertMin   = nullptr;
	rVertMax   = nullptr;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		double currFuncVal = _NOT_A_NUMBER_DBL_;
		if( !currVertex->getFuncValue( &currFuncVal ) ) {
			rFailCount++;
			continue;
		}
		if( std::isfinite( currFuncVal ) ) {
			rFiniteCount++;
		}
		if( std::isnan( currFuncVal ) ) {
			rNanCount++;
			continue;
		}
		if( std::isinf( abs( currFuncVal ) ) ) {
			rInfCount++;
			continue;
		}
		valueSet = true;
		if( rMinVal > currFuncVal ) {
			rMinVal = currFuncVal;
			rVertMin = currVertex;
		}
		if( rMaxVal < currFuncVal ) {
			rMaxVal = currFuncVal;
			rVertMax = currVertex;
		}
	}
	return valueSet;
}

// --- Mesh manipulation - GENERIC -----------------------------------------------------------------------------------------------------------------------------

//! Stub to handle higher level tasks. Typically re-compute OpenGL VBos.
//! @returns false in case of an error. True otherwise.
bool Mesh::changedMesh() {
	return( true );
}

// --- Mesh manipulation - REMOVAL -----------------------------------------------------------------------------------------------------------------------------
//! Removes multiple Vertices from the vertexList. It will also (has to)
//! remove Faces which are defined by the Vertices.
bool Mesh::removeVertices( set<Vertex*>* verticesToRemove ) {
	if( verticesToRemove->empty() ) {
		cout << "[Mesh::" << __FUNCTION__ << "] Nothing to do - no vertices given." << endl;
		return false;
	}

	int facesBefore = getFaceNr();
	int verticesBefore = getVertexNr();
	set<Face*>   facesToRemove;
	set<Vertex*>::iterator itVertexRm;

	// to remove a vertex, we have to determine the faces it belongs to and
	// then remove these faces - otherwise we will screw-up our meshs
	// internal references!

	for(auto vertToRemove : *verticesToRemove)
	{
		vertToRemove->getFaces( &facesToRemove);
	}

	removeFaces( &facesToRemove );
	facesToRemove.clear();
	cout << "[Mesh::" << __FUNCTION__ << "] " << facesBefore-getFaceNr() << " Faces removed." << endl;

	vector<Vertex*> mVerticesNew;
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		if( verticesToRemove->find( (*itVertex) ) != verticesToRemove->end() ) {
			delete (*itVertex);
			continue;
		}
		mVerticesNew.push_back( (*itVertex) );
	}
	mVertices.clear();
	mVertices.swap( mVerticesNew );

	cout << "[Mesh::" << __FUNCTION__ << "] " << verticesBefore - getVertexNr() << " Vertices removed." << endl;
	verticesToRemove->clear();
	mPrimSelected = nullptr;
	estBoundingBox();
	cout << "[Mesh::" << __FUNCTION__ << "] Bounding box estimated." << endl;
	delete mOctree;
	mOctree = nullptr;
	cout << "[Mesh::" << __FUNCTION__ << "] Octree for vertices removed." << endl;

	return true;
}

//! Removes all vertices (and their related faces) stored in mSelectedMVerts.
//! @returns false in case of an error or when no vertices were removed.
bool Mesh::removeVerticesSelected() {
	if( mSelectedMVerts.size() == 0 ) {
		return false;
	}
	bool retVal = removeVertices( &mSelectedMVerts );
	selectedMVertsChanged();
	return retVal;
}

//! Select and remove solo, non-manifold, double-cones and small area vertices.
//! Optional: save result to rFileName.
//! This method has to follow a strict order to achieve a clean Mesh.
//! 
//! Public version writing meta-data.
//!
//! \returns false in case of an error. True otherwise.
bool Mesh::removeUncleanSmall(
        const filesystem::path&   rFileName,      //!< Filename to store intermediate results and the final mesh.
        double                    rPercentArea,   //!< Area relative to the whole mesh.
        bool                      rApplyErosion   //!< Add extra border cleaning.
) {
	bool retVal = false;

	// Track changes for meta-data
	//----------------------------------------------------------
	MeshInfoData rFileInfosPrevious;
	this->getMeshInfoData( rFileInfosPrevious, true );
	uint64_t iterationCount = 0;

	// Measure compute time
	//----------------------------------------------------------
	std::chrono::system_clock::time_point tStart = std::chrono::system_clock::now();

	// CLEAN the mesh
	//----------------------------------------------------------
	retVal = removeUncleanSmallCore( rFileName, rPercentArea, rApplyErosion, iterationCount );

	// Measure compute time
	//----------------------------------------------------------
	std::chrono::system_clock::time_point tStop = std::chrono::system_clock::now();

	// Dump Meta-Data about the cleaning process.
	//----------------------------------------------------------
	MeshInfoData rFileInfos;
	this->getMeshInfoData( rFileInfos, true );
	if( !rFileInfos.writeMeshInfoProcess( rFileInfosPrevious, rFileName, "mesh_clean", iterationCount,
	                                 tStart, tStop ) ) {
		LOG::error() << "[Mesh::" << __FUNCTION__ << "] Writing meta-data failed!\n";
		return( false );
	}

	return( retVal );

}

//! Select and remove solo, non-manifold, double-cones and small area vertices.
//! Optional: save result to rFileName.
//! This method has to follow a strict order to achieve a clean Mesh.
//! 
//! Core version without writing meta-data.
//!
//! \returns false in case of an error. True otherwise.
bool Mesh::removeUncleanSmallCore(
        const filesystem::path&   rFileName,      //!< Filename to store intermediate results and the final mesh.
        double                    rPercentArea,   //!< Area relative to the whole mesh.
        bool                      rApplyErosion,  //!< Add extra border cleaning.
        uint64_t&                 rIteration      //!< Returns the number of iterations in step #7.
) {
	uint64_t vertNoPrev = getVertexNr();
	uint64_t faceNoPrev = getFaceNr();
	// Reset counter
	rIteration = 0;

	std::set<Vertex*> verticesToRemove;
	std::set<Face*> facesToRemove;

	//! 0.) Remove all polylines.
	//!     \todo this has to be done as the polylines will cause a segementation fault, when written to the VBO.
	//!     \bug when polylines are not removed, they cause a crash at the end of this method - seems like a memory leak.
	removePolylinesAll();

	//! 1a.) Select and remove vertices with not-a-number coordinates and ...
	//!     These vertices have to be removed before Non-Manifold and Double-Cones (Singularities) as they may introduce these other types.
	std::cout << "[Mesh::" << __FUNCTION__ << "] --- Not-A-Number Vertices -----------------------" << std::endl;
	getVertNotANumber( &verticesToRemove );
	Mesh::removeVertices( &verticesToRemove );
	//! 1b.) Select and remove vertices of faces having an areo of zero.
	std::cout << "[Mesh::" << __FUNCTION__ << "] --- Zero area faces -----------------------------" << std::endl;
	getVertPartOfZeroFace( &verticesToRemove );
	Mesh::removeVertices( &verticesToRemove );
	//! 2.) Select and remove sticky faces.
	std::cout << "[Mesh::" << __FUNCTION__ << "] --- Sticky --------------------------------------" << std::endl;
	getFaceSticky( &facesToRemove );
	removeFaces( &facesToRemove );
	//! 3.) Select and remove non-manifold faces.
	std::cout << "[Mesh::" << __FUNCTION__ << "] --- Non-Manifold --------------------------------" << std::endl;
	getFaceNonManifold( &facesToRemove );
	removeFaces( &facesToRemove );
	// more agressive removal: getVertNonManifoldFaces( &verticesToRemove );
	//! 4.) Select and remove vertices on edges connecting faces with inverted orientation.
	std::cout << "[Mesh::" << __FUNCTION__ << "] --- Inverted ------------------------------------" << std::endl;
	getVertInverted( verticesToRemove );
	Mesh::removeVertices( &verticesToRemove );
	//! 5.) OPTIONAL apply erosion to remove 'dangling' faces.
	if( rApplyErosion ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] --- Border Erosion ------------------------------" << std::endl;
		if( !removeFacesBorderErosion() ) {
			std::cout << "[Mesh::" << __FUNCTION__ << "] ERROR: removeFacesBorderErosion failed!" << std::endl;
		}
	}
	//! 6.) Select double cones.
	std::cout << "[Mesh::" << __FUNCTION__ << "] --- Double Cones --------------------------------" << std::endl;
	getDoubleCones( &verticesToRemove );
	//! 7.) Remove and select double-cones until there are no more showing up.
	do {
		Mesh::removeVertices( &verticesToRemove ); // "Mesh::" avoids the unnecessary regeneration of OpenGL VBOs within the GUI version - will be taken care of in step 7.
		getDoubleCones( &verticesToRemove );
		rIteration++;
	} while( verticesToRemove.size() > 0 );
	//!     In very rare cases there may be new 'dangling' faces at this point, but no further optional
	//!     erosion will be applied as it requires another slow loop over 4., 5. and 6.
	//! 8.) ... solo vertices and ...
	std::cout << "[Mesh::" << __FUNCTION__ << "] --- Solo Vertices -------------------------------" << std::endl;
	getVertSolo( &verticesToRemove );
	Mesh::removeVertices( &verticesToRemove );
	//! 9.) ... label and select small areas.
	std::cout << "[Mesh::" << __FUNCTION__ << "] --- Small Areas ---------------------------------" << std::endl;
	// first we reset the label and set all NOT to be labled.
	labelVerticesAll();
	getVertLabelAreaRelativeLT( rPercentArea, &verticesToRemove );
	//! 10.) Final remove (including reloading OpenGL buffers and lists.
	removeVertices( &verticesToRemove );

	std::cout << "[Mesh::" << __FUNCTION__ << "] removed " << vertNoPrev - getVertexNr() << " vertices and " 
	          << faceNoPrev - getFaceNr() << " faces." << std::endl;
	if( rFileName.empty() ) {
		return( true );
	}

	//! 11.) Optional: save to file.
	return writeFile( rFileName );
}

//! Removes synthetic connected components using given seed vertices typicall SelMVerts.
//! This function helps to remove falsely filled holes after automatic mesh polishing.
//! @returns false in case of an error or warning. True otherwise.
bool Mesh::removeSyntheticComponents(
		const std::set<Vertex *> &rVerticesSeeds
) {
	std::cout << "[Mesh::" << __FUNCTION__ << "] Seeds: " << rVerticesSeeds.size() << std::endl;

	//! 1.) Select all synthetic vertices and label them.
	set<Vertex*> verticesSynthetic;
	getVertWithFlag( &verticesSynthetic, FLAG_SYNTHETIC );
	if( verticesSynthetic.size() <= 0 ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] WARNING: No synthetic vertices detected." << std::endl;
		return( false );
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] Synthetic: " << verticesSynthetic.size() << std::endl;
	if( !labelSelectedVertices( verticesSynthetic, true ) ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] ERROR: Labeling failed." << std::endl;
		return( false );
	}

	//! 2.) Fetch all label ids of SelMVerts.
	//!     Ignore SelMVerts not being part of a label, because these might be wrong choices
	//!     and would remove most of the mesh.
	std::set<uint64_t> selectedLabelIds;
	getVertLabelIdFrom( rVerticesSeeds, selectedLabelIds );
	std::cout << "[Mesh::" << __FUNCTION__ << "] Number of synthetic components selected: " << selectedLabelIds.size() << std::endl;

	//! 3.) Select all connected components with the label ids determined in the previous step.
	if( !selectVertLabelNo( selectedLabelIds ) ) {
		return( false );
	}

	//! 4.) Remove the selection.
	if( !removeVerticesSelected() ) {
		return( false );
	}

	return( true );
}

//! Removes selected faces.
bool Mesh::removeFacesSelected() {
	cout << "[Mesh::" << __FUNCTION__ << "]." << endl;
	bool retVal = true;
	retVal &= removeFaces( &mFacesSelected );
	selectedMFacesChanged();
	retVal &= changedMesh();
	return( retVal );
}

//! Removes multiple Faces from the faceList.
//! @returns false in case of an error or when no faces were removed.
bool Mesh::removeFaces( set<Face*>* facesToRemove ) {
	if( facesToRemove == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	if( facesToRemove->empty() || mFaces.empty() ) {
		// nothing to do.
		return false;
	}
	int facesBefore = getFaceNr();
	// Maintain face index!
	unsigned int newFaceIndex = 0;
	vector<Face*> mFacesNew;
	bool faceRemoved = false;
	set<Face*>::iterator itFound;
	vector<Face*>::iterator itFace;
	for( itFace = mFaces.begin(); itFace != mFaces.end(); itFace++ ) {
		itFound = facesToRemove->find( (*itFace) );
		if( itFound != facesToRemove->end() ) {
			delete (*itFound);
			facesToRemove->erase( itFound );
			faceRemoved = true;
			continue;
		}
		mFacesNew.push_back( (*itFace) );
		(*itFace)->setIndex( newFaceIndex );
		newFaceIndex++;
	}
	mFaces.clear();
	mFaces.swap( mFacesNew );
	// Set things straight:
	mPrimSelected = nullptr;
	cout << "[Mesh::" << __FUNCTION__ << "] " << facesBefore - getFaceNr() << " Faces removed." << endl;
	return faceRemoved;
}

//! Removes faces with zero area.
//!
//! This is an alternative/older variant of the method used in
//! Mesh::removeUncleanSmall
//! based upon
//! Mesh::getVertPartOfZeroFace
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::removeFacesZeroArea() {
	bool retVal = true;
	retVal &= selectFaceZeroArea();
	retVal &= selectVertFromFaces();
	retVal &= removeVerticesSelected();
	mFacesSelected.clear();
	selectedMFacesChanged();
	return retVal;
}

//! Iterativly removes faces from the border i.e. erosion of
//! 'dangling' faces having three vertices along the border.
//!
//! Solo/single vertices will be removed, when they are related
//! to the erosion. Other solo vertices are kept.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::removeFacesBorderErosion() {
	bool retVal = true;

	uint64_t erosionIterations = 0;
	uint64_t initialVertexNr = getVertexNr();
	uint64_t initialFaceNr = getFaceNr();
	uint64_t oldFaceNr;

	set<Face*> facesToRemove;
	set<Vertex*> verticesCandidatesForRemoval;

	// Erode iterativly
	do {
		oldFaceNr = getFaceNr();
		facesToRemove.clear();
		retVal &= getFaceBorderVertsEdges( facesToRemove, 3, 2 ); // i.e. Dangling faces.
		// Fetch all vertices, which have to be checked for becoming solo/singe.
		for( auto const& currFace: facesToRemove ) {
			currFace->getVertABC( &verticesCandidatesForRemoval );
		}
		if( facesToRemove.size() > 0 ) {
			retVal &= removeFaces( &facesToRemove ); // removeFacesSelected() will be slow due to OpenGL interaction.
		}
		erosionIterations++;
	} while( oldFaceNr - getFaceNr() != 0 );

	// Remove solo vertices found in candidate list
	set<Vertex*> verticesToRemove;
	for( auto const& currVertex: verticesCandidatesForRemoval ) {
		if( currVertex->isSolo() ) {
			verticesToRemove.insert( currVertex );
		}
	}
	uint64_t nrVertsRemoved = verticesToRemove.size();
	removeVertices( &verticesToRemove );

	// Done
	cout << "[Mesh::" << __FUNCTION__ << "] " << to_string( verticesCandidatesForRemoval.size() - nrVertsRemoved ) << " vertices along the border were kept." << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] " << to_string( initialVertexNr - getVertexNr() ) << " vertices removed." << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] " << to_string( initialFaceNr - getFaceNr() ) << " faces removed." << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] " << to_string( erosionIterations ) << " iterations." << endl;
	return( retVal );
}

// --- Mesh manipulation - MESH POLISHING ------------------------------------------------------------------------------------------------------

//! Automatic mesh polishing -- stub used for the GUI.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::completeRestore() {
	// Stub ... see GUI for usage as slot.
	return false;
}

//! Automatic mesh polishing.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::completeRestore(
        const filesystem::path& rFilename,            //!< Optional filname for storing the mesh after each operation. An empty string will prevent saving the mesh.
        double                  rPercentArea,         //!< Connected components with an area smaller are removed. See Mesh::removeUncleanSmall
        bool                    rApplyErosion,        //!< Optional border erosion.
        bool                    rPrevent,             //!< Prevent longest polyline from filling.
        uint64_t                rMaxNumberVertices,   //!< Maximum number of vertices/edges used for filling holes as libpsalm has troubles with larger/complex holes.
        string*                 rResultMsg,           //!< Returns string for display in e.g. a messagebox.
        uint64_t&               rIterationCount       //!< Returns number of iterations.
) {
	// Measure compute time
	//----------------------------------------------------------
	std::chrono::system_clock::time_point tStart = std::chrono::system_clock::now();

	bool retVal = true;
	// Store old mesh size to determine the number of changes
	uint64_t oldVertexNr;
	uint64_t oldFaceNr;

	// Track changes for meta-data
	MeshInfoData rFileInfosPrevious;
	this->getMeshInfoData( rFileInfosPrevious, true );

	// Track iterations and changes to so-called holes
	rIterationCount = 0;
	uint64_t totalHolesFilled = 0;
	uint64_t totalHolesFail = 0;
	uint64_t totalHolesSkipped = 0;
	bool someHolesFilled = true; // Exit condition for the following do-while loop

	do {
		// Track changes to the number of vertices and faces.
		// One cleaning iteration with hole filling.

		oldVertexNr = getVertexNr();
		oldFaceNr = getFaceNr();
		uint64_t subIterationCount = 0;
		removeUncleanSmallCore( rFilename, rPercentArea, rApplyErosion, subIterationCount );
		convertBordersToPolylines();

		if( rPrevent ) {
			selectPolyLongest();
			removePolylinesSelected();
		}

		// Fill so-called holes
		uint64_t holesFilled  = 0;
		uint64_t holesFail    = 0;
		uint64_t holesSkipped = 0;
		fillPolyLines( rMaxNumberVertices, holesFilled, holesFail, holesSkipped );
		totalHolesFilled  += holesFilled;
		totalHolesFail    += holesFail;
		totalHolesSkipped += holesSkipped;
		someHolesFilled = ( holesFilled != 0 );
		// Cleanup after filling holes
		removePolylinesAll();

		rIterationCount++;

	} while( ( oldVertexNr - getVertexNr() ) != 0 ||
	         ( oldFaceNr - getFaceNr() ) !=0      ||
	         ( someHolesFilled ) );

	string tempstr;
	if( ( totalHolesFail == 0 ) && ( totalHolesSkipped == 0 ) ) {
		tempstr = "All holes were filled.";
	}
	if( totalHolesFail>1 ) {
		tempstr = to_string( totalHolesFail ) + " holes were NOT filled.";
		retVal = false;
	}
	if( totalHolesFail==1 ) {
		tempstr = to_string( totalHolesFail ) + " holes were NOT filled.";
		retVal = false;
	}
	if( totalHolesSkipped>1 ) {
		tempstr = to_string( totalHolesSkipped ) + " holes were SKIPPED filled.";
		retVal = false;
	}
	if( totalHolesSkipped==1 ) {
		tempstr = to_string( totalHolesSkipped ) + " holes were SKIPPED filled.";
		retVal = false;
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] " << tempstr << std::endl;
	std::cout << "[Mesh::" << __FUNCTION__ << "] " << std::to_string( rIterationCount ) << " iterations." << std::endl;

	if( rResultMsg != nullptr ) {
		(*rResultMsg) = tempstr + "\n\n" + std::to_string( rIterationCount ) + " iterations.";
	}

	// Adjust/refresh selected vertices:
	mSelectedMVerts.clear();
	for( auto const& currVertex: mVertices ) {
		if( currVertex->getFlag( FLAG_SELECTED ) ) {
			mSelectedMVerts.insert( currVertex );
		}
	}
	selectedMVertsChanged();

	// Adjust/refresh selected faces:
	mFacesSelected.clear();
	for( auto const& currFace: mFaces ) {
		if( currFace->getFlag( FLAG_SELECTED ) ) {
			mFacesSelected.insert( currFace );
		}
	}
	selectedMFacesChanged();

	// Measure compute time
	//----------------------------------------------------------
	std::chrono::system_clock::time_point tStop = std::chrono::system_clock::now();

	// Dump Meta-Data about the cleaning process.
	//----------------------------------------------------------
	MeshInfoData rFileInfos;
	this->getMeshInfoData( rFileInfos, true );
	if( !rFileInfos.writeMeshInfoProcess( rFileInfosPrevious, rFilename, "mesh_polish", rIterationCount,
	                                 tStart, tStop ) ) {
		LOG::error() << "[Mesh::" << __FUNCTION__ << "] Writing meta-data failed!\n";
		return( false );
	}

	// Done
	//----------------------------------------------------------
	return( retVal );
}

// --- Mesh manipulation - Manuall adding primitives -------------------------------------------------------------------------------------------

//! Manually insert vertices by entering triplets of coordinates.
//! Stub ... see GUI for usage and OpenGL class for updating buffers.
//! \todo implement for console mode.
//! @returns true in case of an error. False otherwise.
bool Mesh::insertVerticesEnterManual() {
	return true;
}

//! Insert triplets of coordinates as vertices.
//! See also Mesh::insertVerticesEnterManual()
//! @returns true in case of an error. False otherwise.
bool Mesh::insertVerticesCoordTriplets( vector<double>* rCoordTriplets ) {
	cout << "[Mesh::" << __FUNCTION__ << "] Insert Vertices." << endl;
	// Sanity check
	if( rCoordTriplets == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] NULL pointer given!" << endl;
		return false;
	}
	if( rCoordTriplets->size() % 3 != 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Bad number of values given: " << rCoordTriplets->size() << "!" << endl;
		return false;
	}

	for( unsigned int i=0; i<rCoordTriplets->size(); i+=3 ) {
		Vertex* newVert = new Vertex( Vector3D( rCoordTriplets->at(i), rCoordTriplets->at(i+1), rCoordTriplets->at(i+2) ) );
		newVert->setFlag( FLAG_SYNTHETIC | FLAG_MANUAL );
		mVertices.push_back( newVert );
		cout << "[Mesh::" << __FUNCTION__ << "] Insert Vertex( " << rCoordTriplets->at(i) << " , " << rCoordTriplets->at(i+1) << " , " << rCoordTriplets->at(i+2) << " )" << endl;
	}

	return true;
}

//! Inserts new existing vertices using a vector with given references.
//! See also Mesh::insertVerticesCoordTriplets()
//! @returns true in case of an error. False otherwise.
bool Mesh::insertVertices( vector<Vertex*>* rNewVertices ) {
	// Sanity check
	if( rNewVertices == nullptr ) {
		return false;
	}
	// Insert by appending:
	for( auto const& currVertexRef: (*rNewVertices) ) {
		mVertices.push_back( currVertexRef );
	}
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------

// mainly used to set the initial view (see objwidget) -------------------------

void Mesh::getCenterOfGravity( float* cog ) {
	//! (Old) Estimates the center of gravity (shortly called COG) of the meshes
	//! vertices.
	cog[0] = 0;
	cog[1] = 0;
	cog[2] = 0;

	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		cog[0] += curVertex->getX();
		cog[1] += curVertex->getY();
		cog[2] += curVertex->getZ();
	}

	cog[0] /= static_cast<float>(getVertexNr());
	cog[1] /= static_cast<float>(getVertexNr());
	cog[2] /= static_cast<float>(getVertexNr());
}

//! (New) Estimates the center of gravity (shortly called COG) of the meshes
//! vertices.
//!
//! @returns a Vector3D with the center of gravity.
Vector3D Mesh::getCenterOfGravity() {
	double cogX = 0.0;  // double is numerically a bit safer
	double cogY = 0.0;
	double cogZ = 0.0;

	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		cogX += curVertex->getX();
		cogY += curVertex->getY();
		cogZ += curVertex->getZ();
	}

	cogX /= static_cast<double>(getVertexNr());
	cogY /= static_cast<double>(getVertexNr());
	cogZ /= static_cast<double>(getVertexNr());

	return Vector3D( static_cast<float>(cogX), static_cast<float>(cogY), static_cast<float>(cogZ) );
}

Vector3D Mesh::getBoundingBoxCenter() {
	//! Estimates and returns the center of the bounding box.
	Vector3D bbCenter;
	bbCenter.setX( ( mMaxX + mMinX ) / 2.0 );
	bbCenter.setY( ( mMaxY + mMinY ) / 2.0 );
	bbCenter.setZ( ( mMaxZ + mMinZ ) / 2.0 );
	return bbCenter;
}

//! Compute and returns the size of the bounding box.
bool Mesh::getBoundingBoxSize( Vector3D& rBbSize ) const {
	rBbSize.setX( mMaxX - mMinX );
	rBbSize.setY( mMaxY - mMinY );
	rBbSize.setZ( mMaxZ - mMinZ );
	return( true );
}

//! Compute the radius of a sphere enclosing the bounding box.
double Mesh::getBoundingBoxRadius() {
	Vector3D bbSize;
	getBoundingBoxSize( bbSize );
	double boundingBoxRadius = abs3( bbSize ) / 2.0;
	return boundingBoxRadius;
}

float Mesh::getPerimeterRadius() {
	//! Estimate the distance to the Vertex with the largest distance to the
	//! center of gravity (COG).
	float cogXYZ[3];
	float dx, dy, dz;
	float perimeterRadius, perimeterRadiusMax=0;

	getCenterOfGravity( cogXYZ );
	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		dx = curVertex->getX() - cogXYZ[0];
		dy = curVertex->getY() - cogXYZ[1];
		dz = curVertex->getZ() - cogXYZ[2];
		perimeterRadius = sqrt( pow( dx, static_cast<float>(2.0) ) + pow( dy, static_cast<float>(2.0) ) + pow( dz, static_cast<float>(2.0) ) );
		if( perimeterRadius > perimeterRadiusMax ) {
			perimeterRadiusMax = perimeterRadius;
		}
	}
	return perimeterRadiusMax;
}

bool Mesh::getDistanceToPlaneMinMax(
		const Plane& rPlane,
		double&   rMinDist,
		double&   rMaxDist,
		bool      rAbsDist
) {
	double minDist = +INFINITY;
	double maxDist = -INFINITY;

	Vector3D planeHNF = rPlane.getHNF();

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		double dist = currVertex->estDistanceToPlane( planeHNF, rAbsDist );
		minDist = min( dist, minDist );
		maxDist = max( dist, maxDist );
	}

	rMinDist = minDist;
	rMaxDist = maxDist;

	std::cout << "[Mesh::" << __FUNCTION__ << "] dist: " << minDist << " " << maxDist << std::endl;
	return( true );
}


// Bounding Box Corners --------------------------------------------------------

Vector3D Mesh::getBoundingBoxA() {
	//! Returns a corner from the bounding boxes bottom plane.
	return Vector3D( mMinX, mMinY, mMinZ, 1.0 );
}

Vector3D Mesh::getBoundingBoxB() {
	//! Returns a corner from the bounding boxes bottom plane.
	return Vector3D( mMinX, mMaxY, mMinZ, 1.0 );
}

Vector3D Mesh::getBoundingBoxC() {
	//! Returns a corner from the bounding boxes bottom plane.
	return Vector3D( mMaxX, mMaxY, mMinZ, 1.0 );
}

Vector3D Mesh::getBoundingBoxD() {
	//! Returns a corner from the bounding boxes bottom plane.
	return Vector3D( mMaxX, mMinY, mMinZ, 1.0 );
}

Vector3D Mesh::getBoundingBoxE() {
	//! Returns a corner from the bounding boxes top plane.
	return Vector3D( mMinX, mMinY, mMaxZ, 1.0 );
}

Vector3D Mesh::getBoundingBoxF() {
	//! Returns a corner from the bounding boxes top plane.
	return Vector3D( mMinX, mMaxY, mMaxZ, 1.0 );
}

Vector3D Mesh::getBoundingBoxG() {
	//! Returns a corner from the bounding boxes top plane.
	return Vector3D( mMaxX, mMaxY, mMaxZ, 1.0 );
}

Vector3D Mesh::getBoundingBoxH() {
	//! Returns a corner from the bounding boxes top plane.
	return Vector3D( mMaxX, mMinY, mMaxZ, 1.0 );
}

//! Get the Bounding Box of a projection.
//! Typically used to determine the size in camera coordinates.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getBoundingBoxProjected(
		const Matrix4D& rTransMat,
		double& rMinX, double& rMaxX,
		double& rMinY, double& rMaxY,
		double& rMinZ, double& rMaxZ
) const {
	bool retVal = true;
	rMinX = std::numeric_limits<double>::max();
	rMaxX = std::numeric_limits<double>::min();
	rMinY = std::numeric_limits<double>::max();
	rMaxY = std::numeric_limits<double>::min();
	rMinZ = std::numeric_limits<double>::max();
	rMaxZ = std::numeric_limits<double>::min();
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* curVertex = getVertexPos( vertIdx );
		Vector3D curPosProjected;
		if( curVertex->getTransformedCenterOfGravity( rTransMat, curPosProjected ) ) {
			rMinX = min( rMinX, curPosProjected.getX() );
			rMaxX = max( rMaxX, curPosProjected.getX() );
			rMinY = min( rMinY, curPosProjected.getY() );
			rMaxY = max( rMaxY, curPosProjected.getY() );
			rMinZ = min( rMinZ, curPosProjected.getZ() );
			rMaxZ = max( rMaxZ, curPosProjected.getZ() );
		} else {
			retVal = false;
		}
	}
	return( retVal );
}

// mesh information ----------------------------------------------------------------------------

//! Add vertices not-a-number coordinates to a givens set.
//! @returns true, when vertices were added. False otherwise.
bool Mesh::getVertNotANumber( set<Vertex*>* rSomeVerts ) {
	if( rSomeVerts == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return( false );
	}
	bool vertsAdded = false;
	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( curVertex->isNotANumber() ) {
			rSomeVerts->insert( curVertex );
			vertsAdded = true;
		}
	}
	return( vertsAdded );
}

//! Add vertices being part of face having an area of zero
//! to a givens set.
//! @returns true, when vertices were added. False otherwise.
bool Mesh::getVertPartOfZeroFace( set<Vertex*>* rSomeVerts ) {
	if( rSomeVerts == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return( false );
	}
	bool vertsAdded = false;
	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( curVertex->isPartOfZeroFace() ) {
			rSomeVerts->insert( curVertex );
			vertsAdded = true;
		}
	}
	return( vertsAdded );
}

//! Add solo vertices to a givens set.
//! @returns true, when vertices were added.
bool Mesh::getVertSolo( set<Vertex*>* rSomeVerts ) {
	if( rSomeVerts == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	bool vertsAdded = false;
	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( curVertex->isSolo() ) {
			rSomeVerts->insert( curVertex );
			vertsAdded = true;
		}
	}
	return vertsAdded;
}

bool Mesh::getVertNonManifoldFaces( set<Vertex*>* rSomeVerts ) {
	//! Add all vertices from non-manifold faces to a given set.
	//! Returns true, when vertices were added.
	if( rSomeVerts == nullptr ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	bool vertsAdded = false;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		if( currFace->isManifold() ) {
			continue;
		}
		rSomeVerts->insert( currFace->getVertA() );
		rSomeVerts->insert( currFace->getVertB() );
		rSomeVerts->insert( currFace->getVertC() );
		vertsAdded = true;
	}
	return vertsAdded;
}

bool Mesh::getDoubleCones( set<Vertex*>* rSomeVerts ) {
	//! Returns a list/set of single Vertices connecting two or more parts
	//! of the same or different meshs.
	//! Returns true, when vertices were added.
	bool vertsAdded = false;
	Vertex* curVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		curVertex = getVertexPos( vertIdx );
		if( curVertex->connectedToFacesCount() == 0 ) {
			// we can save some time to skip vertices not belonging to any face
			// to achieve the bigger picture/goal of a clean mesh, we have
			// to remove solo edges and vertices at another point.
			continue;
		}
		if( curVertex->isDoubleCone() ) {
			rSomeVerts->insert( curVertex );
			vertsAdded = true;
		}
	}
	return vertsAdded;
}

//! Determines the id of the largest connected component.
//!
//! Typically used for cleaning purposes.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertLabelAreaLargest(
                uint64_t& rLargestLabelId   //!< Id of the largest connected component (return value)
) {
	double      largestArea    = -_INFINITE_DBL_;
	long        largestLabelId = -1;
	set<Face*>* labelFaces;
	uint64_t labelsNr;

	// Fetch faces grouped by
	if( !labelFacesVert( &labelFaces, labelsNr ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: labelFacesVert failed!" << endl;
		return( false );
	}

	// Estimate area, and select vertices and clear memory.
	set<Face*>::iterator itFace;
	for( uint64_t i=0; i<labelsNr; i++ ) {
		double labelArea = 0.0;
		for( itFace=labelFaces[i].begin(); itFace!=labelFaces[i].end(); itFace++ ) {
			labelArea += (*itFace)->getAreaNormal();
		}
		cout << "[Mesh::" << __FUNCTION__ << "] Label " << i+1 << " area: " << labelArea << endl;
		if( labelArea >= largestArea ) {
			// New largest label found.
			largestArea = labelArea;
			largestLabelId = i+1;
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Largest Label is No. " << largestLabelId << " having an area of " << largestArea << endl;

	// Write back the result
	rLargestLabelId = largestLabelId;

	delete[] labelFaces;

	return( true );
}

bool Mesh::getVertLabelAreaLT( double rAreaMax, set<Vertex*>* rSomeVerts ) {
	//! Fetches all vertices of labels having an area smaller than rAreaMax.
	//! Uses vertex based labels.
	//!
	//! Remark: this will not select single vertices, even they got their own label no!

	set<Face*>*   labelFaces;
	uint64_t labelsNr;

	// Fetch faces grouped by
	if( !labelFacesVert( &labelFaces, labelsNr ) ) {
		return false;
	}

	// Estimate area, and select vertices and clear memory.
	set<Face*>::iterator itFace;
	for( uint64_t i=0; i<labelsNr; i++ ) {
		double labelArea = 0.0;
		for( itFace=labelFaces[i].begin(); itFace!=labelFaces[i].end(); itFace++ ) {
			labelArea += (*itFace)->getAreaNormal();
		}
		cout << "[Mesh::" << __FUNCTION__ << "] Label " << i << " area: " << labelArea << endl;
		if( labelArea >= rAreaMax ) {
			// We don't need the faces anymore and jump to the next label.
			labelFaces[i].clear();
			continue;
		}
		// Add the faces vertices to the given set of vertices.
		for( itFace=labelFaces[i].begin(); itFace!=labelFaces[i].end(); itFace++ ) {
			rSomeVerts->insert( (*itFace)->getVertA() );
			rSomeVerts->insert( (*itFace)->getVertB() );
			rSomeVerts->insert( (*itFace)->getVertC() );
		}
		// We don't need the faces anymore.
		labelFaces[i].clear();
	}

	// Clear allocated memory
	delete[] labelFaces;

	return true;
}

//! Fetches all vertices of labels having an area smaller than rAreaMax.
//! Uses vertex based labels.
//! rPercent accepts only ] 0.0... 1.0 [
//!
//! Remark: this will not select single vertices, even they got their own label no!
bool Mesh::getVertLabelAreaRelativeLT( double rPercent, set<Vertex*>* rSomeVerts ) {

	set<Face*>*   labelFaces;
	uint64_t labelsNr;

	// Fetch faces grouped by
	if( !labelFacesVert( &labelFaces, labelsNr ) ) {
		cout << "[Mesh::" << __FUNCTION__ << "] No labels found." << endl;
		return false;
	}
	if( rPercent <= 0.0 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] negative or zero value given: " << rPercent << endl;
		return false;
	}
	if( rPercent >= 1.0 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] value equal or greater then 1.0 given: " << rPercent << endl;
		return false;
	}

	const uint64_t labelsNrConst = labelsNr;

	// Estimate area
	double labelAreaTotal = 0.0;
	double* labelAreas = new double[labelsNrConst];
	set<Face*>::iterator itFace;
	for( uint64_t i=0; i<labelsNrConst; i++ ) {
		labelAreas[i] = 0.0;
		for( itFace=labelFaces[i].begin(); itFace!=labelFaces[i].end(); itFace++ ) {
			labelAreas[i] += (*itFace)->getAreaNormal();
		}
		cout << "[Mesh::" << __FUNCTION__ << "] Label " << i << " area: " << labelAreas[i] << endl;
		labelAreaTotal += labelAreas[i];
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Total area: " << labelAreaTotal << endl;

	// Select vertices and clear memory.
	int labelsRemoved = 0;
	for( uint64_t i=0; i<labelsNrConst; i++ ) {
		if( labelAreas[i]/labelAreaTotal >= rPercent ) {
			// We don't need the faces anymore and jump to the next label.
			labelFaces[i].clear();
			continue;
		}
		// Add the faces vertices to the given set of vertices.
		for( itFace=labelFaces[i].begin(); itFace!=labelFaces[i].end(); itFace++ ) {
			rSomeVerts->insert( (*itFace)->getVertA() );
			rSomeVerts->insert( (*itFace)->getVertB() );
			rSomeVerts->insert( (*itFace)->getVertC() );
		}
		labelsRemoved++;
		// We don't need the faces anymore.
		labelFaces[i].clear();
	}

	delete[] labelFaces;
	delete[] labelAreas;

	cout << "[Mesh::" << __FUNCTION__ << "] " << labelsRemoved << " out of " << labelsNrConst << " labels selected." << endl;
	// Clear allocated memory
	//! \todo check why delete causes a segmentation fault, when used.
	//delete labelFaces;

	return true;
}

bool Mesh::getVertLabled( set<Vertex*>* rSomeVerts ) {
	//! Adds all vertices having the status labeld to rSomeVerts.
	//! Returns false in case of an error.
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( currVertex->isLabled() ) {
			rSomeVerts->insert( currVertex );
		}
	}
	return true;
}

bool Mesh::getVertBorder( set<Vertex*>* rSomeVerts ) {
	//! Adds all vertices along the border of the Mesh and add them to rSomeVerts.
	//! Returns false in case of an error.
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( currVertex->isBorder() ) {
			rSomeVerts->insert( currVertex );
		}
	}
	return true;
}

bool Mesh::getVertFaceMinAngleLT( double rMaxAngle, set<Vertex*>* rSomeVerts ) {
	//! Adds all vertices from all faces with a maximum angle larger than rMinAngle to rSomeVerts.
	//! rMinAngle has to be in radiant.
	Face*  currFace;
	double currMinAngle;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		currFace->getMinAngle( &currMinAngle );
		if( currMinAngle >= rMaxAngle ) {
			continue;
		}
		rSomeVerts->insert( currFace->getVertA() );
		rSomeVerts->insert( currFace->getVertB() );
		rSomeVerts->insert( currFace->getVertC() );
	}
	return true;
}

bool Mesh::getVertFaceMaxAngleGT( double rMinAngle, set<Vertex*>* rSomeVerts ) {
	//! Adds all vertices from all faces with a maximum angle larger than rMinAngle to rSomeVerts.
	//! rMinAngle has to be in radiant.
	Face*  currFace;
	double currMaxAngle;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		currFace->getMaxAngle( &currMaxAngle );
		if( currMaxAngle <= rMinAngle ) {
			continue;
		}
		rSomeVerts->insert( currFace->getVertA() );
		rSomeVerts->insert( currFace->getVertB() );
		rSomeVerts->insert( currFace->getVertC() );
	}
	return true;
}

//! Adds all vertices with a given label number to the given set.
//! Accepts a negative index for inverted selection.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertLabelNoSingle(
		long rLabelNr,
		std::set<Vertex*>& rSomeVerts
) {
	// Step thru the vertices:
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		uint64_t labelNr;
		if( currVertex->getLabel( labelNr ) ) {
			if( ( rLabelNr < 0 ) && ( labelNr != static_cast<uint64_t>(-rLabelNr) ) ) {
				rSomeVerts.insert( currVertex );
			}
			if( ( rLabelNr >= 0 ) && ( labelNr == static_cast<uint64_t>(rLabelNr) ) ) {
				rSomeVerts.insert( currVertex );
			}
		}
	}
	return( true );
}

//! Adds all vertices with the given label numbers to the given set.
//! Accepts negative indices for inverted selection.
//! Attention: all numbers have to be either positive or negative!
//! A mixture of positive and negative values will result in an error.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertLabelNoMulti(
		const std::set<int64_t>& rLabelNrs,
		std::set<Vertex*>& rSomeVerts
) {
	// Convert vector to set, which will provide the find method.
	set<int64_t> labelsToSelect;
	set<int64_t>::iterator itLabelNo;
	bool labelNrsNegative = false;
	bool labelNrsPositive = false;
	for( itLabelNo=rLabelNrs.begin(); itLabelNo!=rLabelNrs.end(); itLabelNo++ ) {
		int currLabelNr = (*itLabelNo);
		labelsToSelect.insert( currLabelNr );
		if( currLabelNr < 0 ) {
			labelNrsNegative = true;
		} else {
			labelNrsPositive = true;
		}
	}
	if( labelNrsPositive && labelNrsNegative ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] ERROR: Mixture of Negative and positive values is not applicable and therefore not allowed." << std::endl;
		return( false );
	}
	// Step thru the vertices:
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		uint64_t labelNr;
		if( currVertex->getLabel( labelNr ) ) {
			if( labelNrsPositive && ( labelsToSelect.find( labelNr ) != labelsToSelect.end() ) ) {
				rSomeVerts.insert( currVertex );
			}
			if( labelNrsNegative && ( labelsToSelect.find( -static_cast<int64_t>(labelNr) ) == labelsToSelect.end() ) ) {
				rSomeVerts.insert( currVertex );
			}
		}
	}
	return( true );
}

//! Adds all vertices with the given label numbers to the given set.
//! Does NOT accept negative indices for inverted selection.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertLabelNoMulti(
		const std::set<uint64_t>& rLabelNrs,
		std::set<Vertex*>& rSomeVerts
) {
	// Step thru the vertices:
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		uint64_t labelNr;
		if( currVertex->getLabel( labelNr ) ) {
			if( rLabelNrs.find( labelNr ) != rLabelNrs.end() ) {
				rSomeVerts.insert( currVertex );
			}
		}
	}
	return( true );
}

//! Adds vertices, which are labeled background, to a given set.
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertLabelBackGrd( std::set<Vertex*>& rSomeVerts ) {
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		if( currVertex->isLabelBackGround() ) {
			rSomeVerts.insert( currVertex );
		}
	}
	return( true );
}

//! Adds vertices, which are not labeled and not background, to a given set.
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertLabeledNot( set<Vertex*>* rSomeVerts ) {
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		if( ( !currVertex->isLabelBackGround()) && ( !currVertex->isLabled() ) ) {
			rSomeVerts->insert( currVertex );
		}
	}
	return true;
}

//! Fetch the label ids from a given set of vertices.
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertLabelIdFrom(
		const std::set<Vertex*>& rSomeVerts,
		std::set<uint64_t>& rLabelNrs
) {
	for( auto const& currVertex: mVertices ) {
		if( rSomeVerts.count( currVertex ) > 0 ) {
			uint64_t currLabelId = _NOT_A_NUMBER_UINT_;
			if( currVertex->getLabel( currLabelId ) ) {
				rLabelNrs.insert( currLabelId );
				std::cout << "[Mesh::" << __FUNCTION__ << "] Component Id for selection: " << currLabelId << std::endl;
			}
		}
	}
	return( true );
}

//! Adds vertices, which have one or more flags set.
//! See Primitive::ePrimitiveFlags
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertWithFlag( set<Vertex*>* rSomeVerts, ePrimitiveFlags rFlag ) {
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		if( currVertex->getFlag( rFlag ) ) {
			rSomeVerts->insert( currVertex );
		}
	}
	return true;
}

//! Adds all vertices being part of an edge connecting inverted faces
//! to the given set.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getVertInverted( set<Vertex*>& rSomeVerts ) {
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		if( currVertex->isInverse() ) {
			rSomeVerts.insert( currVertex );
		}
	}
	return( true );
}

bool Mesh::getFaceFlag( set<Face*>* rSomeFaces, int rFlag ) {
	//! Adds all faces with a specific flag to the given set.
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		if( currFace->getFlag( rFlag ) ) {
			rSomeFaces->insert( currFace );
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomeFaces->size() << " with flag " << rFlag << endl;
	return true;
}

bool Mesh::getFaceSticky( set<Face*>* rSomeFaces ) {
	//! Adds all sticky faces to the given set.
	return getFaceFlag( rSomeFaces, FLAG_FACE_STICKY );
}

bool Mesh::getFaceNonManifold( set<Face*>* rSomeFaces ) {
	//! Adds all non-manifold faces to the given set.
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		if( currFace->isNonManifold() ) {
			rSomeFaces->insert( currFace );
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomeFaces->size() << endl;
	return true;
}

//! Adds all faces having three border vertices to
//! the given set. These faces contain 'dangling', 'bridge'
//! and others.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getFaceBorderThreeVertices( set<Face*>& rSomeFaces ) {
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		unsigned int numberBorderVert;
		currFace->hasBorderVertex( numberBorderVert );
		if( numberBorderVert >= 3 ) {
			rSomeFaces.insert( currFace );
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomeFaces.size() << endl;
	return( true );
}

//! Adds all faces having three vertices selected to
//! the given set.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getFaceThreeVerticesSelected( std::set<Face*>& rSomeFaces ) {
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		if( !currFace->getVertA()->getFlag( FLAG_SELECTED ) ) {
			continue;
		}
		if( !currFace->getVertB()->getFlag( FLAG_SELECTED ) ) {
			continue;
		}
		if( !currFace->getVertC()->getFlag( FLAG_SELECTED ) ) {
			continue;
		}
		rSomeFaces.insert( currFace );
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomeFaces.size() << std::endl;
	return( true );
}

//! Adds all faces having a given number of border vertices
//! and border edges to the given set.
//!
//! 'Bridges' have rHasBorderVertices == 3 and rHasBorderEdges == 1
//! 'Dangling' have rHasBorderVertices == 3 and rHasBorderEdges == 2
//!
//! Typically user for border erosion.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getFaceBorderVertsEdges(
                set<Face*>& rSomeFaces,
                unsigned int rHasBorderVertices,
                unsigned int rHasBorderEdges
) {
	if( rHasBorderVertices > 3 ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] ERROR: Wrong number of vertices " << rHasBorderVertices << " - vaild: [0...3]!" << std::endl;
		return( false );
	}
	if( rHasBorderEdges > 3 ) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] ERROR: Wrong number of edges " << rHasBorderEdges << " - vaild: [0...3]!" << std::endl;
		return( false );
	}
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		unsigned int numberBorderVertices;
		currFace->hasBorderVertex( numberBorderVertices );
		unsigned int numberBorderEdges;
		currFace->hasBorderEdges( numberBorderEdges );
		if( ( numberBorderVertices >= rHasBorderVertices ) && ( numberBorderEdges == rHasBorderEdges ) ) {
			rSomeFaces.insert( currFace );
		}
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomeFaces.size() << std::endl;
	return( true );
}

//! Adds all faces being a shared corner of three voronoi cells to
//! the given set.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getFaceLabeledVerticesCorner(
                set<Face*>& rSomeFaces
) {
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		if( currFace->isLabelVoronoiCorner() ) {
			rSomeFaces.insert( currFace );
		}
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomeFaces.size() << std::endl;
	return( true );
}

//! Adds all faces with zero area to the given set.
//! See Mesh::getFaceFlag
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getFaceZeroArea( set<Face*>* rSomeFaces ) {
	return getFaceFlag( rSomeFaces, FLAG_FACE_ZERO_AREA );
}

//! Adds all faces containing the at least one of the vertices
//! within the given set.
//!
//! Attention: this might become slow, when the vertex set is large!
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getFaceContainsVert(
		const std::set<Vertex*>& rSomeVerts,
		std::set<Face*>& rSomeFaces
) {
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		if( currFace->requiresOneOrMoreVerticesOf( rSomeVerts ) ) {
			rSomeFaces.insert( currFace );
		}
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomeFaces.size() << std::endl;
	return( true );
}

//! Adds all faces containing vertices with a given label number
//! to the given set.
//!
//! Note: only face with all vertices having the same label number will be added.
//! Due to the use of Face::getLabel
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getFaceHasVertLabelNo(
		const uint64_t rLabelNr,
		std::set<Face*>& rSomeFaces
) {
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		uint64_t currLabel;
		if( currFace->getLabel( currLabel ) ) {
			if( rLabelNr == currLabel ) {
				rSomeFaces.insert( currFace );
			}
		}
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] in set: " << rSomeFaces.size() << std::endl;
	return( true );
}

//! Adds all faces containing vertices with the given label numbers
//! to the given set.
//!
//! Note: only face with all vertices having the same label number will be added.
//! Due to the use of Face::getLabel
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getFaceHasVertLabelNo(
		const std::set<uint64_t>& rLabelNrs,
		std::set<Face*>& rSomeFaces
) {
	for( uint64_t faceIdx = 0; faceIdx < getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		uint64_t currLabel;
		if( currFace->getLabel( currLabel ) ) {
			// search for the iterator of given numbers in set
			std::set<uint64_t>::iterator it = rLabelNrs.find( currLabel );
			// Check if iterator it is valid
			if( it != rLabelNrs.end() ) {
				rSomeFaces.insert( currFace );
			}
		}
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] in set: " << rSomeFaces.size() << std::endl;
	return( true );
}

// --- Polylines -----------------------------------------------------------------------------------------------------------------------------------------------

//! Adds all polylines not being part/border of a label to the given set.
bool Mesh::getPolyNoLabel( set<PolyLine*>* rSomePolyLines ) {
	// Sanity check:
	if( ( rSomePolyLines != nullptr ) && ( mPolyLines.size() == 0 ) ) {
		return false;
	}
	vector<PolyLine*>::iterator itPoly;
	for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
		if( (*itPoly)->isLabled() ) {
			continue;
		}
		rSomePolyLines->insert( (*itPoly) );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Polylines: " << mPolyLines.size() << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomePolyLines->size() << endl;
	return true;
}

//! Adds the polylines with a minimum run length to the given set.
bool Mesh::getPolyRunLenGT( set<PolyLine*>* rSomePolyLines, double rValue ) {
	// Sanity check:
	if( ( rSomePolyLines != nullptr ) && ( mPolyLines.size() == 0 ) ) {
		return false;
	}
	vector<PolyLine*>::iterator itPoly;
	for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
		double currLen;
		if( !(*itPoly)->getLengthAbs( &currLen ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: fetching polyline length!" << endl;
			continue;
		}
		if( currLen <= rValue ) {
			continue;
		}
		rSomePolyLines->insert( (*itPoly) );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Polylines: " << mPolyLines.size() << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomePolyLines->size() << endl;
	return true;
}

//! Adds the polylines with a maximum run length to the given set.
bool Mesh::getPolyRunLenLT( set<PolyLine*>* rSomePolyLines, double rValue ) {
	// Sanity check:
	if( ( rSomePolyLines != nullptr ) && ( mPolyLines.size() == 0 ) ) {
		return false;
	}
	vector<PolyLine*>::iterator itPoly;
	for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
		double currLen;
		if( !(*itPoly)->getLengthAbs( &currLen ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: fetching polyline length!" << endl;
			continue;
		}
		if( currLen >= rValue ) {
			continue;
		}
		rSomePolyLines->insert( (*itPoly) );
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Polylines: " << mPolyLines.size() << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomePolyLines->size() << endl;
	return true;
}

//! Adds the longest polyline to the given set.
bool Mesh::getPolyLongest( set<PolyLine*>* rSomePolyLines ) {
	// Sanity check:
	if( ( rSomePolyLines != nullptr ) && ( mPolyLines.size() == 0 ) ) {
		return false;
	}
	sort( mPolyLines.begin(), mPolyLines.end(), PolyLine::sortByLenDesc );
	vector<PolyLine*>::iterator itPolyVec = mPolyLines.begin();
	rSomePolyLines->insert( (*itPolyVec) );
	cout << "[Mesh::" << __FUNCTION__ << "] Polylines: " << mPolyLines.size() << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomePolyLines->size() << endl;
	return true;
}

//! Adds the shortest polyline to the given set.
bool Mesh::getPolyShortest( set<PolyLine*>* rSomePolyLines ) {
	// Sanity check:
	if( ( rSomePolyLines != nullptr ) && ( mPolyLines.size() == 0 ) ) {
		return false;
	}
	sort( mPolyLines.begin(), mPolyLines.end(), PolyLine::sortByLenAsc );
	vector<PolyLine*>::iterator itPolyVec = mPolyLines.begin();
	rSomePolyLines->insert( (*itPolyVec) );
	cout << "[Mesh::" << __FUNCTION__ << "] Polylines: " << mPolyLines.size() << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] in set:    " << rSomePolyLines->size() << endl;
	return true;
}

//! Adds all polylines with the given label numbers to the given set.
bool Mesh::getPolyLabelNo( const vector<int>& rLabelNrs, set<PolyLine*>* rSomePolyLines ) {
	// Sanity check:
	if( ( rSomePolyLines != nullptr ) && ( mPolyLines.size() == 0 ) ) {
		return false;
	}
	// Convert vector to set, which will provide the find method.
//	set<int> labelsToSelect;
//	vector<int>::iterator itLabelNo;
//	for( itLabelNo=rLabelNrs.begin(); itLabelNo!=rLabelNrs.end(); itLabelNo++ ) {
//		labelsToSelect.insert( (*itLabelNo) );
//	}
	set<int> labelsToSelect(rLabelNrs.begin(), rLabelNrs.end());
	// Step thru the polyline:
	vector<PolyLine*>::iterator itPoly;
	PolyLine* currPoly;
	for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
		currPoly = (*itPoly);
		uint64_t labelNr;
		if( currPoly->getLabel( labelNr ) ) {
			if( labelsToSelect.find( labelNr ) != labelsToSelect.end() ) {
				rSomePolyLines->insert( currPoly );
			}
		}
	}
	return true;
}

//! Adds all polylines not part of a labelto the given set.
bool Mesh::getPolyNotLabel( set<PolyLine*>* rSomePolyLines ) {
	// Sanity check:
	if( ( rSomePolyLines != nullptr ) && ( mPolyLines.size() == 0 ) ) {
		return false;
	}
	// Step thru the polyline:
	vector<PolyLine*>::iterator itPoly;
	PolyLine* currPoly;
	for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
		currPoly = (*itPoly);
		if( !currPoly->isLabled() ) {
			rSomePolyLines->insert( currPoly );
		}
	}
	return true;
}

//! Adds all polyline with a given range of numbers of vertices to the given set.
bool Mesh::getPolyVertexCount( set<PolyLine*>* rSomePolyLines, unsigned int rMinNr, unsigned int rMaxNr ) {
	// Sanity check:
	if( ( rSomePolyLines != nullptr ) && ( mPolyLines.size() == 0 ) ) {
		return false;
	}
	// Step thru the polyline:
	unsigned int linesAdded = 0;
	for( auto const& currPoly: mPolyLines ) {
		unsigned int currLen = currPoly->length();
		if( ( currLen >= rMinNr ) && ( currLen <= rMaxNr ) ) {
			rSomePolyLines->insert( currPoly );
			linesAdded++;
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Lines added: " << linesAdded << endl;
	return true;
}

//! Returns a list/set of Face connected having non-manifold connections.
set<Face*> Mesh::getNonManifoldFaces() {
	set<Face*> nonManifoldFaces;
	for( auto const& currFace: mFaces ) {
		if( currFace->isNonManifold() ) {
			nonManifoldFaces.insert( currFace );
		}
	}
	return nonManifoldFaces;
}

//! Computes the volume within the 3D-Model using the divergence theorem.
//! Remark: for a proper result the surface has to be closed (no holes/borders) and manifold.
//!
//! See also: Face::getVolumeDivergence
//!
//! @returns false in case of an error, which are caused by zero area faces.
bool Mesh::getMeshVolumeDivergence(
                double& rVolumeDX, //!< Volume estimated in x-direction.
                double& rVolumeDY, //!< Volume estimated in y-direction.
                double& rVolumeDZ  //!< Volume estimated in z-direction.
) {
	bool retVal = true;

	rVolumeDX = 0.0;
	rVolumeDY = 0.0;
	rVolumeDZ = 0.0;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		if( !currFace->getVolumeDivergence( rVolumeDX, rVolumeDY, rVolumeDZ ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: getVolumeDivergence did not return a result. Probably a zero area face was encountered!" << endl;
			retVal = false;
		}
	}
	return( retVal );
}

//! Compute the volume below the Mesh's faces to the Mesh plane.
//! @returns false in case of an error, which are caused by zero area faces.
bool Mesh::compVolumePlane( double* rVolumePos, double* rVolumeNeg ) {
	bool retVal = true;
	if( ( rVolumePos == nullptr ) || ( rVolumeNeg == nullptr ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Null pointer given!" << endl;
		return false;
	}
	(*rVolumePos) = 0.0;
	(*rVolumeNeg) = 0.0;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		Face* currFace = getFacePos( faceIdx );
		double faceVolume = 0.0;
		bool   planePos;
		if( !currFace->getVolumeToPlane( &faceVolume, &planePos, &mPlane ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: getVolumeToPlane did not return a result. Probably a zero area face was encountered!" << endl;
			retVal = false;
		}
		if( planePos ) {
			(*rVolumePos) += faceVolume;
		} else {
			(*rVolumeNeg) += faceVolume;
		}
	}
	return retVal;
}

// Histogram (NEW) ---------------------------------------------------------------------------------------------------------------------------------------------

//! Returns an integer array for rendering a raster image of a histogram
bool Mesh::getHistogramValues( eHistogramType        rHistType,  //!< Type of the values shown by the histogram
                           vector<unsigned int>* rNumArray,  //!< Array to be filled has to be of length rNumValues
                           double* rValMin,                  //!< Minimum value represented by rNumArray.
                           double* rValMax                   //!< Maximum value represented by rNumArray.
    ) {
	cout << "[Mesh::" << __FUNCTION__ << "] Histogram request for # " << rHistType << " ." << endl;
	// Sanity checks
	if( ( rNumArray == nullptr ) || ( rValMin == nullptr ) || ( rValMax == nullptr ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: NULL given!" << endl;
		return false;
	}

	// Fetch values
	unsigned int histValuesNr = 0;
	double*      histValues = nullptr;
	switch( rHistType ) {
		case HISTOGRAM_EDGE_LENGTH: {
			histValuesNr = 3*getFaceNr();
			histValues   = new double[histValuesNr];
			Face* currFace;
			for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
				currFace = getFacePos( faceIdx );
				currFace->getEdgeLengths( &(histValues[faceIdx*3]) );
			}
		} break;
		case HISTOGRAM_FEATURE_ELEMENTS_VERTEX: {
			Vertex* currVertex;
			for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
				currVertex = getVertexPos( vertIdx );
				histValuesNr += currVertex->getFeatureVectorLen();
			}
			histValues = new double[histValuesNr];
			unsigned int arrayPos = 0;
			for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
				currVertex = getVertexPos( vertIdx );
				currVertex->copyFeatureVecTo( &histValues[arrayPos] );
				arrayPos += currVertex->getFeatureVectorLen();
			}
		} break;
		case HISTOGRAM_FEATURE_ELEMENTS_VERTEX_DIM: {
			histValuesNr = getVertexNr();
			histValues   = new double[histValuesNr];
			int dimNr;
			getParamIntMesh( HISTOGRAM_SHOW_FEATURE_ELEMENT_VERTEX_DIM, &dimNr );
			Vertex* currVertex;
			for( unsigned int vertIdx=0; vertIdx<histValuesNr; vertIdx++ ) {
				currVertex = getVertexPos( vertIdx );
				currVertex->getFeatureElement( dimNr, &histValues[vertIdx] );
			}
		} break;
		case HISTOGRAM_FUNCTION_VALUES_VERTEX: {
			histValuesNr = getVertexNr();
			histValues   = new double[histValuesNr];
			Vertex* currVertex;
			for( unsigned int vertIdx=0; vertIdx<histValuesNr; vertIdx++ ) {
				currVertex = getVertexPos( vertIdx );
				currVertex->getFuncValue( &histValues[vertIdx] );
			}
		} break;
		case HISTOGRAM_FUNCTION_VALUES_VERTEX_LOCAL_MINIMA: {
			histValuesNr = getVertexNr();
			histValues   = new double[histValuesNr];
			Vertex* currVertex;
			for( unsigned int vertIdx=0; vertIdx<histValuesNr; vertIdx++ ) {
				currVertex = getVertexPos( vertIdx );
				histValues[vertIdx] = _NOT_A_NUMBER_DBL_;
				if( currVertex->isFuncValLocalMinimum() ) {
					currVertex->getFuncValue( &histValues[vertIdx] );
				}
			}
		} break;
		case HISTOGRAM_FUNCTION_VALUES_VERTEX_LOCAL_MAXIMA: {
			histValuesNr = getVertexNr();
			histValues   = new double[histValuesNr];
			Vertex* currVertex;
			for( unsigned int vertIdx=0; vertIdx<histValuesNr; vertIdx++ ) {
				currVertex = getVertexPos( vertIdx );
				histValues[vertIdx] = _NOT_A_NUMBER_DBL_;
				if( currVertex->isFuncValLocalMaximum() ) {
					currVertex->getFuncValue( &histValues[vertIdx] );
				}
			}
		} break;
		case HISTOGRAM_ANGLES_FACES_MINIMUM: {
			histValuesNr = getFaceNr();
			histValues   = new double[histValuesNr];
			Face* currFace;
			// Step thru all faces, fetch largest face angle and determine the global minimum and maximum
			for( unsigned int faceIdx=0; faceIdx<histValuesNr; faceIdx++ ) {
				currFace = getFacePos( faceIdx );
				double currAngle;
				currFace->getMinAngle( &currAngle );
				currAngle *= 180.0/M_PI;
				histValues[faceIdx] = currAngle;
			}
		} break;
		case HISTOGRAM_ANGLES_FACES_MAXIMUM: {
			histValuesNr = getFaceNr();
			histValues   = new double[histValuesNr];
			Face* currFace;
			// Step thru all faces, fetch largest face angle and determine the global minimum and maximum
			for( unsigned int faceIdx=0; faceIdx<histValuesNr; faceIdx++ ) {
				currFace = getFacePos( faceIdx );
				double currAngle;
				currFace->getMaxAngle( &currAngle );
				currAngle *= 180.0/M_PI;
				histValues[faceIdx] = currAngle;
			}
		} break;
		case HISTOGRAM_POLYLINE_RUNLENGHTS: {
			PolyLine* currPoly;
			vector<double> polyLens;
			for( unsigned int i=0; i<getPolyLineNr(); i++ ) {
				currPoly = getPolyLinePos( i );
				double currLen;
				if( !currPoly->getLengthAbs( &currLen ) ) {
					cerr << "[MeshQt::" << __FUNCTION__ << "] ERROR: estimating polyline length!" << endl;
					continue;
				}
				polyLens.push_back( currLen );
			}
			sort( polyLens.begin(), polyLens.end() );

			histValuesNr = polyLens.size();
			histValues   = new double[histValuesNr];
//			double* polyIndex = new double[histValuesNr];
			for( unsigned int i=0; i<histValuesNr; i++ ) {
//				polyIndex[i] = static_cast<double>(i);
				histValues[histValuesNr-i-1] = polyLens.at( i );
				cout << "[MeshQt::" << __FUNCTION__ << "] Polyline " << i << " length: " << histValues[histValuesNr-i-1] << endl;
			}
		} break;
		default:
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Unknown histogram choice: " << rHistType << "!" << endl;
			return false;
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Histogram value cardinality: " << histValuesNr << endl;

	// Compute span of the values
	(*rValMin) = +_INFINITE_DBL_;
	(*rValMax) = -_INFINITE_DBL_;
	unsigned int skippedValues = 0;
	for( unsigned int i=0; i<histValuesNr; i++ ) {
		double histValueCurr = histValues[i];
		// Skip infinite and NaN
		if( !isfinite( histValueCurr ) ) {
			skippedValues++;
			continue;
		}
		if( histValueCurr<(*rValMin) ) {
			(*rValMin) = histValueCurr;
		}
		if( histValueCurr>(*rValMax) ) {
			(*rValMax) = histValueCurr;
		}
	}
	cout << "[Mesh::" << __FUNCTION__ << "] Histogram value range from " << (*rValMin) << " to " << (*rValMax) << "" << endl;
	cout << "[Mesh::" << __FUNCTION__ << "] Ignored " << skippedValues << " values (Infinite, NaN)" << endl;
	double histRange = (*rValMax) - (*rValMin);
	if( histRange <= DBL_EPSILON ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Histogram values have no range!" << endl;
		if(histValues)
		{
			delete[] histValues;
		}
		return false;
	}
	double histIntervalLen = histRange / rNumArray->size();
	if( histIntervalLen <= DBL_EPSILON ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Numeric Error: Zero interval length for histogram!" << endl;
		if(histValues)
		{
			delete[] histValues;
		}
		return false;
	}

	// Compute histogram
	for( unsigned int i=0; i<histValuesNr; i++ ) {
		if( std::isnan( histValues[i] ) ) {
			// we encountered a deleted element and have to
			continue;
		}
		unsigned int histIndex = floor( ( histValues[i] - (*rValMin) ) / histIntervalLen );
		// when we encounter the maximum value:
		if( histIndex >= rNumArray->size() ) {
			rNumArray->at( rNumArray->size() -1 ) = rNumArray->at( rNumArray->size() -1 ) +1;
			continue;
		}
		rNumArray->at( histIndex ) = rNumArray->at( histIndex ) +1;
	}

	cout << "[Mesh::" << __FUNCTION__ << "] Histogram # " << rHistType << " successful." << endl;
	if(histValues)
	{
		delete[] histValues;
	}
	return true;
}

/* ===========================================================================*/

// Bit arrays for labeling, fetch in sphere, etc. ---------------------------------------

//! Allocates and initalizes a bit array for vertices.
//! @returns the number of blocks (length) of the bit array.
uint64_t Mesh::getBitArrayVerts(
                uint64_t** rVertBitArray,
                eBitArrayFlags  rInitFlags
) {
	uint64_t vertNrLongs = getVertexNr() / ( 8*sizeof( uint64_t ) ) + 1;
	// cout << "[Mesh::" << __FUNCTION__  << "] vertNrLongs: " << vertNrLongs << endl;
	(*rVertBitArray) = new uint64_t[vertNrLongs];
	//cout << "[Mesh::getBitArrayVertices]: vertNrLongs: " << vertNrLongs << " equals " << vertNrLongs*sizeof( uint64_t ) << " bytes." << endl;
	// init as calloc does not set values to zero:
	for( uint64_t i=0; i<vertNrLongs; i++ ) {
		(*rVertBitArray)[i] = 0x00000000;
	}
	if( rInitFlags & BIT_ARRAY_MARK_LABEL_BACKGR ) {
		Vertex* currVertex;
		for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
			currVertex = getVertexPos( vertIdx );
			if( currVertex->isLabelBackGround() ) {
				int bitOffset;
				int bitNr;
				currVertex->getIndexOffsetBit( &bitOffset, &bitNr );
				(*rVertBitArray)[bitOffset] |= static_cast<uint64_t>(1)<<bitNr;
			}
		}
	}
	if( rInitFlags & BIT_ARRAY_MARK_BORDER ) {
		Vertex* currVertex;
		for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
			currVertex = getVertexPos( vertIdx );
			if( currVertex->isBorder() ) {
				int bitOffset;
				int bitNr;
				currVertex->getIndexOffsetBit( &bitOffset, &bitNr );
				(*rVertBitArray)[bitOffset] |= static_cast<uint64_t>(1)<<bitNr;
			}
		}
	}

	return vertNrLongs;
}

//! Allocates and initalizes a bit array for faces.
//! @returns the number of blocks (length) of the bit array.
uint64_t Mesh::getBitArrayFaces(
                uint64_t** rFaceBitArray,
                eBitArrayFlags  rInitFlags
) {
	uint64_t faceNrLongs = getFaceNr() / ( 8*sizeof( uint64_t ) ) + 1;
	(*rFaceBitArray) = new uint64_t[faceNrLongs];
	//cout << "[Mesh::getBitArrayFaces]: faceNrLongs: " << faceNrLongs << " equals " << faceNrLongs*sizeof( uint64_t ) << " bytes." << endl;
	// init as calloc does not set values to zero:
	for( uint64_t i=0; i<faceNrLongs; i++ ) {
		(*rFaceBitArray)[i] = 0x00000000;
	}
	if( rInitFlags & BIT_ARRAY_MARK_LABEL_BACKGR ) {
		Face* currFace;
		for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
			currFace = getFacePos( faceIdx );
			if( currFace->isLabelBackGround() ) {
				uint64_t  bitOffset;
				uint64_t  bitNr;
				currFace->getIndexOffsetBit( &bitOffset, &bitNr );
				(*rFaceBitArray)[bitOffset] |= static_cast<uint64_t>(1)<<bitNr;
			}
		}
	}
	if( rInitFlags & BIT_ARRAY_MARK_BORDER ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: BIT_ARRAY_MARK_BORDER not implemented!" << endl;
	}
	return faceNrLongs;
}

//! Allocates and initalizes a bit array for edges (assuming manifoldness!).
//! @returns the number of blocks (length) of the bit array.
uint64_t Mesh::getBitArrayEdges(
                uint64_t** rEdgeBitArray
) {
	uint64_t edgeNrLongs = getFaceNr() * 3 / ( 8*sizeof( uint64_t ) ) + 1;
	(*rEdgeBitArray) =  new uint64_t[edgeNrLongs];
	//cout << "[Mesh::getBitArrayedges]: edgeNrLongs: " << edgeNrLongs << " equals " << edgeNrLongs*sizeof( uint64_t ) << " bytes." << endl;
	// init as calloc does not set values to zero:
	for( uint64_t i=0; i<edgeNrLongs; i++ ) {
		(*rEdgeBitArray)[i] = 0x00000000;
	}
	return edgeNrLongs;
}

// Estimate geodesic neighbourhood -----------------------------------------------------

//! Estimate a geodesic patch for the selected vertices (SelMVerts) -- in sequential order.
bool Mesh::geodPatchVertSel( bool rWeightFuncVal, bool rGeodDistToFuncVal ) {
	return geodPatchVertSel( &mSelectedMVerts, rWeightFuncVal, rGeodDistToFuncVal );
}

//! Estimate a geodesic patch for the given vertices -- in sequential order.
bool Mesh::geodPatchVertSel( set<Vertex*>* rmLabelSeedVerts,  //!< List of seed vertices without duplicates (i.e. set).
                             bool rWeightFuncVal,             //!< If true, use the function values as weigth for the euclidean distances.
                             bool rGeodDistToFuncVal          //!< If true, Write the geodesic distance to the vertices' function values.
                           ) {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;

	if( rmLabelSeedVerts->size() == 0 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] no vertices given." << endl;
		return false;
	}

	// Geodesic distances mapped to vertices:
	map<Vertex*,GeodEntry*> geoDistList;

	// Prepare bit array for visited faces:
	uint64_t* faceBitArray;
	int faceNrBlocks = getBitArrayFaces( &faceBitArray, BIT_ARRAY_MARK_LABEL_BACKGR );

	// Marching front
	deque<EdgeGeodesic*> frontEdges;
	// Heap:
	make_heap( frontEdges.begin(), frontEdges.end() );

	set<Vertex*>::iterator itVertex;
	for( itVertex=rmLabelSeedVerts->begin(); itVertex!=rmLabelSeedVerts->end(); itVertex++ ) {
		// Get initial marching front and mark seed faces visited:
		if( !(*itVertex)->getGeodesicEdgesSeed( &frontEdges, &geoDistList, faceBitArray, faceNrBlocks, rWeightFuncVal ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] initializing front failed!" << endl;
		}
	}

	// Start marching:
	bool retVal = estGeodesicPatch( &geoDistList, &frontEdges, _INFINITE_DBL_, faceBitArray, rWeightFuncVal ); // faceNrBlocks not used

	// Write to function values:
	if( rGeodDistToFuncVal ) {
		estGeodesicPatchFuncVal( &geoDistList ); // "Geodesic Distances for a selection of Vertices."
	}

	// Relabel:
	int newLabelNr = 1;
	for( itVertex=rmLabelSeedVerts->begin(); itVertex!=rmLabelSeedVerts->end(); itVertex++ ) {
		(*itVertex)->setLabel( newLabelNr );
		newLabelNr++;
	}
	estGeodesicPatchRelabel( &geoDistList );

	// Free memory:
	map<Vertex*,GeodEntry*>::iterator itGeod;
	for( itGeod = geoDistList.begin(); itGeod != geoDistList.end(); itGeod++ ) {
		delete (*itGeod).second;
	}
	geoDistList.clear();

	// Remove bit array:
	delete faceBitArray;

	return retVal;
}

//! Estimate a geodesic patch for the selected vertices (SelMVerts) -- in sequential order.
bool Mesh::geodPatchVertSelOrder( bool rWeightFuncVal,            //!< Weight the geodesic distance by the function values of the vertices.
                                  bool rGeodDistToFuncVal         //!< Write the geodesic distance into the vertices' function values.
                                ) {
	vector<Vertex*> mLabelSeedVerts;
	set<Vertex*>::iterator itVert;
	for( itVert=mSelectedMVerts.begin(); itVert!=mSelectedMVerts.end(); itVert++ ) {
		mLabelSeedVerts.push_back( (*itVert) );
	}
	return geodPatchVertSelOrder( &mLabelSeedVerts, rWeightFuncVal, rGeodDistToFuncVal );
}

//! Estimate a geodesic patch for the given vertices -- in sequential order.
bool Mesh::geodPatchVertSelOrder( vector<Vertex*>* rmLabelSeedVerts,    //!< List of vertices used as seed.
                                  bool             rWeightFuncVal,      //!< Weight the geodesic distance by the function values of the vertices.
                                  bool             rGeodDistToFuncVal   //!< Write the geodesic distance into the vertices' function values.
                                ) {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;

	if( rmLabelSeedVerts->size() == 0 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] no vertices given." << endl;
		return false;
	}

	//! *) Sort by function value (descending).
	sort( rmLabelSeedVerts->begin(), rmLabelSeedVerts->end(), Vertex::funcValDescendingLabel );
	//sort( rmLabelSeedVerts->begin(), rmLabelSeedVerts->end(), Vertex::funcValDescending );

	//! *) Clear stop criteria flag (Primitive::FLAG_MARCHING_FRONT_ABORT) for all vertices.
	if( !clearVertexFlagForAll( Primitive::FLAG_MARCHING_FRONT_ABORT ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: clearVertexFlagForAll failed!" << endl;
		return false;
	}

	//! *) Set stop criteria flag for the given vertices.
	vector<Vertex*>::iterator itVertex;
	for( itVertex=rmLabelSeedVerts->begin(); itVertex!=rmLabelSeedVerts->end(); itVertex++ ) {
		if( !(*itVertex)->setFlag( Primitive::FLAG_MARCHING_FRONT_ABORT ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: set flag!" << endl;
		}
	}

	// Geodesic distances mapped to vertices:
	map<Vertex*,GeodEntry*> geoDistList;

	// Prepare bit array for visited faces:
	uint64_t* faceBitArray;
	int faceNrBlocks = getBitArrayFaces( &faceBitArray, BIT_ARRAY_MARK_LABEL_BACKGR );

	// Marching front
	deque<EdgeGeodesic*> frontEdges;
	// Heap:
	make_heap( frontEdges.begin(), frontEdges.end() );

	bool retVal = true;
	uint64_t labelIDLast = _NOT_A_NUMBER_UINT_;
	for( itVertex=rmLabelSeedVerts->begin(); itVertex!=rmLabelSeedVerts->end(); itVertex++ ) {
		uint64_t labelIDCurr = _NOT_A_NUMBER_UINT_;
		(*itVertex)->getLabel( labelIDCurr );
		if( labelIDCurr != labelIDLast ) {
			frontEdges.clear();
			labelIDLast = labelIDCurr;
		}
		// Get initial marching front and mark seed faces visited:
		if( !(*itVertex)->getGeodesicEdgesSeed( &frontEdges, &geoDistList, faceBitArray, faceNrBlocks, rWeightFuncVal ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: initializing front failed!" << endl;
		}
		// Start marching:
		if( estGeodesicPatch( &geoDistList, &frontEdges, _INFINITE_DBL_, faceBitArray, rWeightFuncVal ) ) { // faceNrBlocks not used
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: marching front failed!" << endl;
		}
		//frontEdges.clear();
	}

	// Write to function values:
	if( rGeodDistToFuncVal ) {
		estGeodesicPatchFuncVal( &geoDistList ); // "Geodesic Distances for a selection of Vertices."
	}

	// Relabel:
	uint64_t newLabelNr = 1;
	for( itVertex=rmLabelSeedVerts->begin(); itVertex!=rmLabelSeedVerts->end(); itVertex++ ) {
		// Reset sub-label number per larger connected component (main label)
		uint64_t labelIDCurr = _NOT_A_NUMBER_UINT_;
		(*itVertex)->getLabel( labelIDCurr );
		if( labelIDCurr != labelIDLast ) {
			newLabelNr = 0;
			labelIDLast = labelIDCurr;
		}
		(*itVertex)->setLabel( newLabelNr );
		newLabelNr++;
	}
	estGeodesicPatchRelabel( &geoDistList );

	// Free memory:
	map<Vertex*,GeodEntry*>::iterator itGeod;
	for( itGeod = geoDistList.begin(); itGeod != geoDistList.end(); itGeod++ ) {
		delete (*itGeod).second;
	}
	geoDistList.clear();

	// Remove bit array:
	delete faceBitArray;

	return retVal;
}

//! Estimate the geodesic distance to SelPrim (SelVert or SelFace).
//!
//! @returns false in case of an error or no valid selection. True otherwise.
bool Mesh::estGeodesicPatchSelPrim() {
	bool useFuncValsAsWeigth = false;
	getParamFlagMesh( GEODESIC_USE_FUNCVAL_AS_WEIGHT, &useFuncValsAsWeigth );
	Primitive* primSel = getPrimitiveSelected();
	// Sanity check.
	if( primSel == nullptr ) {
		// nothing selected => nothing to do
		return( false );
	}
	// Support for one selected vertex (SelVert).
	if( primSel->getType() == Primitive::IS_VERTEX ) {
		// Does the vertex belong to a face?
		if( !(static_cast<Vertex*>(primSel))->belongsToFace() ) {
			cout << "[Mesh::" << __FUNCTION__ << "] Selected vertex does not belong to a face." << endl;
			return( false );
		}
		return estGeodesicPatchFuncVal( static_cast<Vertex*>(primSel), _INFINITE_DBL_, useFuncValsAsWeigth );
	}
	// Support for one selected Face (SelFace).
	if( primSel->getType() == Primitive::IS_FACE ) {

		return estGeodesicPatchFuncVal( static_cast<Face*>(primSel), _INFINITE_DBL_, useFuncValsAsWeigth );
	}
	return( false );
}

//! Transfers the label id of the seed Primitive to all related vertices of the geodesic neighbourhood.
//! @returns false in case of an error.
bool Mesh::estGeodesicPatchRelabel( map<Vertex*,GeodEntry*>* geoDistList ) {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;
	bool errorOccured = false;
	map<Vertex*,GeodEntry*>::iterator itVertex;
	for( itVertex=geoDistList->begin(); itVertex!=geoDistList->end(); itVertex++ ) {
		uint64_t labelSeed;
		if( !(*itVertex).second->getLabel( labelSeed ) ) {
			errorOccured = true;
			cerr << "[Mesh::" << __FUNCTION__ << "] getlabel failed!" << endl;
			continue;
		}
		if( !(*itVertex).first->setLabel( labelSeed ) ) {
			errorOccured = true;
			cerr << "[Mesh::" << __FUNCTION__ << "] setlabel failed!" << endl;
			continue;
		}
	}
	labelsChanged();
	return !errorOccured;
}

//! Writes the geodesic distances to the vertices function values.
//! @returns false in case of an error.
bool Mesh::estGeodesicPatchFuncVal(
                map<Vertex*,GeodEntry*>* rGeoDistList
) {
	bool     storeAngle = false;
	getParamFlagMesh( MeshParams::GEODESIC_STORE_DIRECTION, &storeAngle );

	int idxVert = 0;
	map<Vertex*,GeodEntry*>::iterator itVertex;
	for( itVertex=rGeoDistList->begin(); itVertex!=rGeoDistList->end(); itVertex++ ) {
		GeodEntry* currGeodEntry = (*itVertex).second;
		double geodValue;
		if( storeAngle ) {
			currGeodEntry->getGeodAngle( &geodValue );
		} else {
			currGeodEntry->getGeodDist( &geodValue );
		}
		(*itVertex).first->setFuncValue( geodValue );
		//cout << "Vertex " << (*itVertex).first->getIndex() << " d: " << (float)(*(*itVertex).second) << endl;
		Vertex* currVert = (*itVertex).first;
		currVert->setFunctionValue( geodValue );
		idxVert++;
	}

	changedVertFuncVal();
	return( true );
}

//! Computes a geodesic patch starting from a single vertex with a given radius and stores it as function value.
bool Mesh::estGeodesicPatchFuncVal( Vertex* seedVertex, double radius, bool weightFuncVal ) {
	// Geodesic distances mapped to vertices:
	map<Vertex*,GeodEntry*> geoDistList;

	if( !estGeodesicPatch( seedVertex, radius, &geoDistList, weightFuncVal ) ) {
		return false;
	}

	// Write to function values:
	estGeodesicPatchFuncVal( &geoDistList ); // "Geodesic Distances for a selected Vertex."

	// Free memory:
	map<Vertex*,GeodEntry*>::iterator itGeod;
	for( itGeod = geoDistList.begin(); itGeod != geoDistList.end(); itGeod++ ) {
		delete (*itGeod).second;
	}
	geoDistList.clear();

	return true;
}

//! Computes a geodesic patch starting from a single face with a given radius and stores it as function value.
bool Mesh::estGeodesicPatchFuncVal( Face* seedFace, double radius, bool weightFuncVal ) {
	// Geodesic distances mapped to vertices:
	map<Vertex*,GeodEntry*> geoDistList;

	if( !estGeodesicPatch( seedFace, radius, &geoDistList, weightFuncVal ) ) {
		return false;
	}

	// Write to function values:
	estGeodesicPatchFuncVal( &geoDistList ); // "Geodesic Distances for a selected Face."

	// Free memory:
	map<Vertex*,GeodEntry*>::iterator itGeod;
	for( itGeod = geoDistList.begin(); itGeod != geoDistList.end(); itGeod++ ) {
		delete (*itGeod).second;
	}
	geoDistList.clear();

	return true;
}

bool Mesh::estGeodesicPatch( Vertex* seedVertex, double radius, map<Vertex*,GeodEntry*>* geoDistList, bool weightFuncVal ) {
	//! Estimates a geodesic patch starting from a single Vertex, with a given radius.

	// Prepare bit array for visited faces:
	uint64_t* faceBitArray;
	int faceNrBlocks = getBitArrayFaces( &faceBitArray, BIT_ARRAY_MARK_LABEL_BACKGR );

	// estimate geodesic patch:
	if( !estGeodesicPatch( seedVertex, radius, geoDistList, faceBitArray, faceNrBlocks, weightFuncVal ) ) {
		return false;
	}

	// Remove bit array:
	delete faceBitArray;

	return true;
}

bool Mesh::estGeodesicPatch( Face* seedFace, double radius, map<Vertex*,GeodEntry*>* geoDistList, bool weightFuncVal ) {
	//! Estimates a geodesic patch starting from a single Face, with a given radius.

	// Prepare bit array for visited faces:
	uint64_t* faceBitArray;
	int faceNrBlocks = getBitArrayFaces( &faceBitArray );

	// estimate geodesic patch:
	if( !estGeodesicPatch( seedFace, radius, geoDistList, faceBitArray, faceNrBlocks, weightFuncVal ) ) {
		return false;
	}

	// Remove bit array:
	delete faceBitArray;

	return true;
}

bool Mesh::estGeodesicPatch( Vertex* seedVertex, double radius, map<Vertex*,GeodEntry*>* geoDistList, uint64_t* faceBitArray, int faceNrBlocks, bool weightFuncVal ) {
	//! Estimates a geodesic patch starting from a single Vertex, with a given radius.

	// Marching front
	deque<EdgeGeodesic*> frontEdges;
	// Heap:
	make_heap( frontEdges.begin(), frontEdges.end() );
	// Get initial marching front and mark seed faces visited:
	if( !seedVertex->getGeodesicEdgesSeed( &frontEdges, geoDistList, faceBitArray, faceNrBlocks, weightFuncVal ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] initializing front failed!" << endl;
		return false;
	}

	// Start marching:
	bool retVal = estGeodesicPatch( geoDistList, &frontEdges, radius, faceBitArray, weightFuncVal ); // faceNrBlocks not used
	return retVal;
}

bool Mesh::estGeodesicPatch( Face* seedFace, double radius, map<Vertex*,GeodEntry*>* geoDistList, uint64_t* faceBitArray, int faceNrBlocks, bool weightFuncVal ) {
	//! Estimates a geodesic patch starting from a single Face, with a given radius.

	// Marching front
	deque<EdgeGeodesic*> frontEdges;
	// Heap:
	make_heap( frontEdges.begin(), frontEdges.end() );
	// Get initial marching front
	if( !seedFace->getGeodesicEdgesSeed( &frontEdges, geoDistList, faceBitArray, faceNrBlocks, weightFuncVal ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] initializing front failed!" << endl;
		return false;
	}

	// Start marching:
	bool retVal = estGeodesicPatch( geoDistList, &frontEdges, radius, faceBitArray, weightFuncVal ); // faceNrBlocks not used
	return retVal;
}

//! Estimates a geodesic patch from a given (initial) front until a certain distance (radius) is reached.
//! Remark: adding a bit array for vertices has almost no impact (~2-5%) on performance.
//!
//! Stops, when a vertex with Primitive::FLAG_MARCHING_FRONT_ABORT is reached.
//!
//! Usefull for debuging the computation of the triangles:
//! http://www.arndt-bruenner.de/mathe/scripts/Dreiecksberechnung.htm
//!
//! @returns false in case of an error.
bool Mesh::estGeodesicPatch( map<Vertex*,GeodEntry*>* geoDistList,                          //!< already estimated geodesic distances - typically pre-computed for one or more seed primitives.
                             deque<EdgeGeodesic*>*    frontEdges,                           //!< current front - typically pre-computed for one or more seed primitives.
                             double                   radius,                               //!< soft abort criteria - can be set to infinity, because estGeodesicPatch will stop, when no further faces can be visitied.
                             uint64_t*                faceBitArray,                         //!< bitarray for faces
//                             int                      faceNrBlocks,                         //!< maximum number of bit-blocks in faceBitArray
                             bool                     weightFuncVal                         //!< use function values as weights
    ) {
	// Start and stop are easy, but determining a percentage is not straight forward.
	showProgressStart( "Geodesic Patches" );
	showProgress( 0.0, "Geodesic Patches" );

	// Rock'n'roll:
	bool stopFlagFound = false;
	unsigned int badAngles[4] = { 0, 0, 0, 0 };
	while( ( frontEdges->size() > 0 ) && ( !stopFlagFound ) ) {
		// Fetch the next edge (which has to have the smalles geodesic distances.
		EdgeGeodesic* edgeToProc = frontEdges->front();
		//edgeToProc->dumpInfo();
		// Remove from Heap
		pop_heap( frontEdges->begin(), frontEdges->end(), EdgeGeodesic::shorterThan );
		frontEdges->pop_back();
		// Fetch the next Face:
		Face*   currFace = edgeToProc->getFace();
		Face*   nextFace = currFace->getNeighbourFace( edgeToProc->getEdgeIdx() );
		if( nextFace == nullptr ) {
			// ... border reached - nothing else to do => continue
			//cout << "[Mesh::estGeodesicPatc] Border reached." << endl;
			continue;
		}
		// Bit adressing:
		uint64_t  bitOffset;
		uint64_t  bitNr;
		nextFace->getIndexOffsetBit( &bitOffset, &bitNr );
		if( (static_cast<uint64_t>(1) << bitNr) & faceBitArray[bitOffset] ) {
			// ... Face already visited - nothing else to do => continue
			//cout << "[Mesh::estGeodesicPatc] Face already visited." << endl;
			continue;
		}
		// Fetch the opposing vertex and estimate the geodesic distance, and fetch the corresponding edge indices:
		Face::eEdgeNames edgeIdxAC;
		Face::eEdgeNames edgeIdxCB;
		Vertex* nextVert = nextFace->getOposingVertex( currFace, &edgeIdxAC, &edgeIdxCB );
		// Stop criteria 1 - no next vertex => try next on the front!
		if( nextVert == nullptr ) {
			// mark visited:
			//faceBitArray[bitOffset] |= ((uint64_t)1 << bitNr);
			// bail out
			continue;
		}
		// Stop criteria 2 - full stop!
		if( nextVert->getFlag( Primitive::FLAG_MARCHING_FRONT_ABORT ) ) {
			stopFlagFound = true;
			continue;
		}
		// Fetch the edge vertices:
		Vector3D vertAPos;
		Vector3D vertBPos;
		edgeToProc->getPositionA( &vertAPos );
		edgeToProc->getPositionB( &vertBPos );
		// Vertex C is the one we wan't to move on:
		Vector3D vertCPos = nextVert->getPositionVector();
		// Shift vertC using the function values as weights:
		// Edge lengths - next face:
		double vAC = ( vertCPos - vertAPos ).getLength3();
		double vCB = ( vertBPos - vertCPos ).getLength3();
		double vBA = ( vertAPos - vertBPos ).getLength3();
		// Add optional weight
		// Vertex* vertA = edgeToProc->getVertA(); // <- unused
		// Vertex* vertB = edgeToProc->getVertB(); // <- unused
		if( weightFuncVal ) {
		//if( ( weightFuncVal ) && (!badAngle) ) {
			double funcValA;
			if( !edgeToProc->getFuncValueA( &funcValA ) ) {
				cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: could not fetch function value for A!" << endl;
				return false;
			}
			double funcValB;
			if( !edgeToProc->getFuncValueB( &funcValB ) ) {
				cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: could not fetch function value for B!" << endl;
				return false;
			}
			double funcValC;
			if( !nextVert->getFuncValue( &funcValC ) ) {
				cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: could not fetch function value for C!" << endl;
				return false;
			}
			//Vector3D faceNormal = nextFace->getNormal( true );
			//vertAPos += faceNormal * ( funcValA - funcValC );
			//vertBPos += faceNormal * ( funcValB - funcValC );
			//vAC = ( vertCPos - vertAPos ).getLength3();
			//vCB = ( vertBPos - vertCPos ).getLength3();
			//vBA = ( vertAPos - vertBPos ).getLength3();
			Vertex* vertA = edgeToProc->getVertA();
			Vertex* vertB = edgeToProc->getVertB();
			Vector3D vertANormal = vertA->getNormal( true ) * funcValA;
			Vector3D vertBNormal = vertB->getNormal( true ) * funcValB;
			Vector3D vertCNormal = nextVert->getNormal( true ) * funcValC;
			vertAPos += vertANormal;
			vertBPos += vertBNormal;
			vertCPos += vertCNormal;
			vAC = ( vertCPos - vertAPos ).getLength3();
			vCB = ( vertBPos - vertCPos ).getLength3();
			vBA = ( vertAPos - vertBPos ).getLength3();

			//vCB = sqrt( pow( vCB, 2 ) + pow( (funcValB-funcValC)*1.0, 2 ) );
			//vAC = sqrt( pow( vAC, 2 ) + pow( (funcValC-funcValA)*1.0, 2 ) );
			//vBA = sqrt( pow( vBA, 2 ) + pow( (funcValB-funcValA)*1.0, 2 ) );
		}
		// Estimate angles - next face:
		double alphaJ = acos( ( pow( vAC, 2.0 ) + pow( vBA, 2.0 ) - pow( vCB, 2.0 ) ) / ( 2.0 * vBA * vAC ) );
		double betaJ  = acos( ( pow( vBA, 2.0 ) + pow( vCB, 2.0 ) - pow( vAC, 2.0 ) ) / ( 2.0 * vCB * vBA ) );
		// Fetch angles
		double geodAngleA;
		double geodAngleB;
		edgeToProc->getGeodAngleA( &geodAngleA );
		edgeToProc->getGeodAngleB( &geodAngleB );
		// Estimate angle - geodesic face:
		double geodA;
		double geodB;
		edgeToProc->getGeoDistA( &geodA );
		edgeToProc->getGeoDistB( &geodB );
		double alpha0 = ( pow( vBA, 2.0 ) + pow( geodA, 2.0 ) - pow( geodB, 2.0 ) ) / ( 2.0 * geodA * vBA );
		double beta0  = ( pow( geodB, 2.0 ) + pow( vBA, 2.0 ) - pow( geodA, 2.0 ) ) / ( 2.0 * vBA * geodB );
		bool badAngle = false;
		if( alpha0 > 1.0 ) {
			//cout << "Bad Angle: " << alpha0-1.0 << endl;
			// alternative:
			alpha0 = 0.0 + 4.0 * DBL_EPSILON; // 0° + 4x Epsilon
			//badAngle = true;
			badAngles[0]++;
		} else if( alpha0 < -1.0 ) {
			//cout << "Bad Angle: " << alpha0+1.0 << endl;
			// alternative:
			alpha0 = M_PI - 4.0 * DBL_EPSILON; // 180° - 4x Epsilon
			// badAngle = true;
			badAngles[1]++;
		} else {
			alpha0 = acos( alpha0 );
		}
		if( beta0 > 1.0 ) {
			//cout << "Bad Angle: " << beta0-1.0 << endl;
			// alternative:
			beta0 = 0.0 + 4.0 * DBL_EPSILON; // 0° + 4x Epsilon
			//badAngle = true;
			badAngles[2]++;
		} else if( beta0 < -1.0 ) {
			//mSelectedMVerts.insert( edgeToProc->getVertB() ); // for debuging
			// alternative:
			beta0 = M_PI - 4.0 * DBL_EPSILON; // 180° - 4x Epsilon
			// badAngle = true;
			badAngles[3]++;
		} else {
			beta0 = acos( beta0 );
		}
		// Estimate new geodesic distance:
		double     geodC      = 0.0;
		double     geodAngleC = _NOT_A_NUMBER_DBL_;
		Primitive* fromSeed   = nullptr;

		double geodAngleCA = 0.0;
		double geodAngleCB = 0.0;

		if( badAngle ) {
			if( ( geodA + vAC ) < ( geodB + vCB ) ) {
				geodC = geodA + vAC;
				geodAngleC = geodAngleA;
				edgeToProc->getGeoDistARef()->getFromSeed( &fromSeed );
			} else {
				geodC = geodB + vCB;
				geodAngleC = geodAngleB;
				edgeToProc->getGeoDistBRef()->getFromSeed( &fromSeed );
			}
			//cout << "[Mesh::estGeodesicPatch] geodesic dist (0): "  << geodA << " + " << vAC << " | " << geodB << " + " << vCB << endl;
		} else if( alpha0 + alphaJ >= M_PI ) {
			geodC = geodA + vAC;
			geodAngleC = geodAngleA;
			edgeToProc->getGeoDistARef()->getFromSeed( &fromSeed );
			//cout << "[Mesh::estGeodesicPatch] geodesic dist (1): "  << geodC << endl;
		} else if( beta0 + betaJ >= M_PI ) {
			geodC = geodB + vCB;
			geodAngleC = geodAngleB;
			edgeToProc->getGeoDistBRef()->getFromSeed( &fromSeed );
			//cout << "[Mesh::estGeodesicPatch] geodesic dist (2): "  << geodC << endl;
		} else {
			geodC = sqrt( pow( vAC, 2.0 ) + pow( geodA, 2.0 ) - 2.0 * vAC * geodA * cos( alpha0 + alphaJ ) );
			// geodC = sqrt( vAC*vAC + geodA*geodA - 2.0 * vAC * geodA * cos( alpha0 + alphaJ ) );
			//geodC = sqrt( pow( vCB, 2 ) + pow( geodB, 2 ) - 2.0 * vCB * geodB * cos( beta0 + betaJ ) );
			//double
			geodAngleCA = geodAngleA + acos((vAC * vAC - geodA * geodA - geodC * geodC) / (-2.0 * geodA * geodC));
			//double
			geodAngleCB = geodAngleB - acos((vCB * vCB - geodC * geodC - geodB * geodB) / (-2.0 * geodC * geodB));
			//cerr << "geodAngleA: " << geodAngleA << " " << geodAngleA*180.0/M_PI << endl;
			//cerr << "+: " << acos((vAC * vAC - geodA * geodA - geodC * geodC) / (-2.0 * geodA * geodC)) *180.0/M_PI<< endl;
			//cerr << "geodAngleB: " << geodAngleB << " " << geodAngleB *180.0/M_PI << endl;
			//cerr << "-: " << acos((vCB * vCB - geodC * geodC - geodB * geodB) / (-2.0 * geodC * geodB)) *180.0/M_PI<< endl;
			if( geodAngleCA < -M_PI ) {
				geodAngleCA = ( geodAngleCA + 2.0*M_PI );
			}
			if( geodAngleCA > +M_PI ) {
				geodAngleCA = ( geodAngleCA - 2.0*M_PI );
			}
			if( geodAngleCB < -M_PI ) {
				geodAngleCB = ( geodAngleCB + 2.0*M_PI );
			}
			if( geodAngleCB > +M_PI ) {
				geodAngleCB = ( geodAngleCB - 2.0*M_PI );
			}
			geodAngleC = ( geodAngleCA + geodAngleCB ) / 2.0;
			if( ( ( geodAngleCA * geodAngleCB )<0 ) && // ( geodAngleCA < geodAngleCB )
			    ( ( abs(geodAngleCA) > M_PI/2.0) || (abs(geodAngleCB) > M_PI/2.0)  )
			) {
//			if( ( ( geodAngleCA * geodAngleCB )<0 ) && ( ( abs(geodAngleCA) > M_PI/2.0 ) || ( abs(geodAngleCB) > M_PI/2.0 ) ) ) {
				// Debuging info regarding bad angles:
				// std::cerr << "+++geodAngleCA: " << geodAngleCA *180.0/M_PI<< std::endl;
				// std::cerr << "+++geodAngleCB: " << geodAngleCB *180.0/M_PI<< std::endl;
				if( geodAngleC < 0 ) {
					geodAngleC += M_PI;
				} else {
					geodAngleC -= M_PI;
				}
				// std::cerr << "....geodAngleC: " << geodAngleC *180.0/M_PI<< std::endl;
			} else {
				geodAngleC = ( geodAngleCA + geodAngleCB ) / 2.0;
			}
			if( weightFuncVal ) {
				double geodCAlt = sqrt( pow( vCB, 2.0 ) + pow( geodB, 2.0 ) - 2.0 * vCB * geodB * cos( beta0 + betaJ ) );
				if( geodCAlt < geodC ) {
					geodC = geodCAlt;
				}
			}
			if( geodA < geodB ) {
				edgeToProc->getGeoDistARef()->getFromSeed( &fromSeed );
			} else {
				edgeToProc->getGeoDistBRef()->getFromSeed( &fromSeed );
			}
			//cout << "sqrt( pow( " << vAC << ", 2 ) + pow( " << geodA << ", 2 ) - 2.0 * " << vAC << " * " << geodA << " * cos( " << alpha0 << " + " << alphaJ << " ) );" << endl;
			//cout << "[Mesh::estGeodesicPatch] geodesic dist (3): "  << geodC << endl;
		}
// THIS is testing/debuging code ONLY:
//		if( vertA->getIndex() == 5821 ) {
//			cerr << "geodAngleA: " << geodAngleA << " " << geodAngleA*180.0/M_PI << endl;
//			cerr << "+: " << acos((vAC * vAC - geodA * geodA - geodC * geodC) / (-2.0 * geodA * geodC)) *180.0/M_PI<< endl;
//			cerr << "geodAngleB: " << geodAngleB << " " << geodAngleB *180.0/M_PI << endl;
//			cerr << "-: " << acos((vCB * vCB - geodC * geodC - geodB * geodB) / (-2.0 * geodC * geodB)) *180.0/M_PI<< endl;
//			cerr << "-->geodAngleCA: " << geodAngleCA *180.0/M_PI<< endl;
//			cerr << "-->geodAngleCB: " << geodAngleCB *180.0/M_PI<< endl;
//			cerr << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
//		}
		if( std::isnan( geodC ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] bad geodesic distance - not a number!" << endl;
			break;
		}
		if( geodC <= 0.0 ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] bad geodesic distance: " << geodC << "!" << endl;
			break;
		}
		//cout << "[Mesh::" << __FUNCTION__ << "] geodC " << geodC << endl;
		// Check if the Vertex was already visited:
		map<Vertex*,GeodEntry*>::iterator itVertex = geoDistList->find( nextVert );
		bool    vertInList = ( itVertex != geoDistList->end() );
		GeodEntry* geodCRef( nullptr );
		if( vertInList ) {
			// already in list => update, when the new distance is shorter:
			//cout << "[Mesh::" << __FUNCTION__ << "] geodC computed: " << geodC << " in list: " << ((*itVertex).second)->geodDist << endl;
			geodCRef = (*itVertex).second;
			geodCRef->setGeodDistSmaller( geodC, geodAngleC, fromSeed );
		} else {
			// not in list => insert
			geodCRef = new GeodEntry( geodC, geodAngleC, fromSeed );
			//if( (*geodCRef) <= 0.0 ) {
			//	cout << "[Mesh::" << __FUNCTION__ << "] Warning: bad geodCRef " << *geodCRef << endl;
			//}
			geoDistList->insert( pair<Vertex*,GeodEntry*>( nextVert, geodCRef ) );
			//cout << "[Mesh::" << __FUNCTION__ << "] insert geodC: " << geodC << endl;
		}

		// Stop criteria 1 - continue, but do not add any new edges!
		if( geodC > radius ) {
			//cout << "[Mesh::estGeodesicPatch] Stop criteria reached " << geodC << endl;
			continue;
		}
		// add the new two edges to front:
		EdgeGeodesic* edgeAC = new EdgeGeodesic( nextFace, edgeIdxAC, edgeToProc->getGeoDistARef(), geodCRef );
		EdgeGeodesic* edgeCB = new EdgeGeodesic( nextFace, edgeIdxCB, geodCRef, edgeToProc->getGeoDistBRef() );
		// on the heap:
		frontEdges->push_back( edgeAC );
		push_heap( frontEdges->begin(), frontEdges->end(), EdgeGeodesic::shorterThan );
		frontEdges->push_back( edgeCB );
		push_heap( frontEdges->begin(), frontEdges->end(), EdgeGeodesic::shorterThan );
		// mark visited:
		faceBitArray[bitOffset] |= (static_cast<uint64_t>(1) << bitNr);
	}

	// Debug - make only sense, when still edges within the list:
	//cout << "---------------" << endl;
	// deque<EdgeGeodesic*>::iterator itEdgeGeo;
	//for( itEdgeGeo = frontEdges->begin(); itEdgeGeo != frontEdges->end(); itEdgeGeo++ ) {
	//	(*itEdgeGeo)->dumpInfo();
	//}
	//sort_heap( frontEdges->begin(), frontEdges->end(), EdgeGeodesic::shorterThan );
	//cout << "---------------" << endl;
	//for( itEdgeGeo = frontEdges->begin(); itEdgeGeo != frontEdges->end(); itEdgeGeo++ ) {
	//	(*itEdgeGeo)->dumpInfo();
	//}
	//cout << "---------------" << endl;

	// Remove EdgeGeodesic - in case there are any left!
	if( !stopFlagFound ) {
		frontEdges->clear();
	}

	showProgressStop( "Geodesic Patches" );

	if( badAngles[0] > 0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: " << badAngles[0] << " Bad angle(s) Alpha counted +!" << std::endl;
	}
	if( badAngles[1] > 0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: " << badAngles[1] << " Bad angle(s) Alpha counted -!" << std::endl;
	}
	if( badAngles[2] > 0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: " << badAngles[2] << " Bad angle(s) Beta counted +!" << std::endl;
	}
	if( badAngles[3] > 0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: " << badAngles[3] << " Bad angle(s) Beta counted -!" << std::endl;
	}

	return true;
}

// Estimate neighbourhood within a spherical volume ------------------------------------------------------------------------------------------------------------

//! Retrieve all faces within the given radius using a seedVertex browsing
//! the whole faceList.
//!
//! Remark: Will find all primitves within the spheres radius, even if
//! disconnected. But: Volume integrating methods will struggle with
//! these discontinuties. This can be avoided using the method
//! Mesh::fetchSphereMarching based on a marching front, which also
//! will have a better performance -- This method has O(n^2) complexity!
set<Face*> Mesh::fetchSphere( Vertex* seedVertex, float radius, bool noDebugTexture ) {
	float distA, distB, distC;
	int countOutside, countInside;
	set<Face*> facesInsideSphere;
	set<Face*> facesIntersectSphere;

	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		countOutside = 0;
		countInside  = 0;
		distA = seedVertex->estDistanceTo( currFace->getVertA() )-radius;
		distB = seedVertex->estDistanceTo( currFace->getVertB() )-radius;
		distC = seedVertex->estDistanceTo( currFace->getVertC() )-radius;
		if( distA >= 0 ) {
			countOutside++;
		} else {
			countInside++;
		}
		if( distB >= 0 ) {
			countOutside++;
		} else {
			countInside++;
		}
		if( distC >= 0 ) {
			countOutside++;
		} else {
			countInside++;
		}
		if ( countOutside == 3 ) {
			currFace->setRGB( 0.0, 0.0, 1.0 );
			continue;
		} else if ( countInside == 3 ) {
			//cout << getCoutPref() << "fetchSphere Face " << (*itFace)->getIndex() << " is inside the sphere." << endl;
			currFace->setRGB( 0.0, 1.0, 0.0 );
			facesInsideSphere.insert( currFace );
		} else if( ( countInside < 3 ) && ( countOutside < 3 ) ) {
			//cout << getCoutPref() << "fetchSphere Face " << (*itFace)->getIndex() << " intersects the sphere." << endl;
			currFace->setRGB( 1.0, 0.0, 0.0 );
			facesIntersectSphere.insert( currFace );
		}
	}

	set<Face*>::iterator itFace;
	if( !noDebugTexture ) {
		// visually debuging  the marching fronts result as texture map:
		//--------------------------------------------------------------
		//! \todo re-add facesTextureSetMonoRGB
		//facesTextureSetMonoRGB( 0.5, 0.5, 0.5 );
		for( itFace=facesInsideSphere.begin(); itFace!=facesInsideSphere.end(); itFace++ ) {
			(*itFace)->setRGB( 0.0, 1.0, 0.0 );
		}
		for( itFace=facesIntersectSphere.begin(); itFace!=facesIntersectSphere.end(); itFace++ ) {
			(*itFace)->setRGB( 1.0, 0.0, 0.0 );
		}
		stringstream sphereTexMapName;
		sphereTexMapName << "Sphere r=" << radius << ", v_i=" << seedVertex->getIndex() << " (Browse)";
		// Deprecated and removed: storeCurrentTextureMap( sphereTexMapName.str(), Primitive::IS_FACE );
		//--------------------------------------------------------------
	}

	return facesInsideSphere;
}

bool Mesh::fetchSphereMarching( Vertex*     seedVertex,    //!< point of origin of our search
                            set<Face*>* facesInSphere, //!< reference to a(n empty) pre-allocated face list
                            float       radius,        //!< maximum radius of our multi-scale spheres
                            bool        noDebugTexture //!< colors visitied faces for visual debuging. Do NOT use for processing
    ) {
	//! Retrieve all faces within the given radius using a seedVertex and the
	//! marching front algorithm.
	//!
	//! Remark: When the marching front is used, primitives within the sphere
	//! might be missed out, because they might be on disconnected mesh parts.
	//!
	//! Therefore volume integrating methods will give different results.
	//! Depending on their task this can be helpfull ... or not.
	//! Performance may be better using the marching front.

	if( radius <= 0 ) {
		cerr << getCoutPref() << "fetchSphere radius is negative or zero: " << radius << "" << endl;
		return false;
	}

	MarchingFront marchingFront( seedVertex );
	marchingFront.advanceMultiDistToCoord( radius, seedVertex->getX(), seedVertex->getY(), seedVertex->getZ(), true );
	bool retVal = marchingFront.getVisitedFaces( facesInSphere );

	if( !noDebugTexture ) {
		// visually debuging  the marching fronts result as texture map:
		//--------------------------------------------------------------
		//! \todo re-add facesTextureSetMonoRGB
		//facesTextureSetMonoRGB( 0.5, 0.5, 0.5 );
		set<Face*>::iterator itFace;
		for( itFace=facesInSphere->begin(); itFace!=facesInSphere->end(); itFace++ ) {
			(*itFace)->setRGB( 0.0, 1.0, 0.0 );
		}
		// when using marchingFront.advanceMultiDistToCoord(...) the list/set
		// of frontFaces will be empty:
		//set<Face*> frontFaces = marchingFront.getFrontFaces();
		//for( itFace=frontFaces.begin(); itFace!=frontFaces.end(); itFace++ ) {
		//	(*itFace)->setRGB( 1.0, 0.0, 0.0 );
		//}
		stringstream sphereTexMapName;
		sphereTexMapName << "Sphere r=" << radius << ", v_i=" << seedVertex->getIndex() << " (Marching)";
		// Deprecated and removed: storeCurrentTextureMap( sphereTexMapName.str(), Primitive::IS_FACE );
		//----------------------------------------------------------------------
	}

	return retVal;
}

bool Mesh::fetchSphereMarchingDualFront( Vertex*     seedVertex,    //!< point of origin of our search
                                     set<Face*>* facesInSphere, //!< reference to a(n empty) pre-allocated face list
                                     float       radius         //!< maximum radius of our multi-scale spheres
    ) {
	//! Marching front using a second front to prevent to check against all
	//! already visited facse.
	//!
	//! 2-3x faster for larger number of facesInSphere and not slower for smaller numbers.

	double seedXYZ[3];
	seedVertex->copyCoordsTo( seedXYZ );

	set<Face*> frontFaces1;
	set<Face*> frontFaces2;
	set<Face*> frontFacesToContinue;

	set<Face*>::iterator itFace;

	seedVertex->getFaces( &frontFaces2 );

	while( frontFaces2.size() > 0 ) {
		//cout << "frontFaces1/2: " << frontFaces1.size() << " / " << frontFaces2.size() << endl;
		for( itFace=frontFaces2.begin(); itFace!=frontFaces2.end(); itFace++ ) {
			int verticesInRange = (*itFace)->verticesInRange( radius, seedXYZ );
			//cout << "verticesInRange: " << verticesInRange << endl;
			if( verticesInRange == 0 ) {
				//cout << "Implement CHECK" << endl;
			} else {
				frontFacesToContinue.insert( (*itFace) );
				facesInSphere->insert( (*itFace) );
				frontFaces1.insert( (*itFace) );
			}
		}
		//cout << "frontFacesToContinue: " << frontFacesToContinue.size() << endl;
		frontFaces2.clear();
		for( itFace=frontFacesToContinue.begin(); itFace!=frontFacesToContinue.end(); itFace++ ) {
			(*itFace)->getNeighbourFacesExcluding( &frontFaces2, &frontFaces1 );
		}
		//cout << "frontFaces2: " << frontFaces2.size() << endl;
		frontFaces1.clear();
		frontFaces1.swap( frontFacesToContinue );
	}
	return true;
}

//! Fetch all Faces within a sphere plus 1-ring neighbourhood using a bit array (instead of a marching front/queue).
//! ATTENTION: Requires that the vertex indices are set properly!
//! @returns false in case of an error.
bool Mesh::fetchSphereBitArray( Vertex*        rSeedVertex,          //!< point of origin of our search
                vector<Face*>* rFacesInSphere,       //!< reference to a(n empty) pre-allocated face list
                float          rRadius,              //!< maximum radius of our multi-scale spheres
                int            rVertNrLongs,         //!< Nr of 8 byte blocks for vertices.
                uint64_t* rVertBitArrayVisited, //!< Bit array for vertices.
                int            rFaceNrLongs,         //!< Nr of 8 byte blocks for vertices.
                uint64_t* rFaceBitArrayVisited, //!< Bit array for faces.
                bool           rOrderToFuncVal       //!< When set, the method will write the access order of the faces as face function value.
    ) {
	// Get the coordinates
	double seedXYZ[3];
	rSeedVertex->copyCoordsTo( seedXYZ );

	// queue for next:
	set<Vertex*> nextArray;

	//int timeStart, timeStop; // for performance mesurement
	//timeStart = clock();

	int  bitOffset;
	int  bitNr;
	int  currIdx = rSeedVertex->getIndex();
	double seqNr = 0.0; // Only used, when rOrderToFuncVal is setS
	rSeedVertex->getIndexOffsetBit( &bitOffset, &bitNr );
	rVertBitArrayVisited[bitOffset] = 1U <<bitNr;
	nextArray.insert( rSeedVertex );
	//! While there are vertices at the front:
	while( nextArray.size() > 0 ) {
		//! -> Fetch a vertex
		Vertex* currVert = *nextArray.begin();
		//! -> Remove it from the front
		nextArray.erase( nextArray.begin() );
		//! -> Add all its adjacent faces
		currVert->advanceInSphere( seedXYZ, rRadius, rVertBitArrayVisited, &nextArray, rFaceBitArrayVisited, rOrderToFuncVal, &seqNr );
		currVert->getIndexOffsetBit( &bitOffset, &bitNr );
		//cout << "[Mesh::" << __FUNCTION__ << "] seqNr: " << seqNr << endl;
		//cout << "[Mesh::" << __FUNCTION__ << "] visited: " << currIdx << " Offset " << bitOffset << " Bytes - Bit No: " << bitNr << " Dec: " << ((uint64_t)1<<bitNr) << endl;
	}
	//timeStop = clock();
	//cout << "[Mesh] fetchSphereVolume - fetchSphereBitArray: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

	int faceInSphereCount = 0;
	for( int i=0; i<rFaceNrLongs; i++ ) {
		if( rFaceBitArrayVisited[i] == 0 ) {
			continue;
		}
		//cout << "bitArrayNext[" << i << "]: " << bitArrayNext[i] << endl;
		for( int bitIdx=0; bitIdx<64; bitIdx++ ) {
			if( (static_cast<uint64_t>(1)<<bitIdx) & rFaceBitArrayVisited[i] ) {
				faceInSphereCount++;
				currIdx = (i*8*sizeof( uint64_t ))+bitIdx;
				Face* currFace = getFacePos( currIdx );
				//vec.push_back( currVert );
				rFacesInSphere->push_back( currFace );
				//currVert->getFaces( facesInSphere );
				//cout << "offset " << i << " bit " << bitIdx << " set " << ( ((uint64_t)1<<bitIdx) & vertBitArrayVisited[i] ) << endl;
				//cout << "continue with vertex index: " << currIdx << endl;
			}
		}
		// clear all bits:
		rFaceBitArrayVisited[i] = 0;
	}
	//cout << "[Mesh::fetchSphereBitArray] faceInSphereCount: " << faceInSphereCount << endl;

	//! Clears all bits, when finished.
	for( int i=0; i<rVertNrLongs; i++ ) {
		rVertBitArrayVisited[i] = 0;
	}
	return true;
}

//! Fetch all Faces within a sphere plus 1-ring neighbourhood using a bit array (instead of a marching front/queue).
//! ATTENTION: Requires that the vertex indices are set properly!
//!
//! @returns false in case of an error.
bool Mesh::fetchSphereBitArray1R(
                Vertex*        rSeedVertex,          //!< point of origin of our search
                vector<Face*>& rFacesInSphere,       //!< reference to a(n empty) pre-allocated face list
                double         rRadius,              //!< maximum radius of our multi-scale spheres
                uint64_t  rVertNrLongs,         //!< Nr of 8 byte blocks for vertices.
                uint64_t* rVertBitArrayVisited, //!< Bit array for vertices.
                uint64_t  rFaceNrLongs,         //!< Nr of 8 byte blocks for vertices.
                uint64_t* rFaceBitArrayVisited, //!< Bit array for faces.
                bool           rOrderToFuncVal       //!< When set, the method will write the access order of the faces as face function value.
) {
	// queue for next:
	vector<Vertex*> nextArray;

	double seqNr = 0.0; // Only used, when rOrderToFuncVal is setS
	rSeedVertex->markVisited( rVertBitArrayVisited );
	nextArray.push_back( rSeedVertex );
	//! While there are vertices at the front:
	size_t arrPos = 0;
	while( arrPos < nextArray.size() ) {
		//! -> Fetch the next vertex from the front.
		Vertex* currVert = nextArray.at( arrPos );
		//! -> Add 1-ring vertex neighbours at the end of the front vector, when the vertex is within the sphere.
		double currDistance = _NOT_A_NUMBER_DBL_;
		//! \bug distance( Vertex*, Vertex* ) is broken.
		// THIS is BROKEN: currDistance = distance( currVert, rSeedVertex );
		currDistance = currVert->estDistanceTo( rSeedVertex );
		if( currDistance <= rRadius ) {
			currVert->getAdjacentVerticesExcluding( &nextArray, rVertBitArrayVisited );
		}
		//! -> Add all its adjacent faces and vertices.
		currVert->mark1RingVisited( rVertBitArrayVisited, rFaceBitArrayVisited, rOrderToFuncVal, &seqNr );
		//cout << "[Mesh::" << __FUNCTION__ << "] seqNr: " << seqNr << endl;
		arrPos++;
	}

	//! Converting the bit-array to vector<Face*> requires only approx. 5% of the computing time.
	for( uint64_t i=0; i<rFaceNrLongs; i++ ) {
		if( rFaceBitArrayVisited[i] == 0 ) {
			continue;
		}
		//cout << "bitArrayNext[" << i << "]: " << bitArrayNext[i] << endl;
		for( uint64_t bitIdx=0; bitIdx<64; bitIdx++ ) {
			if( (static_cast<uint64_t>(1)<<bitIdx) & rFaceBitArrayVisited[i] ) {
				uint64_t currIdx = (i*8*sizeof( uint64_t ))+bitIdx;
				Face* currFace = getFacePos( currIdx );
				if( currFace != nullptr ) { // can happen as the bit array can be slightly larger by up to 4*8 bits.
					rFacesInSphere.push_back( currFace );
				}
			}
		}
		//! Clear all bits for faces.
		rFaceBitArrayVisited[i] = 0;
	}

	//! Clears all bits for vertices, when finished.
	for( uint64_t i=0; i<rVertNrLongs; i++ ) {
		rVertBitArrayVisited[i] = 0;
	}

	return( true );
}

// Compute or estimate Multi-Scale Integral Invariants (MSII) --------------------------------------------------------------------------------------------------

double Mesh::fetchSphereCubeVolume25D( Vertex*     seedVertex,            //!< equals sphere center
                                      set<Face*>* facesInSphere,         //!< pre-selected list of faces
                                      double      radius,                //!< radius_max of our spheres
                                      double*     rasterArray,           //!< pre-allocated depth-map array of size cubeEdgeLengthInVoxels^2.
                                      int         cubeEdgeLengthInVoxels //!< equals xDim equals yDim equals zDim for our sparse 2.5D voxel cube as well as the sqrt( size of rasterAray )
) {
	//! Returns a 2 1/2 D voxel array, which is virtually a cube large enough to hold a sphere with the given radius.
	//!
	//! The float array returned is of size cubeEdgeLengthInVoxels x cubeEdgeLengthInVoxels.
	//!
	//! Returns the area of the total area of the faces, when successfull.
	//! Returns _NOT_A_NUMBER_ otherwise.

	if( seedVertex->isSolo() ) {
		return _NOT_A_NUMBER_DBL_;
	}

	// in theory to fetch everything in the cube:
	//---------------------------------------------
	// Radius of the outer sphere enclosing the cube, which encloses the
	// inner sphere used for voxelization:
	// float sphereRadius = radius * sqrt(3);

	// practically we need only whatever is within the sphere:
	//---------------------------------------------------------
	// anyway_ due to the MSII filter design we select slightly more than we need
	// as fetchSphereMarching* should have selected faces intersecting the max. sphere

	// we choose a marching front as it is faster and does automatically
	// filter parts of the, which would badly influence the later processing.
	// e.g. in case of a thin wall, we will get only faces on the side of
	// the surface which we are currently investigating
	//----------------------------------------------------------------------
	//cout << "[Mesh::fetchSphereCubeVolume25D] sphereRadius: " << sphereRadius << endl;
	//set<Face*> facesInSphere = fetchSphereMarching( seedVertex, sphereRadius, true );
	//cout << "[Mesh::fetchSphereCubeVolume25D] Faces: " << facesInSphere.size() << endl;

	// 1. Fetch a list of the vertices describing the faces
	//cout << "[Mesh::fetchSphereCubeVolume25D] (1) " << endl;
	int     vertexSize  = facesInSphere->size() * 3;
	double* vertexArray = getTriangleVertices( facesInSphere );

//	seedVertex->dumpInfo();
//	cout << setprecision( 10 );
//	for( int i=0; i<vertexSize; i++ ) {
//		cout << "V[" << i << "]: " << vertexArray[i*4] << " " << vertexArray[i*4+1] << " " << vertexArray[i*4+2] << endl;
//	}
	cout << "-----------------------" << endl;
	if( vertexArray == nullptr ) {
		cerr << "[Mesh::fetchSphereCubeVolume25D] getTriangleVertices failed!" << endl;
		return _NOT_A_NUMBER_DBL_;
	}
	// 2. Translate the Mesh into the origin:
	//cout << "[Mesh::fetchSphereCubeVolume25D] (2) " << endl;
	Matrix4D matAllTransformations( -(seedVertex->getX()), -(seedVertex->getY()), -(seedVertex->getZ()) );
	// 3. Find the orientation of the Mesh:
	//cout << "[Mesh::fetchSphereCubeVolume25D] (3) " << endl;
	//--- Accurate (Face area) -------------------------------------------------
	double normalAverageInSphereArr[3];
	double patchArea = averageNormalByArea( facesInSphere, normalAverageInSphereArr );
	//double angleUnsigned_alt = angle3ToZ( normalAverageInSphereArr );
	//cout << "angleUnsigned_alt: " << angleUnsigned_alt * 180.0/M_PI << endl;
	Vector3D normalAverageInSphere( normalAverageInSphereArr[0], normalAverageInSphereArr[1], normalAverageInSphereArr[2], 0.0 );
	normalAverageInSphere.normalize3();
	Vector3D rotAboutAxis = normalAverageInSphere % Vector3D( 0.0, 0.0, 1.0, 0.0 );
	double angleUnsigned = angle( normalAverageInSphere, Vector3D( 0.0, 0.0, 1.0, 0.0 ), rotAboutAxis );
//	cout << "angleUnsigned: " << angleUnsigned * 180.0/M_PI << endl;
	// Rotate only if the angle is >~ 0!
	if( fabs( angleUnsigned ) > DBL_EPSILON*10 ) {
		//--- Alternative (Face nr)  ------------------------------------------------
		//Vector3D normalAverageInSphere = averageNormal( facesInSphere );
		//float angleUnsigned = angle( normalAverageInSphere, Vector3D( 0.0, 0.0, 1.0, 0.0 ) );
		//--------------------------..........---------------------------------------
		// 4. Rotate the Mesh so that the average normal is parallel to the z-axis:
		//cout << "[Mesh::fetchSphereCubeVolume25D] (4) " << endl;
		//matAllTransformations *= rotateToZ( normalAverageInSphere );

		matAllTransformations *= Matrix4D( Vector3D( 0.0, 0.0, 0.0, 1.0 ), rotAboutAxis, -angleUnsigned );
	}
	// 5. Scale the Mesh for rasterizing
	//cout << "[Mesh::fetchSphereCubeVolume25D] (5) " << endl;
	matAllTransformations *= Matrix4D( _MATRIX4D_INIT_SCALE_, static_cast<float>(cubeEdgeLengthInVoxels-1)/(radius*2.0) );
	// 6. Shift the Mesh for rasterizing
	//cout << "[Mesh::fetchSphereCubeVolume25D] (6) " << endl;
	matAllTransformations *= Matrix4D( static_cast<float>(cubeEdgeLengthInVoxels)/2.0, -0.5+static_cast<float>(cubeEdgeLengthInVoxels)/2.0, 0.0 );
	// 7. Apply the transformatio
	//cout << "[Mesh::fetchSphereCubeVolume25D] (7) " << endl;
	matAllTransformations.applyTo( vertexArray, vertexSize );
//	for( int i=0; i<vertexSize; i++ ) {
//		cout << "V[" << i << "]: " << vertexArray[i*4] << " " << vertexArray[i*4+1] << " " << vertexArray[i*4+2] << endl;
//	}
	//matAllTransformations.dumpInfo( true, "matAllTransformations" );
	// 8. Raster the vertices
	//cout << "[Mesh::fetchSphereCubeVolume25D] (8) " << endl;
	rasterViewFromZ( vertexArray, vertexSize, rasterArray, cubeEdgeLengthInVoxels, cubeEdgeLengthInVoxels );
	delete[] vertexArray;
//	for( int i=0; i<cubeEdgeLengthInVoxels; i++ ) {
//		for( int j=0; j<cubeEdgeLengthInVoxels; j++ ) {
//			cout << rasterArray[i*cubeEdgeLengthInVoxels+j] << " ";
//		}
//		cout << endl;
//	}
	return patchArea;
}

double Mesh::fetchSphereCubeVolume25D( Vertex*        seedVertex,            //!< equals sphere center
                                      vector<Face*>* facesInSphere,         //!< pre-selected list of faces
                                      double         radius,                //!< radius_max of our spheres
                                      double*        rasterArray,           //!< pre-allocated depth-map array of size cubeEdgeLengthInVoxels^2.
                                      uint           cubeEdgeLengthInVoxels //!< equals xDim equals yDim equals zDim for our sparse 2.5D voxel cube as well as the sqrt( size of rasterAray )
) {
	//! Returns a 2 1/2 D voxel array, which is virtually a cube large enough to hold a sphere with the given radius.
	//!
	//! The float array returned is of size cubeEdgeLengthInVoxels x cubeEdgeLengthInVoxels.
	//!
	//! Returns the area of the total area of the faces, when successfull.
	//! Returns _NOT_A_NUMBER_ otherwise.

	if( seedVertex->isSolo() ) {
		return _NOT_A_NUMBER_DBL_;
	}

	// in theory to fetch everything in the cube:
	//---------------------------------------------
	// Radius of the outer sphere enclosing the cube, which encloses the
	// inner sphere used for voxelization:
	// float sphereRadius = radius * sqrt(3);

	// practically we need only whatever is within the sphere:
	//---------------------------------------------------------
	// anyway_ due to the MSII filter design we select slightly more than we need
	// as fetchSphereMarching* should have selected faces intersecting the max. sphere

	// we choose a marching front as it is faster and does automatically
	// filter parts of the, which would badly influence the later processing.
	// e.g. in case of a thin wall, we will get only faces on the side of
	// the surface which we are currently investigating
	//----------------------------------------------------------------------
	//cout << "[Mesh::fetchSphereCubeVolume25D] sphereRadius: " << sphereRadius << endl;
	//set<Face*> facesInSphere = fetchSphereMarching( seedVertex, sphereRadius, true );
	//cout << "[Mesh::fetchSphereCubeVolume25D] Faces: " << facesInSphere.size() << endl;
//int timeStart = clock(); // for performance mesurement
	// 1. Fetch a list of the vertices describing the faces
	//cout << "[Mesh::fetchSphereCubeVolume25D] (1) " << endl;
	uint64_t vertexSize  = facesInSphere->size() * 3;
	double* vertexArray = getTriangleVertices( facesInSphere );
	if( vertexArray == nullptr ) {
		cerr << "[Mesh::fetchSphereCubeVolume25D] getTriangleVertices failed!" << endl;
		return _NOT_A_NUMBER_DBL_;
	}
//cout << "[Mesh] fetchSphereCubeVolume25D (1): " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
//timeStart = clock();
	// 2. Translate the Mesh into the origin:
	//cout << "[Mesh::fetchSphereCubeVolume25D] (2) " << endl;
	vector<double> transVec { -(seedVertex->getX()), -(seedVertex->getY()), -(seedVertex->getZ()) };
	Matrix4D matAllTransformations( Matrix4D::INIT_TRANSLATE, &transVec );
//matAllTransformations.dumpInfo( true, "matAllTransformations" );
	// 3. Find the orientation of the Mesh:
	//cout << "[Mesh::fetchSphereCubeVolume25D] (3) " << endl;

	//--- Accurate (Face area) -------------------------------------------------
	double normalAverageInSphereArr[3];
	double patchArea = averageNormalByArea( facesInSphere, normalAverageInSphereArr );
	//double angleUnsigned_alt = angle3ToZ( normalAverageInSphereArr );
	//cout << "angleUnsigned_alt: " << angleUnsigned_alt * 180.0/M_PI << endl;
	Vector3D normalAverageInSphere( normalAverageInSphereArr[0], normalAverageInSphereArr[1], normalAverageInSphereArr[2], 0.0 );
	normalAverageInSphere.normalize3();
	Vector3D rotAboutAxis = normalAverageInSphere % Vector3D( 0.0, 0.0, 1.0, 0.0 );
	double angleUnsigned = angle( normalAverageInSphere, Vector3D( 0.0, 0.0, 1.0, 0.0 ), rotAboutAxis );
//	cout << "angleUnsigned: " << angleUnsigned * 180.0/M_PI << endl;
	// Rotate only if the angle is >~ 0!
	if( fabs( angleUnsigned ) > DBL_EPSILON*10 ) {
		//--- Alternative (Face nr)  ------------------------------------------------
		//Vector3D normalAverageInSphere = averageNormal( facesInSphere );
		//float angleUnsigned = angle( normalAverageInSphere, Vector3D( 0.0, 0.0, 1.0, 0.0 ) );
		//--------------------------..........---------------------------------------
		// 4. Rotate the Mesh so that the average normal is parallel to the z-axis:
		//cout << "[Mesh::fetchSphereCubeVolume25D] (4) " << endl;
		//matAllTransformations *= rotateToZ( normalAverageInSphere );

		matAllTransformations *= Matrix4D( Vector3D( 0.0, 0.0, 0.0, 1.0 ), rotAboutAxis, -angleUnsigned );
	}
/*

	//--- Accurate (Face area) -------------------------------------------------
	double normalAverageInSphereArr[3];
	double patchArea = averageNormalByArea( facesInSphere, normalAverageInSphereArr );
	double angleUnsigned = angle3ToZ( normalAverageInSphereArr );
//cout << "angleUnsigned: " << angleUnsigned << " " << angleUnsigned*180.0/M_PI << endl;
//cout << "normal: " << normalAverageInSphereArr[0] << " " << normalAverageInSphereArr[1] << " " << normalAverageInSphereArr[2] << " " << endl;
	Vector3D normalAverageInSphere( normalAverageInSphereArr[0], normalAverageInSphereArr[1], normalAverageInSphereArr[2], 0.0 );
//cout << "[Mesh] fetchSphereCubeVolume25D (2,3): " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
//timeStart = clock();
	//--- Alternative (Face nr)  ------------------------------------------------
	//Vector3D normalAverageInSphere = averageNormal( facesInSphere );
	//float angleUnsigned = angle( normalAverageInSphere, Vector3D( 0.0, 0.0, 1.0, 0.0 ) );
	//--------------------------..........---------------------------------------
	// 4. Rotate the Mesh so that the average normal is parallel to the z-axis:
	//cout << "[Mesh::fetchSphereCubeVolume25D] (4) " << endl;
	//matAllTransformations *= rotateToZ( normalAverageInSphere );
	Vector3D rotAboutAxis = normalAverageInSphere % Vector3D( 0.0, 0.0, 1.0, 0.0 );
	matAllTransformations *= Matrix4D( Vector3D( 0.0, 0.0, 0.0, 1.0 ), rotAboutAxis, angleUnsigned );
//Matrix4D( Vector3D( 0.0, 0.0, 0.0, 1.0 ), rotAboutAxis, -angleUnsigned ).dumpInfo( true, "trans1" );
	*/
	// 5. Scale the Mesh for rasterizing
	//cout << "[Mesh::fetchSphereCubeVolume25D] (5) " << endl;
	matAllTransformations *= Matrix4D( _MATRIX4D_INIT_SCALE_,
	                                   static_cast<double>(cubeEdgeLengthInVoxels-1)/(radius*2.0) );
//Matrix4D( _MATRIX4D_INIT_SCALE_, (float)(cubeEdgeLengthInVoxels-1)/(radius*2.0) ).dumpInfo( true, "trans2" );
	// 6. Shift the Mesh for rasterizing
	//cout << "[Mesh::fetchSphereCubeVolume25D] (6) " << endl;
	vector<double> transVecMidPoint { static_cast<double>(cubeEdgeLengthInVoxels)/2.0,
		                -0.5+static_cast<double>(cubeEdgeLengthInVoxels)/2.0, 0.0 };
	matAllTransformations *= Matrix4D( Matrix4D::INIT_TRANSLATE, &transVecMidPoint );
	// old - deprecated: matAllTransformations *= Matrix4D( (float)(cubeEdgeLengthInVoxels)/2.0, -0.5+(float)(cubeEdgeLengthInVoxels)/2.0, 0.0 );
//Matrix4D( (float)(cubeEdgeLengthInVoxels)/2.0, -0.5+(float)(cubeEdgeLengthInVoxels)/2.0, 0.0 ).dumpInfo( true, "trans3" );
//cout << "[Mesh] fetchSphereCubeVolume25D (4-6): " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
//timeStart = clock();
	// 7. Apply the transformatio
	//cout << "[Mesh::fetchSphereCubeVolume25D] (7) " << endl;
	//int i=vertexSize-1;
	//cout << vertexArray[i*4+0] << " " << vertexArray[i*4+1] << " " << vertexArray[i*4+2] << " " << vertexArray[i*4+3] << endl;
//	for( int i=0; i<vertexSize; i++ ) {
//		cout << vertexArray[i*4+0] << " " << vertexArray[i*4+1] << " " << vertexArray[i*4+2] << " " << vertexArray[i*4+3] << endl;
//	}
	matAllTransformations.applyTo( vertexArray, vertexSize );
	//cout << vertexArray[i*4+0] << " " << vertexArray[i*4+1] << " " << vertexArray[i*4+2] << " " << vertexArray[i*4+3] << endl;
//	cout << "pts = [ " << endl;
//	for( int i=0; i<vertexSize; i++ ) {
//		cout << vertexArray[i*4+0] << ", " << vertexArray[i*4+1] << ", " << vertexArray[i*4+2] << ";" << endl; // " << vertexArray[i*4+3] << endl;
//	}
//	cout << "];" << endl;
//	matAllTransformations.dumpInfo( true, "matAllTransformations" );
//cout << "[Mesh] fetchSphereCubeVolume25D (7): " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
//timeStart = clock();
	// 8. Raster the vertices
	//cout << "[Mesh::fetchSphereCubeVolume25D] (8) " << endl;
	rasterViewFromZ( vertexArray, vertexSize, rasterArray, cubeEdgeLengthInVoxels, cubeEdgeLengthInVoxels );
//cout << "[Mesh] fetchSphereCubeVolume25D (8): " << (float)( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
//timeStart = clock();
	delete[] vertexArray;
	return patchArea;
}

//! Compute the Surface Integral Invariant Type A (abbrivated: II.SA)
bool Mesh::fetchSphereArea( Vertex*        rSeedPosition,  //!< Position of the sphere center AKA seed position.
                            vector<Face*>* rFacesInSphere, //!< List of faces, which should contain all faces within the largest sphere. Including faces intersecting the sphere.
                            unsigned int   rRadiiNr,       //!< Number of radii of the concentric spheres.
                            double*        rRadii,         //!< Array of length rRadiiNr holding the radii.
                            double*        rAreas          //!< Pointer to a pre-allocated array of length rRadiiNr for the computed areas.
) {
	// Init areas with zero
	for( unsigned int i=0; i<rRadiiNr; i++ ) {
		rAreas[i] = 0.0;
	}
	// Parse all faces
	for(auto& face : *rFacesInSphere)
	{
		face->surfaceintegralinvariant( rRadiiNr, rRadii, rAreas, rSeedPosition);
	}

	// Normalize areas by circle area to ]0.0,+inf) - 1.0 means the size of circle disc of a great circle, which can be flat.
	for( unsigned int i=0; i<rRadiiNr; i++ ) {
		rAreas[i] /= M_PI * rRadii[i] * rRadii[i];
		//cout << " " << rAreas[i];
	}
	return true;
}

//! Estimate the Surface Integral Invariant Type A (abbrivated: II.SA)
bool Mesh::fetchSphereAreaEst( const Vertex*        rSeedPosition,  //!< Position of the sphere center AKA seed position.
                               std::vector<Face*>* rFacesInSphere, //!< List of faces, which should contain all faces within the largest sphere. Including faces intersecting the sphere.
                               const unsigned int   rRadiiNr,       //!< Number of radii of the concentric spheres.
                               const double*        rRadii,         //!< Array of length rRadiiNr holding the radii.
                               double*        rAreas          //!< Pointer to a pre-allocated array of length rRadiiNr for the computed areas.
) {
	double somePos[3];
	rSeedPosition->copyXYZTo( somePos );
	// Init areas with zero
	for( unsigned int i=0; i<rRadiiNr; i++ ) {
		rAreas[i] = 0.0;
	}
	// Parse all faces
	vector<Face*>::iterator itFace;
	for( itFace=rFacesInSphere->begin(); itFace!=rFacesInSphere->end(); itFace++ ) {
		double dist;
		//(*itFace)->getDistanceFromCenterOfGravityTo( somePos, &dist );
		(*itFace)->getMinDistTo( somePos, &dist );
		for( unsigned int i=0; i<rRadiiNr; i++ ) {
			if( dist > rRadii[i] ) {
				// We assume the face as outside the sphere.
				continue;
			}
			// When assumed inside the sphere: add the whole face area.
			rAreas[i] += (*itFace)->getAreaNormal();
		}
	}
	// Normalize areas by circle area to ]0.0,+inf) - 1.0 means the size of circle disc of a great circle, which can be flat.
	for( unsigned int i=0; i<rRadiiNr; i++ ) {
		rAreas[i] /= M_PI * rRadii[i] * rRadii[i];
	}
	return true;
}

double* Mesh::getTriangleVertices( set<Face*>* someFaceList ) {
	//! Returns an array of homogenous vectors of the Vertices describing the Faces in the list.
	//! The returned array will be of size length(someFaceList) x 3 x 4
	//!
	//! Typically called by Mesh::fetchSphereCubeVolume25D()
	int     vertexSize  = someFaceList->size()*3;
	double* vertexArray = new double[vertexSize*4];
	int     vertexNr    = 0;
	set<Face*>::iterator itFace;
	for( itFace=someFaceList->begin(); itFace!=someFaceList->end(); itFace++ ) {
		(*itFace)->getVertA()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
		(*itFace)->getVertB()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
		(*itFace)->getVertC()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
	}
	return vertexArray;
}

double* Mesh::getTriangleVertices( vector<Face*>* someFaceList ) {
	//! Returns an array of homogenous vectors of the Vertices describing the Faces in the list.
	//! The returned array will be of size length(someFaceList) x 3 x 4
	//!
	//! Typically called by Mesh::fetchSphereCubeVolume25D()
	int     vertexSize  = someFaceList->size()*3;
	double* vertexArray = new double[vertexSize*4];
	int     vertexNr    = 0;
	vector<Face*>::iterator itFace;
	for( itFace=someFaceList->begin(); itFace!=someFaceList->end(); itFace++ ) {
		(*itFace)->getVertA()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
		(*itFace)->getVertB()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
		(*itFace)->getVertC()->copyCoordsTo( &vertexArray[vertexNr*4] );
		vertexArray[vertexNr*4+3] = 1.0;
		vertexNr++;
	}
	return vertexArray;
}

set<Face*> Mesh::getFacesIntersectSphere1( Vector3D positionVec, float radius ) {
	//! Returns the Faces of a Mesh intersected by a given Sphere.
	//! Using Edge::intersectsSphere1

	set<Face*> facesIntersected;

	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		if( currFace->intersectsSphere1( positionVec, radius ) ) {
			facesIntersected.insert( currFace );
			//cout << "[Mesh::getFacesIntersectSphere] Face " << (*itFace)->getIndex() << " intersected." << endl;
		} else {
			// do nothing
			//cout << "[Mesh::getFacesIntersectSphere] Face " << (*itFace)->getIndex() << " NOT intersected." << endl;
		}
	}

	return facesIntersected;
}

set<Face*> Mesh::getFacesIntersectSphere2( Vector3D positionVec, float radius ) {
	//! Returns the Faces of a Mesh intersected by a given Sphere.
	//! Using Edge::intersectsSphere2

	set<Face*> facesIntersected;

	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		if( currFace->intersectsSphere2( positionVec, radius ) ) {
			facesIntersected.insert( currFace );
			//cout << "[Mesh::getFacesIntersectSphere] Face " << (*itFace)->getIndex() << " intersected." << endl;
		} else {
			// do nothing
			//cout << "[Mesh::getFacesIntersectSphere] Face " << (*itFace)->getIndex() << " NOT intersected." << endl;
		}
	}

	return facesIntersected;
}

vector<Vector3D> Mesh::getPointsIntersectSphere( Vector3D positionVec, float radius ) {
	//! Returns the points of intersection between the Faces of the Mesh and a sphere.

	vector<Vector3D> pointsOfIntersection;
	int nrSegments = 0;

	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		nrSegments += currFace->getSphereIntersections( positionVec, radius, &pointsOfIntersection  );
	}
	cout << "[Mesh] getPointsIntersectSphere " << nrSegments << " added." << endl;

	return pointsOfIntersection;
}

// mesh transformation ---------------------------------------------------------

Matrix4D Mesh::rotateToZ( Vector3D directionVec ) {
	//! Provides a Matrix4D, which rotates the whole Mesh so that the given
	//! direction vector is parallel to the z-axis.

	// First we need the angle of rotation within the xy-plane:
	float rotateAngleInXY = directionVec.getAngleToXinXY();
	// Than the angle to the z-axis:
	float rotateAngleZ = directionVec.getAngleToZ();
	//if( debug ) {
	//	directionVec.dumpInfo();
	//	cout << "[Mesh] rotateToZ: rotateAngleInXY = " << 180.0*rotateAngleInXY/M_PI << "° rotateAngleZ = " << 180.0*rotateAngleZ/M_PI << "°" << endl;
	//}

	if( isnan( rotateAngleInXY ) ) {
		cerr << "[Mesh::rotateToZ] rotateAngleInXY isNan!" << endl;
		directionVec.dumpInfo();
	}

	Matrix4D matAllTransformations( _MATRIX4D_INIT_IDENTITY_, 1.0 );
	matAllTransformations *= Matrix4D ( _MATRIX4D_INIT_ROTATION_Z_, -rotateAngleInXY );
	matAllTransformations *= Matrix4D ( _MATRIX4D_INIT_ROTATION_Y_, -rotateAngleZ );
	matAllTransformations *= Matrix4D ( _MATRIX4D_INIT_ROTATION_Z_, +rotateAngleInXY );
	//matAllTransformations.dumpInfo( true, "matAllTransformations" );

	return matAllTransformations;
}

//! Add a datum sphere.
bool Mesh::datumAddSphere( Vector3D rPos, double rRadius, unsigned char rRed, unsigned char rGreen, unsigned char rBlue ) {
	Sphere* someSphere = new Sphere( rPos, rRadius, rRed, rGreen, rBlue );
	mDatumSpheres.push_back( someSphere );
	return true;
}

//! Removes everything from the scene, except the mesh
void Mesh::removeAllDatumObjects()
{
	for(auto& sphere : mDatumSpheres)
	{
		delete sphere;
	}
	mDatumSpheres.clear();

	for(auto& box : mDatumBoxes)
	{
		delete box;
	}
	mDatumBoxes.clear();

	mSelectedPositions.clear();
}

bool Mesh::hasDatumObjects()
{
	return !(mDatumSpheres.empty() && mDatumBoxes.empty() && mSelectedPositions.empty());
}

//! Apply a given transformation matrix Matrix4D to all Vertices (Mesh::mVertices).
//!
//! As this function uses the coordinate-vertex-array instead the Vertex
//! objects, the array has to be set. If not the method can not be executed
//! and false is returned.
//!
//! This method also re-estimates the normals of the faces and the bounding box (for performance reasons).
//!
//! Transformation is additionally applied to the:
//!   .) Axis (used by cone and cylinder)
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::applyTransformationToWholeMesh( Matrix4D rTrans, bool rResetNormals, bool rSaveTransMat ) {
	// Sanity checks:
	if( getVertexNr() == 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: no vertices found!" << endl;
		return false;
	}

	// Apply to ALL vertices:
	set<Vertex*> allVertices;
	for( auto const& currVertex: mVertices ) {
		allVertices.insert( currVertex );
	}

	//!\todo Apply to all cone paramters, the sphere and the mesh plane.
    if(applyTransformation(rTrans, &allVertices, rResetNormals, rSaveTransMat))
	{
		//! .) Apply to (cone/cylinder) axis
		mConeAxisPoints[0] *= rTrans;
		mConeAxisPoints[1] *= rTrans;

		//! .) Apply transformation to the mesh-plane
		applyTransfromToPlane(rTrans);

		//! .) Apply to selectedPositions
		for(auto& position : mSelectedPositions)
		{
			std::cout << std::get<0>(position) << std::endl;
			std::get<0>(position) *= rTrans;
			std::cout << std::get<0>(position) << std::endl;
		}

		//! .) Apply to spheres
		for(auto& sphere : mDatumSpheres)
		{
			sphere->applyTransfrom(&rTrans);
		}

		//! .) Apply to boxes
		for(auto& box : mDatumBoxes)
		{
			box->applyTransfrom(&rTrans);
		}

		return true;
	}
	return false;
}

//! Translate the mesh to a specified position.
//! See Mesh::eTranslate
//!
//! E.g. use the vertex with the lowest y-coordinate on the xz-plane using Mesh::mMinY
//! with optional centering of the bounding box along the y-axis.
//!
//! Optional: multiplys the transformation matrix, when a valid pointer to a matrix is given.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::applyTransformationPlacement(
                eTranslate   rType,
                Matrix4D*    rAppliedMat
) {
	// If mMinZ is invalid or zero there is nothing to do.
	if( !isnormal( mMinZ ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Bounding Box min( z ) is not normal: " <<  mMinZ << "!" << endl;
		return( false );
	}

	vector<double> transVec;
	switch( rType ) {
		case TRANSLATE_PLACE_ON_XZ_ONLY: // Just put the lowest vertex on the xz-plane.
			transVec = { 0.0, -mMinY, 0.0 };
			break;
		case TRANSLATE_PLACE_ON_XZ_CENTER: // Put the lowest vertex on the xz-plane and center bounding box.
			transVec = { -mMinX-( mMaxX-mMinX )/2.0, -mMinY, -mMinZ-( mMaxZ-mMinZ )/2.0 };
			break;
		default:
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Unknown translation id: " <<  rType << "!" << endl;
			return( false );
	}

	bool retVal = true;
	Matrix4D transMat( Matrix4D::INIT_TRANSLATE, &transVec );

	bool resetNormals = false; // Normals stay unchanged for translation.
	retVal &= applyTransformationToWholeMesh( transMat, resetNormals );

	// Multiply the transformation matrix.
	if( rAppliedMat != nullptr ) {
		(*rAppliedMat) *= transMat;
	}

	return( retVal );
}

//! Use the axis for a base change setting the axis as y-axis.
//!
//! Optional: multiplys the transformation matrix, when a valid pointer to a matrix is given.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::applyTransformationAxisToY( Matrix4D* rAppliedMat ) {
	Vector3D axisBottom; // Schaft | Bottom
	Vector3D axisOrient; // Spitze - Schaft
	if( !getConeAxisPosDir( &axisBottom, &axisOrient ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Invalid axis!" << endl;
		//cerr << "[Mesh::" << __FUNCTION__ << "]        Tip:    " << mConeAxisPoints[0] << endl;
		//cerr << "[Mesh::" << __FUNCTION__ << "]        Bottom: " << mConeAxisPoints[1] << endl;
		return( false );
	}

	bool retVal = true;

	Matrix4D baseTrans;
	baseTrans.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Y, &axisBottom, &axisOrient );

	bool resetNormals = true; // Normals will change due to rotation.
	retVal &= applyTransformationToWholeMesh( baseTrans, resetNormals );

	// Multiply the transformation matrix.
	if( rAppliedMat != nullptr ) {
		(*rAppliedMat) *= baseTrans;
	}

	return( retVal );
}

//! Use the given view matrix to set the default view.
//!
//! The given matrix will be manipulated i.e. transformations will be multiplied
//! for storage of the transformation.
//!
//! See MeshWidget::currentViewToDefault
//!
//! @returns false in case of an error or user cancel. True otherwise.
bool Mesh::applyTransformationDefaultViewMatrix( Matrix4D* rViewMatrix ) {
	// Sanity check
	if( rViewMatrix == nullptr ) {
		return( false );
	}

	bool userChoiceUseAxisDirection = false; // Default: do not use the axis.
	bool userChoiceCenterAxis       = false; // Default: do not use the axis.
	if( getConeAxisDefined() ) { // If an axis is defined we ask the user to use it:
		if( !showQuestion( &userChoiceUseAxisDirection, "Axis for orientation (1/2)",
		                   "Use the axis for an upright orientation of the new default view?" ) ) {
			return( false ); // User cancel
		}
		// Ask if we should use the axis to center/align the mesh
		if( userChoiceUseAxisDirection ) {
			if( !showQuestion( &userChoiceCenterAxis, "Axis for orientation (2/2)",
			                   "Centering the mesh:\n\nYES: Center around the axis\n\n NO: Use the bounding box center." ) ) {
				return( false ); // User cancel
			}
		}
	}

	bool retVal = true;

	retVal &= this->applyTransformationToWholeMesh( (*rViewMatrix) );
	if( userChoiceUseAxisDirection ) {
		retVal &= this->applyTransformationAxisToY( rViewMatrix );
	}
	eTranslate translationType = TRANSLATE_PLACE_ON_XZ_CENTER;
	if( userChoiceCenterAxis ) {
		translationType = TRANSLATE_PLACE_ON_XZ_ONLY;
	}
	retVal &= this->applyTransformationPlacement( translationType, rViewMatrix );

	return( retVal );
}

//! Apply a given transformation matrix Matrix4D all given Vertices.
//!
//! As this function uses the coordinate-vertex-array instead the Vertex
//! objects, the array has to be set. If not the method can not be executed
//! and false is returned.
//!
//! This method also re-estimates the normals of the faces and the bounding box (for performance reasons).
bool Mesh::applyTransformation( Matrix4D rTrans, set<Vertex*>* rSomeVerts, bool rResetNormals, bool rSaveTransMat ) {
	//cout << "[Mesh::" << __FUNCTION__ << "] Start" << endl;
	//rTrans.dumpInfo();
	// Sanity checks:
	if( rSomeVerts->size() == 0 ) {
		cout << "[Mesh::" << __FUNCTION__ << "] No vertices given!" << endl;
		return true;
	}

	showProgressStart("Apply Transformation");
	showProgress(0.0, "Apply Transformation");

	//! .) Apply transformation matrix for each of the given vertices.
	unsigned int errCtr = 0;
	double percentDone = 0.0;
	const unsigned int progressStep = rSomeVerts->size() / 10;
	unsigned int count = 0;
	float timeStart = clock();
	for( auto const& currVertex: (*rSomeVerts) ) {
		if( !(currVertex->applyTransfrom( &rTrans )) ) {
			errCtr++;
		}

		++count;
		if(count >= progressStep)
		{
			count = 0;
			percentDone += 0.1;
			showProgress(percentDone, "Apply Transformation");
		}
	}

	showProgress(1.0, "Apply Transformation");

	cout << "[Mesh::" << __FUNCTION__ << "] time: " << ( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	if( errCtr > 0 ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: applyTransfrom failed " << errCtr << " times!" << endl;
	}

	//! .) Recompute the bounding box
	estBoundingBox();
	cout << "[Mesh::" << __FUNCTION__ << "] Bounding box is now: " << mMaxX-mMinX << " x "  << mMaxY-mMinY << " x "  << mMaxZ-mMinZ << " mm (unit assumed)." << endl;

	//! .) Reset face normals
	if( rResetNormals ) {
		double meshArea;
		resetFaceNormals( &meshArea );
		cout << "[Mesh::" << __FUNCTION__ << "] Area of the mesh is now: " << meshArea << " mm² (unit assumed)." << endl;
	}

	//! .) Can also affect the polylines - reset them too:
	polyLinesChanged();

	//! .) Write the transformation to the side-car file, because HiWis tend to forget this.
    if (rSaveTransMat){
        std::filesystem::path transMatFName = getFileLocation().wstring() + getBaseName().wstring() + L"_transmat.txt";
        ofstream transMatFile;
        transMatFile.open( transMatFName, ios::app );
        if( transMatFile.is_open() ) {
            // Fetch time and date as string
            std::time_t t = std::time(nullptr);
            char mbstr[100];
            std::strftime( mbstr, sizeof( mbstr ), "%A %c", std::localtime( &t ) );
            // Fetch matrix as text
            string matStr;
            rTrans.getTextMatrix( &matStr );
            // Write matrix
            transMatFile << "#------------------------------------------------------" << endl;
            transMatFile << "# Transformation applied to " << getBaseName() << endl;
            transMatFile << "#......................................................" << endl;
            transMatFile << matStr;
            transMatFile << "#......................................................" << endl;
            transMatFile << "# on " << mbstr << endl;
            transMatFile << "#------------------------------------------------------" << endl;
            transMatFile << endl;
            transMatFile.close();
            wcout << L"[Mesh::" << __FUNCTION__ << "] Transformation matrix written to: " << transMatFName << endl;
        } else {
            wcerr << L"[Mesh::" << __FUNCTION__ << "] ERROR: writing transformation matrix to: " << transMatFName << "!" << endl;
        }
    }
	showProgressStop("Apply Transformation");
	return ( errCtr == 0 );
}

//! Apply melting with sqrt(r^2-x^2-y^2) -- see also Vertex::applyMeltingSphere
bool Mesh::applyMeltingSphere( double rRadius, double rRel ) {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;

	//set<Vertex*> vertsToRemove;
	vector<Vertex*>::iterator itVertex;
	for( itVertex=mVertices.begin(); itVertex!=mVertices.end(); itVertex++ ) {
		//! Apply to each vertex and add those outside the radius to the selection.
		if( !(*itVertex)->applyMeltingSphere( rRadius, rRel ) ) {
			mSelectedMVerts.insert( (*itVertex) );
		}
		// Apply to each vertex and store those outside the radius for removal.
		//if( !(*itVertex)->applyMeltingSphere( rRadius, rRel ) ) {
		//	vertsToRemove.insert( (*itVertex) );
		//}
	}

	// Removal of vertices outside the sphere.
	//Mesh::removeVertices( &vertsToRemove );
	selectedMVertsChanged();
	return true;
}

//! Inverts the orientation of faces.
//!    If none are selected, than all faces are inverted.
//!    Otherwise SelMFaces are inverted.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::applyInvertOrientationFaces() {
	bool retVal = false;
	if( mFacesSelected.size() == 0 ) {
		bool invertAll = false;
		if( !showQuestion( &invertAll, "Invert all faces", "No faces (SelMFaces) were selected.<br /><br />"
		                                                   "Do you want to invert ALL faces?" ) ) {
			// User canceled.
			return( false );
		}
		if( !invertAll ) {
			// User answered no.
			return( false );
		}
		retVal = applyInvertOrientationFaces( mFaces );
	} else {
		std::vector<Face*> selectedFaces;
		for( auto const& currFace: mFacesSelected ) {
			selectedFaces.push_back( currFace );
		}
		retVal = applyInvertOrientationFaces( selectedFaces );
	}
	return( retVal );
}

//! Inverts the orientation of the given faces.
//! Vertex normals will be recomputed.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::applyInvertOrientationFaces( std::vector<Face*> rFacesToInvert ) {
	bool retVal = true;
	for( auto const& currFace: rFacesToInvert ) {
		retVal &= currFace->applyReOrient();
	}
	retVal &= resetVertexNormals();
	retVal &= changedMesh();
    return( retVal );
}


//! To be called when the normals of the vertices changed
//! e.g. to refresh the Open GL rendering
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::normalsVerticesChanged() {
	// Do nothing - stub for higher level methods i.e. OpenGL.
	return( true );
}

//! Resets all face normals and calculates them based on the current positions
//! of the face vertices.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::resetFaceNormals(
    double* rAreaTotal   //!< Optional pointer to double to retrieve the total area of the mesh.
) {
	bool retVal = true;
	double meshArea = 0.0;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		retVal &= currFace->clearFlag( FLAG_NORMAL_SET );
		meshArea += currFace->getAreaNormal();
	}
	if( rAreaTotal != nullptr ) {
		(*rAreaTotal) = meshArea;
	}
	cout << "[Mesh::" << __FUNCTION__ << "] New mesh area: " << meshArea << " mm2 (unit assumed)." << endl;
	return( retVal );
}

//! Resets vertex normals using the faces of the 1-ring.
//! See VertexOfFace::estNormalAvgAdjacentFaces
//!
//! @returns true, when all normals were (re)set. False otherwise.
bool Mesh::resetVertexNormals() {
	bool retVal(true);
	showProgressStart( __FUNCTION__ );
	uint64_t errorCtr = 0;
	uint64_t nrOfVertices = getVertexNr();
	for( uint64_t vertexIdx = 0; vertexIdx < nrOfVertices; vertexIdx++ ) {
		Vertex* currVertex = getVertexPos( vertexIdx );
		if( !currVertex->estNormalAvgAdjacentFaces() ) {
			errorCtr++;
		}
		showProgress( static_cast<double>(vertexIdx+1)/static_cast<double>(nrOfVertices), __FUNCTION__ );
	}
	showProgressStop( __FUNCTION__ );

	if( errorCtr > 0 ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: estNormalAvgAdjacentFaces failed " << errorCtr << " times!" << std::endl;
		std::cerr << "[Mesh::" << __FUNCTION__ << "]        Faces having a zero area are a possible reason!" << std::endl;
		return( false );
	}
	retVal |= normalsVerticesChanged();
	std::cout << "[Mesh::" << __FUNCTION__ << "] done." << std::endl;
	return( retVal );
}

//! Compute vertex normals using the normals of faces within a given sphere radius.
//!
//! @returns true, when all normals were (re)set. False otherwise.
bool Mesh::normalsVerticesComputeSphere(
                double rRadius   //!< Radius of the sphere used to add face normals.
) {
	// Sanity check
	if( !isnormal( rRadius ) || ( rRadius <= 0.0 ) ) {
		std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Invalid radius of " << rRadius << "given!" << std::endl;
		return( false );
	}

	bool retVal(true);
	showProgressStart( __FUNCTION__ );

	// Prepare normals
	std::vector<MeshIO::grVector3ID> patchNormalsToAssign;
	patchNormalsToAssign.resize( this->getVertexNr() );

	// Determine number of threads using CPU cores minus one.
	const unsigned int availableConcurrentThreads =  std::thread::hardware_concurrency() - 1;
	std::cout << "[GigaMesh::" << __FUNCTION__ << "] Computing vertex normals using "
	          << availableConcurrentThreads << " threads" << std::endl;

	sMeshDataStruct* setMeshData = new sMeshDataStruct[availableConcurrentThreads];
	for( size_t t = 0; t < availableConcurrentThreads; t++ )
	{
		setMeshData[t].threadID               = t;
		setMeshData[t].meshToAnalyze          = this;
		setMeshData[t].radius                 = rRadius;
		setMeshData[t].xyzDim                 = 0;
		setMeshData[t].multiscaleRadiiSize    = 0;
		setMeshData[t].multiscaleRadii        = nullptr;
		setMeshData[t].sparseFilters          = nullptr;
		setMeshData[t].mPatchNormal           = &patchNormalsToAssign;
		setMeshData[t].descriptVolume         = nullptr;
		setMeshData[t].descriptSurface        = nullptr;
	}

	// Use MSII function for parallel processing and normal estimation
	compFeatureVectorsMain( setMeshData, availableConcurrentThreads );

	// Assigning normals
	if( !this->assignImportedNormalsToVertices( patchNormalsToAssign ) ) {
		showWarning( "Faulty normals",
		             "Not all normals could be computed and assigned.<br /><br />"
		             "A possible reason is the existance of very small faces." );
		retVal = false;
	}

	// Update OpenGL conect in GUI
	retVal |= normalsVerticesChanged();

	// Cleanup
	delete[] setMeshData;
	showProgressStop( __FUNCTION__ );

	return( retVal );
}


//! \todo Move the following sets and numbers to appropriate places. It seems that at least some don't need to be global.
set <Face*> originalFaces;
set <Vertex*> originalVertices;
vector <PolyLine*> originalBorderLine;

double offsetDistance;
int numFaces;
int numVertices;



//! This method has to be called, when the mesh was geometrically modified resulting in a new bounding box.
//! See Mesh::estBoundingBox as this will always call changedBoundingBox.
//! e.g. used in MeshGL to reset the vertex buffer of the bounding box.
//! @returns false in case of error. True otherwise.
bool Mesh::changedBoundingBox() {
	return true;
}

//! Estimates the (cached) values of the bounding box.
//! The method is private and must be called from any method changing the geometry of the mesh.
//! Attention: has to be wisley used or the performance will be slow.
//! @returns false in case of an error.
bool Mesh::estBoundingBox() {
	cout << "[Mesh::" << __FUNCTION__ << "]" << endl;

	// Bounding Box coordinates:
	mMinX = +DBL_MAX;
	mMaxX = -DBL_MAX;
	mMinY = +DBL_MAX;
	mMaxY = -DBL_MAX;
	mMinZ = +DBL_MAX;
	mMaxZ = -DBL_MAX;

	double vertXYZ[3];
	for( auto const& currVertex: mVertices ) {
		currVertex->copyXYZTo( vertXYZ );
		if( mMinX > vertXYZ[0] ) {
			mMinX = vertXYZ[0];
		}
		if( mMaxX < vertXYZ[0] ) {
			mMaxX = vertXYZ[0];
		}
		if( mMinY > vertXYZ[1] ) {
			mMinY = vertXYZ[1];
		}
		if( mMaxY < vertXYZ[1] ) {
			mMaxY = vertXYZ[1];
		}
		if( mMinZ > vertXYZ[2] ) {
			mMinZ = vertXYZ[2];
		}
		if( mMaxZ < vertXYZ[2] ) {
			mMaxZ = vertXYZ[2];
		}
	}
	changedBoundingBox();
	return true;
}

// normal vector related -------------------------------------------------------

//! More efficent version as we only pass references not objects nor sets/lists.
//! normalVec has to be a pointer to float[3] (or float[4] - while we will not
//! modify the homogenous coordinate, which would lead to segmentation faults,
//! when float[3] is encountered).
//!
//! \return Total area of faces.
double Mesh::averageNormalByArea( set<Face*>* someFaces, //!< input: list of faces
                                  double*     normalVec  //!< output: average vector weighted by the faces areas. will be initalized to ( 0.0, 0.0, 0.0 )
    ) {
	normalVec[0] = 0.0;
	normalVec[1] = 0.0;
	normalVec[2] = 0.0;
	float area = 0.0;
	set<Face*>::iterator itFace;
	for( itFace=someFaces->begin(); itFace!=someFaces->end(); itFace++ ) {
		(*itFace)->addNormalTo( normalVec );
		area += (*itFace)->getNormalLen();
	}
	normalVec[0] /= area;
	normalVec[1] /= area;
	normalVec[2] /= area;
	return area/2.0;
}

//! More efficent version as we only pass references not objects nor sets/lists.
//! normalVec has to be a pointer to float[3] (or float[4] - while we will not
//! modify the homogenous coordinate, which would lead to segmentation faults,
//! when float[3] is encountered).
//!
//! \return Total area of faces.
double Mesh::averageNormalByArea( vector<Face*>* someFaces, //!< input: list of faces
                              double*        normalVec  //!< output: average vector weighted by the faces areas. will be initalized to ( 0.0, 0.0, 0.0 )
    ) {
	normalVec[0] = 0.0;
	normalVec[1] = 0.0;
	normalVec[2] = 0.0;
	float area = 0.0;
	vector<Face*>::iterator itFace;
	for( itFace=someFaces->begin(); itFace!=someFaces->end(); itFace++ ) {
		(*itFace)->addNormalTo( normalVec );
		area += (*itFace)->getNormalLen();
	}
	normalVec[0] /= area;
	normalVec[1] /= area;
	normalVec[2] /= area;
	return area/2.0;
}

//! Estimates the average normal vector of a given face list (very simple -
//! not even weighted by some distance, etc.).
//!
//! Additional the maximum angle and the average angle to the average normal
//! vector can be estimated, but will cost extra computing time.
Vector3D Mesh::averageNormal( set<Face*>* someFaces, float* maxAngleDeviation, float* avgAngleDeviation ) {

	Vector3D normalVecAverage;

	set<Face*>::iterator itFace;
	for( itFace=someFaces->begin(); itFace!=someFaces->end(); itFace++ ) {
		normalVecAverage += (*itFace)->getNormal();
	}
	normalVecAverage /= someFaces->size();

	if( ( maxAngleDeviation != nullptr ) || ( avgAngleDeviation != nullptr ) ) {
		float maxAngle = 0.0;
		float avgAngle = 0.0;
		float tempAngle = 0.0;
		for( itFace=someFaces->begin(); itFace!=someFaces->end(); itFace++ ) {
			tempAngle = angle( normalVecAverage, (*itFace)->getNormal() );
			if( tempAngle > maxAngle ) {
				maxAngle = tempAngle;
			}
			avgAngle += tempAngle;
		}
		avgAngle /= someFaces->size();
		if( maxAngleDeviation != nullptr ) {
			*maxAngleDeviation = maxAngle;
		}
		if( avgAngleDeviation != nullptr ) {
			*avgAngleDeviation = avgAngle;
		}
	}

	return normalVecAverage;
}

//! Estimates the average normal vector of a given face list (very simple -
//! not even weighted by some distance, etc.).
//!
//! Additional the maximum angle and the average angle to the average normal
//! vector can be estimated, but will cost extra computing time.
Vector3D Mesh::averageNormal( float* maxAngleDeviation, float* avgAngleDeviation ) {

	Vector3D normalVecAverage;

	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		normalVecAverage += currFace->getNormal();
	}
	normalVecAverage /= getFaceNr();

	if( ( maxAngleDeviation != nullptr ) || ( avgAngleDeviation != nullptr ) ) {
		float maxAngle = 0.0;
		float avgAngle = 0.0;
		float tempAngle = 0.0;
		for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
			currFace = getFacePos( faceIdx );
			tempAngle = angle( normalVecAverage, currFace->getNormal() );
			if( tempAngle > maxAngle ) {
				maxAngle = tempAngle;
			}
			avgAngle += tempAngle;
		}
		avgAngle /= getFaceNr();
		if( maxAngleDeviation != nullptr ) {
			*maxAngleDeviation = maxAngle;
		}
		if( avgAngleDeviation != nullptr ) {
			*avgAngleDeviation = avgAngle;
		}
	}

	return normalVecAverage;
}

// rasterization / voxelization ------------------------------------------------

//! Rasters the top-view of the mesh into a float array (similar to a z-buffer).
//! We assume (0,0,z) as the upper left corner of the raster.
//! Everything outside [0..xDim][0..yDim] will be clipped.
double* Mesh::rasterViewFromZ( int xDim, int yDim ) {

	cout << "[Mesh] rasterViewFromZ " << xDim << " x " << yDim << endl;

	double* rasterArray = new double[xDim*yDim];

	// initalize with lowest value possible:
	for( int i=0; i<xDim*yDim; i++ ) {
		rasterArray[i] = -FLT_MAX;
	}

	// write all faces into the raster:
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		currFace->rasterViewFromZ( rasterArray, xDim, yDim );
	}

	return rasterArray;
}

//! Rasters the top-view of the triangles described by the list of the coordinates of their vertices
//! into the given rasterArray.
//!
//! As this might be applied to border areas as well, elements where no triangle is rastered become not-a-number.
//!
//! vertexArr has to be of (homogenous) size vertexSize * 4
//! rasterArray has to be of size rasterSizeX * rasterSizeY
void Mesh::rasterViewFromZ(
                double*        vertexArr,
                uint64_t  vertexSize,
                double*        rasterArray,
                long           rasterSizeX,
                long           rasterSizeY
) {

	// 0. initalize raster:
	if( rasterArray == nullptr ) {
		cerr << "[Mesh::rasterViewFromZ] memory for rasterArray not allocated!" << endl;
		return;
	}
	//! \todo check if we can skip the initialization of rasterArray or make it faster
	// initalize with lowest value possible:
	for( long j=0; j<(rasterSizeX*rasterSizeY); j++ ) {
		rasterArray[j] = _NOT_A_NUMBER_DBL_;
	}
	//cout << "[MeshSeed] rasterViewFromZ rastersize: " << rasterSizeX << ", " << rasterSizeY << endl;

	uint64_t vertexAidx;
	uint64_t vertexBidx;
	uint64_t vertexCidx;
	//cout << "[Mesh::rasterViewFromZ] Array:    " << vertexSize*4 << endl;
	//cout << "[Mesh::rasterViewFromZ] Vertices: " << vertexSize << endl;
	//cout << "[Mesh::rasterViewFromZ] Faces:    " << (vertexSize+1)/3 << endl;
	for( uint64_t faceNr=0; faceNr<(vertexSize+1)/3; faceNr++ ) {
		vertexAidx = faceNr*12;
		vertexBidx = faceNr*12+4;
		vertexCidx = faceNr*12+8;
		//cout << "[Mesh::rasterViewFromZ] " << (vertexSize+1)/3 << " faces." << endl;

		// Bubble sort inspired (while we just have to sort three elements)
		uint64_t tempIdx;
		if( vertexArr[vertexAidx+1] > vertexArr[vertexBidx+1] ) {
			tempIdx    = vertexBidx;
			vertexBidx = vertexAidx;
			vertexAidx = tempIdx;
		}
		if( vertexArr[vertexBidx+1] > vertexArr[vertexCidx+1] ) {
			tempIdx    = vertexCidx;
			vertexCidx = vertexBidx;
			vertexBidx = tempIdx;
			//cout << "B <-> C" << endl;
			if( vertexArr[vertexAidx+1] > vertexArr[vertexBidx+1] ) {
				tempIdx    = vertexBidx;
				vertexBidx = vertexAidx;
				vertexAidx = tempIdx;
				//cout << "A <-> B" << endl;
			}
		}
		// at this point the vertices are sorted by Y ascending - so A has min(y), B has med(y) and C has max(y).

		long  mMinY = static_cast<long>(floor( vertexArr[vertexAidx+1] + 0.5 ));
		long  medY  = static_cast<long>(floor( vertexArr[vertexBidx+1] + 0.5 ));
		long  mMaxY = static_cast<long>(floor( vertexArr[vertexCidx+1] + 0.5 ));
		//cout << "[Mesh::rasterViewFromZ] rasterViewFromZ spans in Y from: " << mMinY << " to " << medY << " to " << mMaxY << "." << endl;

		// For debuging - draw points:
		//int posX; int posY;
		//posX = floor( vertexArr[vertexAidx] + 0.5 );
		//posY = floor( vertexArr[vertexAidx+1] + 0.5 );
		//cout << "A("<<vertexAidx<<"): " << posX << " " << posY << " - ";
		//if( ( posX >= 0 ) && ( posY < rasterSizeY ) ) {
		//	rasterArray[posX+posY*rasterSizeX] = rasterSizeX/2.0;
		//}
		//posX = floor( vertexArr[vertexBidx] + 0.5 );
		//posY = floor( vertexArr[vertexBidx+1] + 0.5 );
		//cout << "B("<<vertexBidx<<"): " << posX << " " << posY << " - ";
		//if( ( posX >= 0 ) && ( posY < rasterSizeY ) ) {
		//	rasterArray[posX+posY*rasterSizeX] = rasterSizeX/2.0;
		//}
		//posX = floor( vertexArr[vertexCidx] + 0.5 );
		//posY = floor( vertexArr[vertexCidx+1] + 0.5 );
		//cout << "C("<<vertexCidx<<"): " << posX << " " << posY << endl;
		//if( ( posX >= 0 ) && ( posY < rasterSizeY ) ) {
		//	rasterArray[posX+posY*rasterSizeX] = rasterSizeX/2.0;
		//}

		double zCurrLong = vertexArr[vertexAidx+2]; // vertMinY->getZ();
		double zStepLong = ( vertexArr[vertexCidx+2] - zCurrLong ) / ( mMaxY - mMinY ); // ( vertMaxY->getZ() - vertMinY->getZ() ) / ( mMaxY - mMinY );
		double xCurrLong = vertexArr[vertexAidx]; // vertMinY->getX();
		double xStepLong = ( vertexArr[vertexCidx] - xCurrLong ) / ( mMaxY - mMinY ); // ( vertMaxY->getX() - vertMinY->getX() ) / ( mMaxY - mMinY );

		double zCurrShort1 = zCurrLong; // vertMinY->getZ();
		double zStepShort1 = ( vertexArr[vertexBidx+2] - zCurrLong ) / ( medY - mMinY ); // ( VertMedY->getZ() - vertMinY->getZ() ) / ( medY - mMinY );
		double xCurrShort1 = xCurrLong; // vertMinY->getX();
		double xStepShort1 = ( vertexArr[vertexBidx] - xCurrLong ) / ( medY - mMinY ); // ( VertMedY->getX() - vertMinY->getX() ) / ( medY - mMinY );

		double zCurrShort2 = vertexArr[vertexBidx+2]; // VertMedY->getZ();
		double zStepShort2 = ( vertexArr[vertexCidx+2] - zCurrShort2 ) / ( mMaxY - medY ); // ( vertMaxY->getZ() - VertMedY->getZ() ) / ( mMaxY - medY );
		double xCurrShort2 = vertexArr[vertexBidx]; // VertMedY->getX();
		double xStepShort2 = ( vertexArr[vertexCidx] - xCurrShort2 ) / ( mMaxY - medY ); // ( vertMaxY->getX() - VertMedY->getX() ) / ( mMaxY - medY );

		xCurrLong -= xStepLong;
		zCurrLong -= zStepLong;
		xCurrShort1 -= xStepShort1;
		zCurrShort1 -= zStepShort1;
		xCurrShort2 -= xStepShort2;
		zCurrShort2 -= zStepShort2;
		long  rasterShortX;
		long  rasterLongX;
		long  scanLineX0;
		long  scanLineX1;
		double scanLineZ0;
		double scanLineZ1;
		double zCurrShort;
		for( long rY=mMinY; rY<mMaxY; rY++ ) {
			xCurrLong += xStepLong;
			zCurrLong += zStepLong;
			rasterLongX = static_cast<long>(floor( xCurrLong + 0.5 ));
			if( rY < medY ) {
				// short1
				xCurrShort1 += xStepShort1;
				zCurrShort1 += zStepShort1;
				zCurrShort   = zCurrShort1;
				rasterShortX = static_cast<long>(floor( xCurrShort1 + 0.5 ));
			} else {
				// short2
				xCurrShort2 += xStepShort2;
				zCurrShort2 += zStepShort2;
				zCurrShort   = zCurrShort2;
				rasterShortX = static_cast<long>(floor( xCurrShort2 + 0.5 ));
			}
			//cout << "[MeshSeed] rasterViewFromZ Scanline: " << rasterShortX << " to " << rasterLongX << endl;
			if( rasterShortX > rasterLongX ) {
				scanLineX0 = rasterLongX;
				scanLineX1 = rasterShortX;
				scanLineZ0 = zCurrLong;
				scanLineZ1 = zCurrShort;
			} else {
				scanLineX0 = rasterShortX;
				scanLineX1 = rasterLongX;
				scanLineZ0 = zCurrShort;
				scanLineZ1 = zCurrLong;
			}
			if( ( scanLineX1 - scanLineX0 ) == 0 ) {
				continue;
			}
			double zCurrStep = ( scanLineZ1 - scanLineZ0 ) / ( scanLineX1 - scanLineX0 );
			double zCurr = scanLineZ0;
			for( long scanX = scanLineX0; scanX < scanLineX1; scanX++ ) {
				zCurr += zCurrStep;
				// we can not set pixels outside the array:
				// old with INT:
				if( ( scanX < 0 ) || ( scanX >= rasterSizeX ) || ( rY < 0 ) || ( rY >= rasterSizeY ) ) {
//				if( ( scanX >= rasterSizeX ) || ( rY >= rasterSizeY ) ) {
					continue;
				}
				//cout << "[MeshSeed] rasterViewFromZ set " << zCurrLong << " at " << scanX << ", " << rY << endl;
				rasterArray[scanX+rY*rasterSizeX] = zCurr;
			}
		}
	}
	//cout << "[MeshSeed] rasterViewFromZ END." << endl;
}

//! Renders the top-view of a rastered mesh (Mesh::rasterViewFromZ) as grayscale (height-map).
//! We assume (0,0,zDim/2) as the upper left corner of the image.
//!
//! Purpose: visual debuging and getting started with the conversion from
//! the mesh to a (cubical) voxel array.
//!
//! Provides a parallel projection of the top-view, which
//! might be of interest for rendering application.
uint8_t*  Mesh::rasterToHeightMap(
                const double* rasterArray,
                const int     xDim,
                const int     yDim,
                const int     zDim,
                const bool    scaleZ
) {
	cout << "[Mesh] rasterViewFromZ zDim: " << zDim << endl;

	uint8_t*  rasterGray  = static_cast<uint8_t*>(calloc( xDim * yDim, sizeof( uint8_t ) ));

	// scale [min(z)...max(z)] to [0..255]
	if( scaleZ ) {
		float zMin = +FLT_MAX;
		float zMax = -FLT_MAX;
		for( int i=0; i<xDim*yDim; i++ ) {
			// as we initalize the z-values with -FLT_MAX in  Mesh::rasterViewFromZ,
			// we have to ignore them:
			if( ( rasterArray[i] > -FLT_MAX ) && ( rasterArray[i] < zMin ) ) {
				zMin = rasterArray[i];
			}
			if( rasterArray[i] > zMax ) {
				zMax = rasterArray[i];
			}
		}
		float zSpan = zMax - zMin;
		cout << "[Mesh] rasterViewFromZ scale z: " << zMin << " ... " << zMax << endl;
		for( int i=0; i<xDim*yDim; i++ ) {
			rasterGray[i] = static_cast<uint>(255) * ( rasterArray[i] - zMin ) / zSpan;
		}
		return rasterGray;
	}

	// scale [-zDim/2...+zDim/] to [0..255]
	for( int i=0; i<xDim*yDim; i++ ) {
		if( rasterArray[i] < -zDim/2 ) {
			rasterGray[i] = 0;
			//cout << "[Mesh] rasterViewFromZ out of range zDim (<)." << endl;
			continue;
		}
		if( rasterArray[i] > zDim/2 ) {
			rasterGray[i] = 255;
			//cout << "[Mesh] rasterViewFromZ out of range zDim (>)." << endl;
			continue;
		}
		rasterGray[i] = static_cast<uint>(floor( ( rasterArray[i] - zDim/2 )*255/zDim + 0.5 ));
	}
	return rasterGray;
}

uint8_t* Mesh::rasterToVolume( const float* rasterArray, const int xDim, const int yDim, const int zDim ) {
	//! Transforms a rasterArray from Mesh::rasterViewFromZ into a cubic
	//! volume representation for further processing or exporting as image
	//! stack.

	uint8_t* cubeArray = static_cast<uint8_t*>(calloc( xDim * yDim * zDim, sizeof( uint8_t ) ));

	for( int z=0.0; z<zDim; z++ ) {
		for( int i=0; i<xDim*yDim; i++ ) {
			float zHeight = rasterArray[i] + zDim/2.0;
			if( z < zHeight ) {
				cubeArray[z*xDim*yDim+i] = 255;
			} else if( ( z - zHeight ) < 1.0 ) {
				cubeArray[z*xDim*yDim+i] = static_cast<uint8_t>(z-zHeight)*255;
				//cout << "[Mesh] rasterToVolume - intersected voxel." << endl;
			} else {
				cubeArray[z*xDim*yDim+i] = 0;
			}
		}
	}

	return cubeArray;
}

// --- IMPORT / EXPORT -----------------------------------------------------------------------------------

//! Uses PSALM as library to fill (closed!) polygonal lines e.g from holes.
//! 
//! In case the polygonal lines are non-manifold the results will be unexpected.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::fillPolyLines(
        const uint64_t&   rMaxNrVertices,   //!< Maximum numbers of vertices within a border for processing. 0 means no limit.
        uint64_t&         rFilled,          //!< Returns number of holes filled.
        uint64_t&         rFail,            //!< Returns number of holes failed to fill by libpsalm.
        uint64_t&         rSkipped          //!< Returns number of holes skipped.
) {
#ifndef LIBPSALM
	rFilled = 0;
	rFail   = mPolyLines.size();
	std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: libpsalm missing!" << std::endl;
	std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: All " << rFail << " holes ignored!" << std::endl;
	return( false );
#else
	// (Re)Set counters:
	rFilled  = 0;
	rFail    = 0;
	rSkipped = 0;

	int timeStart = clock(); // for performance mesurement
	showProgressStart( "Fill holes" );

	// Maintain indices in case the index do not match the position within the vector.
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		currVertex->setIndex( vertIdx );
	}
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		currFace->setIndex( faceIdx );
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] set indices took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds. " << std::endl;
	std::cout << "[Mesh::" << __FUNCTION__ << "] Total number of polylines/holes: " << mPolyLines.size() << std::endl;
	std::cout << "[Mesh::" << __FUNCTION__ << "] Maximum number of vertices/edges: " << rMaxNrVertices << std::endl;

	Vertex* vertexRef;
	float holeCtr = 0.0;
	vector<PolyLine*>::iterator itPoly;
	set<Face*> borderAndNewFaces; // we need this later to sew in the new faces into the mesh
	for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
		// Progressbar:
		showProgress( static_cast<double>(holeCtr+1)/static_cast<double>(mPolyLines.size()), "Fill holes" );
		holeCtr++;
		double newArea = 0.0;
		// Input for fillhole:
		uint64_t numVertices   = ((*itPoly)->length())-1;
		// Take care about smallest holes
		if( numVertices < 3 ) {
			std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Hole has to have more than three vertices. It has only " << numVertices << "!" << std::endl;
			continue;
		}
		// Skip holes larger than ... given by user.
		if( ( rMaxNrVertices > 0 ) && ( rMaxNrVertices < numVertices ) ) {
			std::cout << "[Mesh::" << __FUNCTION__ << "] Hole No. " << holeCtr << " SKIPPED: to many vertices; " << numVertices << std::endl;
			rSkipped++;
			continue;
		}
		if( numVertices == 3 ) { // Trivial triangular hole, to be filled with a triangle.
			VertexOfFace* vertA = static_cast<VertexOfFace*>((*itPoly)->getVertexRef( 0 ));
			VertexOfFace* vertB = static_cast<VertexOfFace*>((*itPoly)->getVertexRef( 1 ));
			VertexOfFace* vertC = static_cast<VertexOfFace*>((*itPoly)->getVertexRef( 2 ));
			vertA->getFaces( &borderAndNewFaces );
			vertB->getFaces( &borderAndNewFaces );
			vertC->getFaces( &borderAndNewFaces );
			int rIndex = mFaces.size();
			Face* newFace = new Face( rIndex, vertA, vertB, vertC );
			newArea += newFace->getAreaNormal(); // Area of the new patch.
			newFace->setFlag( FLAG_SYNTHETIC );  // Tag as synthetic.
			mFaces.push_back( newFace );
			borderAndNewFaces.insert( newFace );
			rFilled++;
			std::cout << "[Mesh::" << __FUNCTION__ << "] Hole No. " << holeCtr << " ADD vertices: none faces: 1" << std::endl;
		} else if( numVertices == 4 ) { // Quadtriangular hole. Attention: concave quadtriangles!
			VertexOfFace* vertA = static_cast<VertexOfFace*>((*itPoly)->getVertexRef( 0 ));
			VertexOfFace* vertB = static_cast<VertexOfFace*>((*itPoly)->getVertexRef( 1 ));
			VertexOfFace* vertC = static_cast<VertexOfFace*>((*itPoly)->getVertexRef( 2 ));
			VertexOfFace* vertD = static_cast<VertexOfFace*>((*itPoly)->getVertexRef( 3 ));
			vertA->getFaces( &borderAndNewFaces );
			vertB->getFaces( &borderAndNewFaces );
			vertC->getFaces( &borderAndNewFaces );
			vertD->getFaces( &borderAndNewFaces );
			Vector3D vAB = vertA->getPositionVector() - vertB->getPositionVector();
			Vector3D vDB = vertD->getPositionVector() - vertB->getPositionVector();
			Vector3D vBC = vertB->getPositionVector() - vertC->getPositionVector();
			Vector3D vDC = vertD->getPositionVector() - vertC->getPositionVector();
			float varA = ( vAB % vDB ).getLength3() + ( vBC % vDC ).getLength3();
			Vector3D vAD = vertA->getPositionVector() - vertD->getPositionVector();
			float varB = ( vAB % -vBC ).getLength3() + ( -vDC % vAD ).getLength3();
			int rIndex = mFaces.size();
			Face* newFaceA;
			Face* newFaceB;
			if( varA < varB ) { // Use the smaller of the two possible patches to prevent degenerated cases occuring due to concavities.
				newFaceA = new Face( rIndex++, vertA, vertB, vertD );
				newFaceB = new Face( rIndex,   vertB, vertC, vertD );
			} else {
				newFaceA = new Face( rIndex++, vertA, vertB, vertC );
				newFaceB = new Face( rIndex,   vertC, vertD, vertA );
			}
			newArea += newFaceA->getAreaNormal(); // Area of the new patch.
			newArea += newFaceB->getAreaNormal(); // Area of the new patch.
			newFaceA->setFlag( FLAG_SYNTHETIC );  // Tag as synthetic.
			newFaceB->setFlag( FLAG_SYNTHETIC );  // Tag as synthetic.
			mFaces.push_back( newFaceA );
			mFaces.push_back( newFaceB );
			borderAndNewFaces.insert( newFaceA );
			borderAndNewFaces.insert( newFaceB );
			rFilled += 2;
			std::cout << "[Mesh::" << __FUNCTION__ << "] Hole No. " << holeCtr << " ADD vertices: none faces: 2" << std::endl;
		} else {
			// Apply libpsalm:
			vector<long>   vertexIDs;
			vector<double> coordinates;
			vertexIDs.resize(   numVertices,   _NOT_A_NUMBER_INT_ );
			coordinates.resize( numVertices*3, _NOT_A_NUMBER_DBL_ );
			// Libpsalm parameters:
			double     borderDensity = 0.0;
			// Color average used for the new vertices
			bool useAverageColor = false;
			uint64_t colorAvgRed = 0;
			uint64_t colorAvgGrn = 0;
			uint64_t colorAvgBlu = 0;
			getParamFlagMesh( FILLPOLYLINES_COLOR_AVG, &useAverageColor );

			// Walk along edges
			for( uint64_t j=0; j<numVertices; j++ ) {
				vertexRef = (*itPoly)->getVertexRef( j );
				vertexIDs.at( j )       = (vertexRef->getIndex())+1; // as an Index of zero is a problem
				coordinates.at( j*3 )   = vertexRef->getX();
				coordinates.at( j*3+1 ) = vertexRef->getY();
				coordinates.at( j*3+2 ) = vertexRef->getZ();
				vertexRef->getFaces( &borderAndNewFaces );
				// Color average used for the new vertices
				if( useAverageColor ) {
					colorAvgRed += vertexRef->getR();
					colorAvgGrn += vertexRef->getG();
					colorAvgBlu += vertexRef->getB();
				}
				// Accumulate area - reciprocal value for density estimation. We have to account only for 1/3 of the area as faces are shared between vertices:
				borderDensity += vertexRef->get1RingArea() * 2.0 / 3.0; // Mulitple by 2 as vertices along the border have approx. half of the faces connected than non-border vertices.
			}

			// ADDITIONAL Check edge lengths:
			// double minLen = +_INFINITE_DBL_;
			// for( int j=1; j<numVertices; j++ ) {
			// 	double currDist = pow( coordinates.at( j*3 )   - coordinates.at( (j-1)*3 ),   2.0 ) +
			// 	                  pow( coordinates.at( j*3+1 ) - coordinates.at( (j-1)*3+1 ), 2.0 ) +
			// 	                  pow( coordinates.at( j*3+2 ) - coordinates.at( (j-1)*3+2 ), 2.0 );
			// 	minLen = min( minLen, sqrt( currDist ) );
			// }
			// cout << "[Mesh::" << __FUNCTION__ << "] Hole No. " << holeCtr << " BORDER minimum length: " << minLen << endl;

			// Color average used for the new vertices
			if( useAverageColor ) {
				colorAvgRed /= numVertices;
				colorAvgGrn /= numVertices;
				colorAvgBlu /= numVertices;
			}
			// Estimate average density:
			borderDensity = numVertices / borderDensity;
			// Alternative:
			std::cout << "[Mesh::" << __FUNCTION__ << "] Hole No. " << holeCtr << " BORDER vertices: " << numVertices << " density: " 
			          << borderDensity << " faces: " << borderAndNewFaces.size() << std::endl;
			//--------------------------------------------------------------------------------------------------------------------------------------
			// Variable for the return values of fillhole:
			size_t        numNewVertices = 0;
			double*    newCoordinates = nullptr;
			int        numNewFaces    = 0;
			long*      newVertexIDs   = nullptr;
			// Actually fill the current hole:
			if( !fill_hole( numVertices, vertexIDs.data(), coordinates.data(),
			                nullptr, nullptr, // was normals along the border (optional)
			                &numNewVertices, &newCoordinates, &numNewFaces, &newVertexIDs ) ) {
				std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Hole No. " << holeCtr << " having " 
				          << numVertices << "vertices FAILED!" << std::endl;
				rFail++;
				continue;
			}
			// Add the new vertices and faces
			std::cout << "[Mesh::" << __FUNCTION__ << "] Hole No. " << holeCtr << " ADD vertices: " << numNewVertices << " faces: " << numNewFaces << std::endl;
			vector<VertexOfFace*> tmpRefNewVertices; // We need this temporarly for connecting the faces.
			tmpRefNewVertices.resize( numNewVertices, nullptr );
			for( size_t i=0; i<numNewVertices; ++i ) {
				tmpRefNewVertices.at( i ) = new VertexOfFace( Vector3D( newCoordinates[i*3], newCoordinates[i*3+1], newCoordinates[i*3+2] ) );
				tmpRefNewVertices.at( i )->setFlag( FLAG_SYNTHETIC );
				tmpRefNewVertices.at( i )->setRGB( 255, 0, 0 );
				// color average
				if( useAverageColor ) {
					tmpRefNewVertices.at( i )->setRGB( colorAvgRed, colorAvgGrn, colorAvgBlu );
				}
				mVertices.push_back( tmpRefNewVertices.at( i ) );
			}
			int faceIdMax = getFaceNr();
			for( int i=0; i<numNewFaces; ++i ) {
				// Face( int setIdx, Vertex* setA, Vertex* setB, Vertex* setC, unsigned char* setTexRGB=NULL );
				VertexOfFace* newVertA = nullptr;
				VertexOfFace* newVertB = nullptr;
				VertexOfFace* newVertC = nullptr;
				if( newVertexIDs[i*3] < 0 ) {
					// Reference to a vertex of the border
					newVertA = static_cast<VertexOfFace*>( getVertexPos( abs( newVertexIDs[i*3] )-1 ) ); // compensate for +1 above!
				} else {
					// Reference to a new vertex
					newVertA = tmpRefNewVertices.at( newVertexIDs[i*3] );
				}
				if( newVertexIDs[i*3+1] < 0 ) {
					// Reference to a vertex of the border
					newVertB = static_cast<VertexOfFace*>( getVertexPos( abs( newVertexIDs[i*3+1] )-1 ) ); // compensate for +1 above!
				} else {
					// Reference to a new vertex
					newVertB = tmpRefNewVertices.at( newVertexIDs[i*3+1] );
				}
				if( newVertexIDs[i*3+2] < 0 ) {
					// Reference to a vertex of the border
					newVertC = static_cast<VertexOfFace*>( getVertexPos( abs( newVertexIDs[i*3+2] )-1 ) ); // compensate for +1 above!
				} else {
					// Reference to a new vertex
					newVertC = tmpRefNewVertices.at( newVertexIDs[i*3+2] );
				}
				Face* newFace = new Face( faceIdMax+i, newVertA, newVertB, newVertC );
				// Add to vector for setup: connecting new faces and reconnecting border faces.
				borderAndNewFaces.insert( newFace );
				// Finally: add to Mesh
				mFaces.push_back( newFace );
				// Area of the new patch:
				double newFaceArea =newFace->getAreaNormal();
				if( newFaceArea == 0.0 ) {
					cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Zero area face " << newFace->getIndex() << " added!" << endl;
				}
				newArea += newFaceArea;
				// Tag as synthetic:
				newFace->setFlag( FLAG_SYNTHETIC );
			}
			std::cout << "[Mesh::" << __FUNCTION__ << "] New density: " << (numNewVertices+numVertices)/newArea << std::endl;
			delete[] newCoordinates; // created in libpsalm
			delete[] newVertexIDs;   // created in libpsalm
			rFilled++;
			//--------------------------------------------------------------------------------------------------------------------------------------
		} // END libpsalm un-modified
		// Re-connect border faces and connect new faces.
		set<Face*>::iterator itFaceBorder;
		for( itFaceBorder=borderAndNewFaces.begin(); itFaceBorder!=borderAndNewFaces.end(); itFaceBorder++ ) {
			(*itFaceBorder)->reconnectToFaces();
		}
		borderAndNewFaces.clear();
	}
	//vector<Face*>::iterator itFace;
	//for( itFace=newFaces.begin(); itFace!=newFaces.end(); itFace++ ) {
	    //(*itFace)->connectToFaces();
	//}
	showProgressStop( "Fill holes" );
	std::cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds. " << std::endl;
	return( true );
#endif
}

//! Exports the polygonal lines as ASCII file in the format:
//! .) Label No. Number of Vertices
//! .) Number of vertices of the polyline.
//! .) List of coordinates (X,Y and Z).
//! .) Optional values (strided):
//!   -) Vertex index within the mesh.
//!   -) Normal of the edge.
//! File extension: .pline
bool Mesh::exportPolyLinesCoords( filesystem::path rFileName, bool rWithNormals, bool rWithVertIdx ) {
	fstream filestr;

	int timeStart = clock(); // for performance mesurement

	if(rFileName.extension().empty())
	{
		rFileName += ".pline";
	}

	filestr.open( rFileName, fstream::out );
	if( !filestr.is_open() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Could not open file: '" << rFileName << "'." << endl;
		return false;
	} else {
		cout << "[Mesh::" << __FUNCTION__ << "] File open for writing: '" << rFileName << "'." << endl;
	}

	time_t rawtime;
	struct tm * timeinfo;

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	string timeInfoStr( asctime( timeinfo ) );
	timeInfoStr = timeInfoStr.substr( 0, timeInfoStr.length()-1 );

	std::stringstream strHeader;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | PLINE file with polylines computed by the GigaMesh Software Framework         |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | WebSite: https://gigamesh.eu                                                  |" << std::endl;
	strHeader << "# | EMail:   info@gigamesh.eu                                                     |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Contact: Hubert MARA <hubert.mara@iwr.uni-heidelberg.de>                      |" << std::endl;
	strHeader << "# |          FCGL - Forensic Computational Geometry Laboratory                    |" << std::endl;
	strHeader << "# |          IWR - Heidelberg University, Germany                                 |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Mesh:       " << getBaseName() << std::endl;
	strHeader << "# | - Vertices: " << getVertexNr() << std::endl; // <- this will be used by fill holes tools from Bastian Rieck
	strHeader << "# | - Faces:    " << getFaceNr() << std::endl;
	strHeader << "# | Polylines:  " << mPolyLines.size() << std::endl;
	strHeader << "# | Timestamp:  " << timeInfoStr << std::endl;
	filestr << strHeader.str();

    if ( rWithNormals && rWithVertIdx) {
        filestr << "# +------------------------------------------------------------------------------------------------------------------------" << endl;
        filestr << "# | Format: Label No. | Number of Vertices | id1 x1 y1 z1 nx1 ny1 nz1 id2 x2 y2 z2 nx2 ny2 nz2 ... idN xN yN zN nxN nyN nzN" << endl;
        filestr << "# +------------------------------------------------------------------------------------------------------------------------" << endl;
    } else if( rWithNormals && !rWithVertIdx ) {
		filestr << "# +------------------------------------------------------------------------------------------------------------" << endl;
		filestr << "# | Format: Label No. | Number of Vertices | x1 y1 z1 nx1 ny1 nz1 x2 y2 z2 nx2 ny2 nz2 ... xN yN zN nxN nyN nzN" << endl;
		filestr << "# +------------------------------------------------------------------------------------------------------------" << endl;
    } else if ( rWithVertIdx && !rWithNormals) {
        filestr << "# +------------------------------------------------------------------------------------" << endl;
        filestr << "# | Format: Label No. | Number of Vertices | id1 x1 y1 z1 id2 x2 y2 z2 ... idN xN yN zN" << endl;
        filestr << "# +------------------------------------------------------------------------------------" << endl;
    } else {
		filestr << "# +------------------------------------------------------------------------" << endl;
		filestr << "# | Format: Label No. | Number of Vertices | x1 y1 z1 x2 y2 z2 ... xN yN zN" << endl;
		filestr << "# +------------------------------------------------------------------------" << endl;
	}

	int vertexCount;
	Vertex* vertexRef;
	vector<PolyLine*>::iterator itPoly;
	for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
		uint64_t currLabel;
		(*itPoly)->getLabel( currLabel );
		filestr << currLabel << " ";
		vertexCount = (*itPoly)->length();
		filestr << (*itPoly)->length();
		double* polyNormals = nullptr;
		if( rWithNormals ) {
			polyNormals = (*itPoly)->getVertexNormals();
		}

		for( int j=0; j<vertexCount; j++ ) {
			vertexRef = (*itPoly)->getVertexRef( j );
			if( rWithVertIdx ) {
				int vertIdx = vertexRef->getIndex();
				filestr << " " << vertIdx;
			}
			filestr << setiosflags( ios::fixed ) << setprecision( 20 );
			filestr << " " << vertexRef->getX();
			filestr << " " << vertexRef->getY();
			filestr << " " << vertexRef->getZ();
			if( polyNormals != nullptr ) {
				filestr << setiosflags( ios::fixed ) << setprecision( 5 );
				filestr << " " << polyNormals[j*3];
				filestr << " " << polyNormals[j*3+1];
				filestr << " " << polyNormals[j*3+2];
			}
		}
		filestr << endl;
	}
	filestr.close();

	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	return true;
}

//! Exports the polygonal lines projected to the mesh plane as ASCII file in the format:
//! .) Label No. Number of Vertices
//! .) Number of vertices of the polyline.
//! .) List of coordinates (X,Y and Z).
//! .) Optional value (strided):
//!   -) Vertex index within the mesh.
//! File extension: .pline
bool Mesh::exportPolyLinesCoordsProjected( filesystem::path rFileName, bool rWithVertIdx, double rAngleRot ) {
	fstream filestr;
	int timeStart = clock(); // for performance mesurement

	if(rFileName.extension().empty())
	{
		rFileName += ".pline";
	}

	filestr.open( rFileName, fstream::out );
	if( !filestr.is_open() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Could not open file: '" << rFileName << "'!" << endl;
		return false;
	} else {
		cout << "[Mesh::" << __FUNCTION__ << "] File open for writing: '" << rFileName << "'." << endl;
	}

	// Get plane HNF ...
	Vector3D planeHNF;
	if( !mPlane.getPlaneHNF( &planeHNF ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: plane not defined!" << endl;
		return false;
	}
	// ... and pre-compute translation vector and rotation matrix into xy-plane.
	Matrix4D transMat;
	if( !mPlane.getChangeOfBasisTrans( &transMat, true ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: plane getChangeOfBasisTrans failed!" << endl;
		return false;
	}
	// Add extra rotation, e.g. from OpenGL
	transMat *= Matrix4D( Vector3D( 0.0, 0.0, 0.0, 1.0 ), Vector3D( 0.0, 0.0, 1.0, 0.0 ), rAngleRot );

	time_t rawtime;
	struct tm * timeinfo;

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	string timeInfoStr( asctime( timeinfo ) );
	timeInfoStr = timeInfoStr.substr( 0, timeInfoStr.length()-1 );

	std::stringstream strHeader;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | PLINE file with polylines computed by the GigaMesh Software Framework         |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | WebSite: https://gigamesh.eu                                                  |" << std::endl;
	strHeader << "# | EMail:   info@gigamesh.eu                                                     |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Contact: Hubert MARA <hubert.mara@iwr.uni-heidelberg.de>                      |" << std::endl;
	strHeader << "# |          FCGL - Forensic Computational Geometry Laboratory                    |" << std::endl;
	strHeader << "# |          IWR - Heidelberg University, Germany                                 |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Mesh:       " << getBaseName() << std::endl;
	strHeader << "# | - Vertices: " << getVertexNr() << std::endl; // <- this will be used by fill holes tools from Bastian Rieck
	strHeader << "# | - Faces:    " << getFaceNr() << std::endl;
	strHeader << "# | Polylines:  " << mPolyLines.size() << std::endl;
	strHeader << "# | Timestamp:  " << timeInfoStr << std::endl;
	filestr << strHeader.str();
	if( rWithVertIdx ) {
		filestr << "# +------------------------------------------------------------------------------------" << endl;
		filestr << "# | Format: Label No. | Number of Vertices | id1 x1 y1 z1 id2 x2 y2 z2 ... idN xN yN zN" << endl;
		filestr << "# +------------------------------------------------------------------------------------" << endl;
	} else {
		filestr << "# +------------------------------------------------------------------------" << endl;
		filestr << "# | Format: Label No. | Number of Vertices | x1 y1 z1 x2 y2 z2 ... xN yN zN" << endl;
		filestr << "# +------------------------------------------------------------------------" << endl;
	}

	int vertexCount;
	vector<PolyLine*>::iterator itPoly;
	for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
		uint64_t currLabel;
		(*itPoly)->getLabel( currLabel );
		filestr << currLabel << " ";
		vertexCount = (*itPoly)->length();
		filestr << (*itPoly)->length();
		// Step thru the list of vertices:
		Vertex* currVert;
		for( int j=0; j<vertexCount; j++ ) {
			currVert = (*itPoly)->getVertexRef( j );
			if( rWithVertIdx ) {
				int vertIdx = currVert->getIndex();
				filestr << " " << vertIdx;
			}
			// Fetch position vector:
			Vector3D vertPos3D = currVert->getCenterOfGravity();
			// Project on to the plane:
			if( !vertPos3D.projectOntoPlane( planeHNF ) ) {
				cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: can not project vertex onto plane!" << endl;
				continue;
			}
			// Shift and rotate plane into xy-plane
			vertPos3D *= transMat;
			// Dump to file:
			filestr << setiosflags( ios::fixed ) << setprecision( 20 );
			filestr << " " << vertPos3D.getX();
			filestr << " " << vertPos3D.getY();
			filestr << " " << vertPos3D.getZ();
		}
		filestr << endl;
	}
	filestr.close();

	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	return true;
}

//! Export the run-length and the function value (e.g. curvature) of the polyline as ASCII file.
bool Mesh::exportPolyLinesFuncVals( filesystem::path rFileName ) {
	fstream filestr;

	int timeStart = clock(); // for performance mesurement

	//! If no polylines are selected, automatically choose all.
	bool emptySelected = false;
    if( mPolyLinesSelected.empty() ) {
		emptySelected = true;
		vector<PolyLine*>::iterator itPoly;
		for( itPoly=mPolyLines.begin(); itPoly!=mPolyLines.end(); itPoly++ ) {
			mPolyLinesSelected.insert( (*itPoly) );
		}
	}

	if(rFileName.extension().empty())
	{
		rFileName += ".txt";
	}

	filestr.open( rFileName, fstream::out );
	if( !filestr.is_open() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] Could not open file: '" << rFileName << "'." << endl;
		return false;
	} else {
		cout << "[Mesh::" << __FUNCTION__ << "] File open for writing: '" << rFileName << "'." << endl;
	}

	time_t rawtime;
	struct tm * timeinfo;

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	string timeInfoStr( asctime( timeinfo ) );
	timeInfoStr = timeInfoStr.substr( 0, timeInfoStr.length()-1 );

	std::stringstream strHeader;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | PLINE file with polylines computed by the GigaMesh Software Framework         |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | WebSite: https://gigamesh.eu                                                  |" << std::endl;
	strHeader << "# | EMail:   info@gigamesh.eu                                                     |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Contact: Hubert MARA <hubert.mara@iwr.uni-heidelberg.de>                      |" << std::endl;
	strHeader << "# |          FCGL - Forensic Computational Geometry Laboratory                    |" << std::endl;
	strHeader << "# |          IWR - Heidelberg University, Germany                                 |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Mesh:       " << getBaseName() << std::endl;
	strHeader << "# | - Vertices: " << getVertexNr() << std::endl; // <- this will be used by fill holes tools from Bastian Rieck
	strHeader << "# | - Faces:    " << getFaceNr() << std::endl;
	strHeader << "# | Polylines:  " << mPolyLines.size() << std::endl;
	strHeader << "# | Timestamp:  " << timeInfoStr << std::endl;
	filestr << strHeader.str();
	filestr << "# +-------------------------------------------------------------------------------------------------" << endl;
	filestr << "# | Format: Label No. | Number of Vertices (N) | closed | rl1 rl2 rl3 ... rlN | fv1 fv2 fv3 ... fvN " << endl;
	filestr << "# +-------------------------------------------------------------------------------------------------" << endl;

	set<PolyLine*>::iterator itPoly;
	for( itPoly=mPolyLinesSelected.begin(); itPoly!=mPolyLinesSelected.end(); itPoly++ ) {
		PolyLine* currPoly = (*itPoly);
		int       vertexCount = currPoly->length();
		uint64_t currLabel;
		bool      isClosed = currPoly->isClosed();
		vector<double> edgeLengths;
		vector<double> vertFuncVals;
		currPoly->getLabel( currLabel );
		currPoly->getEdgeLengths( &edgeLengths );
		currPoly->getEdgeFuncVals( &vertFuncVals );
		filestr << currLabel;
		filestr << " " << vertexCount;
		filestr << " " << isClosed;
		vector<double>::iterator itDouble;
		// Write the edges' run-lengths - which always begins with 0.0
		filestr << " " << 0.0;
		for( itDouble=edgeLengths.begin(); itDouble!=edgeLengths.end(); itDouble++ ) {
			filestr << " " << (*itDouble);
		}
		// Write the function values of the vertices.
		for( itDouble=vertFuncVals.begin(); itDouble!=vertFuncVals.end(); itDouble++ ) {
			filestr << " " << (*itDouble);
		}
		filestr << endl;
	}
	filestr.close();

	if( emptySelected ) {
		// just in case all were automatically selected:
		mPolyLinesSelected.clear();
	}

	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	return true;
}

//! Exports the function values of the vertices.
//! File extension: .txt
bool Mesh::exportFuncVals( filesystem::path rFileName, bool rWithVertIdx ) {
	fstream filestr;

	int timeStart = clock(); // for performance mesurement

	if(rFileName.extension().empty())
	{
		rFileName += ".pline";
	}

	filestr.open( rFileName, fstream::out );
	if( !filestr.is_open() ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Could not open file: '" << rFileName << "'!" << endl;
		return false;
	} else {
		cout << "[Mesh::" << __FUNCTION__ << "] File open for writing: '" << rFileName << "'." << endl;
	}

	time_t rawtime;
	struct tm * timeinfo;

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	string timeInfoStr( asctime( timeinfo ) );
	timeInfoStr = timeInfoStr.substr( 0, timeInfoStr.length()-1 );

	std::stringstream strHeader;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | PLINE file with polylines computed by the GigaMesh Software Framework         |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | WebSite: https://gigamesh.eu                                                  |" << std::endl;
	strHeader << "# | EMail:   info@gigamesh.eu                                                     |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Contact: Hubert MARA <hubert.mara@iwr.uni-heidelberg.de>                      |" << std::endl;
	strHeader << "# |          FCGL - Forensic Computational Geometry Laboratory                    |" << std::endl;
	strHeader << "# |          IWR - Heidelberg University, Germany                                 |" << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	strHeader << "# | Mesh:       " << getBaseName() << std::endl;
	strHeader << "# | - Vertices: " << getVertexNr() << std::endl;
	strHeader << "# | - Faces:    " << getFaceNr() << std::endl;
	strHeader << "# | Timestamp:  " << timeInfoStr << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;
	filestr << strHeader.str();

	// Step thru the list of vertices:
	Vertex* currVert;
	for( uint64_t j=0; j<getVertexNr(); j++ ) {
		currVert = getVertexPos( j );
		if( rWithVertIdx ) {
			int vertIdx = currVert->getIndex();
			filestr << vertIdx << " ";
		}
		// Fetch function value:
		double funcVal;
		if( ! currVert->getFuncValue( &funcVal ) ) {
			cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: can not fetch function value!" << endl;
		}
		filestr << funcVal << endl;
	}
	filestr.close();

	cout << "[Mesh::" << __FUNCTION__ << "] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	return true;
}

//! Imports function values of the vertices
//! File extension: .txt or .mat
//! assumes that the files are either single column with the functionValue, or double column with index + functionValue
//! lines starting with # are treated as comments
bool Mesh::importFuncValsFromFile(const filesystem::path& rFileName, bool withVertIdx)
{
	ifstream filestr(rFileName);

	filestr.imbue(std::locale("C"));

	if(!filestr.is_open())
	{
		LOG::error() << "[Mesh::" << __FUNCTION__ << "] Could not open file: '" << rFileName << "'.\n";
		return false;
	}

	auto numVerts = getVertexNr();
	std::string line;
	double funcVal = 0.0;

	//importing with index
	if(withVertIdx)
	{
		uint64_t currIndex = 0;
		while(std::getline(filestr, line))
		{
			if(!line.empty())
			{
				if(line[0] == '#')
				{
					continue;
				}

				std::stringstream lineStream(line);
				if(!(lineStream >> currIndex >> funcVal))
				{
					LOG::warn() << "[Mesh::" << __FUNCTION__ << "] File: '" << rFileName << "' contains invalid values!\n";
					continue;
				}

				if(currIndex < numVerts)
				{
					mVertices[currIndex]->setFuncValue(funcVal);
				}
				else
				{
					LOG::warn() << "[Mesh::" << __FUNCTION__ << "] warning: function value out of range: " << currIndex << "\n";
				}
			}
		}
	}
	//importing without index
	else
	{
		uint64_t currIndex = 0;
		while(std::getline(filestr, line))
		{
			if(!line.empty())
			{
				if(line[0] == '#')
				{
					continue;
				}

				if(currIndex > numVerts)
				{
					LOG::warn() << "[Mesh::" << __FUNCTION__ << "] warning: function value out of range: " << currIndex - 1 << "\n";
					break;
				}
				try {
					funcVal = std::stod(line);
				}
				catch (std::exception& e)
				{
					LOG::warn() << "[Mesh::" << __FUNCTION__ << "] File: '" << rFileName << "' contains invalid values!\n";
					mVertices[currIndex++]->setFuncValue(0);
					continue;
				}

				mVertices[currIndex++]->setFuncValue(funcVal);
			}
		}
	}

	filestr.close();

	changedVertFuncVal();
	return true;
}

//! Imports labels of the vertices
//! File extension: .txt or .mat
//! assumes that the files are either single column with the labels, or double column with index + label
//! lines starting with # are treated as comments
//! labels are expected to be integers
bool Mesh::importLabelsFromFile(const filesystem::path& rFileName, bool withVertIdx)
{
    ifstream filestr(rFileName);

    filestr.imbue(std::locale("C"));

    if(!filestr.is_open())
    {
        LOG::error() << "[Mesh::" << __FUNCTION__ << "] Could not open file: '" << rFileName << "'.\n";
        return false;
    }

    auto numVerts = getVertexNr();
    std::string line;
    uint64_t labelNr = 0;

    //importing with index
    if(withVertIdx)
    {
        uint64_t currIndex = 0;
        while(std::getline(filestr, line))
        {
            if(!line.empty())
            {
                if(line[0] == '#')
                {
                    continue;
                }

                std::stringstream lineStream(line);
                if(!(lineStream >> currIndex >> labelNr))
                {
                    LOG::warn() << "[Mesh::" << __FUNCTION__ << "] File: '" << rFileName << "' contains invalid values!\n";
                    continue;
                }
                if(currIndex < numVerts)
                {
                    mVertices[currIndex]->setLabel(labelNr);
                }
                else
                {
                    LOG::warn() << "[Mesh::" << __FUNCTION__ << "] warning: function value out of range: " << currIndex << "\n";
                }
            }
        }
    }
    //importing without index
    else
    {
        uint64_t currIndex = 0;
        while(std::getline(filestr, line))
        {
            if(!line.empty())
            {
                if(line[0] == '#')
                {
                    continue;
                }

                if(currIndex > numVerts)
                {
                    LOG::warn() << "[Mesh::" << __FUNCTION__ << "] warning: function value out of range: " << currIndex - 1 << "\n";
                    break;
                }
                try {
                    labelNr = std::stod(line);
                }
                catch (std::exception& e)
                {
                    LOG::warn() << "[Mesh::" << __FUNCTION__ << "] File: '" << rFileName << "' contains invalid values!\n";
                    mVertices[currIndex++]->setLabel(0);
                    continue;
                }

                mVertices[currIndex++]->setLabel(labelNr);
            }
        }
    }

    filestr.close();
    labelsChanged();
    return true;
}

//! Imports the polylines coordinates from file
//! File extension: .pline
//! each line of the file is a line definition with coordinates
//! lines starting with # are treated as comments
//! currently only takes lines where the points are given as id, x, y, z, nx, ny, nz
bool Mesh::importPolylinesFromFile(const filesystem::path& rFileName)
{
    ifstream filestr(rFileName);

    filestr.imbue(std::locale("C"));

    if(!filestr.is_open())
    {
        LOG::error() << "[Mesh::" << __FUNCTION__ << "] Could not open file: '" << rFileName << "'.\n";
        return false;
    }
    std::string line;
    std::string elem;
    std::string substring;
    while(std::getline(filestr, line))
    {
        if(!line.empty())
        {
            //each line of the file without "#" is a polyline
            if(line[0] == '#')
            {
                continue;
            }
            PolyLine* tmpPolyLine = new PolyLine();
            //PolyLine* tmpPolyLine = new PolyLine( Vector3D( _NOT_A_NUMBER_, _NOT_A_NUMBER_, _NOT_A_NUMBER_, 1.0 ),
            //                                              Vector3D( _NOT_A_NUMBER_, _NOT_A_NUMBER_, _NOT_A_NUMBER_, 0.0 ) );
            int valueCount = 0;
			// need flag to only add tmpPolyline or delete it, otherwise can cause bugs if more 0 polylines are added than >3 polylines
            bool flag_add = true;
            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
            double normalX = 0.0;
            double normalY = 0.0;
            double normalZ = 0.0;
            //variables for the division of the values from String/line
            // values are seperated by space
            int start = 0;
            int end = line.find(" ");
            //create polyline
            while(end != -1){
                elem = line.substr(start,end-start);
                start = end + 1;
                end = line.find(" ",start);
                // check if polyline is at least 2 vertices long
                if(valueCount == 1){
                    if(stod(elem) < 3){
                        flag_add = false;
                        break;
                    }
                }
                //ignore the first 2 values id and number of vertices
                if(valueCount > 1){
                    //extract vertex coordinate
                    //each vertex has 7 elments of information: seperator, x,y,z normal x, ny,nz
                    //ignore the seperator "-1"
                    //separator often also is the index of the point in the mesh it was taken from

                    // -2 on the count because of the first 2 ignored values
                    if((valueCount-2) % 7 == 1){
                        x = stod(elem);
                    }
                    if((valueCount-2) % 7 == 2){
                        y = stod(elem);
                    }
                    if((valueCount-2) % 7 == 3){
                        z = stod(elem);
                    }
                    if((valueCount-2) % 7 == 4){
                        normalX = stod(elem);
                    }
                    if((valueCount-2) % 7 == 5){
                        normalY = stod(elem);
                    }
                    if((valueCount-2) % 7 == 6){
                        normalZ  = stod(elem);

                        //add vertex to polyline
                        Vertex* newVertex = new Vertex(Vector3D(x,y,z),Vector3D(normalX,normalY,normalZ));
                        tmpPolyLine->addBack( newVertex );
                    }
                }
                //count the extracted values of the string
                valueCount++;
            }

            //add the new polyline to the mesh
            if (flag_add){
                mPolyLines.push_back( tmpPolyLine );
            } else {
                delete tmpPolyLine;
            }
        }
    }

    filestr.close();

    polyLinesChanged();
    return true;
}

//! Imports the transformation matrices from file
//! File extension: .txt
//! the transmat.txt file after transformation with F6 is used
//! the file can contain more than one matrix
//! they are devides by
//! #Transformation ...
//! #......................................................
bool Mesh::importApplyTransMatFromFile(const std::filesystem::path &rFileName)
{
    ifstream filestr(rFileName);

    filestr.imbue(std::locale("C"));

    if(!filestr.is_open())
    {
        LOG::error() << "[Mesh::" << __FUNCTION__ << "] Could not open file: '" << rFileName << "'.\n";
        return false;
    }
    std::string line;
    std::string elem;
    std::string substring;
    int lineCount = 0;
    vector<double> values;
    while(std::getline(filestr, line))
    {
        if(!line.empty())
        {
            //each line of the file without "#" discribes a transfomration matrix
            //always with 4 lines (4x4 matrix)
            if(line[0] == '#')
            {
                continue;
            }
            // matrix line found
            lineCount++;
            //variables for the division of the values from String/line
            // values are seperated by space
            int start = 0;
            int end = line.find(" ");
            while(end != -1){
                elem = line.substr(start,end-start);
                start = end + 1;
                end = line.find(" ",start);
                values.push_back(stod(elem));
            }
            //add last value because the line ends not with space
            elem = line.substr(start,line.size());
            values.push_back(stod(elem));

            //matrix values are complete
            if(lineCount == 4){
                //transpose values
                std::array<double,16> transposedValues;
                unsigned int row = 0;
                unsigned int column = 0;
                for(const auto& value : values){
                    transposedValues[(row*4)+column] = value;
                    row++;
                    if( row == 4 ){
                        column++;
                        row = 0;
                    }
                }
                vector<double> transposedValuesVec(std::begin(transposedValues),std::end(transposedValues));
                Matrix4D matrix(transposedValuesVec);
                applyTransformationToWholeMesh(matrix, true, false);
                values.clear();
                lineCount = 0;
            }
        }
    }

    filestr.close();
    return true;
}
//! Exports the faces normals as sphereical coordinates as ASCII file in the format:
//! Face Number Phi Theta Radius
//! File extension: .facen
bool Mesh::exportFaceNormalAngles( filesystem::path filename ) {

	fstream filestr;

	int timeStart = clock(); // for performance mesurement

	if( filename.extension().empty()) {
		filename += ".facen";
	}

	filestr.open( filename, fstream::out );
	if( !filestr.is_open() ) {
		cerr << "[Mesh::exportFaceNormalAngles] Could not open file: '" << filename << "'." << endl;
		return false;
	} else {
		cout << "[Mesh::exportFaceNormalAngles] File open for writing: '" << filename << "'." << endl;
	}

	time_t rawtime;
	struct tm * timeinfo;

	time( &rawtime );
	timeinfo = localtime( &rawtime );
	string timeInfoStr( asctime( timeinfo ) );
	timeInfoStr = timeInfoStr.substr( 0, timeInfoStr.length()-1 );

	filestr << "# " << getFaceNr() << " Face normals from Mesh " << getBaseName() << endl;
	filestr << "# Exported " << timeInfoStr << " from GigaMesh" << endl;
	filestr << "#------------------------------------------------------------------------" << endl;
	filestr << "# Format: Face No. | Phi | Theta | Radius                                " << endl;
	filestr << "#------------------------------------------------------------------------" << endl;

	Face* currFace;
	double phi;
	double theta;
	double radius;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		currFace->setIndex( faceIdx );
		currFace->getNormalSpherical( &phi, &theta, &radius );
		filestr << faceIdx << " ";
		filestr << phi     << " ";
		filestr << theta   << " ";
		filestr << radius << endl;
	}

	filestr.close();

	cout << "[Mesh::exportFaceNormalAngles] took " << static_cast<float>( clock() - timeStart ) / CLOCKS_PER_SEC << " seconds. " << endl;
	return true;
}



// Extra menu --------------------------------------------------------------------------------------------------

//! Apply a LaTeX template to set meta-data for the current mesh e.g. report.
void Mesh::cuneiformFigureLaTeX() {
	// Do nothing, becaue overloaded in MeshQt.
	cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: No methods for LATEX_TEMPLATE here!" << endl;
}

// Mesh information --------------------------------------------------------------------

bool Mesh::getFaceSurfSum( double* rSurfSum ) {
	//! Returns the sum of all the faces areas.
	if( rSurfSum == nullptr ) {
		return false;
	}
	(*rSurfSum) = 0.0;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		(*rSurfSum) += currFace->getAreaNormal();
	}
	return true;
}

//! Returns a string containing the Vertex indices and position defining the Mesh plabe.
bool Mesh::getPlaneVPos( string* rPlaneInfo ) {
	if( rPlaneInfo == nullptr ) {
		return false;
	}
	double coords[9];
	if( !mPlane.getVertCoords( coords ) ) {
		return false;
	}
	ostringstream strs;
	strs << coords[0] << " ";
	strs << coords[1] << " ";
	strs << coords[2] << endl;
	strs << coords[3] << " ";
	strs << coords[4] << " ";
	strs << coords[5] << endl;
	strs << coords[6] << " ";
	strs << coords[7] << " ";
	strs << coords[8] << endl;
	(*rPlaneInfo) = strs.str();
	return true;
}

//! Returns a string containing the Hesse Normal Form (HNF) defining the Mesh plabe.
bool Mesh::getPlaneHNF( string* rPlaneInfo ) {
	if( rPlaneInfo == nullptr ) {
		return false;
	}
	Vector3D planeHNF;
	if( !mPlane.getPlaneHNF( &planeHNF ) ) {
		return false;
	}
	ostringstream strs;
	strs << planeHNF << endl;
	(*rPlaneInfo) = strs.str();
	return true;
}

//! Print matlab string with the normals of the selected faces in spherical coordinates.
bool Mesh::dumpMatlabFaceNormalsSel( string* rStrMatlab ) {
	stringstream matlabArr;
	matlabArr << "faceNormalSel = [";
	set<Face*>::iterator itFace;
	for( itFace=mFacesSelected.begin(); itFace!=mFacesSelected.end(); itFace++ ) {
		Vector3D faceNormal = (*itFace)->getNormal( false );
		matlabArr << faceNormal.getSphPhiDeg() << " ";
		matlabArr << faceNormal.getSphThetaDeg() << " ";
		matlabArr << faceNormal.getSphRadius() << "; ";
	}
	matlabArr << " ];";

	if( rStrMatlab != nullptr ) {
		//! When rStrLatex is not NULL, we will use it to return the LaTeX string.
		(*rStrMatlab) = matlabArr.str();
	} else {
		//! Otherwise the string will be dumped to stdout.
		cout << matlabArr.str() << endl;
	}
	return true;
}

//! Return a map of strings to fill the place holders of the LaTeX files.
//! Place holders set:
bool Mesh::latexFetchFigureInfos( vector<pair<string,string>>* rStrings ) {
	//! PDF Header
	// Write hostname and username - see: https://stackoverflow.com/questions/27914311/get-computer-name-and-logged-user-name
#ifdef WIN32 //mingw and msvc
	WSAData wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	char hostname[256] = {0};
	auto error = gethostname(hostname, 256);

	if(error != 0)
	{
		std::cerr << "Error, could not get hostname!" << std::endl;
	}
	char username[UNLEN- 1] = {0};
	DWORD len = UNLEN - 1;
	if(!GetUserNameA(username,&len))
	{
		std::cerr << "Error, could not get username!" << std::endl;
	}
	WSACleanup();
#else
    #ifndef HOST_NAME_MAX
	    const size_t HOST_NAME_MAX = 256;
    #endif
    #ifndef LOGIN_NAME_MAX
		const size_t LOGIN_NAME_MAX = 256;
    #endif
	char hostname[HOST_NAME_MAX] = {0};
	char username[LOGIN_NAME_MAX] = {0};
	gethostname( hostname, HOST_NAME_MAX );
	auto error = getlogin_r( username, LOGIN_NAME_MAX );

	if(error != 0)
	{
		std::cerr << "Error, could not get username!" << std::endl;
	}
#endif
	rStrings->push_back( pair<string,string>( string( "__PDF_AUTHOR__"  ),
	                     string( username )+"@"+string( hostname ) ) ); //! __PDF_AUTHOR__

	//! Dimensions:
	Vector3D bbDim;
	getBoundingBoxSize( bbDim );
	double bbWdith  = round( bbDim.getX()*1.0 ) / 10.0;
	double bbHeight = round( bbDim.getY()*1.0 ) / 10.0;
	double bbThick  = round( bbDim.getZ()*1.0 ) / 10.0;

	rStrings->push_back( pair<string,string>( string( "__BOUNDING_BOX_WIDTH__"  ), to_string( bbWdith  ) ) ); //! __BOUNDING_BOX_WIDTH__
	rStrings->push_back( pair<string,string>( string( "__BOUNDING_BOX_HEIGHT__" ), to_string( bbHeight ) ) ); //! __BOUNDING_BOX_HEIGHT__
	rStrings->push_back( pair<string,string>( string( "__BOUNDING_BOX_THICK__"  ), to_string( bbThick  ) ) ); //! __BOUNDING_BOX_THICK__

	//! Primitive count:
    //have to be double otherwise the trailing zeros will be deleted
    double vertexCount = getVertexNr();
    double faceNr = getFaceNr();
	rStrings->push_back( pair<string,string>( string( "__VERTEX_COUNT__" ), to_string( vertexCount )   ) ); //! __VERTEX_COUNT__
    rStrings->push_back( pair<string,string>( string( "__FACE_COUNT__"  ),  to_string( faceNr  ) ) ); //! __FACE_COUNT__

	//! Meta-data:
	std::string metaObjectId       = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
	std::string metaObjectMaterial = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
	std::string metaObjectIdLaTeX;
	std::string metaObjectMaterialLaTeX;
	std::regex e( "_" );
	std::regex_replace( std::back_inserter(metaObjectIdLaTeX), metaObjectId.begin(), metaObjectId.end(), e, "\\_" );
	std::regex_replace( std::back_inserter(metaObjectMaterialLaTeX), metaObjectMaterial.begin(), metaObjectMaterial.end(), e, "\\_" );
	rStrings->push_back( pair<string,string>( string( "__OBJECT_ID__" ), metaObjectIdLaTeX             ) ); //! __OBJECT_ID__
	rStrings->push_back( pair<string,string>( string( "__OBJECT_MATERIAL__" ), metaObjectMaterialLaTeX ) ); //! __OBJECT_MATERIAL__
	// Adapt web-reference
	//! \todo extend for multiple links.
	std::string strWebRef = getModelMetaDataRef().getModelMetaString( ModelMetaData::META_REFERENCE_WEB );
	if( strWebRef.size() == 0 ) { // When empty
		rStrings->push_back( pair<string,string>( string( "__WEB_REFERENCE__" ), "" ) ); //! __WEB_REFERENCE__ when empty
	} else { //! \todo check if there is at least one properly formatted web-reference.
		std::size_t posLeftBracket  = strWebRef.find( "[" );
		std::size_t posRightBracket = strWebRef.find( "]" );
		std::size_t posCenter       = strWebRef.find( "|" );
		std::string strWebRefLaTeX = "\\href{" + strWebRef.substr( posLeftBracket+1, posCenter-posLeftBracket-1 ) +
		                             "}{" + strWebRef.substr( posCenter+1, posRightBracket-posCenter-1 ) + "}";
		rStrings->push_back( pair<string,string>( string( "__WEB_REFERENCE__" ), strWebRefLaTeX ) ); //! __WEB_REFERENCE__ formatted for LaTeX
	}

	//! Area and average resolution
	double areaAcq;
	getFaceSurfSum( &areaAcq );
	double areaResMetric = round( vertexCount/areaAcq )*100.0;
	double areaResDPI    = round( 25.4*sqrt(vertexCount/areaAcq) );
	areaAcq = round( areaAcq );
	rStrings->push_back( pair<string,string>( string( "__AREA_TOTAL__" ),             to_string( areaAcq )       ) ); //! __AREA_TOTAL__
	rStrings->push_back( pair<string,string>( string( "__AREA_RESOLUTION_METRIC__" ), to_string( areaResMetric ) ) ); //! __AREA_RESOLUTION_METRIC__
	rStrings->push_back( pair<string,string>( string( "__AREA_RESOLUTION_DPI__" ),    to_string( areaResDPI )    ) ); //! __AREA_RESOLUTION_DPI__

	//! Volume
	double volumeDXYZ[3];
	bool numericErrorVolume = false;
	if( !getMeshVolumeDivergence( volumeDXYZ[0], volumeDXYZ[1], volumeDXYZ[2] ) ) {
		cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: estimateVolumeDivergence encountered an error - probably due to zero area faces!" << endl;
		numericErrorVolume = true;
	}
	double volumeAvg = ( volumeDXYZ[0] + volumeDXYZ[1] + volumeDXYZ[2] ) / 3.0;
	double volumeDev = abs( volumeDXYZ[0] - volumeAvg );
	if( volumeDev < abs( volumeDXYZ[1] - volumeAvg ) ) {
		volumeDev = abs( volumeDXYZ[1] - volumeAvg );
	}
	if( volumeDev < abs( volumeDXYZ[2] - volumeAvg ) ) {
		volumeDev = abs( volumeDXYZ[2] - volumeAvg );
	}
	volumeAvg = round( volumeAvg/1000.0 );
	volumeDev = round( volumeDev/1000.0 );
	//! __VOLUME_TOTAL__with or without deviation (for surface with holes)
	string strVolumeTotal = to_string( volumeAvg );
	if( numericErrorVolume ) {
		strVolumeTotal += " % CHECK due to numeric errors. This mesh may contain zero area faces. %";
	}
	rStrings->push_back( pair<string,string>( string( "__VOLUME_TOTAL__" ), strVolumeTotal ) );

	//! Enforce "." as decimal point i.e. override locale settings.
	for( pair<string,string>& rString : *rStrings ) {
		//string placeHolder = itStringPairs->first;
		if( rString.first == "__OBJECT_MATERIAL__" ) {
			continue; // Ignore text
		}
		if( rString.first == "__OBJECT_ID__" ) {
			continue; // Ignore text
		}
		string content = rString.second;
		size_t kommaPos = content.find_last_of( ',' );
		if( kommaPos != string::npos ) {
			content.replace( kommaPos, 1, 1, '.' );
			rString.second = content;
		}
		// Add +/- for volume, when deviation is >0
		if( ( rString.first == "__VOLUME_TOTAL__" ) && ( volumeDev > 0.0 ) ) {
			// Remove trailing zeros
			content.erase( content.find_last_not_of('0') + 1, std::string::npos );
			content.erase( content.find_last_not_of('.') + 1, std::string::npos );

			// Enforce "." for deviation
			std::string volumeDevText = to_string(volumeDev);
			kommaPos = volumeDevText.find_last_of( ',' );
			if( kommaPos != string::npos ) {
				volumeDevText.replace( kommaPos, 1, 1, '.' );
			}
			content += "}\\,\\pm\\numprint{" + volumeDevText;

			rString.second = content;
		}
		//! Remove trailing zeros after the decimal point.
		content.erase( content.find_last_not_of('0') + 1, std::string::npos );
		content.erase( content.find_last_not_of('.') + 1, std::string::npos );
		rString.second = content;
	}

	return( true );
}

// Debuging --------------------------------------------------------------------

//! Show mesh information as HTML.
//!
//! Based on Mesh::getMeshInfoData for string generation.
//! See also Mesh::dumpMeshInfo for plain text.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::showInfoMeshHTML() {
	MeshInfoData infoData;
    bool withSelfIntersection = false;
    if(!showQuestion(&withSelfIntersection, "Selfintersection Detection", "Do you want to calculate the number of self-intersecting faces?\n This could take several minutes!"))
        return false;


    if( !getMeshInfoData( infoData, true, withSelfIntersection ) ) {
		return( false );
	}
	std::string infoString;
	if( !infoData.getMeshInfoHTML( infoString ) ) {
		return( false );
	}
	showInformation( "Mesh Information", infoString );
	return true;

}

//! Fetch mesh information as Numbers.
//!
//! See Mesh::dumpMeshInfo for plain text.
//! See Mesh::getMeshInfoHTML for HTML.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::getMeshInfoData(
        MeshInfoData& rMeshInfos,
        const bool    rAbsolutePath,
        bool rWithSelfIntersectedFaces
) {
	// Initialize
	rMeshInfos.reset();

	// String data
	if( rAbsolutePath ) {
		rMeshInfos.mStrings[MeshInfoData::FILENAME]       = this->getFullName().string();
	} else {
		rMeshInfos.mStrings[MeshInfoData::FILENAME]       = this->getBaseName().string();
	}
	rMeshInfos.mStrings[MeshInfoData::MODEL_ID]           = this->getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
	rMeshInfos.mStrings[MeshInfoData::MODEL_MATERIAL]     = this->getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
	rMeshInfos.mStrings[MeshInfoData::MODEL_WEBREFERENCE] = this->getModelMetaDataRef().getModelMetaString( ModelMetaData::META_REFERENCE_WEB );

	// Primitive count
	rMeshInfos.mCountULong[MeshInfoData::VERTICES_TOTAL] = this->getVertexNr();
	rMeshInfos.mCountULong[MeshInfoData::FACES_TOTAL]    = this->getFaceNr();

	// Area and average resolution
	double areaAcq = 0.0;
	this->getFaceSurfSum( &areaAcq );
	rMeshInfos.mCountDouble[MeshInfoData::TOTAL_AREA] = std::round( areaAcq );
	rMeshInfos.mCountDouble[MeshInfoData::TOTAL_AREA] /= 100.0; // cm^2
	
	// Fetch Bounding Box
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_X]  = static_cast<double>( std::round( mMinX*10000.0 ) )/10000.0;
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Y]  = static_cast<double>( std::round( mMinY*10000.0 ) )/10000.0;
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_MIN_Z]  = static_cast<double>( std::round( mMinZ*10000.0 ) )/10000.0;
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_X]  = static_cast<double>( std::round( mMaxX*10000.0 ) )/10000.0;
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Y]  = static_cast<double>( std::round( mMaxY*10000.0 ) )/10000.0;
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_MAX_Z]  = static_cast<double>( std::round( mMaxZ*10000.0 ) )/10000.0;
	Vector3D bbDim;
	this->getBoundingBoxSize( bbDim );
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_WIDTH]  = static_cast<double>( std::round( bbDim.getX()*1.0 ) )/10.0; // cm
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_HEIGHT] = static_cast<double>( std::round( bbDim.getY()*1.0 ) )/10.0; // cm
	rMeshInfos.mCountDouble[MeshInfoData::BOUNDINGBOX_THICK]  = static_cast<double>( std::round( bbDim.getZ()*1.0 ) )/10.0; // cm

	// Volume
	double volDXYZ[3];
	this->getMeshVolumeDivergence( volDXYZ[0], volDXYZ[1], volDXYZ[2] );
	rMeshInfos.mCountDouble[MeshInfoData::TOTAL_VOLUME_DX] = volDXYZ[0];
	rMeshInfos.mCountDouble[MeshInfoData::TOTAL_VOLUME_DY] = volDXYZ[1];
	rMeshInfos.mCountDouble[MeshInfoData::TOTAL_VOLUME_DZ] = volDXYZ[2];

	double progressSteps = static_cast<double>( getVertexNr() + getFaceNr() );
	showProgressStart( "Mesh information" );

	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( currVertex->isNotANumber() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_NAN]++;
		}
		double vertexNormalLen = currVertex->getNormalLen();
		if( !isnormal( vertexNormalLen ) ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_NORMAL_LEN_NORMAL]++;
		}
		// Mesh structure:
		if( currVertex->isSolo() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_SOLO]++;
		}
		if( currVertex->isBorder() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_BORDER]++;
		}
		if( currVertex->isNonManifold() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_NONMANIFOLD]++;
		}
		if( currVertex->isDoubleCone() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_SINGULAR]++;
		}
		if( currVertex->isPartOfZeroFace() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_PART_OF_ZERO_FACE]++;
		}
		if( currVertex->isInverse() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_ON_INVERTED_EDGE]++;
		}
		// Special flags and conditions:
		if( currVertex->getFlag( FLAG_BELONGS_TO_POLYLINE ) ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_POLYLINE]++;
		}
		if( currVertex->getFlag( FLAG_SYNTHETIC ) ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_SYNTHETIC]++;
		}
		if( currVertex->getFlag( FLAG_MANUAL ) ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_MANUAL]++;
		}
		if( currVertex->getFlag( FLAG_CIRCLE_CENTER ) ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_CIRCLE_CENTER]++;
		}
		if( currVertex->getFlag( FLAG_SELECTED ) ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_SELECTED]++;
		}
		// Function value related:
		if( currVertex->isFuncValFinite() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_FUNCVAL_FINITE]++;
		}
		if( currVertex->isFuncValLocalMinimum() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MIN]++;
		}
		if( currVertex->isFuncValLocalMaximum() ) {
			rMeshInfos.mCountULong[MeshInfoData::VERTICES_FUNCVAL_LOCAL_MAX]++;
		}
		showProgress( static_cast<double>(vertIdx)/progressSteps, "Mesh information" );
	}

	// Set initial values
	rMeshInfos.mCountDouble[MeshInfoData::FACES_AREA_SMALLEST] = std::numeric_limits<double>::infinity();
	rMeshInfos.mCountDouble[MeshInfoData::FACES_AREA_LARGEST]  = 0.0;

	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		if( currFace->isBorder() ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_BORDER]++;
		}
		// Special border configurations
		unsigned int nrEdgesBorder = 0;
		currFace->hasBorderEdges( nrEdgesBorder );
		if( nrEdgesBorder == 3 ) { // Same as: if( currFace->isSolo() ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_SOLO]++;
		}
		unsigned int nrVerticesBorder = 0;
		currFace->hasBorderVertex( nrVerticesBorder );
		if( ( nrVerticesBorder == 3 ) && ( nrEdgesBorder == 0 ) ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_BORDER_BRDIGE_TRICONN]++;
		}
		if( ( nrVerticesBorder == 3 ) && ( nrEdgesBorder == 1 ) ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_BORDER_BRDIGE]++;
		}
		if( ( nrVerticesBorder == 3 ) && ( nrEdgesBorder == 2 ) ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_BORDER_DANGLING]++;
		}
		if( nrVerticesBorder == 3 ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_BORDER_THREE_VERTICES]++;
		}
		// ...
		if( currFace->isManifold() ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_MANIFOLD]++;
		}
		if( currFace->isNonManifold() ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_NONMANIFOLD]++;
		}
		if( currFace->getFlag( FLAG_FACE_STICKY ) ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_STICKY]++;
		}
		if( currFace->getFlag( FLAG_FACE_ZERO_AREA ) ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_ZEROAREA]++;
		}
		if( currFace->isInverse() ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_INVERTED]++;
		}
		unsigned int synthVerticesNr = _NOT_A_NUMBER_UINT_;
		currFace->hasSyntheticVertex( synthVerticesNr );
		if( synthVerticesNr >= 3 ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_WITH_SYNTH_VERTICES]++;
		}
		if( currFace->getFlag( FLAG_SELECTED ) ) {
			rMeshInfos.mCountULong[MeshInfoData::FACES_SELECTED]++;
		}
		// floating point properties
		double faceArea = currFace->getAreaNormal();
		if( faceArea < rMeshInfos.mCountDouble[MeshInfoData::FACES_AREA_SMALLEST] ) {
			rMeshInfos.mCountDouble[MeshInfoData::FACES_AREA_SMALLEST] = faceArea;
		}
		if( faceArea > rMeshInfos.mCountDouble[MeshInfoData::FACES_AREA_LARGEST] ) {
			rMeshInfos.mCountDouble[MeshInfoData::FACES_AREA_LARGEST] = faceArea;
		}
		showProgress( static_cast<double>( faceIdx+getVertexNr() )/progressSteps, "Mesh information" );
	}
    //detect self itersection
    if(rWithSelfIntersectedFaces){
        // Octree required - time consuming
        if( mOctree == nullptr ) {
            generateOctree( 1000 );
        }
        vector<Face*> intersectedFaces;
        mOctree->detectselfintersections(intersectedFaces);
        //delete duplicates
        sort(intersectedFaces.begin(),intersectedFaces.end());
        intersectedFaces.erase( unique( intersectedFaces.begin(),intersectedFaces.end()),intersectedFaces.end());
        rMeshInfos.mCountULong[MeshInfoData::FACES_SELFINTERSECTED] = intersectedFaces.size();
    }
    else{
        rMeshInfos.mCountULong[MeshInfoData::FACES_SELFINTERSECTED] = -1;
    }

	// Labeled connected components
	labelCount( Primitive::IS_VERTEX, rMeshInfos.mCountULong[MeshInfoData::CONNECTED_COMPONENTS] );

	// Done.
	showProgressStop( "Mesh information" );
	return( true );
}


//! Dump information about the mesh to stdout.
void Mesh::dumpMeshInfo( bool avoidSlow //!< discards some information requiring a lot of computing time.
    ) {

	unsigned int nanVertices         = 0;
	unsigned int soloVertices        = 0;
	unsigned int polyVertices        = 0;
	unsigned int borderVertices      = 0;
	unsigned int nonManifoldVertices = 0;
	unsigned int syntheticVertices   = 0;
	unsigned int selectedVertices    = 0;
	Vertex* currVertex;
	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		currVertex = getVertexPos( vertIdx );
		if( currVertex->isNotANumber() ) {
			nanVertices++;
		}
		//cout << "[Mesh::dumpMeshInfo] Vertex " << (*itVertex)->getIndexOriginal() << " test solo." << endl;
		if( currVertex->isSolo() ) {
			soloVertices++;
		}
		//cout << "[Mesh::dumpMeshInfo] Vertex " << (*itVertex)->getIndexOriginal() << " test border." << endl;
		if( currVertex->isBorder() ) {
			borderVertices++;
		}
		//cout << "[Mesh::dumpMeshInfo] Vertex " << (*itVertex)->getIndexOriginal() << " test manifold." << endl;
		if( currVertex->isNonManifold() ) {
			nonManifoldVertices++;
		}
		if( currVertex->getFlag( FLAG_BELONGS_TO_POLYLINE ) ) {
			polyVertices++;
		}
		if( currVertex->getFlag( FLAG_SYNTHETIC ) ) {
			syntheticVertices++;
		}
		if( currVertex->getFlag( FLAG_SELECTED ) ) {
			selectedVertices++;
		}
	}

	unsigned int soloFaces        = 0;
	unsigned int borderFaces      = 0;
	unsigned int manifoldFaces    = 0;
	unsigned int nonManifoldFaces = 0;
	unsigned int stickyFaces      = 0;
	unsigned int zeroAreaFaces    = 0;
	Face* currFace;
	for( uint64_t faceIdx=0; faceIdx<getFaceNr(); faceIdx++ ) {
		currFace = getFacePos( faceIdx );
		if( currFace->isSolo() ) {
			soloFaces++;
		}
		if( currFace->isBorder() ) {
			borderFaces++;
		}
		if( currFace->isManifold() ) {
			manifoldFaces++;
		}
		if( currFace->isNonManifold() ) {
			nonManifoldFaces++;
		}
		if( currFace->getFlag( FLAG_FACE_STICKY ) ) {
			stickyFaces++;
		}
		if( currFace->getFlag( FLAG_FACE_ZERO_AREA ) ) {
			zeroAreaFaces++;
		}
	}

	if( avoidSlow ) {
		cout << "[Mesh] ============================================= FAST ===" << endl;
	} else {
		cout << "[Mesh] ======================================= ALL / SLOW ===" << endl;
	}
	cout << "[Mesh] MESH ........ BBox X:              " << mMinX << " / " << mMaxX << endl;
	cout << "[Mesh]               BBox Y:              " << mMinY << " / " << mMaxY << endl;
	cout << "[Mesh]               BBox Z:              " << mMinZ << " / " << mMaxZ << endl;
	cout << "[Mesh] ......................................................" << endl;
	cout << "[Mesh]               BDim X:              " << mMaxX - mMinX << " mm (assumed)" << endl;
	cout << "[Mesh]               BDim Y:              " << mMaxY - mMinY << " mm (assumed)" << endl;
	cout << "[Mesh]               BDim Z:              " << mMaxZ - mMinZ << " mm (assumed)" << endl;
	cout << "[Mesh] ------------------------------------------------------" << endl;
	cout << "[Mesh] VERTICES .... total:               " << getVertexNr() << " * " << sizeof(Vertex) << " Byte = " << getVertexNr()*sizeof(Vertex)/(1024*1024) << " MBytes" << endl;
	cout << "[Mesh]               not a number:        " << nanVertices << endl; // this should be typically zero
	cout << "[Mesh]               solo:                " << soloVertices << endl; // this should be typically zero or equal to ...
	cout << "[Mesh]               belongs to polyline: " << polyVertices << " (are accounted as solo!) " << endl; // ... the number of vertices of the polylines
	cout << "[Mesh]               border:              " << borderVertices << endl;
	cout << "[Mesh]               non-manifold:        " << nonManifoldVertices << endl;
	cout << "[Mesh]               synthetic:           " << syntheticVertices << endl;
	cout << "[Mesh]               selected:            " << selectedVertices << endl;
	if( !avoidSlow ) {
		set<Vertex*> doubleCones;
		getDoubleCones( &doubleCones );
		cout << "[Mesh]               double cones:        " << doubleCones.size() << endl;
		doubleCones.clear();
	}
	cout << "[Mesh] ------------------------------------------------------" << endl;
	cout << "[Mesh] FACES ....... total:              " << getFaceNr() << " * " << sizeof(Face) << " Byte = " << getFaceNr()*sizeof(Face)/(1024*1024) << " MBytes" << endl;
	cout << "[Mesh]               solo:               " << soloFaces << endl;
	cout << "[Mesh]               border:             " << borderFaces << endl;
	cout << "[Mesh]               manifold:           " << manifoldFaces << endl;
	cout << "[Mesh]               non-manifold:       " << nonManifoldFaces << " (includes sticky faces per definition)" << endl;
	cout << "[Mesh]               sticky:             " << stickyFaces << "" << endl;
	cout << "[Mesh]               zero area:          " << zeroAreaFaces << "" << endl;
	cout << "[Mesh] ------------------------------------------------------" << endl;
	cout << "[Mesh] POLYLINES ... total:              " << getPolyLineNr() << endl;
	cout << "[Mesh] ======================================================" << endl;
}

//! Stub: Show information about the selection of a single primitive (SelPrim) as HTML.
//! @returns false in case of an error. True otherwise.
bool Mesh::showInfoSelectionHTML() {
	return false;
}

//! Stub: Show information about the function value as HTML.
//! @returns false in case of an error. True otherwise.
bool Mesh::showInfoFuncValHTML() {
	return false;
}

//! Stub: Show information about the labled vertices.
//!
//! @returns false in case of an error. True otherwise.
bool Mesh::showInfoLabelPropsHTML() {
	uint64_t nrOfLabels;
	if( !labelCount( Primitive::IS_VERTEX, nrOfLabels ) ) {
		showWarning( "Label properties", "No labels found." );
		return( false );
	}

	std::vector<double> labelArea;
	labelArea.resize( nrOfLabels+1, 0.0 );

	for( uint64_t vertIdx=0; vertIdx<getVertexNr(); vertIdx++ ) {
		Vertex* currVertex = getVertexPos( vertIdx );
		uint64_t currLabel = 0; // Zero is background
		currVertex->getLabel( currLabel );
		double oneRingAreaThird = currVertex->get1RingArea() / 3.0;
		labelArea.at( currLabel ) += oneRingAreaThird;
	}

	// Add volume - see: Mesh::estimateVolumeDivergence
	// process faces collect: volume and circumference, count closed labels
	// more complex: count number of border polylines -> topology!
	// PCA would be great as well

	std::string strCSV = "Label No.;Area;\n";
	for( uint64_t i=0; i<nrOfLabels; i++ ) {
		strCSV += to_string( i ) + ";";
		strCSV += to_string( labelArea.at( i ) );
		strCSV += "\n";
	}

	std::sort( labelArea.begin(), labelArea.end() );
	double labelAreaMin = labelArea.at( 0 );
	double labelAreaMax = labelArea.at( nrOfLabels );
	double labelAreaMedian = labelArea.at( nrOfLabels / 2 );
	double labelAreaPerc25 = labelArea.at( nrOfLabels / 4 );
	double labelAreaPerc75 = labelArea.at( nrOfLabels*3 / 4 );

	std::stringstream strHTML;
	strHTML << "<body>" << std::endl;
	strHTML << "Number of labels: " << to_string( nrOfLabels ) << "<br />" << std::endl;
	// Table
	strHTML << "<table border=\"0\">" << std::endl;
	strHTML << "<tr>" << std::endl <<
	           "<td>Area min:</td><td align=\"right\">"          + to_string( labelAreaMin )    + "</td>" << std::endl <<
	           "</tr><tr>" << std::endl <<
	           "<td>Area 25 percentil:</td><td align=\"right\">" + to_string( labelAreaPerc25 ) + "</td>" << std::endl <<
	           "</tr><tr>" << std::endl <<
	           "<td>Area median:</td><td align=\"right\">"       + to_string( labelAreaMedian ) + "</td>" << std::endl <<
	           "</tr><tr>" << std::endl <<
	           "<td>Area 75 percentil:</td><td align=\"right\">" + to_string( labelAreaPerc75 ) + "</td>" << std::endl <<
	           "</tr><tr>" << std::endl <<
	           "<td>Area max:</td><td align=\"right\">"          + to_string( labelAreaMax )    + "</td>" << std::endl <<
	           "</tr>" << std::endl <<
	           "</table>" << std::endl;
	strHTML << "</body>" << std::endl;

	showInformation( "Label properties", strHTML.str(), strCSV );
	return( true );
}

//! Stub: Show information about the (cone) axis.
//! @returns false in case of an error. True otherwise.
bool Mesh::showInfoAxisHTML() {
	return( false );
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Add a position to the selection.
//! @returns false in case of an error.
bool Mesh::addSelectedPosition( Vector3D rPos, Primitive* rPrimFrom, bool rLast ) {
	mSelectedPositions.emplace_back( tuple<Vector3D,Primitive*,bool>( rPos, rPrimFrom, rLast ) );
	selectedMPositionsChanged();
	return true;
}

//! Add positions to the given vector.
//! @returns false in case of an error.
bool Mesh::getSelectedPosition( vector<std::tuple<Vector3D, Primitive*, bool>>* rVec ) {
	*rVec = mSelectedPositions;
	return true;
}

//! Add positions to the given vector for rendering as line segments.
//! @returns false in case of an error.
bool Mesh::getSelectedPositionLines(std::vector<std::tuple<Vector3D, Primitive *> > *rVec ) {
	Vector3D lastPos( _NOT_A_NUMBER_DBL_ );
	Primitive* lastPrim = nullptr;
	rVec->reserve(mSelectedPositions.size() * 2);
	for( auto const& currPosTuple: mSelectedPositions ) {
		Vector3D currPos = get<0>( currPosTuple );
		Primitive* currPrim = get<1> (currPosTuple);
		if( isnormal( lastPos.getLength3() ) ) {
			rVec->push_back( std::tuple<Vector3D, Primitive*>(lastPos, lastPrim) );
			rVec->push_back( std::tuple<Vector3D, Primitive*>(currPos, currPrim) );
		}
		lastPos = currPos;
		lastPrim = currPrim;
		if( get<2>( currPosTuple ) ) { // End of a line strip.
			lastPos.set( _NOT_A_NUMBER_DBL_ );
			lastPrim = nullptr;
		}
	}
	return true;
}

//! Computes all distances between SelPrims and returns them
//! as one vector.
//!
//! Note: all distances of all selected segments are concatenated.
//!
//! @returns false in case of an error.
bool Mesh::getSelectedPositionDistancesAll( std::vector<double>& rAllDistances ) {
	bool retVal{true};
	Vector3D lastPos( _NOT_A_NUMBER_DBL_ ); //= get<0>(mSelectedPositions.front());
	for( auto const& currPos : mSelectedPositions ) {
		double dist = ( get<0>(currPos) - lastPos ).getLength3();
		if( isnormal( dist ) ) {
			rAllDistances.push_back( dist );
		}
		lastPos = get<0>(currPos);
		// Do not compute distances between line segments
		bool endOfLine = get<2>(currPos);
		if( endOfLine ) {
			lastPos = Vector3D( _NOT_A_NUMBER_DBL_ );
		}
	}
	return( retVal );
}

//! Computes all distances between SelPrims and returns them
//! as HTML text.
//!
//! @returns false in case of an error.
bool Mesh::getSelectedPositionDistancesHTML( std::string &rDistanceText ) {
	if( mSelectedPositions.size() == 0 ) {
		rDistanceText = "No positions (SelPos) were selected.";
		return( true );
	}
	std::string distanceInfoStr = "<ol><li>";
	Vector3D lastPos( _NOT_A_NUMBER_DBL_ );
	bool endOfLine{false};
	double sumDist = 0.0;
	for( auto const& currPos : mSelectedPositions ) {
		if( endOfLine ) {
			distanceInfoStr += "&Sigma;&nbsp;<b>" + to_string( sumDist ) + "</li>\n<li>";
			sumDist = 0.0;
			lastPos = Vector3D( _NOT_A_NUMBER_DBL_ );
		}
		double dist = ( get<0>(currPos) - lastPos ).getLength3();
		if( isnormal( dist ) ) {
			distanceInfoStr += to_string( dist ) + "&nbsp;";
			sumDist += dist;
		}
		lastPos = get<0>(currPos);
		endOfLine = get<2>(currPos);
	}
	distanceInfoStr += "&Sigma;&nbsp;<b>" + to_string( sumDist ) + "</li>\n</ol>";
	rDistanceText.clear();
	rDistanceText = distanceInfoStr;
	return( true );
}

//! Adds vertices to the given vector using triplets of positions to compute circumscribed circles.
//! The Vertex properties are set as follows:
//!     Position ............... Center of the circle.
//!     Normal ................. Normal of the plane defined by the triplet.
//!     Length of the normal ... Angle of the arc defined by the triplet i.e. confidence. (TODO!)
//!     Function value ......... Radius of the circle.
//!     Label Id ............... Corresponds to the line segment of the positions (TODO!)
//! @returns false in case of an error. True otherwise.
bool Mesh::getSelectedPositionCircleCenters( vector<Vertex*>* rCenterVertices ) {
	// Sanity check
	if( rCenterVertices == nullptr ) {
		return false;
	}

	deque<Vector3D> posABC;
	for( auto const& currPosTuple: mSelectedPositions ) {
		posABC.push_back( get<0>( currPosTuple ) );
		if( posABC.size() == 3  ) {
			// Compute circle
			LOG::info() << "[Mesh::" << __FUNCTION__ << "] Compute circle using:\n";
			LOG::info() << "[Mesh::" << __FUNCTION__ << "]      " << posABC.at(0) << "\n";
			LOG::info() << "[Mesh::" << __FUNCTION__ << "]      " << posABC.at(1) << "\n";
			LOG::info() << "[Mesh::" << __FUNCTION__ << "]      " << posABC.at(2) << "\n";
			// Plane of the circle:
			Plane circlePlane( posABC.at(0), posABC.at(1), posABC.at(2) );
			Vector3D normalPlane = circlePlane.getNormal( true );
			Vector3D centerOfGravity = circlePlane.getCenterOfGravity();

			//! \todo CHECK: circlePlane.isValid()
			// Transformation matrices for shift and rotat to XY-plane:
			Matrix4D baseChange;
			baseChange.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Z, &centerOfGravity, &normalPlane );

			// compute:
			Vector3D posA2 = posABC.at(0) * baseChange;
			Vector3D posB2 = posABC.at(1) * baseChange;
			Vector3D posC2 = posABC.at(2) * baseChange;
			LOG::info() << "[Mesh::" << __FUNCTION__ << "] Compute circle in 2D using: " << "\n";
			LOG::info() << "[Mesh::" << __FUNCTION__ << "]      " << posA2 << "\n";
			LOG::info() << "[Mesh::" << __FUNCTION__ << "]      " << posB2 << "\n";
			LOG::info() << "[Mesh::" << __FUNCTION__ << "]      " << posC2 << "\n";

			baseChange.invert();

			double d = 2.0*( posA2.getX()*( posB2.getY()-posC2.getY() ) +
			                 posB2.getX()*( posC2.getY()-posA2.getY() ) +
			                 posC2.getX()*( posA2.getY()-posB2.getY() )
			               );
			double a = pow( posA2.getX(), 2 ) + pow( posA2.getY(), 2 );
			double b = pow( posB2.getX(), 2 ) + pow( posB2.getY(), 2 );
			double c = pow( posC2.getX(), 2 ) + pow( posC2.getY(), 2 );
			double xu = ( a*( posB2.getY()-posC2.getY() ) + b*( posC2.getY()-posA2.getY() ) + c*( posA2.getY()-posB2.getY() ) )/d;
			double yu = ( a*( posC2.getX()-posB2.getX() ) + b*( posA2.getX()-posC2.getX() ) + c*( posB2.getX()-posA2.getX() ) )/d;
			Vector3D circleCenter( xu, yu, 0.0, 1.0 );

			LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Center in 2D: " << circleCenter << "\n";
			circleCenter *= baseChange;
			LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Center in 3D: " << circleCenter << "\n";

			// Radius in 3D:
			double rA3 = ( posABC.at(0) - circleCenter ).getLength3();
			double rB3 = ( posABC.at(1) - circleCenter ).getLength3();
			double rC3 = ( posABC.at(2) - circleCenter ).getLength3();
			LOG::debug() << "[Mesh::" << __FUNCTION__ << "] Radius: " << rA3 << " "  << rB3 << " "  << rC3 << "\n";

			// Add new center
			Vertex* centerVertex = new Vertex( circleCenter );
			centerVertex->setFlag( FLAG_SYNTHETIC | FLAG_CIRCLE_CENTER);
			centerVertex->setFuncValue( rA3 );
			centerVertex->setNormal( &normalPlane );
			rCenterVertices->push_back( centerVertex );

			// Remove first element i.e shift
			posABC.pop_front();
			// End of line strip:
			if( get<2>( currPosTuple ) ) {
				posABC.clear();
			}
			LOG::info() << "[Mesh::" << __FUNCTION__ << "] .................................\n";
		}
	}
	LOG::info() << "[Mesh::" << __FUNCTION__ << "] DONE.\n";
	return true;
}

//! Stub for (graphical) user interaction.
//! @returns true, when the user wants to compute an axis from the circle centers. False otherwise.
bool Mesh::getAxisFromCircleCenters() {
	// It is stub and there is no graphical user interaction:
	return false;
}

//! Use vertices tagged as circle centers to estimate an axis.
//! @returns false in case of an error. True otherwise.
bool Mesh::getAxisFromCircleCenters(
		Vector3D& rTop,
		Vector3D& rBottom
) {
	// Fetch circle centers
	set<Vertex*> verticesCircleCenters;
	if( !getVertWithFlag( &verticesCircleCenters, FLAG_CIRCLE_CENTER ) ) {
		return( false );
	}
	// Stop if there is nothing to do.
	if( verticesCircleCenters.empty()) {
		std::cout << "[Mesh::" << __FUNCTION__ << "] No circle centers present - DONE." << std::endl;
		return( true );
	}

	// Compute average orientation using the (weighted) normals.
	Vector3D avgOrientation;
	avgOrientation.setH( 0.0 ); // Directional vector
	for( auto const& currCircle: verticesCircleCenters ) {
		Vector3D currOrient = currCircle->getNormal( false );
		avgOrientation += currOrient;
	}
	avgOrientation.normalize3();
	std::cout << "[Mesh::" << __FUNCTION__ << "] Average orientation: " << avgOrientation << std::endl;

	// Compute average position.
	Vector3D originPos( 0.0, 0.0, 0.0, 1.0 );
	Matrix4D baseChange;
	baseChange.setBaseChange( Matrix4D::SET_NEW_BASE_TO_Z, &originPos, &avgOrientation );
	Vector3D avgPosition( 0.0 );
	for( auto const& currCircle: verticesCircleCenters ) {
		Vector3D currOrient = currCircle->getNormal( false );
		double weight = currOrient.getLength3();
		// Transform into axis coordinate system:
		Vector3D posInAxisOrient = baseChange * currCircle->getCenterOfGravity();
		posInAxisOrient.normalizeW();
		// Shift down to xy-plane:
		posInAxisOrient.setZ( 0.0 );
		avgPosition += ( weight * posInAxisOrient );
	}
	std::cout << "[Mesh::" << __FUNCTION__ << "] Average position in Axis: " << avgPosition << std::endl;
	avgPosition.normalizeW();
	std::cout << "[Mesh::" << __FUNCTION__ << "] Average position in Axis normalized: " << avgPosition << std::endl;
	// Transform from axis coordinates to world coordinates.
	baseChange.invert();
	avgPosition *= baseChange;
	std::cout << "[Mesh::" << __FUNCTION__ << "] Average position: " << avgPosition << std::endl;

//	This was a bad idea, because the axis can be very well outside the bounding sphere:
//	However, the code might be usefull for other tasks.
//------------------------------------------------------------------------------
//	// Compute top and bottom points by intersecting the axis with the bounding sphere.
//	Vector3D bbCenter = getBoundingBoxCenter();
//	double   bbRadius = getBoundingBoxRadius();
//	// Line-Sphere intersection -- see also: https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
//	double b = 2.0*( avgOrientation * ( avgPosition - bbCenter ) );
//	double c = ( avgPosition - bbCenter ) * ( avgPosition - bbCenter ) - pow( bbRadius, 2.0 );
//	double d1 = -b + sqrt( pow( b, 2.0 ) - c );
//	double d2 = -b - sqrt( pow( b, 2.0 ) - c );

	Plane planeOrthoToAxis( avgPosition, avgOrientation );
	double distMin, distMax;
	getDistanceToPlaneMinMax( planeOrthoToAxis, distMin, distMax, false );

	// Distance may be flipped!
	if( abs(distMin) > abs(distMax) ) {
		rTop    = avgPosition - distMax * avgOrientation;
		rBottom = avgPosition - distMin * avgOrientation;
	} else {
		rTop    = avgPosition + distMax * avgOrientation;
		rBottom = avgPosition + distMin * avgOrientation;
	}

	std::cout << "[Mesh::" << __FUNCTION__ << "] DONE." << std::endl;
    return( true );
}
//! Use selected Vertices to fit an ellipse.
//! The first 3 Vertices describe a plane
//! A axis is calculated with the normal of this plane and the center of the ellipse
//! @returns false in case of an error. True otherwise.
bool Mesh::getAxisFromEllipseFit()
{
    //get the first 3 vertices of the set to describe a plan
    vector<Vertex*> planeVertices;
    for( auto const& selVertex : mSelectedMVerts ) {
        planeVertices.push_back(selVertex);
    }
    //create plane of the ellipse
    Plane plane = Plane(planeVertices[0]->getPositionVector(),planeVertices[1]->getPositionVector(),planeVertices[2]->getPositionVector());

    //projection of all selected Vertices to one plane
    vector<pair<double,double> > ellipseCandidatePoints;
    for( auto const& selVertex : mSelectedMVerts ) {
        Vector3D selPoint = selVertex->getPositionVector();
        selPoint.projectOntoPlane(plane.getHNF());
        double xPos2 = selPoint.getX();
        double yPos2 = selPoint.getZ(); //changed to Z because the Y axis points upwards
        ellipseCandidatePoints.emplace_back( std::make_pair(xPos2,yPos2) );
    }

    EllipseDisc ellipse;
    ellipse.findEllipseParams( EllipseDisc::CONIC, ellipseCandidatePoints );
    ellipse.dumpInfo();

    //create a center vertex
    std::vector<Vertex*> centerVertices; //needed as vector for the insertVertices function
    double planeY = plane.getY();
    Vector3D normalPlane = plane.getHNF();
    Vector3D ellipseCenter(ellipse.mCenterX,planeY,ellipse.mCenterY); //Y and Z changed before
    Vertex* centerVertex = new Vertex( ellipseCenter );
    centerVertex->setFlag( FLAG_SYNTHETIC | FLAG_CIRCLE_CENTER);
    centerVertex->setNormal( &normalPlane );

    // add the synthetic vertex
    centerVertices.push_back(centerVertex);
    insertVertices( &centerVertices );

    //create and show the axis
    Vector3D topPoint;
    Vector3D bottomPoint;
    getAxisFromCircleCenters( topPoint, bottomPoint );
    setConeAxis( &topPoint, &bottomPoint );


    return( true );
}

//! Getter Method of the protected attribute mVertices as set
std::set<Vertex*> Mesh::getVertices(){
    return std::set<Vertex*>(mVertices.begin(), mVertices.end());
    //return mSelectedMVerts;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------
