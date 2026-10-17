// Properties for Phong shading
uniform vec4  AmbientProduct;
uniform float Shininess;
uniform vec4  colorSolid;
uniform vec4  colorSolidBack;
// Properties for Phong shading related to light sources
uniform vec4 FixedCam_DiffuseProduct;
uniform vec4 FixedCam_SpecularProduct;
uniform vec4 FixedWorld_DiffuseProduct;
uniform vec4 FixedWorld_SpecularProduct;
// +++ Vector for computing the effect of the light
varying vec3 normal;
varying vec3 FixedCam_halfVector,FixedCam_L;
varying vec3 FixedWorld_halfVector,FixedWorld_L;
// -----------------------------------------------
void main(void) {
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
		vec4 color = AmbientProduct + FixedCam_diffuse + FixedCam_specular + FixedWorld_diffuse + FixedWorld_specular;
		color.a = 1.0;
		gl_FragColor = colorSolid * color;
	} else {
		gl_FragColor = colorSolidBack;
	}
}
