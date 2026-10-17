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

#include "image2d.h"

using namespace std;

int main(void) {  
	//! Main routine for testing access to 2D-Images
	//==========================================================================
	Image2D someImage;
	// write a 5x4 TIFF with a red, green and blue pixle:
	char raster[ 5 * 4 * 3 ] = {
		0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0xF, 0x0, 0x0,  0x0, 0x0, 0x0,
		0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0xF, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,
		0x0, 0x0, 0x0,  0x0, 0x0, 0xF,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,
		0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0,  0x0, 0x0, 0x0
	};
	someImage.writeTIFF( "test1.tiff", 5, 4, raster );

	// write a 100x50 TIFF with random colours:
	cout << "RAND_MAX: " << RAND_MAX << endl;
	srand( rand() );
	float rasterRGB[ 100*50*3 ];
	for( unsigned int i=0; i<(100*50*3); i++ ) { 
		rasterRGB[i] = (float)abs(rand())/RAND_MAX;
	}
	someImage.writeTIFF( "test2.tiff", 100, 50, rasterRGB );

	// write a 100x50 TIFF with random grayscale:
	cout << "RAND_MAX: " << RAND_MAX << endl;
	srand( rand() );
	float rasterGray[ 100*50 ];
	for( unsigned int i=0; i<(100*50); i++ ) { 
		rasterGray[i] = (float)abs(rand())/RAND_MAX;
	}
	someImage.writeTIFF( "test3.tiff", 100, 50, rasterGray, false );
}
