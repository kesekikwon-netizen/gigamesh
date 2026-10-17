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

#ifndef TRIANGULARPRISM_H
#define TRIANGULARPRISM_H

#include <memory>

#include "vector3d.h"

class Plane;
class Line;

class TriangularPrism {
	public:
		TriangularPrism(Vector3D* vertices);
		Line gettriangularPrismEdge(int nr);
		std::unique_ptr<Plane> gettriangularPrismPlane(int nr);
		bool PointisintringularPrism(Vector3D& p);

	private:
		Vector3D mVertex[6];

};

#endif // TRIANGULARPRISM_H
