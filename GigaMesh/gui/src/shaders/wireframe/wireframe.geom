#version 150
// IMPORTANT: It isn't possible to create a program with geometry shaders that handle multiple primitive types.
// This means: out is either triangle_strip or line_strip!

layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out; // 3 for the face itselt. +3 for normal. +2x3 for light directions.

// +++ Homogenous matrices for camera orientation and projection: ----------------------------------------------------------------------------------------------
uniform highp mat4 modelview;
uniform highp mat4 projection;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Direction of the light fixed in relation to the camera: -------------------------------------------------------------------------------------------------
uniform vec3 uLightDirectionFixedCamera = vec3( 0.0, 0.0, 1.0 );
uniform vec3 uLightDirectionFixedWorld  = vec3( 0.0, 0.0, 1.0 );
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Rendering of face normals as small triangles: -----------------------------------------------------------------------------------------------------------
uniform bool  uFaceNormals  = false;
uniform float uNormalLength = 1.5;
uniform float uNormalWidth  = 0.1;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Rendering of a light direction as line-like triangles: --------------------------------------------------------------------------------------------------
uniform int   uLightVectors    = 0; // 0 = off, 1 = MOUSE_MODE_MOVE_LIGHT_FIXED_CAM, 2 = MOUSE_MODE_MOVE_LIGHT_FIXED_OBJECT
uniform int   uLightVecModVal  = 10;
uniform float uLightVeclLength = 20.0;
uniform float uLightVecWidth   =  0.005;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

// +++ Values to be passed from the vertex.
in struct grVertex {
	vec4  ec_pos;        // eye coordinate position to be used for on-the-fly-computation of a triangles normal within the fragment shader.
	vec3  normal_interp; // Normal vector, which will be interpolated
	vec3  FixedCam_halfVector,FixedCam_L;
	vec3  FixedWorld_halfVector,FixedWorld_L;
} oVertex[];

out grVertex gVertex;
flat out uint gInvertColor;

// +++ Edge/Wireframe Rendering 
noperspective out vec3 vEdgeDist;              // Barycenter coordinates.
uniform vec2 uViewPortSize = vec2( 860, 718 ); // ( width, height ) of the viewport in pixel

//uniform float uExplodeFactor = 0.12;

void main(void) {
	// Edge/Wireframe Rendering - taken from 'Single-Pass Wireframe Rendering' ( http://www2.imm.dtu.dk/pubdb/views/edoc_download.php/4884/pdf/imm4884.pdf )
	vec2 p0 = uViewPortSize * gl_in[0].gl_Position.xy/gl_in[0].gl_Position.w;
	vec2 p1 = uViewPortSize * gl_in[1].gl_Position.xy/gl_in[1].gl_Position.w;
	vec2 p2 = uViewPortSize * gl_in[2].gl_Position.xy/gl_in[2].gl_Position.w;
	vec2 v0 = p2-p1;
	vec2 v1 = p2-p0;
	vec2 v2 = p1-p0;
	float area = abs( v1.x * v2.y - v1.y * v2.x );

	gInvertColor = 0u;
	int i;
	// Pass thru the triangle as it is.
	for( i=0; i<gl_in.length(); i++ ) {
		// According to http://www.opengl.org/wiki/Built-in_Variable_%28GLSL%29#Vertex_shader_outputs
		gl_Position                   = gl_in[i].gl_Position; // + vec4( uExplodeFactor * oVertex[i].normal_interp, 0.0);
		gl_ClipDistance[0]            = gl_in[i].gl_ClipDistance[0];
		gl_ClipDistance[1]            = gl_in[i].gl_ClipDistance[1];
		gl_ClipDistance[2]            = gl_in[i].gl_ClipDistance[2];
		// +++ Pass on data
		gVertex.ec_pos                = oVertex[i].ec_pos;
		gVertex.normal_interp         = oVertex[i].normal_interp;
		gVertex.FixedCam_halfVector   = oVertex[i].FixedCam_halfVector;
		gVertex.FixedCam_L            = oVertex[i].FixedCam_L;
		gVertex.FixedWorld_halfVector = oVertex[i].FixedWorld_halfVector;
		gVertex.FixedWorld_L          = oVertex[i].FixedWorld_L;
		// +++ Edge/Wireframe Rendering 
		if( i==0 ) {
			vEdgeDist = vec3( area/length(v0), 0.0, 0.0 );
		}
		if( i==1 ) {
			vEdgeDist = vec3( 0.0, area/length(v1), 0.0 );
		}
		if( i==2 ) {
			vEdgeDist = vec3( 0.0, 0.0, area/length(v2) );
		}
		// +++ DONE
		EmitVertex();
	}
	EndPrimitive();

	if( uFaceNormals ) {
		gInvertColor = 1u;
		// Calculate the center of gravity of the triangle:
		vec3 cog = ( oVertex[0].ec_pos.xyz + oVertex[1].ec_pos.xyz + oVertex[2].ec_pos.xyz ) / 3.0;

		// Calculate two vectors in the plane of the input triangle
		vec3 ab = oVertex[1].ec_pos.xyz - oVertex[0].ec_pos.xyz;
		vec3 ac = oVertex[2].ec_pos.xyz - oVertex[0].ec_pos.xyz;
		vec3 normal = normalize( cross( ab, ac ) ) * uNormalLength;

		vec3 diagNormal = cross( normal, vec3( 0.0, 0.0, uNormalWidth ) );

		gl_Position = projection * vec4( cog, 1.0 );
		EmitVertex();

		vec4 sideShift = vec4( normal + diagNormal, 0.0 );
	
		gl_Position = projection * ( vec4( cog, 1.0 ) + sideShift );
		EmitVertex();

		sideShift = vec4( normal - diagNormal, 0.0 );
		gl_Position = projection * ( vec4( cog, 1.0 ) + sideShift );
		EmitVertex();

		EndPrimitive();
		gInvertColor = 0u;
	}
	
        if( uLightVectors != 0 && ( mod( gl_PrimitiveIDIn, uLightVecModVal ) == 0 ) ) {
        gInvertColor = 1u;
        // Calculate the center of gravity of the triangle:
        vec3 cog = ( oVertex[0].ec_pos.xyz + oVertex[1].ec_pos.xyz + oVertex[2].ec_pos.xyz ) / 3.0;

        // Calculate two vectors in the plane of the input triangle
        vec3 ab = oVertex[1].ec_pos.xyz - oVertex[0].ec_pos.xyz;
        vec3 ac = oVertex[2].ec_pos.xyz - oVertex[0].ec_pos.xyz;
        vec3 normal = normalize( cross( ab, ac ) );

        vec3 lightDirFix;
        if(uLightVectors == 1)
        {
            lightDirFix = normalize( uLightDirectionFixedCamera );
        }
        else if(uLightVectors == 2)
        {
            lightDirFix = ( normalize( vec4( uLightDirectionFixedWorld, 0.0 ) )*inverse(modelview) ).xyz;
        }
        if( ( length( lightDirFix ) > 0.0 ) &&       // Show only if the light is turned on
            ( dot( normal, lightDirFix ) > 0.1 ) ) { // Show only if the light hits the face
                lightDirFix *= uLightVeclLength;

                vec3 diagNormal = cross( lightDirFix, vec3( 0.0, 0.0, uLightVecWidth ) );

                gl_Position = projection * vec4( cog, 1.0 );
                EmitVertex();

                vec4 sideShift = vec4( lightDirFix + diagNormal, 0.0 );

                gl_Position = projection * ( vec4( cog, 1.0 ) + sideShift );
                EmitVertex();

                sideShift = vec4( lightDirFix - diagNormal, 0.0 );
                gl_Position = projection * ( vec4( cog, 1.0 ) + sideShift );
                EmitVertex();

                EndPrimitive();
        }
		gInvertColor = 0u;
	}
}
