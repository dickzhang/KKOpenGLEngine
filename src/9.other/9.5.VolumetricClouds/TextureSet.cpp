#include "TextureSet.h"
#include <glad/glad.h>
#include <iostream>

TextureSet::TextureSet(int W, int H, int num)
{
	if(W > 0 && H > 0 && num > 0)
	{
		nTextures = num;
		texture = new unsigned int[num];
		for(int i = 0; i < num; ++i) {
			texture[i] = generateTexture2D(W, H);
		}
	}
}

unsigned int TextureSet::getColorAttachmentTex(int i)
{
	if(i < 0 || i > nTextures) {
		std::cout << "COLOR ATTACHMENT OUT OF RANGE" << std::endl;
		return 0;
	}
	return texture[i];
}

void TextureSet::bindTexture(int i, int unit)
{
	bindTexture2D(texture[i], unit);
}

void TextureSet::bind()
{
	for(int i = 0; i < nTextures; ++i)
		bindTexture2D(texture[i], i);
}

unsigned int TextureSet::generateTexture2D(int w, int h)
{
	unsigned int tex_output;
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	return tex_output;
}

void TextureSet::bindTexture2D(unsigned int tex, int unit)
{
	glBindImageTexture(unit, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}