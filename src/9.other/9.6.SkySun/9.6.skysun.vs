#version 330 core

layout (location = 0)in vec3 inPosition;
layout (location = 1)in vec2 inTexCoord0;

uniform mat4 uMvpMatrix;
uniform mat4 uWorldMatrix;

out vec2 varTexCoord0;
out vec4 varWorldPos;

void main(void)
{
	varTexCoord0 = inTexCoord0;
	varWorldPos = uWorldMatrix * vec4(inPosition,1.0);
	gl_Position = uMvpMatrix * vec4(inPosition,1.0);
}