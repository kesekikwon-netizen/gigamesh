#version 150

uniform sampler2D uLabelTexMap;  // Texturemap holding an image e.g. logo.

//in vec4  ecPosition;           // Interpolated position of the fragment in eye coordinates.
in vec2 texCoord;

// Output i.e. color of the fragment
out vec4 fragColor;

// Orthogonal grid with rotation:
//-------------------------------
void main(void) {
	// Defaults:
	//vec4  outputColor = vec4( 1.0, 1.0, 1.0, 0.0 ); // White with full transparency for the background.
	vec4 outputColor = texture( uLabelTexMap, texCoord );
	//vec4 texColor = texture( uLabelTexMap, vec2( 1.0/512.0, 34.5/512.0 ) ); // e.g. one color from a given pixel.

	// Minimize wrong values within the z-buffer, which is used for selection:
	if( outputColor.a == 0.0 ) {
		discard;
	}

	// Output the final color to the pipline.
	fragColor = outputColor;
}
