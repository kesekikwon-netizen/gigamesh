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

#include <string>
#include <thread>
#include <future>
#include <mutex>
#include <functional>

#include <ctime>
#include <cstdio>
#include <cstdlib> // calloc

#ifdef _MSC_VER // windows version for getopt
#include "getoptwin.h"
#elif defined(__MINGW32__) || defined(__MINGW64__)
#include "getoptwin.h"
#else
#include <getopt.h>
#endif

#include <GigaMesh/printbuildinfo.h>
#include <GigaMesh/getuserandhostname.h>
#include <GigaMesh/mesh/compfeaturevecs.h>

//#include "voxelcuboid.h"
#include <GigaMesh/mesh/voxelfilter25d.h>

#include <sys/stat.h> // statistics for files
#include <GigaMesh/logging/Logging.h>


#define _DEFAULT_FEATUREGEN_RADIUS_       1.0
#define _DEFAULT_FEATUREGEN_XYZDIM_       256
#define _DEFAULT_FEATUREGEN_RADIICOUNT_   4 // power of 2


//! Process one file with the given paramters.
bool generateFeatureVectors(
                const std::filesystem::path&   fileNameIn,
                const std::filesystem::path&   rOptFileSuffix,
                double       radius,
                unsigned int xyzDim,
                unsigned int radiiCount,
                bool         replaceFiles,
                bool                           rNoVolumeIntInv,
                bool                           rNoAreaIntInv,
                bool                           rNoNormalsFile,
                bool                           rConcatResults,
                const std::string&             rHostname,
                const std::string&             rUsername
) {
	// Check existance of the input file:
	if( !std::filesystem::exists( fileNameIn ) ) {
		std::cerr << "[GigaMesh] ERROR: File '" << fileNameIn << "' not found!" << std::endl;
		return( false );
	}

	// Check extension:
	if( !fileNameIn.has_extension() ) {
		std::cerr << "[GigaMesh] ERROR: No extension/type for the given input file '" << fileNameIn << "' specified!" << std::endl;
		return( false );
	}

    //get absolute path, is necessary to save the texture paths correct in the new PLY File
    std::filesystem::path absoluteInPath = std::filesystem::absolute( fileNameIn );

	// Fileprefix for output
    std::filesystem::path fileNameOut( absoluteInPath );
	fileNameOut.replace_extension( "" );

	// Prepare suffix for the output file
	char tmpBuffer[512];
	sprintf( tmpBuffer, "_r%0.2f_n%i_v%i", radius, radiiCount, xyzDim );
	fileNameOut += rOptFileSuffix;
	fileNameOut += std::string( tmpBuffer );

	// Check: Output file for normal used to rotate the local patch
	std::filesystem::path fileNameOutPatchNormal( fileNameOut );
	if( rNoNormalsFile ) {
		fileNameOutPatchNormal.clear();
	} else {
		fileNameOutPatchNormal += ".normal.mat";
		if( std::filesystem::exists(fileNameOutPatchNormal) ) {
			if( !replaceFiles ) {
				std::cerr << "[GigaMesh] File '" << fileNameOutPatchNormal << "' already exists!" << std::endl;
				return( false );
			}
			std::cerr << "[GigaMesh] Warning: File '" << fileNameOutPatchNormal << "' will be replaced!" << std::endl;
		}
	}

	// Output file for volume descriptor (1st integral invariant)
	std::filesystem::path fileNameOutVol;
	if( !rNoVolumeIntInv ) {
		// Check: Output file for volume descriptor
		fileNameOutVol = fileNameOut;
		fileNameOutVol += ".volume.mat";
		if( std::filesystem::exists( fileNameOutVol ) ) {
			if( !replaceFiles ) {
				std::cerr << "[GigaMesh] File '" << fileNameOutVol << "' already exists!" << std::endl;
				return( false );
			}
			std::cerr << "[GigaMesh] Warning: File '" << fileNameOutVol << "' will be replaced!" << std::endl;
		}
	}

	// Output for the concatenated results (1st and 2nd integral invariant)
	std::filesystem::path fileNameOutVS;
	if( !rNoVolumeIntInv & !rNoAreaIntInv & rConcatResults ) {
		// Check: Output file for volume AND surface descriptor
		fileNameOutVS = fileNameOut;
		fileNameOutVS += ".vs.mat";
		if( std::filesystem::exists(fileNameOutVS)) {
			if( !replaceFiles ) {
				std::cerr << "[GigaMesh] File '" << fileNameOutVS << "' already exists!" << std::endl;
				return( false );
			}
			std::cerr << "[GigaMesh] Warning: File '" << fileNameOutVS << "' will be replaced!" << std::endl;
		}
	}

	// Output file for surface descriptor (2nd integral invariant)
	std::filesystem::path fileNameOutSurf( fileNameOut );
	if( !rNoAreaIntInv ) {
		fileNameOutSurf += ".surface.mat";
		if( std::filesystem::exists( fileNameOutSurf ) ) {
			if( !replaceFiles ) {
				std::cerr << "[GigaMesh] File '" << fileNameOutSurf << "' already exists!" << std::endl;
				return( false );
			}
			std::cerr << "[GigaMesh] Warning: File '" << fileNameOutSurf << "' will be replaced!" << std::endl;
		}
	}

	// Output file for the technical meta-data
	std::filesystem::path fileNameOutMeta( fileNameOut );
	fileNameOutMeta += ".info.txt";
	if( std::filesystem::exists( fileNameOutMeta ) ) {
		if( !replaceFiles ) {
			std::cerr << "[GigaMesh] File '" << fileNameOutMeta << "' already exists!" << std::endl;
			return( false );
		}
		std::cerr << "[GigaMesh] Warning: File '" << fileNameOutMeta << "' will be replaced!" << std::endl;
	}

	// Output file for 3D data including the (1st) volumetric feature vectors.
	std::filesystem::path fileNameOut3D( fileNameOut );
	if( rNoVolumeIntInv ) {
		fileNameOut3D += ".surface";
	} else {
		fileNameOut3D += ".volume";
	}
	fileNameOut3D += ".ply";
	if( std::filesystem::exists( fileNameOut3D )  ) {
		if( !replaceFiles ) {
			std::cerr << "[GigaMesh] File '" << fileNameOut3D << "' already exists!" << std::endl;
			return( false );
		}
		std::cerr << "[GigaMesh] Warning: File '" << fileNameOut3D << "' will be replaced!" << std::endl;
	}

	// All parameters OK => infos to stdout and file with metadata  -----------------------------------------------------------
	std::fstream fileStrOutMeta;
	fileStrOutMeta.open( fileNameOutMeta, std::fstream::out );

	// Set the formatting properties of the output
	std::cout << std::setprecision( 2 ) << std::fixed;
	fileStrOutMeta << std::setprecision( 2 ) << std::fixed;

	time_t rawtime;

	time( &rawtime );
	struct tm * timeInfoMeshLoad = localtime( &rawtime );
	time_t timeStampMeshLoad = time( nullptr ); // clock() is not multi-threading save (to measure the non-CPU or real time ;) )
	int timeLoaded{-1};

	// Prepare data structures
	//--------------------------------------------------------------------------
	bool readSucess;
	Mesh someMesh( fileNameIn, readSucess );
	if( !readSucess ) {
		std::cerr << "[GigaMesh] Error: Could not open file '" << fileNameIn << "'!" << std::endl;
		return( false );
	}

	timeLoaded = static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampMeshLoad );

	// Fetch mesh data
	Vector3D bbDim;
	someMesh.getBoundingBoxSize( bbDim );
	const double bbWdith{static_cast<double>(std::round( bbDim.getX()*1.0 )) / 10.0};
	const double bbHeight{static_cast<double>(std::round( bbDim.getY()*1.0 )) / 10.0};
	const double bbThick{static_cast<double>(std::round( bbDim.getZ()*1.0 )) / 10.0};
	// Area and average resolution
	double areaAcq{0};
	someMesh.getFaceSurfSum( &areaAcq );
	areaAcq = round( areaAcq );
	double volDXYZ[3]{ 0.0, 0.0, 0.0 };
	someMesh.getMeshVolumeDivergence( volDXYZ[0], volDXYZ[1], volDXYZ[2] );
	std::string modelID = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
	std::string modelMat = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
	std::string modelWebRef = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_REFERENCE_WEB );
	// Write data to console
#ifdef VERSION_PACKAGE
	std::cout << "[GigaMesh] Version:         " << VERSION_PACKAGE << std::endl;
#else
	std::cout << "[GigaMesh] Version:         unknown" << std::endl;
#endif
	std::cout << "[GigaMesh] ==================================================" << std::endl;
	std::cout << "[GigaMesh] File IN:         " << fileNameIn << std::endl;
	std::cout << "[GigaMesh] File OUT/Prefix: " << fileNameOut << std::endl;
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;
	std::cout << "[GigaMesh] Model ID:        " << modelID << std::endl;
	std::cout << "[GigaMesh] Material:        " << modelMat << std::endl;
	std::cout << "[GigaMesh] Web-reference:   " << modelWebRef << std::endl;
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;
	std::cout << "[GigaMesh] Vertices:        " << someMesh.getVertexNr() << "" << std::endl;
	std::cout << "[GigaMesh] Faces:           " << someMesh.getFaceNr() << "" << std::endl;
	std::cout << "[GigaMesh] Bounding Box:    " << bbWdith << " x " << bbHeight << " x " << bbThick << " cm" << std::endl;
	std::cout << "[GigaMesh] Area:            " << areaAcq/100.0 << " cm^2" << std::endl;
	std::cout << "[GigaMesh] Volume (dx):     " << volDXYZ[0]/1000.0 << " cm^3" << std::endl;
	std::cout << "[GigaMesh] Volume (dy):     " << volDXYZ[1]/1000.0 << " cm^3" << std::endl;
	std::cout << "[GigaMesh] Volume (dz):     " << volDXYZ[2]/1000.0 << " cm^3" << std::endl;
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;
	std::cout << "[GigaMesh] Radius:          " << radius << " mm (unit assumed)" << std::endl;
	std::cout << "[GigaMesh] Radii:           2^" << radiiCount << " = " << pow( 2.0, static_cast<double>(radiiCount) ) << std::endl;
	std::cout << "[GigaMesh] Rastersize:      " << xyzDim << "^3" << std::endl;
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;

	// Write technical meta-data to file
#ifdef VERSION_PACKAGE
	fileStrOutMeta << "GigaMesh Version:   " << VERSION_PACKAGE << std::endl;
#else
	fileStrOutMeta << "GigaMesh Version:   unknown" << std::endl;
#endif
	fileStrOutMeta << "Path INPUT:         " << fileNameIn.parent_path() << std::endl;
	fileStrOutMeta << "File INPUT:         " << fileNameIn.filename() << std::endl;
	fileStrOutMeta << "Model ID:           " << modelID << std::endl;
	fileStrOutMeta << "Material:           " << modelMat << std::endl;
	fileStrOutMeta << "Web-reference:      " << modelWebRef << std::endl;
	fileStrOutMeta << "Vertices:           " << someMesh.getVertexNr() << "" << std::endl;
	fileStrOutMeta << "Faces:              " << someMesh.getFaceNr() << "" << std::endl;
	fileStrOutMeta << "Bounding Box:       " << bbWdith << " x " << bbHeight << " x " << bbThick << " cm" << std::endl;
	fileStrOutMeta << "Area:               " << areaAcq/100.0 << " cm^2" << std::endl;
	fileStrOutMeta << "Volume (dx):        " << volDXYZ[0]/1000.0 << " cm^3" << std::endl;
	fileStrOutMeta << "Volume (dy):        " << volDXYZ[1]/1000.0 << " cm^3" << std::endl;
	fileStrOutMeta << "Volume (dz):        " << volDXYZ[2]/1000.0 << " cm^3" << std::endl;
	// Output & Parameter
	fileStrOutMeta << "Path OUTPUT:        " << fileNameOut.parent_path() << std::endl;
	fileStrOutMeta << "File OUTPUT-Prefix: " << fileNameOut.filename() << std::endl;

	// Pre-compute relative radii:
	uint64_t multiscaleRadiiSize = std::pow( 2.0, static_cast<double>(radiiCount) );
	double*       multiscaleRadii     = new double[multiscaleRadiiSize];
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		multiscaleRadii[i] = 1.0 - static_cast<double>(i) /
		                           static_cast<double>(multiscaleRadiiSize);
	}
	fileStrOutMeta << "Radius:             " << radius << " mm (unit assumed)" << std::endl;
	fileStrOutMeta << "Radii:              2^" << radiiCount << " = " << std::pow( 2.0, static_cast<float>(radiiCount) ) << std::endl;
	std::cout << "[GigaMesh] Radii: (relative)          ";
	fileStrOutMeta << "Radii (relative):  ";
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		std::cout << " " << multiscaleRadii[i];
		fileStrOutMeta << " " << multiscaleRadii[i];
	}
	std::cout << std::endl;
	fileStrOutMeta << std::endl;
	fileStrOutMeta << "Radii (absolute):  ";
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		fileStrOutMeta << " " << ( multiscaleRadii[i] * radius );
	}
	fileStrOutMeta << std::endl;
	fileStrOutMeta << "Rastersize:         " << xyzDim << "^3" << std::endl;

	// Info about files
	std::cout << "[GigaMesh] File to write technical Meta-Data:   " << fileNameOutMeta << std::endl;
	if( !fileNameOutVol.empty() ) {
		std::cout << "[GigaMesh] File to write Volume (1st MSII):     " << fileNameOutVol << std::endl;
	}
	if( !fileNameOutSurf.empty() ) {
		std::cout << "[GigaMesh] File to write Surface (2nd MSII):    " << fileNameOutSurf << std::endl;
	}
	if( !fileNameOutPatchNormal.empty() ) {
		std::cout << "[GigaMesh] File to write Surface patch normal:  " << fileNameOutPatchNormal << std::endl;
	}
	if( !fileNameOutVS.empty() ) {
		std::cout << "[GigaMesh] File to write concatenated V+S:      " << fileNameOutVS << std::endl;
	}

	struct tm * timeinfo;

	// Header for the .mat files
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
	strHeader << "# | Mesh:       " << fileNameIn.filename() << std::endl;
	strHeader << "# | - Vertices: " << someMesh.getVertexNr() << std::endl;
	strHeader << "# | - Faces:    " << someMesh.getFaceNr() << std::endl;
	strHeader << "# | Timestamp:  " << timeInfoStr << std::endl;
	strHeader << "# +-------------------------------------------------------------------------------+" << std::endl;

	// Prepare array for (1st) volume integral invariant filter responses
	double* descriptVolume{nullptr};
	if( !rNoVolumeIntInv ) {
		descriptVolume  = new double[someMesh.getVertexNr()*multiscaleRadiiSize];
	}

	// Prepare array for (2nd) surface patch integral invariant filter responses
	double* descriptSurface{nullptr};
	if( !rNoAreaIntInv ) {
		descriptSurface = new double[someMesh.getVertexNr()*multiscaleRadiiSize];
	}

	// Initialize array, when required=allocated.
	if( descriptVolume != NULL ) {
		for( uint i=0; i<someMesh.getVertexNr()*multiscaleRadiiSize; i++ ) {
			descriptVolume[i]  = _NOT_A_NUMBER_DBL_;
		}
	}
	if( descriptSurface != NULL ) {
		for( uint i=0; i<someMesh.getVertexNr()*multiscaleRadiiSize; i++ ) {
			descriptSurface[i] = _NOT_A_NUMBER_DBL_;
		}
	}

	// Prepare normals
	std::vector<MeshIO::grVector3ID> patchNormalsToAssign;
	patchNormalsToAssign.resize( someMesh.getVertexNr() );

	// Determine number of threads using CPU cores minus one.
	const unsigned int availableConcurrentThreads =  std::thread::hardware_concurrency() - 1;
	std::cout << "[GigaMesh] Computing feature vectors using "
	            << availableConcurrentThreads << " threads" << std::endl;

	// Meta-data continued
	if( rNoVolumeIntInv ) {
		fileStrOutMeta << "Volume integral:    No" << std::endl;
	} else {
		fileStrOutMeta << "Volume integral:    Yes" << std::endl;
	}
	if( rNoAreaIntInv ) {
		fileStrOutMeta << "Area integral:      No" << std::endl;
	} else {
		fileStrOutMeta << "Area integral:      Yes" << std::endl;
	}
	fileStrOutMeta << "Hostname:           " << rHostname << std::endl;
	fileStrOutMeta << "Username:           " << rUsername << std::endl;
	fileStrOutMeta << "Mesh loaded:        " << asctime( timeInfoMeshLoad ); // no endl required as asctime will add a linebreak
	fileStrOutMeta << "Load walltime:      " << timeLoaded << " seconds" << std::endl;
	fileStrOutMeta << "Number of threads:  " << availableConcurrentThreads << std::endl;

	// +++ Collect time for parallel processing
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	time_t timeStampParallel = time( nullptr ); // clock() is not multi-threading save (to measure the non-CPU or real time ;) )
	fileStrOutMeta << "Compute start:      " << asctime( timeinfo ); // no endl required as asctime will add a linebreak
	// --- Collect time for parallel processing

	// Pre-compute sparse filte:
	voxelFilter2DElements* sparseFilters;
	generateVoxelFilters2D( multiscaleRadiiSize, multiscaleRadii, xyzDim, &sparseFilters );

	sMeshDataStruct* setMeshData = new sMeshDataStruct[availableConcurrentThreads];
	for( size_t t = 0; t < availableConcurrentThreads; t++ )
	{
		setMeshData[t].threadID               = t;
		setMeshData[t].meshToAnalyze          = &someMesh;
		setMeshData[t].radius                 = radius;
		setMeshData[t].xyzDim                 = xyzDim;
		setMeshData[t].multiscaleRadiiSize    = multiscaleRadiiSize;
		setMeshData[t].multiscaleRadii        = multiscaleRadii;
		setMeshData[t].sparseFilters          = &sparseFilters;
		setMeshData[t].mPatchNormal           = &patchNormalsToAssign;
		setMeshData[t].descriptVolume         = descriptVolume;
		setMeshData[t].descriptSurface        = descriptSurface;
	}

	compFeatureVectorsMain( setMeshData, availableConcurrentThreads );

	delete[] setMeshData;

	// +++ Collect time for parallel processing
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	fileStrOutMeta << "Compute end:        " << asctime( timeinfo ); // no endl required as asctime will add a linebreak
	// ... Collect walltimes of the threads
	for( unsigned int threadCount = 0; threadCount < availableConcurrentThreads; threadCount++ ) {
		fileStrOutMeta << "Walltime thread " << threadCount << ":  "
		               << setMeshData[threadCount].mWallTimeThread << " seconds" << std::endl;
	}
	fileStrOutMeta << "Compute walltime:   "
	               << static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampParallel )
	               << " seconds" << std::endl;
	// --- Collect time for parallel processing

	timeStampParallel = time( nullptr );

	bool retVal(true);
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;

	// Feature vector file for volume descriptor (1st integral invariant)
	if( (!fileNameOutVol.empty()) && ( descriptVolume != NULL ) ) {
		std::fstream filestrVol;
		filestrVol.open( fileNameOutVol, std::fstream::out );
		if( !filestrVol.is_open() ) {
			std::cerr << "[GigaMesh] ERROR: Could not open '" << fileNameOutVol << "' for writing!" << std::endl;
			retVal = false;
		} else {
			filestrVol << std::fixed << std::setprecision( 10 );
			filestrVol << strHeader.str();
			for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
				Vertex* currVert = someMesh.getVertexPos( i );
				if( !currVert->assignFeatureVec( &descriptVolume[i*multiscaleRadiiSize],
				                                 multiscaleRadiiSize ) ) {
					std::cerr << "[GigaMesh] ERROR: Assignment of volume based feature vectors"
					          << "to vertices failed for Vertex No. " << i << "!" << std::endl;
				}
				// Index:
				filestrVol << i;
				// Scales or elements of the feature vector:
				for( uint j=0; j<multiscaleRadiiSize; j++ ) {
					filestrVol << " " << descriptVolume[i*multiscaleRadiiSize+j];
				}
				filestrVol << std::endl;
			}
			filestrVol.close();
			std::cout << "[GigaMesh] Volume descriptors stored in:             " << fileNameOutVol << std::endl;
		}
	}

	// Feature vector file for surface descriptor (2nd integral invariant)
	if( (!fileNameOutSurf.empty()) && ( descriptSurface != NULL )) {
		std::fstream filestrSurf;
		filestrSurf.open( fileNameOutSurf, std::fstream::out );
		if( !filestrSurf.is_open() ) {
			std::cerr << "[GigaMesh] ERROR: Could not open '" << fileNameOutSurf << "' for writing!" << std::endl;
			retVal = false;
		} else {
			filestrSurf << std::fixed << std::setprecision( 10 );
			filestrSurf << strHeader.str();
			for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
				// Assign 2nd feature vector only in case the 1st is not present!
				if( descriptVolume == NULL ) {
					Vertex* currVert = someMesh.getVertexPos( i );
					if( !currVert->assignFeatureVec( &descriptSurface[i*multiscaleRadiiSize],
					                                 multiscaleRadiiSize ) ) {
						std::cerr << "[GigaMesh] Assignment of area based feature vectors to vertices failed for Vertex No. " << i << "!" << std::endl;
					}
				}
				// Index:
				filestrSurf << i;
				// Scales:
				for( uint j=0; j<multiscaleRadiiSize; j++ ) {
					filestrSurf << " " << descriptSurface[i*multiscaleRadiiSize+j];
				}
				filestrSurf << std::endl;
			}
			filestrSurf.close();
			std::cout << "[GigaMesh] Surface descriptors stored in:            " << fileNameOutSurf << std::endl;
		}
	}

	// File for normal estimated as byproduct of the integral invariants:
	if( (!fileNameOutPatchNormal.empty()) && ( patchNormalsToAssign.size() > 0 ) ) {
		std::fstream filestrNormal;
		filestrNormal.open( fileNameOutPatchNormal, std::fstream::out );
		if( !filestrNormal.is_open() ) {
			std::cerr << "[GigaMesh] ERROR: Could not open '" << fileNameOutPatchNormal << "' for writing!" << std::endl;
			retVal = false;
		} else {
			filestrNormal << std::fixed << std::setprecision( 10 );
			//! \todo Replace "feature" with "normal" in strHeader.
			filestrNormal << strHeader.str();
			for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
				// Index of the vertex
				filestrNormal << patchNormalsToAssign.at( i ).mId;
				// Normal - always three elements
				filestrNormal << " " << patchNormalsToAssign.at( i ).mX;
				filestrNormal << " " << patchNormalsToAssign.at( i ).mY;
				filestrNormal << " " << patchNormalsToAssign.at( i ).mZ;
				filestrNormal << std::endl;
			}
			filestrNormal.close();
			std::cout << "[GigaMesh] Patch normal stored in:                   " << fileNameOutPatchNormal << std::endl;
			// Could be an option: someMesh.assignImportedNormalsToVertices( patchNormalsToAssign );
		}
	}

	// Feature vector file for BOTH descriptors (volume and surface)
	if( (!fileNameOutVS.empty()) && ( descriptSurface != NULL ) && ( descriptVolume != NULL ) ) {
		std::fstream filestrVS;
		filestrVS.open( fileNameOutVS, std::fstream::out );
		if( !filestrVS.is_open() ) {
			std::cerr << "[GigaMesh] ERROR: Could not open '" << fileNameOutVS << "' for writing!" << std::endl;
			retVal = false;
		} else {
			filestrVS << std::fixed << std::setprecision( 10 );
			filestrVS << strHeader.str();
			for( uint64_t i=0; i<someMesh.getVertexNr(); i++ ) {
				filestrVS << i;
				// Scales - Volume:
				for( uint j=0; j<multiscaleRadiiSize; j++ ) {
					filestrVS << " " << descriptVolume[i*multiscaleRadiiSize+j];
				}
				// Scales - Surface:
				for( uint j=0; j<multiscaleRadiiSize; j++ ) {
					filestrVS << " " << descriptSurface[i*multiscaleRadiiSize+j];
				}
				filestrVS << std::endl;
			}
			filestrVS.close();
			std::cout << "[GigaMesh] Volume and surface descriptors stored in: " << fileNameOutVS << std::endl;
		}
	}

	// PLY file INCLUDING ONE feature vector using the
	// volume descriptor (1st integral invariant), when both are present -- see above.
	// Additionally a function value is computed using the feature vector.
	if( (!fileNameOut3D.empty()) && 
	    ( ( descriptVolume != NULL ) || ( descriptSurface != NULL ) )
	    ) {
		// Apply feature vector metric:
		//--------------------------------------
		//vector<double> referenceVector;
		//double pNorm = 2.0;
		//Mesh::eFuncFeatureVecPNormWeigth weigthChoosen = Mesh::FEATURE_VECTOR_PNORM_WEIGTH_CUBIC;
		//referenceVector.resize( multiscaleRadiiSize, 0.0 );
		//someMesh.funcVertFeatureVecPNorm( referenceVector, pNorm, weigthChoosen );

		// Apply (simple) feature vector metric:
		//--------------------------------------
		someMesh.funcVertFeatureVecMax();
		// Save mesh having volumetric feature vectors.
		if( !someMesh.writeFile( fileNameOut3D ) ) {
			retVal = false;
		} else {
			std::cout << "[GigaMesh] PLY with volume descriptors stored in:    " << fileNameOut3D << std::endl;
		}
	}

	// Done
	fileStrOutMeta.close();
	std::cout << "[GigaMesh] Technical meta-data stored in:            " << fileNameOutMeta << std::endl;
	std::cout << "[GigaMesh] --------------------------------------------------" << std::endl;
	std::cout << "[GigaMesh] Writing the files took " << static_cast<int>( time( nullptr ) ) - static_cast<int>( timeStampParallel ) << " seconds." << std::endl;

	if( descriptVolume ) {
		delete[] descriptVolume;
	}
	if( descriptSurface  ) {
		delete[] descriptSurface;
	}
	delete[] multiscaleRadii;

	return( retVal );
}

//! Show software version.
void printVersion() {
	std::cout << "GigaMesh Software Framework FEATUREVECTORS 3D-data " << VERSION_PACKAGE << std::endl;
	std::cout << "Multi-threading with " << std::thread::hardware_concurrency() - 1 << " (dynamic) threads." << std::endl;
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName )
{
	std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
	std::cout << "GigaMesh Software Framework FEATUREVECTORS (1st and 2nd or V and P)" << std::endl << std::endl;
	std::cout << "Computes the first and second integral invariants for the given meshes on multiple scales\n"
	             "shortly known as MSII filtering. The first integral invariant computes the Volume, while \n"
	             "the second computes the Patch surface integral invariant." << std::endl;
	std::cout << "Note: for the 3rd and 4th integral invariant see 'gigamesh-featurevectors-sl'." << std::endl;
	std::cout << std::endl;
	std::cout << "The output file(s) will be namend the same as the input file " << std::endl;
	std::cout << "always using the following suffix:" << std::endl;
	std::cout << std::endl;
	std::cout << "  _r<number> ... radius of the largest sphere/scale" << std::endl;
	std::cout << "  _n<number> ... 2^<number> of scales" << std::endl;
	std::cout << "  _v<number> ... discretization for the largest scale. Large numbers are more precise\n"
	             "                 resulting in slower computation" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help                              Displays this help." << std::endl;
	std::cout << "  -v, --version                           Displays version information." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for output files:" << std::endl;
	std::cout << "  -k, --overwrite-existing                Overwrite exisitng files, which is not done by default" << std::endl;
	std::cout << "                                          to prevent accidental data loss. If this option is not set" << std::endl;
	std::cout << "                                          and the output file exists, the file will be skipped." << std::endl;
	std::cout << "  -s, --output-suffix <string>            Write the converted file using the given <string> as suffix for its name." << std::endl;
	std::cout << "                                          This options has no effect on the suffix created for the parameters." << std::endl;
	std::cout << "    , --concat-results                    Concatenate 1st and 2nd feature vector and store it in one file." << std::endl;
	std::cout << "                                          The file has the extenstion .vs.mat." << std::endl;
	std::cout << "                                          Has no effect, when only one integral invariant is computed." << std::endl;
	std::cout << "    , --no-normals-file                   Do not write the file with the normal vectors averaged per vertex" << std::endl;
	std::cout << "                                          of the triangles within the largest sphere." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for MSII filtering:" << std::endl;
	std::cout << "  -r, --radius SIZE                       Radius of the largest sphere/scale. Default is 1.0 (mm, unit assumed!)" << std::endl;
	std::cout << "                                          Recommended: should be equal or larger than the largest feature to be detected." << std::endl;
	std::cout << "  -n, --numScales SIZE                    Power of two for the number of scales. Default is 4 i.e. 16 scales." << std::endl;
	std::cout << "                                          Recommended: The default works well for most applications." << std::endl;
	std::cout << "  -l, --voxelSize SIZE                    Discretization for the (1st) volume integral invariant. Default is 256." << std::endl;
	std::cout << "                                          Recommended: should be a power of two. Values of 512 and 1024 can increase the results" << std::endl;
	std::cout << "                                          slightly at the cost of extra compute time." << std::endl;
	std::cout << "  -1, --no-volume-integral                Skip the (1st) volume integral invariant." << std::endl;
	std::cout << "  -2, --no-area-integral                  Skip the (2nd) patch area integral invariant." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for testing and debugging:" << std::endl;
	std::cout << "    , --log-level [0-4]                   Sets the log level of this application.\n"
	             "                                          Higher numbers increases verbosity.\n"
	             "                                          (Default: 1)" << std::endl;
	//std::cout << "" << std::endl;
}

//! Main routine for generating an array of feature vectors
//!
//! Remark: prefer MeshSeed over Mesh as it is faster by a factor of 3+
//==========================================================================
int main( int argc, char *argv[] ) {

	LOG::initLogging();

	// Default arameters
	std::filesystem::path optFileSuffix;
	double       radius{_DEFAULT_FEATUREGEN_RADIUS_};
	unsigned int xyzDim{_DEFAULT_FEATUREGEN_XYZDIM_};
	unsigned int radiiCount{_DEFAULT_FEATUREGEN_RADIICOUNT_};
	bool         replaceFiles{false};
	bool         noVolumeIntegral{false};
	bool         noAreaIntegral{false};
	bool         noNormalsFile{false};
	bool         concatResults{false};

	static struct option longOptions[] = {
		{ "radius"            , required_argument, nullptr, 'r' },
		{ "voxelSize"         , required_argument, nullptr, 'l' },
		{ "numScales"         , required_argument, nullptr, 'n' },
		{ "overwrite-existing", no_argument      , nullptr, 'k' },
		{ "no-volume-integral", no_argument      , nullptr, '1' },
		{ "no-area-integral"  , no_argument      , nullptr, '2' },
		{ "no-normals-file"   , no_argument      , nullptr,  0  },
		{ "output-suffix"     , required_argument, nullptr, 's' },
		{ "concat-results"    , no_argument      , nullptr,  0  },
		{ "version"           , no_argument,       nullptr, 'v' },
		{ "help"              , no_argument      , nullptr, 'h' },
		{ "log-level"         , required_argument, nullptr,  0  },
		{ nullptr, 0, nullptr, 0 }
	};

	// PARSE command line options
	//--------------------------------------------------------------------------
	opterr = 0;
	int c{0};
	int optionIndex{0};
	int tmpInt{0};
	bool radiusSet{false};
	while( ( c = getopt_long_only( argc, argv, "r:l:n:k12s:vh",
	                               longOptions, &optionIndex) ) != -1 ) {
		switch( c ) {
			//! Option r: absolut radius (in units, default: 1.0)
			case 'r':
				radius = atof( optarg );
				radiusSet = true;
				break;
			//! Option v: edge length of the voxel cube (in voxels, default: 256)
			case 'l':
				tmpInt = atof( optarg );
				if( tmpInt <= 0 ) {
					std::cerr << "[GigaMesh] Error: negative or zero value given: " << tmpInt << " for the number of voxels (option -v)!" << std::endl;
					exit( EXIT_FAILURE );
				}
				xyzDim = static_cast<unsigned int>(tmpInt);
				break;
			//! Option n: 2^n scales (default: 4 => 16 scales)
			case 'n':
				tmpInt = atof( optarg );
				if( tmpInt < 0 ) {
					std::cerr << "[GigaMesh] Error: negative value given: " << tmpInt << " for the number of radii (option -n)!" << std::endl;
					exit( EXIT_FAILURE );
				}
				radiiCount = static_cast<unsigned int>(tmpInt);
				break;
			//! Option k: replaces output files
			case 'k':
				std::cout << "[GigaMesh] Warning: files might be replaced!" << std::endl;
				replaceFiles = true;
				break;
			//! Option 2: skip area/surface based integral invariant
			case '1':
				std::cout << "[GigaMesh] Warning: volume (1st) integral invariant will NOT be computed!" << std::endl;
				noVolumeIntegral = true;
				break;
			//! Option 2: skip area/surface based integral invariant
			case '2':
				std::cout << "[GigaMesh] Warning: area (2nd) integral invariant will NOT be computed!" << std::endl;
				noAreaIntegral = true;
				break;
			//! Option s: output file suffix
			case 's':
				optFileSuffix = std::string( optarg );
				break;
			//! Option h: print help
			case 'h':
				printHelp( argv[0] );
				std::exit( EXIT_SUCCESS );
				break;
			//! Option v: print version
			case 'v':
				printVersion();
				std::exit( EXIT_SUCCESS );
				break;
			case '?':
				std::cerr << "[GigaMesh] ERROR: Unknown option!" << std::endl;
				break;
			// Non-short options:
			case 0:
				if( std::string(longOptions[optionIndex].name) == "log-level" ) {
					unsigned int arg = optarg[0] - '0';
					if(arg <= 5)
					{
						LOG::setLogLevel(static_cast<LOG::LogLevel>(arg));
					}
					else
					{
						std::cerr << "[GigaMesh] WARNING: Log level is out of range [0-4]!" << std::endl;
					}
				}
				if( std::string(longOptions[optionIndex].name) == "concat-results" ) {
					concatResults = true;
				}
				if( std::string(longOptions[optionIndex].name) == "no-normals-file" ) {
					noNormalsFile = true;
				}
				break;
			default:
				std::cerr << "[GigaMesh] Error: Unknown option '" << c << "'!" << std::endl;
				std::exit( EXIT_FAILURE );
		}
	}
	// Check disable options
	if( noVolumeIntegral && noAreaIntegral && noNormalsFile ) {
		std::cerr << "[GigaMesh] ERROR: Nothing to do, because all computation were deactivated!" << std::endl;
		std::exit( EXIT_FAILURE );
	} 
	// Check argument ranges
	if( radius <= 0.0 ) {
		std::cerr << "[GigaMesh] Error: negative or zero radius given: " << radius << " (option -r)!" << std::endl;
		std::exit( EXIT_FAILURE );
	}
	if( !radiusSet ) {
		std::cout << "[GigaMesh] Warning: default radius is used (option -r missing)!" << std::endl;
	}

	// SHOW Build information
	printBuildInfo();

	// Fetch username and host for the technical meta-data
	std::string userName( "unknown" );
	std::string hostName( "unknown" );
	getUserAndHostName( userName, hostName );

	unsigned long filesProcessed = 0UL;
	unsigned long filesFailed    = 0UL;

	for(auto nonOptionArgumentIndex = optind; nonOptionArgumentIndex < argc; ++nonOptionArgumentIndex)
	{
		std::filesystem::path nonOptionArgumentPath(argv[nonOptionArgumentIndex]);

		if(std::filesystem::exists(nonOptionArgumentPath))
		{
			std::wcout << L"[GigaMesh] Processing file " << nonOptionArgumentPath.wstring() << L"..." << std::endl;

			if( !generateFeatureVectors( nonOptionArgumentPath,
			                             optFileSuffix,
			                             radius,
			                             xyzDim,
			                             radiiCount,
			                             replaceFiles,
			                             noVolumeIntegral,
			                             noAreaIntegral,
			                             noNormalsFile,
			                             concatResults,
			                             hostName, userName
			                           ) )
			{
				LOG::error() << "[GigaMesh] ERROR: generate featurevectors failed for: " << nonOptionArgumentPath.string() << " !\n";
				filesFailed++;
			}
			++filesProcessed;
		}
	}

	// No file was processed:
	if( filesProcessed == 0 ) {
		std::cout << "[GigaMesh] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
		std::cerr << "[GigaMesh] ERROR: no filename for input given!" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Some files were not processed:
	if( filesFailed > 0 ) {
		std::cout << "[GigaMesh] !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
		std::cerr << "[GigaMesh] ERROR: " << filesFailed << " of " << filesProcessed << " files could not be processed!" << std::endl;
		std::exit( EXIT_FAILURE );
	}

	// Everything fine:
	std::cout << "[GigaMesh] ========================================================" << std::endl;
	std::cout << "[GigaMesh] successfully processed " << filesProcessed << " file(s)." << std::endl;
	std::exit( EXIT_SUCCESS );
}
