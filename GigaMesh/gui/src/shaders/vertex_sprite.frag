#version 150

// Backfaces -- GL_CULL_* has no effect for sprites!
// Therefore gl_FrontFacing has no effect.

// Switch betwenn flat and smooth shading -- not in sprites!

// Shape of the sprites -- see MeshGLParams::eSpriteShapes
// 0 ... Box
// 1 ... Disc
// 2 ... Rounded star
// 3 ... Polar rose
// in all other cases, a the box is used.
uniform int       uSpriteShape = 2;

// Selection for the rendered color:
// 1 ... Color per vertex
// 2 ... Function values
// 3 ... Labels
// in all other case, the solid color is used.
uniform int       uRenderColor = 0;

// Label settings:
uniform sampler2D uLabelTexMap;                                          // Texturemap storing the label colors.
uniform float     uLabelTexMapSel       =  1.0;                          // Selected row within the texture map.
uniform float     uLabelColorCount      = 11.0;                          // Number of colors available within the selected row of the texture map.
uniform float     uLabelCountOffset     =  0.0;                          // Offset to shift the color with the row of the texture map.
uniform vec4      uLabelBorderColor     = vec4( 0.25, 0.25, 0.25, 1.0 ); // Color for connected components tagged as background
uniform vec4      uLabelBackgroundColor = vec4( 0.25, 0.25, 0.25, 1.0 ); // Color for connected components tagged as background
uniform vec4      uLabelNoColor         = vec4( 0.61, 0.09, 0.09, 1.0 ); // Color for areas not labeled - i.e. not being part of a connect component nor the backgroud
uniform vec4      uLabelSingleColor     = vec4( 0.00, 0.00, 0.00, 1.0 ); // Show all labels with the same color (instead of using sampler2D uLabelTexMap).
uniform bool      uLabelSameColor       = false;                         // Show all labels with the same color (instead of using sampler2D uLabelTexMap).

// Solid color
uniform vec4  colorSolid     = vec4( 186.0/255.0, 186.0/255.0, 186.0/255.0, 1.0 );
// Properties for Phong shading
uniform vec4  AmbientProduct = vec4( 1.0, 1.0, 1.0, 1.0 ); // vec4( 0.1, 0.1, 0.1, 1.0 );
uniform float Shininess      = 14.154; // exp(2.65)

// Properties for Phong shading related to light sources
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

// Function value of the vertex
uniform sampler2D uFuncValTexMap;

// +++ Values to be passed from the vertex.
in struct grVertex {
	vec4  ec_pos;        // Interpolated position of the fragment in eye coordinates.
	vec3  normal_interp; // Interpolated normal (also in eye coordinates).
	vec3  FixedCam_halfVector,FixedCam_L;
	vec3  FixedWorld_halfVector,FixedWorld_L;
	//+++ Color of the vertex
	vec4  vertexColor;
	// +++ Function value of the vertex passed to the fragment shader:
	float vertexFuncVal;
	vec2  vertexFuncValTexCoord;
	// +++ Labels
	float labelNr;       // Interpolated label ID. However this should be an integer value, because interpolation will only happen at faces along a label border. Corresponds to vLabelID
	float flagNoLabel; 
} oVertex;

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
	// The following Dot computations fail due to a bug in the driver, when the CoreProfile is not active!
	vec2 spCoord = gl_PointCoord * 2.0 - vec2( 1.0 );  //from [0,1] to [-1.0,+1.0]
	if( uSpriteShape == 1 ) {
		// Circular Dots
		if( length( spCoord ) > 1.0 ) {                    //outside of circle radius?
			discard;
		}
	} else if( uSpriteShape == 2 ) {
		// Star shaped dots
		float dotSP = dot( spCoord, spCoord );
		float r = sqrt( dotSP );
		float theta = atan( spCoord.y, spCoord.x );
		if( dotSP > cos( theta*5 ) ) {
			discard;
		}
	} else if( uSpriteShape == 3 ) {
		// Polar rose
		float dotSP = dot( spCoord, spCoord );
		float theta = atan( spCoord.y, spCoord.x );
		if( dotSP > 0.5*(exp(cos(theta*5)*0.75)) ) {
			discard;
		}
	}
	
	vec3 normal = oVertex.normal_interp; // face normal in eyespace:

	// +++ Add fog (if present):
	float fFogCoord  = 0.0;
	float fFogFactor = 0.0;
	if( fogPresent ) {
		fFogCoord  = abs( oVertex.ec_pos.z / oVertex.ec_pos.w );
		fFogFactor = getFogFactor( fogParams, fFogCoord );
	}

	// Set a default color to be overwritten later:
	vec4 outputColor = vec4( 0.5, 0.0, 0.0, 1.0 );
// ++++ Compute the illumination equation for the lights:
	vec4 fixedCamLight   = getLightAmount( oVertex.FixedCam_L,   normal, oVertex.FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
	vec4 fixedWorldLight = getLightAmount( oVertex.FixedWorld_L, normal, oVertex.FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );
// ++++ Sum up all parts of the light:
	vec4 colorLight = AmbientProduct + fixedCamLight + fixedWorldLight;
	colorLight.a = 1.0;
// ++++ Solid color:
	outputColor = colorSolid * colorLight;
// ++++ Color per Vertex:
	if( uRenderColor == 1 ) {
		outputColor  =  oVertex.vertexColor * colorLight;
	}
// ++++ Function value mapped to color ramp:
	if( uRenderColor == 2 ) {
		vec4 texColor = texture( uFuncValTexMap, oVertex.vertexFuncValTexCoord );
		outputColor  =  texColor * colorLight;
	}
// ++++ Label ID mapped to color:
	// Shade background faces:
	// ... todo ...
	// Shade faces with a certain color:
	if( uRenderColor == 3 ) {
		// Shade labeled areas:
		float labelNrShifted = oVertex.labelNr + uLabelCountOffset;
		float labelIDMod = labelNrShifted - uLabelColorCount * floor( labelNrShifted / uLabelColorCount );
		float labelTexCoordMap = (512.0 - 10.0*uLabelTexMapSel+4.5)/512.0; // Texture map with color ramps is 512x512 pixel and each colorramp is 10 pixel wide (in y).
		float labelTexCoord = (4.5 + 10.0*labelIDMod)/512.0;
		if( uLabelSameColor ) {
			outputColor  =  uLabelSingleColor * colorLight;
		} else {
			vec4 texColor = texture( uLabelTexMap, vec2( labelTexCoord, labelTexCoordMap ) );
			outputColor  =  texColor * colorLight;
		}
		// Shade faces between different labels (or background vertices) differently:
		if( abs( oVertex.labelNr - float( floor( oVertex.labelNr ) ) ) > 0.0 ) {
			outputColor = uLabelBorderColor * colorLight;
		}
		// Shading color if the fragment is part of the background label:
		if( false ) {
			outputColor = uLabelBackgroundColor * colorLight;
		}
		// Shade faces not being a label with a given color:
		if( oVertex.flagNoLabel > 0.0 ) {
			outputColor = uLabelNoColor * colorLight;
		}
	}
// +++ Add fog (if present):
	if( fogPresent ) {
		outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
	}
	fragColor = outputColor; // was gl_FragColor, which is deprecated
}
