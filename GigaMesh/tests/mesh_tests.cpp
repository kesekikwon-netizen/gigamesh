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
#include <GigaMesh/mesh/mesh.h>

//Mock wrapper class for Mesh
// Goals:
// -- make testing of showEnter* functions possible
// -- silence show* functions
class MockMesh : public Mesh
{
	public:
		MockMesh(const std::string& file, bool& success) : Mesh(file, success) {}
		virtual ~MockMesh() override = default;
		// UserInteraction interface
	protected:
		void showInformation(const std::string& rHead, const std::string& rMsg, const std::string& rToClipboard) override
		{
		}
		void showWarning(const std::string& rHead, const std::string& rMsg) override
		{
		}
		bool showEnterText(std::string& rSomeStrg, const char* rTitle) override
		{
			return true;
		}
		bool showEnterText(uint64_t& rULongInt, const char* rTitle) override
		{
			return true;
		}
		bool showEnterText(double& rDoubleVal, const char* rTitle) override
		{
			return true;
		}
		bool showEnterText(std::set<int64_t>& rIntegers, const char* rTitle) override
		{
			return true;
		}
		bool showEnterText(std::vector<long>& rIntegers, const char* rTitle) override
		{
			return true;
		}
		bool showEnterText(std::vector<double>& rDoubles, const char* rTitle) override
		{
			return true;
		}
		bool showEnterText(Matrix4D* rMatrix4x4, bool selectedVerticesOnly = false) override
		{
			return true;
		}
		bool showSlider(double* rValueToChange, double rMin, double rMax, const char* rTitle) override
		{
			return true;
		}
		bool showQuestion(bool* rUserChoice, const std::string& rHead, const std::string& rMsg) override
		{
			return true;
		}

		// ShowProgress interface
	public:
		void showProgressStart(const std::string& rMsg) override
		{
		}
		bool showProgress(double rVal, const std::string& rMsg) override
		{
			return true;
		}
		void showProgressStop(const std::string& rMsg) override
		{
		}
};

//catch TEST_CASE / SECTION format
TEST_CASE("Testing basic mesh with invalid file", "[mesh]")
{
	SECTION("Creating a new mesh with invalid input")
	{
		bool success = false;
		MockMesh testMesh("notavalidMeshname", success);
		REQUIRE(success == false);
	}
}

//catch BDD style testing
SCENARIO("Loading a valid mesh", "[mesh]")
{
	GIVEN("A mesh with valid data")
	{
		bool success = false;
		MockMesh testMesh("testdata/singletriangle.obj", success);
		REQUIRE(success == true);  //REQUIRE => abort if fails
		WHEN("Loading the singletriangle.obj")
		{
			THEN("The position should be the center of the triangle")
			{
				/*
				 * the points of singletriangle.obj:
				 * p1(10.0, 10.0, 10.0);
				 * p2(200.0, 20.0, 20.0);
				 * p3(100.0, 200.0, 30.0);
				 *
				 * getX/Y/Z is defined by (BBoxMax + BBoxMin) / 2
				*/

				Vector3D pmin(10.0, 10.0, 10.0);
				Vector3D pmax(200.0, 200.0, 30.0);
				Vector3D centerpoint = (pmin + pmax) / 2.0;

				CHECK(testMesh.getX() == Approx(centerpoint.getX())); //CHECK => continue, even with failure
				CHECK(testMesh.getY() == Approx(centerpoint.getY()));
				CHECK(testMesh.getZ() == Approx(centerpoint.getZ()));
			}
			AND_THEN("The center of gravity should be in the middle of the triangle") //careful, each scope has its own mesh, so a new Mesh is created here!
			{
				// the points of singletriangle.obj:
				Vector3D p1(10.0, 10.0, 10.0);
				Vector3D p2(200.0, 20.0, 20.0);
				Vector3D p3(100.0, 200.0, 30.0);

				auto pCenter = (p1+p2+p3) / 3.0;

				auto cog = testMesh.getCenterOfGravity();

				CHECK(pCenter.getX() == Approx(cog.getX()));
				CHECK(pCenter.getY() == Approx(cog.getY()));
				CHECK(pCenter.getZ() == Approx(cog.getZ()));
			}
		}
	}
}

SCENARIO("Testing translation of selected vertices", "[mesh]")
{
	GIVEN("A simple triangle mesh")
	{
		bool success = false;
		MockMesh testMesh("testdata/singletriangle.obj", success);
		REQUIRE(success == true);
		REQUIRE(testMesh.getVertexNr() == 3);
		WHEN("Translating a selected vertex")
		{
			auto selVertex = *testMesh.getVertexPos(0);
			auto vert2 = *testMesh.getVertexPos(1);
			auto vert3 = *testMesh.getVertexPos(2);
			testMesh.selectVertsByIdx(std::vector<long>{0});

			std::set<Vertex*> selectedVertices;
			testMesh.getSelectedVerts(&selectedVertices);

			THEN("Translating the vertex by 1 unit along x")
			{
				auto transMat = {1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  1.0, 0.0, 0.0, 1.0};
				Matrix4D transform(transMat);

				testMesh.applyTransformation(transform, &selectedVertices);

				auto transFormedVertex = testMesh.getVertexPos(0);

				//selected vertex get transformed by 1 unit along x
				CHECK(selVertex.getX() + 1.0 == Approx(transFormedVertex->getX()));
				CHECK(selVertex.getY()       == Approx(transFormedVertex->getY()));
				CHECK(selVertex.getZ()       == Approx(transFormedVertex->getZ()));

				//other vertices are untouched
				CHECK(vert2.getX() == Approx(testMesh.getVertexPos(1)->getX()));
				CHECK(vert2.getY() == Approx(testMesh.getVertexPos(1)->getY()));
				CHECK(vert2.getZ() == Approx(testMesh.getVertexPos(1)->getZ()));

				CHECK(vert3.getX() == Approx(testMesh.getVertexPos(2)->getX()));
				CHECK(vert3.getY() == Approx(testMesh.getVertexPos(2)->getY()));
				CHECK(vert3.getZ() == Approx(testMesh.getVertexPos(2)->getZ()));
			}
			THEN("Translating the vertex by 1 unit along y")
			{
				auto transMat = {1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 1.0, 0.0, 1.0};
				Matrix4D transform(transMat);

				testMesh.applyTransformation(transform, &selectedVertices);

				auto transFormedVertex = testMesh.getVertexPos(0);

				//selected vertex get transformed by 1 unit along y
				CHECK(selVertex.getX()       == Approx(transFormedVertex->getX()));
				CHECK(selVertex.getY() + 1.0 == Approx(transFormedVertex->getY()));
				CHECK(selVertex.getZ()       == Approx(transFormedVertex->getZ()));

				//other vertices are untouched
				CHECK(vert2.getX() == Approx(testMesh.getVertexPos(1)->getX()));
				CHECK(vert2.getY() == Approx(testMesh.getVertexPos(1)->getY()));
				CHECK(vert2.getZ() == Approx(testMesh.getVertexPos(1)->getZ()));

				CHECK(vert3.getX() == Approx(testMesh.getVertexPos(2)->getX()));
				CHECK(vert3.getY() == Approx(testMesh.getVertexPos(2)->getY()));
				CHECK(vert3.getZ() == Approx(testMesh.getVertexPos(2)->getZ()));
			}
			THEN("Translating the vertex by 1 unit along z")
			{
				auto transMat = {1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 1.0};
				Matrix4D transform(transMat);

				testMesh.applyTransformation(transform, &selectedVertices);

				auto transFormedVertex = testMesh.getVertexPos(0);

				//selected vertex get transformed by 1 unit along z
				CHECK(selVertex.getX()       == Approx(transFormedVertex->getX()));
				CHECK(selVertex.getY()       == Approx(transFormedVertex->getY()));
				CHECK(selVertex.getZ() + 1.0 == Approx(transFormedVertex->getZ()));

				//other vertices are untouched
				CHECK(vert2.getX() == Approx(testMesh.getVertexPos(1)->getX()));
				CHECK(vert2.getY() == Approx(testMesh.getVertexPos(1)->getY()));
				CHECK(vert2.getZ() == Approx(testMesh.getVertexPos(1)->getZ()));

				CHECK(vert3.getX() == Approx(testMesh.getVertexPos(2)->getX()));
				CHECK(vert3.getY() == Approx(testMesh.getVertexPos(2)->getY()));
				CHECK(vert3.getZ() == Approx(testMesh.getVertexPos(2)->getZ()));
			}
		}
	}
}
