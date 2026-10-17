#version 150
//#extension GL_ARB_shader_bit_encoding : enable

// +++ Homogenous matrices for camera orientation and projection:
uniform highp mat4 modelview;
uniform highp mat4 projection;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Vertex buffers -- this corresponds to MeshGL::grVertexStripeElment
in vec3  position;
in vec3  vNormal;
in vec4  vColor;
in float vRadius;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Direction of the light fixed in relation to the camera:
uniform vec3 LightDirectionFixedCam   = vec3( 0.0, 0.0, 1.0 );
uniform vec3 LightDirectionFixedWorld = vec3( 0.0, 0.0, 1.0 );

// +++ Clipping
uniform vec4 uClipPlane0 = vec4( 0.0, -1.0, 0.0, 0.0 ); // Classic clipping plane, provided by the plane of the Mesh.
uniform vec3 uClipBefore = vec3( 0.0, 0.0, 0.0 );       // Point in world coordinates sed when a single primitve is selected, than everything in front of it is clipped.

// +++ Values to be passed on to the geometry shader:
out struct grVertex {
	vec4  ec_pos;        // eye coordinate position to be used for on-the-fly-computation of a triangles normal within the fragment shader.
	vec3  normal_interp; // Normal vector, which will be interpolated
	vec3  FixedCam_halfVector,FixedCam_L;
	vec3  FixedWorld_halfVector,FixedWorld_L;
	//+++ Color of the vertex
	vec4  vertexColor;
	// +++ Function value of the vertex passed to the fragment shader:
	float vertexRadius;
	vec3  normalCutPlane;
} oVertex;

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

void main(void) {
	oVertex.ec_pos        = modelview * vec4( position, 1.0 );                  // Transform the vertex position into the eye coordinate system.
	//oVertex.normal_interp = normalize( modelview * vec4( vNormal, 0.0 ) ).xyz;  // Transform the vertex normal into eye coordinates.
	oVertex.normalCutPlane = normalize( modelview * vec4( vNormal, 0.0 ) ).xyz;
	oVertex.normal_interp = vec3( 0.0, 0.0, 1.0 );
	vec3 E = normalize( -oVertex.ec_pos.xyz );                                  // "eye" vector, which is the vector from the vertex's eye-space position to the origin.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Light direction fixed to the camera -----
	oVertex.FixedCam_L = normalize( LightDirectionFixedCam );
	oVertex.FixedCam_halfVector = normalize( oVertex.FixedCam_L + E );                  // "half vector"  which is the normalized vector half-way between the light and eye vectors.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Light direction fixed to the world/object -----
	oVertex.FixedWorld_L = normalize( modelview * vec4( LightDirectionFixedWorld, 0.0 ) ).xyz;
	oVertex.FixedWorld_halfVector = normalize( oVertex.FixedWorld_L + E );              // "half vector"  which is the normalized vector half-way between the light and eye vectors.

	gl_Position   = projection * oVertex.ec_pos;
	gl_ClipDistance[0] = dot( uClipPlane0, vec4( position, 1.0 ) );     //  using "ec_pos" instead of "vec4( position, 1.0 )" will clip in view space!
	gl_ClipDistance[2] = ( modelview * vec4( uClipBefore, 1.0 ) ).z - oVertex.ec_pos.z;
	oVertex.vertexColor   = vColor;
	oVertex.vertexRadius  = vRadius;
}
