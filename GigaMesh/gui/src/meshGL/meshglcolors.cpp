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

#include "meshglcolors.h"

#include <iostream>

//! Constructor.
MeshGLColors::MeshGLColors() {
	// Initalize colors:
	mColorSetting[COLOR_MESH_SOLID][0]    = 187;
	mColorSetting[COLOR_MESH_SOLID][1]    = 187;
	mColorSetting[COLOR_MESH_SOLID][2]    = 187;
	mColorSetting[COLOR_MESH_SOLID][3]    = 255;

	mColorSetting[COLOR_MESH_BACKFACE][0] = 128; // 200
	mColorSetting[COLOR_MESH_BACKFACE][1] =  92; // 200
	mColorSetting[COLOR_MESH_BACKFACE][2] =  92; // 0
	mColorSetting[COLOR_MESH_BACKFACE][3] = 255; // 255

	mColorSetting[COLOR_VERTEX_MONO][0]   = 128;
	mColorSetting[COLOR_VERTEX_MONO][1]   = 128;
	mColorSetting[COLOR_VERTEX_MONO][2]   = 128;
	mColorSetting[COLOR_VERTEX_MONO][3]   = 255;

	mColorSetting[COLOR_VERTEX_LOCAL_MIN][0]   = 215; // colorbrewer2: RdYIGn >3 elements
	mColorSetting[COLOR_VERTEX_LOCAL_MIN][1]   =  25;
	mColorSetting[COLOR_VERTEX_LOCAL_MIN][2]   =  28;
	mColorSetting[COLOR_VERTEX_LOCAL_MIN][3]   = 255;

	mColorSetting[COLOR_VERTEX_LOCAL_MAX][0]   =  26; // colorbrewer2: RdYIGn >3 elements
	mColorSetting[COLOR_VERTEX_LOCAL_MAX][1]   = 150;
	mColorSetting[COLOR_VERTEX_LOCAL_MAX][2]   =  65;
	mColorSetting[COLOR_VERTEX_LOCAL_MAX][3]   = 255;

	mColorSetting[COLOR_EDGE_MONO][0]     =  32;
	mColorSetting[COLOR_EDGE_MONO][1]     =  32;
	mColorSetting[COLOR_EDGE_MONO][2]     =  32;
	mColorSetting[COLOR_EDGE_MONO][3]     = 255;

	mColorSetting[COLOR_POLYLINE_MONO][0] =   0;
	mColorSetting[COLOR_POLYLINE_MONO][1] = 170;
	mColorSetting[COLOR_POLYLINE_MONO][2] = 255;
	mColorSetting[COLOR_POLYLINE_MONO][3] = 255;

	mColorSetting[COLOR_LABEL_NOT_ASSIGNED][0] = 157;
	mColorSetting[COLOR_LABEL_NOT_ASSIGNED][1] =  24;
	mColorSetting[COLOR_LABEL_NOT_ASSIGNED][2] =  24;
	mColorSetting[COLOR_LABEL_NOT_ASSIGNED][3] = 255;

	mColorSetting[COLOR_LABEL_SOLID][0]        =   0;
	mColorSetting[COLOR_LABEL_SOLID][1]        =   0;
	mColorSetting[COLOR_LABEL_SOLID][2]        =   0;
	mColorSetting[COLOR_LABEL_SOLID][3]        = 255;

	mColorSetting[COLOR_LABEL_BORDER][0] =  64;
	mColorSetting[COLOR_LABEL_BORDER][1] =  64;
	mColorSetting[COLOR_LABEL_BORDER][2] =  64;
	mColorSetting[COLOR_LABEL_BORDER][3] = 255;

	mColorSetting[COLOR_LABEL_BACKGROUND][0] =  64;
	mColorSetting[COLOR_LABEL_BACKGROUND][1] =  64;
	mColorSetting[COLOR_LABEL_BACKGROUND][2] =  64;
	mColorSetting[COLOR_LABEL_BACKGROUND][3] = 255;

	mColorSetting[COLOR_NPR_DIFFUSE1][0] = 234;
	mColorSetting[COLOR_NPR_DIFFUSE1][1] = 234;
	mColorSetting[COLOR_NPR_DIFFUSE1][2] = 234;
	mColorSetting[COLOR_NPR_DIFFUSE1][3] = 255;

	mColorSetting[COLOR_NPR_DIFFUSE2][0] = 203;
	mColorSetting[COLOR_NPR_DIFFUSE2][1] = 203;
	mColorSetting[COLOR_NPR_DIFFUSE2][2] = 203;
	mColorSetting[COLOR_NPR_DIFFUSE2][3] = 255;

	mColorSetting[COLOR_NPR_DIFFUSE3][0] = 152;
	mColorSetting[COLOR_NPR_DIFFUSE3][1] = 152;
	mColorSetting[COLOR_NPR_DIFFUSE3][2] = 152;
	mColorSetting[COLOR_NPR_DIFFUSE3][3] = 255;

	mColorSetting[COLOR_NPR_DIFFUSE4][0] = 101;
	mColorSetting[COLOR_NPR_DIFFUSE4][1] = 101;
	mColorSetting[COLOR_NPR_DIFFUSE4][2] = 101;
	mColorSetting[COLOR_NPR_DIFFUSE4][3] = 255;

	mColorSetting[COLOR_NPR_DIFFUSE5][0] = 50;
	mColorSetting[COLOR_NPR_DIFFUSE5][1] = 50;
	mColorSetting[COLOR_NPR_DIFFUSE5][2] = 50;
	mColorSetting[COLOR_NPR_DIFFUSE5][3] = 255;

	mColorSetting[COLOR_NPR_SPECULAR][0] = 255;
	mColorSetting[COLOR_NPR_SPECULAR][1] = 255;
	mColorSetting[COLOR_NPR_SPECULAR][2] = 255;
	mColorSetting[COLOR_NPR_SPECULAR][3] = 255;

	mColorSetting[COLOR_NPR_OUTLINE][0] = 0;
	mColorSetting[COLOR_NPR_OUTLINE][1] = 0;
	mColorSetting[COLOR_NPR_OUTLINE][2] = 0;
	mColorSetting[COLOR_NPR_OUTLINE][3] = 255;

	mColorSetting[COLOR_NPR_HATCHLINE][0] = 0;
	mColorSetting[COLOR_NPR_HATCHLINE][1] = 0;
	mColorSetting[COLOR_NPR_HATCHLINE][2] = 0;
	mColorSetting[COLOR_NPR_HATCHLINE][3] = 255;
}

//! Writes the color as float to the given array[4].
//! @returns false in case of an error.
bool MeshGLColors::getColorSettings( eColorSettings rColorId, GLfloat* rVec4 ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
	// Sanity check:
	if( rVec4 == nullptr ) {
		return false;
	}
	rVec4[0] = static_cast<GLfloat>(mColorSetting[rColorId][0])/255.0;
	rVec4[1] = static_cast<GLfloat>(mColorSetting[rColorId][1])/255.0;
	rVec4[2] = static_cast<GLfloat>(mColorSetting[rColorId][2])/255.0;
	rVec4[3] = static_cast<GLfloat>(mColorSetting[rColorId][3])/255.0;
	return true;
}

//! Takes the color from a given float array[4].
//! @returns false in case of an error.
bool MeshGLColors::setColorSettings(
                eColorSettings rColorId, //!< Id of the color to be stored.
                const GLfloat* iVec4     //!< R,G,B and A values of the new color in [0.0...1.0].
) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << "[MeshGL::" << __FUNCTION__ << "]" << endl;
#endif
	// Sanity check.
	if( iVec4 == nullptr ) {
		return( false );
	}
	mColorSetting[rColorId][0] = iVec4[0] * 255.0;
	mColorSetting[rColorId][1] = iVec4[1] * 255.0;
	mColorSetting[rColorId][2] = iVec4[2] * 255.0;
	mColorSetting[rColorId][3] = iVec4[3] * 255.0;

	// Sort of a bugfix: backface colors have to be opaque. Otherwise the backfaces are shown in white.
	if( rColorId == COLOR_MESH_BACKFACE ) {
		if( iVec4[3] != 1.0 ) {
			std::cerr << "[MeshGLColors::" << __FUNCTION__ << "] ERROR: Transparency not supported for backfaces!" << std::endl;
		}
		mColorSetting[rColorId][3] = 255.0;
	}

	return( true );
}
