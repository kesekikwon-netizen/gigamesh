#version 330

uniform sampler2D uFBO_Texture_ID;

uniform sampler2D uHatchMap0_2; //hatchmap 0 - 2 in rgb
uniform sampler2D uHatchMap3_5; //hatchmap 3 - 5 in rgb
/*uniform sampler2D uHatchMap2;
uniform sampler2D uHatchMap3;
uniform sampler2D uHatchMap4;
uniform sampler2D uHatchMap5;*/

uniform vec2 uViewPortSize = vec2( 860, 718 ); // ( width, height ) of the viewport in pixel
uniform vec2 uHatchMapSize = vec2( 500, 500 ); // ( width, height ) of the hatchmap
uniform vec2 uViewPortShift = vec2(0,0); //shift of the viewport during tiled rendering

uniform float uOutlineWidth = 1.0; //linewidth of the outlines

uniform float uHatchTextureRotation = 0.0;
uniform float uHatchTextureScale = 1.0;

uniform float uOutlineThreshold = 0.3;

uniform int uEnableFlags = 15; //Bit 0 = Outlines, Bit 1 = Hatchlines, Bit 2 = ToonShading, Bit 3 = Intensity influences hatches
uniform int uHatchDitherStyle = 0;

//matrial properties
//----------------------------------------------------------
uniform vec4 uSpecularColor = vec4(1.0,1.0,1.0,1.0);

uniform vec4 uDiffuseColor1 = vec4(0.8,0.8,0.8,1.0);
uniform vec4 uDiffuseColor2 = vec4(0.6,0.6,0.6,1.0);
uniform vec4 uDiffuseColor3 = vec4(0.4,0.4,0.4,1.0);
uniform vec4 uDiffuseColor4 = vec4(0.2,0.2,0.2,1.0);
uniform vec4 uDiffuseColor5 = vec4(0.1,0.1,0.1,1.0);

uniform vec4 uOutlineColor = vec4(0.0,0.0,0.0,1.0);
uniform vec4 uHatchColor = vec4(0.0,0.0,0.0,1.0);

//----------------------------------------------------------
/*
in struct grVertex {
        vec4  ec_pos;        // eye coordinate position
} oVertex;
*/

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

void main(void)
{
    //calculate the uv-coordinates of the textures in screen space
    vec2 UV = texCoord;//gl_FragCoord.xy / uViewPortSize;

    fragColor = texture(uFBO_Texture_ID, UV); //store sobel and light intensity temporary in fragColor

    if(fragColor.b == 1.0) //discard fragment if it is on the backplane -> needed for tiled rendering to crop image correctly
        discard;           //note for future: don't do it if paper background or something similar should be used, then other cropping than alpha has to be used
                           //e.g. setting fragCoord.z to 1.0, as it causes also the tiled render to crop theese regions
    float dx = 1.0 / uViewPortSize.x;
    float dy = 1.0 / uViewPortSize.y;


    //find edges
    //----------------------------------------------------------

    //to increase the width of the outline, sample multiple times with different resolutions
    float intensity = 0.0;
    if((uEnableFlags & 1) == 1)    //check if outlines should be rendered
    {
        float width = uOutlineWidth;
        intensity = fragColor.r;
        //performance heavy... Probably better to unroll loop...
        for(float i = floor(-width); i <= ceil(width); i += 1.0)
        {
            //two possible ideas for the gradient value: either take the max from all the samples or the average
            for(float j = floor(-width); j<= ceil(width); j += 1.0)
            {
                if(length( vec2(i, j) ) <= width)
                    intensity = max(intensity, texture(uFBO_Texture_ID, UV + vec2(i * dx, j*dy)).r );
            }
        }
    }
    //thresholding for intensities?

    if(intensity <= uOutlineThreshold)
        intensity = 0.0;
    //-----------------------------------------------------------

    //calculate lighting model
    //-----------------------------------------------------------
    float cosTheta = fragColor.a;//texture(uFBO_Texture_ID, UV).a; //light intensity is coded in the alpha value
    fragColor.a = 1.0;

    cosTheta /= 0.9;//scale it back up to [0.0 ; 1.0]
    //simple toon-shading. Devide Diffuse Color in 5 regions and choose the assigned color for it.
    //--> for the future, let the user choose the number of regions and ranges
    fragColor.rgb = vec3(1.0,1.0,1.0); //sobel Intensity is not needed anymore

    if((uEnableFlags & 4) == 4 ) //check if toon-shading is enabled
    {
        if(cosTheta > 1.1)
           fragColor.rgb = uSpecularColor.rgb;

        else if(cosTheta >= 0.7)
           fragColor.rgb = uDiffuseColor1.rgb;

        else if(cosTheta >= 0.5)
            fragColor.rgb = mix(uDiffuseColor2.rgb, uDiffuseColor1.rgb, pow((cosTheta - 0.5) / 0.2 , 100));

        else if(cosTheta >= 0.3)
            fragColor.rgb = mix(uDiffuseColor3.rgb, uDiffuseColor2.rgb, pow((cosTheta - 0.3) / 0.2, 100));

        else if(cosTheta >= 0.1)
            fragColor.rgb = mix(uDiffuseColor4.rgb, uDiffuseColor3.rgb, pow((cosTheta - 0.1) / 0.2 , 100));

        else
            fragColor.rgb = mix(uDiffuseColor5.rgb, uDiffuseColor4.rgb, pow(cosTheta / 0.1 , 100));
    }
    //--------------------------------------------------------------

    //alternate approach with multiple hatch-levels
    //--------------------------------------------------------------
    cosTheta = min(cosTheta, 1.0);
    if((uEnableFlags & 2) == 2)
    {
        vec2 outlineOffset = 2 * floor(vec2(uOutlineWidth, uOutlineWidth));

        float c = 1.0;
        if(uHatchDitherStyle <= 1) //hatching with texture
        {

            vec2 UVHatch = (((floor(gl_FragCoord.xy)
                              - uViewPortShift * (uViewPortSize - outlineOffset)))  //OutlineWidth is needed here for tiled rendering
                            / uHatchMapSize) //scale to fit texture coordinates
                    * uHatchTextureScale;   //scale up or down
            float rot = 2 * 3.14 * uHatchTextureRotation / 360.0;  //rotate

            mat2x2 rotHatch = mat2x2(cos(  rot) , -sin(rot) , sin( rot) , cos( rot));

            UVHatch = rotHatch * UVHatch;


            float step = 1./ 6.;


            if( cosTheta <= step ){
                c = mix( texture( uHatchMap3_5, UVHatch ).b, texture( uHatchMap3_5, UVHatch ).g, 6. * cosTheta );
            }
            if( cosTheta > step && cosTheta <= 2. * step ){
                c = mix( texture( uHatchMap3_5, UVHatch ).g, texture( uHatchMap3_5, UVHatch).r , 6. * ( cosTheta - step ) );
            }
            if( cosTheta > 2. * step && cosTheta <= 3. * step ){
                c = mix( texture( uHatchMap3_5, UVHatch ).r, texture( uHatchMap0_2, UVHatch ).b, 6. * ( cosTheta - 2. * step ) );
            }
            if( cosTheta > 3. * step && cosTheta <= 4. * step ){
                c = mix( texture( uHatchMap0_2, UVHatch ).b, texture( uHatchMap0_2, UVHatch ).g, 6. * ( cosTheta - 3. * step ) );
            }
            if( cosTheta > 4. * step && cosTheta <= 5. * step ){
                c = mix( texture( uHatchMap0_2, UVHatch ).g, texture( uHatchMap0_2, UVHatch ).r, 6. * ( cosTheta - 4. * step ) );
            }
            if( cosTheta > 5. * step && cosTheta <= 1.1){
                c = mix( texture( uHatchMap0_2, UVHatch ).r,  1.0 , 6. * ( cosTheta - 5. * step ) );
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

        vec3 Color = mix(uHatchColor.xyz , fragColor.rgb , c);
        //mix in the hatch-texture based on the diffuse strength cosTheta
        if((uEnableFlags & 8) == 8)
        {
            Color = mix( Color, fragColor.rgb, min(cosTheta,1.0)  );
        }

        fragColor.xyz = Color;
    }
    //--------------------------------------------------------------------

    fragColor.xyz = mix(fragColor.xyz, uOutlineColor.rgb, intensity); //mix in outlines
}

//experimental code that is not working, but maybe interesting in the future...

/*
vec2 RidgeDetect(vec2 UV, float dx,float  dy)
{
    //From "Line drawings via abstracted shading", Y. Lee, L. Markosian, S. Lee, J.F. Hughes
    // A  = (X^T * X)^-1 * X^T * T
    // (X^T * X)^-1 * X =
    //  |  0.166666 -0.333333  0.166666  0.166666 -0.333333  0.166666  0.166666 -0.333333  0.166666 |
    //  |  0.125000  0.000000 -0.125000  0.000000  0.000000  0.000000 -1.250000  0.000000  0.125000 |
    //  |  0.166666  0.166666  0.166666 -0.333333 -0.333333 -0.333333  0.166666  0.166666  0.166666 |
    //  | -0.166666  0.000000  0.166666 -0.166666  0.000000  0.166666 -0.166666  0.000000  0.166666 |
    //  | -0.166666 -0.166666 -0.166666  0.000000  0.000000  0.000000  0.166666  0.166666  0.166666 |
    //  | -0.111111  0.222222 -0.111111  0.222222  0.555555  0.222222 -0.111111  0.222222 -0.111111 |

      T = Texture values from (x,y) = (-1,-1) to (1,1) from bottom left to top right



    //Calculation of A

    float T0 = texture(uFBO_Texture_ID, vec2(UV.x - dx, UV.y - dy)).x;
    float T1 = texture(uFBO_Texture_ID, vec2(UV.x     , UV.y - dy)).x;
    float T2 = texture(uFBO_Texture_ID, vec2(UV.x + dx, UV.y - dy)).x;
    float T3 = texture(uFBO_Texture_ID, vec2(UV.x - dx, UV.y     )).x;
    float T4 = texture(uFBO_Texture_ID, vec2(UV.x     , UV.y     )).x;
    float T5 = texture(uFBO_Texture_ID, vec2(UV.x + dx, UV.y     )).x;
    float T6 = texture(uFBO_Texture_ID, vec2(UV.x - dx, UV.y + dy)).x;
    float T7 = texture(uFBO_Texture_ID, vec2(UV.x     , UV.y + dy)).x;
    float T8 = texture(uFBO_Texture_ID, vec2(UV.x + dx, UV.y + dy)).x;

    float a0 =  0.166666 * T0 - 0.333333 * T1 + 0.166666 * T2 + 0.166666 * T3 - 0.333333 * T4 + 0.166666 * T5 + 0.166666 * T6 - 0.333333 * T7 + 0.166666 * T8;
    float a1 =  0.125000 * T0                 - 0.125000 * T2                                                 - 1.250000 * T6                 + 0.125000 * T8;
    float a2 =  0.166666 * T0 + 0.166666 * T1 + 0.166666 * T2 - 0.333333 * T3 - 0.333333 * T4 - 0.333333 * T5 + 0.166666 * T6 + 0.166666 * T7 + 0.166666 * T8;
    float a3 = -0.166666 * T0                 + 0.166666 * T2 - 0.166666 * T3                 + 0.166666 * T5 - 0.166666 * T6                 + 0.166666 * T8;
    float a4 = -0.166666 * T0 - 0.166666 * T1 - 0.166666 * T2                                                 + 0.166666 * T6 + 0.166666 * T7 + 0.166666 * T8;
    //a5 is not needed in further calculations...

    mat2 M = mat2(a0,a1,a1,a3);
    vec2 C = -0.5 * inverse(M) * vec2(a3,a4);

    //ITS COMPLETE UNTIL HERE, THE REST IS INCOMPLETE!

    vec2 Q = (UV - C) * M * (UV - C);
    for(int i = 0; i<5; ++i)
    {
    //Calculate eigenvalues and eigenvectors of M via Jacobi rotation
    float theta = (M[1][1] - M[0][0]) / (2 * M[0][1]);
    float t1 = -theta + sqrt(theta * theta + 1);
    float t2 = -theta - sqrt(theta * theta + 1);

    float t;
    if(abs(t1) < abs(t2))
         t = t1;
    else
        t = t2;

    float c = 1.0 / sqrt(1.0 + t * t);
    float s = c * t;

    float eig1 = (c * c * M[0][0]) - (2.0 * c * s * M[0][1]) + (s * s * M[1][1]);
    float eig2 = (c * c * M[1][1]) + (1.0 * c * s * M[0][1]) + (s * s * M[0][0]);

    vec2 ev1 = vec2(c, -s);
    vec2 ev2 = vec2(s,c);

    }

    return Q;
}

*/
