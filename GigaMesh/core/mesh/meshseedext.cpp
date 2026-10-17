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

#include <GigaMesh/mesh/meshseedext.h>

#include <fstream>
#include <ctime>
#include <string>

#include <GigaMesh/logging/Logging.h>

using namespace std;

using uint = unsigned int;

#define MESHSEEDEXTINITDEFAULTS                      \
	mFeatureVecVerticesLen( 0 )                  \

//! Minimalistic constructur initalizing variables and pointers.
MeshSeedExt::MeshSeedExt()
    : MESHSEEDEXTINITDEFAULTS {
    LOG::debug() << "[MeshSeedExt::" << __FUNCTION__ << "].\n";
}

//! Destructor.
MeshSeedExt::~MeshSeedExt() {
	// Features:
	mFeatureVecVertices.clear();
	mFeatureVecVerticesLen = 0;
	// Polygonal lines:
	mPolyLineVertIndices.clear();
	mPolyLabelID.clear();
	mPolyPrimInfo.clear();
}

// Memory management -----------------------------------------------------

//! Frees the memory.
void MeshSeedExt::clear() {
	//! Removes all arrays, sets pointers and counters to zero
	//! and the polyline's indices
	mPolyLineVertIndices.clear();
	mPolyLabelID.clear();
}

// Import / Export -------------------------------------------------------

//! Imports feature vectors to be mapped to the Vertex objects.
//!
//! Expected format ASCII: VertexOriginalIndex featureElement_1 ... featureElement_N (N will be vectorArraySize)
//! The expected delimiter is one space.
//!
//! Returns an array of the vertices indicies of length nrLines.
//! and the feature vectors as vectorArray by size nrLines*vectorArraySize
//!
//! @returns false in case of an error. True otherwise.
bool MeshSeedExt::importFeatureVectors(
                const filesystem::path&   rFileName,            //!< Filename to parse.
                uint64_t                  rNrVertices,          //!< Maximum number of vertices within the Mesh.
                vector<double>&           rFeatureVecs,         //!< Feature vectors.
                uint64_t&                 rMaxFeatVecLen,       //!< Length of the longest vector.
                bool                      rVertexIdInFirstCol   //!< Does the feature vector file have a vertex id within the first column?
) {
	// LOCALE .... because of "." vs ","
	const char* oldLocale = setlocale( LC_NUMERIC, "" );
	setlocale( LC_NUMERIC, "C" );

	ifstream fp( rFileName );
	if( !fp.is_open() ) {
		LOG::error() << "[MeshSeedExt::" << __FUNCTION__ << "] Could not open file: '" << rFileName << "'.\n";
		setlocale(LC_NUMERIC, oldLocale);
		return( false );
	}
	LOG::debug() << "[MeshSeedExt::" << __FUNCTION__ << "] File opened: '" << rFileName << "'.\n";

	// Determine the amount of data by parsing the data for a start
	int    lineNr = 0;
	string line;
	char   nextByte;
	rMaxFeatVecLen = _NOT_A_NUMBER_UINT_;
	uint64_t vertID = 0;

	while( fp.good() ) {
		line.clear();
		nextByte = fp.peek();
		lineNr++;
		// Comment line:
		if( nextByte == '#' ) {
			getline( fp, line );
			continue;
		}
		// Empty line:
		if( ( nextByte == '\n' ) || ( nextByte == '\r' ) || ( nextByte == -1 ) ) {
			getline( fp, line );
			continue;
		}
		// Optional vertex ID (1/2):
		if( rVertexIdInFirstCol ) {
			fp >> vertID;
		}
		nextByte = fp.peek();
		uint64_t valuesPerID = 0;
		while( ( nextByte != '\n' ) && ( nextByte != '\r' ) && ( nextByte != -1 ) ) {
			fp.get();
			string valueAsStr;
			fp >> valueAsStr;
			// double valueAsDouble = stod( valueAsStr );
			nextByte = fp.peek();
			valuesPerID++;
		}
		if( rMaxFeatVecLen < valuesPerID ) {
			rMaxFeatVecLen = valuesPerID;
		}
		// Optional vertex ID (2/2):
		if( !rVertexIdInFirstCol ) {
			vertID++;
		}
		// Fetch the rest of the line
		getline( fp, line );
	}
	LOG::debug() << "[MeshSeedExt::" << __FUNCTION__ << "] nextByte: '" << hex << nextByte << dec << "'\n";
	LOG::debug() << "[MeshSeedExt::" << __FUNCTION__ << "] " << lineNr << " lines parsed.\n";

	// Allocate memory
	rFeatureVecs.clear();
	// and set elements to not-a-number in case there are empty fields
	rFeatureVecs.assign( rMaxFeatVecLen*rNrVertices, _NOT_A_NUMBER_DBL_ );

	// Fetch data
	fp.clear();
	fp.seekg( ios_base::beg );
	vertID = 0;
	while( fp.good() ) {
		nextByte = fp.peek();
		// Comment line:
		if( nextByte == '#' ) {
			getline( fp, line );
			LOG::debug() << "[MeshSeedExt::" << __FUNCTION__ << "] Comment: " << line << "\n";
			continue;
		}
		// Empty line:
		if( ( nextByte == '\n' ) || ( nextByte == '\r' ) || ( nextByte == -1 ) ) {
			getline( fp, line );
			LOG::debug() << "[MeshSeedExt::" << __FUNCTION__ << "] Empty line.\n";
			continue;
		}
		// Optional vertex ID (1/2):
		if( rVertexIdInFirstCol ) {
			fp >> vertID;
		}
		// ... which should be within range
		if( vertID >= rNrVertices ) {
			LOG::warn() << "[MeshSeedExt::" << __FUNCTION__ << "] ERROR: Vertex ID larger than vertex count OR negative!\n";
			getline( fp, line );
			continue;
		}
		nextByte = fp.peek();
		int featElementNr = 0;
		while( ( nextByte != '\n' ) && ( nextByte != '\r' ) && ( nextByte != -1 ) ) {
			string valueAsStr;
			fp >> valueAsStr;
			double valueAsDouble = _NOT_A_NUMBER_DBL_;
			try{
				valueAsDouble = stod( valueAsStr );
			} catch( const std::invalid_argument& ia ) {
				LOG::error() << "[MeshSeedExt::" << __FUNCTION__ << "] ERROR: Conversion to floating point failed for string '" << valueAsStr << "'!\n";
				LOG::error() << "[MeshSeedExt::" << __FUNCTION__ << "]        Invalid argument: " << ia.what() << "\n";
			}
			rFeatureVecs.at( vertID*rMaxFeatVecLen+featElementNr ) = valueAsDouble;
			nextByte = fp.peek();
			featElementNr++;
		}
		// Optional vertex ID (2/2):
		if( !rVertexIdInFirstCol ) {
			vertID++;
		}
		// Fetch the rest of the line
		getline( fp, line );
	}
	fp.close();

	// LOCALE .... because of "." vs ","
	setlocale( LC_NUMERIC, oldLocale );

	return( true );
}

// Feature Vectors - STUB to be overloaded by Mesh -------------------------------------------------------------------------------------------------------------

//! Returns 0 (zero) as there are no feature vectors.
//! Stub to be used by MeshIO::write*
uint64_t MeshSeedExt::getFeatureVecLenMax( int rPrimitiveType ) {
	LOG::warn() << "[MeshSeed::" << __FUNCTION__ << "] can not get maximum length of feature vectors for primitve type " << rPrimitiveType << "!\n";
	return 0;
}

void MeshSeedExt::setFeatureVecVerticesLen(uint64_t len)
{
	mFeatureVecVerticesLen = len;
}

// Polyline related ---------------------------------------------------------------------------------------------

//! Returns the number of polylines within the seed.
unsigned int MeshSeedExt::getPolyLineNr() {
	return mPolyLineVertIndices.size();
}

//! Returns the number of elements of a polyline within the seed.
unsigned int MeshSeedExt::getPolyLineLength( unsigned int rPolyIdx ) {
	vector<int>* somePolyLine = mPolyLineVertIndices.at( rPolyIdx );
	return somePolyLine->size();
}

//! Returns the index of the vertex of a polyline within the seed
int MeshSeedExt::getPolyLineVertIdx( unsigned int rPolyIdx, unsigned int rElementIdx ) {
	vector<int>* somePolyLine = mPolyLineVertIndices.at( rPolyIdx );
	int index = somePolyLine->at( rElementIdx );
	return index;
}

//! Returns the label ID of a polyline.
uint64_t MeshSeedExt::getPolyLineLabel( unsigned int rPolyIdx ) {
	uint64_t labelID = mPolyLabelID.at( rPolyIdx );
	return labelID;
}

//! Returns the primitive information of a polyline, like position vector and normal.
PrimitiveInfo MeshSeedExt::getPolyLinePrimInfo( unsigned int rPolyIdx ) {
	PrimitiveInfo primInfo = mPolyPrimInfo.at( rPolyIdx );
	return primInfo;
}
