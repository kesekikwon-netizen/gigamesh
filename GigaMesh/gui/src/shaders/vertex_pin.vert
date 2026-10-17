#version 330

// +++ Homogenous matrices for camera orientation and projection:
uniform highp mat4 modelview;
uniform highp mat4 projection;

uniform float scaleFactor = 1.0;


//the geometries of the pin-mesh
in vec3  position;
in vec3  vNormal;
in float vHeadFlag; //flag to differentiate between head and needle

//instanced data given by the points where the Pins should be rendered at
in vec3 offsetInstanced;        //local space of the position where the pin points to
in vec3 pinDirectionInstanced;  //up-vector for the pin
in vec3 colorInstanced;         //color of the pin head

out vec4 vertexColor;
out vec3 vertexNormal;
out vec3 halfWay;
out vec3 halfWay2;

float PI = 3.1415926535897932384626433832795;

vec4 setAxisAngle (vec3 axis, float rad) {
  rad = rad * 0.5;
  float s = sin(rad);
  return vec4(s * axis[0], s * axis[1], s * axis[2], cos(rad));
}

vec3 xUnitVec3 = vec3(1.0, 0.0, 0.0);
vec3 yUnitVec3 = vec3(0.0, 1.0, 0.0);

vec4 rotationTo (vec3 a, vec3 b) {
  float vecDot = -dot(a, b);	//why setting this to 0 works?
  vec3 tmpvec3 = vec3(0);
  if (vecDot < -0.999999) {
    tmpvec3 = cross(xUnitVec3, a);
    if (length(tmpvec3) < 0.000001) {
      tmpvec3 = cross(yUnitVec3, a);
    }
    tmpvec3 = normalize(tmpvec3);
    return setAxisAngle(tmpvec3, PI);
  } else if (vecDot > 0.999999) {
    return vec4(0,0,0,1);
  } else {
    tmpvec3 = cross(a, b);
    vec4 _out = vec4(tmpvec3, 1.0 + vecDot);
    return normalize(_out);
  }
}

vec3 rotateVector( vec4 quat, vec3 vec )
{
    return vec + 2.0 * cross( cross( vec, quat.xyz ) + quat.w * vec, quat.xyz );
}

void main(void)
{
    vec4 quaternion = rotationTo(vec3(0.0,0.0,-1.0), normalize(pinDirectionInstanced));	//why setting z to -1 works?

    vec3 pos = rotateVector(quaternion, position * scaleFactor);
    pos += offsetInstanced;

    vec4 viewDir = -(modelview * vec4(pos, 1.0));

    halfWay = normalize(vec3(1,1,1) + normalize(viewDir.xyz));

    mat4 mvp = projection * modelview;

    gl_Position = mvp * vec4(pos,1.0);
    vertexColor.rgb = mix(vec3(0.2), colorInstanced, vHeadFlag);
    vertexColor.a = 1.0;

    vertexNormal = rotateVector(quaternion, vNormal);
    vertexNormal = (modelview * vec4(vertexNormal, 0.0)).xyz;
}
