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

#include <stdio.h>
#include <stdlib.h> // calloc
#include <string>

#include "voxelcuboid.h"
#include "vector3d.h"
#include "image2d.h"

using namespace std;

int main(void) {  
	//! Main routine for testing various VoxelCuboid examples
	//==========================================================================

	if( false ) {
		uint xDim = 128;
		uint yDim = 128;
		uint zDim = 128;
		VoxelCuboid someCube( xDim, yDim, zDim );
		double volumeR = someCube.rasterSetTo( 127 );
		double volume  = someCube.getVolume();
		cout << "Volume (es): " << volumeR << endl;
		cout << "Volume (is): " << volume << endl;
		cout << "Volume (ep): " << xDim*yDim*zDim*127/255 << endl;
		Image2D someImage;
		someImage.writeTIFFStack( "sphere", xDim, yDim, zDim, someCube.getCuboidArrayRef(), false );
	}

	if( false ) {
		// heuristic error estimation:
		uint xDim = 512;
		uint yDim = 512;
		uint zDim = 512;
		VoxelSphereParam sphereParam;
		sphereParam.baseDensity  = 1.0F;                 // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
		sphereParam.rasterMode   = _RASTER_MODE_OR_;    // Defines the operation with existing data in the grid.
		sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_; // Defines the function used to estimate a voxels density as function of the radius.
		float sphereRadiusMin    = 10.0F;
		float sphereRadiusSteps  = 25.0F;
		int   sphereRadiusStepNr = 10.0F;
		float* errorDiscrete = new float[sphereRadiusStepNr];
		for( int i=0; i<sphereRadiusStepNr; i++ ) {
			VoxelCuboid someCube( xDim, yDim, zDim );
			sphereParam.sphereRadius =  sphereRadiusMin + sphereRadiusSteps * i; // Radius of the sphere in voxel.
			float sphereVolume = someCube.rasterSphere( sphereParam );
			float volumeIdeal = (4*M_PI*pow(sphereParam.sphereRadius, 3.0F))/3.0F;
			cout << "Sphere radius:        " << sphereParam.sphereRadius << " Voxels." << endl;
			cout << "Sphere volume filled: " << sphereVolume << " Voxels." << endl;
			cout << "Sphere volume ideal:  " << volumeIdeal  << " Voxels." << endl;
			cout << "=> Sphere error:      " << 100*(sphereVolume-volumeIdeal)/volumeIdeal << " %." << endl;
			errorDiscrete[i] = 100*(sphereVolume-volumeIdeal)/volumeIdeal;
		}
		cout << "errFunc = [ ";
		for( int i=0; i<sphereRadiusStepNr-1; i++ ) {
			cout << errorDiscrete[i] << ", ";
		}
		cout << errorDiscrete[sphereRadiusStepNr-1] << " ];" << endl;

		delete[] errorDiscrete;
	}



	if( false ) {
		uint xDim = 128;
		uint yDim = 128;
		uint zDim = 128;
		VoxelCuboid someCube( xDim, yDim, zDim );
		VoxelSphereParam sphereParam;
		sphereParam.sphereRadius =  60;                 // Radius of the sphere in voxel.
		sphereParam.baseDensity  = 1.5;                 // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
		sphereParam.rasterMode   = _RASTER_MODE_OR_;    // Defines the operation with existing data in the grid.
		sphereParam.radiusFunc   = _SPHERE_FUNC_GAUSS_; // Defines the function used to estimate a voxels density as function of the radius.
		//sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_; // Defines the function used to estimate a voxels density as function of the radius.
		sphereParam.gaussFuncSig = 0.4;                 // Parameter sigma of the gaussian function (use 0.4 for a start as it will scale the density to ~maximum at the center).
		sphereParam.gaussFuncMu  = 0.0;                 // Parameter mu (µ) of the gaussian function (use 0.0 for a start as it will scale the density to ~0 at the spheres surface).
		double sphereVolume = someCube.rasterSphere( sphereParam );
		float volumeIdeal = (4*M_PI*pow((float)60,(float)3))/3;
		cout << "Sphere volume real:   " << sphereVolume << " Voxels." << endl;
		cout << "Sphere volume ideal:  " << volumeIdeal  << " Voxels." << endl;
		cout << "Sphere volume err:    " << 100*(sphereVolume-volumeIdeal)/volumeIdeal  << " %." << endl;
		Image2D someImage;
		someImage.writeTIFFStack( "sphere_128x3_various", xDim, yDim, zDim, someCube.getCuboidArrayRef(), false );
	}

	if( false ) {
		uint xDim = 512;
		uint yDim = 512;
		uint zDim = 512;
		VoxelCuboid someCube( xDim, yDim, zDim );
		//someCube.rasterSetTo( UCHAR_MAX );

		VoxelSphereParam sphereParam;
		sphereParam.sphereRadius = xDim/2;              // Radius of the sphere in voxel.
		sphereParam.baseDensity  = 1.0;                 // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
		sphereParam.rasterMode   = _RASTER_MODE_OR_;    // Defines the operation with existing data in the grid.
		//sphereParam.radiusFunc   = _SPHERE_FUNC_GAUSS_; // Defines the function used to estimate a voxels density as function of the radius.
		sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_; // Defines the function used to estimate a voxels density as function of the radius.
		sphereParam.gaussFuncSig = 0.4;                 // Parameter sigma of the gaussian function (use 0.4 for a start as it will scale the density to ~maximum at the center).
		sphereParam.gaussFuncMu  = 0.0;                 // Parameter mu (µ) of the gaussian function (use 0.0 for a start as it will scale the density to ~0 at the spheres surface).
		//double sphereVolume = someCube.rasterSphere( sphereParam );
		someCube.writeDat( "sphere.dat" );

		Image2D someImage;
		float multiscaleRadii[10] = { 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1 };
		someImage.writeTIFFStack( "sphere_128x3_various_start", xDim, yDim, zDim, someCube.getCuboidArrayRef(), false );
		double* multiScaleVolumes = someCube.integralSpheres( (float*)&multiscaleRadii, (uint) 10 );
		someImage.writeTIFFStack( "sphere_128x3_various_end", xDim, yDim, zDim, someCube.getCuboidArrayRef(), false );
		cout << "featureVec = [ ";
		for( uint i=0; i<10; i++ ) {
			cout << multiScaleVolumes[i] << " ";
		}
		cout << "];" << endl;
	}

	if( true ) {
		uint xyzDim = 256;
		double multiscaleRadii[10] = { 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1 };
		double featureVector[10];
		VoxelCuboid someCube( xyzDim, 10, multiscaleRadii );
		someCube.rasterSetTo( 255 );
		// will return the same value only, but good for performance measurment:
		someCube.applyFilters( featureVector );
	}

	return 0;
}
