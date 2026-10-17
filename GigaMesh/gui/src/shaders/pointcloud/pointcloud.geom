#version 150

layout (points) in;
layout (points, max_vertices = 1) out; //generate one point per point

uniform int reductionFactor = 1;

// +++ Values to be passed from the vertex.
in struct grVertex {
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
} oVertex[];

out grVertex gVertex;

void main(void)
{
    gl_Position                   = gl_in[0].gl_Position; // + vec4( uExplodeFactor * oVertex[i].normal_interp, 0.0);
    gl_ClipDistance[0]            = gl_in[0].gl_ClipDistance[0];
    gl_ClipDistance[1]            = gl_in[0].gl_ClipDistance[1];
    gl_ClipDistance[2]            = gl_in[0].gl_ClipDistance[2];
    // +++ Pass on data
    gVertex.ec_pos                = oVertex[0].ec_pos;
    gVertex.normal_interp         = oVertex[0].normal_interp;
    gVertex.FixedCam_halfVector   = oVertex[0].FixedCam_halfVector;
    gVertex.FixedCam_L            = oVertex[0].FixedCam_L;
    gVertex.FixedWorld_halfVector = oVertex[0].FixedWorld_halfVector;
    gVertex.FixedWorld_L          = oVertex[0].FixedWorld_L;
    // +++ Color of the vertex
    gVertex.vertexColor           = oVertex[0].vertexColor;
    // +++ Function value of the vertex passed to the fragment shader:
    gVertex.vertexFuncVal         = oVertex[0].vertexFuncVal;
    gVertex.vertexFuncValTexCoord = oVertex[0].vertexFuncValTexCoord;
    // +++ Labels
    gVertex.labelNr               = oVertex[0].labelNr;
    gVertex.flagNoLabel           = oVertex[0].flagNoLabel;

    if(gl_PrimitiveIDIn % reductionFactor == 0)
    {
	EmitVertex();
	EndPrimitive();
    }
}
