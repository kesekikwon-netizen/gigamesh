#version 330

uniform sampler2D uColorTexture;
uniform sampler2D uLightingTexture;

uniform vec2 uViewPortSize = vec2( 860, 718 ); // ( width, height ) of the viewport in pixel

uniform vec3 uToonColors[6];	//0-4 diffuse, 5 specular

uniform int uToonFunction = 0; //0: old version => map light to fixed color version; 1: set discrete light values and reduce colors

uniform float uLightingSteps = 6.0;
uniform float uHSV_H_Steps = 20.0;
uniform float uHSV_S_Steps = 10.0;
uniform float uHSV_V_Steps = 10.0;

in vec2 texCoord;

out vec4 fragColor;

//conversion routines from:
//http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


float discretize(float value, float steps)
{
    return clamp( floor( value * steps + 1.0) / steps,
                   0.0, 1.0);
}

vec4 discretizeVec4(vec4 values, vec4 steps)
{
    return clamp( floor( values * steps + 1.0) / steps,
                  0.0, 1.0);
}

vec3 discretizeVec3(vec3 values, vec3 steps)
{
    return clamp( floor( values * steps + 1.0) / steps,
                  0.0, 1.0);
}

void main(void)
{
    vec4 lighting = texture(uLightingTexture, texCoord);
    vec4 color = texture(uColorTexture, texCoord);

    //old ton version:
    if(uToonFunction == 0)
    {
	if(lighting.a > 0.9)
	    fragColor.rgb = uToonColors[5];
	else
	{
	    //map diffuse value to toon-color, stepsize = 0.2
	    fragColor.rgb = uToonColors[int(clamp(floor(lighting.b * 5.0f), 0.0, 4.0))];
	}
    }
    //alternate toon version with lighting
    else
    {

	fragColor.a = 1.0;

	/*
	lighting *= 6.0;
	lighting = ceil(lighting + 1.0);
	clamp(lighting /= 6.0,0.0,1.0);
	*/

	lighting.b = discretize(lighting.b, uLightingSteps);

	if(lighting.a > 0.9)
	    lighting.b = 1.5;

	//one idea of color-reduction:
	//-translate color into hsv space
	//-split up the hsv-space into smaller discrete regions
	//-convert back to rgb

	//would need more test data. Maybe add settings to give more control over reduction
	vec3 hsvColor = rgb2hsv(color.rgb);

	hsvColor = discretizeVec3(hsvColor, vec3(uHSV_H_Steps,
	                                         uHSV_S_Steps,
	                                         uHSV_V_Steps));

	color.rgb = hsv2rgb(hsvColor);
	fragColor.rgb = color.rgb * lighting.b;

    }
    fragColor.a = 1.0;
}

