#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float nearPlane;
uniform float farPlane;

float LinearizeDepth(float depth) 
{
    return (2.0 *  nearPlane * farPlane) / (farPlane +  nearPlane - depth * (farPlane -  nearPlane));	
}

void main()
{
    float depth = LinearizeDepth(texture(depthMap, TexCoords).r) / farPlane; // divide by far for demonstration
    FragColor = vec4(vec3(depth), 1.0);
}