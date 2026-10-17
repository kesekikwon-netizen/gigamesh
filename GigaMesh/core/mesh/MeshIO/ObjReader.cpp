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

#include "ObjReader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <clocale>
#include <list>
#include <filesystem>
#include <map>

#include "MtlParser.h"
#include <GigaMesh/logging/Logging.h>
using namespace std;

struct ObjVertexPosition {
		double x = _NOT_A_NUMBER_DBL_;
		double y = _NOT_A_NUMBER_DBL_;
		double z = _NOT_A_NUMBER_DBL_;
		double funcVal = 0.0;
		unsigned char r = 187;
		unsigned char g = 187;
		unsigned char b = 187;
};

struct ObjNormal {
		double x = _NOT_A_NUMBER_DBL_;
		double y = _NOT_A_NUMBER_DBL_;
		double z = _NOT_A_NUMBER_DBL_;
};

struct ObjTexCoord {
	float s = _NOT_A_NUMBER_;
	float t = _NOT_A_NUMBER_;
};

struct ObjFaceVertex {
		ptrdiff_t v  = -1;
		ptrdiff_t vn = -1;
		ptrdiff_t vt = -1;
};

struct ObjFace {
		std::list<ObjFaceVertex> vertices;
		unsigned char textureId = 0;
};

//tokens 0 : "f" tag for face, 1 ... n-1: obj face triple v1/vt1/vn1 ...
//----------------------------
// Allowed variants in OBJ:
//----------------------------
// f v1 v2 v3 ...
// f v1/vt1 v2/vt2 v3/vt3 ...
// f v1//vn1 v2//vn2 v3//vn3 ...
// f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ....
inline ObjFace parseFaceProperty(const std::vector<std::string>& tokens, MtlMaterial* material, std::unordered_map<std::string, unsigned char>& textureToIdMap)
{
	ObjFace face;

	if(material != nullptr)
	{
		if(!material->map_Kd.empty())
		{
			face.textureId = textureToIdMap[material->map_Kd];
		}
	}

	for(size_t i = 1; i<tokens.size(); ++i)
	{
		std::stringstream tokenStream(tokens[i]);

		const auto tokenToValue = [&](ptrdiff_t& val) {
			tokenStream >> val >> std::ws;
			val -= 1;
		};

		while(!tokenStream.eof())
		{
			ObjFaceVertex faceVert;
			tokenToValue(faceVert.v);

			if(tokenStream.peek() == '/') //check for first '/' => there are textureCoordinates/normals
			{
				tokenStream.get();
				if(tokenStream.peek() == '/') //check if immediate '/' => no texturecoordinates
				{
					tokenStream.get();
					tokenToValue(faceVert.vn);
				}
				else //we have texturecoordinates
				{
					tokenToValue(faceVert.vt);
					if(tokenStream.peek() == '/') //and we have normals
					{
						tokenStream.get();
						tokenToValue(faceVert.vn);
					}
				}
			}


			face.vertices.emplace_back(faceVert);
		}

	}

	return face;
}

inline ObjVertexPosition parseVertexProperty(const std::vector<std::string>& tokens)
{
	ObjVertexPosition vertex;
	if(tokens.size() >= 4)
	{
		vertex.x = std::stod(tokens[1]);
		vertex.y = std::stod(tokens[2]);
		vertex.z = std::stod(tokens[3]);
	}
	if(tokens.size() > 6)
	{
		vertex.r = std::stoi(tokens[4]);
		vertex.g = std::stoi(tokens[5]);
		vertex.b = std::stoi(tokens[6]);
	}

	if( ( tokens.size() == 5 ) || ( tokens.size() == 8 ) ) { // There is function value at the end of the line
		vertex.funcVal = std::stod(tokens.back());
	}

	return vertex;
}

inline ObjNormal parseNormalProperty(const std::vector<std::string>& tokens)
{
	ObjNormal normal;
	if(tokens.size() >= 4)
	{
		normal.x = std::stod(tokens[1]);
		normal.y = std::stod(tokens[2]);
		normal.z = std::stod(tokens[3]);
	}

	return normal;
}

inline ObjTexCoord parseTextureCoordinate(const std::vector<std::string>& tokens)
{
	ObjTexCoord texCoord;
	if(tokens.size() >= 3)
	{
		texCoord.s = std::stof(tokens[1]);
		texCoord.t = std::stof(tokens[2]);
	}

	return texCoord;
}

inline sVertexProperties genVertexProperty(const std::pair<ptrdiff_t, ptrdiff_t>& posNormalID,
                                           const std::vector<ObjVertexPosition>& objVertices,
                                           const std::vector<ObjNormal>& objNormals)
{
	sVertexProperties vertProp;
	const auto& vertex = objVertices[posNormalID.first];
	vertProp.mCoordX = vertex.x;
	vertProp.mCoordY = vertex.y;
	vertProp.mCoordZ = vertex.z;

	vertProp.mColorRed = vertex.r;
	vertProp.mColorGrn = vertex.g;
	vertProp.mColorBle = vertex.b;
	vertProp.mColorAlp = 255;

	vertProp.mFuncVal = vertex.funcVal;

	if(posNormalID.second >= 0)
	{
		const auto& normal = objVertices[posNormalID.second];
		vertProp.mNormalX = normal.x;
		vertProp.mNormalY = normal.y;
		vertProp.mNormalZ = normal.z;
	}

	return vertProp;
}

inline void copyObjToMeshProperties(const std::vector<ObjVertexPosition>& objVertices,
                                    const std::vector<ObjNormal>& objNormals,
                                    const std::vector<ObjTexCoord>& objTexCoords,
                                    const std::vector<ObjFace>& objFaces,
                                    std::vector<sVertexProperties>& vertexProperties,
                                    std::vector<sFaceProperties>& faceProperties)
{
	size_t vertexIdx = 0;
	std::map<std::pair<ptrdiff_t,ptrdiff_t>, size_t> posNormalPairToIdMap;

	vertexProperties.reserve(objVertices.size() * 2);
	faceProperties.  reserve(   objFaces.size()    );

	for(const auto& objFace : objFaces)
	{
		sFaceProperties faceProp;
		faceProp.textureId = objFace.textureId;
		faceProp.vertexIndices.reserve(objFace.vertices.size());

		const bool hasTexture = objFace.vertices.begin()->vt >= 0;

		if(hasTexture)
		{
			faceProp.textureCoordinates.reserve(objFace.vertices.size() * 2);
		}

		for(const auto& faceVertex : objFace.vertices)
		{
			std::pair<ptrdiff_t, ptrdiff_t> posNormalId = std::make_pair(faceVertex.v, faceVertex.vn);

			auto it = posNormalPairToIdMap.find(posNormalId);

			if(it != posNormalPairToIdMap.end())
			{
				faceProp.vertexIndices.push_back(it->second);
			}
			else
			{
				posNormalPairToIdMap[posNormalId] = vertexIdx;
				faceProp.vertexIndices.push_back(vertexIdx);
				++vertexIdx;

				vertexProperties.emplace_back(genVertexProperty(posNormalId,objVertices,objNormals));
			}

			if(hasTexture)
			{
				faceProp.textureCoordinates.push_back(objTexCoords[faceVertex.vt].s);
				faceProp.textureCoordinates.push_back(objTexCoords[faceVertex.vt].t);
			}
		}

		faceProperties.emplace_back(faceProp);
	}

	vertexProperties.shrink_to_fit();
	faceProperties.shrink_to_fit();
}

//! Reads an plain-text Alias Wavefront OBJ file.
//!
//! After reading the file lots of other operations like meshing cleaning
//! are executed. Furthermore functions are tested here.
//!
//! see .OBJ specification: http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
bool ObjReader::readFile(const std::filesystem::path &rFilename, std::vector<sVertexProperties> &rVertexProps, std::vector<sFaceProperties> &rFaceProps, MeshSeedExt& rMeshSeed)
{
	char* oldLocale = std::setlocale( LC_NUMERIC, nullptr );
	std::setlocale( LC_NUMERIC, "C" );

	// commmon variables for internal use
	string line;
	string linePrefix;

	std::unordered_map<std::string, MtlMaterial> materials;

	// Counters for .OBJ elements and lines:
	size_t  obj_linesTotal            = 0;
	size_t  obj_linesIgnoredTotal     = 0;
	size_t  obj_linesUnsupportedTotal = 0;
	size_t  obj_verticesTotal         = 0;
	size_t  obj_VerticesNormalsTotal  = 0;
	size_t  obj_TexCoordsTotal        = 0;
	size_t  obj_FacesTotal            = 0;
	size_t  obj_commentsTotal         = 0;

	// functionality:
	ifstream fp( rFilename );
	if( !fp.is_open() ) {
		LOG::error() << "[ObjReader::" << __FUNCTION__ << "] Could not open file: '" << rFilename << "'.\n";
		std::setlocale( LC_NUMERIC, oldLocale );

		return false;
	}

	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] File opened: '" << rFilename << "'.\n";

	std::filesystem::path prevRootPath = std::filesystem::current_path();
	std::filesystem::current_path(std::filesystem::absolute(rFilename).parent_path());

	// determine the amount of data by parsing the data for a start:
	while( fp >> linePrefix ) {
		// Vertex Data -------------------------------------------------------------------------
		if( linePrefix == "vt" ) {           // texture vertices
			++obj_TexCoordsTotal;
		} else if( linePrefix == "vn" ) {     // vertex normals
			++obj_VerticesNormalsTotal;
		} else if( linePrefix == "vp" ) {     // paramter space vertics
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "v" ) {      // vertices
			obj_verticesTotal++;
		} else if( linePrefix == "cstype" ) { // free-form curve/surface attributes
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "deg" ) {    // degree
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "bmat" ) {   // basis matrix
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "step" ) {   // step size
			++obj_linesUnsupportedTotal;
		} else
		// Elements ----------------------------------------------------------------------------
		if( linePrefix == "p" ) {             // point
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "l"  ) {      // line
			// ++obj_linesUnsupportedTotal;
		} else if( linePrefix == "f" ) {       // Faces (triangles or ngons!)
			++obj_FacesTotal;
		} else if( linePrefix == "curv" ) {   // curve
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "curv2" ) {  // 2d-curve
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "surf" ) {   // surface
			++obj_linesUnsupportedTotal;
		} else
		// Free-form curve/surface body statements ---------------------------------------------
		if( linePrefix == "parm" ) {          // parameter values
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "trim" ) {   // outer trimming loop
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "hole" ) {   // inner trimming loop
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "scrv" ) {   // special curve
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "sp" ) {     // special point
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "end" ) {    // end statement
			++obj_linesUnsupportedTotal;
		} else
		// Connectivity between free-form surfaces ---------------------------------------------
		if( linePrefix == "con" ) {          // connect
			++obj_linesUnsupportedTotal;
		} else
		// Grouping ----------------------------------------------------------------------------
		if( linePrefix == "g" ) {            // group name
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "s" ) {     // smoothing group
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "mg" ) {    // merging group
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "o" ) {     // object name
			++obj_linesUnsupportedTotal;
		} else
		// Display/render attributes -----------------------------------------------------------
		if( linePrefix == "bevel" ) {             // bevel interpolation
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "c_interp" ) {   // color interpolation
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "d_interp" ) {   // dissolve interpolation
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "lod" ) {        // level of detail
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "usemtl" ) {     // material name
			//++obj_linesUnsupportedTotal;
		} else if( linePrefix == "mtlib" ) {      // material library
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "shadow_obj" ) { // shadow casting
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "trace_obj" ) {  // ray tracing
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "ctech" ) {      // curve approximation technique
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "stech" ) {      // surface approximation technique
			++obj_linesUnsupportedTotal;
		} else if( linePrefix == "mtllib") {      // mtl libs. Parse directy, to have all materials before using them guarenteed.
			{
				std::string fileName;
				std::getline(fp, fileName);

				//trim leading whitespaces
				if(auto firstCharPos = fileName.find_first_not_of(' '); firstCharPos != std::string::npos)
					fileName = fileName.substr(firstCharPos);

				//remove possible trailing \r
				if(auto trailingRPos = fileName.find_last_of('\r'); trailingRPos != std::string::npos)
				{
					fileName = fileName.substr(0, trailingRPos);

				}

				std::filesystem::path filePath(fileName);

				if(filePath.is_relative())
				{
					fileName = std::filesystem::absolute(filePath).string();
				}

				MtlParser parser;
				parser.parseFile(fileName);

				materials.insert(parser.getMaterialsRef().begin(), parser.getMaterialsRef().end());
			}
		} else
		// Comments ----------------------------------------------------------------------------
		if(  linePrefix == "#" ) {                 // comments
			obj_commentsTotal++;
		} else {
		// lines to ignore - in the ideal case there are none ... -----------------------------
			obj_linesIgnoredTotal++;
		}
		// fetch the rest of the line - even if it is just the end-of-line character:
		getline( fp, line );
		// may cause problems at the end of the file if not cleared:
		line.clear();
		linePrefix.clear();
	}

	// what we just found:
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] " << obj_linesTotal            << " lines read.\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] =========================================================\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] " << obj_verticesTotal         << " vertices found.\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] " << obj_VerticesNormalsTotal   << " vertex normals found.\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] " << obj_TexCoordsTotal         << " texture coordinates found.\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] " << obj_FacesTotal             << " faces found.\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] =========================================================\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] " << obj_commentsTotal         << " lines of comments.\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] " << obj_linesUnsupportedTotal << " lines not supported.\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] " << obj_linesIgnoredTotal     << " lines ignored.\n";
	LOG::info() << "[ObjReader::" << __FUNCTION__ << "] =========================================================\n";


	MeshReader::getModelMetaDataRef().setHasTextureCoordinates(obj_TexCoordsTotal > 0);

	std::unordered_map<std::string, TextureHandle> textureToIdMap;
	for(const auto & material : materials)
	{
		if(!(material.second.map_Kd.empty()) )
		{
			if(textureToIdMap.find(material.second.map_Kd) == textureToIdMap.end())
			{
				textureToIdMap[material.second.map_Kd] = MeshReader::getModelMetaDataRef().addTextureName(material.second.map_Kd);
			}
		}
	}

	MtlMaterial* currentMaterial = nullptr;

	std::vector<ObjFace> objFaces;
	objFaces.reserve(obj_FacesTotal);

	std::vector<ObjVertexPosition> objVertices;
	objVertices.reserve(obj_verticesTotal);

	std::vector<ObjNormal> objNormals;
	objNormals.reserve(obj_VerticesNormalsTotal);

	std::vector<ObjTexCoord> objTexCoords;
	objTexCoords.reserve(obj_TexCoordsTotal);


	// rewind file and fetch data:
	const auto timeStart = clock();

	fp.clear();
	fp.seekg( ios_base::beg );
	uint64_t lineNr = 0;
	for( std::string someLine; getline(fp,someLine); ) {
		++lineNr;
		if( someLine.length() == 0 ) {
			continue; // because of empty line OR EndOfFile.
		}
		istringstream iss( someLine );
		vector<string> tokens{ istream_iterator<string>{iss}, istream_iterator<string>{} };
		if( tokens.empty() ) {
			continue; // Essentially an empty lines with just a \cr.
		}
		string firstToken = tokens.at( 0 );
		if( firstToken.compare( 0, 1, "#" ) == 0 ) { // Comments
			// Meta-Data strings stored as comments -------------------------------------------------------
			if( tokens.size() < 2 ) {
				continue;
			}
			string possibleMetaDataName = tokens.at( 1 );
			ModelMetaData::eMetaStrings foundMetaId;
			if( MeshReader::getModelMetaDataRef().getModelMetaStringId( possibleMetaDataName, foundMetaId ) ) {
				uint64_t preMetaLen = 3 + possibleMetaDataName.size(); // 1 for comment sign '#' plus 2x space.
				string metaContent = someLine.substr( preMetaLen );
				LOG::debug() << "[ObjReader::" << __FUNCTION__ << "] Meta-Data: " << possibleMetaDataName << " (" << foundMetaId << ") = " << metaContent << "\n";
				if( !MeshReader::getModelMetaDataRef().setModelMetaString( foundMetaId, metaContent ) ) {
					LOG::warn() << "[ObjReader::" << __FUNCTION__ << "] Meta-Data not set!\n";
				}
			}
			continue; // because of line with comment.
			// --------------------------------------------------------------------------------------------
		}
		else if( firstToken == "v" ) { // Vertex
			const auto valueCount = tokens.size()-1;
			if( ( valueCount < 3 ) || ( valueCount == 5 ) || ( valueCount > 7 ) ) {
				LOG::warn() << "[ObjReader::" << __FUNCTION__ << "] Wrong token count: " << tokens.size()-1 << "!"  << "\n";
				LOG::warn() << "[ObjReader::" << __FUNCTION__ << "] " << someLine << "\n";
				continue;
			}
			objVertices.emplace_back(parseVertexProperty(tokens));
		}
		else if( firstToken == "f" ) { // faces i.e. triangles, quadtriangles and trianglestrips
			objFaces.emplace_back(parseFaceProperty(tokens,currentMaterial, textureToIdMap));
		}
		else if( firstToken == "vn" ) { // Vertex normals
			if( tokens.size() != 4 ) {
				LOG::warn() << "[ObjReader::" << __FUNCTION__ << "] Wrong token count: " << tokens.size() << "!"  << "\n";
				LOG::warn() << "[ObjReader::" << __FUNCTION__ << "] " << someLine << "\n";
				continue;
			}
			objNormals.emplace_back(parseNormalProperty(tokens));
		}
		else if( firstToken == "l" ) { // Polygonal lines
			auto somePolyLinesIndices = new vector<int>;
			for( size_t i=1; i<tokens.size(); i++ ) { //! \todo test the import of polylines from OBJs.
				int vertIndex = stoi( tokens.at( i ) )-1; // OBJs start with ONE, while GigaMesh's indices start wit ZERO
				somePolyLinesIndices->push_back( vertIndex );
			}
			rMeshSeed.getPolyLineVertIndicesRef().push_back( somePolyLinesIndices );			
		}
		else if ( firstToken == "vt" ) { //texture Coordinats
			if(tokens.size() != 3)
			{
				continue;
			}
			objTexCoords.emplace_back(parseTextureCoordinate(tokens));
		}
		else if ( firstToken == "usemtl") {
			if(tokens.size() != 2)
			{
				continue;
			}
			currentMaterial = &materials[tokens[1]];
		}
		else if (firstToken == "mtllib") {
			//ignore, mtllib is handled in first parsing run
		}
		else {
			LOG::warn() << "[ObjReader::" << __FUNCTION__ << "] line ignored: " << someLine << "\n";
		}
	}

	fp.close();

	copyObjToMeshProperties(objVertices,objNormals,objTexCoords, objFaces, rVertexProps, rFaceProps);

	const auto timeStop  = clock();
	LOG::debug() << "[ObjReader::" << __FUNCTION__ << "] fetch data from file:       " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << "\n";
	std::setlocale( LC_NUMERIC, oldLocale );
	std::filesystem::current_path(prevRootPath);
	return true;
}
