#version 150
uniform mat4 transformMat;
uniform int xResolution;
uniform int yResolution;
uniform sampler2D depths;
uniform float zTolerance;
uniform int vertIDOffset;

// +++ Vertex buffers -- this corresponds to MeshGL::grVertexElmentBasic
in vec3  position;
in vec3  vNormal;
in vec4  vColor;
in float vFuncVal;
// +++ Vertex buffers -- this corresponds to MeshGL::grVertexStripeElment
in float vLabelID; // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
in float vFlags;   // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

out float brightness;

void main() {
	vec4 clipSpacePos = transformMat * vec4(position, 1.0f);
        float depth = (texture( depths, vec2(0.5,0.5)+0.5*clipSpacePos.xy )).x;
	
	// check for occlusion and sufficient length of the normal
	if (0.5 + 0.5 * clipSpacePos.z <= depth + zTolerance && length(vNormal) >= 0.01) {
		vec4 transformedNormal = transformMat * vec4(vNormal, 0.0f);
		
		// check orientation of the normal
		if (transformedNormal.z < 0.0) {
			brightness = -transformedNormal.z / length(transformedNormal.xyz);
		} else {
			brightness = 0.0;
		}
	} else {
		brightness = 0.0;
	}
	
	gl_PointSize = 1.0;

	// position vertices at the center of individual pixels from left to right and from top to bottom starting at the top left pixel
	gl_Position = vec4 (((gl_VertexID-vertIDOffset) % xResolution + 0.5)/(0.5 * xResolution) - 1.0, ((gl_VertexID-vertIDOffset) / xResolution + 0.5)/(0.5 * yResolution) - 1.0, 0, 1);
}
