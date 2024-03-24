#pragma once
#include <stb_image.h>
#include <string>
#include <iostream>
#include <vector>
#include <glad/glad.h>
using namespace std;

class TextureUtil
{
public:
	static unsigned int TextureFromFile(const char * path, const string & directory, bool gamma = false);
	static unsigned int loadCubemap(vector<std::string> faces);
	static unsigned int generateTexture2D(int w, int h);
	static unsigned int generateTexture3D(int w, int h, int d);
	static void bindTexture2D(unsigned int tex, int unit = 0);
};

