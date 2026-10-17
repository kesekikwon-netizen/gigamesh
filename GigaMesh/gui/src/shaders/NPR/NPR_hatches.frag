#version 330

uniform sampler2D uLightingTexture;

uniform sampler2D uHatchMap0_2; //hatchmap 0 - 2 in rgb
uniform sampler2D uHatchMap3_5; //hatchmap 3 - 5 in rgb

uniform vec2 uViewPortSize = vec2( 860, 718 ); // ( width, height ) of the viewport in pixel
uniform vec2 uHatchMapSize = vec2( 500, 500 ); // ( width, height ) of the hatchmap
uniform vec2 uViewPortShift = vec2(0,0); //shift of the viewport during tiled rendering

uniform float uHatchTextureRotation = 0.0;
uniform float uHatchTextureScale = 1.0;

uniform int uLightInfluence;

uniform int uHatchDitherStyle = 0;
uniform float uOutlineWidth;
uniform vec4 uHatchColor = vec4(1.0,0.0,0.0,1.0);

// Archaeological cortex rendering (Raczynski-Henk 2017)
uniform int uIsCortexRegion = 0;      // 0=normal, 1=cortex region
uniform sampler2D uCortexPattern;      // Cross-hatch texture for cortex
uniform float uCortexScale = 1.0;      // Tiling scale for cortex pattern

in vec2 texCoord;         //texture coord

float bayerDither(int x, int y, float c0)
{
    float dither[64] = float[64](
         0, 32, 8, 40, 2, 34, 10, 42,   // 8x8 Bayer ordered dithering
        48, 16, 56, 24, 50, 18, 58, 26, // pattern.
        12, 44, 4, 36, 14, 46, 6, 38,
        60, 28, 52, 20, 62, 30, 54, 22,
         3, 35, 11, 43, 1, 33, 9, 41,
        51, 19, 59, 27, 49, 17, 57, 25,
        15, 47, 7, 39, 13, 45, 5, 37,
        63, 31, 55, 23, 61, 29, 53, 21 );

    return step((dither[(x & 7)+ (y & 7) * 8])/64.0, c0);
}

//simple pseudo-random dither
//TODO: find a better RNG!
float randomDither(int x, int y, float c0)
{

    float sin_val = fract(sin(dot(vec2(x,y) ,vec2(12.9898,78.233))) * 43758.5453);
    return step(sin_val, c0);
}


out vec4 fragColor;


void main(void) {
    float dx = 1.0 / uViewPortSize.x;
    float dy = 1.0 / uViewPortSize.y;

    vec2 outlineOffset = 2 * floor(vec2(uOutlineWidth, uOutlineWidth));

    float c = 1.0;
    vec4 lighting = texture(uLightingTexture,texCoord);
    float cosTheta = lighting.r;
    float specular = lighting.g;

    // Cortex cross-hatch rendering (Raczynski-Henk 2017 Section 2.7)
    if(uIsCortexRegion == 1 && !(specular > 0)) {
        // Calculate UV coordinates for cortex pattern
        vec2 UVCortex = (floor(gl_FragCoord.xy) - uViewPortShift * (uViewPortSize - outlineOffset))
                        / uHatchMapSize * uCortexScale;

        // No rotation for cortex - texture already has 45° cross-hatch baked in

        // Sample cortex cross-hatch texture
        float cortexIntensity = texture(uCortexPattern, UVCortex).r;

        // Apply lighting influence to cortex pattern
        // Lighter areas show less cross-hatching, darker areas show more
        float cortexShading = mix(cortexIntensity, 1.0, cosTheta * 0.5);

        c = cortexShading;

        // Use lighter color for cortex (weathered surface appearance)
        fragColor.rgb = vec3(0.85, 0.82, 0.75); // Warm gray tone
        if(uLightInfluence == 0)
            cosTheta = 0.0;

        fragColor.a = clamp((1.0 - c) * (1.0 - min(cosTheta, 0.9)), 0.0, 1.0);
        return; // Skip normal hatching
    }

    if(uHatchDitherStyle <= 1 && !(specular > 0) ) //hatching with texture, skip if specular highlight
    {

	vec2 UVHatch = (((floor(gl_FragCoord.xy)
                          - uViewPortShift * (uViewPortSize - outlineOffset)))  //OutlineWidth is needed here for tiled rendering
                        / uHatchMapSize) //scale to fit texture coordinates
                * uHatchTextureScale;   //scale up or down
        float rot = 2 * 3.14 * uHatchTextureRotation / 360.0;  //rotate

        mat2x2 rotHatch = mat2x2(cos(  rot) , -sin(rot) , sin( rot) , cos( rot));

        UVHatch = rotHatch * UVHatch;

	vec4 hatchIntesities[2];
        hatchIntesities[0] = texture(uHatchMap0_2, UVHatch);
        hatchIntesities[1] = texture(uHatchMap3_5, UVHatch);

        float step = 1./ 6.;

        if( cosTheta <= step ){
	    c = mix( hatchIntesities[1].b, hatchIntesities[1].g, 6. * cosTheta );
        }
        if( cosTheta > step && cosTheta <= 2. * step ){
	    c = mix( hatchIntesities[1].g, hatchIntesities[1].r , 6. * ( cosTheta - step ) );
        }
        if( cosTheta > 2. * step && cosTheta <= 3. * step ){
	    c = mix( hatchIntesities[1].r, hatchIntesities[0].b, 6. * ( cosTheta - 2. * step ) );
        }
        if( cosTheta > 3. * step && cosTheta <= 4. * step ){
	    c = mix( hatchIntesities[0].b, hatchIntesities[0].g, 6. * ( cosTheta - 3. * step ) );
        }
        if( cosTheta > 4. * step && cosTheta <= 5. * step ){
	    c = mix( hatchIntesities[0].g, hatchIntesities[0].r, 6. * ( cosTheta - 4. * step ) );
        }
	if( cosTheta > 5. * step && cosTheta <= 1.0){
	    c = mix( hatchIntesities[0].r,  1.0 , 6. * ( cosTheta - 5. * step ) );
        }
    }
    else if(uHatchDitherStyle == 2) //random dithering
    {
        vec2 UVDither = floor(gl_FragCoord.xy) - uViewPortShift * (uViewPortSize - outlineOffset );
        c = randomDither(int(UVDither.x), int(UVDither.y), cosTheta);
    }

    else if(uHatchDitherStyle == 3) //bayer dithering
    {
        vec2 UVDither = floor(gl_FragCoord.xy) - uViewPortShift * (uViewPortSize - outlineOffset );
        c = bayerDither(int(UVDither.x), int(UVDither.y), cosTheta);
    }

    fragColor.rgb = uHatchColor.rgb;
    if(uLightInfluence == 0)
	cosTheta = 0.0;

    fragColor.a = clamp( (1.0 - c) * (1.0 - min(cosTheta, 0.9)) ,0.0,1.0);
}
