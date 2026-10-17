#version 150

//=============================================
// Highlight center:
//---------------------------------------------
// Line  IV = to mark the central pixel (quick and dirty using a 1.000 mm grid)
//---------------------------------------------
uniform float uAngleRotateRaster = 0.00; // Rotation for all lines in radiant
// Line properties - spacing:
uniform float uGridSpace4 = 1000.0;      // Spacing of grid lines (No.  IV)
// Line properties - width:
uniform float uGridWidth4 =  3.2;        // Width of grid line no.  IV
// Colors for the three lines:
uniform vec4  uLineColor4 = vec4(  71.0/255.0,  11.0/255.0,   6.0/255.0, 1.0 );  // Line no.  IV - Hex: 470b06ff (RGBA)
uniform vec4  uLineColor5 = vec4( 215.0/255.0, 215.0/255.0,   0.0/255.0, 1.0 );  // Line no.  IV - Hex: ffff00ff (RGBA)
//=============================================
uniform float uHighlightDepth = 0.0;

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

	// Highlight central pixel
	gl_FragDepth = gl_FragCoord.z * uHighlightDepth;

	// Line No. IV - extra cross-hair
	rectGridSettings.mGridDist           = uGridSpace4;
	rectGridSettings.mGridLinePixelWidth = uGridWidth4;
	rectGridSettings.mGridLineColor      = uLineColor4;
	outputColor = gridLineColor( posRotX, deltaRotX, outputColor, rectGridSettings );
	outputColor = gridLineColor( posRotY, deltaRotY, outputColor, rectGridSettings );

	// Additional cross-hair
	float xPosPow2 = gridPos.x * gridPos.x / ( gridDeltaX * gridDeltaX );
	float yPosPow2 = gridPos.y * gridPos.y / ( gridDeltaY * gridDeltaY );
	float radiusPow2 = 10.0 * 10.0;
	if( xPosPow2 + yPosPow2 < radiusPow2 ) {
		if( ( gridPos.x > -gridDeltaX*3.0 ) && ( gridPos.x < +gridDeltaX*3.0 ) ){
			outputColor = uLineColor5;
			//outputColor = vec4( 1.0, 1.0, 0.0, 1.0 );
			gl_FragDepth = 0.0;
		}
		if( ( gridPos.y > -gridDeltaY*3.0 ) && ( gridPos.y < +gridDeltaY*3.0 ) ){
			outputColor = uLineColor5;
			//outputColor = vec4( 1.0, 1.0, 0.0, 1.0 );
			gl_FragDepth = 0.0;
		}
		if( ( gridPos.x > -gridDeltaX*1.0 ) && ( gridPos.x < +gridDeltaX*1.0 ) ){
			outputColor = uLineColor4;
			gl_FragDepth = 0.0;
		}
		if( ( gridPos.y > -gridDeltaY*1.0 ) && ( gridPos.y < +gridDeltaY*1.0 ) ){
			outputColor = uLineColor4;
			gl_FragDepth = 0.0;
		}
	}

	// Output the final color to the pipline.
	fragColor = outputColor;
}
