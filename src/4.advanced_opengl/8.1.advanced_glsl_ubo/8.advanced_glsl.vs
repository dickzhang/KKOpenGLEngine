#version 330 core
#extension GL_ARB_shading_language_420pack : enable
layout (location = 0) in vec3 aPos;

struct Light
{
	mat4 aa;
    mat4 bb;
    mat4 cc;
    mat4 dd;
    mat4 ee;
};

layout (std140,binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

layout (std140,binding = 1) uniform tempMatrices
{
    mat4 aprojection;
    mat4 aview;
    Light aLight[200];
};

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}  