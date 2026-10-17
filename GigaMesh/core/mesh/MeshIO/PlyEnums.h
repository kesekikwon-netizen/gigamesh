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

#ifndef PLYENUMS_H
#define PLYENUMS_H
enum ePlySections {
	PLY_VERTEX,
	PLY_FACE,
	PLY_POLYGONAL_LINE,
	PLY_SECTION_UNSUPPORTED,
	PLY_SECTIONS_COUNT
};

enum ePlyProperties {
	PLY_UNSUPPORTED,
	PLY_COORD_X,
	PLY_COORD_Y,
	PLY_COORD_Z,
	PLY_COLOR_RED,
	PLY_COLOR_GREEN,
	PLY_COLOR_BLUE,
	PLY_COLOR_ALPHA,
	PLY_FLAGS,
	PLY_VERTEX_QUALITY,
	PLY_VERTEX_INDEX,
	PLY_LABEL,
	PLY_VERTEX_NORMAL_X,
	PLY_VERTEX_NORMAL_Y,
	PLY_VERTEX_NORMAL_Z,
	PLY_VERTEX_TEXCOORD_S,	//blender style texture coordinates
	PLY_VERTEX_TEXCOORD_T,  //blender style texture coordinates
	PLY_LIST_IGNORE,
	PLY_LIST_FEATURE_VECTOR,
	PLY_LIST_VERTEX_INDICES,
	PLY_LIST_TEXCOORDS,	//meshlab style texture coordinates
	PLY_FACE_TEXNUMBER, //meshlab style texture-id for faces
	PLY_PROPERTIES_COUNT
};

// from PLY-spec ( http://www.cs.kuleuven.ac.be/~ares/libply/ply-0.1/doc/PLY_FILES.txt )
//--------------------------------------------------------------------------------------
// name        type        number of bytes
// ---------------------------------------
// int8       character                 1
// uint8      unsigned character        1
// int16      short integer             2
// uint16     unsigned short integer    2
// int32      integer                   4
// uint32     unsigned integer          4
// float32    single-precision float    4
// float64    double-precision float    8

enum ePlyPropertySize {
	PLY_SIZE_UNDEF = -1,
	PLY_INT8       =  1,
	PLY_UINT8      =  1,
	PLY_SHORT_INT  =  2,
	PLY_USHORT_INT =  2,
	PLY_INT32      =  4,
	PLY_UINT32     =  4,
	PLY_FLOAT32    =  4,
	PLY_FLOAT64    =  8,
	PLY_FLOAT      =  4, // seems not to be from the official spec
	PLY_CHAR       =  1, // seems not to be from the official spec
	PLY_UCHAR      =  1, // seems not to be from the official spec
};
#endif // PLYENUMS_H
