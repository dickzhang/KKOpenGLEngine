#version 430 core

in vec2 texcoord[9];
in vec2 texuv;
uniform sampler2D mainTex;
uniform vec4 edgecolor;
uniform vec4 backgroundColor;
uniform float edgeOnly;
uniform int showMode;
out vec4 FragColor;

//灰度图
float luminance(vec4 color) 
{
    return  0.2125 * color.r + 0.7154 * color.g + 0.0721 * color.b; 
}

float Sobel()
{
float Gx[9] = {-1.0,  0.0,  1.0,
	-2.0,  0.0,  2.0,
	-1.0,  0.0,  1.0};

float Gy[9] = {-1.0, -2.0, -1.0,
	0.0,  0.0,  0.0,
	1.0,  2.0,  1.0};

	float texColor;
	float edgeX = 0.0;
	float edgeY = 0.0;
	for (int it = 0; it < 9; it++) {
		texColor = luminance(texture(mainTex, texcoord[it]));
		edgeX += texColor * Gx[it];
		edgeY += texColor * Gy[it];
	}
	// 合并横向和纵向	
	float edge = 1.0 - abs(edgeX) - abs(edgeY);
	return edge;
}

void main()
{
	float edge = Sobel();
	vec4 withEdgeColor = mix(edgecolor, texture(mainTex, texcoord[4]), edge);
	vec4 onlyEdgeColor = mix(edgecolor, backgroundColor, edge);
	if(showMode==0)
	{
		FragColor = texture(mainTex, texcoord[4]);
	}else if(showMode==1)
	{
		FragColor=mix(withEdgeColor, onlyEdgeColor, edgeOnly);
		
	}else
	{
		FragColor = mix(texture(mainTex, texcoord[4]),withEdgeColor, edgeOnly);
	}
};