#version 150

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
} gVertex;

// Backfaces -- glDisable( GL_CULL_FACE ); has to be set
uniform bool  backCulling    = true;
uniform bool  backLighting   = true;
uniform vec4  colorSolidBack = vec4( 128.0/255.0, 92.0/255.0, 92.0/255.0, 1.0 );


// Selection for the rendered color:
// 1 ... Color per vertex
// 2 ... Function values
// 3 ... Labels
// in all other case, the solid color is used.
uniform int       uRenderColor = 0;

// Label settings:
uniform sampler2D uLabelTexMap;                                          // Texturemap storing the label colors.
uniform float     uLabelTexMapSel       =  1.0;                          // Selected row within the texture map.
uniform float     uLabelColorCount      = 10.0;                          // Number of colors available within the selected row of the texture map.
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


// Function value of the vertex
uniform sampler2D uFuncValTexMap;

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

void main(void)
{
    vec3 normal = gVertex.normal_interp;

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
	    if( uLightEnabled && length(normal) > 0.0 ) {
		    vec4 fixedCamLight   = getLightAmount( gVertex.FixedCam_L,   normal, gVertex.FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
		    vec4 fixedWorldLight = getLightAmount( gVertex.FixedWorld_L, normal, gVertex.FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );
	    // ++++ Sum up all parts of the light:
		    colorLight = AmbientProduct + fixedCamLight + fixedWorldLight;
		    colorLight.a = 1.0;
	    }
    // ++++ Solid color:
	    outputColor = colorSolid * colorLight;
    // ++++ Color per Vertex:
	    if( uRenderColor == 1 ) {
		    outputColor  =  gVertex.vertexColor * colorLight;
	    }
    // ++++ Function value mapped to color ramp:
	    if( uRenderColor == 2 ) {
		    vec4 texColor = texture( uFuncValTexMap, gVertex.vertexFuncValTexCoord );
		    outputColor  =  texColor * colorLight;
	    }
    // ++++ Label ID mapped to color:
	    // Shade background faces:
	    // ... todo ...
	    // Shade faces with a certain color:
	    if( uRenderColor == 3 ) {
		    // Shade labeled areas:
		    float labelNrShifted = gVertex.labelNr + uLabelCountOffset;
			float labelIDMod = mod(labelNrShifted , uLabelColorCount);
			float labelTexCoordMap = (512.0 - 11.0*uLabelTexMapSel+4.5)/512.0; // Texture map with color ramps is 512x512 pixel and each colorramp is 10 pixel wide (in y).
			float labelTexCoord = (4.5 + 11.0*labelIDMod)/512.0;
		    if( uLabelSameColor ) {
			    outputColor  =  uLabelSingleColor * colorLight;
		    } else {
			    vec4 texColor = texture( uLabelTexMap, vec2( labelTexCoord, labelTexCoordMap ) );
			    outputColor  =  texColor * colorLight;
		    }
		    // Shading color if the fragment is part of the background label:
		    if( false ) {
			    outputColor = uLabelBackgroundColor * colorLight;
		    }
		    // Shade faces not being a label with a given color:
		    if( gVertex.flagNoLabel > 0.0 ) {
			    outputColor = uLabelNoColor * colorLight;
		    }
	    }
    // +++ Add fog (if present):
	    if( fogPresent ) {
		    outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
	    }

	    fragColor = outputColor; // was gl_FragColor, which is deprecated
    } else if( backCulling ) { // Backfaces color & culling
	    vec4 lighting = vec4(1.0f);

	    if( uLightEnabled && length(normal) > 0.0 ) {
		vec4 fixedCamLight   = getLightAmount( gVertex.FixedCam_L,   -normal, gVertex.FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
		vec4 fixedWorldLight = getLightAmount( gVertex.FixedWorld_L, -normal, gVertex.FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );
		// ++++ Sum up all parts of the light:
		lighting = AmbientProduct + fixedCamLight + fixedWorldLight;
	    }
	    vec4 outputColor  = mix(colorSolidBack, colorSolidBack * lighting, bvec4(backLighting));
    // +++ Add fog (if present):
	    if( fogPresent ) {
		    outputColor = mix( outputColor, fogParams.vFogColor, fFogFactor );
	    }
	    fragColor = outputColor; // was gl_FragColor, which is deprecated
    } else { // No backfaces at all.
	    discard; // "discard" is better then setting a transparent color as it will not manipulate to the z-buffer.
	    //fragColor = vec4( 0.0, 0.0, 0.0, 0.0 ); // was gl_FragColor, which is deprecated
    }

    fragColor.a = step(0.5, 1.0 - length(vec2(0.5) -gl_PointCoord));
    if(fragColor.a == 0.0)
	discard;
}
