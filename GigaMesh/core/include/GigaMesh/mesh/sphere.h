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

#ifndef SPHERE_H
#define SPHERE_H

#include "vertex.h"

//!
//! \brief Class for handling spheres. (Layer 0.5 - Datums)
//!
//! Sphere (Datum) class.
//!
//! Layer 0.5
//!

class Sphere : public Vertex {
	public:
		// const- & destructor:
		Sphere( Vector3D vertPos, float setRadius, unsigned char setR=0, unsigned char setG=0, unsigned char setB=0 );
		~Sphere();

		float getRadius();
		int   getType();

	private:
		float radius;       //!< Radius of the sphere.
};

#endif
