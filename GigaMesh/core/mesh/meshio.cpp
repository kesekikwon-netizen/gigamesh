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

#include <GigaMesh/mesh/meshio.h>

#include <chrono>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <list>

#include <algorithm>  // e.g. ::transform
#include <sstream>    // stringstream
#include <iterator>   // istream_iterator

#include <cstdlib>
#include <cstdio>
#include <clocale>
#include <cctype>

#include <filesystem>
#include <chrono>

#include <GigaMesh/mesh/primitive.h>
#include <GigaMesh/icoSphereTree/IcoSphereTree.h>
#include "MeshIO/ObjReader.h"
#include "MeshIO/PlyReader.h"
#include "MeshIO/TxtReader.h"
#include "MeshIO/RegularGridTxtReader.h"

#include "MeshIO/PlyWriter.h"
#include "MeshIO/ObjWriter.h"
#include "MeshIO/VRMLWriter.h"
#include "MeshIO/TxtWriter.h"
#include "MeshIO/gltfWriter.h"


#include "util/triangulation.h"

#include <GigaMesh/logging/Logging.h>

using namespace std;

//! Constructor
//! checks for endianness as suggested here: http://www.ibm.com/developerworks/aix/library/au-endianc/index.html?ca=drs-#list5
MeshIO::MeshIO()
{
	int  intTest = 1;
	mSystemIsBigEndian = ( (*reinterpret_cast<char*>(&intTest)) == 0 );
	if( mSystemIsBigEndian ) {
		LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] System is BIG Endian.\n";
	} else {
		LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] System is LITTLE Endian.\n";
	}

	// Init flags:
	for( bool& exportFlag : mExportFlags ) {
		exportFlag = true;
	}

	// Initialize strings holding meta-data
	if( !mModelMetaData.clearModelMetaStrings() ) {
		LOG::error() << "[MeshIO::" << __FUNCTION__ << "] ERROR: clearModelMetaStrings failed!\n";
	}
}

// READ ------------------------------------------------------------------------

//triangulate ngon-faces
void triangulateFaces(std::vector<sFaceProperties>& rFaceProps, const std::vector<sVertexProperties>& rVertexProps)
{
	std::queue<sFaceProperties> newFaces;

	for(auto& faceProp : rFaceProps)
	{
		if(faceProp.vertexIndices.size() > 3)
		{
			std::vector<Vector3D> vertices;
			vertices.reserve(faceProp.vertexIndices.size());

			for(auto index : faceProp.vertexIndices)
			{
				vertices.emplace_back(Vector3D(rVertexProps[index].mCoordX,
				                               rVertexProps[index].mCoordY,
				                               rVertexProps[index].mCoordZ));
			}

			auto newIndices = GigaMesh::Util::triangulateNgon(vertices);

			//construct new faces
			for(size_t i = 3; i < newIndices.size(); i+=3)
			{
				sFaceProperties newProperties;
				newProperties.vertexIndices.resize(3);
				for(size_t j = 0; j<3; ++j)
				{
					newProperties.vertexIndices[j] = faceProp.vertexIndices[newIndices[i+j]];
				}

				if(!faceProp.textureCoordinates.empty())
				{
					newProperties.textureCoordinates.resize(6);
					for(size_t j = 0; j<3; ++j)
					{
						newProperties.textureCoordinates[j * 2    ] = faceProp.textureCoordinates[newIndices[i+j] * 2    ];
						newProperties.textureCoordinates[j * 2 + 1] = faceProp.textureCoordinates[newIndices[i+j] * 2 + 1];
					}
				}
				newProperties.textureId = faceProp.textureId;
				newFaces.emplace(newProperties);
			}

			//resize current face
			std::vector<uint64_t> tmpIndices(3);

			for(size_t i = 0; i < 3; ++i)
			{
				tmpIndices[i] = faceProp.vertexIndices[newIndices[i]];
			}

			faceProp.vertexIndices = tmpIndices;

			if(!faceProp.textureCoordinates.empty())
			{
				std::vector<float> tmpTexCoods(6);

				for(size_t i = 0; i<3; ++i)
				{
					tmpTexCoods[i * 2    ] = faceProp.textureCoordinates[newIndices[i] * 2    ];
					tmpTexCoods[i * 2 + 1] = faceProp.textureCoordinates[newIndices[i] * 2 + 1];
				}

				faceProp.textureCoordinates = tmpTexCoods;
			}
		}
	}

	rFaceProps.reserve(rFaceProps.size() + newFaces.size());
	while(!newFaces.empty())
	{
		rFaceProps.emplace_back(newFaces.front());
		newFaces.pop();
	}
}

//! Reads a file with a 3D-mesh. This method wraps around the other read-methods.
//! The method for reading a file is automatically choosen by its file extension.
//!
//! @returns false in case of an error. True otherwise.
bool MeshIO::readFile(
                const filesystem::path& rFileName,
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	// Extension - lower case and without dot
	std::string fileExtension = rFileName.extension().string();
	for( char& character : fileExtension ) {
		character = static_cast<char>( std::tolower( static_cast<unsigned char>( character ) ) );
	}

	if( fileExtension.empty())
	{
		return false;
	}

	if( fileExtension.at(0) == '.' ) {
		fileExtension.erase( fileExtension.begin() );
	}
	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] Extension (lowercase): " << fileExtension << "\n";

	bool readStatus = false;
	std::unique_ptr<MeshReader> reader = nullptr;
	if( fileExtension == "obj" ) {
		reader = std::make_unique<ObjReader>();
	} else if( fileExtension == "txt" ) {
		bool askRegular;
		if( !readIsRegularGrid( &askRegular ) ) {
			LOG::error() << "[MeshIO::" << __FUNCTION__ << "] Read TXT file canceled!\n";
			return( false );
		}
		if( askRegular ) {
			reader = std::make_unique<RegularGridTxtReader>();
		} else {
			reader = std::make_unique<TxtReader>();
		}
	} else if( fileExtension == "xyz" ) {
		reader = std::make_unique<TxtReader>();
	} else if( fileExtension == "ply" ) {
		reader = std::make_unique<PlyReader>();
		dynamic_cast<PlyReader*>(reader.get())->setIsBigEndian(mSystemIsBigEndian);
	} else {
		LOG::error() << "[MeshIO::" << __FUNCTION__ << "] Error: Unknown extension/type '" << fileExtension << "' specified!\n";
		return( false );
	}

	if(reader != nullptr)
	{
		readStatus = reader->readFile(rFileName, rVertexProps, rFaceProps, *this);
	}

	if( !readStatus ) {
		LOG::error() << "[MeshIO::" << __FUNCTION__ << "] ERROR: Read failed!\n";
		return( false );
	}

	triangulateFaces(rFaceProps, rVertexProps);

	mModelMetaData = reader->getModelMetaDataRef();

	if(mModelMetaData.hasTextureFiles())
	{
		auto prevPath = std::filesystem::current_path();
		std::filesystem::current_path(std::filesystem::absolute(rFileName).parent_path());

		std::filesystem::path texturePath(mModelMetaData.getModelMetaString(ModelMetaData::META_TEXTUREFILE));

		for(auto& textureFileString : mModelMetaData.getTexturefilesRef())
		{
			std::filesystem::path texturePath(textureFileString);
			if(texturePath.is_relative())
			{
				textureFileString = std::filesystem::absolute(texturePath).string();
			}
		}
		std::filesystem::current_path(prevPath);
	}

	mExportFlags[EXPORT_TEXTURE_COORDINATES] = mModelMetaData.hasTextureCoordinates();

	// Store filename with absolute path.
	mFileNameFull = std::filesystem::absolute( rFileName );
	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] File - Absolute:    " << mFileNameFull << "\n";
	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] File - Stem:        " << std::filesystem::path( mFileNameFull ).stem() << "\n";
	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] File - Extension:   " << std::filesystem::path( mFileNameFull ).extension() << "\n";

	return( true );
}

//! Ask the user if a regular grid is to be read.
//! Has to be overloaded by GUI class e.g. MeshQt.
//! Returns false by default.
bool MeshIO::readIsRegularGrid( bool* rIsGrid ) {
	char userAnswer = 'n';
	std::cout << "[MeshIO::" << __FUNCTION__ << "] USER Input: Is this file a regular grid [y/N]? ";
	std::cin >> userAnswer;
	if( userAnswer == 'y' || userAnswer == 'Y' ) {
		(*rIsGrid) = true;
		LOG::info() << "[MeshIO::" << __FUNCTION__ << "] Your answer was: YES.\n";
	} else {
		(*rIsGrid) = false;
		LOG::info() << "[MeshIO::" << __FUNCTION__ << "] Your answer was: NO.\n";
	}
	return( true );
}

// Texturemap ----------------------------------------------------------------------

//! Imports a texture map (per Vertex) with the format int originalIndex, float red,
//! float green, float blue from a given ASCII file.
//!
//! Note missing Vertex indices will get a default color (specified elsewhere).
//!
//! @returns false in case of an error. True otherwise.
bool MeshIO::importTEXMap(const filesystem::path&          rFileName,
                           int*            rNrLines,
                           uint64_t** rRefToPrimitves,
                           unsigned char** rTexMap
) {
	fstream filestr;
	string  lineToParse;

	int timeStart, timeStop; // for performance mesurement

	timeStart = clock();
	filestr.open( rFileName, fstream::in );
	if( !filestr.is_open() ) {
		LOG::error() << "[MeshIO::" << __FUNCTION__ << "] Could not open: '" << rFileName << "'!\n";
		return( false );
	}
	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] Open: '" << rFileName << "'\n";

	// count lines to be read to ....
	while( !filestr.eof() ) {
		getline( filestr, lineToParse );
		(*rNrLines)++;
	}
	(*rNrLines)--;
	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] Lines to read: " << (*rNrLines) << ".\n";

	// initalize values:
	*rNrLines        = 0;
	*rRefToPrimitves = nullptr;
	*rTexMap         = nullptr;
	float texRtmp;
	float texGtmp;
	float texBtmp;

	// ... allocate enough memory:
	*rRefToPrimitves = static_cast<uint64_t*>(calloc( (*rNrLines), sizeof(uint64_t) ));
	*rTexMap = static_cast<unsigned char*>(calloc( (*rNrLines)*3, sizeof(unsigned char) ));

	filestr.clear(); // since the stream object has encountered eof it is in a bad state
	filestr.seekg( 0, ios::beg ); // now rewind (won't work without the previous call of clear).

	for( int i=0; i<(*rNrLines); i++ ) {
		getline( filestr, lineToParse );
		int refToPrimitveIdx;
		if( sscanf( lineToParse.c_str(), "%i %f %f %f", &refToPrimitveIdx, &texRtmp, &texGtmp, &texBtmp ) == 4 ) {
			(*rRefToPrimitves)[i] = static_cast<uint64_t>(refToPrimitveIdx);
			(*rTexMap)[i*3]   = static_cast<unsigned char>(texRtmp*255);
			(*rTexMap)[i*3+1] = static_cast<unsigned char>(texGtmp*255);
			(*rTexMap)[i*3+2] = static_cast<unsigned char>(texBtmp*255);
		} else {
			LOG::warn() << "[MeshIO::" << __FUNCTION__ << "] Problem in parsing line no. " << i << " of " << rFileName << "\n";
		}
	}

	filestr.close();

	timeStop  = clock();
	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] took " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds.\n";
	
	return( true );
}

//! Import an ASCII file with normal vectors.
//! Expected format: <integer/primitive_id> <double/x-component> <double/y-component> <double/z-component>
bool MeshIO::importNormals( const filesystem::path& rFileName, vector<grVector3ID>* rNormals ) {
	// Prepare
	std::fstream fileStream;
	fileStream.open( rFileName, std::fstream::in );
	if( !fileStream.is_open() ) {
		LOG::error() << "[MeshIO::" << __FUNCTION__ << "] ERROR: Could not open " << rFileName << "!\n";
		return false;
	}
	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] Reading from '" << rFileName << "'\n";

	// Fetch data
	while( fileStream.good() ) {
		char nextByte = fileStream.peek();
		// Comment line:
		if( nextByte == '#' ) {
			std::string line;
			getline( fileStream, line );
			continue;
		}
		// Regular line:
		unsigned long primID;
		fileStream >> primID;
		double normalX;
		double normalY;
		double normalZ;
		fileStream >> normalX;
		fileStream >> normalY;
		fileStream >> normalZ;
		rNormals->push_back( grVector3ID{ primID, normalX, normalY, normalZ } );
		//cout << "[MeshIO::" << __FUNCTION__ << "] " << primID << " " << normalX << " " << normalY << " " << normalZ << endl;
	}

	// Done
	fileStream.close();
	return( true );
}

// WRITE -----------------------------------------------------------------------

//! Set an export flag see MeshIO::eExportFlags
bool MeshIO::setFlagExport( eExportFlags rFlag, bool rSetTo ) {
	mExportFlags[rFlag] = rSetTo;
	return true;
}

//! Write a Mesh to a file using a filename given by the user.
//!
//! @returns false in case of an error. True otherwise.
bool MeshIO::writeFileUserInteract(const bool isLegacy) {
	// STUB
	LOG::error() << "[MeshIO::" << __FUNCTION__ << "] ERROR: Not impemented!\n";
	return( false );
}

//! Write a Mesh to a file. The file type is automatically determined by the file
//! extension. See methods like: MeshIO::writeOBJ, MeshIO::writePLY and MeshIO::writeVRML
bool MeshIO::writeFilePrimProps(
                const filesystem::path& rFileName,
                std::vector<sVertexProperties>& rVertexProps,
                std::vector<sFaceProperties>& rFaceProps
) {
	string fileExtension = rFileName.extension().string();

	if(fileExtension.empty())
	{
		LOG::error() << "[MeshIO::" << __FUNCTION__ << "] No extension/type for file '" << rFileName << "' specified!\n";
		return false;
	}

	for(char& character : fileExtension)
	{
		character = std::tolower(character);
	}

	if( fileExtension.at(0) == '.') {
		fileExtension.erase( fileExtension.begin() );
	}

	LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] extension: " << fileExtension << "\n";
	if( mExportFlags[EXPORT_BINARY] ) {
		LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] BINARY.\n";
	} else {
		LOG::debug() << "[MeshIO::" << __FUNCTION__ << "] ASCII.\n";
	}

	std::unique_ptr<MeshWriter> writer = nullptr;

	bool fileWriteOk = false;
	if( fileExtension == "obj" ) {
		writer = std::make_unique<ObjWriter>();
	}
	else if( fileExtension == "wrl" ) {
		writer = std::make_unique<VRMLWriter>();
	}
	else if( fileExtension == "txt" ) {
		writer = std::make_unique<TxtWriter>();
	}
	else if( fileExtension == "xyz" ) {
		writer = std::make_unique<TxtWriter>();
	}
	else if( fileExtension == "ply" ) {
		writer = std::make_unique<PlyWriter>();
	}
    else if( fileExtension == "gltf" ) {
        writer = std::make_unique<GltfWriter>();
    }


	if(writer != nullptr)
	{
		writer->setModelMetaData(mModelMetaData);

		if(mExportFlags[EXPORT_TEXTURE_FILE] && mModelMetaData.hasTextureFiles())
		{
			for(size_t i = 0; i<mModelMetaData.getTexturefilesRef().size(); ++i)
			{
				std::filesystem::path targetPath = rFileName.parent_path().wstring() + L"/" +
				        mModelMetaData.getTexturefilesRef()[i].filename().wstring();
				writer->getModelMetaDataRef().getTexturefilesRef()[i] = targetPath;
			}
		}

		writer->setIsBigEndian(mSystemIsBigEndian);
		writer->setExportBinary(mExportFlags[EXPORT_BINARY]);
		writer->setExportVertColor(mExportFlags[EXPORT_VERT_COLOR]);
		writer->setExportVertNormal(mExportFlags[EXPORT_VERT_NORMAL]);
		writer->setExportVertFlags(mExportFlags[EXPORT_VERT_FLAGS]);
		writer->setExportVertLabel(mExportFlags[EXPORT_VERT_LABEL]);
		writer->setExportVertFeatureVector(mExportFlags[EXPORT_VERT_FTVEC]);
		writer->setExportPolyline(mExportFlags[EXPORT_POLYLINE]);
		writer->setExportTextureCoordinates(mExportFlags[EXPORT_TEXTURE_COORDINATES]);

		fileWriteOk = writer->writeFile(rFileName, rVertexProps, rFaceProps, *this);
	}

	if( !fileWriteOk ) {
		LOG::error() << "[MeshIO::" << __FUNCTION__ << "] Unknown extension/type '" << fileExtension << "' specified!\n";
		return false;
	}

	mFileNameFull = rFileName;

	if(mExportFlags[EXPORT_TEXTURE_FILE] && mModelMetaData.hasTextureFiles())
	{
		auto texTargetFolder = std::filesystem::absolute(rFileName).parent_path();

		size_t texId = 0;
		for(const auto& textureFile : mModelMetaData.getTexturefilesRef())
		{
			try {
				std::filesystem::copy(textureFile, texTargetFolder);
			} catch (const std::filesystem::filesystem_error& e) {
				LOG::warn() << e.what() << " (target file may already exist)\n";
			}

			std::filesystem::path targetPath = rFileName.parent_path().wstring() + L"/" + textureFile.filename().wstring();
			mModelMetaData.getTexturefilesRef()[texId++] = targetPath;
		}
	}

	return true;
}

ModelMetaData &MeshIO::getModelMetaDataRef()
{
	return mModelMetaData;
}

// Provide Information ---------------------------------------------------------

//! Get the extension of the file name in lower case without dot.
//!
//! @returns extension of the file name in lower case without dot.
filesystem::path MeshIO::getFileExtension() const {
	// Extension - lower case and without dot
	std::wstring fileExtension = mFileNameFull.extension().wstring();
	for( auto& character : fileExtension) {
		character = static_cast<char>( std::tolower( static_cast<unsigned char>( character ) ) );
	}
	if( fileExtension.at(0) == '.' ) {
		fileExtension.erase( fileExtension.begin() );
	}
	return fileExtension;
}

//! Returns the base name = rFileName without extension nor path.
filesystem::path MeshIO::getBaseName() const {
	return mFileNameFull.stem();
}

//! Returns the path of the file.
filesystem::path MeshIO::getFileLocation() const {
	auto retPath = mFileNameFull;
	return retPath.remove_filename();
}

//! Returns the full name and path of the file.
filesystem::path MeshIO::getFullName() const {
	return mFileNameFull;
}

bool MeshIO::writeIcoNormalSphereData(const filesystem::path& rFilename, const std::list<sVertexProperties>& rVertexProps, int subdivisions, bool sphereCoordinates)
{
	fstream filestr;
	filestr.imbue(std::locale("C"));
	filestr.open( rFilename, fstream::out );
	if( !filestr.is_open() ) {
		LOG::error() << "[MeshIO] Could not open file: '" << rFilename << "'.\n";
		return false;
	}

	IcoSphereTree icoSphereTree(subdivisions);

	for(const auto& vertexProp : rVertexProps)
	{
		Vector3D normal(vertexProp.mNormalX, vertexProp.mNormalY, vertexProp.mNormalZ);
		auto incSize = normal.normalize3();

		size_t selIndex = icoSphereTree.getNearestVertexIndexAt(normal);
		icoSphereTree.incData(selIndex, incSize);
	}

	std::vector<float> normals = icoSphereTree.getVertices();
	const std::vector<double>& normalNums = *icoSphereTree.getVertexDataP();

	for(size_t i = 0; i<normalNums.size(); ++i)
	{
		float* n = &normals[i*3];
		if(sphereCoordinates)
		{
			float theta = acos(n[2]);
			float phi = atan2(n[1], n[0]);

			filestr << theta << "," << phi << "," << normalNums[i] << "\n";
		}
		else
		{
			filestr << n[0] << "," << n[1] << "," << n[2] << "," << normalNums[i] << "\n";
		}

	}

	filestr.close();

	return true;
}
