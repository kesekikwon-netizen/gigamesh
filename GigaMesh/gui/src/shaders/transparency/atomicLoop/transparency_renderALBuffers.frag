#version 430

uniform vec2 uViewPortSize;
uniform int NUM_LAYERS = 10;
uniform int uOverflowHandling = 0;

layout (std430, binding = 0) buffer DepthArray
{
  uint depthVals[];
};

layout (std430, binding = 1) buffer ColorArray
{
  //packed:
  uint colorVals[];
  //not packed:
  //vec4 colorVals[];
};

uniform sampler2DRect ColorTex0;
uniform sampler2DRect ColorTex1;

out vec4 fragColor;

//-------------------------------
void main(void) {
    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 screenSize = ivec2(uViewPortSize);

    fragColor = vec4(.0,.0,.0,.0);
    //int numLayers = NUM_LAYERS - uOverflowHandling;
    if(coords.x>=0 && coords.y>=0 && coords.x<screenSize.x && coords.y<screenSize.y ){

	//uint offset = coords.x * NUM_LAYERS + coords.y * screenSize.x * NUM_LAYERS;
	uint offset = coords.x + coords.y * screenSize.x;
	uint pageSize = screenSize.x * screenSize.y;

        if(depthVals[offset] == 0xFFFFFFFFu)
        {
            discard;
        }

        else
        {
            //front to back blending --> better due to early termination
            for(int i = 0; i< NUM_LAYERS; ++i)
            {
                //terminate early
		if(depthVals[offset + i * pageSize] == 0xFFFFFFFFu)
                    break;
                //reset depth
		depthVals[offset + i * pageSize] = 0xFFFFFFFFu;

		//packed:
		vec4 color = unpackUnorm4x8(colorVals[offset + i * pageSize]);
		colorVals[offset + i * pageSize] = 0;

		//not packed:
		//vec4 color = colorVals[offset + i * pageSize];
		//colorVals[offset + i * pageSize] = vec4(0.0,0.0,0.0,0.0);

	       fragColor.rgb = fragColor.rgb + color.rgb * (1.0 - fragColor.a);
	       fragColor.a = fragColor.a + color.a * (1.0 - fragColor.a);


            }

	    if(uOverflowHandling == 1)
	    {
		vec4 sumColor = texture(ColorTex0, gl_FragCoord.xy);
		float transmittance = texture(ColorTex1, gl_FragCoord.xy).r;
		vec3 averageColor = sumColor.rgb / max(sumColor.a, 0.00001);
		vec4 tailColor = vec4(averageColor, 1.0 - transmittance);


		fragColor.rgb = fragColor.rgb + tailColor.rgb * (1.0 - fragColor.a);
		fragColor.a = fragColor.a + tailColor.a * (1.0 - fragColor.a);
	    }

            if(fragColor.a == 0.0)
                discard;
        }
    }
    else
    {
        discard;
    }
}
