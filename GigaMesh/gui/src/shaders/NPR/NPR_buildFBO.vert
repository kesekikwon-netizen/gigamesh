#version 330
//#extension GL_ARB_shader_bit_encoding : enable

// +++ Homogenous matrices for camera orientation and projection:
uniform highp mat4 modelview;
uniform highp mat4 projection;

uniform highp mat4 uModelViewExtra = mat4( 1.0, 0.0, 0.0, 0.0,
                                           0.0, 1.0, 0.0, 0.0,
                                           0.0, 0.0, 1.0, 0.0,
                                           0.0, 0.0, 0.0, 1.0
                                         );

uniform float uScaleHeight       = 1.0;
uniform float uScaleRadialBottom = 1.0;
uniform float uScaleRadialTop    = 1.0;

// +++ Vertex buffers -- this corresponds to MeshGL::grVertexElmentBasic
in vec3  position;
in vec3  vNormal;
in vec4  vColor;
in float vFuncVal;
/*
// +++ Vertex buffers -- this corresponds to MeshGL::grVertexStripeElment
in float vLabelID; // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
in float vFlags;   // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
*/
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Direction of the light fixed in relation to the camera:
uniform vec3 uLightDirectionFixedCamera = vec3( 0.0, 0.0, 1.0 );
uniform vec3 uLightDirectionFixedWorld  = vec3( 0.0, 0.0, 1.0 );

// +++ Clipping
/*
uniform vec4 uClipPlane0 = vec4( 0.0, -1.0, 0.0, 0.0 ); // Classic clipping plane, provided by the plane of the Mesh.
uniform vec3 uClipBefore = vec3( 0.0, 0.0, 0.0 );       // Point in world coordinates sed when a single primitve is selected, than everything in front of it is clipped.
*/

// +++ Function value of the vertex - values to compute the texture coordinate for the fragment shader:
uniform bool  uFuncValInvert = false;
uniform float uFuncValMin;
uniform float uFuncValMax;
uniform float uFuncValColorMap  = 1.0;
uniform bool  uFuncValRepeat    = false;
uniform float uFuncValIntervall = 10.0;
uniform float uFuncValLogGamma  =  1.0;


// +++ Values to be passed on to the geometry shader:
out struct grVertex {
        vec4  ec_pos;        // eye coordinate position
        vec3  normal_interp; // Normal vector, which will be interpolated
	vec4  vColor;
        vec3  FixedCam_L;
        vec3  FixedWorld_L;
        vec3 FixedCam_halfVector;
        vec3 FixedWorld_halfVector;
	float funcValNormalized;
} oVertex;

void main(void)
{
    highp mat4  modelViewMat  = modelview * uModelViewExtra;
          float scaleRadial   = uScaleRadialBottom * ( 0.5 - position.z ) + uScaleRadialTop * ( 0.5 + position.z );
          vec3  modelPosition = vec3( position.x*scaleRadial, position.y*scaleRadial, position.z * uScaleHeight );

          oVertex.ec_pos        = modelViewMat * vec4( modelPosition.xyz, 1.0 );                  // Transform the vertex position into the eye coordinate system.
          oVertex.normal_interp = normalize( modelview * vec4( vNormal, 0.0 ) ).xyz;  // Transform the vertex normal into eye coordinates.
          vec3 E = normalize( -oVertex.ec_pos.xyz );                                  // "eye" vector, which is the vector from the vertex's eye-space position to the origin.
  // -------------------------------------------------------------------------------------------------------------------------------------------------------------
  // ---- Light direction fixed to the camera -----
          oVertex.FixedCam_L = normalize( uLightDirectionFixedCamera );
          oVertex.FixedCam_halfVector = normalize( oVertex.FixedCam_L + E );                  // "half vector"  which is the normalized vector half-way between the light and eye vectors.
  // -------------------------------------------------------------------------------------------------------------------------------------------------------------
  // ---- Light direction fixed to the world/object -----
          oVertex.FixedWorld_L = normalize( modelview * vec4( uLightDirectionFixedWorld, 0.0 ) ).xyz;
          oVertex.FixedWorld_halfVector = normalize( oVertex.FixedWorld_L + E );              // "half vector"  which is the normalized vector half-way between the light and eye vectors.

	  oVertex.funcValNormalized = ( vFuncVal - uFuncValMin ) / ( uFuncValMax - uFuncValMin );

	  oVertex.vColor = vColor;

          gl_Position   = projection * oVertex.ec_pos;
}
