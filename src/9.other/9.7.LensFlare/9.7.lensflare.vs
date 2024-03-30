#version 330 core

layout (location = 0)in vec3 inPosition;
layout (location = 1)in vec2 inTexCoord0;

out vec2 texCoord;

void main(void)
{
	texCoord = inTexCoord0;
	gl_Position =vec4(inPosition,1.0);
}