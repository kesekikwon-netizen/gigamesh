#version 330

in vec4 vertexColor;
in vec3 vertexNormal;
in vec3 halfWay;
out vec4 frag_color;

void main()
{
    //compute fake lighting...
    vec3 vN = normalize(vertexNormal);

    vec3 lightDir = normalize(vec3(1.0,1.0,1.0));

    float diffuse = max(dot(vN, lightDir), 0.5) + 0.1 * clamp(dot(vN, -lightDir),0.0, 1.0);

    vec3 specular = vec3(pow(max(dot( vN, halfWay), 0.0), 10.0));

    frag_color.rgb = (vertexColor.rgb * diffuse) + specular;
    frag_color.a = 1.0;
}
