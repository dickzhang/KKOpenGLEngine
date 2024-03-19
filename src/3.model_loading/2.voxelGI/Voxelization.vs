#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out Vertex
{
    vec2 texCoord;
    vec3 normal;
};
uniform mat4 model;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
    normal = mat3(transpose(inverse(model))) * aNormal;
    normal = normalize(normal);
    texCoord = aTexCoords;    
}