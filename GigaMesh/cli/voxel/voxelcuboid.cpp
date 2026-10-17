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

#include <cstdio>

#include "voxelcuboid.h"

#include "gmcommon.h" // for windows

using namespace std;

VoxelCuboid::VoxelCuboid( uint setXYZDim ) {
	//! Constructor for a cubic Voxel grid.
	xDim = setXYZDim;
	yDim = setXYZDim;
	zDim = setXYZDim;

	// we have no filters attached:
	filterKernelNr      = 0;
	filterKernelArray   = nullptr;
	filterKernelVolumes = nullptr;

	cuboidArray         = static_cast<unsigned char*>(calloc( xDim * yDim * zDim, sizeof( unsigned char ) ));
	cuboidArrayOriginal = static_cast<unsigned char*>(calloc( xDim * yDim * zDim, sizeof( unsigned char ) ));
}

VoxelCuboid::VoxelCuboid( uint setXYZDim, uint setFilterKernelNr, double* sphereRadii, bool silent ) {
	//! Constructor for a cubic Voxel grid and initalization of sphereical filter kernels.
	xDim = setXYZDim;
	yDim = setXYZDim;
	zDim = setXYZDim;

	int timeStart, timeStop;    // for performance mesurement
	if( !silent ) {
		timeStart = clock();
	}

	cout << setiosflags( ios_base::fixed );

	// we attach spherical filters (and allocate memory);
	filterKernelNr      = setFilterKernelNr;
	filterKernelArray   = static_cast<unsigned char**>(calloc( filterKernelNr, sizeof( unsigned char* ) ));
	filterKernelVolumes = static_cast<double*>(calloc( filterKernelNr, sizeof( double ) ));
	for( uint i=0; i<filterKernelNr; i++ ) {
		filterKernelArray[i] = static_cast<unsigned char*>(calloc( xDim * yDim * zDim, sizeof( unsigned char ) ));
		filterKernelVolumes[i] = setFilterKernelSphere( filterKernelArray[i], sphereRadii[i]*setXYZDim/2.0 );
		double filterKernelVolumeIdeal = estVolumeIdealSphere( sphereRadii[i]*setXYZDim/2.0 );
		if( !silent ) {
			cout << "[VoxelCuboid::VoxelCuboid] Volume filter Kernel (r=" << setprecision( 1 ) << sphereRadii[i] << "): " << static_cast<int>(filterKernelVolumes[i]) << " (Ideal: " << static_cast<int>(filterKernelVolumeIdeal) << ") Error: " << setprecision( 5 ) << (filterKernelVolumes[i]-filterKernelVolumeIdeal)/filterKernelVolumeIdeal << " %" << endl;
		}
	}

	if( !silent ) {
		timeStop  = clock();
		cout << "[VoxelCuboid::VoxelCuboid] estimating filter kernels took " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	}

	// allocate memory for our voxelcuboid
	cuboidArray         = static_cast<unsigned char*>(calloc( xDim * yDim * zDim, sizeof( unsigned char ) ));
	cuboidArrayOriginal = static_cast<unsigned char*>(calloc( xDim * yDim * zDim, sizeof( unsigned char ) ));
}

VoxelCuboid::VoxelCuboid( uint setXDim, uint setYDim, uint setZDim ) {
	//! Constructor for a non-cubic Voxel grid.
	xDim = setXDim;
	yDim = setYDim;
	zDim = setZDim;

	// we have no filters attached:
	filterKernelNr      = 0;
	filterKernelArray   = nullptr;
	filterKernelVolumes = nullptr;

	cuboidArray         = static_cast<unsigned char*>(calloc( xDim * yDim * zDim, sizeof( unsigned char ) ));
	cuboidArrayOriginal = static_cast<unsigned char*>(calloc( xDim * yDim * zDim, sizeof( unsigned char ) ));
}

VoxelCuboid::~VoxelCuboid() {
	//! Destructor

	//int timeStart, timeStop;    // for performance mesurement
	//timeStart = clock();

	if( cuboidArray != nullptr ) {
		free( cuboidArray );
	}
	if( cuboidArrayOriginal != nullptr ) {
		free( cuboidArrayOriginal );
	}
	if( filterKernelArray != nullptr ) {
		for( uint i=0; i<filterKernelNr; i++ ) {
			free( filterKernelArray[i] );
		}
		free( filterKernelArray );
	}
	if( filterKernelVolumes != nullptr ) {
		free( filterKernelVolumes );
	}
	//timeStop  = clock();
	//cout << "[VoxelCuboid::~VoxelCuboid] freeing memory took " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
}

// filters --------------------------------------------------------------------

double VoxelCuboid::setFilterKernelSphere( unsigned char* filterVoxels, float sphereRadius ) {
	//! Sets an internal voxel filter to a solid, centered sphere with a radius in Voxel.
	//!
	//! Returns the volume of this spherical filter kernel.
	//!
	//! This is a simplified version of VoxelCuboid::rasterSphere

	double voxlesFilled = 0.0; // do NOT use float here - will get you into troubles in regard to precision
	float  voxelDistanceToCenter = 0.0;
	float  xd, yd, zd;
	float  fillLevel;
	float  setValue;
	for( uint z=0; z<zDim/2; z++ ) {
		for( uint x=0; x<xDim/2; x++ ) {
			for( uint y=0; y<yDim/2; y++ ) {
				// fast but unexact:
				xd = x; yd = y; zd = z; 
				voxelDistanceToCenter = sqrt( pow( (xd-xDim/2), 2 ) + pow( (yd-yDim/2), 2 ) + pow( (zd-zDim/2), 2 ) );
				//cout << "[VoxelCuboid] " << xd << ", " << z << ", " << zd-(zDim/2) << " voxelDistanceToCenter: " << voxelDistanceToCenter << " voxels." << endl;
				if( voxelDistanceToCenter <= sphereRadius+sqrt(3) ) {
					// inside sphere or on the surface of the sphere:
					if( voxelDistanceToCenter > sphereRadius ) {
						// on the surface of the sphere - some heuristic guess for the density:
						fillLevel = ( 1.0-( voxelDistanceToCenter-sphereRadius ) / sqrt(3) ) / 1.150;
					} else {
						// inside the sphere, we wan't maximum density
						fillLevel = 1.0;
					}
					// estimate the final density:
					setValue = fillLevel * UCHAR_MAX;
					//! \todo check if we need to take care about over- and underflow
					// take care about UCHAR over- or underflow:
					if( setValue > 255.0 ) {
						//cout << "[VoxelCuboid] voxel > 1.0 " << setValue << endl;
						setValue = 255;
					} else if( setValue < 0 ) {
						//cout << "[VoxelCuboid] voxel < 0.0 " << setValue << endl;
						setValue = 0;
					}
					// final set the voxels density:
					voxlesFilled += setEightVoxelsSymmetric( filterVoxels, x, y, z, setValue, _RASTER_MODE_SET_ );
				} else {
					setEightVoxelsSymmetric( filterVoxels, x, y, z, 0, _RASTER_MODE_SET_ );
				}
			}
		}
	}
	return voxlesFilled;
}

void VoxelCuboid::applyFilters( double* featureVec ) {
	//! Apply the filter kernels and return the feature vector

	//double* featureVec = (double*) calloc( filterKernelNr, sizeof( double ) );

	unsigned char* currentFilterKernel;
	double nominator = pow( UCHAR_MAX, 2.0 );
	uint stopCrit = xDim*yDim*zDim;

	//int timeStart, timeStop;    // for performance mesurement
	//timeStart = clock();
	for( uint fNr=0; fNr<filterKernelNr; fNr++ ) {
		double featureElement = 0.0;
		currentFilterKernel = filterKernelArray[fNr];
		for( uint i=0; i<stopCrit; i++ ) {
			featureElement += ( cuboidArray[i] * currentFilterKernel[i] );
		}
		featureVec[fNr] = ( featureElement / nominator ) / filterKernelVolumes[fNr];
		//cout << "[VoxelCuboid::~applyFilters] feature(" << fNr << ") = " << featureVec[fNr] << endl;
	}
	//timeStop  = clock();
	//cout << "[VoxelCuboid::applyFilters] took " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

/*
	// SLOWER by a factor of ~2
	timeStart = clock();
	for( uint fNr=0; fNr<filterKernelNr; fNr++ ) {
		featureVec[fNr] = 0.0;
	}
	double currElement;
	for( uint i=0; i<stopCrit; i++ ) {
		currElement = cuboidArray[i];
		for( uint fNr=0; fNr<filterKernelNr; fNr++ ) {
			featureVec[fNr] += ( currElement * filterKernelArray[fNr][i] ) / nominator;
		}
	}
	for( uint fNr=0; fNr<filterKernelNr; fNr++ ) {
		cout << "[VoxelCuboid::~applyFilters] feature(" << fNr << ") = " << featureVec[fNr] << endl;
	}
	timeStop  = clock();
	cout << "[VoxelCuboid::applyFilters] took " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
*/
	//return featureVec;
}

// getting data/information ---------------------------------------------------

unsigned char* VoxelCuboid::getCuboidArrayRef() { 
	//! Returns the reference to the cuboids voxel array.
	return cuboidArray;
}

bool VoxelCuboid::isCube() {
	//! Returns true, when the cubiod is actually a cube.
	if( ( xDim == yDim ) && ( yDim == zDim ) ) {
		return true;
	}
	return false;
}

double VoxelCuboid::getVolume() {
	//! Returns the sum of the volume of ALL voxels.
	double voxelVolume = 0.0;
	//cout << "[VoxelCuboid::getVolume] " << xDim*yDim*zDim << endl;
	for( uint i=0; i<(xDim*yDim*zDim); i++ ) {
		voxelVolume += static_cast<double>(cuboidArray[i]);
	}
	return voxelVolume/UCHAR_MAX;
}

// setting data ---------------------------------------------------------------

double VoxelCuboid::rasterSetTo( unsigned char value ) {
	//! Sets ALL voxels to value. Usefull to initalize a cuboid.
	//!
	//! Returns the volume in voxels under respect to their density~fill-height.
	double voxelVolume = 0.0;
	for( uint i=0; i<(xDim*yDim*zDim); i++ ) {
		cuboidArray[i] = value;
		voxelVolume += static_cast<double>(cuboidArray[i]);
	}

	// store a copy (as it might be corrupted by filters)
	memcpy( cuboidArrayOriginal, cuboidArray, xDim*yDim*zDim*sizeof(unsigned char) );
	return voxelVolume/UCHAR_MAX;
}

double VoxelCuboid::rasterZBuffer( const double* rasterArray ) {
	//! Transforms a rasterArray from Mesh::rasterViewFromZ or 
	//! MeshSeed::rasterViewFromZ into a cubic
	//! volume representation for further processing or exporting as image
	//! stack.
	//!
	//! Returns the volume in voxels under respect to their density~fill-height.
	//!
	//! IMPORTANT: rasterArray has to have a size of xDim * yDim!
	//! While the central raster element should have z=0.

	double voxlesFilled = 0.0;

	for( uint z=0; z<zDim; z++ ) {
		for( uint i=0; i<xDim*yDim; i++ ) {
			double zHeight = rasterArray[i] + zDim/2.0;
			if( z < zHeight ) {
				cuboidArray[static_cast<int>(z)*xDim*yDim+i] = 255;
				voxlesFilled += 1.0;
			} else if( ( z - zHeight ) < 1.0 ) {
				cuboidArray[static_cast<int>(z)*xDim*yDim+i] = static_cast<char>(z-zHeight)*255;
				//cout << "[Mesh] rasterToVolume - intersected voxel." << endl;
				voxlesFilled += z - zHeight;
			} else {
				cuboidArray[static_cast<int>(z)*xDim*yDim+i] = 0;
			}
		}
	}
	//cout << "[VoxelCuboid] Volume: " << voxlesFilled << " voxels (" << voxlesFilled*100/(xDim*yDim*zDim) << "%)." << endl;

	// store a copy (as it might be corrupted by filters)
	memcpy( cuboidArrayOriginal, cuboidArray, xDim*yDim*zDim*sizeof(unsigned char) );
	return voxlesFilled;
}

double VoxelCuboid::rasterSphere( VoxelSphereParam sphereParam ) {
	//! Fills voxels inside the given sphere's parameters.
	//!
	//! Returns the volume of the filled voxels.
	//! Returns a negative value in case of an error.
	//!
	//! for estimation of the volume of an ideal sphere see: 
	//! http://en.wikipedia.org/wiki/Sphere#Volume_of_a_sphere

	int timeStart, timeStop;    // for performance mesurement
	timeStart = clock();
	
	double voxlesFilled = 0.0; // do NOT use float here - will get you into troubles in regard to precision
	//! \todo: Check for cubes having an edge length by the power of 2.

	// to be implemented (1/2):
	// for now we always assume our sphere in the center of the VoxelCuboid:
	//Vector3D sphereCenter( xDim/2.0, yDim/2.0, zDim/2.0, 1.0 );
	//float    sphereRadius = xDim/2.0;
	//Vector3D voxelTopLeftBack;
	//float    voxelEdgeLen = 1.0;
	//double   voxelDensitySphere;

	if( isCube() ) {
		// a cube is nice as we simply can estimate the voxels fo 1/8 of the sphere and replicate to the other "quadrants"
		float voxelDistanceToCenter = 0.0;
		float xd, yd, zd;
		float funcValue;
		float fillLevel;
		float setValue;
		for( uint z=0; z<zDim/2; z++ ) {
			for( uint x=0; x<xDim/2; x++ ) {
				for( uint y=0; y<yDim/2; y++ ) {
					// to be implemented (2/2):
					//voxelTopLeftBack.set( x, y, z, 1.0 );
					//voxelDensitySphere = estimateVoxelIntersectSphere( &voxelTopLeftBack, voxelEdgeLen, &sphereCenter, sphereRadius );
					//setEightVoxelsSymmetric( x, y, z, voxelDensitySphere, sphereParam.rasterMode );
					//continue;

					// fast but unexact:
					xd = x; yd = y; zd = z; 
					voxelDistanceToCenter = sqrt( pow( (xd-xDim/2), 2 ) + pow( (yd-yDim/2), 2 ) + pow( (zd-zDim/2), 2 ) );
					//cout << "[VoxelCuboid] " << xd << ", " << z << ", " << zd-(zDim/2) << " voxelDistanceToCenter: " << voxelDistanceToCenter << " voxels." << endl;
					if( voxelDistanceToCenter <= sphereParam.sphereRadius+sqrt(3) ) {
						// inside sphere or on the surface of the sphere:
						switch( sphereParam.radiusFunc ) {
							case _SPHERE_FUNC_SOLID_:
									funcValue = 1.0;
								break;
							case _SPHERE_FUNC_GAUSS_:
									funcValue = (1/(sphereParam.gaussFuncSig*sqrt(2*M_PI))) * \
									    exp( -0.5 * pow( ( (voxelDistanceToCenter/sphereParam.sphereRadius)-sphereParam.gaussFuncMu)/sphereParam.gaussFuncSig, 2 ) );
								break;
							default:
								cerr << "[VoxelCuboid] Undefined radiusFunc: " << sphereParam.radiusFunc << "!" << endl;
                                funcValue = 1.0;    //make it neutral
                                break;
						}
						
						if( voxelDistanceToCenter > sphereParam.sphereRadius ) {
							// on the surface of the sphere - some heuristic guess for the density:
							fillLevel = ( 1.0-( voxelDistanceToCenter-sphereParam.sphereRadius ) / sqrt(3) ) / 1.150;
						} else {
							// inside the sphere, we wan't maximum density
							fillLevel = 1.0;
						}
						// estimate the final density:
						setValue = fillLevel * funcValue * sphereParam.baseDensity * UCHAR_MAX;
						// take care about UCHAR over- or underflow:
						if( setValue > 255.0 ) {
							//cout << "[VoxelCuboid] voxel > 1.0 " << setValue << endl;
							setValue = 255;
						} else if( setValue < 0 ) {
							//cout << "[VoxelCuboid] voxel < 0.0 " << setValue << endl;
							setValue = 0;
						}
						// final set the voxels density:
						voxlesFilled += setEightVoxelsSymmetric( x, y, z, setValue, sphereParam.rasterMode );
					} else {
						setEightVoxelsSymmetric( x, y, z, 0, sphereParam.rasterMode );
					}
				}
			}
		}
	} else {
		//! \todo Adapt rasterSphere for non-cubes.
		cerr << "[VoxelCuboid] rasterSphere NOT yet IMPLEMENTED for non-cubes!" << endl;
	}
	timeStop  = clock();
	cout << "[VoxelCuboid::rasterSphere] took " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	
	// store a copy (as it might be corrupted by filters)
	memcpy( cuboidArrayOriginal, cuboidArray, xDim*yDim*zDim*sizeof(unsigned char) );
	return voxlesFilled;
}

double VoxelCuboid::estimateVoxelIntersectSphere( Vector3D* voxelTopLeftBack, float voxelEdgeLen, Vector3D* sphereCenter, float sphereRadius ) {
	//! Estimate the volume of a voxel inside/along a sphere in the VoxelCuboid.
	//!
	//! Remark: Using Vector3D is quite nice for coding, but seems to slow down the generation
	//! by a factor of ~20%
	//!
	//!   F---G
	//!  /   /|     |
	//!  B--C H     E--
	//!  |  |/     /
	//!  A--D
	//!
	//! F = voxelTopLeftBack

	//! \todo FINISH (this more precise and slow) method
	//! \bug totally buggy as unfinished.

	// First check if the voxel is inside the sphere:
	float distVoxelTopLeftBack = abs3( *voxelTopLeftBack - *sphereCenter );
	if( distVoxelTopLeftBack < sphereRadius ) {
		return DBL_MAX;
	}

	return -1.0;
}

double* VoxelCuboid::integralSpheres( float* sphereRadii, //!< Array holding radii (multiple scales)
	                 uint   nrRadii      //!< Number of radii
	) {
	//! Integrates the volumes of spheres in multiple scales.
	//!
	//! Attention: sphereRadii has to be sorted by size reverse.
	//!
	//! Returns an array of size nrRadii with the percentages of the filled spheres.

	//! @todo: Refactor to use new statement with default constructor call
	//double* multiscaleVolumes = new double[nrRadii]();
	double* multiscaleVolumes = static_cast<double*>(calloc( nrRadii, sizeof( double ) ));
	float lastRadii = FLT_MAX;
	
	VoxelSphereParam sphereParam;
	sphereParam.baseDensity  = 1.0;                          // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
	sphereParam.rasterMode   = _RASTER_MODE_MULTIPLY_SCALE_; // Defines the operation with existing data in the grid.
	sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_;          // Defines the function used to estimate a voxels density as function of the radius.

	double volumeRastered;
	double volumeIdeal;

	for( uint i=0; i<nrRadii; i++ ) {
		// check if radii or sorted by size reverse (otherwise we will run into troubles).
		if( lastRadii > sphereRadii[i] ) {
			lastRadii = sphereRadii[i];
		} else {
			cerr << "[VoxelCuboid] Warning: integralSpheres requires radii to be sorted by size reversed! Last radius: " << lastRadii << " Next radius: " << sphereRadii[i] << endl;
		}
		sphereParam.sphereRadius = sphereRadii[i] * xDim/2; // Radius of the sphere in voxel.
		volumeRastered = rasterSphere( sphereParam );
		volumeIdeal    = estVolumeIdealSphere( sphereParam.sphereRadius );
		//cout << "[VoxelCuboid] integralSpheres radius:         " << sphereRadii[i] << endl;
		//cout << "[VoxelCuboid] integralSpheres volumeRastered: " << volumeRastered << endl;
		//cout << "[VoxelCuboid] integralSpheres volumeIdeal:    " << volumeIdeal << endl;
		multiscaleVolumes[i] = volumeRastered / volumeIdeal;
	}

	return multiscaleVolumes;
}

double VoxelCuboid::estVolumeIdealSphere( float voxelRadius ) {
	//! Estimates the ideal volume a sphere - in contrast to voxelized spheres.
	return (4*M_PI*pow(voxelRadius,3))/3;
}

// io-operation ---------------------------------------------------------------

int VoxelCuboid::writeDat( const string& filename ) {
	//! Writes the volume to a raw-data file.
	//!
	//! \author: Daniel Jungblut and Hubert Mara
	//!
	//! \todo: Rewrite for C++ style.
	
	FILE * file;
	if((file = fopen(filename.c_str(), "wb")) == nullptr) {
		cerr << "[VoxelCuboid] Could not open file '" << filename << "' for writing." << endl;
		return -1;
	}
 
	fwrite(&xDim, 4, 1, file);
	fwrite(&yDim, 4, 1, file);
	fwrite(&zDim, 4, 1, file);

	int dx = 1;
	int dy = 1;
	int dz = 1;

	fwrite(&dx, 4, 1, file);
	fwrite(&dy, 4, 1, file);
	fwrite(&dz, 4, 1, file);

	for( uint i=0; i<xDim*yDim*zDim; i++) {
		fwrite( &cuboidArray[i], 1, 1, file );
	}
 	fclose(file);
	cout << "[VoxelCuboid] " << filename << " written successfully." << std::endl;
	return 0;
}

// private --------------------------------------------------------------------

double VoxelCuboid::setEightVoxelsSymmetric( uint x, uint y, uint z, unsigned char value, int rasterMode ) {
	//! Sets the same value to all eight voxels arranged as a corners 
	//! of cuboid around the center of the cuboid.
	//!
	//! Returns the sum of the volume of the voxels manipulated.
	//! In case of an error a negative value is returned.
	//!
	//! This method will be used to generate symmetric structers like a sphere.
	//! see also: VoxelCuboid::rasterSphere.
	//! see also: http://webster.cs.ucr.edu/AoA/Windows/HTML/Arraysa2.html
	return setEightVoxelsSymmetric( cuboidArray, x, y, z, value, rasterMode );
}

double VoxelCuboid::setEightVoxelsSymmetric( unsigned char* someCuboid, uint x, uint y, uint z, unsigned char value, int rasterMode ) {
	//! Sets the same value to all eight voxels arranged as a corners 
	//! of cuboid around the center of the cuboid.
	//!
	//! Returns the sum of the volume of the voxels manipulated.
	//! In case of an error a negative value is returned.
	//!
	//! This method will be used to generate symmetric structers like a sphere.
	//! see also: VoxelCuboid::rasterSphere.
	//! see also: http://webster.cs.ucr.edu/AoA/Windows/HTML/Arraysa2.html
	// Index in a 3D-array: ((depthindex*col_size+colindex) * row_size + rowindex)
	int indices[8] = {
	    (static_cast<int>(z)*static_cast<int>(xDim)+static_cast<int>(y))*static_cast<int>(yDim)+static_cast<int>(x),
	    (static_cast<int>(z)*static_cast<int>(xDim)+static_cast<int>(y))*static_cast<int>(yDim)+(static_cast<int>(xDim)-static_cast<int>(x)-1),
	    (static_cast<int>(z)*static_cast<int>(xDim)+(static_cast<int>(yDim)-static_cast<int>(y)-1))*static_cast<int>(yDim)+static_cast<int>(x),
	    (static_cast<int>(z)*static_cast<int>(xDim)+(static_cast<int>(yDim)-static_cast<int>(y)-1))*static_cast<int>(yDim)+(static_cast<int>(xDim)-static_cast<int>(x)-1),
	    ((static_cast<int>(zDim)-static_cast<int>(z)-1)*static_cast<int>(xDim)+static_cast<int>(y))*static_cast<int>(yDim)+static_cast<int>(x),
	    ((static_cast<int>(zDim)-static_cast<int>(z)-1)*static_cast<int>(xDim)+static_cast<int>(y))*static_cast<int>(yDim)+(static_cast<int>(xDim)-static_cast<int>(x)-1),
	    ((static_cast<int>(zDim)-static_cast<int>(z)-1)*static_cast<int>(xDim)+(static_cast<int>(yDim)-static_cast<int>(y)-1))*static_cast<int>(yDim)+static_cast<int>(x),
	    ((static_cast<int>(zDim)-static_cast<int>(z)-1)*static_cast<int>(xDim)+(static_cast<int>(yDim)-static_cast<int>(y)-1))*static_cast<int>(yDim)+(static_cast<int>(xDim)-static_cast<int>(x)-1)
	};
	double voxelVolume = 0.0; 
	float  setValue;
	switch( rasterMode ) {
		case _RASTER_MODE_SET_:
			    for( const int index : indices ) {
					someCuboid[index] = value;
					voxelVolume += static_cast<double>(someCuboid[index]);
				}
			break;
		case _RASTER_MODE_AND_:
			    for( const int index : indices ) {
					someCuboid[index] &= value;
					voxelVolume += static_cast<double>(someCuboid[index]);
				}
			break;
		case _RASTER_MODE_OR_:
			    for( const int index : indices ) {
					someCuboid[index] |= value;
					voxelVolume += static_cast<double>(someCuboid[index]);
				}
			break;
		case _RASTER_MODE_INTERSECT_GREATER_:
			    for(const int index : indices ) {
					// estimate the final density: 
					if( value == 0 )
						continue;
					if( someCuboid[index] == 0 )
						continue;
					if( someCuboid[index] > value )
						setValue = someCuboid[index];
					else
						setValue = value;
					// take care about UCHAR over- or underflow:
					if( setValue > UCHAR_MAX ) {
						//cout << "[VoxelCuboid] voxel > 1.0 " << setValue << endl;
						setValue = UCHAR_MAX;
					} else if( setValue < 0 ) {
						//cout << "[VoxelCuboid] voxel < 0.0 " << setValue << endl;
						setValue = 0;
					}
					someCuboid[index] = setValue;
					voxelVolume += static_cast<double>(someCuboid[index]);
				}
			break;
		case _RASTER_MODE_MULTIPLY_SCALE_:
			    for( const int index : indices ) {
					setValue = ( someCuboid[index] * value ) / UCHAR_MAX;;
					if( setValue > UCHAR_MAX ) {
						//cout << "[VoxelCuboid] voxel > 1.0 " << setValue << endl;
						setValue = UCHAR_MAX;
					} else if( setValue < 0 ) {
						//cout << "[VoxelCuboid] voxel < 0.0 " << setValue << endl;
						setValue = 0;
					}
					someCuboid[index] = setValue;
					voxelVolume += static_cast<double>(someCuboid[index]);
				}
			break;
		default:
			cerr << "[VoxelCuboid] unknown raster mode: " << rasterMode << "!" << endl;
			return -1.0;
	}
	//cout << "voxelVolume: " << voxelVolume << endl;
	return voxelVolume/UCHAR_MAX;
}


