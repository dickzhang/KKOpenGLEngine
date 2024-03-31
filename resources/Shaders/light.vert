#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 modelViewProj;
uniform mat4 modelView;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
    vec4 pos = vec4(aPos, 1.0);
    
    FragPos = vec3(modelView * pos);
    Normal = mat3(transpose(inverse(modelView))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = modelViewProj * pos;
}