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

#include <GigaMesh/mesh/voxelfilter25d.h>

using namespace std;

double* generateVoxelFilter2D( double radiusRel,                   //!< Radius relative to xyzDim.
	                       uint   xyzDim,                      //!< Size in Voxels of the cube
	                       voxelFilter2DElements* sparseFilter //!< Number and indices of the non-nan elements.
	) {
	//! Generate a 2D (or 2.5D) filter mask for given a radius and voxelsize.
	//! Returns also references to the actual elements of the filter (sparse matrix concept).

	double* voxelFilter2D = static_cast<double*>(calloc( xyzDim * xyzDim, sizeof(double) ));

	double radius = (xyzDim-2) * radiusRel / 2.0;
	double xCoord;
	double yCoord;
	double toBeSqrt;
	double pixVal;
	double xCoordSub;
	double yCoordSub;
	#if _SUB_PIXEL_COUNT_ == 1
		double subPixelCoordsX[_SUB_PIXEL_COUNT_] = { 0.00 };
		double subPixelCoordsY[_SUB_PIXEL_COUNT_] = { 0.00 };
	#endif
	#if _SUB_PIXEL_COUNT_ == 9
		double subPixelCoordsX[_SUB_PIXEL_COUNT_] = { -0.25, +0.00, +0.25, \
		                                              -0.25, +0.00, +0.25, \
		                                              -0.25, +0.00, +0.25 };
		double subPixelCoordsY[_SUB_PIXEL_COUNT_] = { -0.25, -0.25, -0.25, \
		                                              +0.00, +0.00, +0.00, \
		                                              +0.25, +0.25, +0.25 };
	#endif
	#if _SUB_PIXEL_COUNT_ == 25
		double subPixelCoordsX[_SUB_PIXEL_COUNT_] = { -0.50, -0.25, +0.00, +0.25, +0.50, \
		                                              -0.50, -0.25, +0.00, +0.25, +0.50, \
		                                              -0.50, -0.25, +0.00, +0.25, +0.50, \
		                                              -0.50, -0.25, +0.00, +0.25, +0.50, \
		                                              -0.50, -0.25, +0.00, +0.25, +0.50 };
		double subPixelCoordsY[_SUB_PIXEL_COUNT_] = { -0.50, -0.50, -0.50, -0.50, -0.50, \
		                                              -0.25, -0.25, -0.25, -0.25, -0.25, \
		                                              +0.00, +0.00, +0.00, +0.00, +0.00, \
		                                              +0.25, +0.25, +0.25, +0.25, +0.25, \
		                                              +0.50, +0.50, +0.50, +0.50, +0.50 };
	#endif

	// definitly to large - resize at the end of the initalization.
	int*    elementIndices = static_cast<int*>(calloc( xyzDim*xyzDim, sizeof(int) ));
	double* elementValues  = static_cast<double*>(calloc( xyzDim*xyzDim, sizeof(double) ));
	int sparseIdxCtr = 0;
	sparseFilter->nrElements = 0;

	for( uint x=0; x<xyzDim; x++ ) {
		xCoord = x - ( xyzDim / 2.0 ) + 0.5;
		//cout << "pixVal: (" << radius << "): ";
		for( uint y=0; y<xyzDim; y++ ) {
			yCoord = y - ( xyzDim / 2.0 ) + 0.5;
			pixVal = numeric_limits<float>::quiet_NaN();
			for( uint pixIdx=0; pixIdx<_SUB_PIXEL_COUNT_; pixIdx++ ) {
				xCoordSub = xCoord + subPixelCoordsX[pixIdx];
				yCoordSub = yCoord + subPixelCoordsY[pixIdx];
				toBeSqrt = (radius*radius) - (xCoordSub*xCoordSub) - (yCoordSub*yCoordSub);
				if( toBeSqrt < 0.0 ) {
					continue;
				}
				// as we can not add to not-a-number, we have to initalize:
				if( isnan( pixVal ) ) {
					pixVal = 0.0;
				}
				pixVal += 2.0 * sqrt( toBeSqrt ) / _SUB_PIXEL_COUNT_;
			}
			if( !isnan( pixVal ) ) {
				elementIndices[sparseFilter->nrElements] = sparseIdxCtr;
				elementValues[sparseFilter->nrElements] = pixVal;
				sparseFilter->nrElements++;
			}
			//cout << pixVal << " ";
			voxelFilter2D[x*xyzDim+y] = pixVal;
			//cout << "[generateVoxelFilter2D] " << xCoord << " " << yCoord << " " << voxelFilter2D[x*xyzDim+y] << endl;
			sparseIdxCtr++;
		}
		//cout << endl;
	}
	// init with proper size ...
	sparseFilter->elementIndices = static_cast<int*>(calloc( sparseFilter->nrElements, sizeof(int) ));
	sparseFilter->elementValues  = static_cast<double*>(calloc( sparseFilter->nrElements, sizeof(double) ));
	// ... and fill with values ...
	memcpy( sparseFilter->elementIndices, elementIndices, sparseFilter->nrElements*sizeof(int) );
	memcpy( sparseFilter->elementValues,  elementValues,  sparseFilter->nrElements*sizeof(double) );
	// ... free temporary arrays:
	free( elementIndices );
	free( elementValues );

	return voxelFilter2D;
}

double** generateVoxelFilters2D( uint multiscaleRadiiSize, double* multiscaleRadii, uint xyzDim, voxelFilter2DElements** sparseFilters ) {
	//! Generate a 2D (or 2.5D) filter masks for the given radii and voxelsize.
	double** voxelFilters2D = static_cast<double**>(calloc( multiscaleRadiiSize, sizeof(double*) ));
	*sparseFilters = static_cast<voxelFilter2DElements*>(calloc( multiscaleRadiiSize, sizeof(voxelFilter2DElements) ));
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		voxelFilters2D[i] = generateVoxelFilter2D( multiscaleRadii[i], xyzDim, &((*sparseFilters)[i]) );
	}
	return voxelFilters2D;
}

bool applyVoxelFilter2D( double* featureElement, double* rasterArray, voxelFilter2DElements* sparseFilter, uint xyzDim ) {
	//! Estimates the integral for one voxel filter. Along border the filter acts anisotrop.
	//!
	//! \return true when "dirty", which typically means that we are on the border of the Mesh.

	bool   dirty = false;
	int    rasterIdx;
	double filterElement;
	double rasterElement;
	double filtSum = 0.0; // to normalize with the filter volume.
	// init elemnent:
	*featureElement = 0.0;
	for( int i=0; i<sparseFilter->nrElements; i++ ) {
		rasterIdx = sparseFilter->elementIndices[i];
		if( isnan( rasterArray[rasterIdx] ) ) {
			// outside the mesh => nothing to add. and es we do not increase filtSum we achieve kind of an insotropy.
			dirty = true;
			continue;
		}
		filterElement = sparseFilter->elementValues[i];
		// we have to shift the raster Values by the half height of the sphere.
		// while the voxelFilter2D has the full height of the sphere.
		rasterElement = rasterArray[rasterIdx] + ( filterElement / 2.0 );// + ((double) xyzDim) / 2.0;
		//cout << "rasterElement[" << i << "]: " << rasterElement << " = " << rasterArray[i] << " + ( " << voxelFilter2D[i] << " / 2.0 ) " << endl;
		filtSum += filterElement;
		if( rasterElement <= 0.0 ) {
			// nothing to add.
			continue;
		}
		// the integral for each pixel (=voxel stack) is the lower value:
		if( rasterElement < filterElement ) {
			*featureElement += rasterElement;
		} else {
			*featureElement += filterElement;
		}
	}
	//cout << "filtSum: " << (*featureElement) << "/" << filtSum << endl;
	(*featureElement) *= (2.0/filtSum);
	(*featureElement) -= 1.0;
	return dirty;
}

void applyVoxelFilters2D( double* featureArray, double* rasterArray, voxelFilter2DElements** sparseFilters, uint multiscaleRadiiSize, uint xyzDim ) {
	//! Estimates the integral (sum) of the voxel filters using the sparse matrices.
	//!
	//! featureArray has to be initalized with Zeros ... unless a - probably random - set-off ist wanted.
	//!
	//! Remark: featureArray and sparseFilters has to be of length multiscaleRadiiSize
	// Sanity check
	if( (*sparseFilters) == nullptr ) {
		cerr << "[applyVoxelFilters2D] NULL pointer for sparseFilters given!" << endl;
		return;
	}
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		applyVoxelFilter2D( &featureArray[i], rasterArray, &((*sparseFilters)[i]), xyzDim );
	}
}


double sumVoxelFilter2D( double* voxelFilter2D, uint xyzDim ) {
	//! Integrates a filter mask (or an area to be filtered).
	double filterSum = 0.0;
	for( uint i=0; i<xyzDim*xyzDim; i++ ) {
		if( isnan( voxelFilter2D[i] ) ) {
			continue;
		}
		filterSum += voxelFilter2D[i];
	}
	return filterSum;
}

bool applyVoxelFilter2D( double* featureElement, double* rasterArray, double* voxelFilter2D, uint xyzDim ) {
	//! Estimates the integral for one voxel filter. Along border the filter acts anisotrop.
	//!
	//! \return true when "dirty", which typically means that we are on the border of the Mesh.

	bool   dirty = false;
	double rasterElement;
	double filtSum = 0.0; // to normalize with the filter volume.
	// init elemnent:
	*featureElement = 0.0;
	for( uint i=0; i<xyzDim*xyzDim; i++ ) {
		if( isnan( voxelFilter2D[i] ) ) {
			// outside the filter-range => nothing to add.
			continue;
		}
		if( isnan( rasterArray[i] ) ) {
			// outside the mesh => nothing to add. and es we do not increase filtSum we achieve kind of an insotropy.
			dirty = true;
			continue;
		}
		// we have to shift the raster Values by the half height of the sphere.
		// while the voxelFilter2D has the full height of the sphere.
		rasterElement = rasterArray[i] + ( voxelFilter2D[i] / 2.0 );// + ((double) xyzDim) / 2.0;
		//cout << "rasterElement[" << i << "]: " << rasterElement << " = " << rasterArray[i] << " + ( " << voxelFilter2D[i] << " / 2.0 ) " << endl;
		filtSum += voxelFilter2D[i];
		if( rasterElement <= 0.0 ) {
			// nothing to add.
			continue;
		}
		// the integral for each pixel (=voxel stack) is the lower value:
		if( rasterElement < voxelFilter2D[i] ) {
			*featureElement += rasterElement;
		} else {
			*featureElement += voxelFilter2D[i];
		}
	}
	//cout << "filtSum: " << (*featureElement) << "/" << filtSum << endl;
	(*featureElement) *= (2.0/filtSum);
	(*featureElement) -= 1.0;
	return dirty;
}

void applyVoxelFilters2D( double* featureArray, double* rasterArray, uint multiscaleRadiiSize, double** voxelFilters2D, uint xyzDim ) {
	//! Estimates the integral (sum) of the voxel filters.
	//!
	//! featureArray has to be initalized with Zeros ... unless a - probably random - set-off ist wanted.
	//!
	//! Remark: featureArray has to be of length multiscaleRadiiSize
	//! and rasterArray of the same size as each of the voxelFilters2D.
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		applyVoxelFilter2D( &featureArray[i], rasterArray, voxelFilters2D[i], xyzDim );
	}
}

double* applyVoxelFilter2DImage( double* rasterArray, double* voxelFilter2D, uint xyzDim ) {
	//! The same as applyVoxelFilter2D() - but returns the raster array (=image).
	//!
	//! \todo return dirty and filtSum - otherwise results along borders will not match applyVoxelFilter2D().
	double* featureArray = static_cast<double*>(calloc( xyzDim*xyzDim, sizeof(double) ));
	double  rasterElement = 0.0;
	double  featureElement = 0.0;
	double  filtSum = 0.0; // to normalize with the filter volume.
	for( uint i=0; i<xyzDim*xyzDim; i++ ) {
		if( isnan( voxelFilter2D[i] ) ) {
			// outside the filter-range => nothing to add.
			featureArray[i] = 0.0;
			continue;
		}
		if( isnan( rasterArray[i] ) ) {
			// outside the mesh => nothing to add. and es we do not increase filtSum we achieve kind of an insotropy.
			continue;
		}
		// we have to shift the raster Values by the half height of the sphere.
		// while the voxelFilter2D has the full height of the sphere.
		rasterElement = rasterArray[i] + ( voxelFilter2D[i] / 2.0 );// + ((double) xyzDim) / 2.0;
		//cout << "rasterElement: " << rasterElement << endl;
		filtSum += voxelFilter2D[i];
		if( rasterElement <= 0.0 ) {
			// nothing to add.
			featureArray[i] = 0.0;
			continue;
		}
		// the integral for each pixel (=voxel stack) is the lower value:
		if( rasterElement < voxelFilter2D[i] ) {
			featureArray[i] =  rasterElement;
			featureElement  += rasterElement;
		} else {
			featureArray[i] =  voxelFilter2D[i];
			featureElement  += voxelFilter2D[i];
		}
	}
	//cout << "[applyVoxelFilter2DImage] " << (*featureElement) << "/" << filtSum << " = " << (featureElement/filtSum) << endl;
	return featureArray;
}

double** applyVoxelFilters2DImage( double* rasterArray, uint multiscaleRadiiSize, double** voxelFilters2D, uint xyzDim ) {
	//! The same as applyVoxelFilters2DImage() - but returns the raster arrays (=images).
	//!
	//! Do not forget to free the returned array* and arrays!
	double** featureArrays = static_cast<double**>(calloc( multiscaleRadiiSize, sizeof( double* ) ));
	for( uint i=0; i<multiscaleRadiiSize; i++ ) {
		featureArrays[i] = applyVoxelFilter2DImage( rasterArray, voxelFilters2D[i], xyzDim );
	}
	return featureArrays;
}
