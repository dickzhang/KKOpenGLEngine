#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;

uniform mat4 mvp;
uniform vec2 TexelSize;
out vec2 texcoord[9];
out vec2 texuv;

void main()
{
	texuv=uv;
	//计算周围像素的纹理坐标位置，其中4为原始点，
	texcoord[0] = uv + TexelSize.xy * vec2(-1.0, -1.0);
	texcoord[1] = uv + TexelSize.xy * vec2(0.0, -1.0);
	texcoord[2] = uv + TexelSize.xy * vec2(1.0, -1.0);
	texcoord[3] = uv + TexelSize.xy * vec2(-1.0, 0.0);
	texcoord[4] = uv + TexelSize.xy * vec2(0.0, 0.0);		//原点
	texcoord[5] = uv + TexelSize.xy * vec2(1.0, 0.0);
	texcoord[6] = uv + TexelSize.xy * vec2(-1.0, 1.0);
	texcoord[7] = uv + TexelSize.xy * vec2(0.0, 1.0);
	texcoord[8] = uv + TexelSize.xy * vec2(1.0, 1.0);

    gl_Position = mvp * vec4(aPos, 1.0);
}