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

#include "meshwidget_params.h"
#include <GigaMesh/mesh/mesh_params.h>

#include <QSettings>

using namespace std;

//! Constructor
MeshWidgetParams::MeshWidgetParams() {

	// Initialize flags with false:
	for(bool& paramFlag : mParamFlag) {
		paramFlag = false;
	}

	// Set true, when the default is not false.
	mParamFlag[ORTHO_MODE]               = true;
	mParamFlag[SHOW_HISTOGRAM_SCENE_LOG] = true;
	mParamFlag[SHOW_GRID_RECTANGULAR]    = true;
	mParamFlag[SHOW_GRID_HIGHLIGHTCENTER_FRONT] = true;
	mParamFlag[LIGHT_ENABLED]            = true;
	mParamFlag[LIGHT_FIXED_CAM]          = true;
	mParamFlag[LIGHT_AMBIENT]            = true;
	mParamFlag[CROP_SCREENSHOTS]         = true;
	mParamFlag[EXPORT_SVG_AXIS_DASHED]   = true;
	mParamFlag[EXPORT_SIDE_VIEWS_SIX]    = true;
    mParamFlag[EXPORT_TTL_WITH_PNG] = true;

	// Initalize parameters (int):
	for(int& paramInt : mParamInt) {
		paramInt = 0;
	}
	mParamInt[MOUSE_MODE]         = MOUSE_MODE_MOVE_CAMERA;
	mParamInt[SELECTION_MODE]     = SELECTION_MODE_VERTEX;
	mParamInt[HISTOGRAM_TYPE]     = MeshParams::HISTOGRAM_FUNCTION_VALUES_VERTEX;
	mParamInt[HISTOGRAM_POSX]     =  20;
	mParamInt[HISTOGRAM_POSY]     =  20;
	mParamInt[HISTOGRAM_WIDTH]    = 258;
	mParamInt[HISTOGRAM_HEIGHT]   = 102;
	mParamInt[VIDEO_FRAME_WIDTH]  = 720;
	mParamInt[VIDEO_FRAME_HEIGHT] = 576;
	mParamInt[IMAGE_SPLIT_SIZE]   = 20000;
	mParamInt[LIGHT_VECTORS_SHOWN_MAX] = 3000;
	mParamInt[GRID_CENTER_POSITION] = 0;

	// Initalize parameters (float):
	for(double& paramFlt : mParamFlt) {
		paramFlt = 0.0;
	}
	mParamFlt[ORTHO_ZOOM]           = 1.0;
	// Perspective:
	mParamFlt[FOV_ANGLE]            = 45.0;
	// Navigation:
	mParamFlt[ROTATION_STEP]        = 1.0;
	// Surface properties:
	mParamFlt[AMBIENT_LIGHT]        = AMBIENT_LIGHT_DEFAULT;
	mParamFlt[MATERIAL_SPECULAR]    = MATERIAL_SPECULAR_DEFAULT;
	mParamFlt[MATERIAL_SHININESS]   = exp( MATERIAL_SHININESS_DEFAULT );
	mParamFlt[SPHERICAL_STEPPING]   =  10.0;
	// Light defaults:
	mParamFlt[LIGHT_STEPPING]                = 10.0/180.0;
	mParamFlt[LIGHT_FIXED_WORLD_ANGLE_PHI]   = LIGHTX_ANGLE_PHI_DEFAULT;
	mParamFlt[LIGHT_FIXED_WORLD_ANGLE_THETA] = LIGHTX_ANGLE_THETA_DEFAULT;
	mParamFlt[LIGHT_FIXED_CAM_INTENSITY]     = LIGHTX_INTENSITY_DEFAULT;
	mParamFlt[LIGHT_FIXED_CAM_ANGLE_PHI]     = LIGHTX_ANGLE_PHI_DEFAULT;
	mParamFlt[LIGHT_FIXED_CAM_ANGLE_THETA]   = LIGHTX_ANGLE_THETA_DEFAULT;
	mParamFlt[LIGHT_FIXED_WORLD_INTENSITY]   = LIGHTX_INTENSITY_DEFAULT;
	// Fog defaults:
	mParamFlt[FOG_LINEAR_START] = 100.0f;
	mParamFlt[FOG_LINEAR_END]   = 150.0f;
	// Video defaults:
	mParamFlt[VIDEO_DURATION]       = 50.0;
	mParamFlt[VIDEO_FRAMES_PER_SEC] = 27.0;
	mParamFlt[VIDEO_SLOW_STARTSTOP] = 10.0;
	// True to scale ruler:
	mParamFlt[RULER_HEIGHT]         =   2.0;
	mParamFlt[RULER_WIDTH]          = 100.0; //10.0;
	mParamFlt[RULER_UNIT]           =  10.0; //1.0;
	mParamFlt[RULER_UNIT_TICKS]     =   1.0; //0.0;
	mParamFlt[SVG_SCALE]            =  72.0 / 25.4; // 72 DPInch is the default expected by Inkscape
	mParamFlt[HIGHDPI_ZOOM_FACTOR]  =   1.0;

	// Initalize parameters (string):
	mParamStr[FILENAME_EXPORT_VR]          = "gigamesh_still_image_%05i_%05i_%02i.png";
	mParamStr[FILENAME_EXPORT_VIEWS]       = "%01_%02_%03.png";
	mParamStr[FILENAME_EXPORT_VIEWS_LATEX] = "%s_%02i_%s_test.png";
    mParamStr[FILENAME_EXPORT_RULER]       = "%s_ruler%s.png";
	mParamStr[RULER_WIDTH_UNIT]            = "mm";

	//initialize string parameters for tool-paths
	QSettings settings;
	std::string tmpString("");
	tmpString = settings.value("Inkscape_Path", "").toString().toStdString();
	if(tmpString.length() == 0)
		mParamStr[INKSCAPE_COMMAND] = "inkscape";
	else
		mParamStr[INKSCAPE_COMMAND] = tmpString;

	tmpString = settings.value("PdfLatex_Path", "").toString().toStdString();
	if(tmpString.length() == 0)
		mParamStr[PDF_LATEX_COMMAND] = "pdflatex";
	else
		mParamStr[PDF_LATEX_COMMAND] = tmpString;

	tmpString = settings.value("PdfViewer_Path", "").toString().toStdString();
	if(tmpString.length() == 0)
		mParamStr[PDF_VIEWER_COMMAND] = "evince";
	else
		mParamStr[PDF_VIEWER_COMMAND] = tmpString;

    tmpString = settings.value("PdfViewer_Path", "").toString().toStdString();
    if(tmpString.length() == 0)
        mParamStr[PDF_VIEWER_COMMAND_ALT1] = "okular";
    else
        mParamStr[PDF_VIEWER_COMMAND_ALT1] = tmpString;

    tmpString = settings.value("PdfViewer_Path", "").toString().toStdString();
    if(tmpString.length() == 0)
        mParamStr[PDF_VIEWER_COMMAND_ALT2] = "atril";
    else
        mParamStr[PDF_VIEWER_COMMAND_ALT2] = tmpString;

	tmpString = settings.value("Python3_Path", "").toString().toStdString();
	if(tmpString.length() == 0)
		mParamStr[PYTHON3_COMMAND] = "python3";
	else
		mParamStr[PYTHON3_COMMAND] = tmpString;
}

//! Constructor copying all the settings.
MeshWidgetParams::MeshWidgetParams( const MeshWidgetParams* const rParams ) : MeshWidgetParams() {
	if( rParams == nullptr ) {
		// Do nothing and use defaults.
		return;
	}
	for( unsigned long i=0; i<PARAMS_FLAG_COUNT; i++ ) {
		rParams->getParamFlagMeshWidget( (eParamFlag)i, &mParamFlag[i] );
	}
	for( unsigned long i=0; i<PARAMS_INT_COUNT; i++ ) {
		rParams->getParamIntegerMeshWidget( (eParamInt)i, &mParamInt[i] );
	}
	for( unsigned long i=0; i<PARAMS_FLT_COUNT; i++ ) {
		rParams->getParamFloatMeshWidget( (eParamFlt)i, &mParamFlt[i] );
	}
	for( unsigned long i=0; i<PARAMS_STR_COUNT; i++ ) {
		rParams->getParamStringMeshWidget( (eParamStr)i, &mParamStr[i] );
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------

//! Fetch the flag's state.
//! @returns false in case of an error, true otherwise.
bool MeshWidgetParams::getParamFlagMeshWidget( eParamFlag rFlagNr, bool* rState ) const {
	// Sanity check:
	if( rState == nullptr ) {
		cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	(*rState) = mParamFlag[rFlagNr];
	return true;
}

//! Set flag controlling the display of Primitives, etc.
//! @returns true when the flag was changed. false otherwise.
bool MeshWidgetParams::setParamFlagMeshWidget( eParamFlag rFlagNr, bool rState ) {
	//cout << "[MeshWidgetParams::" << __FUNCTION__ << "] " << rFlagNr << " : " << mParamFlag[rFlagNr] << endl;
	// prevent infinite loops:
	if( mParamFlag[rFlagNr] == rState ) {
		return false;
	}
	mParamFlag[rFlagNr] = rState;
	//cout << "[MeshWidgetParams::" << __FUNCTION__ << "] " << rFlagNr << " : " << mParamFlag[rFlagNr] << endl;
	return true;
}

//! Inverts the given flag controlling the display of Primitives, etc.
//! @returns the old state.
bool MeshWidgetParams::toggleShowFlag( eParamFlag rFlagNr ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	bool retVal = mParamFlag[rFlagNr];
	setParamFlagMeshWidget( rFlagNr, not( mParamFlag[rFlagNr] ) );
	//cout << "[MeshWidgetParams::" << __FUNCTION__ << "] " << rFlagNr << " : " << mParamFlag[rFlagNr] << endl;
	return retVal;
}

//! Fetch the parameters integer point value
//! @returns true when the flag was changed. false otherwise.
bool MeshWidgetParams::getParamIntegerMeshWidget( eParamInt rParamID, int* rValue ) const {
	//! Returns the setting for light, material, etc.
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	// Sanity check:
	if( rValue == nullptr ) {
		cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	(*rValue) = mParamInt[rParamID];
	return true;
}

//! Set the parameters floating point value
//! @returns true when the flag was changed. false otherwise.
bool MeshWidgetParams::setParamIntegerMeshWidget( eParamInt rParamID, int rValue ) {
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	// Prevent infinite loops:
	if( rValue == mParamInt[rParamID] ) {
		return false;
	}
	// Check values:
	switch( rParamID ) {
		case PARAMS_INT_COUNT:
		default:
			// Do noting.
			break;
	}
	mParamInt[rParamID] = rValue;
	// Do usefull things:
	if( ( rParamID == MOUSE_MODE ) && ( rValue == MOUSE_MODE_MOVE_LIGHT_FIXED_CAM ) ) {
		setParamFlagMeshWidget( LIGHT_FIXED_CAM, true );
	}
	if( ( rParamID == MOUSE_MODE ) && ( rValue == MOUSE_MODE_MOVE_LIGHT_FIXED_OBJECT ) ) {
		setParamFlagMeshWidget( LIGHT_FIXED_WORLD, true );
	}
	//cout << "[MeshWidgetParams::" << __FUNCTION__ << "] " << rParamID << " : " << rValue << endl;
	return true;
}

//! Fetch the parameters floating point value
//! @returns true when the flag was changed. false otherwise.
bool MeshWidgetParams::getParamFloatMeshWidget( eParamFlt rParamID, double* rValue ) const {
	//! Returns the setting for light, material, etc.
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	// Sanity check:
	if( rValue == nullptr ) {
		cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	(*rValue) = mParamFlt[rParamID];
	return true;
}

//! Set the parameters floating point value
//! @returns true when the value was changed. false otherwise.
bool MeshWidgetParams::setParamFloatMeshWidget( eParamFlt rParamID, double rValue ) {
	//! Sets the parameters for ambient light, materials, etc.
	//! Ignores values out of range.
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	// Prevent infinite loops:
	if( rValue == mParamFlt[rParamID] ) {
		return false;
	}
	switch( rParamID ) {
		case VIEW_DIST_DECREMENT:
			break;
		case ORTHO_ZOOM:
			if( rValue <= 0.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] ORTHO_ZOOM has to be non-zero and positive " << rValue << "!" << endl;
				return false;
			}
			break;
		case GRID_SHIFT_DEPTH:
			if( rValue < -2.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] to low: " << rValue << "!" << endl;
				return false;
			}
			if( rValue > +0.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] to high: " << rValue << "!" << endl;
				return false;
			}
			break;
		case FOV_ANGLE:
			if( ( rValue <= 0.0 ) || ( rValue >= 180.0 ) ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] FOV_ANGLE] ]0.0 ... 180.0[ out of range: " << rValue << "!" << endl;
				return false;
			}
			break;
		case MATERIAL_SPECULAR:
			if( rValue < 0.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] to low: " << rValue << "!" << endl;
				return false;
			}
			if( rValue > 1.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] to high: " << rValue << "!" << endl;
				return false;
			}
			break;
		case AMBIENT_LIGHT:
		case LIGHT_FIXED_CAM_INTENSITY:
		case LIGHT_FIXED_WORLD_INTENSITY:
			if( rValue < 0.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] to low: " << rValue << "!" << endl;
				return false;
			}
			if( rValue > 5.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] to high: " << rValue << "!" << endl;
				return false;
			}
			break;
		case LIGHT_FIXED_WORLD_ANGLE_PHI:
		case LIGHT_FIXED_WORLD_ANGLE_THETA:
		case LIGHT_FIXED_CAM_ANGLE_PHI:
		case LIGHT_FIXED_CAM_ANGLE_THETA:
			// Limit to +/- 180
			if( rValue < -180.0 ) {
				rValue = -180.0;
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] angle min: " << rValue << endl;
			}
			if( rValue > +180.0 ) {
				rValue = +180.0;
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] angle max: " << rValue << endl;
			}
			break;
		case FOG_LINEAR_START:
		case FOG_LINEAR_END:
			break;
		case MATERIAL_SHININESS:
			rValue = exp( rValue ) - 1.0;
			if( rValue < 0.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] MATERIAL_SHININESS to low: " << rValue << endl;
				return false;
			}
			if( rValue > 128.0 ) {
				cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] MATERIAL_SHININESS to high: " << rValue << endl;
				return false;
			}
			break;
		case VIDEO_DURATION:
		case VIDEO_FRAMES_PER_SEC:
		case VIDEO_SLOW_STARTSTOP:
			//! \todo implent check for VIDEO_* parameters.
			break;
		case LIGHT_STEPPING:
		case SPHERICAL_STEPPING:
		case SVG_SCALE:
		case ROTATION_STEP:
		case RULER_HEIGHT:
		case RULER_WIDTH:
		case RULER_UNIT:
			// no negative nor zero values permitted
			if( rValue <= 0.0 ) {
				return false;
			}
			break;
		case RULER_UNIT_TICKS:
			// no negative permitted
			if( rValue < 0.0 ) {
				return false;
			}
			break;
		case ORTHO_SHIFT_HORI:
		case ORTHO_SHIFT_VERT:
			// Nothing to do
			break;
		case HIGHDPI_ZOOM_FACTOR:
			if( rValue <= 0.0 ) {
				return false;
			}
			break;
		case PARAMS_FLT_COUNT:
		default:
			std::cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] unknown paramNr: " << rParamID << " val: " << rValue << std::endl;
			break;
	}
	mParamFlt[rParamID] = rValue;
	return true;
}

//! Fetch the parameters string
//! @returns true when the flag was changed. false otherwise.
bool MeshWidgetParams::getParamStringMeshWidget( eParamStr rParamID, string* rString ) const {
	//! Returns the setting for light, material, etc.
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	// Sanity check:
	if( rString == nullptr ) {
		cerr << "[MeshWidgetParams::" << __FUNCTION__ << "] ERROR: NULL pointer given!" << endl;
		return false;
	}
	(*rString) = mParamStr[rParamID];
	return true;
}

//! Set the parameters string
//! @returns true when the flag was changed. false otherwise.
bool MeshWidgetParams::setParamStringMeshWidget( eParamStr rParamID, const string& rString ) {
	//! Sets the parameters for ambient light, materials, etc.
	//! Ignores values out of range.
#ifdef DEBUG_SHOW_ALL_METHOD_CALLS
	cout << __PRETTY_FUNCTION__ << endl;
#endif
	// Prevent infinite loops:
	if( rString == mParamStr[rParamID] ) {
		return false;
	}
	// Checks:
	switch( rParamID ) {
		case FILENAME_EXPORT_VR:
			//! \todo implent check for parameters.
			break;

		case INKSCAPE_COMMAND:
		case PDF_LATEX_COMMAND:
		case PDF_VIEWER_COMMAND:
		case PYTHON3_COMMAND:
			break;
		default:
			cerr << "[MeshWidget::" << __FUNCTION__ << "] unknown paramNr: " << rParamID << " string: " << rString << endl;
	}
	mParamStr[rParamID] = rString;
	return true;
}

//! Set all Paramters.
bool MeshWidgetParams::setParamAllMeshWidget( const MeshWidgetParams& rParams ) {
	for( unsigned long i=0; i<PARAMS_FLAG_COUNT; i++ ) {
		bool tempFlag;
		rParams.getParamFlagMeshWidget( (eParamFlag)i, &tempFlag );
		setParamFlagMeshWidget( (eParamFlag)i, tempFlag );
	}
	for( unsigned long i=0; i<PARAMS_INT_COUNT; i++ ) {
		int tempInt;
		rParams.getParamIntegerMeshWidget( (eParamInt)i, &tempInt );
		setParamIntegerMeshWidget( (eParamInt)i, tempInt );
	}
	for( unsigned long i=0; i<PARAMS_FLT_COUNT; i++ ) {
		double tempFlt;
		rParams.getParamFloatMeshWidget( (eParamFlt)i, &tempFlt );
		setParamFloatMeshWidget( (eParamFlt)i, tempFlt );
	}
	for( unsigned long i=0; i<PARAMS_STR_COUNT; i++ ) {
		std::string tempStr;
		rParams.getParamStringMeshWidget( (eParamStr)i, &tempStr );
		setParamStringMeshWidget( (eParamStr)i, tempStr );
	}
	return( true );
}

//! Determine the offset within the viewport
//! depending on the MeshWidgetParams::GRID_CENTER_POSITION
//!
//! @returns false in case of an error. True otherwise.
bool MeshWidgetParams::getGridCenterPosOffsets(
                double& rXOffset,
                double& rYOffset
) {
	int centerPos = 0;
	this->getParamIntegerMeshWidget( MeshWidgetParams::GRID_CENTER_POSITION, &centerPos );
	rXOffset = 0.0;
	rYOffset = 0.0;
	switch (centerPos) {
		case 1:	//TL
			rXOffset =  1.0;
			rYOffset = -1.0;
			break;
		case 2:	//T
			rYOffset = -1.0;
			break;
		case 3: //TR
			rXOffset = -1.0;
			rYOffset = -1.0;
			break;
		case 4: //L
			rXOffset =  1.0;
			break;
		case 5: //R
			rXOffset = -1.0;
			break;
		case 6:	//BL
			rXOffset =  1.0;
			rYOffset =  1.0;
			break;
		case 7:	//B
			rYOffset =  1.0;
			break;
		case 8: //BR
			rXOffset = -1.0;
			rYOffset =  1.0;
			break;
		default:	//Center, nothing to do
			break;
	}
	return( true );
}
