// Helpfull hints:
// http://www.opengl.org/discussion_boards/showthread.php/165360-Access-to-triangle-normals-rather-than-vertex?p=1169422&viewfull=1#post1169422
// http://stackoverflow.com/a/16368768
// To understand how these instructions work, it helps to understand the basic execution architecture of GPUs and how fragment programs map to that architecture.
// GPUs run a bunch of threads in 'lock-step' over the same program, which each thread having its own set of registers. So it fetches an instruction, then executes that instruction N times, once for each running thread. To deal with conditional branches and such, they also have an 'active mask' for the currently running group of threads. Threads that are not active in the mask don't actually run (so their registers don't change). Whenever there is a conditional branch or join (branch target) the thread mask is changed appropriately.
// Now when a fragment program is run, the fragments to be run are arranged into "quads" -- 2x2 squares of 4 pixels that always run together in a thread group. Each thread in the group knows its own pixel coordinate, and can easily find the coordinate of the adjacent pixel in the quad by flipping the lowest bit of the x (or y) coord.
// When the GPU executes a DDX or DDY instruction, what happens is that it peeks at the registers for the thread for the adjacent pixel and does a subtract with the value from the current pixel -- subtracting the value for the higher coordinate (lowest bit 1) from the lower (lowest bit 0).
// This has implications if you use dFdx or dFdy in a conditional branch -- if one of the threads in a quad is active while the other is not, the GPU will still look at the register of the inactive thread, which might have any old value in it, so the result could be anything.

// Fog with GLSL:
// http://www.mbsoftworks.sk/index.php?page=tutorials&series=1&tutorial=15

#version 150

// +++ Vertex buffers
in vec3 position;
in vec3 vNormal;
// +++ Homogenous matrices for camera orientation and projection:
uniform highp mat4 modelview;
uniform highp mat4 projection;
// +++ Direction of the light fixed in relation to the camera:
uniform vec3 LightDirectionFixedCam   = vec3( 0.0, 0.0, -1.0 );
uniform vec3 LightDirectionFixedWorld = vec3( 0.0, 0.0, -1.0 );
// +++ Values to be passed on to the fragment shader:
out vec4 ec_pos; // eye coordinate position to be used for on-the-fly-computation of a triangles normal within the fragment shader.
out vec3 normal_interp;
out vec3 FixedCam_halfVector,FixedCam_L;
out vec3 FixedWorld_halfVector,FixedWorld_L;
// -----------------------------------------------
void main(void) {
	ec_pos        = modelview * vec4( position, 1.0 );                  // Transform the vertex position into the eye coordinate system.
	normal_interp = normalize( modelview * vec4( vNormal, 0.0 ) ).xyz;  // Transform the vertex normal into eye coordinates.
	vec3 E = normalize( -ec_pos.xyz );                                  // "eye" vector, which is the vector from the vertex's eye-space position to the origin.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Light direction fixed to the camera -----
	FixedCam_L = normalize( LightDirectionFixedCam );
	FixedCam_halfVector = normalize( FixedCam_L + E );                  // "half vector"  which is the normalized vector half-way between the light and eye vectors.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Light direction fixed to the world/object -----
	FixedWorld_L = normalize( modelview * vec4( LightDirectionFixedWorld, 0.0 ) ).xyz;
	FixedWorld_halfVector = normalize( FixedWorld_L + E );              // "half vector"  which is the normalized vector half-way between the light and eye vectors.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
	gl_Position = projection * ec_pos;
}
