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

#include "image2d.h"

#include <sstream>

using namespace std;

constexpr int WRITE_OK = 0;
constexpr int WRITE_ERROR = -1;



#ifndef LIBTIFF
    #include <QImage>
#endif


Image2D::Image2D() {
	//! Constructor
#ifdef LIBTIFF
	resolutionUnit = RESUNIT_NONE;
#endif
	xRes = 0.0;
	yRes = 0.0;
}

void Image2D::setResolution( const double setXRes, const double setYRes, const short setResolutionUnit ) {
	//! 
	resolutionUnit = setResolutionUnit;
	xRes = setXRes;
	yRes = setYRes;
}

int Image2D::writePNG( const filesystem::path&  filename, //!< Name of the file to be written.
            const uint32_t  width,                 //!< Image width (pixel).
            const uint32_t  height,                //!< Image height (pixel).
                    double* raster,              //!< Colour-data width*height*(3|1) [0.0..maxVal].
                    const double  maxVal,        //!< maximum value for grayscale (or each of the RGB-channels)
                    const bool    isRGB          //!< RGB or Grayscale
	) {
	//! Convert some double data and write to a TIFF file.

	uint8_t* rasterRGB = new uint8_t[width*height*3];

	if( isRGB ) {
		for( uint32_t i=0; i<width*height*3; i++ ) {
			// some precaution about under- and overflows:
			if( raster[i] < 0.0 ) {
				cout << "[Image2D::writeTIFF] warning: underflow." << endl;
				rasterRGB[i] = 0x0;
			} else if( raster[i] > maxVal ) {
				cout << "[Image2D::writeTIFF] warning: overflow." << endl;
				rasterRGB[i] = 0xF;
			} else {
				rasterRGB[i] = floor( 255.0 * raster[i] / maxVal );
			}
		}
	} else {
		for( uint32_t i=0; i<width*height; i++ ) {
			// some precaution about under- and overflows:
			if( raster[i] < 0.0 ) {
				cout << "[Image2D::writeTIFF] warning: underflow: " << raster[i] << endl;
				rasterRGB[i] = 0x00;
			} else if( raster[i] > maxVal ) {
				cout << "[Image2D::writeTIFF] warning: overflow: " << raster[i] << endl;
				rasterRGB[i] = 0xFF;
			} else {
				rasterRGB[i] = floor( 255.0 * raster[i] / maxVal );
			}
		}
	}

    return writePNG( filename, width, height, rasterRGB, isRGB );

	delete[] rasterRGB;
}

//! Convert some float data and write to a TIFF file.
//! When both minVal and maxVal are a number (in contrast to not-a-number)
//! the values of raster will be normalized.
//!
//! Shows a warning in case of over- and underflows.
//! Sets pixels with under-/overflow to black/white
int Image2D::writePNG( const filesystem::path& filename, //!< Name of the file to be written.
            const uint32_t width,                 //!< Image width (pixel).
            const uint32_t height,                //!< Image height (pixel).
                    float* raster,              //!< Colour-data width*height*(3|1) [minVal...maxVal].
                    const float  minVal,        //!< minimum Value (for normalization to [0...255]
                    const float  maxVal,        //!< minimum Value (for normalization to [0...255]
                    const bool   isRGB)          //!< RGB or Grayscale
{
	//cout << "[Image2D::writeTIFF] FLOAT" << endl;

	unsigned char*  rasterArray = nullptr;
	uint32_t rasterArraySize;
	bool   normalize = false;
	float  range = 0.0;
	float  newValue;
	bool   warnOverFlow = false;
	bool   warnUnderFlow = false;

	// set normalization
	if( !isnan( minVal ) && !isnan( maxVal ) ) {
		range = maxVal - minVal;
		normalize = true;
		if( range <= 0.0 ) {
			cerr << __PRETTY_FUNCTION__ << " Normalization can not be performed: minVal > maxVal!";
			normalize = false;
		}
	}
	// set proper size
	if( isRGB ) {
		rasterArraySize = width*height*3;
	} else {
		rasterArraySize = width*height;
	}
	// allocate memory
	rasterArray = new unsigned char[rasterArraySize];
	// fill/convert char array:
	for( uint32_t i=0; i<rasterArraySize; i++ ) {
		if( normalize ) {
			newValue = floor( CHAR_MAX * ( ( raster[i]-minVal ) / range ) + 0.5 );
		} else {
			newValue = floor( raster[i] + 0.5 );
		}
		if( newValue < 0.0 ) {
			newValue      = 0;
			warnUnderFlow = true;
		}
		if( newValue > 255.0 ) {
			newValue     = 255;
			warnOverFlow = true;
		}
		rasterArray[i] = newValue;
	}
	if( warnOverFlow ) {
		cerr << __PRETTY_FUNCTION__ << " Overflow Warning, value > 255 found!";
	}
	if( warnUnderFlow ) {
		cerr << __PRETTY_FUNCTION__ << " Underflow Warning, value < 0 found!";
	}

    bool retVal = writePNG( filename, width, height, rasterArray, isRGB );
	delete[] rasterArray;

	return retVal;
}

int Image2D::writePNG( const filesystem::path&  filename, //!< Name of the file to be written.
            uint32_t  width,    //!< Image width (pixel).
            uint32_t  height,   //!< Image height (pixel).
	                double* raster,   //!< Colour-data width*height*(3|1) [minVal...maxVal].
	                double  minVal,   //!< minimum Value (for normalization to [0...255]
	                double  maxVal,   //!< minimum Value (for normalization to [0...255]
	                bool    isRGB     //!< RGB or Grayscale
	) {
	//! Convert some double data and write to a TIFF file.
	//! When both minVal and maxVal are a number (in contrast to not-a-number) 
	//! the values of raster will be normalized.
	//!
	//! Shows a warning in case of over- and underflows.
	//! Sets pixels with under-/overflow to black/white 

	//cout << "[Image2D::writeTIFF] DOUBLE" << endl;

	unsigned char*   rasterArray = nullptr;
	uint32_t  rasterArraySize;
	bool    normalize = false;
	double  range = 0.0;
	double  newValue;
	bool    warnOverFlow = false;
	bool    warnUnderFlow = false;

	// set normalization
	if( !std::isnan( minVal ) && !std::isnan( maxVal ) ) {
		range = maxVal - minVal;
		normalize = true;
		if( range <= 0.0 ) {
			cerr << __PRETTY_FUNCTION__ << " Normalization can not be performed: minVal > maxVal!";
			normalize = false;
		}
	}
	// set proper size
	if( isRGB ) {
		rasterArraySize = width*height*3;
	} else {
		rasterArraySize = width*height;
	}
	// allocate memory
	rasterArray = new unsigned char[rasterArraySize];
	// fill/convert char array:
	for( uint32_t i=0; i<rasterArraySize; i++ ) {
		if( normalize ) {
			newValue = floor( CHAR_MAX * ( ( raster[i]-minVal ) / range ) + 0.5 );
		} else {
			newValue = floor( raster[i] + 0.5 );
		}
		if( newValue < 0.0 ) {
			newValue      = 0;
			warnUnderFlow = true;
		}
		if( newValue > 255.0 ) {
			newValue     = 255;
			warnOverFlow = true;
		}
		rasterArray[i] = newValue;
	}
	if( warnOverFlow ) {
		cerr << __PRETTY_FUNCTION__ << " Overflow Warning, value > 255 found!";
	}
	if( warnUnderFlow ) {
		cerr << __PRETTY_FUNCTION__ << " Underflow Warning, value < 0 found!";
	}

    bool retVal = writePNG( filename, width, height, rasterArray, isRGB );
	delete[] rasterArray;

	return retVal;
}

int Image2D::writePNG(filesystem::path filename, //!< Name of the file to be written.
            uint32_t width,    //!< Image width (pixel).
            uint32_t height,   //!< Image height (pixel).
            unsigned char*  raster,   //!< Colour-data width*height*(3|1) [0..255].
			bool   isRGB     //!< RGB or Grayscale
	) {
    //! Write some data to a PNG file.
	//!
	//! and more important a working real-world example: 
	//! for a binary-image: http://www.ibm.com/developerworks/linux/library/l-libtiff/
	//! for a colour-image: http://www.ibm.com/developerworks/linux/library/l-libtiff2/
	//cerr << "[Image2D::" << __FUNCTION__ << "] ERROR: libtiff NOT present!" << endl;

	if(filename.extension().empty())
	{
		filename += ".png";
	}
	else
	{
		filename.replace_extension(".png");
	}

	QImage img(raster, static_cast<int>(width),
	           static_cast<int>(height), 
			   (isRGB ? 3 * width * sizeof(unsigned char) : width * sizeof(unsigned char)),
	           (isRGB ? QImage::Format_RGB888 : QImage::Format_Grayscale8));

    if(!img.save(QString::fromStdWString(filename.wstring())))
        return WRITE_ERROR;

	return  WRITE_OK;

}

/*
int Image2D::writeTIFF( const string& filename,       //!< Name of the file to be written.
			uint32_t width,          //!< Image width (pixel).
			uint32_t height,         //!< Image height (pixel).
	                unsigned char* raster, //!< Colour-data width*height*(3|1) [0..255].
	                bool isRGB             //!< RGB or Grayscale
	) {
	//! Write some unsigned data to a TIFF file.
	return writeTIFF( filename, width, height, reinterpret_cast<char*>(raster), isRGB );
}
*/

