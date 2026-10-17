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

#ifndef COMPFEATUREVECS_H
#define COMPFEATUREVECS_H
#include <GigaMesh/mesh/mesh.h>

//! Struct to pass settings and results from and to the threads
//! applying the MSII filter.
struct sMeshDataStruct {
	int     threadID{0}; //!< ID of the posix thread
	// input:
	Mesh*   meshToAnalyze{nullptr};
	double  radius{0.0};
	uint    xyzDim{0};
	uint    multiscaleRadiiSize{0};
	double* multiscaleRadii{nullptr};
	// output:
	int     ctrIgnored{0};
	int     ctrProcessed{0};
	// Smooth normal of the largest spherical neighbourhood i.e. largest scale
	std::vector<MeshIO::grVector3ID>* mPatchNormal{nullptr}; //!< Normal used for orientation into 2.5D representation
	// our most precious feature vectors (as array):
	double* descriptVolume{nullptr};  //!< Volume descriptors
	double* descriptSurface{nullptr}; //!< Surface descriptors
	// and the voxel filter
	voxelFilter2DElements** sparseFilters{nullptr};
	// Collect compute time
	int     mWallTimeThread{0};
};

//! Compute the Multi-Scale Integral Invariant feature vectors.
//! Function to be called multiple times depending on the requested
//! number of threads. Typically called by compFeatureVectorsMain.
void compFeatureVectorsThread(
                sMeshDataStruct*   rMeshData,
                const size_t       rThreadOffset,
                const size_t       rThreadVertexCount
);

//! Compute the Multi-Scale Integral Invariant feature vectors.
//! Main function to be called. Takes care about multi-threaded
//! computation.
void compFeatureVectorsMain(
                sMeshDataStruct*   rMeshData,
                const unsigned int rThreadVertexCount //!<
);

#endif
