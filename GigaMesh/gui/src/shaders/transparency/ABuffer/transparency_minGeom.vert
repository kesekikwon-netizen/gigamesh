#version 150
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

// +++ Clipping
uniform vec4 uClipPlane0 = vec4( 0.0, -1.0, 0.0, 0.0 ); // Classic clipping plane, provided by the plane of the Mesh.
uniform vec3 uClipBefore = vec3( 0.0, 0.0, 0.0 );       // Point in world coordinates sed when a single primitve is selected, than everything in front of it is clipped.

in vec3  position;
in vec3  vNormal;
in vec4  vColor;
in float vFuncVal;
// +++ Vertex buffers -- this corresponds to MeshGL::grVertexStripeElment
in float vLabelID; // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
in float vFlags;   // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929

void main(void)
{
    highp mat4  modelViewMat  = modelview * uModelViewExtra;
          float scaleRadial   = uScaleRadialBottom * ( 0.5 - position.z ) + uScaleRadialTop * ( 0.5 + position.z );
          vec3  modelPosition = vec3( position.x*scaleRadial, position.y*scaleRadial, position.z * uScaleHeight );

          vec4 ec_pos        = modelViewMat * vec4( modelPosition.xyz, 1.0 );                  // Transform the vertex position into the eye coordinate system.
          gl_Position   = projection * ec_pos;

          gl_ClipDistance[0] = dot( uClipPlane0, uModelViewExtra * vec4( position, 1.0 ) );     //  using "ec_pos" instead of "vec4( position, 1.0 )" will clip in view space!
          gl_ClipDistance[2] = ( modelview * vec4( uClipBefore, 1.0 ) ).z - ec_pos.z;
}
