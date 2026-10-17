#version 150
// IMPORTANT: It isn't possible to create a program with geometry shaders that handle multiple primitive types.
// This means: out is either triangle_strip or line_strip!

layout (lines) in;
layout (triangle_strip, max_vertices = 10) out; // 4 per line, 2*3 per vertex for the normals

// +++ Homogenous matrices for camera orientation and projection:
uniform highp mat4 modelview;
uniform highp mat4 projection;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

uniform float uLineWidth = 0.2;

uniform bool  uPolyNormals  = false;
uniform float uNormalLength = 2.0;
uniform float uNormalWidth  = 0.1;

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
flat out uint gInvertColor;

// +++ Edge/Wireframe Rendering 
noperspective out vec3 vEdgeDist;              // Barycenter coordinates. REQUIRED by funcval.frag
out vec3 vBarycenter;            // normalized Barycenter coordinates
flat out vec3 vLabelNumbers;                        // vector to hold all three labelNr's to get uninterpolated result

//uniform float uExplodeFactor = 0.12;

void main(void) {
	vEdgeDist = vec3( 1.0, 1.0, 1.0 ); // REQUIRED by funcval.frag
	vBarycenter = vec3(1.0);
	vLabelNumbers = vec3(1.0);
	vec3 axisDir   = normalize( oVertex[1].ec_pos.xyz - oVertex[0].ec_pos.xyz );
	vec4 sideShift = vec4( normalize( cross( axisDir, vec3( 0.0, 0.0, 1.0 ) ) )*uLineWidth/2.0, 0.0 );

	gInvertColor = 0u;
	int i;
	for( i=0; i<gl_in.length(); i++ ) {
		// +++ Pass on data
		gVertex.ec_pos                = oVertex[i].ec_pos + sideShift;
		gVertex.normal_interp         = oVertex[i].normal_interp;
		gVertex.FixedCam_halfVector   = oVertex[i].FixedCam_halfVector;
		gVertex.FixedCam_L            = oVertex[i].FixedCam_L;
		gVertex.FixedWorld_halfVector = oVertex[i].FixedWorld_halfVector;
		gVertex.FixedWorld_L          = oVertex[i].FixedWorld_L;
		gVertex.vertexColor           = oVertex[i].vertexColor;
		// +++ Function value of the vertex passed to the fragment shader:
		gVertex.vertexFuncVal         = oVertex[i].vertexFuncVal;
		gVertex.vertexFuncValTexCoord = oVertex[i].vertexFuncValTexCoord;
		// +++ Labels
		gVertex.labelNr               = oVertex[i].labelNr;
		gVertex.flagNoLabel           = oVertex[i].flagNoLabel; 
		// According to http://www.opengl.org/wiki/Built-in_Variable_%28GLSL%29#Vertex_shader_outputs
		gl_Position                   = projection * gVertex.ec_pos;
		gl_ClipDistance[0]            = gl_in[i].gl_ClipDistance[0];
		gl_ClipDistance[1]            = gl_in[i].gl_ClipDistance[1];
		gl_ClipDistance[2]            = gl_in[i].gl_ClipDistance[2];
		// +++ DONE
		EmitVertex();
	}
	for( i=0; i<gl_in.length(); i++ ) {
		// +++ Pass on data
		gVertex.ec_pos                = oVertex[i].ec_pos - sideShift;
		gVertex.normal_interp         = oVertex[i].normal_interp;
		gVertex.FixedCam_halfVector   = oVertex[i].FixedCam_halfVector;
		gVertex.FixedCam_L            = oVertex[i].FixedCam_L;
		gVertex.FixedWorld_halfVector = oVertex[i].FixedWorld_halfVector;
		gVertex.FixedWorld_L          = oVertex[i].FixedWorld_L;
		// +++ Color of the vertex
		gVertex.vertexColor           = oVertex[i].vertexColor;
		// +++ Function value of the vertex passed to the fragment shader:
		gVertex.vertexFuncVal         = oVertex[i].vertexFuncVal;
		gVertex.vertexFuncValTexCoord = oVertex[i].vertexFuncValTexCoord;
		// +++ Labels
		gVertex.labelNr               = oVertex[i].labelNr;
		gVertex.flagNoLabel           = oVertex[i].flagNoLabel;
 		// According to http://www.opengl.org/wiki/Built-in_Variable_%28GLSL%29#Vertex_shader_outputs
		gl_Position                   = projection * gVertex.ec_pos;
		gl_ClipDistance[0]            = gl_in[i].gl_ClipDistance[0];
		gl_ClipDistance[1]            = gl_in[i].gl_ClipDistance[1];
		gl_ClipDistance[2]            = gl_in[i].gl_ClipDistance[2];
		// +++ DONE
		EmitVertex();
	}
	EndPrimitive();

	if( uPolyNormals ) {
		gInvertColor = 1u;
		for( i=0; i<gl_in.length(); i++ ) {
			// +++ Pass on data
			gVertex.ec_pos                = oVertex[i].ec_pos;
			gVertex.normal_interp         = oVertex[i].normal_interp;
			gVertex.FixedCam_halfVector   = oVertex[i].FixedCam_halfVector;
			gVertex.FixedCam_L            = oVertex[i].FixedCam_L;
			gVertex.FixedWorld_halfVector = oVertex[i].FixedWorld_halfVector;
			gVertex.FixedWorld_L          = oVertex[i].FixedWorld_L;
			gVertex.vertexColor           = oVertex[i].vertexColor;
			// +++ Function value of the vertex passed to the fragment shader:
			gVertex.vertexFuncVal         = oVertex[i].vertexFuncVal;
			gVertex.vertexFuncValTexCoord = oVertex[i].vertexFuncValTexCoord;
			// +++ Labels
			gVertex.labelNr               = oVertex[i].labelNr;
			gVertex.flagNoLabel           = oVertex[i].flagNoLabel; 
			// According to http://www.opengl.org/wiki/Built-in_Variable_%28GLSL%29#Vertex_shader_outputs
			gl_Position                   = projection * gVertex.ec_pos;
			gl_ClipDistance[0]            = gl_in[i].gl_ClipDistance[0];
			gl_ClipDistance[1]            = gl_in[i].gl_ClipDistance[1];
			gl_ClipDistance[2]            = gl_in[i].gl_ClipDistance[2];
			// +++ DONE
			EmitVertex();
			
			vec3 normal     = normalize( oVertex[i].normal_interp ) * uNormalLength;
			vec3 diagNormal = cross( normal, vec3( 0.0, 0.0, uNormalWidth ) );

			vec4 sideShift  = vec4( normal + diagNormal, 0.0 );
			gl_Position = projection * ( oVertex[i].ec_pos + sideShift );
			EmitVertex();

			sideShift  = vec4( normal - diagNormal, 0.0 );
			gl_Position = projection * ( oVertex[i].ec_pos + sideShift );
			EmitVertex();

			EndPrimitive();
		}
	}
}
