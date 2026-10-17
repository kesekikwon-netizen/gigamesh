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

#ifndef BITFLAGARRAY_H
#define BITFLAGARRAY_H

#include <cstdint>

class BitFlagArray {

public:
	// Constructor
	BitFlagArray();
	virtual ~BitFlagArray() = default;

	// Pre-define flags
	enum eBitFlags {
		FLAG_00 = 1U<<0,
		FLAG_01 = 1U<<1,
		FLAG_02 = 1U<<2,
		FLAG_03 = 1U<<3,
		FLAG_04 = 1U<<4,
		FLAG_05 = 1U<<5,
		FLAG_06 = 1U<<6,
		FLAG_07 = 1U<<7,
		FLAG_08 = 1U<<8,
		FLAG_09 = 1U<<9,
		FLAG_10 = 1U<<10,
		FLAG_11 = 1U<<11,
		FLAG_12 = 1U<<12,
		FLAG_13 = 1U<<13  // Cortex region flag (Raczynski-Henk 2017)
	};

	// Single flags
	bool     getFlag( uint64_t rFlagNr ) const;
	bool     setFlag( uint64_t rFlagNr );
	bool     clearFlag( uint64_t rFlagNr );
	// All flags
	bool     getFlagAll( uint64_t* rFlags ) const;
	bool     setFlagAll( uint64_t rFlags );

private:
	// One BitArray for 64 flags:
	uint64_t mBitFlags;     //! perfect for a 64-bit system - we can handle 64 flags

};

#endif // BITFLAGARRAY_H
