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

#include <stdio.h>
#include <stdlib.h> // calloc
#include <string>
#include <algorithm>
#include <iostream>
#include <functional>
#include <cctype>
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


#include <GigaMesh/printbuildinfo.h>
#include <GigaMesh/mesh/mesh.h>
#include <GigaMesh/logging/Logging.h>
//#include "meshseed.h"

// //#include "voxelcuboid.h"
//#include "voxelfilter25d.h"

#include <sys/stat.h> // statistics for files

using namespace std;

bool splitMeshInComponents(
                const filesystem::path&   rFilePath,
                const bool      rExportInfo
) {
        if( rFilePath.extension().wstring().size() != 4 ) {
                cerr << "[GigaMesh] ERROR: File extension '" << rFilePath.extension().string() << "' is faulty!" << endl;
                return( false );
        }

        // Check: Input file exists?
        if( !std::filesystem::exists( rFilePath ) ) {
            cerr << "[GigaMesh] Error: File '" << rFilePath << "' not found!" << endl;
            return( false );
        }

        //check: txt file with label information of this mesh exist
        //same name but 'txt' instead of 'ply'

        //create the path of the label information file
        std::filesystem::path labelInfoFilePath = rFilePath;
        labelInfoFilePath.replace_extension( filesystem::path( "txt" ) );
        bool labelFileExist = false;
        if( std::filesystem::exists( labelInfoFilePath ) ) {
            labelFileExist = true;
        }

        // All parameters OK => infos to stdout and file with metadata  -----------------------------------------------------------
        cout << "[GigaMesh] File IN:         " << rFilePath << endl;



        // load mesh, Prepare data structures
        //--------------------------------------------------------------------------
        bool readSucess;
        Mesh someMesh( rFilePath, readSucess );
        if( !readSucess ) {
                cerr << "[GigaMesh] Error: Could not open file '" << rFilePath << "'!" << endl;
                return( false );
        }

        //read or compute labels
        //--------------------------------------------------------------------------

        // if no labels information file inside the same directory
        // --> labels have to be computed
        if (!labelFileExist){
            if( !someMesh.labelVerticesAll() ) {
                std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: labeling failed!" << std::endl;
                return( false );
            }
            std::cout << "[Mesh::" << __FUNCTION__ << "] labels computed " << std::endl;
        }
        else{
            //read label info file
            if ( !someMesh.importLabelsFromFile(labelInfoFilePath,true) ){
                std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Import labels failed!" << std::endl;
                return( false );
            }
            std::cout << "[Mesh::" << __FUNCTION__ << "] labels import was succesful " << std::endl;
        }

        //get label numbers
        //--------------------------------------------------------------------------
        //TODO/idea: use only the labels with a minimum component size
        //someMesh.selectVertLabelAreaLT( 5000.0 );

        std::set<uint64_t> labelNrs;
        //ATTENTION METHOD: getMSelectedMVerts returns all mVerts
        std::cout.setstate(std::ios_base::failbit); //needed for cout clear
        if( !someMesh.getVertLabelIdFrom( someMesh.getVertices(), labelNrs ) ) {
            std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: getVertLabelIdFrom failed!" << std::endl;
            return( false );
        }
        std::cout.clear(); //clear the cout spam of Methdo getVertLabelIdFrom
        std::cout << "[Mesh::" << __FUNCTION__ << "] Amount of labels: " << labelNrs.size() << std::endl;

        // Inform the user if there are no connected components defined
        if( labelNrs.size() == 0 ) {
            std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: No connected components (labels) defined!";
            return( false );
        }
        uint fileError = 0;
        uint fileOkay = 0;
        uint infoFileError = 0;
        for( auto const& labelNr: labelNrs ) {
            std::set<Face*> facesWithLabel;
            bool retVal = someMesh.getFaceHasVertLabelNo( labelNr, facesWithLabel );
            if( retVal ) {
                filesystem::path fileName = someMesh.getFullName();

                // The following looks wired due to Windows build
                std::string oriExtension = fileName.extension().string();
                std::string suffixExtension = "comp.";
                suffixExtension += std::to_string( labelNr );
                suffixExtension += oriExtension;

                fileName.replace_extension( filesystem::path( suffixExtension ) );
                Mesh meshToWrite( &facesWithLabel );
                meshToWrite.getModelMetaDataRef().setModelMeta( someMesh.getModelMetaDataRef() );
                if( meshToWrite.writeFile( fileName ) ) {
                    std::cout << "[Mesh::" << __FUNCTION__ << "] Connected component written to file: " << fileName.string() << std::endl;
                    fileOkay++;
                } else {
                    std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Connected component NOT written to file " << fileName.string() << "!" << std::endl;
                    fileError++;
                }

                //write the information e.g. volume of the mesh component to json file
                if( rExportInfo ){
                    MeshInfoData meshInfoData;
                    if( !meshToWrite.getMeshInfoData( meshInfoData, true ) ) {
                        std::wcerr << "[GigaMesh] ERROR: Could not fetch mesh information about '" << fileName.wstring() << "'!" << std::endl;
                        infoFileError++;
                    }
                    std::filesystem::path infoFileOutJSON = someMesh.getFullName();
                    std::string suffixExtensionInfo = "comp.";
                    suffixExtensionInfo += std::to_string( labelNr );
                    suffixExtensionInfo += "_info.json";
                    infoFileOutJSON.replace_extension( filesystem::path( suffixExtensionInfo ) );
                    meshInfoData.writeMeshInfo(infoFileOutJSON);
                }
            }
        }

        // Notify the user about errors.
        if( fileError > 0 ) {
            std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Error writing files occured!";
            return( false );
        }

        if( infoFileError > 0 ) {
            std::cerr << "[Mesh::" << __FUNCTION__ << "] ERROR: Error writing information files of the components occured!";
            return( false );
        }

        return( true );
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName ) {
        std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
        std::cout << "GigaMesh Software Framework save the labeled components seperatly " << std::endl << std::endl;
        std::cout << "Saves each component as <filename>.comp.<label ID>.ply " << std::endl;
        std::cout << "If a <filename>.txt with label information exist in the same directory, then the label will be used to describe the components." << std::endl;
        std::cout << std::endl << std::endl;
        std::cout << "Options:" << endl;
        std::cout << "  -h, --help                              Displays this help." << std::endl;
        std::cout << "  -v, --version                           Displays version information." << std::endl << std::endl;
        std::cout << "  -i, --info                              Write the mesh informations of the components in <filename>.comp.<label ID>_info.json" << std::endl;
        std::cout << "  -r, --recursive-iteration               Move through all subdirectories of the input path" << std::endl;
        std::cout << "                                          All '.ply' files will be used for the mesh split in components " << std::endl;
}

//! Main routine for loading a (binary) PLY and save the componentes in a separate file
//! If a file with the same name and the suffix .txt exist, then this file will be used for the the label information
//! otherwise the labels will be computed in splitMeshInComponents
//! The Labels File must have vertex indices
//==============================================================================================================================================================
int main( int argc, char *argv[] ) {
        LOG::initLogging();

        // Default flags
        bool optExportInfos = false;
        bool optRecursiveIteration = false;

        // PARSE command line options
        //--------------------------------------------------------------------------
        // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
        static struct option longOptions[] = {
                { "info",                required_argument, nullptr, 'i' },
                { "recursive-iteration",           no_argument,       nullptr, 'r' },
                { "version",                      no_argument,       nullptr, 'v' },
                { "help",                         no_argument,       nullptr, 'h' },
                { nullptr, 0, nullptr, 0 }
        };

        int character = 0;
        int optionIndex = 0;
        while( ( character = getopt_long_only( argc, argv, ":irvh",
                 longOptions, &optionIndex ) ) != -1 ) {

                switch(character) {

                        case 'i': // optional info file suffix
                                optExportInfos = true;
                                break;

                        case 'r': // recursive directory iteration
                                optRecursiveIteration = true;
                                break;

                        case 'v':
                                std::cout << "GigaMesh Software Framework GNSPHERE Stanford Polygon Files (PLYs) " << VERSION_PACKAGE << endl;
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

            std::filesystem::path nonOptionArgumentString ( argv[nonOptionArgumentCount] );

            if( !nonOptionArgumentString.empty() ) {
                //non recursive directory iteration
                if(!optRecursiveIteration){
                    std::cout << "[GigaMesh] Processing file " << nonOptionArgumentString << "..." << std::endl;

                    if( !splitMeshInComponents( nonOptionArgumentString, optExportInfos)) {
                        std::cerr << "[GigaMesh] ERROR: split mesh in components per label failed!" << std::endl;
                        //std::exit( EXIT_FAILURE );
                    }
                    filesProcessed++;
                }
                else{
                    //recursive directory iteration
                    for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(nonOptionArgumentString.parent_path())){
                        //only use 3D-objects. the recursive iterator returns also directry names and other files
                        string fileExtension = dir_entry.path().extension().string();
                        //file extension to lower (transform is not working)
                        for (int iter = 0; iter < fileExtension.length(); iter++) {
                            fileExtension[iter] = tolower(fileExtension[iter]);
                        }
                        //std::transform(fileExtension , fileExtension.end(), fileExtension.begin(), ::tolower);
                        if( fileExtension == ".ply"){
                            std::cout << "[GigaMesh] Processing file " << dir_entry.path() << "..." << std::endl;

                            if( !splitMeshInComponents( dir_entry.path(),  optExportInfos) ) {
                                std::cerr << "[GigaMesh] ERROR: split mesh in components per label failed!" << std::endl;
                                //std::exit( EXIT_FAILURE );
                            }
                            filesProcessed++;
                        }
                    }
                }
            }

        }


        std::cout << "[GigaMesh] Processed files: " << filesProcessed << std::endl;
        std::exit( EXIT_SUCCESS );
}
