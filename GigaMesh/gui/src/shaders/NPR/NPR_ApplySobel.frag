#version 330

uniform sampler2D uFBO_Texture_ID;
uniform vec2 uViewPortSize = vec2( 860, 718 ); // ( width, height ) of the viewport in pixel

in vec2 texCoord;

out vec4 fragColor;
/*
//3x3 gauss kernel
vec3 Smooth_Gauss(vec2 UV, float dx, float dy)
{
    vec3 Color =
       (texture(uFBO_Texture_ID, vec2(UV.x -dx, UV.y - dy)).xyz + 2 * texture(uFBO_Texture_ID, vec2(UV.x, UV.y - dy)).xyz +     texture(uFBO_Texture_ID, vec2(UV.x + dx, UV.y - dy)).xyz
   + 2* texture(uFBO_Texture_ID, vec2(UV.x -dx, UV.y     )).xyz + 4 * texture(uFBO_Texture_ID, vec2(UV.x, UV.y     )).xyz + 2 * texture(uFBO_Texture_ID, vec2(UV.x + dx, UV.y     )).xyz
   +    texture(uFBO_Texture_ID, vec2(UV.x -dx, UV.y + dy)).xyz + 2 * texture(uFBO_Texture_ID, vec2(UV.x, UV.y + dy)).xyz +     texture(uFBO_Texture_ID, vec2(UV.x + dx, UV.y + dy)).xyz
    ) / 16.0;

    return Color;

}

vec3 smooth_Sobel(vec2 UV, float dx, float dy)
{
    vec3 Sx = -      Smooth_Gauss(UV + vec2(-dx,-dy), dx, dy)
              -2.0 * Smooth_Gauss(UV + vec2(-dx,  0), dx, dy)
              -      Smooth_Gauss(UV + vec2(-dx,+dy), dx, dy)
              +      Smooth_Gauss(UV + vec2(+dx,-dy), dx, dy)
              +2.0 * Smooth_Gauss(UV + vec2(+dx,- 0), dx, dy)
              +      Smooth_Gauss(UV + vec2(+dx,+dy), dx, dy);

    vec3 Sy = -      Smooth_Gauss(UV + vec2(-dx,-dy), dx, dy)
              -2.0 * Smooth_Gauss(UV + vec2(  0,-dy), dx, dy)
              -      Smooth_Gauss(UV + vec2(+dx,-dy), dx, dy)
              +      Smooth_Gauss(UV + vec2(-dx,+dy), dx, dy)
              +2.0 * Smooth_Gauss(UV + vec2(  0,+dy), dx, dy)
              +      Smooth_Gauss(UV + vec2(+dx,+dy), dx, dy);

    //boost depth
    Sx.z *= 2.0;
    Sy.z *= 2.0;

    vec3 grad_mag = sqrt(Sx * Sx + Sy * Sy);

    return grad_mag;
}
*/
void main(void) {
    //fragColor = texture( uFBO_Texture_ID, texCoord);

    float dx = 1.0 / uViewPortSize.x;
    float dy = 1.0 / uViewPortSize.y;


    //vec3 Sobel = smooth_Sobel(texCoord, dx , dy);

    //fragColor.r = clamp( max(Sobel.r, max(Sobel.g, Sobel.b)) , 0.0 , 1.0 );

    //Sobel x-direction
    vec4 Sx = -       texture(uFBO_Texture_ID, texCoord + vec2(-dx, -dy))
              - 2.0 * texture(uFBO_Texture_ID, texCoord + vec2(-dx,   0))
              -       texture(uFBO_Texture_ID, texCoord + vec2(-dx,  dy))
              +       texture(uFBO_Texture_ID, texCoord + vec2( dx, -dy))
              + 2.0 * texture(uFBO_Texture_ID, texCoord + vec2( dx,   0))
              +       texture(uFBO_Texture_ID, texCoord + vec2( dx,  dy));

    //Sobel y-direction
    vec4 Sy = -       texture(uFBO_Texture_ID, texCoord + vec2(-dx, -dy))
              - 2.0 * texture(uFBO_Texture_ID, texCoord + vec2(  0, -dy))
              -       texture(uFBO_Texture_ID, texCoord + vec2( dx, -dy))
              +       texture(uFBO_Texture_ID, texCoord + vec2(-dx,  dy))
              + 2.0 * texture(uFBO_Texture_ID, texCoord + vec2(  0,  dy))
              +       texture(uFBO_Texture_ID, texCoord + vec2( dx,  dy));

    fragColor = sqrt(Sx * Sx + Sy * Sy);
}
