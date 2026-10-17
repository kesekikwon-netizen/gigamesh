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

#include <vector>
#include <string>
#include <regex>
#include <cstdio>
#include <cstdlib>

#ifdef _MSC_VER	//windows version for hostname and login
//#include <winsock.h>
//#include <WinBase.h>
//#include <lmcons.h>
#include "getoptwin.h"

#else
#include <unistd.h> // gethostname, getlogin_r

#include <getopt.h>
#endif

#include <sys/stat.h> // Statistics for files
#include <filesystem>
#include <iostream>

#include <GigaMesh/printbuildinfo.h>

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/logging/Logging.h>

bool cleanupGigaMeshData(
                const  std::filesystem::path& fileNameIn,
                const  std::filesystem::path& fileNameOutSuffix,
                const  std::string& rMaterialWhenEmpty,
                bool                rMaterialWhenEmptySet,
                bool                rFileNameAsIdWhenEmpty,
                std::size_t         rRemoveTrailingChars,
                bool                rEnforceIdMaterial,
                double          percentArea = 0.1,
                bool            applyBorderErosion = true,
                bool            replaceFiles = false,
                bool            removeOnlyFlag = false,
                bool            keepLargestComponent = false,
                bool            skipLargestHole = false,
                unsigned long   maxNumberVertices = 3000
) {
	// Check file extension for input file
	if( !fileNameIn.has_extension() ) {
		std::cerr << "[GigaMesh] ERROR: No extension/type for input file '" << fileNameIn << "' specified!" << std::endl;
		return( false );
	}

	// Prepare filename for output.
	std::filesystem::path suffix      = fileNameOutSuffix;
	std::filesystem::path fileNameOut = fileNameIn.stem();
	if( std::regex_match( fileNameOut.wstring(), std::wregex( L".*_GM[oOxXcCfFpP]*" ) ) ) {
		// Seems to be already processed, so check for Orientation
		std::wregex rgxGMOrientated( L"(.*)(_GM)(o|O)(.*)" );
		if( std::regex_match( fileNameOut.wstring(), rgxGMOrientated ) ) {
			suffix = suffix.wstring().insert( 3, 1, L'O' );
		}
		fileNameOut = std::regex_replace( fileNameOut.wstring(), std::wregex(L"_GM[oOxXcCfFpP]*") , L"" );
	}
	fileNameOut += suffix;
	fileNameOut += L".ply";

	if( !std::filesystem::exists(fileNameIn)) {
		std::cerr << "[GigaMesh] ERROR: Input file '" << fileNameIn << "' not found!" << std::endl;
		return( false );
	}

	if( std::filesystem::exists(fileNameOut) ) {
		if(!replaceFiles) {
			std::cerr << "[GigaMesh] File '" << fileNameOut << "' already exists - SKIPPED!" << std::endl;
			return( true );
		}
		std::cerr << "[GigaMesh] Warning: File '" << fileNameOut << "' will be replaced!" << std::endl;
	}

	// Filenames without path
	std::string fileNameInName  = std::filesystem::path( fileNameIn ).filename().string();
	std::string fileNameOutName = std::filesystem::path( fileNameOut ).filename().string();

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// PREPARE data structures and Meta-data
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	bool readSucess;
	Mesh someMesh( fileNameIn, readSucess );
	if( !readSucess ) {
		std::cerr << "[GigaMesh] Error: Could not open file '" << fileNameIn << "'!" << std::endl;
		return false;
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Meta-data options:
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if( rMaterialWhenEmptySet ) {
		std::string currMat = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_MATERIAL );
		if( ( currMat.size() == 0 ) || rEnforceIdMaterial ){
			someMesh.getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_MATERIAL, rMaterialWhenEmpty );
		}
	}
	if( rFileNameAsIdWhenEmpty ) {
		std::string currModelId = someMesh.getModelMetaDataRef().getModelMetaString( ModelMetaData::META_MODEL_ID );
		if( ( currModelId.size() == 0 ) || rEnforceIdMaterial ) {
			currModelId = std::filesystem::path( fileNameInName ).stem().string();
			std::replace( currModelId.begin(), currModelId.end(), '_', ' ' );
			// Remove trailing chars
			if( rRemoveTrailingChars > 0 ) {
				std::size_t charsRemain = currModelId.size() - rRemoveTrailingChars;
				currModelId = currModelId.substr( 0, charsRemain );
			}
			someMesh.getModelMetaDataRef().setModelMetaString( ModelMetaData::META_MODEL_ID, currModelId );
		}
	}

	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// + MAIN FUNCTIONS
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// Initial numbers of primitives
	auto oldVertexNr = someMesh.getVertexNr();
	auto oldFaceNr = someMesh.getFaceNr();

	// Keep ONLY the largest connected component.
	//----------------------------------------------------------
	if( !keepLargestComponent ) {
		uint64_t largestLabelId;
		std::set<int64_t> labelNrs;

		// Determine connected components and select the largest
		//----------------------------------------------------------
		someMesh.labelVerticesAll();
		someMesh.getVertLabelAreaLargest( largestLabelId );

		// Select and remove everything except the largest component
		//----------------------------------------------------------
		std::cout << "[GigaMesh] Label kept: " << largestLabelId << std::endl;
		labelNrs.insert( -static_cast<int64_t>(largestLabelId) );

		someMesh.deSelMVertsAll();
		someMesh.selectVertLabelNo( labelNrs );
		someMesh.removeVerticesSelected();
	}

	// MAIN cleaning procedure
	//----------------------------------------------------------
	uint64_t iterationCount = 0;
	if( removeOnlyFlag ) {
		// Single iteration of automatic mesh polishing, BECAUSE
		someMesh.removeUncleanSmall( fileNameOut, percentArea, applyBorderErosion ); // use fileNameOut3D instead of "" for saving the intermediate mesh.
		iterationCount++;

		std::cout << "[GigaMesh] REMOVE ONLY: " << oldVertexNr-someMesh.getVertexNr() << " Vertices and ";
		std::cout << oldFaceNr-someMesh.getFaceNr() << " Faces removed." << std::endl;
	} else {
		//! \bug Check why completeRestore is unstable without intermediate storage.
		// BROKEN: Apply automatic mesh polishing using the corresponding method
		// someMesh.completeRestore( "", percentArea, applyBorderErosion, skipLargestHole, maxNumberVertices, nullptr ); // use fileNameOut instead of "" for saving the intermediate mesh.
		someMesh.completeRestore( fileNameOut, percentArea, applyBorderErosion, 
		                          skipLargestHole, maxNumberVertices, 
		                          nullptr, iterationCount ); // use fileNameOut instead of "" for saving the intermediate mesh.

		std::cout << "[GigaMesh] POLISH: Vertex count changed by " << static_cast<long>(someMesh.getVertexNr())-static_cast<long>(oldVertexNr) << std::endl;
		std::cout << "[GigaMesh]         Face count changed by   " << static_cast<long>(someMesh.getFaceNr())-static_cast<long>(oldFaceNr) << std::endl;

		// Prepare for editing holes i.e. false-positiv filled connected components
		//-------------------------------------------------------------------------
		someMesh.selVertByFlag( Primitive::FLAG_SYNTHETIC );
		someMesh.labelSelectedVerticesBackGrd();
	}

	// WRITE clean data
	//----------------------------------------------------------
	//! \todo supress writing in removeUncleanSmall and completeRestore OR check for file write success. For now the following code to save the file does not make any sense.
//	if( !someMesh.writeFile( fileNameOut ) ) {
//		std::cerr << "[GigaMesh] ERROR: writing file " << fileNameOut << "!" << std::endl;
//		return( false );
//	}

	// Done.
	//----------------------------------------------------------
	return( true );
}


//! Show software version.
void printVersion() {
	std::cout << "GigaMesh Software Framework CLEAN 3D-data " << VERSION_PACKAGE << std::endl;
	std::cout << "Multi-threading with " << std::thread::hardware_concurrency() - 1 << " (dynamic) threads." << std::endl;
#ifdef LIBPSALM
	std::cout << "Hole filling enabled. POLISH possible." << std::endl;
#else
	std::cout << "Hole filling disabled. CLEAN only." << std::endl;
#endif
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName ) {
	std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
	std::cout << "GigaMesh Software Framework CLEAN 3D-data" << std::endl << std::endl;
	std::cout << "Cleans given meshes. The output file(s) will be namend the same as the input file " << std::endl;
	std::cout << "using the suffix GMC[F]:" << std::endl;
	std::cout << std::endl;
	std::cout << "  GM ... processed with GigaMesh" << std::endl;
	std::cout << "  C .... cleaned i.e. faulty parts removed" << std::endl;
	std::cout << "  F .... holes filled unless -r is used" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help                              Displays this help." << std::endl;
	std::cout << "  -v, --version                           Displays version information." << std::endl;
	std::cout << "  -k, --overwrite-existing                Overwrite existing files, which is not done by default" << std::endl;
	std::cout << "                                          to prevent accidental data loss. If this option is not set" << std::endl;
	std::cout << "                                          and the output file exists, the file will be skipped." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for cleaning and polishing:" << std::endl;
	std::cout << "  -r, --remove-only                       No holes are filled. Only corrupt mesh parts are removed." << std::endl;
	std::cout << "                                          File suffix is set to GMC. Options -s and -g will have no effect." << std::endl;
	std::cout << "  -p, --remove-components-relative SIZE   Remove all connected components i.e. surface parts smaller than" << std::endl;
	std::cout << "                                          the given percentage [0.0...1.0]. The default for SIZE is 0.1." << std::endl;
	std::cout << "  -l, --keep-largest-component            Keep only the largest connected component." << std::endl;
	std::cout << "  -s, --skip-largest-hole                 Do not fill the largest hole. Has no effect, when -r is used." << std::endl;
	std::cout << "                                          Typically used for partial 3D-scans, where e.g. the backside is missing." << std::endl;
	std::cout << "  -g, --skip-holes-larger SIZE            Holes with more than SIZE vertices along its border are not filled." << std::endl;
	std::cout << "                                          Typically used for surface with large holes, which often have a complex shape." << std::endl;
	std::cout << "                                          The default for SIZE is 3000. Set 0 (zero) to attempted all holes to be filled." << std::endl;
	std::cout << "                                          Has no effect, when -r is used." << std::endl;
	std::cout << "  -n, --no-border-erosion                 Do not apply border erosion i.e. keep dangling faces along the border." << std::endl;
	std::cout << std::endl;
	std::cout << "Options to (pre)set the embedded Meta-data:" << std::endl;
	std::cout << "  -m, --set-material-when-empty STRING    Set the material to STRING, when empty." << std::endl;
	std::cout << "  -i, --set-id-when-empty                 Set the stem of the <file> as id, when empty." << std::endl;
	std::cout << "  -j, --set-id-remove-trailing-chars SIZE Remove SIZE trailing characters from the filename stem for the id."<< std::endl;
	std::cout << "                                          Will be ignored if -i is not used." << std::endl;
	std::cout << "  -o, --set-material-id-forced            Enforce id and material, even when NOT empty." << std::endl;
	std::cout << std::endl;
	std::cout << "Options for testing and debugging:" << std::endl;
	std::cout << "    , --log-level [0-4]                   Sets the log level of this application.\n"
	             "                                          Higher numbers increases verbosity.\n"
	             "                                          (Default: 1)" << std::endl;
	//std::cout << "" << std::endl;
}



//! Main routine for loading a (binary) PLY and store it after cleaning the mesh.
//==============================================================================================================================================================

int main( int argc, char* argv[] ) {

	LOG::initLogging();

	// Default string parameter
	std::string fileNameOutSuffix = "_GMCF";
	std::string materialWhenEmpty = "n.a.";

	// Default integer parameter
	unsigned long skipHolesLargerThan = 3000;
	std::size_t   removeTrailingChars = 0;

	// Default float parameter
	double percentArea = 0.1;

	// Default flags
	bool replaceFiles = false;
	bool removeOnlyFlag = false;
	bool keepLargestComponent = false;
	bool skipLargestHole = false;
	bool applyBorderErosion = true;
	bool materialWhenEmptySet = false;
	bool fileNameAsIdWhenEmpty = false;
	bool enforceIdMaterial     = false;

	// PARSE command line options
	//--------------------------------------------------------------------------
	// https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
	static struct option longOptions[] = {
		{ "overwrite-existing",           no_argument,       nullptr, 'k' },
		{ "remove-only",                  no_argument,       nullptr, 'r' },
		{ "remove-components-relative",   required_argument, nullptr, 'p' },
		{ "keep-largest-component",       no_argument,       nullptr, 'l' },
		{ "skip-largest-hole",            no_argument,       nullptr, 's' },
		{ "skip-holes-larger",            required_argument, nullptr, 'g' },
		{ "no-border-erosion",            no_argument,       nullptr, 'n' },
		{ "set-material-when-empty",      required_argument, nullptr, 'm' },
		{ "set-id-when-empty",            no_argument,       nullptr, 'i' },
		{ "set-id-remove-trailing-chars", required_argument, nullptr, 'j' },
		{ "set-material-id-forced",       no_argument,       nullptr, 'o' },
		{ "version",                      no_argument,       nullptr, 'v' },
		{ "help",                         no_argument,       nullptr, 'h' },
		{ "log-level",                    required_argument, nullptr,  0  },
		{ nullptr, 0, nullptr, 0 }
	};

	int character = 0;
	int optionIndex = 0;

	while( ( character = getopt_long_only( argc, argv, ":krp:lsg:nm:ij:ovh",
	         longOptions, &optionIndex ) ) != -1 ) {
		switch(character) {
			case 0:

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

				// printf ("option %s", long_options[option_index].name);
				// if (optarg) printf (" with arg %s", optarg);	
				break;

			case 'k': // replaces output files
				std::cout << "[GigaMesh] Warning: files might be replaced!" << std::endl;
				replaceFiles = true;
				break;

			case 'r':
				// --remove-only flag was set.
				removeOnlyFlag = true;
				fileNameOutSuffix = "_GMC";
				break;

			case 'p':
				percentArea = std::atof( optarg );
				break;

			case 'l':
				keepLargestComponent = true;
				break;

			case 's':
				skipLargestHole = true;
				break;

			case 'n':
				applyBorderErosion = false;
				break;

			case 'g':
				skipHolesLargerThan = std::stoul( optarg );
				break;

			case 'm':
				materialWhenEmpty = std::string( optarg );
				materialWhenEmptySet = true;
				break;

			case 'i':
				fileNameAsIdWhenEmpty = true;
				break;

			case 'j':
				removeTrailingChars = std::stoul( optarg );
				break;

			case 'o':
				enforceIdMaterial = true;
				break;

			case 'v':
				printVersion();
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

	// Show build info
	printBuildInfo();

	// Show parameters and flags
	//! \todo add/show ALL parameters and flags used.
	std::cout << "[GigaMesh] Holes consisting of more than " << skipHolesLargerThan << " vertices/edges are not attempted to be filled." << std::endl;
	std::cout << "[GigaMesh] Surfaces with an area less than " << ( percentArea*100.0 ) << "% of the total surface will be removed." << std::endl;
	std::cout << "[GigaMesh] ----------------------------------------------------------------------------------" << std::endl;

	// Process given files
	unsigned long filesProcessed = 0;
	for( int nonOptionArgumentCount = optind;
	     nonOptionArgumentCount < argc; nonOptionArgumentCount++ ) {

		std::string nonOptionArgumentString = std::string( argv[nonOptionArgumentCount] );

		if( !nonOptionArgumentString.empty() ) {
			std::cout << "[GigaMesh] Processing file " << nonOptionArgumentString << "..." << std::endl;

			if( !cleanupGigaMeshData( nonOptionArgumentString, fileNameOutSuffix,
			                          materialWhenEmpty, materialWhenEmptySet, fileNameAsIdWhenEmpty,
			                          removeTrailingChars, enforceIdMaterial, percentArea, applyBorderErosion,
			                          replaceFiles, removeOnlyFlag,
			                          keepLargestComponent, skipLargestHole,
			                          skipHolesLargerThan ) ) {
				std::cerr << "[GigaMesh] ERROR: cleanupGigaMeshData failed!" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			filesProcessed++;
		}
	}

	std::cout << "[GigaMesh] Processed files: " << filesProcessed << std::endl;
	std::exit( EXIT_SUCCESS );
}
