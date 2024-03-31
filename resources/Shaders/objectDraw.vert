#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 modelViewProj;
uniform mat4 model;
uniform mat4 lightModelViewProj;
uniform mat4 view;

out vec3 Normal;
out vec3 FragPosWorldSpace;
out vec2 TexCoords;
out vec4 FragPosViewSpace;

void main()
{
    vec4 pos = vec4(aPos, 1.0);
    
    FragPosWorldSpace = vec3(model * pos);
    FragPosViewSpace = view * pos;

    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = modelViewProj * pos;
}