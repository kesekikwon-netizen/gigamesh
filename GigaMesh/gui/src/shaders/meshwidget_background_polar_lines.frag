#version 150

uniform float uAngleRotateRaster = 0.00; // Rotation for all lines
// Line properties:
uniform float uGridSpace1 =  1.00;       // Spacing of grid lines (No.   I)  e.g. 11.25
uniform float uGridSpace2 =  5.00;       // Spacing of grid lines (No.  II)  e.g. 22.50
uniform float uGridSpace3 = 10.00;       // Spacing of grid lines (No. III)  e.g. 45.00
uniform float uGridWidth1 =  1.6;        // Width of grid line no.   I
uniform float uGridWidth2 =  1.7;        // Width of grid line no.  II
uniform float uGridWidth3 =  1.8;        // Width of grid line no. III
// Colors for the three lines:
uniform vec4  uLineColor1 = vec4( 215.0/255.0, 187.0/255.0, 190.0/255.0, 1.0 );
uniform vec4  uLineColor2 = vec4( 167.0/255.0, 128.0/255.0, 131.0/255.0, 1.0 );
uniform vec4  uLineColor3 = vec4( 119.0/255.0,  70.0/255.0,  66.0/255.0, 1.0 );

in vec2  gridPos;              // Interpolated position of the grid i.e. pixel coordinates converted to world coordinates
in vec4  ecPosition;         // Interpolated position of the fragment in eye coordinates.

// Output i.e. color of the fragment
out vec4 fragColor;

struct grGridSettings {
	float mGridDist;
	float mGridOffset;
	float mGridMinSpace;
	float mGridLinePixelWidth;
	vec4  mGridLineColor;
};

// similar to gridLineColor( ... ) using polar coordinates to draw its axis
// gridPhi <-> funcVal | gridRadius <-> funcValDelta | Extra: gridDeltaRadius
vec4 gridLineColorPolar( float gridPhi, float gridRadius, float gridDeltaRadius, vec4 outputColor, grGridSettings rSettings ) { 
	// minimum spacing: UNTESTED!
	//if( funcValDelta >= rSettings.gridMinSpace*rSettings.pixelWidth ) {
	//	return outputColor;
	//}

	float gridDistAngleMod = mod(gridPhi + rSettings.mGridOffset, rSettings.mGridDist);
	float gridRemainder = sin( rSettings.mGridDist- gridDistAngleMod )*gridRadius;
	float gridLinePixelWidthHalf = rSettings.mGridLinePixelWidth/2.0;
	if( gridRemainder < gridDeltaRadius*(gridLinePixelWidthHalf-1.0) ) {
		outputColor = rSettings.mGridLineColor;
	} else if( gridRemainder < gridDeltaRadius*(gridLinePixelWidthHalf) ) {
		float gridRemainderWeight = (gridDeltaRadius*(gridLinePixelWidthHalf)-gridRemainder)/gridDeltaRadius;
		outputColor = mix( outputColor, rSettings.mGridLineColor, gridRemainderWeight );
	}
	//someSettingsAngular.mGridLineColor = vec4( 0.0, 1.0, 0.0, 1.0 );
	gridRemainder = sin( gridDistAngleMod )*gridRadius;
	if( gridRemainder < gridDeltaRadius*(gridLinePixelWidthHalf-1.0) ) {
		outputColor = rSettings.mGridLineColor;
	} else if( gridRemainder < gridDeltaRadius*(gridLinePixelWidthHalf) ) {
		float gridRemainderWeight = (gridDeltaRadius*(gridLinePixelWidthHalf)-gridRemainder)/gridDeltaRadius;
		outputColor = mix( outputColor, rSettings.mGridLineColor, gridRemainderWeight );
	}

	return outputColor;
}


// Lines for polar grid:
//-------------------------------
void main(void) {
	// Defaults:
	vec4  outputColor = vec4( 1.0, 1.0, 1.0, 0.0 ); // White with full transparency for the background.
	float pi = 3.1415926535897932384626;
	float gridDeltaX = dFdx( gridPos.x );
	float gridDeltaY = dFdy( gridPos.y );

	float gridRadius = length( gridPos );
	float gridDeltaRadius = length( vec2( gridDeltaX, gridDeltaY ) );
	float gridPhi = atan( gridPos.y / gridPos.x );

	// Settings for polar lines
	grGridSettings settingsPolarLines;
	settingsPolarLines.mGridOffset         = 0.0;
	settingsPolarLines.mGridMinSpace       = 9.0;

	// Line No. I
	if( gridRadius > 50.0*gridDeltaRadius ) {
		settingsPolarLines.mGridDist           = uGridSpace1 * pi / 180.0;
		settingsPolarLines.mGridLinePixelWidth = uGridWidth1;
		settingsPolarLines.mGridLineColor      = uLineColor1;
		outputColor = gridLineColorPolar( gridPhi, gridRadius, gridDeltaRadius, outputColor, settingsPolarLines );
	}

	// Line No. II
	if( gridRadius > 25.0*gridDeltaRadius ) {
		settingsPolarLines.mGridDist           = uGridSpace2 * pi / 180.0;
		settingsPolarLines.mGridLinePixelWidth = uGridWidth2;
		settingsPolarLines.mGridLineColor      = uLineColor2;
		outputColor = gridLineColorPolar( gridPhi, gridRadius, gridDeltaRadius, outputColor, settingsPolarLines );
	}

	// Line No. III
	settingsPolarLines.mGridDist           = uGridSpace3 * pi / 180.0;
	settingsPolarLines.mGridLinePixelWidth = uGridWidth3;
	settingsPolarLines.mGridLineColor      = uLineColor3;
	outputColor = gridLineColorPolar( gridPhi, gridRadius, gridDeltaRadius, outputColor, settingsPolarLines );

	// Output the final color to the pipline.
	fragColor = outputColor;
}
