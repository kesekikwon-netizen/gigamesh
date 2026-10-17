#version 150

// +++ Values to be passed from the vertex or geometry shader
in struct grVertex {
	vec4  ec_pos;        // eye coordinate position
	vec3  normal_interp; // Normal vector, which will be interpolated
	vec3  FixedCam_L;
	vec3  FixedWorld_L;
	vec3 FixedCam_halfVector;
	vec3 FixedWorld_halfVector;
} oVertex;

uniform float Shininess      = 14.154;

uniform vec4 FixedCam_DiffuseProduct;
uniform vec4 FixedCam_SpecularProduct;
uniform vec4 FixedWorld_DiffuseProduct;
uniform vec4 FixedWorld_SpecularProduct;
uniform vec4  AmbientProduct = vec4( 1.0, 1.0, 1.0, 1.0 ); // vec4( 0.1, 0.1, 0.1, 1.0 );

// Switch betwenn flat and smooth shading
uniform bool  flatShade = false;

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

    vec3 normal = normalize(oVertex.normal_interp); // face normal in eyespace:
    if( flatShade ) { // flat shading
	    normal = normalize( cross( dFdx( oVertex.ec_pos.xyz ), dFdy( oVertex.ec_pos.xyz ) ) ); // normal of the triangle
    }

/*    vec4 fixedCamLight   = getLightAmount( oVertex.FixedCam_L,   normal, oVertex.FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
    vec4 fixedWorldLight = getLightAmount( oVertex.FixedWorld_L, normal, oVertex.FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );

    float Kd_cam = max( dot( oVertex.FixedCam_L, normal ), 0.0 );
    float Ks_cam = step(0.000000001, Kd_cam) * pow( clamp( dot( normal,oVertex.FixedCam_halfVector ), 0.0, 1.0 ), Shininess );

    float light_cam = Kd_cam;// + Ks_cam;

    float Kd_fixed = max( dot( oVertex.FixedWorld_L, normal ), 0.0 );
    float Ks_fixed = step(0.0000001, Kd_fixed) * pow( clamp( dot( normal,oVertex.FixedWorld_halfVector ), 0.0, 1.0 ), Shininess );

    float light_fixed = Kd_fixed;// + Ks_fixed;

    light_fixed += light_cam;
*/

  //  vec4 colorLight = vec4( 1.0, 1.0, 1.0, 1.0 );

	    vec4 fixedCamLight   = getLightAmount( oVertex.FixedCam_L,   normal, oVertex.FixedCam_halfVector,   FixedCam_DiffuseProduct,   FixedCam_SpecularProduct   );
	    vec4 fixedWorldLight = getLightAmount( oVertex.FixedWorld_L, normal, oVertex.FixedWorld_halfVector, FixedWorld_DiffuseProduct, FixedWorld_SpecularProduct );
    // ++++ Sum up all parts of the light:
	    fragColor = AmbientProduct + fixedCamLight + fixedWorldLight;
	    fragColor.a = 1.0;


 //   fragColor.rgb = vec3(1.0,1.0,1.0) * light_fixed;
}
