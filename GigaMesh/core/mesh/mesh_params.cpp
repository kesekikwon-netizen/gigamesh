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

#include <GigaMesh/mesh/mesh_params.h>

//! Constructor:
//! Sets the default values for the boolean, integer and floating point values.
MeshParams::MeshParams() {
	// Defaults for boolean parameters:
	mParamFlag[FILE_TRANSFORMATION_APPLIED]    = false;
	mParamFlag[GEODESIC_STORE_DIRECTION]       = false;
	mParamFlag[GEODESIC_USE_FUNCVAL_AS_WEIGHT] = false;
	mParamFlag[LABELING_USE_STEP_AS_FUNCVAL]   = false;
	mParamFlag[FILLPOLYLINES_COLOR_AVG]        = true;

	// Defaults for integer parameters:

	// Defaults for floating point parameters:
	mParamFlt[AXIS_PRIMEMERIDIAN] = 0.0;
	mParamFlt[CYLINDER_RADIUS]    = 25.0;
	mParamFlt[SMOOTH_LENGTH] = 0.5;
}

//! Get the state of a flag.
//! @returns: false in case of an error.
bool MeshParams::getParamFlagMesh( MeshParams::eParamFlag rParamID, bool* rValue ) {
	(*rValue) = mParamFlag[rParamID];
	return true;
}

//! Set flag controlling the display of Primitives, etc.
//! @returns: true, when the value was changed. false otherwise.
bool MeshParams::setParamFlagMesh( MeshParams::eParamFlag rParamID, bool rState ) {
	// Prevent infinite loops and unnecessary changes.
	if( mParamFlag[rParamID] == rState ) {
		return false;
	}
	mParamFlag[rParamID] = rState;
	return true;
}

//! Set an integer parameter of the Mesh class.
//! @returns: true, when the value was changed. false otherwise.
bool MeshParams::setParamIntMesh( MeshParams::eParamInt rParam, int rValue ) {
	int oldParam = mParamInt[rParam];
	// Prevent infinite loops:
	if( oldParam == rValue ) {
		return false;
	}
	mParamInt[rParam] = rValue;
	return true;
}

//! Get an integer parameter of the Mesh class.
//! @returns: false in case of an error.
bool MeshParams::getParamIntMesh( MeshParams::eParamInt rParam, int* rValue ) {
	if( rValue == nullptr ) {
		return false;
	}
	(*rValue) = mParamInt[rParam];
	return true;
}

//! Set a floating parameter (double) of the Mesh class.
//! @returns: true, when the value was changed. false otherwise.
bool MeshParams::setParamFloatMesh( MeshParams::eParamFlt rParam, double rValue ) {
	int oldParam = mParamFlt[rParam];
	// Prevent infinite loops:
	if( oldParam == rValue ) {
		return false;
	}
	mParamFlt[rParam] = rValue;
	return true;
}

//! Get a floating parameter (double) of the Mesh class.
//! @returns: false in case of an error.
bool MeshParams::getParamFloatMesh( MeshParams::eParamFlt rParam, double* rValue ) {
	if( rValue == nullptr ) {
		return false;
	}
	(*rValue) = mParamFlt[rParam];
	return true;
}
