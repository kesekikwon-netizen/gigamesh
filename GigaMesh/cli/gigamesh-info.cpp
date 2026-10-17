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

#include <GigaMesh/printbuildinfo.h>

#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/logging/Logging.h>

bool infoGigaMeshData(
                const std::filesystem::path&   rFileNameIn,    //!< Input - filename.
                MeshInfoData&                  rFileInfos,     //!< Output - data properties.
                bool                           rSuppressSelfIntersection,   //!< Option: Decision whether the selfintersected face are calculated (computationally intensive)
                bool                           rAbsolutePath   //!< Option: display absolute path or stem only.
) {
	// Check: Input file exists
	if( !std::filesystem::exists( rFileNameIn) ) {
		std::wcerr << "[GigaMesh] ERROR: File '" << rFileNameIn.wstring() << "' not found!" << std::endl;
		return( false );
	}

	// Prepare data structures
	//--------------------------------------------------------------------------
	bool readSucess;
	Mesh someMesh( rFileNameIn, readSucess );
	if( !readSucess ) {
		std::wcerr << "[GigaMesh] ERROR: Could not open file '" << rFileNameIn.wstring() << "'!" << std::endl;
		return( false );
	}

	// Fetch count of connected components via labeling
	someMesh.labelVerticesAll();

	// Count primitives and their properties
    if( !someMesh.getMeshInfoData( rFileInfos, rAbsolutePath, !rSuppressSelfIntersection) ) {
		std::wcerr << "[GigaMesh] ERROR: Could not fetch mesh information about '" << rFileNameIn.wstring() << "'!" << std::endl;
		return( false );
	}

	// Done
	return( true );
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName ) {
	std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
	std::cout << "GigaMesh Software Framework INFO 3D-data" << std::endl << std::endl;
	std::cout << "Provides information about the given meshes." << std::endl << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help                              Displays this help." << std::endl;
	std::cout << "  -v, --version                           Displays version information." << std::endl << std::endl;
	std::cout << "  -o, --output-csv-file <file>            Write the collected information to a CSV file. One line per file." << std::endl;
	std::cout << "                                          The information will be written to stdout, when this option is not set." << std::endl;
	std::cout << "  -j, --write-sidecar-file-json           Write mesh information in JSON as side car file." << std::endl;
	std::cout << "  -l, --write-sidecar-file-ttl            Write mesh information in TTL as side car file." << std::endl;
	std::cout << "  -x, --write-sidecar-file-xml            Write mesh information in XML as side car file." << std::endl;
	std::cout << "  -t, --write-sidecar-file-html           Write mesh information in HTML as side car file." << std::endl;
	std::cout << "  -a, --write-sidecar-files               Write all the above side car files." << std::endl;
    std::cout << "  -q, --quick                             Suppress detection of self-intersections (computationally intensive)." << std::endl;
	//! \todo integrate '-k' option for '-j/l/x/t/a' and NOT for '-o'
//	std::cout << "  -k, --overwrite-existing                Overwrite exisitng files, which is not done by default" << std::endl;
//	std::cout << "                                          to prevent accidental data loss." << std::endl;
	std::cout << "  -s, --show-absolute-filename            Show the filename with extenstion and absolute path." << std::endl;
	std::cout << "                                          If not given only the stem of the filename is printed." << std::endl;
	std::cout << "                                          Affects all types of output i.e. side car files and tabular." << std::endl;
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
	std::filesystem::path fileNameCSVOut;

	// Default flags
	//! \todo integrate bool optReplaceFiles = false;
	bool optAbsolutePath = false;
	bool optSideCarHTML  = false;
	bool optSideCarXML   = false;
	bool optSideCarJSON  = false;
	bool optSideCarTTL   = false;
    bool optSuppressSelfIntersection = false;

	// PARSE command line options
	//--------------------------------------------------------------------------
	// https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
	static struct option longOptions[] = {
		{ "output-csv-file",              required_argument, nullptr, 'o' },
		{ "show-absolute-filename",       no_argument,       nullptr, 's' },
		{ "write-sidecar-file-html",      no_argument,       nullptr, 't' },
		{ "write-sidecar-file-xml",       no_argument,       nullptr, 'x' },
		{ "write-sidecar-file-json",      no_argument,       nullptr, 'j' },
		{ "write-sidecar-file-ttl",       no_argument,       nullptr, 'l' },
		{ "write-sidecar-files",          no_argument,       nullptr, 'a' },
		{ "overwrite-existing",           no_argument,       nullptr, 'k' },
        { "quick",                        no_argument,       nullptr, 'q' },
		{ "version",                      no_argument,       nullptr, 'v' },
		{ "help",                         no_argument,       nullptr, 'h' },
		{ "log-level",                    required_argument, nullptr,  0  },
		{ nullptr, 0, nullptr, 0 }
	};

	int character = 0;
	int optionIndex = 0;

    while( ( character = getopt_long_only( argc, argv, ":o:stkqxjlavh",
	         longOptions, &optionIndex ) ) != -1 ) {

        switch(character) {
			case 0:
				// printf ("option %s", long_options[option_index].name);
                //if (optarg) printf (" with arg %s", optarg);

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

			case 'o':
				fileNameCSVOut = std::string( optarg );
				break;

			case 's':
				optAbsolutePath = true;
				break;

			case 't':
				optSideCarHTML = true;
				break;

			case 'l':
				optSideCarTTL = true;
				break;

			case 'j':
				optSideCarJSON = true;
				break;

			case 'x':
				optSideCarXML = true;
				break;

            case 'q':
                optSuppressSelfIntersection = true;
                break;

			case 'a':
				optSideCarHTML = true;
				optSideCarTTL  = true;
				optSideCarJSON = true;
				optSideCarXML  = true;
				break;

			case 'k': // replaces output files
				std::cout << "[GigaMesh] Warning: files might be replaced!" << std::endl;
				//! \todo integrate optReplaceFiles = true;
				break;

			case 'v':
				std::cout << "GigaMesh Software Framework INFO 3D-data " << VERSION_PACKAGE << std::endl;
				std::cout << "Multi-threading with " << std::thread::hardware_concurrency() - 1 << " (dynamic) threads." << std::endl;
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

	// Process given files
	std::vector<MeshInfoData> fileInfosAll;
	unsigned long filesProcessed = 0;
	unsigned long filesMeshInfo = 0;
	for( int nonOptionArgumentCount = optind;
	     nonOptionArgumentCount < argc; nonOptionArgumentCount++ ) {

		std::string nonOptionArgumentString = std::string( argv[nonOptionArgumentCount] );

		if( !nonOptionArgumentString.empty() ) {
			std::cout << "[GigaMesh] Processing file " << nonOptionArgumentString << "..." << std::endl;

			MeshInfoData fileInfoSingle;
			if( !infoGigaMeshData( nonOptionArgumentString,
                                   fileInfoSingle,
                                   optSuppressSelfIntersection,
                                   optAbsolutePath
                                   ) ) {
				std::cerr << "[GigaMesh] ERROR: infoGigaMeshData failed!" << std::endl;
				std::exit( EXIT_FAILURE );
			}
			if( optSideCarHTML ) {
				//! \todo integrate optReplaceFiles (bool)
				// Determine filename for HTML sidecar file
				std::filesystem::path htmlFileName = std::filesystem::path( nonOptionArgumentString ).replace_extension( ".html" );
				if( fileInfoSingle.writeMeshInfo( htmlFileName ) ) {
					filesMeshInfo++;
				}
			}
			if( optSideCarXML ) {
				//! \todo integrate optReplaceFiles (bool)
				// Determine filename for XML sidecar file
				std::filesystem::path xmlFileName = std::filesystem::path( nonOptionArgumentString ).replace_extension( ".xml" );
				if( fileInfoSingle.writeMeshInfo( xmlFileName ) ) {
					filesMeshInfo++;
				}
			}
			if( optSideCarJSON ) {
				//! \todo integrate optReplaceFiles (bool)
				// Determine filename for JSON sidecar file
				std::filesystem::path jsonFileName = std::filesystem::path( nonOptionArgumentString ).replace_extension( ".json" );
				if( fileInfoSingle.writeMeshInfo( jsonFileName ) ) {
					filesMeshInfo++;
				}
			}
			if( optSideCarTTL ) {
				//! \todo integrate optReplaceFiles (bool)
				// Determine filename for TTL sidecar file
				std::filesystem::path ttlFileName = std::filesystem::path( nonOptionArgumentString ).replace_extension( ".ttl" );
				if( fileInfoSingle.writeMeshInfo( ttlFileName ) ) {
					filesMeshInfo++;
				}
			}
			fileInfosAll.push_back( fileInfoSingle );
			filesProcessed++;
		}
	}

	// Prepare header
	MeshInfoData fileInfo;
	std::string csvHeaderLine;
	for( unsigned long i=0; i<MeshInfoData::STRING_COUNT; i++ ) {
		std::string propName;
		fileInfo.getMeshInfoPropertyName( static_cast<MeshInfoData::eMeshPropertyString>(i), propName );
		csvHeaderLine += propName + ";";
	}
	for( unsigned long i=0; i<MeshInfoData::ULONG_COUNT; i++ ) {
		std::string propName;
		fileInfo.getMeshInfoPropertyName( static_cast<MeshInfoData::eMeshPropertyULongCount>(i), propName );
		csvHeaderLine += propName + ";";
	}
	for( unsigned long i=0; i<MeshInfoData::DOUBLE_COUNT; i++ ) {
		std::string propName;
		fileInfo.getMeshInfoPropertyName( static_cast<MeshInfoData::eMeshPropertyDouble>(i), propName );
		csvHeaderLine += propName + ";";
	}
	csvHeaderLine += "Connected components";

	// Prepare CSV conent
	std::string csvContent;
	for( auto const& currInfoSet: fileInfosAll ) {
		for( unsigned long i=0; i<MeshInfoData::STRING_COUNT; i++ ) {
			csvContent += currInfoSet.mStrings[i] + ";";
		}
		for( unsigned long i=0; i<MeshInfoData::ULONG_COUNT; i++ ) {
			csvContent += std::to_string( currInfoSet.mCountULong[i] ) + ";";
		}
		for( unsigned long i=0; i<MeshInfoData::DOUBLE_COUNT; i++ ) {
			csvContent += std::to_string( currInfoSet.mCountDouble[i] ) + ";";
		}
		csvContent += "\n";
	}

	// Output fetched data
	if( fileNameCSVOut.empty() ) {
		std::cout << csvHeaderLine << std::endl;
		std::cout << csvContent << std::endl;
	} else {
		// Write HTML to file.
		std::fstream fileStrOutCSV;
		fileStrOutCSV.open( fileNameCSVOut, std::fstream::out );
		if( fileStrOutCSV.is_open() ) {
			fileStrOutCSV << csvHeaderLine << std::endl;
			fileStrOutCSV << csvContent;
			fileStrOutCSV.close();
			std::cerr <<  "[GigaMesh] Information as CSV was written to: " << fileNameCSVOut << std::endl;
		} else {
			std::cerr <<  "[GigaMesh] ERROR: Failed to write to: " << fileNameCSVOut << std::endl;
			std::exit( EXIT_FAILURE );
		}
	}

	std::cout << "[GigaMesh] Meshinfo sidecar files written: " << filesMeshInfo << std::endl;
	std::cout << "[GigaMesh] Processed files:                " << filesProcessed << std::endl;
	std::exit( EXIT_SUCCESS );
}
