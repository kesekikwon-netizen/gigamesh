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

#ifndef GEODENTRY_H
#define GEODENTRY_H

#include "primitive.h"

class Primitive;

//!
//! \brief Struct for storing a geodesic distance and the Primitive it is closest to. (Layer -0.5)
//!
//! ...
//!
//! Layer -0.5
//!

class GeodEntry {
	public:
		GeodEntry( double rDist, double rAngle, Primitive* rFromSeed );
		~GeodEntry() = default;

		bool setGeodDistSmaller( double rDist, double rAngle, Primitive* rFromSeed );

		bool getGeodDist( double* rDist );
		bool getGeodAngle( double* rAngle );
		bool getFromSeed( Primitive** rFromSeed );

		bool getLabel( uint64_t& rLabelId ) const;

	private:
		double     mGeodDist;  //!< The geodesic distance.
		double     mGeodAngle; //!< Experimental: angle related to geodesics
		Primitive* mFromSeed;  //!< Reference to the seed Primitive.
};

#endif
