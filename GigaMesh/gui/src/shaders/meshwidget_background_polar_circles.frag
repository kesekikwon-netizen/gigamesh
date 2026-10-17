#version 150

// Circle properties:
uniform float uGridSpace1 =  1.0;        // Spacing of polar circles (No.   I)
uniform float uGridSpace2 =  5.0;        // Spacing of polar circles (No.  II)
uniform float uGridSpace3 = 10.0;        // Spacing of polar circles (No. III)
uniform float uGridWidth1 =  1.6;        // Width of polar circle no.   I
uniform float uGridWidth2 =  1.7;        // Width of polar circle no.  II
uniform float uGridWidth3 =  1.8;        // Width of polar circle no. III
// Colors for the three circles:
uniform vec4  uLineColor1 = vec4( 215.0/255.0, 187.0/255.0, 190.0/255.0, 1.0 );
uniform vec4  uLineColor2 = vec4( 167.0/255.0, 128.0/255.0, 131.0/255.0, 1.0 );
uniform vec4  uLineColor3 = vec4( 119.0/255.0,  70.0/255.0,  66.0/255.0, 1.0 );


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

// Concentric circles:
//-------------------------------
void main(void) {
	// Defaults:
	vec4  outputColor = vec4( 1.0, 1.0, 1.0, 0.0 ); // White with full transparency for the background.
	float pi = 3.1415926535897932384626;
	float gridDeltaX = dFdx( gridPos.x );
	float gridDeltaY = dFdy( gridPos.y );

	// Radius:
	float gridRadius = length( gridPos );
	float gridDeltaRadius = length( vec2( gridDeltaX, gridDeltaY ) );

	grGridSettings circleSettings;
	circleSettings.mGridOffset         = 0.0;
	circleSettings.mGridMinSpace       = 8.0;

	// Line No. I
	circleSettings.mGridDist           = uGridSpace1;
	circleSettings.mGridLinePixelWidth = uGridWidth1;
	circleSettings.mGridLineColor      = uLineColor1;
	outputColor = gridLineColor( gridRadius, gridDeltaRadius, outputColor, circleSettings );

	// Line No. II
	circleSettings.mGridDist           = uGridSpace2;
	circleSettings.mGridLinePixelWidth = uGridWidth2;
	circleSettings.mGridLineColor      = uLineColor2;
	outputColor = gridLineColor( gridRadius, gridDeltaRadius, outputColor, circleSettings );

	// Line No. III
	circleSettings.mGridDist           = uGridSpace3;
	circleSettings.mGridLinePixelWidth = uGridWidth3;
	circleSettings.mGridLineColor      = uLineColor3;
	outputColor = gridLineColor( gridRadius, gridDeltaRadius, outputColor, circleSettings );

	// Output the final color to the pipline.
	fragColor = outputColor;
}
