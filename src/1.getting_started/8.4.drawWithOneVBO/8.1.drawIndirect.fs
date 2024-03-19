#version 400 core
layout (location = 0) out vec4 color;
uniform sampler2DArray textureArray;

in vec2 uv;
flat in uint drawID;
void main(void)
{
	if(drawID==0)
	{
		color = texture(textureArray, vec3(uv.x, uv.y, drawID));
	}
	else
	{
		color=vec4(1);
	}
		
	
	
	
}