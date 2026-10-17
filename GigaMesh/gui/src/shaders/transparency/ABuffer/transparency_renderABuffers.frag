#version 430

uniform vec2 uViewPortSize;

// Output i.e. color of the fragment
out vec4 fragColor;

struct TransFragmentData {
    uint Color;      //color data
    float depth;        //depth for sorting
    int pNext; //pointer to next fragment in linked list
};

struct TransListData {
    int pHead; //start of list
    uint numberOfFragments;
};

layout (std430, binding = 1) buffer Fragment_Data
{
    TransFragmentData fragData[];
};


layout (std430, binding = 2) buffer Fragment_Lists
{
  TransListData fragList[];
};


void main(void) {
    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 screenSize = ivec2(uViewPortSize);
    fragColor = vec4(0,0,0,0);

    if(!(coords.x>=0 && coords.y>=0 && coords.x<screenSize.x && coords.y<screenSize.y) ){
	discard;
    }

    int listID = coords.x + coords.y * screenSize.x;
    int listHead = fragList[listID].pHead;
    uint fragCount = fragList[listID].numberOfFragments;

    if(fragCount == 0 || listHead == -1)
    {
	fragList[listID].pHead = -1;
	fragList[listID].numberOfFragments = 0;
	discard;
    }

    //selection sort as suggested by Carnecky et al.
    //performance bottle neck, if anything, this needs improvement if possible

    for(uint i = 0; i < fragCount ; ++i)
    {
        int currIndex = listHead;
        int maxIndex = currIndex;
        float maxDepth = fragData[currIndex].depth;

        for(uint j = 0; j < (fragCount - 1) - i; ++j)
        {
            currIndex = fragData[currIndex].pNext;

            if(fragData[currIndex].depth > maxDepth)
            {
                maxIndex = currIndex;
                maxDepth = fragData[currIndex].depth;
            }
        }

        //swap max depth fragment to the end
	uint maxRGBA = fragData[maxIndex].Color;

        fragData[maxIndex].Color = fragData[currIndex].Color;
        fragData[maxIndex].depth = fragData[currIndex].depth;

        fragData[currIndex].Color = maxRGBA;
        fragData[currIndex].depth = maxDepth;

	vec4 color = unpackUnorm4x8(maxRGBA);
        //mix max to fragColor: back-to-front
	fragColor.rgb = color.rgb + fragColor.rgb * (1.0 - color.a );
	fragColor.a = color.a + fragColor.a * (1 - color.a);
    }


    //render number of fragments...
    /*fragColor.r = min(float(fragList[listID].numberOfFragments) / 10.0 , 1.0);
    fragColor.g = clamp((float(fragList[listID].numberOfFragments) / 10.0) - 1.0 , 0.0, 1.0);
    fragColor.b = clamp((float(fragList[listID].numberOfFragments) / 10.0) - 2.0 , 0.0, 1.0);
    fragColor.a = 1.0;
    */

    //reset Buffer
    fragList[listID].pHead = -1;
    fragList[listID].numberOfFragments = 0;

}
