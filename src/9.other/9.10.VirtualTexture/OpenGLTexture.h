#pragma once
#include <stb_image.h>
#include <string>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include "Common.h"
#include "VTCommon.h"
using namespace std;

class OpenGLTexture
{
public:
	static unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
	static unsigned int loadCubemap(vector<std::string> faces);
	static unsigned int generateTexture2D(int w, int h);
	static unsigned int generateTexture3D(int w, int h, int d);
	static void bindTexture2D(unsigned int tex, int unit = 0);
	static unsigned int generateTexture2D(TextureInfo info, const Memory* _mem = NULL);
	static void updateTexture2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, const void* pixels);
};

