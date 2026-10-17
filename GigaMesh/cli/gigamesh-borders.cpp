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

#include <cstdio>
#include <cstdlib> // calloc
#include <string>
#ifdef _MSC_VER	//windows version for hostname and login
//#include <winsock.h>
//#include <WinBase.h>
//#include <lmcons.h>
#include "getoptwin.h"

#else
#include <unistd.h> // gethostname, getlogin_r

#include <getopt.h>
#endif
#include <filesystem>
#include <iostream>


#include <GigaMesh/printbuildinfo.h>
#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/logging/Logging.h>
//#include "meshseed.h"

// //#include "voxelcuboid.h"
//#include "voxelfilter25d.h"

#include <sys/stat.h> // statistics for files

using namespace std;

bool convertMeshData(
             const std::filesystem::path&   rFileName,
		     const bool      rWriteVertexId,
		     const bool      rWriteNormals,
		     const bool      rReplaceFiles
) {
	std::filesystem::path fileExtension =  rFileName .extension();
	// Check file extension for input file
	if( !(fileExtension == ".obj" || fileExtension == ".ply") ) {
		LOG::error() << "[GigaMesh] ERROR: File extension '" << fileExtension << "' is faulty!\n";
		return( false );
	}

	// Add parameters to output prefix
	std::filesystem::path fileNameOut = std::filesystem::path( rFileName ).stem();
	fileNameOut += L".pline";

	// Check: Input file exists?
	if( !std::filesystem::exists( rFileName ) ) {
		LOG::error() << "[GigaMesh] Error: File '" << rFileName << "' not found!\n";
		return( false );
	}

	// Output file for 3D data including the volumetric feature vectors.
	std::filesystem::path pathOut = std::filesystem::absolute(  rFileName ).remove_filename();
	std::filesystem::path fileNameOut3D = pathOut;
	fileNameOut += fileNameOut;
	if( std::filesystem::exists(fileNameOut3D) ) {
		if( !rReplaceFiles ) {
			LOG::error() << "[GigaMesh] File '" << fileNameOut3D << "' already exists!\n";
			return( false );
		}
		LOG::error() << "[GigaMesh] Warning: File '" << fileNameOut3D << "' will be replaced!\n";
	}

	// All parameters OK => infos to stdout and file with metadata  -----------------------------------------------------------
	cout << "[GigaMesh] File IN:         " << rFileName << endl;
	wcout << "[GigaMesh] File OUT/Prefix: " << fileNameOut << endl;

	// Prepare data structures
	//--------------------------------------------------------------------------
	bool readSucess;
	Mesh someMesh( rFileName, readSucess );
	if( !readSucess ) {
		cerr << "[GigaMesh] Error: Could not open file '" << rFileName << "'!" << endl;
		return( false );
	}

	// Fetch mesh data
	Vector3D bbDim;
	someMesh.getBoundingBoxSize( bbDim );
	double bbWdith  = (double)round( bbDim.getX()*1.0 ) / 10.0;
	double bbHeight = (double)round( bbDim.getY()*1.0 ) / 10.0;
	double bbThick  = (double)round( bbDim.getZ()*1.0 ) / 10.0;
	// Area and average resolution
	double areaAcq;
	someMesh.getFaceSurfSum( &areaAcq );
	areaAcq = round( areaAcq );
	double volDXYZ[3]{ 0.0, 0.0, 0.0 };
	someMesh.getMeshVolumeDivergence( volDXYZ[0], volDXYZ[1], volDXYZ[2] );
	string modelID  = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
	string modelMat = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
	// Write data to file
	cout << "[GigaMesh] Model ID:        " << modelID << endl;
	cout << "[GigaMesh] Material:        " << modelMat << endl;
	cout << "[GigaMesh] Vertices:        " << someMesh.getVertexNr() << "" << endl;
	cout << "[GigaMesh] Faces:           " << someMesh.getFaceNr() << "" << endl;
	cout << "[GigaMesh] Bounding Box:    " << bbWdith << " x " << bbHeight << " x " << bbThick << " cm" << endl;
	cout << "[GigaMesh] Area:            " << areaAcq/100.0 << " cm^2" << endl;
	cout << "[GigaMesh] Volume (dx):     " << volDXYZ[0]/1000.0 << " cm^3" << endl;
	cout << "[GigaMesh] Volume (dy):     " << volDXYZ[1]/1000.0 << " cm^3" << endl;
	cout << "[GigaMesh] Volume (dz):     " << volDXYZ[2]/1000.0 << " cm^3" << endl;

	time_t     rawtime;
	struct tm* timeinfo;
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	cout << "[GigaMesh] Start date/time is: " << asctime( timeinfo );// << endl;
	bool convertSuccess = someMesh.convertBordersToPolylines();
	if( !convertSuccess ) {
		cerr << "[GigaMesh] Error: Could not convert borders of mesh '" << rFileName << "'!" << endl;
		return( false );
	}

	bool writeSuccess = someMesh.exportPolyLinesCoords(fileNameOut3D,rWriteNormals,rWriteVertexId);
	if( !writeSuccess ) {
		wcerr << L"[GigaMesh] Error: Could not write file '" << fileNameOut3D << "'!" << endl;
		return( false );
	}

	cout << "[GigaMesh] End date/time is: " << asctime( timeinfo );// << endl;

	return( true );
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName ) {
	std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
	std::cout << "GigaMesh Software Framework export borders of 3D-data." << std::endl << std::endl;
	std::cout << "Extracts and exports the borders of the given meshes as Polylines ('.pline')." << std::endl;
	std::cout << "Normals per vertex and vertex ID's are written per default.";
	std::cout << std::endl << std::endl;
	std::cout << "Options:" << endl;
	std::cout << "  -h, --help                              Displays this help." << std::endl;
	std::cout << "  -v, --version                           Displays version information." << std::endl << std::endl;
	std::cout << "  -p, --write-vertices                    Write the ids per vertex." << std::endl;
	std::cout << "  -n, --write-normals                     Write the normal vectors per vertex." << std::endl;
	std::cout << "  -k, --overwrite-existing                Overwrite exisitng files, which is" << std::endl;
	std::cout << "                                          not done by default to prevent" << std::endl;
	std::cout << "                                          accidental data loss." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for testing and debugging:" << std::endl;
	std::cout << "    , --log-level [0-4]                   Sets the log level of this application.\n"
	             "                                          Higher numbers increases verbosity.\n"
	             "                                          (Default: 1)" << std::endl;
	//std::cout << "" << endl;
}

//! Main routine for loading a PLY and exporting borders as .pline
//==============================================================================================================================================================
int main( int argc, char *argv[] ) {

	LOG::initLogging();

	// Default flags
	bool optReplaceFiles = false;
	bool optWriteVertexId  = false;
	bool optWriteNormals = false;

	// PARSE command line options
	//--------------------------------------------------------------------------
	// https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
	static struct option longOptions[] = {
		{ "write-vertices",               no_argument,       nullptr, 'p' },
		{ "write-normals",                no_argument,       nullptr, 'n' },
		{ "overwrite-existing",           no_argument,       nullptr, 'k' },
		{ "version",                      no_argument,       nullptr, 'v' },
		{ "help",                         no_argument,       nullptr, 'h' },
		{ "log-level",                    required_argument, nullptr,  0  },
		{ nullptr, 0, nullptr, 0 }
	};

	int character = 0;
	int optionIndex = 0;

	while( ( character = getopt_long_only( argc, argv, ":pnkvh",
		 longOptions, &optionIndex ) ) != -1 ) {
		switch(character) {
			case 0:
				// printf ("option %s", long_options[option_index].name);
				// if (optarg) printf (" with arg %s", optarg);

				if(std::string(longOptions[optionIndex].name) == "log-level")
				{
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

				break;

			case 'p': // write vertex ids
				std::cout << "[GigaMesh] Vertex ID's will be added." << std::endl;
				optWriteVertexId = true;
				break;

			case 'n': // write vertex normals
				std::cout << "[GigaMesh] Vertex normals will be added." << std::endl;
				optWriteNormals = true;
				break;

			case 'k': // replaces output files
				std::cout << "[GigaMesh] Warning: files might be replaced!" << std::endl;
				optReplaceFiles = true;
				break;

			case 'v':
				std::cout << "GigaMesh Software Framework export borders of 3D Data " << VERSION_PACKAGE << endl;
				std::cout << "Multi-threading with " << std::thread::hardware_concurrency() - 1 << " threads." << endl;
				std::exit( EXIT_SUCCESS );
				break;

			case 'h':
				printHelp( argv[0] );
				std::exit( EXIT_SUCCESS );
				break;

			default: // Unknown option given
				std::cerr << "[GigaMesh] ERROR: Unknown option '" << character << "'!" << std::endl;
				std::cerr << "[GigaMesh]        See -h or --help for available options." << std::endl;
				std::exit( EXIT_FAILURE );
		}
	}

	// No files given i.e. wrong arguments
	if( argc-optind <= 0 ) {
		std::cerr << "[GigaMesh] ERROR: No files given!" << std::endl << std::endl;
		printHelp( argv[0] );
		std::exit( EXIT_FAILURE );
	}

	// SHOW Build information
	printBuildInfo();

	// Process given files
	unsigned long filesProcessed = 0;
	for( int nonOptionArgumentCount = optind;
	     nonOptionArgumentCount < argc; nonOptionArgumentCount++ ) {

		std::filesystem::path filePath( argv[nonOptionArgumentCount] );

		if( filePath.empty() ) {
			std::cout << "[GigaMesh] Processing file " << filePath << "..." << std::endl;

			if( !convertMeshData( filePath, optWriteVertexId,
					      optWriteNormals, optReplaceFiles ) ) {
				std::cerr << "[GigaMesh] ERROR: convertMeshData failed!" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			filesProcessed++;
		}
	}

	std::cout << "[GigaMesh] Processed files: " << filesProcessed << std::endl;
	exit( EXIT_SUCCESS );
}
