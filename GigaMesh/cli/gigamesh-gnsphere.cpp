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



bool convertMeshData(
                const filesystem::path&   rFileName,
                const filesystem::path&   rOutputPath,
                const filesystem::path&   rFileSuffix,
                const filesystem::path&   rInfoFileSuffix,
                int& rSubdivisionLevel,
                int& rXRotationAngle,
                int& rYRotationAngle,
                int& rZRotationAngle,
                double& rRadiusRecomputeNormals,
                const bool      rFaceNormals,
                const bool      rReplaceFiles,
                const bool      rCleanMesh,
                const bool      rInformationExport
) {
        if( rFileName.extension().wstring().size() != 4 ) {
                cerr << "[GigaMesh] ERROR: File extension '" << rFileName.extension().string() << "' is faulty!" << endl;
                return( false );
        }

        // Check: Input file exists?
        if( !std::filesystem::exists( rFileName ) ) {
            cerr << "[GigaMesh] Error: File '" << rFileName << "' not found!" << endl;
            return( false );
        }

        // create output path
        //get file name of original and append suffix
        std::filesystem::path fileNameOut = rFileName.stem();
        fileNameOut += rFileSuffix;
        //combine output path and input name
        // Output file for the csv with the gaussian normal sphere data.
        std::filesystem::path fileNameOutCSV( rOutputPath );
        fileNameOutCSV += fileNameOut;

        //add angle information
        if( rXRotationAngle != 0 || rYRotationAngle != 0 || rZRotationAngle != 0 ){
            string xAngString = to_string(rXRotationAngle);
            string yAngString = to_string(rYRotationAngle);
            string zAngString = to_string(rZRotationAngle);
            fileNameOutCSV += "_ANGLES:X";
            fileNameOutCSV += xAngString;
            fileNameOutCSV += "Y";
            fileNameOutCSV += yAngString;
            fileNameOutCSV += "Z";
            fileNameOutCSV += zAngString;
        }

        fileNameOutCSV += ".csv";
        if( std::filesystem::exists( fileNameOutCSV ) ) {
            if( !rReplaceFiles ) {
                cerr << "[GigaMesh] File '" << fileNameOutCSV << "' already exists!" << endl;
                return( false );
            }
            cout << "[GigaMesh] Warning: File '" << fileNameOutCSV << "' will be replaced!" << endl;
        }

        // All parameters OK => infos to stdout and file with metadata  -----------------------------------------------------------
        cout << "[GigaMesh] File IN:         " << rFileName << endl;
        cout << "[GigaMesh] File OUT/Prefix: " << fileNameOut << endl;


        // Prepare data structures
        //--------------------------------------------------------------------------
        bool readSucess;
        Mesh someMesh( rFileName, readSucess );
        if( !readSucess ) {
                cerr << "[GigaMesh] Error: Could not open file '" << rFileName << "'!" << endl;
                return( false );
        }

        //recompute normals
        if( rRadiusRecomputeNormals <= 0.0 ) {
            someMesh.resetVertexNormals();
        } else {
            someMesh.normalsVerticesComputeSphere( rRadiusRecomputeNormals );
        }

        //rotation
        //--------------------------------------------------------------------------
        if( rXRotationAngle != 0 || rYRotationAngle != 0 || rZRotationAngle != 0 ){
            vector<double> matrixParam;
            //constructor of the Matrix4D class needs a vector
            matrixParam.clear();
            double angleRadians = rXRotationAngle * (M_PI/180);
            matrixParam.push_back(angleRadians);
            Matrix4D xRotationMatrix = Matrix4D(Matrix4D::INIT_ROTATE_ABOUT_X,&matrixParam);
            //someMesh.applyTransformationToWholeMesh(xRotationMatrix);
            matrixParam.clear();
            angleRadians = rYRotationAngle * (M_PI/180);
            matrixParam.push_back(angleRadians);
            Matrix4D yRotationMatrix = Matrix4D(Matrix4D::INIT_ROTATE_ABOUT_Y,&matrixParam);
            //someMesh.applyTransformationToWholeMesh(yRotationMatrix);
            matrixParam.clear();
            angleRadians = rZRotationAngle * (M_PI/180);
            matrixParam.push_back(angleRadians);
            Matrix4D zRotationMatrix = Matrix4D(Matrix4D::INIT_ROTATE_ABOUT_Z,&matrixParam);
            Matrix4D transformationMatrix = xRotationMatrix*yRotationMatrix*zRotationMatrix;
            someMesh.applyTransformationToWholeMesh(transformationMatrix);
        }
        //--------------------------------------------------------------------------
        //prepare normal data
        bool sphereCoordinates = true;

        std::list<sVertexProperties> vertexProps;

        auto fAddNormal = [&vertexProps](const Vector3D& normal) -> void {
            if( std::isnan(normal.getX()) || std::isnan(normal.getY()) || std::isnan(normal.getZ()) )
            {
                return;
            }
            sVertexProperties prop;
            prop.mNormalX = normal.getX();
            prop.mNormalY = normal.getY();
            prop.mNormalZ = normal.getZ();

            vertexProps.emplace_back(prop);
        };
        if(rFaceNormals)
        {
            auto faceCount = someMesh.getFaceNr();

            for( uint64_t faceIdx = 0; faceIdx < faceCount; ++faceIdx)
            {
                Face* currFace = someMesh.getFacePos( faceIdx );
                auto normal = currFace->getNormal(false);
                fAddNormal(normal);
            }
        }
        else
        {
            auto vertCount = someMesh.getVertexNr();

            for( uint64_t vertIdx=0; vertIdx<vertCount; vertIdx++ ) {
                Vertex* currVertex = someMesh.getVertexPos( vertIdx );
                auto normal = currVertex->getNormal(false);
                fAddNormal(normal);
            }
        }

        //------------------------------------------------------
        // Write data to file

        time_t     rawtime;
        struct tm* timeinfo;
        time( &rawtime );
        timeinfo = localtime( &rawtime );
        cout << "[GigaMesh] Start date/time is: " << asctime( timeinfo );// << endl;
        someMesh.writeIcoNormalSphereData(fileNameOutCSV, vertexProps , rSubdivisionLevel, sphereCoordinates);
        timeinfo = localtime( &rawtime );
        cout << "[GigaMesh] End date/time is: " << asctime( timeinfo );// << endl;
        if(rCleanMesh == true){
            //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            // Clean mesh and calculate volume
            //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            cout << "[GigaMesh] Start cleaning procedure" << endl;
            // Initial numbers of primitives
            auto oldVertexNr = someMesh.getVertexNr();
            auto oldFaceNr = someMesh.getFaceNr();

            //! \todo define input parameters for each variable
            //static variables
            double percentArea = 0.1;
            bool applyBorderErosion = true;
            bool skipLargestHole = false;
            bool keepLargestComponent = false;
            unsigned long maxNumberVertices = 3000;
            uint64_t iterationCount = 0;

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

            //cleaning procedure
            someMesh.completeRestore( "", percentArea, applyBorderErosion,
                                      skipLargestHole, maxNumberVertices,
                                      nullptr, iterationCount ); // use fileNameOut instead of "" for saving the intermediate mesh.



            std::cout << "[GigaMesh] POLISH: Vertex count changed by " << static_cast<long>(someMesh.getVertexNr())-static_cast<long>(oldVertexNr) << std::endl;
            std::cout << "[GigaMesh]         Face count changed by   " << static_cast<long>(someMesh.getFaceNr())-static_cast<long>(oldFaceNr) << std::endl;

            // Prepare for editing holes i.e. false-positiv filled connected components
            //-------------------------------------------------------------------------
            someMesh.selVertByFlag( Primitive::FLAG_SYNTHETIC );
            someMesh.labelSelectedVerticesBackGrd();
        }



        if( rInformationExport ){
            //calculate volume
            // Count primitives and their properties
            MeshInfoData meshInfoData;
            if( !someMesh.getMeshInfoData( meshInfoData, true ) ) {
                std::wcerr << "[GigaMesh] ERROR: Could not fetch mesh information about '" << rFileName.wstring() << "'!" << std::endl;
                return( false );
            }
            // create output path
            //get file name of original and append suffix
            std::filesystem::path infoFileNameOut = rFileName.stem();
            infoFileNameOut += rInfoFileSuffix;
            //combine output path and input name
            // Output file for the mesh information
            std::filesystem::path infoFileOutJSON( rOutputPath );
            infoFileOutJSON += infoFileNameOut;

            //add angle information
            if( rXRotationAngle != 0 || rYRotationAngle != 0 || rZRotationAngle != 0 ){
                string xAngString = to_string(rXRotationAngle);
                string yAngString = to_string(rYRotationAngle);
                string zAngString = to_string(rZRotationAngle);
                infoFileOutJSON += "_ANGLES:X";
                infoFileOutJSON += xAngString;
                infoFileOutJSON += "Y";
                infoFileOutJSON += yAngString;
                infoFileOutJSON += "Z";
                infoFileOutJSON += zAngString;
            }

            infoFileOutJSON += ".json";
            meshInfoData.writeMeshInfo(infoFileOutJSON);
        }
        return( true );
}

//! Help i.e. usage of paramters.
void printHelp( const char* rExecName ) {
        std::cout << "Usage: " << rExecName << " [options] (<file>)" << std::endl;
        std::cout << "GigaMesh Software Framework export gaussian normal sphere data " << std::endl << std::endl;
        std::cout << "Exports the gaussian normal sphere data of the given mesh" << std::endl;
        std::cout << std::endl << std::endl;
        std::cout << "Options:" << endl;
        std::cout << "  -h, --help                              Displays this help." << std::endl;
        std::cout << "  -v, --version                           Displays version information." << std::endl << std::endl;
        std::cout << "  -l, --subdivision-level                 subdivision-level of the export" << std::endl;
        std::cout << "  -n, --normals-recompution-radius <float>radius to recompute the normals. Default 0.0" << std::endl;
        std::cout << "  -f, --face-normals                      Export the face normals. Default: Vertex Normals" << std::endl;
        std::cout << "  -c, --clean-mesh                        Activates the mesh cleaning procedure before the volume calculation " << std::endl;
        std::cout << "  -e, --mesh-information-export           Export the mesh information as json file" << std::endl;
        std::cout << "  -o, --output-path <string>              Path to save the export" << std::endl;
        std::cout << "  -i, --info-suffix <string>              Write the exported information file of the mesh with the given <string> as suffix for its name." << std::endl;
        std::cout << "                                          Default suffix is '_info' " << std::endl;
        std::cout << "  -s, --output-suffix <string>            Write the exported GNS file using the given <string> as suffix for its name." << std::endl;
        std::cout << "                                          Default suffix is '_GNS' " << std::endl;
        std::cout << "  -k, --overwrite-existing                Overwrite exisitng files, which is not done by default" << std::endl;
        std::cout << "                                          to prevent accidental data loss." << std::endl;
        std::cout << "  -r, --recursive-iteration               Move through all subdirectories of the input path" << std::endl;
        std::cout << "                                          All '.ply' files and '.obj' files in this directories will be used for the export. " << std::endl;
        std::cout << "  -x, --x-rotation-angle <int>            Rotate the mesh about the x-axis with this angle in degree" << std::endl;
        std::cout << "                                          Default angle is 0." << std::endl;
        std::cout << "  -y, --y-rotation-angle <int>            Rotate the mesh about the y-axis with this angle in degree" << std::endl;
        std::cout << "                                          Default angle is 0." << std::endl;
        std::cout << "  -z, --z-rotation-angle <int>            Rotate the mesh about the z-axis with this angle in degree" << std::endl;
        std::cout << "                                          Default angle is 0." << std::endl;
        std::cout << "                                          if any rotation is used, then the file gets 'ANGLES:X<angle>Y<angle>Z<angle>' as suffix." << std::endl;
}

//! Main routine for loading a (binary) PLY and compute gaussian normal sphere supplied by GigaMesh
//==============================================================================================================================================================
int main( int argc, char *argv[] ) {
        LOG::initLogging();

        //error cases variables
        int numberOfErrorCases = 0;

        // Default string parameter
        std::filesystem::path optOutputPath;
        std::filesystem::path optFileSuffix;
        std::filesystem::path optInfoSuffix;

        // Default flags
        bool optFaceNormals = false;
        bool optReplaceFiles = false;
        bool optRecursiveIteration = false;
        bool optCleanMesh = false;
        bool optInformationExport = false;


        //Default integer Parameter
        int optSubdivisionLevel = 6;
        int optXRotationAngle = 0;
        int optYRotationAngle = 0;
        int optZRotationAngle = 0;

        //Default double parameters
        double optRadiusRecomputeNormals{0.0};


        // PARSE command line options
        //--------------------------------------------------------------------------
        // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
        static struct option longOptions[] = {
                { "output-suffix",                required_argument, nullptr, 's' },
                { "info-suffix",                required_argument, nullptr, 'i' },
                { "output-path",                required_argument, nullptr, 'o' },
                { "subdivision-level",                     required_argument, nullptr, 'l'},
                { "normals-recompution-radius",           required_argument, nullptr, 'n'},
                { "x-rotation-angle",           required_argument, nullptr, 'x'},
                { "y-rotation-angle",           required_argument, nullptr, 'y'},
                { "z-rotation-angle",           required_argument, nullptr, 'z'},
                { "face-normals",           no_argument,       nullptr, 'f' },
                { "recursive-iteration",           no_argument,       nullptr, 'r' },
                { "mesh-information-export",           no_argument,       nullptr, 'e' },
                { "clean-mesh",           no_argument,       nullptr, 'c' },
                { "version",                      no_argument,       nullptr, 'v' },
                { "help",                         no_argument,       nullptr, 'h' },
                { nullptr, 0, nullptr, 0 }
        };

        int character = 0;
        int optionIndex = 0;
        while( ( character = getopt_long_only( argc, argv, ":l:n:o:s:i:x:y:z:freckvh",
                 longOptions, &optionIndex ) ) != -1 ) {

                switch(character) {

                        case 'o': // output path
                                optOutputPath = std::string( optarg );
                                break;

                        case 's': // optional file suffix
                                optFileSuffix = std::string( optarg );
                                break;

                        case 'i': // optional info file suffix
                                optInfoSuffix = std::string( optarg );
                                break;

                        case 'f': // export face normals
                                optFaceNormals = true;
                                break;

                        case 'l': // subdivision level
                                optSubdivisionLevel = stoi(std::string( optarg ));
                                break;

                        case 'x': // x rotation angle
                                optXRotationAngle = stoi(std::string( optarg ));
                                break;
                        case 'y': // y rotation angle
                                optYRotationAngle = stoi(std::string( optarg ));
                                break;
                        case 'z': // z rotation angle
                                optZRotationAngle = stoi(std::string( optarg ));
                                break;

                        case 'n': // recompute normals radius
                                optRadiusRecomputeNormals = stod(std::string( optarg ));
                                break;

                        case 'k': // replaces output files
                                std::cout << "[GigaMesh] Warning: files might be replaced!" << std::endl;
                                optReplaceFiles = true;
                                break;

                        case 'r': // recursive directory iteration
                                optRecursiveIteration = true;
                                break;

                        case 'e': // export mesh informatation
                                optInformationExport = true;
                                break;


                        case 'c': // cleaning procedure before volume calculation
                                optCleanMesh = true;
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

        // Add default output path
        if( optOutputPath.empty() ) {
                optOutputPath = "./";
        }


        // Add default gns file suffix
        if( optFileSuffix.empty() ) {
            optFileSuffix = "_GNS";
        }
        // Add default info file suffix
        if( optInfoSuffix.empty() ) {
            optInfoSuffix = "_info";
        }

        // SHOW Build information
        printBuildInfo();


        // Process given files
        unsigned long filesProcessed = 0;
        for( int nonOptionArgumentCount = optind;
             nonOptionArgumentCount < argc; nonOptionArgumentCount++ ) {

                std::filesystem::path nonOptionArgumentString ( argv[nonOptionArgumentCount] );

                if( !nonOptionArgumentString.empty() ) {
                    try{
                        //non recursive directory iteration
                        if(!optRecursiveIteration){
                            std::cout << "[GigaMesh] Processing file " << nonOptionArgumentString << "..." << std::endl;

                            if( !convertMeshData( nonOptionArgumentString, optOutputPath, optFileSuffix, optInfoSuffix, optSubdivisionLevel,
                                                  optXRotationAngle, optYRotationAngle, optZRotationAngle, optRadiusRecomputeNormals,
                                                  optFaceNormals, optReplaceFiles, optCleanMesh, optInformationExport )) {
                                std::cerr << "[GigaMesh] ERROR: export Normalsphere failed!" << std::endl;
                                //std::exit( EXIT_FAILURE );
                            }
                            filesProcessed++;
                        }
                        else{
                            //recursive directory iteration
                            for (auto const& dir_entry : std::filesystem::recursive_directory_iterator(nonOptionArgumentString.parent_path())){
                                //only use 3D-objects. the recursive iterator returns also directry names and other files
                                string fileExtension = dir_entry.path().extension().string();
                                //file extensio to lower (transform is not working)
                                for (int iter = 0; iter < fileExtension.length(); iter++) {
                                    fileExtension[iter] = tolower(fileExtension[iter]);
                                }
                                //std::transform(fileExtension , fileExtension.end(), fileExtension.begin(), ::tolower);
                                if( fileExtension == ".ply" or fileExtension == ".obj"){
                                    std::cout << "[GigaMesh] Processing file " << dir_entry.path() << "..." << std::endl;

                                    if( !convertMeshData( dir_entry.path(), optOutputPath, optFileSuffix, optInfoSuffix, optSubdivisionLevel,
                                                          optXRotationAngle, optYRotationAngle, optZRotationAngle, optRadiusRecomputeNormals,
                                                          optFaceNormals, optReplaceFiles, optCleanMesh, optInformationExport) ) {
                                        std::cerr << "[GigaMesh] ERROR: export Normalsphere failed!" << std::endl;
                                        //std::exit( EXIT_FAILURE );
                                    }
                                    filesProcessed++;
                                }
                            }
                        }
                    }
                    //count the error cases
                    catch(const char* message){
                        numberOfErrorCases++;
                    }
                }
        }

        std::cout << "[GigaMesh] Processed files: " << filesProcessed << std::endl;
        std::cout << "[GigaMesh] Number of error cases: " << numberOfErrorCases << std::endl;
        std::exit( EXIT_SUCCESS );
}
