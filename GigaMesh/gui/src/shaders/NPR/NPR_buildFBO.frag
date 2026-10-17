#version 330

// +++ Values to be passed from the vertex or geometry shader
in struct grVertex {
        vec4  ec_pos;        // eye coordinate position
        vec3  normal_interp; // Normal vector, which will be interpolated
	vec4  vColor;
        vec3  FixedCam_L;
        vec3  FixedWorld_L;
        vec3 FixedCam_halfVector;
        vec3 FixedWorld_halfVector;
	float funcValNormalized;
} oVertex;

uniform float Shininess      = 14.154;
uniform float HighlightSize = 0.2;
uniform float UseSpecular = 1.0;

// Properties for Phong shading related to light sources
uniform vec4 FixedCam_DiffuseProduct;
uniform vec4 FixedCam_SpecularProduct;
uniform vec4 FixedWorld_DiffuseProduct;
uniform vec4 FixedWorld_SpecularProduct;

uniform int flag_useFuncVal = 0; //bit 0 = funcval for edges, bit 1 = funcval for toon, bit 2 = funcval for hatching

//out vec4 fragColor;
layout(location=0) out vec4 oNormalDepth;   //output: rgb = normals, a = depth
layout(location=1) out vec4 oLighting;	    //output: rg = light intensity diff/spec for hatches, ba = light intensity diff/spec for toon
layout(location=2) out vec4 oVertexColor;   //output: raw vertex- rgb colors for toon shading

void main(void)
{

    //Normal Map + Depth for outlines
    oNormalDepth = vec4(0,0,0,1);

    if((flag_useFuncVal & 1) != 0)
    {
	oNormalDepth = vec4(oVertex.funcValNormalized,0.0, 0.0, 0.0);
    }
    else
    {
	oNormalDepth = vec4((oVertex.normal_interp.xyz + vec3(1.0,1.0,1.0)) * 0.5,
			     gl_FragCoord.z);
    }

    //calculate lightning
    vec3 norm = normalize(oVertex.normal_interp);


    float maxFixed = max(FixedCam_DiffuseProduct.r, max(FixedCam_DiffuseProduct.g, FixedCam_DiffuseProduct.b));
    float maxWorld = max(FixedWorld_DiffuseProduct.r, max(FixedWorld_DiffuseProduct.g, FixedWorld_DiffuseProduct.b));

    float cosTheta = min(clamp( dot( norm,oVertex.FixedCam_L ) * 1.0, 0.0,1.0 ) * maxFixed + clamp( dot( norm,oVertex.FixedWorld_L ) * 1.0, 0.0 ,1.0 ) * maxWorld , 1.0) ;

    maxFixed = max(FixedCam_SpecularProduct.r, max(FixedCam_SpecularProduct.g, FixedCam_SpecularProduct.b));
    maxWorld = max(FixedWorld_SpecularProduct.r, max(FixedWorld_SpecularProduct.g, FixedWorld_SpecularProduct.b));

    float Ks = clamp(pow( max( dot( norm,oVertex.FixedCam_halfVector ), 0.0 ), 14.154 /* Shininess*/ ) * maxFixed +
		     pow( max( dot( norm,oVertex.FixedWorld_halfVector ), 0.0 ), 14.154/*Shininess*/ ) * maxWorld
                     ,0,1);


    Ks = step(1.0 - HighlightSize ,Ks * step(0.0, cosTheta) * UseSpecular); //if Ks strong enough, set it to 1, otherwise 0

    //setting lightintensity for hatches
    if((flag_useFuncVal & 4) != 0)
    {
	oLighting.r = oVertex.funcValNormalized;
	oLighting.g = 0.0;
    }
    else
    {
	oLighting.r = cosTheta;
	oLighting.g = Ks;
    }

    //setting lightintensity for toon
    if((flag_useFuncVal & 2) != 0)
    {
	oLighting.b = oVertex.funcValNormalized;
	oLighting.a = 0.0;
    }
    else
    {
	oLighting.b = cosTheta;
	oLighting.a = Ks;
    }

    //vertex color
    oVertexColor = oVertex.vColor;
}
