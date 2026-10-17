#version 150

// Backfaces -- glDisable( GL_CULL_FACE ); has to be set
uniform bool  backCulling    = true;
uniform vec4  colorSolidBack = vec4( 128.0/255.0, 92.0/255.0, 92.0/255.0, 1.0 );

// Switch betwenn flat and smooth shading
uniform bool  flatShade = false;

// Solid color
uniform vec4  colorSolid     = vec4( 186.0/255.0, 186.0/255.0, 186.0/255.0, 1.0 );
// Properties for Phong shading
uniform vec4  AmbientProduct = vec4( 1.0, 1.0, 1.0, 1.0 ); // vec4( 0.1, 0.1, 0.1, 1.0 );
uniform float Shininess      = 14.154; // exp(2.65)

// Properties for Phong shading related to light sources
uniform bool uLightEnabled = false;
uniform vec4 FixedCam_DiffuseProduct;
uniform vec4 FixedCam_SpecularProduct;
uniform vec4 FixedWorld_DiffuseProduct;
uniform vec4 FixedWorld_SpecularProduct;

// Fog
uniform bool fogPresent = false;
uniform struct FogParameters {
	vec4  vFogColor; // Fog color
	float fStart;    // This is only for linear fog
	float fEnd;      // This is only for linear fog
	float fDensity;  // For exp and exp2 equation
	int   iEquation; // 0 = linear, 1 = exp, 2 = exp2 -- see defines GLSL_FOG_EQUATION_*
} fogParams;

// +++ Values to be passed from the vertex or geometry shader
in struct grVertex {
	vec4  ec_pos;        // eye coordinate position to be used for on-the-fly-computation of a triangles normal within the fragment shader.
	vec3  normal_interp; // Normal vector, which will be interpolated
	vec3  FixedCam_halfVector,FixedCam_L;
	vec3  FixedWorld_halfVector,FixedWorld_L;
} gVertex;

flat in uint gInvertColor;

// +++ Edge/Wireframe Rendering
noperspective in vec3 vEdgeDist;                           // Barycenter coordinates.
uniform vec4 uEdgeColor      = vec4( 0.1, 0.1, 0.1, 1.0 ); // Color of the edge
uniform bool uWireFrame      = false;
uniform vec4 uWireFrameColor = vec4( 1.0, 1.0, 1.0, 1.0 ); // This color should be equal to the background color of the scence.

// Output i.e. color of the fragment
out vec4 fragColor;

// --- Light function (Phong) ----------------------------------------------------------------------------------------------------------------------------------
vec4 getLightAmount( vec3 L, vec3 normal, vec3 halfVector, vec4 diffuseProduct, vec4 specularProduct ) { // Product essentially means color
	// Compute terms in the illumination equation for the lights:
	float Kd = max( dot( L, normal ), 0.0 );
	vec4  diffuse = Kd * diffuseProduct;
	float Ks = pow( max( dot( normal,halfVector ), 0.0 ), Shininess );
	vec4  specular = Ks * specularProduct;
	if( dot( L,normal ) < 0.0 ) {
		// specular = vec4( 0.0, 0.0, 0.0, 1.0 );
		return diffuse; // As there is no specular amount.
	}
	vec4 lightTotal = diffuse + specular;
	return lightTotal;
}

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
	vec3 normal = gVertex.normal_interp; // face normal in eyespace:
	if( flatShade ) { // flat shading
		normal = normalize( cross( dFdx( gVertex.ec_pos.xyz ), dFdy( gVertex.ec_pos.xyz ) ) ); // normal of the triangle
	}

	// +++ Edge/Wireframe Rendering
	float edgeIntensity = 0.0;
	if( uWireFrame ) {
		float nearD = min( min( vEdgeDist[0], vEdgeDist[1] ), vEdgeDist[2] );  // determine frag distance to closest edge
		edgeIntensity = exp2( -1.0*nearD*nearD );         // -1.0 correlates to the width - the smaller the number the broader the line.
	}

	// +++ Add fog (if present):
	float fFogCoord  = 0.0;
	float fFogFactor = 0.0;
	if( fogPresent ) {
		fFogCoord  = abs( gVertex.ec_pos.z / gVertex.ec_pos.w );
		fFogFactor = getFogFactor( fogParams, fFogCoord );
	}
	if( gl_FrontFacing ) {
		// Set a default color to be overwritten later:
		vec4 outputColor = vec4( 0.5, 0.0, 0.0, 1.0 );
	// ++++ Compute the illumination equation for the lights:
		vec4 colorLight = vec4( 1.0, 1.0, 1.0, 1.0 );
		if( uLightEnabled ) {
			vec4 fixedCamLight   = getLightAmount( gVertex.FixedCam_L,   normal, gVertex.FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
			vec4 fixedWorldLight = getLightAmount( gVertex.FixedWorld_L, normal, gVertex.FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );
		// ++++ Sum up all parts of the light:
			colorLight = AmbientProduct + fixedCamLight + fixedWorldLight;
			colorLight.a = 1.0;
		}
	// ++++ Solid color:
		outputColor = colorSolid * colorLight;
	// +++ Edge/Wireframe Rendering
		if( uWireFrame ) {
			if( edgeIntensity <= 0.0039 ) { // (1.0/256.0)
				discard;
			}
			outputColor = mix( uWireFrameColor, uEdgeColor, edgeIntensity );
		}
	// +++ Add fog (if present):
		if( fogPresent ) {
			outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
		}
	// +++ Invert color, when request e.g. for normals:
		if( gInvertColor > 0u ) {
			outputColor.rgb = 1.0 - outputColor.rgb;
		}
		fragColor = outputColor; // was gl_FragColor, which is deprecated
	} else if( backCulling ) { // Backfaces color & culling
		vec4 outputColor  = colorSolidBack;
	// +++ Edge/Wireframe Rendering 
		if( uWireFrame ) {
			if( edgeIntensity <= 0.0039 ) { // (1.0/256.0) 
				discard;
			}
			outputColor = mix( uWireFrameColor, uEdgeColor, edgeIntensity );
		}
	// +++ Add fog (if present):
		if( fogPresent ) {
			outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
		}
		fragColor = outputColor; // was gl_FragColor, which is deprecated
	} else { // No backfaces at all.
		discard; // "discard" is better then setting a transparent color as it will not manipulate to the z-buffer.
	}
}
