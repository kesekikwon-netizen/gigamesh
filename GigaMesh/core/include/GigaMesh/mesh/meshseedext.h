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

#ifndef MESHSEEDEXT_H
#define MESHSEEDEXT_H

// generic C++ includes:
#include <iostream> // cout, endl
#include <vector>

#include <cstdlib> // uint, calloc
#include <cmath>    // pow, sqrt, floor, exp, ...
#include <filesystem>

// C++ includes:
#include "gmcommon.h"

struct PrimitiveInfo {
	double mPosX;
	double mPosY;
	double mPosZ;
	double mNormalX;
	double mNormalY;
	double mNormalZ;
};

//!
//! \brief Class for handling extended properties. (Layer 0)
//!
//! Extends the MeshSeed class:
//! Feature Vectors, selected vertices, etc. to be read/written to files.
//!
//! Layer 0
//!

class MeshSeedExt {

	public:
		MeshSeedExt();
		~MeshSeedExt();

		// Memory management
		virtual void clear();

		// Import / Export - Feature Vectors
		virtual bool importFeatureVectors( const std::filesystem::path& rFileName, uint64_t rNrVerticesMax, std::vector<double>& rFeatureVecs, uint64_t& rMaxFeatVecLen, bool rVertexIdInFirstCol );

		// Feature std::vectors - STUB to be overloaded by Mesh
		virtual uint64_t getFeatureVecLenMax( int rPrimitiveType );
		std::vector<double>& getFeatureVecVerticesRef() {return mFeatureVecVertices;}

		void setFeatureVecVerticesLen(uint64_t len);


		// Polyline related
		virtual unsigned int  getPolyLineNr();
		virtual unsigned int  getPolyLineLength( unsigned int rPolyIdx );
		virtual int           getPolyLineVertIdx( unsigned int rPolyIdx, unsigned int rElementIdx );
		virtual uint64_t getPolyLineLabel( unsigned int rPolyIdx );
		virtual PrimitiveInfo getPolyLinePrimInfo( unsigned int rPolyIdx );

		std::vector<std::vector<int>*>& getPolyLineVertIndicesRef() {return mPolyLineVertIndices;}
		std::vector<uint64_t>& getPolyLabelIDRef() {return mPolyLabelID;}
		std::vector<PrimitiveInfo>& getPolyPrimInfoRef() {return mPolyPrimInfo;}

	protected:
		// Features:
		std::vector<double>        mFeatureVecVertices;      //!< Vector holding the feature vectors for vertices. The number of elements will be featureVecVerticesLen x verticesSize. Note: that featureVecVerticesLen is a maximum if vectors of different length are provided.
		uint64_t         mFeatureVecVerticesLen;   //!< Length of the feature vector of type Vertex.
		// Polygonal lines:
		std::vector<std::vector<int>*>  mPolyLineVertIndices;  //!< Indices refering to the vertices of the polylines.
		std::vector<uint64_t> mPolyLabelID;          //!< Current polyline's label ID.
		std::vector<PrimitiveInfo> mPolyPrimInfo;         //!< Primitive info of the polyline, e.g. the center of gravity -- see PrimitiveInfo.
};

#endif
