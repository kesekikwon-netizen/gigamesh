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

#include <GigaMesh/mesh/geodentry.h>

//! Constructor
GeodEntry::GeodEntry( double rDist, double rAngle, Primitive* rFromSeed )
	: mGeodDist( rDist ), mGeodAngle( rAngle ), mFromSeed( rFromSeed ) {
	//cout << "[GeodEntry::" << __FUNCTION__ << "] Angle: " << rAngle << endl;
	if( rAngle < -M_PI ) {
		mGeodAngle = -( rAngle + M_PI );
	}
	if( rAngle > +M_PI ) {
		mGeodAngle = -( rAngle - M_PI );
	}
}

//! Updates the entry only, when rDist is smaller.
//! @returns true, when an update was made -- false otherwise.
bool GeodEntry::setGeodDistSmaller( double rDist, double rAngle, Primitive* rFromSeed ) {
	if( rDist > mGeodDist ) {
		return false;
	}
	mGeodDist = rDist;
//	if( rDist == mGeodDist ) {
//		mGeodAngle = ( mGeodAngle + rAngle ) / 2.0;
//	} else {
//		mGeodAngle = rAngle;
//	}
	mGeodAngle = rAngle;
	if( rAngle < -M_PI ) {
		mGeodAngle = -( rAngle + M_PI );
	}
	if( rAngle > +M_PI ) {
		mGeodAngle = -( rAngle - M_PI );
	}
	mFromSeed = rFromSeed;
	return true;
}

//! Retrieve the geodisc distance.
//! @returns false in case of an error.
bool GeodEntry::getGeodDist( double* rDist ) {
	(*rDist) = mGeodDist;
	return true;
}

//! Retrieve the geodisc angle.
//! @returns false in case of an error.
bool GeodEntry::getGeodAngle( double* rAngle ) {
	(*rAngle) = mGeodAngle;
	return true;
}

//! Retrieve the pointer to the seed Primitive.
//! @returns false in case of an error.
bool GeodEntry::getFromSeed( Primitive** rFromSeed ) {
	(*rFromSeed) = mFromSeed;
	return true;
}

//! Returns the label id of the seed Primitive -- see Primitive::getLabelID
//! @returns false in case of an error.
bool GeodEntry::getLabel( uint64_t& rLabelId ) const {
	return mFromSeed->getLabel( rLabelId );
}
