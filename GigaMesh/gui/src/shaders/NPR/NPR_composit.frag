#version 330


uniform sampler2D uColorTexture;
uniform sampler2D uHatchTexture;
uniform sampler2D uEdgeTexture;
uniform sampler2D uDepthTexture;

uniform vec2 uViewPortSize = vec2( 860, 718 ); // ( width, height ) of the viewport in pixel
uniform int uEnableFlags = 15; //Bit 0 = Outlines, Bit 1 = Hatchlines, Bit 2 = ToonShading, Bit 3 = Intensity influences hatches
uniform vec3 uOutlineColor;
uniform float uOutlineThreshold = 0.0;
uniform float uOutlineWidth = 4.0;

in vec2 texCoord;

out vec4 fragColor;



//takes the different layers of NPR-Textures and mixes them together

void main(void)
{
    vec4 depth = texture(uDepthTexture, texCoord);


    float dx = 1.0 / uViewPortSize.x;
    float dy = 1.0 / uViewPortSize.y;

    fragColor = vec4(1.0,1.0,1.0,1.0);


    //Add Toon
    if((uEnableFlags & 4) != 0)
    {
        vec4 toonColor = texture(uColorTexture, texCoord);
	fragColor.rgb = toonColor.rgb;
    }

    //Add Hatches
    if((uEnableFlags & 2) != 0)
    {
        vec4 hatch = texture(uHatchTexture, texCoord);

	fragColor.rgb = mix(fragColor.rgb, hatch.rgb, hatch.a);
    }

    //Add outlines
    if((uEnableFlags & 1) != 0)
    {
	float edgeIntensity = 0.0;
	for(float i = floor(-uOutlineWidth); i<= ceil(uOutlineWidth); i += 1.0)
	{
	    for(float j = floor(-uOutlineWidth); j <= ceil(uOutlineWidth); j += 1.0)
	    {
		if(length( vec2(i,j) ) <= uOutlineWidth)
		{
                    vec3 edge = texture(uEdgeTexture, texCoord + vec2(i*dx, j*dy)).rga;
		    edge.z *= 2.0;
		    edgeIntensity = max(edgeIntensity, max(edge.x, max(edge.y, edge.z)));
		}
	    }
	}

	if(edgeIntensity <= uOutlineThreshold)
	    edgeIntensity = 0.0;

	fragColor.rgb = mix(fragColor.rgb, uOutlineColor, edgeIntensity);

        //hack to not discard outlines outside of the mesh => ideally this should be the depth of the fragment the edge belongs to
        if(edgeIntensity != 0.0 && depth.r == 1.0)
            depth.r = 0.5;
    }

    //have to discard late, because of outline-check, otherwise it would be better to discard asap
    if(depth.r == 1.0)
        discard;
    else
        gl_FragDepth = depth.r;

}

