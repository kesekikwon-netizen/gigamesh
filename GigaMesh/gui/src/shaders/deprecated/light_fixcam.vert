// +++ Vertex buffers
attribute vec3 position;
attribute vec3 vNormal;
// +++ Homogenous matrices for camera orientation and projection:
uniform highp mat4 modelview;
uniform highp mat4 projection;
// +++ Position of the Light
//uniform vec3 LightPosition;
// +++ Direction of the light fixed in relation to the camera:
uniform vec3 LightDirectionFixedCam;
uniform vec3 LightDirectionFixedWorld;
// +++ Values to be passed on to the fragment shader:
varying vec3 normal; // obviously the same for both light sources.
varying vec3 FixedCam_halfVector,FixedCam_L;
varying vec3 FixedWorld_halfVector,FixedWorld_L;
// -----------------------------------------------
void main(void) {
	vec3 pos = ( modelview * vec4( position, 1.0 ) ).xyz;               // Transform the vertex position into the eye coordinate system.
	normal   = normalize( modelview * vec4( vNormal, 0.0 ) ).xyz;       // Transform the vertex normal into eye coordinates.
	vec3 E = normalize( -pos );                                         // "eye" vector, which is the vector from the vertex's eye-space position to the origin.
// ---- Postional light fixed to the object/world -----
//	vec4 LightPositionFix = ( modelview * vec4( LightPosition, 1.0 ) ); // Fixed to object
//	L = normalize( LightPositionFix.xyz - pos );                        // vector from the vertex to the light.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Postional light fixed to the camera -----
//	L = normalize( LightPosition.xyz - pos );                           // vector from the vertex to the light.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Light direction fixed to the camera -----
	FixedCam_L = normalize( LightDirectionFixedCam );
	FixedCam_halfVector = normalize( FixedCam_L + E );                  // "half vector"  which is the normalized vector half-way between the light and eye vectors.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Light direction fixed to the world/object -----
	FixedWorld_L = normalize( modelview * vec4( LightDirectionFixedWorld, 0.0 ) ).xyz;
	FixedWorld_halfVector = normalize( FixedWorld_L + E );              // "half vector"  which is the normalized vector half-way between the light and eye vectors.
// -------------------------------------------------------------------------------------------------------------------------------------------------------------
// ---- Toon shader ---
//		//"vec4 LightPosition = gl_LightSource[4].position;
//		"	vec3 lightDir = normalize(vec3(LightPosition));"
//		"	float intensity = dot(lightDir,( modelview * vec4( vNormal, 0.0 ) ).xyz );
//		"	gl_FrontColor = vec4( vec3( 1.0, 0.5, 0.0 )*intensity, 1.0 ); // Diffuse shader - ignore the rest for a simple diffuse shader.
//		"	if (intensity > 0.95) "
//		"		 color = vec4(1.0,0.5,0.5,1.0); "
//		"	else if (intensity > 0.5) "
//		"		 color = vec4(0.6,0.3,0.3,1.0);"
//		"	else if (intensity > 0.25) "
//		"		color = vec4(0.4,0.2,0.2,1.0);"
//		"	else "
//		"		color = vec4(0.2,0.1,0.1,1.0);"
//		"	gl_FrontColor = color;
// ---- Toon shader ---
//		"	gl_FrontColor = vec4( 1.0, 0.5, 0.0, 1.0 );
//		"	gl_BackColor  = vec4( 0.0, 0.5, 1.0, 1.0 );
// --------------------
	gl_Position = projection * modelview * vec4( position, 1.0 );
}
