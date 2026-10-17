#version 150

uniform sampler2D uFBO_Texture_ID;
uniform vec2 uViewPortSize = vec2( 860, 718 ); // ( width, height ) of the viewport in pixel

uniform float upperThreshold = 0.995;
uniform float lowerThreshold = 0.1;

in vec2 texCoord;

out vec4 fragColor;



void main(void) {

    vec4 lightInfo = texture(uFBO_Texture_ID, texCoord);
    float maxLight = max(lightInfo.r, max(lightInfo.g,lightInfo.b));

    if(lightInfo.a < .5 || (maxLight < upperThreshold && maxLight > lowerThreshold))
    {
	discard;
    }

    fragColor.rgb = mix(vec3(1.0,0.0,0.0), vec3(0.0,1.0,0.0), bvec3(maxLight < upperThreshold));

    //add pretty stripes
    fragColor.a = mix(0.4, 0.9, mod(gl_FragCoord.x - gl_FragCoord.y,10) == 0.0);
}
