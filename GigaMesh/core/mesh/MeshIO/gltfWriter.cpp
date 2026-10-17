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
#include "gltfWriter.h"
#include <fstream>
#include <iostream>
#include <locale>
#include <filesystem>
#include <map>
#include <list>
#include<iostream>
#include<string>
#include <algorithm>

#include "gltfWriter.h"
#include <GigaMesh/logging/Logging.h>

using namespace std;

//source: https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c

std::string base64_encode(BYTE const* buf, unsigned int bufLen) {
  std::string ret;
  int i = 0;
  int j = 0;
  BYTE char_array_3[3];
  BYTE char_array_4[4];

  while (bufLen--) {
    char_array_3[i++] = *(buf++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';
  }

  return ret;
}

void GltfWriter::addFloatHexStringToBuffer(std::vector<BYTE> *buffer, float value)
{
    char const * valueStream = (char const *)&value;
    for (size_t i = 0; i != sizeof(float); ++i)
    {
        int byteValue = (int)valueStream[i];
        unsigned char byteAsChar = byteValue;
        buffer->push_back(byteAsChar);
    }
}

//! Calculates the offset within the buffer --> defines buffer windows
//! Each Buffer of each part may have a different size
//! returns the offset values for part 'rPart'
void GltfWriter::getBufferByteOffsets(int rPart, std::vector<int> indexBufferSizes, std::vector<int> vertexBufferSizes,std::vector<int> vertexColorBufferSizes, std::vector<int> normalsBufferSizes, std::vector<int> uvCoordsBufferSizes,
                                             int *offsetIndexBuffer, int *offsetVertexBuffer, int *offsetVertexColorBuffer, int *offsetNormalsBuffer, int *offsetTextureCoordsBuffer)
{
    *offsetIndexBuffer = 0;
    *offsetVertexBuffer = indexBufferSizes[0];
    *offsetVertexColorBuffer = *offsetVertexBuffer + vertexBufferSizes[0];
    *offsetNormalsBuffer = *offsetVertexColorBuffer + vertexColorBufferSizes[0];
    *offsetTextureCoordsBuffer = *offsetNormalsBuffer + normalsBufferSizes[0];

    //sum up the other parts, if required
    for (int i=1; i <= rPart; i++){
        *offsetIndexBuffer = *offsetTextureCoordsBuffer + uvCoordsBufferSizes[i-1];
        *offsetVertexBuffer = *offsetIndexBuffer + indexBufferSizes[i];
        *offsetVertexColorBuffer = *offsetVertexBuffer + vertexBufferSizes[i];
        *offsetNormalsBuffer = *offsetVertexColorBuffer + vertexColorBufferSizes[i];
        *offsetTextureCoordsBuffer = *offsetNormalsBuffer + normalsBufferSizes[i];
    }
}

void GltfWriter::createBuffersWithoutTextureCoords( std::vector<BYTE> *indexBuffer, std::vector<BYTE> *vertexBuffer, std::vector<BYTE> *vertexColorBuffer, std::vector<BYTE> *normalsBuffer,
                                                    float minPosition[3], float maxPosition[3], float minColor[3], float maxColor[3], float minNormals[3], float maxNormals[3],
                                                    const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps)
{

    //add all faces to file buffer
    for( const auto& faceProp : rFaceProps) {
        //add for each face the assigned vertices
        for( const auto& faceVertexIndex: faceProp.vertexIndices){

            // get the bytes
            int a = (faceVertexIndex >>  24) & 0xff;  // next byte, bits 24-32
            int b = (faceVertexIndex >>  16) & 0xff;  // next byte, bits 16-23
            int c = (faceVertexIndex >>  8) & 0xff;  // next byte, bits 8-15
            int d = faceVertexIndex         & 0xff;  // low-order byte: bits 0-7

            //change the parts to achieve little endian stringt
            unsigned char part1Byte = a;
            unsigned char part2Byte = b;
            unsigned char part3Byte = c;
            unsigned char part4Byte = d;
            indexBuffer->push_back(part4Byte);
            indexBuffer->push_back(part3Byte);
            indexBuffer->push_back(part2Byte);
            indexBuffer->push_back(part1Byte);
        }
    }

    //The Vertices and Normals are stored once
    //the mesh is defined by the faces through the indices
    //the index refers to a position/normal


    for( const auto& vertProp : rVertexProps) {

        //-----------------------------------
        //Position
        //-----------------------------------

        //save vertex coords as float
        float coordX = (float) vertProp.mCoordX;
        float coordY = (float) vertProp.mCoordY;
        float coordZ = (float) vertProp.mCoordZ;

        //update min max
        if (coordX < minPosition[0]){
            minPosition[0] = coordX;
        }
        if (coordY < minPosition[1]){
            minPosition[1] = coordY;
        }
        if (coordZ < minPosition[2]){
            minPosition[2] = coordZ;
        }
        if (coordX > maxPosition[0]){
            maxPosition[0] = coordX;
        }
        if (coordY > maxPosition[1]){
            maxPosition[1] = coordY;
        }
        if (coordZ > maxPosition[2]){
            maxPosition[2] = coordZ;
        }

        addFloatHexStringToBuffer(vertexBuffer,coordX);
        addFloatHexStringToBuffer(vertexBuffer,coordY);
        addFloatHexStringToBuffer(vertexBuffer,coordZ);

        //-----------------------------------
        //Vertex Color
        //-----------------------------------filestr << "}" << '\n';
        //save vertex color as float and ignore alpha value
        //convert the 8 bit color to values between 0 and 1
        float colorBle = (float) vertProp.mColorBle/255.0;
        float colorGrn = (float) vertProp.mColorGrn/255.0;
        float colorRed = (float) vertProp.mColorRed/255.0;

        //update min max
        if (colorBle < minColor[0]){
            minColor[0] = colorBle;
        }
        if (colorGrn < minColor[1]){
            minColor[1] = colorGrn;
        }
        if (colorRed < minColor[2]){
            minColor[2] = colorRed;
        }
        if (colorBle > maxColor[0]){
            maxColor[0] = colorBle;
        }
        if (colorGrn > maxColor[1]){
            maxColor[1] = colorGrn;
        }
        if (colorRed > maxColor[2]){
            maxColor[2] = colorRed;
        }


        addFloatHexStringToBuffer(vertexColorBuffer,colorRed);
        addFloatHexStringToBuffer(vertexColorBuffer,colorGrn);
        addFloatHexStringToBuffer(vertexColorBuffer,colorBle);

        //--------------------------
        //Normals
        //--------------------------
        if(mExportVertNormal){
            //save vertex coords as float
            float normalX = (float) vertProp.mNormalX;
            float normalY = (float) vertProp.mNormalY;
            float normalZ = (float) vertProp.mNormalZ;


            //update min max
            if (normalX < minNormals[0]){
                minNormals[0] = normalX;
            }
            if (normalY < minNormals[1]){
                minNormals[1] = normalY;
            }
            if (normalZ < minNormals[2]){
                minNormals[2] = normalZ;
            }
            if (normalX > maxNormals[0]){
                maxNormals[0] = normalX;
            }
            if (normalY > maxNormals[1]){
                maxNormals[1] = normalY;
            }
            if (normalZ > maxNormals[2]){
                maxNormals[2] = normalZ;
            }

            addFloatHexStringToBuffer(normalsBuffer,normalX);
            addFloatHexStringToBuffer(normalsBuffer,normalY);
            addFloatHexStringToBuffer(normalsBuffer,normalZ);

        }
    }
}

void GltfWriter::createBuffersIncludingTextureCoords(const unsigned int textureId, std::vector<BYTE> *indexBuffer, std::vector<BYTE> *vertexBuffer, std::vector<BYTE> *vertexColorBuffer, std::vector<BYTE> *normalsBuffer, std::vector<BYTE> *uvCoordsBuffer,
                                                     float minPosition[3], float maxPosition[3], float minColor[3], float maxColor[3],float minNormals[3], float maxNormals[3], float minTextureCoords[2], float maxTextureCoords[2],
                                                      const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps)
{
    //initialize min and max
    minTextureCoords[0] = (float) rFaceProps[0].textureCoordinates[0];
    minTextureCoords[1] = (float) rFaceProps[0].textureCoordinates[1];
    maxTextureCoords[0] = (float) rFaceProps[0].textureCoordinates[0];
    maxTextureCoords[1] = (float) rFaceProps[0].textureCoordinates[1];
    unsigned int index = 0;
    //add all faces to file buffer
    for( const auto& faceProp : rFaceProps) {
        //skip the faces that belong to another texture
        if( faceProp.textureId != textureId){
            continue;
        }

        //add for each face the assigned vertices

        //the texture coordinates are saved in a face property
        //therefore, the textureIndex serves to access them
        unsigned int textureIndex = 0;
        for( const auto& faceVertexIndex: faceProp.vertexIndices){

            // get the bytes
            int a = (index >>  24) & 0xff;  // next byte, bits 24-32
            int b = (index >>  16) & 0xff;  // next byte, bits 16-23
            int c = (index >>  8) & 0xff;  // next byte, bits 8-15
            int d = index         & 0xff;  // low-order byte: bits 0-7

            //change the parts to achieve little endian stringt
            unsigned char part1Byte = a;
            unsigned char part2Byte = b;
            unsigned char part3Byte = c;
            unsigned char part4Byte = d;
            indexBuffer->push_back(part4Byte);
            indexBuffer->push_back(part3Byte);
            indexBuffer->push_back(part2Byte);
            indexBuffer->push_back(part1Byte);

            //-----------------------------------
            //Position
            //-----------------------------------

            //save vertex coords as float
            float coordX = (float) rVertexProps[faceVertexIndex].mCoordX;
            float coordY = (float) rVertexProps[faceVertexIndex].mCoordY;
            float coordZ = (float) rVertexProps[faceVertexIndex].mCoordZ;

            //update min max
            if (coordX < minPosition[0]){
                minPosition[0] = coordX;
            }
            if (coordY < minPosition[1]){
                minPosition[1] = coordY;
            }
            if (coordZ < minPosition[2]){
                minPosition[2] = coordZ;
            }
            if (coordX > maxPosition[0]){
                maxPosition[0] = coordX;
            }
            if (coordY > maxPosition[1]){
                maxPosition[1] = coordY;
            }
            if (coordZ > maxPosition[2]){
                maxPosition[2] = coordZ;
            }

            addFloatHexStringToBuffer(vertexBuffer,coordX);
            addFloatHexStringToBuffer(vertexBuffer,coordY);
            addFloatHexStringToBuffer(vertexBuffer,coordZ);

            //-----------------------------------
            //Vertex Color
            //-----------------------------------
            //save vertex color as float and ignore alpha value
            //convert the 8 bit color to values between 0 and 1
            float colorBle = (float) rVertexProps[faceVertexIndex].mColorBle/255.0;
            float colorGrn = (float) rVertexProps[faceVertexIndex].mColorGrn/255.0;
            float colorRed = (float) rVertexProps[faceVertexIndex].mColorRed/255.0;

            //update min max
            if (colorBle < minColor[0]){
                minColor[0] = colorBle;
            }
            if (colorGrn < minColor[1]){
                minColor[1] = colorGrn;
            }
            if (colorRed < minColor[2]){
                minColor[2] = colorRed;
            }
            if (colorBle > maxColor[0]){
                maxColor[0] = colorBle;
            }
            if (colorGrn > maxColor[1]){
                maxColor[1] = colorGrn;
            }
            if (colorRed > maxColor[2]){
                maxColor[2] = colorRed;
            }


            addFloatHexStringToBuffer(vertexColorBuffer,colorRed);
            addFloatHexStringToBuffer(vertexColorBuffer,colorGrn);
            addFloatHexStringToBuffer(vertexColorBuffer,colorBle);

            //--------------------------
            //Normals
            //--------------------------
            if(mExportVertNormal){
                //save vertex coords as float
                float normalX = (float) rVertexProps[faceVertexIndex].mNormalX;
                float normalY = (float) rVertexProps[faceVertexIndex].mNormalY;
                float normalZ = (float) rVertexProps[faceVertexIndex].mNormalZ;


                //update min max
                if (normalX < minNormals[0]){
                    minNormals[0] = normalX;
                }
                if (normalY < minNormals[1]){
                    minNormals[1] = normalY;
                }
                if (normalZ < minNormals[2]){
                    minNormals[2] = normalZ;
                }
                if (normalX > maxNormals[0]){
                    maxNormals[0] = normalX;
                }
                if (normalY > maxNormals[1]){
                    maxNormals[1] = normalY;
                }
                if (normalZ > maxNormals[2]){
                    maxNormals[2] = normalZ;
                }

                addFloatHexStringToBuffer(normalsBuffer,normalX);
                addFloatHexStringToBuffer(normalsBuffer,normalY);
                addFloatHexStringToBuffer(normalsBuffer,normalZ);

            }
            //--------------------------
            //Texture Coords (UV)
            //--------------------------

            //save texture coords as float
            //mirrow the values
            float coordU = (float) faceProp.textureCoordinates[2*textureIndex];
            float coordV = 1.0- (float) faceProp.textureCoordinates[2*textureIndex + 1];

            //update min max
            if (coordU < minTextureCoords[0]){
                minTextureCoords[0] = coordU;
            }
            if (coordV < minTextureCoords[1]){
                minTextureCoords[1] = coordV;
            }
            if (coordU > maxTextureCoords[0]){
                maxTextureCoords[0] = coordU;
            }
            if (coordV > maxTextureCoords[1]){
                maxTextureCoords[1] = coordV;
            }


            addFloatHexStringToBuffer(uvCoordsBuffer,coordU);
            addFloatHexStringToBuffer(uvCoordsBuffer,coordV);

            textureIndex++;
            index++;

        }
    }
}

//!In this method the entire string of the gltf file is built.
//! TODO Parts of the filesuvCoordsBuffertream routines can be written in separate methods for better readability.
bool GltfWriter::writeFile(const std::filesystem::path& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed)
{
    fstream filestr;
    filestr.imbue(std::locale("C"));
    int timeStart; // for performance mesurement

    timeStart = clock();
    filestr.open( rFilename, fstream::out );
    if( !filestr.is_open() ) {
        LOG::error() << "[GltfWriter::" << __FUNCTION__ << "] ERROR: Could not open file: '" << rFilename << "'!" << '\n';
        return false;
    } else {
        LOG::debug() << "[GltfWriter::" << __FUNCTION__ << "] File open for writing: '" << rFilename << "'." << '\n';
    }

    //following https://github.khronos.org/glTF-Tutorials/gltfTutorial/gltfTutorial_003_MinimalGltfFile.html
    //basic structure as JSON
    //Mesh and scene description
    filestr << "{" << '\n';
    filestr << "\"scene\": 0," << '\n';
    filestr << "\"scenes\" : [" << '\n';
    filestr << "{" << '\n';
    //DYNAMIC PART
    //If there are more than one texture the mesh has to be split into several parts
    // Due to the restriction that a gltf mesh can not deal with different textures for different parts of the mesh
    // As far as I know 2024/01/16 Ernst S.
    std::vector<std::filesystem::path>& textureFiles = MeshWriter::getModelMetaDataRef().getTexturefilesRef();
    unsigned int nrPartsOfMesh = textureFiles.size();
    //if there is not texture -> mesh consists of one part
    if (nrPartsOfMesh == 0)nrPartsOfMesh = 1;
    filestr << "\"nodes\" : [";
    for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
        filestr << indexMeshPart;
        if (indexMeshPart+1 != nrPartsOfMesh){
            filestr << ",";
        }
    }
    filestr << "]" << '\n';
    filestr << "}" << '\n';
    filestr << "]," << '\n';



    // add meshes as node: This signalize the gltf that there are n meshes
    // in gltf the meshpart from gigamesh's point of view is an independent mesh
    filestr << "\"nodes\" : [" << '\n';
    for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
        filestr << "{" << '\n';
        filestr << "\"mesh\" : " << std::to_string(indexMeshPart) << '\n';
        //last closing brackets aren't allowed to have a comma
        if (indexMeshPart+1 == nrPartsOfMesh){
            filestr << "}" << '\n';
        }
        else{
            filestr << "}," << '\n';
        }
    }
    filestr << "]," << '\n';

    //define how many different buffer types are used
    //at least 3 (index, vertex position and color)
    int nrOfBufferTypes = 3;
    if(mExportVertNormal)nrOfBufferTypes++;
    if(mExportTextureCoordinates)nrOfBufferTypes++;

    //content definition for each mesh
    // each primitive points to another part of the buffer
    // the buffer parts are numbered and defined later
    filestr << "\"meshes\" : [" << '\n';
    for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
        filestr << "{" << '\n';
        filestr << "\"primitives\" : [ {" << '\n';
        filestr << "\"attributes\" : {" << '\n';

        if(mExportVertNormal){
            filestr << "\"POSITION\" : "<< std::to_string(indexMeshPart*nrOfBufferTypes+1) << "," << '\n';
            filestr << "\"COLOR_0\" : " << std::to_string(indexMeshPart*nrOfBufferTypes+2) << "," << '\n';
            if (mExportTextureCoordinates){
                    filestr << "\"NORMAL\" : " << std::to_string(indexMeshPart*nrOfBufferTypes+3) << "," << '\n';
                    filestr << "\"TEXCOORD_0\" : " << std::to_string(indexMeshPart*nrOfBufferTypes+4) << '\n';
            }
            else
            {
               filestr << "\"NORMAL\" : " << std::to_string(indexMeshPart*nrOfBufferTypes+3) << '\n';
            }
        }
        else{
            if (mExportTextureCoordinates){
                    filestr << "\"POSITION\" : "<< std::to_string(indexMeshPart*nrOfBufferTypes+1) << "," << '\n';
                    filestr << "\"COLOR_0\" : " << std::to_string(indexMeshPart*nrOfBufferTypes+2) << "," << '\n';
                    filestr << "\"TEXCOORD_0\" : " << std::to_string(indexMeshPart*nrOfBufferTypes+3) << '\n';
            }
            else
            {
                filestr << "\"POSITION\" : "<< std::to_string(indexMeshPart*nrOfBufferTypes+1) << "," << '\n';
                filestr << "\"COLOR_0\" : " << std::to_string(indexMeshPart*nrOfBufferTypes+2) << '\n';
            }
        }

        filestr << "}," << '\n';
        if (!mExportTextureCoordinates){
            filestr << "\"indices\" : " << std::to_string(indexMeshPart*nrOfBufferTypes) << '\n';
        }
        else{
             filestr << "\"indices\" : " << std::to_string(indexMeshPart*nrOfBufferTypes) << "," << '\n';
             filestr << "\"material\" : "  << std::to_string(indexMeshPart) << '\n';
        }
        filestr << "} ]" << '\n';
        //last closing brackets aren't allowed to have a comma
        if (indexMeshPart+1 == nrPartsOfMesh){
            filestr << "}" << '\n';
        }
        else{
            filestr << "}," << '\n';
        }
    }
    filestr << "]," << '\n';


    //add texture configuration
    //uv coordinates are in the buffer
    if (mExportTextureCoordinates){
        // each texture is defined by a own material
        // gigamesh ignores pcr parameter --> constant
        filestr << "\"materials\" : [ " << '\n';
        for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
            filestr << "{" << '\n';
            filestr << "\"pbrMetallicRoughness\" : {" << '\n';
            filestr << "\"baseColorTexture\" : {" << '\n';
            filestr << "\"index\" : " << std::to_string(indexMeshPart) << '\n';
            filestr << " }," << '\n';
            filestr << "\"metallicFactor\" : 0.0," << '\n';
            filestr << "\"roughnessFactor\" : 1.0" << '\n';
            filestr << "}" << '\n';

            //last closing brackets aren't allowed to have a comma
            if (indexMeshPart+1 == nrPartsOfMesh){
                filestr << "}" << '\n';
            }
            else{
                filestr << "}," << '\n';
            }
        }
        filestr << "]," << '\n';


        filestr << "\"textures\" : [ " << '\n';
        for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
            filestr << "{" << '\n';
            filestr << "\"sampler\" : 0," << '\n'; // is defined once (later)
            filestr << "\"source\" : " << std::to_string(indexMeshPart) << '\n';
            //last closing brackets aren't allowed to have a comma
            if (indexMeshPart+1 == nrPartsOfMesh){
                filestr << "}" << '\n';
            }
            else{
                filestr << "}," << '\n';
            }
        }
        filestr << "]," << '\n';

        //set file system to the mesh file, otherwise the relative dir based on the GigaMesh build dir
        auto prevPath = std::filesystem::current_path();
        std::filesystem::current_path(std::filesystem::absolute(rFilename).parent_path());
        filestr << "\"images\" : [ " << '\n';
        for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
            filestr << "{" << '\n';
            filestr << "\"uri\" : \"" << std::filesystem::relative(textureFiles[indexMeshPart]).string() << "\"" << '\n';
            //last closing brackets aren't allowed to have a comma
            if (indexMeshPart+1 == nrPartsOfMesh){
                filestr << "}" << '\n';
            }
            else{
                filestr << "}," << '\n';
            }
        }
        filestr << "]," << '\n';
        //reset filesystem
        std::filesystem::current_path(prevPath);

        //use the same sampler for each texture
        filestr << "\"samplers\" : [ {" << '\n';
        filestr << "\"magFilter\" : 9729," << '\n';
        filestr << "\"minFilter\" : 9987," << '\n';
        filestr << "\"wrapS\" : 33648," << '\n';
        filestr << "\"wrapT\" : 33648" << '\n';
        filestr << "} ]," << '\n';
    }
    //----------------------------------------------------------------------------------------
    //Buffer creation
    //----------------------------------------------------------------------------------------
    //Buffer contains the actual data
    //save all buffersizes in bytes to describe them later
    std::vector<BYTE> entireBuffer;
    std::vector<int> indexBufferSizes;
    std::vector<int> vertexBufferSizes;
    std::vector<int> vertexColorBufferSizes;
    std::vector<int> normalsBufferSizes;
    std::vector<int> uvCoordsBufferSizes;

    //save all min max values per buffer
    //the vector is filled with arrays, since the array was used before.
    //!TODO:the code must be refactored to make it consistent
    std::vector<vector<float>>bufferMinPositions;
    std::vector<vector<float>> bufferMaxPositions;
    std::vector<vector<float>>bufferMinNormals;
    std::vector<vector<float>>bufferMaxNormals;
    std::vector<vector<float>>bufferMinColors;
    std::vector<vector<float>>bufferMaxColors;
    std::vector<vector<float>>bufferMinUVPositions;
    std::vector<vector<float>>bufferMaxUVPositions;

    //number of premitives (vertex values and text coords per part
    std::vector<int> partNrOfPremitives;
    //number of indices per part (usually 3 times faces)
    std::vector<int> partNrOfIndices;

    //max values of used index in part
    //index = vertex index to describe the faces
    std::vector<int> partMaxIndices;


    //calculate the buffer for each part of the mesh
    for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
        std::vector<BYTE> indexBuffer;
        std::vector<BYTE> vertexBuffer;
        std::vector<BYTE> vertexColorBuffer;
        std::vector<BYTE> normalsBuffer;
        std::vector<BYTE> uvCoordsBuffer;
        //save min max of the vectors for the accessors
        //structure: x, y ,z
        float minPosition[3] = {(float) rVertexProps[0].mCoordX, (float) rVertexProps[0].mCoordY,(float) rVertexProps[0].mCoordZ};
        float maxPosition[3] = {(float) rVertexProps[0].mCoordX, (float) rVertexProps[0].mCoordY,(float) rVertexProps[0].mCoordZ};
        float minNormals[3] = {(float) rVertexProps[0].mNormalX, (float) rVertexProps[0].mNormalY,(float) rVertexProps[0].mNormalZ};
        float maxNormals[3] = {(float) rVertexProps[0].mNormalX, (float) rVertexProps[0].mNormalY,(float) rVertexProps[0].mNormalZ};
        //Color
        float minColors[3] = {(float) rVertexProps[0].mColorBle, (float) rVertexProps[0].mColorGrn,(float) rVertexProps[0].mColorRed};
        float maxColors[3] = {(float) rVertexProps[0].mColorBle, (float) rVertexProps[0].mColorGrn,(float) rVertexProps[0].mColorRed};
        //convert to values between 0 and 1 (8-Bit color)
        minColors[0] = minColors[0]/255.0;
        minColors[1] = minColors[1]/255.0;
        minColors[2] = minColors[2]/255.0;
        maxColors[0] = maxColors[0]/255.0;
        maxColors[1] = maxColors[1]/255.0;
        maxColors[2] = maxColors[2]/255.0;

        //will be initialized within the method
        //structure: u, v
        float minTextureCoords[2];
        float maxTextureCoords[2];

        //is required inside the buffer accessor definition
        //primitives = vertex, face and texture coordinates
        unsigned int nrOfPrimitives = 0;
        //similar behavior as above: the maximum value of the indices depends on the export method
        //if each vertex of each face is written separatly, then the max values is 3 times the number of faces because the index of the primitives are just incremented
        //otherwise the indices include the actual indices of the vertices
        unsigned int maxIndexValue = 0;
        unsigned int nrOfIndices = 0;
        if(!mExportTextureCoordinates){
            createBuffersWithoutTextureCoords(&indexBuffer,&vertexBuffer,&vertexColorBuffer, &normalsBuffer, minPosition, maxPosition, minColors, maxColors, minNormals, maxNormals, rVertexProps,rFaceProps);
            //the face are described by indices --> each vertex is written once
            // only meshes with textures could be consists of different parts
            nrOfPrimitives = rVertexProps.size();
            maxIndexValue = rVertexProps.size()-1;
            nrOfIndices= 3*rFaceProps.size();
        }
        else{
            //if the texture coordinates have to be saved, then the faces must be stored by vertices
            //--> For each face, 3 positions, normals and texture coordinates are saved
            //This is due to the GLTF requirement that primitive buffers (positions, normals and textcoords) must consist of the number of entries.
            //the meshpart refers to the texture id --> each part has its own texture
            createBuffersIncludingTextureCoords(indexMeshPart, &indexBuffer,&vertexBuffer,&vertexColorBuffer, &normalsBuffer,&uvCoordsBuffer, minPosition, maxPosition, minColors, maxColors, minNormals, maxNormals, minTextureCoords, maxTextureCoords, rVertexProps,rFaceProps);
            //indexBuffer/4 is the number of saved vertices for this part
            nrOfPrimitives = indexBuffer.size()/4;
            maxIndexValue = indexBuffer.size()/4 - 1;
            nrOfIndices= indexBuffer.size()/4;
        }
        partNrOfPremitives.push_back(nrOfPrimitives);
        partMaxIndices.push_back(maxIndexValue);
        partNrOfIndices.push_back(nrOfIndices);


        int nrOfBytesIndexBuffer = indexBuffer.size();
        //add zero bytes if it isn't a multiple of 4
        int bytesToBeFilled = nrOfBytesIndexBuffer % 4;
        if (bytesToBeFilled != 0){
            unsigned char zeroBytes = int(0);
            for(int i = 0; i < bytesToBeFilled; i++){
                indexBuffer.push_back(zeroBytes);
            }
        }
        int nrOfBytesIndexBufferPlusOffset = indexBuffer.size();

        //Every buffer are concatenated to the final buffer
        //and base64 encoded
        entireBuffer.insert(entireBuffer.end(), indexBuffer.begin(), indexBuffer.end());
        entireBuffer.insert(entireBuffer.end(), vertexBuffer.begin(), vertexBuffer.end());
        entireBuffer.insert(entireBuffer.end(), vertexColorBuffer.begin(), vertexColorBuffer.end());
        entireBuffer.insert(entireBuffer.end(), normalsBuffer.begin(), normalsBuffer.end());
        if (mExportTextureCoordinates){
            entireBuffer.insert(entireBuffer.end(), uvCoordsBuffer.begin(), uvCoordsBuffer.end());
        }

        int nrOfBytesVertexBuffer = vertexBuffer.size();
        int nrOfBytesVertexColorBuffer = vertexColorBuffer.size();
        int nrOfBytesNormalsBuffer = normalsBuffer.size();
        int nrOfBytesUVBuffer = uvCoordsBuffer.size();


        //save values to history
        indexBufferSizes.push_back(nrOfBytesIndexBufferPlusOffset);
        vertexBufferSizes.push_back(nrOfBytesVertexBuffer);
        vertexColorBufferSizes.push_back(nrOfBytesVertexColorBuffer);
        normalsBufferSizes.push_back(nrOfBytesNormalsBuffer);
        uvCoordsBufferSizes.push_back(nrOfBytesUVBuffer);

        //convert all arrays to vector --> REFACTORING: Replace arrays with vector
        bufferMinPositions.push_back(std::vector<float> (std::begin(minPosition),std::end(minPosition)));
        bufferMaxPositions.push_back(std::vector<float> (std::begin(maxPosition),std::end(maxPosition)));
        bufferMinNormals.push_back(std::vector<float> (std::begin(minNormals),std::end(minNormals)));
        bufferMaxNormals.push_back(std::vector<float> (std::begin(maxNormals),std::end(maxNormals)));
        bufferMinColors.push_back(std::vector<float> (std::begin(minColors),std::end(minColors)));
        bufferMaxColors.push_back(std::vector<float> (std::begin(maxColors),std::end(maxColors)));
        bufferMinUVPositions.push_back(std::vector<float> (std::begin(minTextureCoords),std::end(maxTextureCoords)));
        bufferMaxUVPositions.push_back(std::vector<float> (std::begin(maxTextureCoords),std::end(maxTextureCoords)));


    }
    //total buffer size
    std::string entireBufferEncoded = base64_encode(&entireBuffer[0], entireBuffer.size());
    int nrOfBytes = entireBuffer.size();


    //-------------------------------------------------------------------------------------------------------------
    // Buffer Describtion
    // Includes where the primitives are written
    //-------------------------------------------------------------------------------------------------------------
    filestr << "\"buffers\" : [" << '\n';
    filestr << "{" << '\n';
    filestr << "\"uri\" : \"data:application/octet-stream;base64,"<< entireBufferEncoded << "\"," << '\n';
    filestr << "\"byteLength\" : " << std::to_string(nrOfBytes) << '\n';
    filestr << "}" << '\n';
    filestr << "]," << '\n';


    // bufferviews together with accessors defines different windows inside the buffer
    // for each part of the mesh (different texture) the buffer consists of:
    //first the indices as unsigned int
    //second vertex data as flot vec3 and vec2 for the texturecoords
    filestr << "\"bufferViews\" : [" << '\n';
    for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
        //get the buffer of set of this part
        int offsetIndexBuffer;
        int offsetVertexBuffer;
        int offsetVertexColorBuffer;
        int offsetNormalsBuffer;
        int offsetTextureCoordsBuffer;
        getBufferByteOffsets(indexMeshPart,indexBufferSizes,vertexBufferSizes,vertexColorBufferSizes,normalsBufferSizes,uvCoordsBufferSizes,&offsetIndexBuffer,&offsetVertexBuffer,&offsetVertexColorBuffer,&offsetNormalsBuffer,&offsetTextureCoordsBuffer);

        filestr << "{" << '\n';
        filestr << "\"buffer\" : 0," << '\n';
        filestr << "\"byteOffset\" : " << std::to_string(offsetIndexBuffer) << "," << '\n';
        filestr << "\"byteLength\" : " << std::to_string(indexBufferSizes[indexMeshPart]) << "," << '\n';
        filestr << "\"target\" : 34963" << '\n';
        filestr << "}," << '\n';

        //vertex buffer
        filestr << "{" << '\n';
        filestr << "\"buffer\" : 0," << '\n';
        filestr << "\"byteOffset\" : " << std::to_string(offsetVertexBuffer) << "," << '\n';
        filestr << "\"byteLength\" : " << std::to_string(vertexBufferSizes[indexMeshPart]) << "," << '\n';
        filestr << "\"target\" : 34962" << '\n';
        filestr << "}," << '\n';

        //vertex color bufferr << "}" << '\n';
        filestr << "{" << '\n';
        filestr << "\"buffer\" : 0," << '\n';
        filestr << "\"byteOffset\" : " << std::to_string(offsetVertexColorBuffer) << "," << '\n';
        filestr << "\"byteLength\" : " << std::to_string(vertexColorBufferSizes[indexMeshPart]) << "," << '\n';
        filestr << "\"target\" : 34962" << '\n';

        if (!mExportVertNormal && !mExportTextureCoordinates){
            //no further value buffers --> last entry
            //last closing brackets aren't allowed to have a comma
            if (indexMeshPart+1 == nrPartsOfMesh){
                filestr << "}" << '\n';
            }
            else{
                filestr << "}," << '\n';
            }
        }
        else{
            filestr << "}," << '\n';

            if(mExportVertNormal){
                //normals buffer
                filestr << "{" << '\n';
                filestr << "\"buffer\" : 0," << '\n';
                filestr << "\"byteOffset\" : " << std::to_string(offsetNormalsBuffer) << "," << '\n';
                filestr << "\"byteLength\" : " << std::to_string(normalsBufferSizes[indexMeshPart]) << "," << '\n';
                filestr << "\"target\" : 34962" << '\n';
                if (!mExportTextureCoordinates){
                    //last closing brackets aren't allowed to have a comma
                    if (indexMeshPart+1 == nrPartsOfMesh){
                        filestr << "}" << '\n';
                    }
                    else{
                        filestr << "}," << '\n';
                    }
                }
                else {
                    filestr << "}," << '\n';
                }
            }
            if (mExportTextureCoordinates){
                //uv buffer
                filestr << "{" << '\n';
                filestr << "\"buffer\" : 0," << '\n';
                filestr << "\"byteOffset\" : " << std::to_string(offsetTextureCoordsBuffer) << "," << '\n';
                filestr << "\"byteLength\" : " << std::to_string(uvCoordsBufferSizes[indexMeshPart]) << "," << '\n';
                filestr << "\"target\" : 34962" << '\n';
                //last closing brackets aren't allowed to have a comma
                if (indexMeshPart+1 == nrPartsOfMesh){
                    filestr << "}" << '\n';
                }
                else{
                    filestr << "}," << '\n';
                }
            }
        }
    }
    filestr << "]," << '\n';

    //accessors
    //index accessor
    filestr << "\"accessors\" : [" << '\n';
    for(unsigned int indexMeshPart = 0; indexMeshPart < nrPartsOfMesh; indexMeshPart++){
        filestr << "{" << '\n';
        filestr << "\"bufferView\" : " << std::to_string(indexMeshPart*nrOfBufferTypes) << "," << '\n';
        filestr << "\"byteOffset\" : 0," << '\n';
        filestr << "\"componentType\" : 5125," << '\n';
        filestr << "\"count\" : " << std::to_string(partNrOfIndices[indexMeshPart]) << "," << '\n';
        filestr << "\"type\" : \"SCALAR\"," << '\n';
        filestr << "\"max\" : [" << std::to_string(partMaxIndices[indexMeshPart]) << "]," << '\n';
        filestr << "\"min\" : [ 0 ]" << '\n';
        filestr << "}," << '\n';

        //-----------------------------------------------------
        //vertex accessor
        filestr << "{" << '\n';
        filestr << "\"bufferView\" : " << std::to_string(indexMeshPart*nrOfBufferTypes +1) << "," <<  '\n';
        filestr << "\"byteOffset\" : 0," << '\n';
        filestr << "\"componentType\" : 5126," << '\n';
        //filestr << "\"count\" : " << std::to_string(index) << "," << '\n';
        filestr << "\"count\" : " << std::to_string(partNrOfPremitives[indexMeshPart]) << "," << '\n';
        filestr << "\"type\" : \"VEC3\"," << '\n';
        //keep the dot save in the export. Location configuration of the system could lead to comma export of floats
        std::string maxXString = std::to_string(bufferMaxPositions[indexMeshPart][0]);
        std::replace( maxXString.begin(), maxXString.end(), ',', '.');
        std::string maxYString = std::to_string(bufferMaxPositions[indexMeshPart][1]);
        std::replace( maxYString.begin(), maxYString.end(), ',', '.');
        std::string maxZString = std::to_string(bufferMaxPositions[indexMeshPart][2]);
        std::replace( maxZString.begin(), maxZString.end(), ',', '.');
        std::string minXString = std::to_string(bufferMinPositions[indexMeshPart][0]);
        std::replace( minXString.begin(), minXString.end(), ',', '.');
        std::string minYString = std::to_string(bufferMinPositions[indexMeshPart][1]);
        std::replace( minYString.begin(), minYString.end(), ',', '.');
        std::string minZString = std::to_string(bufferMinPositions[indexMeshPart][2]);
        std::replace( minZString.begin(), minZString.end(), ',', '.');

        filestr << "\"max\" : [ " << maxXString << "," << maxYString << "," << maxZString << "]," << '\n';
        filestr << "\"min\" : [ " << minXString << "," << minYString << "," << minZString << "]" << '\n';
        filestr << "}," << '\n';

        //vertex color accessor
        filestr << "{" << '\n';
        filestr << "\"bufferView\" : " << std::to_string(indexMeshPart*nrOfBufferTypes + 2) << "," <<  '\n';
        filestr << "\"byteOffset\" : 0," << '\n';
        filestr << "\"componentType\" : 5126," << '\n';
        filestr << "\"count\" : " << std::to_string(partNrOfPremitives[indexMeshPart]) << "," << '\n';
        filestr << "\"type\" : \"VEC3\"," << '\n';
        //keep the dot save in the export. Location configuration of the system could lead to comma export of floats
        std::string maxBlueString = std::to_string(bufferMaxColors[indexMeshPart][0]);
        std::replace( maxBlueString.begin(), maxBlueString.end(), ',', '.');
        std::string maxGreenString = std::to_string(bufferMaxColors[indexMeshPart][1]);
        std::replace( maxGreenString.begin(), maxGreenString.end(), ',', '.');
        std::string maxRedString = std::to_string(bufferMaxColors[indexMeshPart][2]);
        std::replace( maxRedString.begin(), maxRedString.end(), ',', '.');
        std::string minBlueString = std::to_string(bufferMinColors[indexMeshPart][0]);
        std::replace( minBlueString.begin(), minBlueString.end(), ',', '.');
        std::string minGreenString = std::to_string(bufferMinColors[indexMeshPart][1]);
        std::replace( minGreenString.begin(), minGreenString.end(), ',', '.');
        std::string minRedString = std::to_string(bufferMinColors[indexMeshPart][2]);
        std::replace( minRedString.begin(), minRedString.end(), ',', '.');

        filestr << "\"max\" : [ " << maxRedString << "," << maxGreenString << "," << maxBlueString << "]," << '\n';
        filestr << "\"min\" : [ " << minRedString << "," << minGreenString << "," << minBlueString << "]" << '\n';

        if (!mExportVertNormal && !mExportTextureCoordinates){
            //last closing brackets aren't allowed to have a comma
            if (indexMeshPart+1 == nrPartsOfMesh){
                filestr << "}" << '\n';
            }
            else{
                filestr << "}," << '\n';
            }
        }
        else{
            filestr << "}," << '\n';
            if(mExportVertNormal){
                filestr << "{" << '\n';
                //------------------------------------------------------
                //normal accesor
                filestr << "\"bufferView\" : " << std::to_string(indexMeshPart*nrOfBufferTypes + 3) << "," <<  '\n';
                filestr << "\"byteOffset\" : 0," << '\n';
                filestr << "\"componentType\" : 5126," << '\n';
                filestr << "\"count\" : " << std::to_string(partNrOfPremitives[indexMeshPart]) << "," << '\n';
                filestr << "\"type\" : \"VEC3\"," << '\n';
                std::setprecision(16);
                std::string maxNormalXString = std::to_string(bufferMaxNormals[indexMeshPart][0]);
                std::replace( maxNormalXString.begin(), maxNormalXString.end(), ',', '.');
                std::string maxNormalYString = std::to_string(bufferMaxNormals[indexMeshPart][1]);
                std::replace( maxNormalYString.begin(), maxNormalYString.end(), ',', '.');
                std::string maxNormalZString = std::to_string(bufferMaxNormals[indexMeshPart][2]);
                std::replace( maxNormalZString.begin(), maxNormalZString.end(), ',', '.');
                std::string minNormalXString = std::to_string(bufferMinNormals[indexMeshPart][0]);
                std::replace( minNormalXString.begin(), minNormalXString.end(), ',', '.');
                std::string minNormalYString = std::to_string(bufferMinNormals[indexMeshPart][1]);
                std::replace( minNormalYString.begin(), minNormalYString.end(), ',', '.');
                std::string minNormalZString = std::to_string(bufferMinNormals[indexMeshPart][2]);
                std::replace( minNormalZString.begin(), minNormalZString.end(), ',', '.');
                std::setprecision(6);

                filestr << "\"max\" : [ " << maxNormalXString << "," << maxNormalYString << "," << maxNormalZString  << "]," << '\n';
                filestr << "\"min\" : [ " << minNormalXString << "," << minNormalYString << "," << minNormalZString  << "]" << '\n';

                if (!mExportTextureCoordinates){
                    filestr << "}" << '\n';
                }
                else{
                    filestr << "}," << '\n';
                }
            }
            //-----------------------------------------------------------
            //uv buffer / texture coords accesor
            if(mExportTextureCoordinates){
                filestr << "{" << '\n';
                //bufferView ID depends on the normals export decision
                if (mExportVertNormal){
                    filestr << "\"bufferView\" : " << std::to_string(indexMeshPart*nrOfBufferTypes + 4) << "," <<  '\n';
                }
                else{
                    filestr << "\"bufferView\" : " << std::to_string(indexMeshPart*nrOfBufferTypes + 3) << "," <<  '\n';
                }
                filestr << "\"byteOffset\" : 0," << '\n';
                filestr << "\"componentType\" : 5126," << '\n';
                filestr << "\"count\" : " << std::to_string(partNrOfPremitives[indexMeshPart]) << "," << '\n';
                filestr << "\"type\" : \"VEC2\"," << '\n';
                std::string maxUString = std::to_string(bufferMaxUVPositions[indexMeshPart][0]);
                std::replace( maxUString.begin(), maxUString.end(), ',', '.');
                std::string maxVString = std::to_string(bufferMaxUVPositions[indexMeshPart][1]);
                std::replace( maxVString.begin(), maxVString.end(), ',', '.');
                std::string minUString = std::to_string(bufferMinUVPositions[indexMeshPart][0]);
                std::replace( minUString.begin(), minUString.end(), ',', '.');
                std::string minVString = std::to_string(bufferMinUVPositions[indexMeshPart][1]);
                std::replace( minVString.begin(), minVString.end(), ',', '.');

                filestr << "\"max\" : [ " << maxUString << "," << maxVString << "]," << '\n';
                filestr << "\"min\" : [ " << minUString << "," << minVString << "]" << '\n';
                //seperate by comma, if there follow buffer of other mesh parts
                if(indexMeshPart < nrPartsOfMesh-1){
                    filestr << "}," << '\n';
                }
                else{
                    filestr << "}" << '\n';
                }

            }
        }
    }
    filestr << "]," << '\n';

    //GLTF version
    filestr << "\"asset\" : {" << '\n';
    filestr << "\"version\" : \"2.0\"" << '\n';
    filestr << "}" << '\n';
    filestr << "}" << '\n';

    filestr.close();

    LOG::debug() << "[GltfWriter::" << __FUNCTION__ << "] write ASCII GLTF:      " << static_cast<float>( clock() -timeStart ) / CLOCKS_PER_SEC << " seconds. " << '\n';

    return true;
}
