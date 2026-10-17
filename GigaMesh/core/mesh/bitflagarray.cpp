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

#include <GigaMesh/mesh/bitflagarray.h>

#define BITFLAGARRAY_INITDEFAULTS \
	mBitFlags( 0x00000000 )

//! Constructor
BitFlagArray::BitFlagArray()
	: BITFLAGARRAY_INITDEFAULTS {
}


// Single flags ------------------------------------------------------------------------------------------------------------------------------------------------

//! Checks if one (or more) flag(s) are set - see BitFlagArray::mBitFlags
bool BitFlagArray::getFlag( uint64_t rFlagNr ) const {
	return ( mBitFlags & rFlagNr );
}

//! Sets a given flag to true - see BitFlagArray::mBitFlags
bool BitFlagArray::setFlag( uint64_t rFlagNr ) {
	mBitFlags |= rFlagNr;
	return true;
}

//! Sets a given flag to false - see BitFlagArray::mBitFlags
bool BitFlagArray::clearFlag( uint64_t rFlagNr ) {
	mBitFlags &= ~rFlagNr;
	return true;
}

// All flags ---------------------------------------------------------------------------------------------------------------------------------------------------

//! Set bit flag array to rFlags.
bool BitFlagArray::getFlagAll( uint64_t* rFlags ) const {
	(*rFlags) = mBitFlags;
	return true;
}

//! Set bit flag array to rFlags.
bool BitFlagArray::setFlagAll( uint64_t rFlags ) {
	mBitFlags = rFlags;
	return true;
}
