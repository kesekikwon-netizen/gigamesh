/* * GigaMesh - The GigaMesh Software Framework is a modular software for display,
 * editing and visualization of 3D-data typically acquired with structured light or
 * structure from motion.
 * Copyright (C) 2009-2020 Hubert Mara
 *
 * This file is part of GigaMesh.
 *
 * GigaMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GigaMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GigaMesh.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GLTFWRITER_H
#define GLTFWRITER_H

#include "MeshWriter.h"
typedef unsigned char BYTE;

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


class GltfWriter : public MeshWriter
{
public:
    GltfWriter() = default;

    // MeshWriter interface
public:
    bool writeFile(const std::filesystem::path& rFilename, const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps, MeshSeedExt& rMeshSeed) override;

private:
    void createBuffersWithoutTextureCoords(std::vector<BYTE> *indexBuffer, std::vector<BYTE> *vertexBuffer, std::vector<BYTE> *vertexColorBuffer,  std::vector<BYTE> *normalsBuffer,
                                           float minPosition[3], float maxPosition[3], float minColor[3], float maxColor[3], float minNormals[3], float maxNormals[3],
                                            const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps);
    void createBuffersIncludingTextureCoords(const unsigned int textureId, std::vector<BYTE> *indexBuffer, std::vector<BYTE> *vertexBuffer, std::vector<BYTE> *vertexColorBuffer, std::vector<BYTE> *normalsBuffer, std::vector<BYTE> *uvCoordsBuffer,
                                           float minPosition[3], float maxPosition[3], float minColor[3], float maxColor[3], float minNormals[3], float maxNormals[3], float minTextureCoords[2], float maxTextureCoords[2],
                                            const std::vector<sVertexProperties>& rVertexProps, const std::vector<sFaceProperties>& rFaceProps);
    void addFloatHexStringToBuffer(std::vector<BYTE> *buffer, float value);

    void getBufferByteOffsets(int rPart, std::vector<int> indexBufferSizes, std::vector<int> vertexBufferSizes,std::vector<int> vertexColorBufferSizes, std::vector<int> normalsBufferSizes, std::vector<int> uvCoordsBufferSizes,
                             int *offsetIndexBuffer, int *offsetVertexBuffer, int *offsetVertexColorBuffer, int *offsetNormalsBuffer, int *offsetTextureCoordsBuffer);
};

#endif // GLTFWRITER_H
