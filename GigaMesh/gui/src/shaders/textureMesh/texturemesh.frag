#version 330

in vec3 normal;
in vec3 FixedCam_halfVector;
in vec3 FixedWorld_halfVector;
in vec3 FixedCam_L;
in vec3 FixedWorld_L;

in vec2 uv;

uniform sampler2D uTexture;

uniform bool uLightEnabled = false;
uniform vec4 FixedCam_DiffuseProduct;
uniform vec4 FixedCam_SpecularProduct;
uniform vec4 FixedWorld_DiffuseProduct;
uniform vec4 FixedWorld_SpecularProduct;
uniform vec4 AmbientProduct;
uniform vec3 uBackFaceColor;

uniform float Shininess;

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


void main(void)
{
	vec4 colorLight = vec4( 1.0, 1.0, 1.0, 1.0 );
	vec3 norm = normalize(normal);
	if(!gl_FrontFacing)
	{
		norm = -norm;
	}

	if( uLightEnabled )
	{
		vec4 fixedCamLight   = getLightAmount( FixedCam_L,   norm, FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
		vec4 fixedWorldLight = getLightAmount( FixedWorld_L, norm, FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );
		// ++++ Sum up all parts of the light:
		colorLight = AmbientProduct + fixedCamLight + fixedWorldLight;
		colorLight.a = 1.0;
	}

	if(gl_FrontFacing)
	{
		fragColor = texture(uTexture, uv) * colorLight;
	}

	else
	{
		fragColor.rgba = vec4(uBackFaceColor,1.0) * colorLight;
	}
}
