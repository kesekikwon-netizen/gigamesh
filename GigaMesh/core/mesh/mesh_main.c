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

#include "mesh.h"

#include "voxelcuboid.h"
#include "image2d.h"
#include "voxelfilter25d.h"

using namespace std;

int main(void) {  
	//! Main routine for testing various Mesh examples
	//==========================================================================

	bool readSuccess;
	if( false ) {
		Mesh someMesh( "../INV4279_SM1282-450_3.ply", readSuccess );
	}

	if( true ) {
		// test marching front
		Mesh someMesh( "../0976_REDUX.ply", readSuccess );
//		someMesh.readFile( "/export/home/hmara/3D_Keilschrift_NACHARBEITEN/W_20219,1/Warka_W20219_1.PLY" );
//		someMesh.readFile( "/export/home/hmara/3D_Keilschrift_NACHARBEITEN/W_20219,1/Warka_W20219_1_Vertex_4921_2429_Patch_r20.ply" );
//		someMesh.readFile( "../INV4279_SM1282-450_3.ply" );
//		someMesh.readFile( "../0976_REDUX.ply" );
		float radius  = 20.0;
//		int   findIdx = 4921;
		int   findIdx = 2429;
		Vertex* seedVertex = someMesh.getVertexByIdxOriginal( findIdx );
		if( seedVertex == NULL ) {
			return EXIT_FAILURE;
		}

		int timeStart, timeStop; // for performance mesurement

		set<Face*> facesInSphere;
		timeStart = clock();
		someMesh.fetchSphereMarchingDualFront( seedVertex, &facesInSphere, radius );
		timeStop = clock();
		cout << "[Mesh] fetchSphereVolume - fetchSphereMarchingDualFront: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		cout << "Faces in sphere: " << facesInSphere.size() << endl;

		set<Face*> facesInSphereOLD;
		timeStart = clock();
		someMesh.fetchSphereMarching( seedVertex, &facesInSphereOLD, radius );
		timeStop = clock();
		cout << "[Mesh] fetchSphereVolume - fetchSphereMarching: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		cout << "Faces in sphere: " << facesInSphereOLD.size() << endl;

		vector<Face*> facesInSphereALT;
		// Allocate the bit arrays for vertices:
		unsigned long vertNrLongs = someMesh.getVertexNr() / ( 8*sizeof( unsigned long ) ) + 1;
		unsigned long* vertBitArrayVisited = (unsigned long*) calloc( vertNrLongs, sizeof( unsigned long ) );
		//cout << "[Mesh::fetchSphereBitArray]: vertNrLongs: " << vertNrLongs << " equals " << vertNrLongs*sizeof( unsigned long ) << " bytes." << endl;
		// Allocate the bit arrays for faces:
		unsigned long faceNrLongs = someMesh.getFaceNr() / ( 8*sizeof( unsigned long ) ) + 1;
		unsigned long* faceBitArrayVisited = (unsigned long*) calloc( faceNrLongs, sizeof( unsigned long ) );
		//cout << "[Mesh::fetchSphereBitArray]: faceNrLongs: " << faceNrLongs << " equals " << faceNrLongs*sizeof( unsigned long ) << " bytes." << endl;
		timeStart = clock();
		someMesh.fetchSphereBitArray( seedVertex, &facesInSphereALT, radius, vertNrLongs, vertBitArrayVisited, faceNrLongs, faceBitArrayVisited );
		timeStop = clock();
		cout << "[Mesh] fetchSphereVolume - fetchSphereBitArray: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		cout << "Faces in sphere: " << facesInSphereALT.size() << endl;

		vector<Face*> facesInSphereALT_R1;
		timeStart = clock();
		someMesh.fetchSphereBitArray1R( seedVertex, facesInSphereALT_R1, radius,
		                                vertNrLongs, vertBitArrayVisited,
		                                faceNrLongs, faceBitArrayVisited, false );
		timeStop = clock();
		free( vertBitArrayVisited );
		free( faceBitArrayVisited );
		cout << "[Mesh] fetchSphereVolume - fetchSphereBitArray1R: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		cout << "Faces in sphere: " << facesInSphereALT_R1.size() << endl;


		uint    xyzDim = 512*8;
		uint    rasterSize = xyzDim * xyzDim;
		double* rasterArray = new double[rasterSize];
		timeStart = clock();
		someMesh.fetchSphereCubeVolume25D( seedVertex, &facesInSphere, radius, rasterArray, xyzDim );
		timeStop = clock();
		cout << "[Mesh] fetchSphereCubeVolume25D (set): " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

		timeStart = clock();
		someMesh.fetchSphereCubeVolume25D( seedVertex, &facesInSphereALT, radius, rasterArray, xyzDim );
		timeStop = clock();
		cout << "[Mesh] fetchSphereCubeVolume25D (vector): " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

		timeStart = clock();
		someMesh.fetchSphereCubeVolume25D( seedVertex, &facesInSphereALT_R1, radius, rasterArray, xyzDim );
		timeStop = clock();
		cout << "[Mesh] fetchSphereCubeVolume25D+R1 (vector): " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

		int radiiCount = 4;
		unsigned int multiscaleRadiiSize = pow( 2.0, radiiCount );
		double*      multiscaleRadii     = new double[multiscaleRadiiSize];
		for( uint i=0; i<multiscaleRadiiSize; i++ ) {
			multiscaleRadii[i] = 1.0 - (double)i/(double)multiscaleRadiiSize;
		}

		voxelFilter2DElements* sparseFilters;
		timeStart = clock();
		double** voxelFilters2D = generateVoxelFilters2D( multiscaleRadiiSize, multiscaleRadii, xyzDim, &sparseFilters );
		timeStop = clock();
		cout << "[Mesh] generateVoxelFilters2D: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		double* absolutRadii = new double[multiscaleRadiiSize];
		double* resultArr = new double[multiscaleRadiiSize];
		//applyVoxelFilters2D( double* featureArray, double* rasterArray, voxelFilter2DElements** sparseFilters, uint multiscaleRadiiSize, uint xyzDim );
		timeStart = clock();
		applyVoxelFilters2D( resultArr, rasterArray, &sparseFilters, multiscaleRadiiSize, xyzDim );
		timeStop = clock();
		cout << "[Mesh] applyVoxelFilters2D: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		for( uint i=0; i<multiscaleRadiiSize; i++ ) {
			cout << resultArr[i] << endl;
			absolutRadii[i] = radius * ((double)i+1.0)/(double)multiscaleRadiiSize;
		}

		//Vector3D seedPosition = seedVertex->getCenterOfGravity();
		timeStart = clock();
		someMesh.fetchSphereArea( seedVertex, &facesInSphereALT, multiscaleRadiiSize, absolutRadii, resultArr );
		timeStop = clock();
		cout << "[Mesh] fetchSphereArea: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		timeStart = clock();
		someMesh.fetchSphereArea( seedVertex, &facesInSphereALT_R1, multiscaleRadiiSize, absolutRadii, resultArr );
		timeStop = clock();
		cout << "[Mesh] fetchSphereArea+R1: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

		//---------------------------------------------------------------------------------------------------------------
		// geodesic distance estimation will be slower than the ones tested above, but only by a small, constant factor:
		timeStart = clock();
		someMesh.estGeodesicPatchFuncVal( seedVertex, radius, false ); // false = no weight by function value.
		timeStop = clock();
		cout << "[Mesh] estGeodesicPatch (vertex): " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
	}

	if( false ) {
		// test read-write of .obj and ply
		Mesh someMesh( "../0976_REDUX.obj", readSuccess );
		//! \todo Binary and ASCII are no option anymore - reintegrate!
		someMesh.writeFile( string( "test_binary.ply" )/*, _WRITE_BINARY_*/ );
		someMesh.writeFile( string( "test_ascii.ply" )/*,  _WRITE_ASCII_*/ );
		Mesh someOtherMesh1( string( "test_binary.ply" ), readSuccess );
		Mesh someOtherMesh2( string( "test_ascii.ply" ), readSuccess );
	}

	if( false ) {
		uint xDim = 256;
		uint yDim = 256;
		uint zDim = 256;
		Image2D someImage;
		//string filename( "singletriangle" ); string extension( "obj" );
		//string filename( "twotriangles" ); string extension( "obj" );
		string filename( "somemeshpartorientated" ); string extension( "ply" );
		Mesh someMesh( filename + "." + extension, readSuccess );
		// deprecated - use transform: someMesh.scale( 0.5 );
		// face rendering using z:
		double* rasterArray                  = someMesh.rasterViewFromZ( xDim, yDim );
		uint8_t*   rasterGrayHeightMap          = someMesh.rasterToHeightMap( rasterArray, xDim, yDim, zDim, false );
		uint8_t*   rasterGrayHeightMapEqualized = someMesh.rasterToHeightMap( rasterArray, xDim, yDim, zDim, true );
		someImage.writeTIFF( filename + "_hmap",       xDim, yDim, rasterGrayHeightMap, false );
		someImage.writeTIFF( filename + "_hmap_equal", xDim, yDim, rasterGrayHeightMapEqualized, false );

		//char* imageStack = someMesh.rasterToVolume( rasterArray, xDim, yDim, zDim );
		//someImage.writeTIFFStack( filename, xDim, yDim, zDim, imageStack, false );

		VoxelCuboid someCube( xDim, yDim, zDim );
		someCube.rasterZBuffer( rasterArray );
		VoxelSphereParam sphereParam;
		sphereParam.sphereRadius = 126;                 // Radius of the sphere in voxel.
		sphereParam.baseDensity  = 1.5;                 // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
		sphereParam.rasterMode   = _RASTER_MODE_AND_;   // Defines the operation with existing data in the grid.
		//sphereParam.radiusFunc   = _SPHERE_FUNC_GAUSS_; // Defines the function used to estimate a voxels density as function of the radius.
		sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_; // Defines the function used to estimate a voxels density as function of the radius.
		sphereParam.gaussFuncSig = 0.4;                 // Parameter sigma of the gaussian function (use 0.4 for a start as it will scale the density to ~maximum at the center).
		sphereParam.gaussFuncMu  = 0.0;                 // Parameter mu (µ) of the gaussian function (use 0.0 for a start as it will scale the density to ~0 at the spheres surface).
		double cubeVolume = someCube.rasterSphere( sphereParam );
		//someCube.writeDat( filename + "_volume.dat" );
		someImage.writeTIFFStack( filename, xDim, yDim, zDim, someCube.getCuboidArrayRef(), false );
		cout << "cubeVolume: " << cubeVolume << endl; 

		someCube.writeDat( "mond.dat" );
	}

	if( false ) {
		// volume in different scales:
		int nrFiles = 1;
		string* filenames = new string[nrFiles]();
		filenames[0] = "meshpart_vertex_1";
/*
		filenames[0] = "somePyramid1";
		filenames[1] = "somePyramid2";
		filenames[2] = "somePyramid3";
		filenames[3] = "somePyramid4";
*/
		double** featureVecs = new double*[nrFiles]();
		int timeStart, timeStop; // for performance mesurement
		for( int i=0; i<nrFiles; i++ ) {
			uint xDim = 256;
			uint yDim = xDim;
			uint zDim = xDim;
			//someMesh.setSilent( true );
			string filename( filenames[i] ); string extension( "obj" );
			Mesh someMesh( filename + "." + extension, readSuccess );
			// deprecated - use transform: someMesh.scale( 0.5 );
			double* rasterArray = someMesh.rasterViewFromZ( xDim, yDim );
			uint8_t*   rasterGrayHeightMapEqualized = someMesh.rasterToHeightMap( rasterArray, xDim, yDim, zDim, true );
			Image2D someImage;
			someImage.writeTIFF( filename + "_hmap_equal", xDim, yDim, rasterGrayHeightMapEqualized, false );
			if( true ) {
				// visualization - volume in different scales:
				VoxelCuboid someCube( xDim, yDim, zDim );
				someCube.rasterZBuffer( rasterArray );

				double cubeVolume;
				VoxelSphereParam sphereParam;
				sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_; // Defines the function used to estimate a voxels density as function of the radius.

				sphereParam.sphereRadius = xDim/2;              // Radius of the sphere in voxel.
				sphereParam.baseDensity  = 0.1;                 // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
				sphereParam.rasterMode   = _RASTER_MODE_AND_;   // Defines the operation with existing data in the grid.
				cubeVolume = someCube.rasterSphere( sphereParam );

				sphereParam.rasterMode   = _RASTER_MODE_INTERSECT_GREATER_;   // Defines the operation with existing data in the grid.
				for( float i=0.2; i<=1.0; i+=0.1 ) {
					sphereParam.sphereRadius = (xDim/2)*(1.0-i+0.1); // Radius of the sphere in voxel.
					sphereParam.baseDensity  = i;                    // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
					cubeVolume = someCube.rasterSphere( sphereParam );
				}
				someImage.writeTIFFStack( filename, xDim, yDim, zDim, someCube.getCuboidArrayRef(), false );
				someCube.writeDat( "spheremultiscale.dat" );
			}

			if( true ) {
				// estimation - volume in different scales:
				VoxelCuboid someCube( xDim, yDim, zDim );
				timeStart = clock();
				someCube.rasterZBuffer( rasterArray );
				timeStop  = clock();
				cout << "[Mesh_Main] rasterZBuffer estimation: " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;

				float multiscaleRadii[10] = { 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1 };
				timeStart = clock();
				featureVecs[i] = someCube.integralSpheres( reinterpret_cast<float*>(&multiscaleRadii), 10U );
				timeStop  = clock();
				cout << "[Mesh_Main] integralSpheres estimation: " << static_cast<float>( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
			}
		} // for
		fstream filestr;
		filestr.open( "featureVecs.mat", fstream::out );
		for( int i=0; i<nrFiles; i++ ) {
				filestr << "featureVec{" << i << "} = [ ";
				cout << "featureVec{" << i << "} = [ ";
				for( uint j=0; j<10; j++ ) {
					filestr << featureVecs[i][j] << " ";
					cout << featureVecs[i][j] << " ";
				}
				filestr << "];" << endl;
				cout << "];" << endl;
		} // for

		delete[] filenames;

		for ( size_t featureVectorCount = 0; featureVectorCount < nrFiles;
		                                            featureVectorCount++ )
		{
			free(featureVecs[featureVectorCount]);
		}

		delete[] featureVecs;

		filestr.close();
	}

	if( false ) {
		// compare scales:
		int timeStart, timeStop; // for performance mesurement
		string filename( "somemeshpartorientated" ); string extension( "ply" );
		Mesh someMesh( filename + "." + extension, readSuccess );
		{
			timeStart = clock();
			uint xDim = 512;
			uint yDim = 512;
			uint zDim = 512;
			double* rasterArray = someMesh.rasterViewFromZ( xDim, yDim );
			VoxelCuboid someCube( xDim, yDim, zDim );
			someCube.rasterZBuffer( rasterArray );
			VoxelSphereParam sphereParam;
			sphereParam.sphereRadius = (48/2);              // Radius of the sphere in voxel.
			sphereParam.baseDensity  = 1.0;                 // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
			sphereParam.rasterMode   = _RASTER_MODE_AND_;   // Defines the operation with existing data in the grid.
			sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_; // Defines the function used to estimate a voxels density as function of the radius.
			double cubeVolume = someCube.rasterSphere( sphereParam );
			cout << "[Mesh_Main] cubeVolume: " << int( cubeVolume ) << endl; 
			timeStop  = clock();
			cout << "[Mesh_Main] Volume estimation: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		}
		// deprecated - use transform: someMesh.scale( 0.5 );
		{
			timeStart = clock();
			uint xDim = 256;
			uint yDim = 256;
			uint zDim = 256;
			double* rasterArray = someMesh.rasterViewFromZ( xDim, yDim );
			VoxelCuboid someCube( xDim, yDim, zDim );
			someCube.rasterZBuffer( rasterArray );
			VoxelSphereParam sphereParam;
			sphereParam.sphereRadius = (24/2);              // Radius of the sphere in voxel.
			sphereParam.baseDensity  = 1.0;                 // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
			sphereParam.rasterMode   = _RASTER_MODE_AND_;   // Defines the operation with existing data in the grid.
			sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_; // Defines the function used to estimate a voxels density as function of the radius.
			double cubeVolume = someCube.rasterSphere( sphereParam );
			cout << "[Mesh_Main] cubeVolume: " << int( cubeVolume )*8 << endl; 
			timeStop  = clock();
			cout << "[Mesh_Main] Volume estimation: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		}
		// deprecated - use transform: someMesh.scale( 0.5 );
		{
			timeStart = clock();
			uint xDim = 128;
			uint yDim = 128;
			uint zDim = 128;
			double* rasterArray = someMesh.rasterViewFromZ( xDim, yDim );
			VoxelCuboid someCube( xDim, yDim, zDim );
			someCube.rasterZBuffer( rasterArray );
			VoxelSphereParam sphereParam;
			sphereParam.sphereRadius = (12/2);              // Radius of the sphere in voxel.
			sphereParam.baseDensity  = 1.0;                 // Density offset multiplied with whatever density function used (use 1.0 for full voxels).
			sphereParam.rasterMode   = _RASTER_MODE_AND_;   // Defines the operation with existing data in the grid.
			sphereParam.radiusFunc   = _SPHERE_FUNC_SOLID_; // Defines the function used to estimate a voxels density as function of the radius.
			double cubeVolume = someCube.rasterSphere( sphereParam );
			cout << "[Mesh_Main] cubeVolume: " << int( cubeVolume )*64 << endl; 
			timeStop  = clock();
			cout << "[Mesh_Main] Volume estimation: " << (float)( timeStop - timeStart ) / CLOCKS_PER_SEC << " seconds."  << endl;
		}
		// Results for 512, 256, 128 ^ 3 and a rather small sphere (radius ~10% of Dim):
		// [Mesh_Main] cubeVolume: 36592
		// [Mesh_Main] Volume estimation: 4.82 seconds.
		// [Mesh_Main] cubeVolume: 37688
		// [Mesh_Main] Volume estimation: 0.61 seconds.
		// [Mesh_Main] cubeVolume: 37632
		// [Mesh_Main] Volume estimation: 0.07 seconds.
	}
}

