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

#ifndef VOXELSPHEREPARAM_H
#define VOXELSPHEREPARAM_H

#include <stdlib.h> // uint

//!
//! \brief Structure to set parameters for estimation of a Sphere described by voxels.
//!
//! The struct holds the diameter, density, raster mode, etc.
//!
typedef unsigned int uint;

struct VoxelSphereParam {
	uint  sphereRadius; //!< Radius of the sphere in voxel.
	float baseDensity;  //!< Density offset multiplied with whatever density function used (use 1.0 for full voxels). 
	int   rasterMode;   //!< Defines the operation with existing data in the grid.
	int   radiusFunc;   //!< Defines the function used to estimate a voxels density as function of the radius.
	float gaussFuncSig; //!< Parameter sigma of the gaussian function (use 0.4 for a start as it will scale the density to ~maximum at the center).
	float gaussFuncMu;  //!< Parameter mu (µ) of the gaussian function (use 0.0 for a start as it will scale the density to ~0 at the spheres surface).
};

#endif
