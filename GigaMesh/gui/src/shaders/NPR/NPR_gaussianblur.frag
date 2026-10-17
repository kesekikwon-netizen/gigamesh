#version 330

uniform sampler2D uFBO_Texture_ID;
uniform vec2 uViewPortSize = vec2( 860, 718 ); // ( width, height ) of the viewport in pixel

in vec2 texCoord;

out vec4 fragColor;



void main(void) {
    float dx = 1.0 / uViewPortSize.x;
    float dy = 1.0 / uViewPortSize.y;

    //3x3 gauss filter
    fragColor =
       (texture(uFBO_Texture_ID, vec2(texCoord.x -dx, texCoord.y - dy)) + 2 * texture(uFBO_Texture_ID, vec2(texCoord.x, texCoord.y - dy)) +     texture(uFBO_Texture_ID, vec2(texCoord.x + dx, texCoord.y - dy))
   + 2* texture(uFBO_Texture_ID, vec2(texCoord.x -dx, texCoord.y     )) + 4 * texture(uFBO_Texture_ID, vec2(texCoord.x, texCoord.y     )) + 2 * texture(uFBO_Texture_ID, vec2(texCoord.x + dx, texCoord.y     ))
   +    texture(uFBO_Texture_ID, vec2(texCoord.x -dx, texCoord.y + dy)) + 2 * texture(uFBO_Texture_ID, vec2(texCoord.x, texCoord.y + dy)) +     texture(uFBO_Texture_ID, vec2(texCoord.x + dx, texCoord.y + dy))
                  ) / 16.0;
}
