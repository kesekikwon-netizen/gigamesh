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

#include <GigaMesh/mesh/sphere.h>

using namespace std;

Sphere::Sphere( Vector3D vertPos, float setRadius, unsigned char setR, unsigned char setG, unsigned char setB )
	: Vertex( vertPos ) {
	//! Constructor - requires only coordiantes and radii. Memory will be allocated. Optional a color can be set.
	//!
	//! This constructor is more convinient as it does not require an array to be handled outside this object.
	//!
	//! But: e.g. if this object has to be transfomed, this has to be done with its own method. 
	//!
	//! Or in other words: using this Constructor prevents CUDA/OpenCL integration and should therefore only be used 
	//! for testing or simple visualizations.
	radius = setRadius;
	if( !setRGB( setR, setG, setB ) ) {
		cerr << "[Sphere::" << __FUNCTION__ << "] ERROR: Could not set RGB!" << endl;
	}
}

Sphere::~Sphere() {
	//! Destructor. sets properties to not a number or NULL.
	radius = _NOT_A_NUMBER_;
}

float Sphere::getRadius() {
	//! Returns the radius of the Sphere.
	return radius;
}

int Sphere::getType() {
	//! Return the type (=class) of this object as an id.
	return Primitive::IS_SPHERE;
}
