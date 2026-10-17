#version 150

//=============================================
// Gridlines:
//---------------------------------------------
// Line   I = typically  1 mm grid
// Line  II = typically  5 mm grid
// Line III = typically 10 mm grid
//---------------------------------------------
uniform float uAngleRotateRaster = 0.00; // Rotation for all lines in radiant
// Line properties - spacing:
uniform float uGridSpace1 =  1.0;        // Spacing of grid lines (No.   I)
uniform float uGridSpace2 =  5.0;        // Spacing of grid lines (No.  II)
uniform float uGridSpace3 = 10.0;        // Spacing of grid lines (No. III)
// Line properties - width:
uniform float uGridWidth1 =  1.6;        // Width of grid line no.   I
uniform float uGridWidth2 =  1.7;        // Width of grid line no.  II
uniform float uGridWidth3 =  1.8;        // Width of grid line no. III
// Colors for the three lines:
uniform vec4  uLineColor1 = vec4( 215.0/255.0, 187.0/255.0, 190.0/255.0, 1.0 );  // Line no.   I - Hex: d7bbbeff (RGBA)
uniform vec4  uLineColor2 = vec4( 167.0/255.0, 128.0/255.0, 131.0/255.0, 1.0 );  // Line no.  II - Hex: a78083ff (RGBA)
uniform vec4  uLineColor3 = vec4( 119.0/255.0,  70.0/255.0,  66.0/255.0, 1.0 );  // Line no. III - Hex: 774642ff (RGBA)
//=============================================

in vec2  gridPos;              // Interpolated position of the grid i.e. pixel coordinates converted to world coordinates
//in vec4  ecPosition;         // Interpolated position of the fragment in eye coordinates.

// Output i.e. color of the fragment
out vec4 fragColor;

struct grGridSettings {
	float mGridDist;
	float mGridOffset;
	float mGridMinSpace;
	float mGridLinePixelWidth;
	vec4  mGridLineColor;
};

vec4 gridLineColor( float funcVal, float funcValDelta, vec4 outputColor, grGridSettings rSettings ) {
	if( funcValDelta >= rSettings.mGridDist/rSettings.mGridMinSpace ) {
		return outputColor;
	}

	float gridModIsoDist = mod(funcVal + rSettings.mGridOffset, rSettings.mGridDist);
	float gridRemainder = rSettings.mGridDist -gridModIsoDist;
	float gridLinePixelWidthHalf = rSettings.mGridLinePixelWidth/2.0;
	if( gridRemainder < funcValDelta*(gridLinePixelWidthHalf-1.0) ) {
		outputColor = rSettings.mGridLineColor;
	} else if( gridRemainder < funcValDelta*(gridLinePixelWidthHalf) ) {
		float gridRemainderWeight = (funcValDelta*(gridLinePixelWidthHalf)-gridRemainder)/funcValDelta;
		outputColor = mix( outputColor, rSettings.mGridLineColor, gridRemainderWeight );
	}

	// rSettings.mGridLineColor = vec4( 0.5, 0.5, 0.5, 1.0 ); // visual debugging
	gridRemainder = gridModIsoDist;
	if( gridRemainder < funcValDelta*(gridLinePixelWidthHalf-1.0) ) {
		outputColor = rSettings.mGridLineColor;
	} else if( gridRemainder < funcValDelta*(gridLinePixelWidthHalf) ) {
		float gridRemainderWeight = (funcValDelta*(gridLinePixelWidthHalf)-gridRemainder)/funcValDelta;
		outputColor = mix( outputColor, rSettings.mGridLineColor, gridRemainderWeight );
	}

	return outputColor;
}

// Orthogonal grid with rotation:
//-------------------------------
void main(void) {
	// Defaults:
	vec4  outputColor = vec4( 1.0, 1.0, 1.0, 0.0 ); // White with full transparency for the background.
	float pi = 3.1415926535897932384626;
	float gridDeltaX = dFdx( gridPos.x );
	float gridDeltaY = dFdy( gridPos.y );

	float angleRotateRaster = uAngleRotateRaster;
	float posRotX   = gridPos.x*sin(angleRotateRaster) - gridPos.y*cos(angleRotateRaster);
	float deltaRotX = length( vec2( gridDeltaX*sin(angleRotateRaster), gridDeltaY*cos(angleRotateRaster) ) );
	angleRotateRaster += pi/2.0;
	float posRotY   = gridPos.x*sin(angleRotateRaster) - gridPos.y*cos(angleRotateRaster);
	float deltaRotY = length( vec2( gridDeltaX*sin(angleRotateRaster), gridDeltaY*cos(angleRotateRaster) ) );

	grGridSettings rectGridSettings;
	rectGridSettings.mGridOffset         = 0.0;
	rectGridSettings.mGridMinSpace       = 8.0;

	// Line No. I
	rectGridSettings.mGridDist           = uGridSpace1;
	rectGridSettings.mGridLinePixelWidth = uGridWidth1;
	rectGridSettings.mGridLineColor      = uLineColor1;
	outputColor = gridLineColor( posRotX, deltaRotX, outputColor, rectGridSettings );
	outputColor = gridLineColor( posRotY, deltaRotY, outputColor, rectGridSettings );

	// Line No. II
	rectGridSettings.mGridDist           = uGridSpace2;
	rectGridSettings.mGridLinePixelWidth = uGridWidth2;
	rectGridSettings.mGridLineColor      = uLineColor2;
	outputColor = gridLineColor( posRotX, deltaRotX, outputColor, rectGridSettings );
	outputColor = gridLineColor( posRotY, deltaRotY, outputColor, rectGridSettings );

	// Line No. III
	rectGridSettings.mGridDist           = uGridSpace3;
	rectGridSettings.mGridLinePixelWidth = uGridWidth3;
	rectGridSettings.mGridLineColor      = uLineColor3;
	outputColor = gridLineColor( posRotX, deltaRotX, outputColor, rectGridSettings );
	outputColor = gridLineColor( posRotY, deltaRotY, outputColor, rectGridSettings );

	// Vertical grid lines (without rotation):
	//outputColor = gridLineColor( gridPos.x, gridDeltaX, outputColor, rectGridSettings );
	// Horizontal grid lines (without rotation):
	//outputColor = gridLineColor( gridPos.y, gridDeltaY, outputColor, rectGridSettings );

	// Output the final color to the pipline.
	fragColor = outputColor;
}
