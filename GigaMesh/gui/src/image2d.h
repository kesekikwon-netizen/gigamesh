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

#ifndef IMAGE2D_H
#define IMAGE2D_H
#ifdef LIBTIFF
	#include "tiffio.h"
	// for insights about uint32_t visit: http://stackoverflow.com/questions/911035/uint32_t-int16-and-the-like-are-they-standard-c
#endif

// C++ includes:
#include <GigaMesh/mesh/gmcommon.h>

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib> // uint, callo
#include<filesystem>

// identical to meshio:

//!
//! \brief Class handling 2D images. (Layer -1)
//!
//! This class should be used to access 2D images. (Layer -1)
//!
//! This was the class to save tiff images, however tifs are not longer required
//! the class is refactored completly to PNG
//!
//! DEPRECATED
//! Requires: libtiff
//! see: http://www.libtiff.org
//!
//! If libtiff is not present, tiffs are substituted by PNG's
//! Layer -1
//!

class Image2D {
	//! \todo: collect common flags from image2d.h and meshio.h (_READ_..., etc.)
	
	public:
		// constructor and deconstructor:
		Image2D();
		~Image2D() = default;

		void setResolution(const double xRes, const double yRes, const short resolutionUnit=0 );

        int writePNG(const std::filesystem::path& filename, uint32_t width, uint32_t height, double* raster, double maxVal, bool isRGB=true );
        int writePNG(const std::filesystem::path& filename, uint32_t width, uint32_t height, float*  raster, float  minVal=_NOT_A_NUMBER_, float  maxVal=_NOT_A_NUMBER_, bool isRGB=true );
        int writePNG(const std::filesystem::path& filename, uint32_t width, uint32_t height, double* raster, double minVal=_NOT_A_NUMBER_, double maxVal=_NOT_A_NUMBER_, bool isRGB=true );
		//int writeTIFF(      std::string  filename, uint32_t width, uint32_t height, unsigned char* raster, bool isRGB=true );
        int writePNG(std::filesystem::path filename, uint32_t width, uint32_t height, unsigned char* raster, bool isRGB=true );

	private:
		short  resolutionUnit; //!< None, Inch (DPI), Centimeter
		double xRes;           //!< dots per resolutionUnit in x
		double yRes;           //!< dots per resolutionUnit in y
};

#endif
