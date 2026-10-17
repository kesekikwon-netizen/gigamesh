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

#include <catch.hpp>
#include "../core/mesh/MeshIO/ObjReader.h"
#include "../core/mesh/MeshIO/PlyReader.h"
#include "../core/mesh/MeshIO/PlyWriter.h"
#include "../core/mesh/MeshIO/ObjWriter.h"
#include <GigaMesh/mesh/meshio.h>
#include <GigaMesh/mesh/vector3d.h>
#include "../core/mesh/util/triangulation.h"

const std::string gTestFilesPath("testdata/");

//unit test => check if the reader to their work properly
TEST_CASE("Mesh Reader Tests", "[meshio]")
{
	std::vector<sVertexProperties> vertexProperties;
	std::vector<sFaceProperties> faceProperties;
	MeshSeedExt meshSeed;

	SECTION("ObjReader - reading plain obj")
	{
		ObjReader reader;
		reader.readFile(gTestFilesPath + "cube.obj", vertexProperties, faceProperties, meshSeed);

		CHECK(vertexProperties.size() == 8);
		CHECK(faceProperties.  size() == 12);
	}

	SECTION("ObjReader - reading obj with quads")
	{
		ObjReader reader;
		reader.readFile(gTestFilesPath + "flat.obj", vertexProperties, faceProperties, meshSeed);

		CHECK(vertexProperties.size() == 25);
		CHECK(faceProperties.  size() == 16);

		for(auto& face : faceProperties)
		{
			REQUIRE(face.vertexIndices.size() == 4);
		}
	}

	SECTION("ObjReader - reading ngon obj")
	{
		ObjReader reader;
		reader.readFile(gTestFilesPath + "ngon_concave.obj", vertexProperties, faceProperties, meshSeed);

		CHECK(vertexProperties.size() == 5);
		REQUIRE(faceProperties.size() == 1);
		CHECK(faceProperties[0].vertexIndices.size() == 5);
		CHECK(faceProperties[0].textureCoordinates.size() == 0);
	}

	SECTION("PlyReader - reading ngon ascii ply")
	{
		PlyReader reader;
		reader.readFile(gTestFilesPath + "ngon_concave.ply", vertexProperties, faceProperties, meshSeed);

		CHECK(vertexProperties.size() == 5);
		REQUIRE(faceProperties.size() == 1);
		CHECK(faceProperties[0].vertexIndices.size() == 5);
		CHECK(faceProperties[0].textureCoordinates.size() == 0);
	}
}

TEST_CASE("MeshIO Write Tests", "[meshio]")
{
	MeshIO meshIO;
	std::vector<sVertexProperties> vertexProperties;
	std::vector<sFaceProperties> faceProperties;


	meshIO.readFile(gTestFilesPath + "singletriangle.obj", vertexProperties, faceProperties);

	SECTION("MeshIO ply saving")
	{
		std::filesystem::path outFile(gTestFilesPath + "tmpPly.ply");
		if(std::filesystem::exists(outFile))
		{
			std::filesystem::remove(outFile);
		}

		meshIO.writeFilePrimProps( outFile, vertexProperties, faceProperties );

		REQUIRE(std::filesystem::exists(outFile) == true);

		std::filesystem::remove(outFile);
	}

	SECTION("MeshIO obj saving")
	{
		std::filesystem::path outFile(gTestFilesPath + "tmpObj.obj");
		if(std::filesystem::exists(outFile))
		{
			std::filesystem::remove(outFile);
		}

		meshIO.writeFilePrimProps( outFile, vertexProperties, faceProperties );

		REQUIRE(std::filesystem::exists(outFile) == true);

		std::filesystem::remove(outFile);
	}
}

//unit test => no need to check if the mesh was loaded correctly, only that it was loaded
//==> correct functionality is checked by the unit test
TEST_CASE("MeshIO integration test", "[meshio]")
{
	MeshIO meshIO;

	std::vector<sVertexProperties> vertexProperties;
	std::vector<sFaceProperties> faceProperties;
	SECTION("MeshIO obj integration")
	{
		meshIO.readFile(gTestFilesPath + "singletriangle.obj", vertexProperties, faceProperties);

		CHECK(vertexProperties.empty() == false);
		CHECK(faceProperties.  empty() == false);

		auto baseName = meshIO.getBaseName();
		REQUIRE(baseName == "singletriangle");
		auto extension = meshIO.getFileExtension();
		REQUIRE(extension == "obj");

		auto filePath = meshIO.getFileLocation();
		auto fullName = meshIO.getFullName();

		REQUIRE(fullName.wstring() == filePath.wstring() + baseName.wstring() + L"." + extension.wstring());
	}
}

double triangleArea(const Vector3D& vertA, const Vector3D& vertB, const Vector3D& vertC)
{
	auto AB = vertB - vertA;
	AB.setH(1.0);
	auto AC = vertC - vertA;
	AC.setH(1.0);
	return 0.5 * (AB * AC);
}

TEST_CASE("Mesh Triangulation Test", "[meshio]")
{
	std::vector<Vector3D> vertices;

	SECTION("Triangulate concave mesh")
	{
		vertices.reserve(5);

		vertices.emplace_back(Vector3D(0.0, 0.0, 0.0));
		vertices.emplace_back(Vector3D(0.0, 2.0, 0.0));
		vertices.emplace_back(Vector3D(1.0, 1.0, 0.0));
		vertices.emplace_back(Vector3D(2.0, 2.0, 0.0));
		vertices.emplace_back(Vector3D(2.0, 0.0, 0.0));

		const size_t numTriangleIndices = (vertices.size() - 2) * 3;
		auto indices = GigaMesh::Util::triangulateNgon(vertices);

		REQUIRE(indices.size() == numTriangleIndices);

		for(auto index : indices)
		{
			REQUIRE(index < vertices.size());
		}

		for(size_t i = 0; i<indices.size(); i +=3)
		{
			auto area = triangleArea(vertices[indices[i]],vertices[indices[i+1]],vertices[indices[i+2]]);
			CHECK(area != Approx(0.0));
		}
	}

	SECTION("Tirangulate Quad Mesh")
	{
		vertices.reserve(4);

		SECTION("short 1-3 edge")
		{
			vertices.emplace_back(Vector3D( 0.0, 0.0, 0.0));
			vertices.emplace_back(Vector3D(-1.0, 2.0, 0.0));
			vertices.emplace_back(Vector3D( 0.0, 4.0, 0.0));
			vertices.emplace_back(Vector3D( 1.0, 2.0, 0.0));

			auto indices = GigaMesh::Util::triangulateNgon(vertices);


			REQUIRE(indices.size() == 6);

			bool correctlyTriangulated = true;
			std::vector<size_t> validTriangulation = {0,1,3,1,2,3};

			for(size_t i = 0; i<6;++i)
			{
				correctlyTriangulated &= validTriangulation[i] == indices[i];
			}

			CHECK(correctlyTriangulated == true);
		}

		SECTION("short 0-2 edge")
		{
			vertices.emplace_back(Vector3D( 0.0, 0.0, 0.0));
			vertices.emplace_back(Vector3D(-2.0, 1.0, 0.0));
			vertices.emplace_back(Vector3D( 0.0, 2.0, 0.0));
			vertices.emplace_back(Vector3D( 1.0, 1.0, 0.0));

			auto indices = GigaMesh::Util::triangulateNgon(vertices);


			REQUIRE(indices.size() == 6);

			bool correctlyTriangulated = true;
			std::vector<size_t> validTriangulation = {0,1,2,0,2,3};

			for(size_t i = 0; i<6;++i)
			{
				correctlyTriangulated &= validTriangulation[i] == indices[i];
			}

			CHECK(correctlyTriangulated == true);
		}
	}
}
