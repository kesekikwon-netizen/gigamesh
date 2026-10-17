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
in float vFuncVal;
in float vLabelID; // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
in float vFlags;   // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Direction of the light fixed in relation to the camera:
uniform vec3 LightDirectionFixedCam   = vec3( 0.0, 0.0, 1.0 );
uniform vec3 LightDirectionFixedWorld = vec3( 0.0, 0.0, 1.0 );

// +++ Clipping
uniform vec4 uClipPlane0 = vec4( 0.0, -1.0, 0.0, 0.0 ); // Classic clipping plane, provided by the plane of the Mesh.
uniform vec3 uClipBefore = vec3( 0.0, 0.0, 0.0 );       // Point in world coordinates sed when a single primitve is selected, than everything in front of it is clipped.

// +++ Function value of the vertex - values to compute the texture coordinate for the fragment shader:
uniform bool  uFuncValInvert = false;
uniform float uFuncValMin;
uniform float uFuncValMax;
uniform float uFuncValColorMap  = 1.0;
uniform bool  uFuncValRepeat    = false;
uniform float uFuncValIntervall = 10.0;

// +++ Values to be passed on to the geometry shader:
out struct grVertex {
	vec4  ec_pos;        // eye coordinate position to be used for on-the-fly-computation of a triangles normal within the fragment shader.
	vec3  normal_interp; // Normal vector, which will be interpolated
	vec3  FixedCam_halfVector,FixedCam_L;
	vec3  FixedWorld_halfVector,FixedWorld_L;
	//+++ Color of the vertex
	vec4  vertexColor;
	// +++ Function value of the vertex passed to the fragment shader:
	float vertexFuncVal;
	vec2  vertexFuncValTexCoord;
	// +++ Labels
	float labelNr;       // corresponds to vLabelID
	float flagNoLabel; 
} oVertex;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Values for point sprites
uniform float uPointSizeBase        = 20.0;
uniform float uPointSizeBaseMinimal =  5.0;
uniform float uPointShiftViewZ      =  0.0; // offset in view coordinates to prevent z-fighting.

void main(void) {
	oVertex.ec_pos        = modelview * vec4( position, 1.0 );                  // Transform the vertex position into the eye coordinate system.
	oVertex.normal_interp = normalize( modelview * vec4( vNormal, 0.0 ) ).xyz;  // Transform the vertex normal into eye coordinates.
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
	oVertex.vertexFuncVal = vFuncVal;
	oVertex.labelNr       = vLabelID;

	// Or more generally for pixel i in a N wide texture the proper texture coordinate is :=  (2i + 1)/(2N)
	float iPixel = 10.0*uFuncValColorMap+4.0;
	oVertex.vertexFuncValTexCoord.t = ( 2.0*iPixel + 1.0 )/1024.0; // Texture map with color ramps is 512x512 pixel and each colorramp is 10 pixel wide (in y).
	// Repeat color ramp, when requested:
	if( uFuncValRepeat ) {
		float funcValForTexCoords = 2.0 * mod( vFuncVal, uFuncValIntervall ) / uFuncValIntervall;
		if( funcValForTexCoords > 1.0 ) {
			funcValForTexCoords = ( -funcValForTexCoords + 2.0 );
		}
		oVertex.vertexFuncValTexCoord.s = ( 511.0 * funcValForTexCoords + 0.5 ) / 512.0;
	} else {
		oVertex.vertexFuncValTexCoord.s = ( ( 511.0 * ( vFuncVal - uFuncValMin ) ) / ( 512.0 * ( uFuncValMax - uFuncValMin ) ) ) + 0.5 / 512.0;
		if( vFuncVal <= uFuncValMin ) {
			oVertex.vertexFuncValTexCoord.s = 1.0 / 1024.0;
		}
		if( vFuncVal >= uFuncValMax ) {
			oVertex.vertexFuncValTexCoord.s = 1023.0 / 1024.0;
		}
	}
	if( uFuncValInvert ) {
		oVertex.vertexFuncValTexCoord.s = 1.0 - oVertex.vertexFuncValTexCoord.s;
	}

	// Point sprite specifics:
	gl_PointSize = uPointSizeBase * ( 1.0 - ( (gl_Position.z / gl_Position.w) + 1.0 )/2.0 ) + uPointSizeBaseMinimal;
	gl_Position.z += uPointShiftViewZ;

	// EXAMPLE for passing on flags:
	// See MeshGL::vboPrepareVerticesStriped() for the values used to pass the flags.
	if( bool( uint(vFlags) & 0x08u )  ) {
		oVertex.flagNoLabel = 1.0;
	} else {
		oVertex.flagNoLabel = 0.0;
	}
}
