#version 430

//uniform sampler2D Texture_ID;  // Texturemap holding an image e.g. logo.

uniform vec2 uViewPortSize;

//in vec4  ecPosition;           // Interpolated position of the fragment in eye coordinates.
//in vec2 texCoord;

struct TransListData {
    int pHead; //start of list
    uint numberOfFragments; //total number of fragments... not strictly necessary
};

layout (std430, binding = 2) buffer ListHeads
{
  TransListData fragLists[];
};


//-------------------------------
void main(void) {
    ivec2 coords = ivec2(gl_FragCoord.xy);
    ivec2 screenSize = ivec2(uViewPortSize);

    //fragmentCounter = 0;

    if(coords.x>=0 && coords.y>=0 && coords.x<screenSize.x && coords.y<screenSize.y ){
        fragLists[coords.x + coords.y * screenSize.x].pHead = -1;
        fragLists[coords.x + coords.y * screenSize.x].numberOfFragments = 0;
    }

    discard;
}
