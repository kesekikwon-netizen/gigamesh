#version 150
uniform mat4 transformMat;

// +++ Vertex buffers -- this corresponds to MeshGL::grVertexElmentBasic
in vec3  position;
in vec3  vNormal;
in vec4  vColor;
in float vFuncVal;
// +++ Vertex buffers -- this corresponds to MeshGL::grVertexStripeElment
in float vLabelID; // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
in float vFlags;   // this should be UINT, but thanks to fixed normalization this does not work -- see: http://qt-project.org/forums/viewthread/38929
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

void main() {
    gl_Position = transformMat * vec4(position, 1.0f);
}