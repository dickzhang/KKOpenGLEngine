#version 330 core

layout (location = 0)in vec3 inPosition;
layout (location = 1)in vec2 inTexCoord0;

uniform mat4 mvpMatrix;
out vec2 v_texcoord0;

void main(void)
{
	v_texcoord0 = inTexCoord0;
	gl_Position = mvpMatrix * vec4(inPosition,1.0);
}