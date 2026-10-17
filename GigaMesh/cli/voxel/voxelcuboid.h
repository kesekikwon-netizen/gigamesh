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

#ifndef VOXELCUBE_H
#define VOXELCUBE_H

// generic C++ includes:
#include <string>
#include <iostream> // cout, endl

#include <cstdlib> // uint, calloc
#include <cmath>    // pow, sqrt, floor, exp, ...
#include <climits> // UCHAR_MAX, ...
#include <cfloat>  // FLT_MAX, ...
#include <cstdio>	// fopen, fwrite, fclose

#include <chrono>

// C++ includes:
#include "voxelsphereparam.h"
#include "vector3d.h"


#define _RASTER_MODE_SET_               0
#define _RASTER_MODE_AND_               1
#define _RASTER_MODE_OR_                2
#define _RASTER_MODE_INTERSECT_GREATER_ 3
#define _RASTER_MODE_MULTIPLY_SCALE_    4

#define _SPHERE_FUNC_SOLID_  0
#define _SPHERE_FUNC_GAUSS_  1

using uint = unsigned int;

//!
//! \brief VoxelCuboid for handling volume data. (Layer 0)
//!
//! The VoxelCuboid - or in most cases a cube of voxels - stores
//! and handles volume data (in contrast to Mesh). (Layer 0)
//!
//! Layer 0
//!

class VoxelCuboid {
	public:
		// constructor and deconstructor:
		VoxelCuboid( uint setXYZDim );
		VoxelCuboid( uint setXYZDim, uint setFilterKernelNr, double* sphereRadii, bool silent=true );
		VoxelCuboid( uint setXDim, uint setYDim, uint setZDim );
		~VoxelCuboid();

		// filters:
		double  setFilterKernelSphere( unsigned char* filterVoxels, float sphereRadius );
		void applyFilters( double* featureVec ); // double* applyFilters();

		// getting data/information:
		unsigned char* getCuboidArrayRef();
		bool           isCube();
		double         getVolume();

		// setting raster data:
		double rasterSetTo( unsigned char value );
		double rasterZBuffer(const double* rasterArray );
		double rasterSphere( VoxelSphereParam sphereParam );

		double estimateVoxelIntersectSphere( Vector3D* voxelTopLeftBack, float voxelEdgeLen, Vector3D* sphereCenter, float sphereRadius );

		// integral:
		double* integralSpheres( float* sphereRadii, uint nrRadii );
		double  estVolumeIdealSphere( float voxelRadius=1.0 );

		// io-operation:
		int writeDat(const std::string& filename );

	private:
		double setEightVoxelsSymmetric( unsigned char* someCuboid, uint x, uint y, uint z, unsigned char value, int rasterMode=_RASTER_MODE_SET_ );
		double setEightVoxelsSymmetric( uint x, uint y, uint z, unsigned char value, int rasterMode=_RASTER_MODE_SET_ );

	private:
		uint xDim;                           //!< number of voxles along the x-axis
		uint yDim;                           //!< number of voxles along the y-axis
		uint zDim;                           //!< number of voxles along the z-axis
		unsigned char*  cuboidArray;         //!< array storing the voxels densities [0...255]
		unsigned char*  cuboidArrayOriginal; //!< array storing the voxels original densities - can (and should not be) alterd! Required by VoxelCuboid::applyFilters, set by raster methods.
		unsigned char** filterKernelArray;   //!< array storing the voxels densities [0...255]
		double*         filterKernelVolumes; //!< Volume of each filter kernel (for normalization).
		uint            filterKernelNr;      //!< number of filters
};

#endif
