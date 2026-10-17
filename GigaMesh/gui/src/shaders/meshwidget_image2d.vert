#version 150

uniform float uDepthPos = 0.0;    // Offset for the default range for the frustum z: +1.0-Epsilon (Far) to -1.0 (Near)

// +++ Vertex buffers
in vec2 vertPosition;        // Positions of the veritces defining the background - their typicall position is equal to the four corners of the viewport
in vec2 textureCoords;       // Texture Coordinates.

out vec4 ecPosition;         // Interpolated position of the fragment in eye coordinates.
out vec2 texCoord;           // Texture coordiantes -- passed thru.

void main(void) {
	texCoord = textureCoords;
	ecPosition = vec4( vertPosition, uDepthPos+0.9999, 1.0 );
	gl_Position = ecPosition;
}
