#version 330

uniform sampler2DRect ColorTex0;
uniform sampler2DRect ColorTex1;

layout(location = 0) out vec4 outColor;


//-------------------------------
void main(void) {
    vec4 sumColor = texture(ColorTex0, gl_FragCoord.xy);
    float transmittance = texture(ColorTex1, gl_FragCoord.xy).r;
    vec3 averageColor = sumColor.rgb / max(sumColor.a, 0.00001);
    outColor = vec4(averageColor, 1.0 - transmittance);

    if(transmittance == 1.0)
	discard;
}
