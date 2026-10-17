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

#ifndef VOXELFILTER25D_H
#define VOXELFILTER25D_H

#include <iostream> // cout, endl
#include <limits>   // numeric_limits

#include <cstdlib> // uint, calloc
#include <cmath>   // sqrt
#include <cstring> // memcpy, strncmp

//! \todo Remove using declaration from header
using uint = unsigned int;

//#define _SUB_PIXEL_COUNT_ 1
//#define _SUB_PIXEL_COUNT_ 9
#define _SUB_PIXEL_COUNT_ 25

struct voxelFilter2DElements {
	int     nrElements;     //!< Number of the non-nan elements.
	int*    elementIndices; //!< Indices of the non-nan elements - has length nrFilterElementsSet.
	double* elementValues;  //!< Values of the non-nan elements - has length nrFilterElementsSet.
};

double*  generateVoxelFilter2D( double radiusRel, uint xyzDim, voxelFilter2DElements* sparseFilter );
double** generateVoxelFilters2D( uint multiscaleRadiiSize, double* multiscaleRadii, uint xyzDim, voxelFilter2DElements** sparseFilters );

bool     applyVoxelFilter2D( double* featureElement, double* rasterArray, voxelFilter2DElements* sparseFilter, uint xyzDim );
void     applyVoxelFilters2D( double* featureArray, double* rasterArray, voxelFilter2DElements** sparseFilters, uint multiscaleRadiiSize, uint xyzDim );

double   sumVoxelFilter2D( double* voxelFilter2D, uint xyzDim );
bool     applyVoxelFilter2D( double* featureElement, double* rasterArray, double* voxelFilter2D, uint xyzDim );
void     applyVoxelFilters2D( double* featureArray, double* rasterArray, uint multiscaleRadiiSize, double** voxelFilters2D, uint xyzDim );
double*  applyVoxelFilter2DImage( double* rasterArray, double* voxelFilter2D, uint xyzDim );
double** applyVoxelFilters2DImage( double* rasterArray, uint multiscaleRadiiSize, double** voxelFilters2D, uint xyzDim );

#endif

/*
	64x64 Voxelfilter with sphere:
	==============================
	0 seconds for keilschrift_schnipsel

	NO Sub-Pixels:
	--------------
	Sum of the filter mask:   137283.1 Ideal:   137258.3 Error: -0.018 %
	Sum of the filter mask:   100063.9 Ideal:   100061.3 Error: -0.003 %
	Sum of the filter mask:    70266.3 Ideal:    70276.2 Error:  0.014 %
	Sum of the filter mask:    47075.1 Ideal:    47079.6 Error:  0.010 %
	Sum of the filter mask:    29649.2 Ideal:    29647.8 Error: -0.005 %
	Sum of the filter mask:    17168.5 Ideal:    17157.3 Error: -0.066 %
	Sum of the filter mask:     8785.9 Ideal:     8784.5 Error: -0.015 %
	Sum of the filter mask:     3705.4 Ideal:     3706.0 Error:  0.017 %
	Sum of the filter mask:     1096.4 Ideal:     1098.1 Error:  0.152 %
	Sum of the filter mask:      137.5 Ideal:      137.3 Error: -0.179 %

	9 Sub-Pixels: (sub-optimal)
	---------------------------
	Sum of the filter mask:   137289.0 Ideal:   137258.3 Error: -0.022 %
	Sum of the filter mask:   100072.4 Ideal:   100061.3 Error: -0.011 %
	Sum of the filter mask:    70244.7 Ideal:    70276.2 Error:  0.045 %
	Sum of the filter mask:    47072.4 Ideal:    47079.6 Error:  0.015 %
	Sum of the filter mask:    29666.1 Ideal:    29647.8 Error: -0.062 %
	Sum of the filter mask:    17190.4 Ideal:    17157.3 Error: -0.193 %
	Sum of the filter mask:     8792.0 Ideal:     8784.5 Error: -0.086 %
	Sum of the filter mask:     3689.4 Ideal:     3706.0 Error:  0.448 %
	Sum of the filter mask:     1088.1 Ideal:     1098.1 Error:  0.906 %
	Sum of the filter mask:      140.7 Ideal:      137.3 Error: -2.504 %

	25 Sub-Pixels: (optimal)
	------------------------
	Sum of the filter mask:   137236.8 Ideal:   137258.3 Error:  0.016 %
	Sum of the filter mask:   100059.9 Ideal:   100061.3 Error:  0.001 %
	Sum of the filter mask:    70280.7 Ideal:    70276.2 Error: -0.006 %
	Sum of the filter mask:    47086.1 Ideal:    47079.6 Error: -0.014 %
	Sum of the filter mask:    29645.6 Ideal:    29647.8 Error:  0.007 %
	Sum of the filter mask:    17148.8 Ideal:    17157.3 Error:  0.049 %
	Sum of the filter mask:     8783.9 Ideal:     8784.5 Error:  0.007 %
	Sum of the filter mask:     3708.7 Ideal:     3706.0 Error: -0.074 %
	Sum of the filter mask:     1098.9 Ideal:     1098.1 Error: -0.079 %
	Sum of the filter mask:      137.2 Ideal:      137.3 Error:  0.078 %

	128x128 Voxelfilter with sphere:
	================================
	2 seconds for keilschrift_schnipsel

	NO Sub-Pixels:
	--------------
	Sum of the filter mask:  1098236.1 Ideal:  1098066.2 Error: -0.015 %
	Sum of the filter mask:   800461.2 Ideal:   800490.3 Error:  0.004 %
	Sum of the filter mask:   562206.9 Ideal:   562209.9 Error:  0.001 %
	Sum of the filter mask:   376697.4 Ideal:   376636.7 Error: -0.016 %
	Sum of the filter mask:   237170.6 Ideal:   237182.3 Error:  0.005 %
	Sum of the filter mask:   137289.0 Ideal:   137258.3 Error: -0.022 %
	Sum of the filter mask:    70244.7 Ideal:    70276.2 Error:  0.045 %
	Sum of the filter mask:    29666.1 Ideal:    29647.8 Error: -0.062 %
	Sum of the filter mask:     8792.0 Ideal:     8784.5 Error: -0.086 %
	Sum of the filter mask:     1088.1 Ideal:     1098.1 Error:  0.906 %

	9 Sub-Pixels: (good)
	---------------------------
	Sum of the filter mask:  1098109.0 Ideal:  1098066.2 Error: -0.004 %
	Sum of the filter mask:   800474.4 Ideal:   800490.3 Error:  0.002 %
	Sum of the filter mask:   562222.6 Ideal:   562209.9 Error: -0.002 %
	Sum of the filter mask:   376640.9 Ideal:   376636.7 Error: -0.001 %
	Sum of the filter mask:   237177.1 Ideal:   237182.3 Error:  0.002 %
	Sum of the filter mask:   137283.1 Ideal:   137258.3 Error: -0.018 %
	Sum of the filter mask:    70266.3 Ideal:    70276.2 Error:  0.014 %
	Sum of the filter mask:    29649.2 Ideal:    29647.8 Error: -0.005 %
	Sum of the filter mask:     8785.9 Ideal:     8784.5 Error: -0.015 %
	Sum of the filter mask:     1096.4 Ideal:     1098.1 Error:  0.152 %

	25 Sub-Pixels: (better)
	------------------------
	Sum of the filter mask:  1098028.7 Ideal:  1098066.2 Error:  0.003 %
	Sum of the filter mask:   800506.9 Ideal:   800490.3 Error: -0.002 %
	Sum of the filter mask:   562207.1 Ideal:   562209.9 Error:  0.000 %
	Sum of the filter mask:   376637.5 Ideal:   376636.7 Error: -0.000 %
	Sum of the filter mask:   237192.8 Ideal:   237182.3 Error: -0.004 %
	Sum of the filter mask:   137236.8 Ideal:   137258.3 Error:  0.016 %
	Sum of the filter mask:    70280.7 Ideal:    70276.2 Error: -0.006 %
	Sum of the filter mask:    29645.6 Ideal:    29647.8 Error:  0.007 %
	Sum of the filter mask:     8783.9 Ideal:     8784.5 Error:  0.007 %
	Sum of the filter mask:     1098.9 Ideal:     1098.1 Error: -0.079 %

	256x256 Voxelfilter with sphere:
	================================
	8 seconds for keilschrift_schnipsel

	512x512 Voxelfilter with sphere:
	================================
	45 seconds for keilschrift_schnipsel

	1024x1024 Voxelfilter with sphere:
	==================================
	291 seconds for keilschrift_schnipsel

*/
