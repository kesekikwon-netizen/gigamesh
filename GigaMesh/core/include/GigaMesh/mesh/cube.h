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

#ifndef CUBE_H
#define CUBE_H

#include "vertex.h"
#include "vector3d.h"
//#include "face.h"
#include "line.h"
#include "triangularprism.h"


enum relativePosition {
	CONTAINED,
	INTERSECTION,
	DISJOINT
};


class Cube {
	public:

		Cube(const Vector3D & center, double scale);
		Cube(double x, double y, double z, double scale);



		Vector3D getVertex(int nr);
		Line getEdge(int nr);
		std::unique_ptr<Plane> getPlane(int nr);

		bool PointisinCube(Vector3D& p);
        bool isFaceCompletelyInCube(Face* face);

		//! check if theres an intersection between triangular prism and octnode cube
		//test if there is intersection of this cube with triangular prism,
		//tri is a vector containing the 6 points which describe 5 planes of the triangular prism
		enum relativePosition CubeisintriangularPrism(TriangularPrism& tri, std::vector<Line> &drawlines);


		bool linecubeintersection(Vector3D* a, Vector3D* b);
		bool trianglecubeintersection(Face* face);



		Vector3D mcenter;
		double mscale; //is 0.5 times the edgelen of this cube

};

#endif // CUBE_H
