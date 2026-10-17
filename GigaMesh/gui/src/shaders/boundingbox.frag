#version 150

// Solid color
uniform bool  uColorSolidForce = false;
uniform vec4  uColorSolid      = vec4( 186.0/255.0, 186.0/255.0, 186.0/255.0, 1.0 );

// Properties for Phong shading
uniform vec4  AmbientProduct = vec4( 1.0, 1.0, 1.0, 1.0 ); // vec4( 0.1, 0.1, 0.1, 1.0 );
uniform float Shininess      = 14.154; // exp(2.65)
// Backfaces -- glDisable( GL_CULL_FACE ); has to be set
uniform bool  backCulling    = true;
uniform vec4  colorSolidBack = vec4( 128.0/255.0, 92.0/255.0, 92.0/255.0, 1.0 );
// Switch betwenn flat and smooth shading
uniform bool  flatShade = false;

// Properties for Phong shading related to light sources
uniform vec4 FixedCam_DiffuseProduct    = vec4( 0.0, 0.0, 0.0, 1.0 ); // vec4( 1.0, 1.0, 1.0, 1.0 );
uniform vec4 FixedCam_SpecularProduct   = vec4( 0.0, 0.0, 0.0, 1.0 );
uniform vec4 FixedWorld_DiffuseProduct  = vec4( 0.0, 0.0, 0.0, 1.0 );
uniform vec4 FixedWorld_SpecularProduct = vec4( 0.0, 0.0, 0.0, 1.0 );
// +++ Fog
uniform bool fogPresent = false;
uniform struct FogParameters {
	vec4  vFogColor; // Fog color
	float fStart;    // This is only for linear fog
	float fEnd;      // This is only for linear fog
	float fDensity;  // For exp and exp2 equation
	int   iEquation; // 0 = linear, 1 = exp, 2 = exp2 -- see defines GLSL_FOG_EQUATION_*
} fogParams;

// +++ Color of the vertex
in vec4 vertexColor;

// +++ Vector for computing the effect of the light
in vec4 ec_pos;         // Interpolated position of the fragment in eye coordinates.
in vec3 normal_interp;  // Interpolated normal (also in eye coordinates).
in vec3 FixedCam_halfVector,FixedCam_L;
in vec3 FixedWorld_halfVector,FixedWorld_L;

// +++ Output i.e. color of the fragment
out vec4 fragColor;

// --- Fog function ---------------------------------------------------------------------------------------------------------------------------------------------
float getFogFactor( FogParameters params, float fFogCoord ) {
	float fResult = 0.0;
	if( params.iEquation == 0 ) {
		fResult = ( params.fEnd-fFogCoord )/( params.fEnd-params.fStart );
	} else if( params.iEquation == 1 ) {
		fResult = exp(-params.fDensity*fFogCoord );
	} else if( params.iEquation == 2 ) {
		fResult = exp(-pow(params.fDensity*fFogCoord, 2.0));
	}
	fResult = 1.0 - clamp( fResult, 0.0, 1.0 );
	return fResult;
}

// --- MAIN ----------------------------------------------------------------------------------------------------------------------------------------------------
void main(void) {
	vec3 normal = normal_interp; // face normal in eyespace:
	if( flatShade ) { // flat shading
		normal = normalize(cross(dFdx(ec_pos.xyz), dFdy(ec_pos.xyz))); // normal of the triangle
	}
	// +++ Add fog (if present):
	float fFogCoord  = 0.0;
	float fFogFactor = 0.0;
	if( fogPresent ) {
		fFogCoord  = abs( ec_pos.z / ec_pos.w );
		fFogFactor = getFogFactor( fogParams, fFogCoord );
	}
	if( gl_FrontFacing ) {
	// ++++ Compute terms in the illumination equation for the light fixed to the camera
		float FixedCam_Kd = max( dot(FixedCam_L,normal), 0.0 );
		vec4  FixedCam_diffuse = FixedCam_Kd * FixedCam_DiffuseProduct;
		float FixedCam_Ks = pow( max( dot(normal,FixedCam_halfVector), 0.0 ), Shininess );
		vec4  FixedCam_specular = FixedCam_Ks * FixedCam_SpecularProduct;
		if( dot(FixedCam_L,normal) < 0.0 ) {
			FixedCam_specular = vec4( 0.0, 0.0, 0.0, 1.0 );
		}
	// ++++ Compute terms in the illumination equation for the light fixed to the camera
		float FixedWorld_Kd = max( dot(FixedWorld_L,normal), 0.0 );
		vec4  FixedWorld_diffuse = FixedWorld_Kd * FixedWorld_DiffuseProduct;
		float FixedWorld_Ks = pow( max( dot(normal,FixedWorld_halfVector), 0.0 ), Shininess );
		vec4  FixedWorld_specular = FixedWorld_Ks * FixedWorld_SpecularProduct;
		if( dot(FixedWorld_L,normal) < 0.0 ) {
			FixedWorld_specular = vec4( 0.0, 0.0, 0.0, 1.0 );
		}
	// ++++ Sum up all parts:
		vec4 colorLight = AmbientProduct + FixedCam_diffuse + FixedCam_specular + FixedWorld_diffuse + FixedWorld_specular;
		colorLight.a = 1.0;
		vec4 outputColor = vertexColor * colorLight;
	// +++ Force solid color:
		if( uColorSolidForce ) {
			outputColor = uColorSolid * colorLight;
		}
	// +++ Add fog (if present):
		if( fogPresent ) {
			outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
		}
		fragColor = outputColor; // was gl_FragColor, which is deprecated
	} else if( backCulling ) { // Backfaces color & culling
		vec4 outputColor  = colorSolidBack;
	// +++ Add fog (if present):
		if( fogPresent ) {
			outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
		}
		fragColor = outputColor; // was gl_FragColor, which is deprecated
	}
}
