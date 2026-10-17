#version 430

uniform vec2 uViewPortSize;
uniform int NUM_LAYERS = 10;

layout (std430, binding = 0) buffer DepthArray
{
  uint depthVals[];
};

/*
layout (std430, binding = 1) buffer ColorArray
{
  vec4 colorVals[];
};
*/
//-------------------------------
void main(void) {
    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 screenSize = ivec2(uViewPortSize);

    if(coords.x>=0 && coords.y>=0 && coords.x<screenSize.x && coords.y<screenSize.y ){

	//uint offset = coords.x * NUM_LAYERS + coords.y * screenSize.x * NUM_LAYERS;
	uint offset = coords.x + coords.y * screenSize.x;
	uint pageSize = screenSize.x * screenSize.y;

        for(int i = 0; i< NUM_LAYERS; ++i)
        {
	    depthVals[offset + i * pageSize] = 0xFFFFFFFFu;
            //colorVals[offset + i] = vec4(.0,.0,.0,.0);
        }
    }

    discard;
}
