#version 430

uniform vec2 uViewPortSize;
uniform int NUM_LAYERS = 10;

layout (std430, binding = 0) buffer DepthArray
{
  uint depthVals[];
};

void main(void) {
    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 screenSize = ivec2(uViewPortSize);

    if(coords.x>=0 && coords.y>=0 && coords.x<screenSize.x && coords.y<screenSize.y ){

	//uint offset = coords.x * NUM_LAYERS + coords.y * screenSize.x * NUM_LAYERS;
	uint offset = coords.x + coords.y * screenSize.x;
	uint pageSize = screenSize.x * screenSize.y;

	uint ztest = floatBitsToUint(gl_FragCoord.z);
        //for speedup test against last/middle element of list
        //if too far, skip below or start at middle
	int i = ztest >= depthVals[offset + (NUM_LAYERS - 1) * pageSize] ? NUM_LAYERS :
		      ztest >= depthVals[offset + (NUM_LAYERS / 2) * pageSize] ? NUM_LAYERS / 2 : 0;

	for(; i< NUM_LAYERS; ++i)
        {
	    uint zold = atomicMin(depthVals[offset + i * pageSize], ztest);
            if(zold == 0xFFFFFFFFu || zold == ztest){
                break;
            }
            ztest = max(zold, ztest);
        }

    }

    discard;
}
