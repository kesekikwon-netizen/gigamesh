#version 150
// IMPORTANT: It isn't possible to create a program with geometry shaders that handle multiple primitive types.
// This means: out is either triangle_strip or line_strip!

layout (lines) in;
layout (triangle_strip, max_vertices = 34) out; // 4 for the axis PLUS Segments -- max seems to be 34 for a GTX 470

// +++ Homogenous matrices for camera orientation and projection:
uniform highp mat4 modelview;
uniform highp mat4 projection;
// -------------------------------------------------------------------------------------------------------------------------------------------------------------

uniform float uAxisWidth = 0.5;
uniform vec4  uAxisColor = vec4( 0.5, 0.0, 0.0, 1.0 );

// +++ Values to be passed from the vertex.
in struct grVertex {
	vec4  ec_pos;        // eye coordinate position to be used for on-the-fly-computation of a triangles normal within the fragment shader.
	vec3  normal_interp; // Normal vector, which will be interpolated
	vec3  FixedCam_halfVector,FixedCam_L;
	vec3  FixedWorld_halfVector,FixedWorld_L;
	//+++ Color of the vertex
	vec4  vertexColor;
	// +++ Function value of the vertex passed to the fragment shader:
	float vertexRadius;
	vec3  normalCutPlane;
} oVertex[];

out struct grVertexX {
	vec4  ec_pos;        // eye coordinate position to be used for on-the-fly-computation of a triangles normal within the fragment shader.
	vec3  normal_interp; // Normal vector, which will be interpolated
	vec3  FixedCam_halfVector,FixedCam_L;
	vec3  FixedWorld_halfVector,FixedWorld_L;
	//+++ Color of the vertex
	vec4  vertexColor;
} gVertex;

mat4 rotationMatrix( vec3 rPosition, vec3 rDirection, float rAngle ) {
	// Init with identy matrix I(1), which is returned in case of an error.
	mat4 rotMat = mat4( 1.0, 0.0, 0.0, 0.0, 
	                    0.0, 1.0, 0.0, 0.0, 
	                    0.0, 0.0, 1.0, 0.0, 
	                    0.0, 0.0, 0.0, 1.0 );

	// No rotation et all => return identity matrix,
	if( abs( rAngle ) <= 0.00 ) {
		return rotMat;
	}

	float a = rPosition.x;
	float b = rPosition.y;
	float c = rPosition.z;

	float u = rDirection.x;
	float v = rDirection.y;
	float w = rDirection.z;

	// Set some intermediate values, Part 1.
	float u2 = u*u;
	float v2 = v*v;
	float w2 = w*w;
	float l2 = u2 + v2 + w2;

	// l2 == 0 means that there the given direction vector has no length and therefore is invalid.
	if( l2 == 0 ) {
		return rotMat;
	}

	// Set some intermediate values, Part 2.
	float cosT = cos( rAngle );
	float sinT = sin( rAngle );
	float l =  sqrt(l2);

	// Build the matrix entries element by element. 
	rotMat[0][0] = (u2 + (v2 + w2) * cosT)/l2;
	rotMat[0][1] = (u*v * (1 - cosT) - w*l*sinT)/l2;
	rotMat[0][2] = (u*w * (1 - cosT) + v*l*sinT)/l2;
	rotMat[0][3] = (a*(v2 + w2) - u*(b*v + c*w)
	                + (u*(b*v + c*w) - a*(v2 + w2))*cosT + (b*w - c*v)*l*sinT)/l2;

	rotMat[1][0] = (u*v * (1 - cosT) + w*l*sinT)/l2;
	rotMat[1][1] = (v2 + (u2 + w2) * cosT)/l2;
	rotMat[1][2] = (v*w * (1 - cosT) - u*l*sinT)/l2;
	rotMat[1][3] = (b*(u2 + w2) - v*(a*u + c*w)
	                + (v*(a*u + c*w) - b*(u2 + w2))*cosT + (c*u - a*w)*l*sinT)/l2;

	rotMat[2][0] = (u*w * (1 - cosT) - v*l*sinT)/l2;
	rotMat[2][1] = (v*w * (1 - cosT) + u*l*sinT)/l2;
	rotMat[2][2] = (w2 + (u2 + v2) * cosT)/l2;
	rotMat[2][3] = (c*(u2 + v2) - w*(a*u + b*v)
	                + (w*(a*u + b*v) - c*(u2 + v2))*cosT + (a*v - b*u)*l*sinT)/l2;

	return rotMat;
}

void main(void) {
	vec3 axisDir   = normalize( oVertex[1].ec_pos.xyz - oVertex[0].ec_pos.xyz );
	vec4 sideShift = vec4( normalize( cross( axisDir, vec3( 0.0, 0.0, 1.0 ) ) )*uAxisWidth/2.0, 0.0 );

	int i;
	for( i=0; i<gl_in.length(); i++ ) {
		// +++ Pass on data
		gVertex.ec_pos                = oVertex[i].ec_pos + sideShift;
		gVertex.normal_interp         = oVertex[i].normal_interp;
		gVertex.FixedCam_halfVector   = oVertex[i].FixedCam_halfVector;
		gVertex.FixedCam_L            = oVertex[i].FixedCam_L;
		gVertex.FixedWorld_halfVector = oVertex[i].FixedWorld_halfVector;
		gVertex.FixedWorld_L          = oVertex[i].FixedWorld_L;
		// +++ Color of the vertex
		gVertex.vertexColor           = uAxisColor;
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
		gVertex.vertexColor           = uAxisColor;
		// According to http://www.opengl.org/wiki/Built-in_Variable_%28GLSL%29#Vertex_shader_outputs
		gl_Position                   = projection * gVertex.ec_pos;
		gl_ClipDistance[0]            = gl_in[i].gl_ClipDistance[0];
		gl_ClipDistance[1]            = gl_in[i].gl_ClipDistance[1];
		gl_ClipDistance[2]            = gl_in[i].gl_ClipDistance[2];
		// +++ DONE
		EmitVertex();
	}
	EndPrimitive();


	float pi = asin(1.0);

	vec3  origin    = vec3( 0.0, 0.0, 0.0 );
	vec3  zAxis     = vec3( 0.0, 0.0, 1.0 );
	//vec3  planeNorm = (inverse(modelview)*vec4(axisDir,0.0)).xyz; // <- this works for planes orthogonal to the cone's axis.

	vec3  planeNormA = normalize( (inverse(modelview)*vec4(oVertex[0].normalCutPlane,0.0)).xyz );
	vec3  rotAxisA   = normalize( cross( planeNormA, zAxis  ) );
	float rotAngleA  = acos( dot( zAxis, planeNormA ) );
	// Take care about the case that the plane normal is colinear with the z-axis:
	if( length( cross( planeNormA, zAxis ) ) <= 0.00001 ) {
		rotAngleA = 0.0;
		if( length( planeNormA - zAxis ) > 0.0 ) { // flip if the axis is in oposite direction
			rotAngleA = 2*pi;
			rotAxisA = vec3( 0.0, 1.0, 0.0 );
		}
	}
	vec4 radialShiftA;

	vec3  planeNormB = normalize( (inverse(modelview)*vec4(oVertex[1].normalCutPlane,0.0)).xyz );
	vec3  rotAxisB   = normalize( cross( planeNormB, zAxis  ) );
	float rotAngleB  = acos( dot( zAxis, planeNormB ) );
	// Take care about the case that the plane normal is colinear with the z-axis:
	if( length( cross( planeNormA, zAxis ) ) <= 0.00001 ) {
		rotAngleB = 0.0;
		if( length( planeNormA - zAxis ) > 0.0 ) { // flip if the axis is in oposite direction
			rotAngleB = 2*pi;
			rotAxisB = vec3( 0.0, 1.0, 0.0 );
		}
	}
	vec4 radialShiftB;

	float idx;
	for( idx = -1.0; idx <= +1.0; idx += 0.25 ) {
		float alpha = idx * 2.0 * pi;
		radialShiftA = modelview * ( rotationMatrix( origin, rotAxisA, rotAngleA ) * ( vec4( sin(alpha), cos(alpha), 0.0, 0.0 ) * oVertex[0].vertexRadius ) );
		radialShiftB = modelview * ( rotationMatrix( origin, rotAxisB, rotAngleB ) * ( vec4( sin(alpha), cos(alpha), 0.0, 0.0 ) * oVertex[1].vertexRadius ) );

		i=1;
		// +++ Pass on data
		gVertex.ec_pos                = oVertex[i].ec_pos + radialShiftB;
		gVertex.normal_interp         = oVertex[i].normal_interp;
		gVertex.FixedCam_halfVector   = oVertex[i].FixedCam_halfVector;
		gVertex.FixedCam_L            = oVertex[i].FixedCam_L;
		gVertex.FixedWorld_halfVector = oVertex[i].FixedWorld_halfVector;
		gVertex.FixedWorld_L          = oVertex[i].FixedWorld_L;
		// +++ Color of the vertex
		gVertex.vertexColor           = oVertex[i].vertexColor;
		// According to http://www.opengl.org/wiki/Built-in_Variable_%28GLSL%29#Vertex_shader_outputs
		gl_Position                   = projection * gVertex.ec_pos;
		gl_ClipDistance[0]            = gl_in[i].gl_ClipDistance[0];
		gl_ClipDistance[1]            = gl_in[i].gl_ClipDistance[1];
		gl_ClipDistance[2]            = gl_in[i].gl_ClipDistance[2];
		// +++ DONE
		EmitVertex();

		i=0;
		// +++ Pass on data
		gVertex.ec_pos                = oVertex[i].ec_pos + radialShiftA;
		gVertex.normal_interp         = oVertex[i].normal_interp;
		gVertex.FixedCam_halfVector   = oVertex[i].FixedCam_halfVector;
		gVertex.FixedCam_L            = oVertex[i].FixedCam_L;
		gVertex.FixedWorld_halfVector = oVertex[i].FixedWorld_halfVector;
		gVertex.FixedWorld_L          = oVertex[i].FixedWorld_L;
		// +++ Color of the vertex
		gVertex.vertexColor           = oVertex[i].vertexColor;
		// According to http://www.opengl.org/wiki/Built-in_Variable_%28GLSL%29#Vertex_shader_outputs
		gl_Position                   = projection * gVertex.ec_pos;
		gl_ClipDistance[0]            = gl_in[i].gl_ClipDistance[0];
		gl_ClipDistance[1]            = gl_in[i].gl_ClipDistance[1];
		gl_ClipDistance[2]            = gl_in[i].gl_ClipDistance[2];
		// +++ DONE
		EmitVertex();
	}

	EndPrimitive();
}
