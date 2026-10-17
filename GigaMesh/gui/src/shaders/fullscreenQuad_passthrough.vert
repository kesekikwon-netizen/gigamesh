#version 150

//uniform float uScaleX   = 1.0;    // Scale for x-axis -- typically mm/pixel.
//uniform float uScaleY   = 1.0;    // Scale for y-axis -- should always be the same value as uScaleX.
uniform float uDepthPos = -.5;    // Offset for the default range for the frustum z: +1.0-Epsilon (Far) to -1.0 (Near)

// +++ Vertex buffers
in vec2 vertPosition;        // Positions of the vertices defining the screenquad

out vec2 texCoord;         //texture coord

void main(void) {

        texCoord = (vertPosition + 1.0) * 0.5;
        gl_Position = vec4( vertPosition, uDepthPos, 1.0 );
}
