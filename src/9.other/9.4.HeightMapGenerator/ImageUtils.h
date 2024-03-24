#pragma once
#include"FreeImage/include/FreeImage.h"

class ImageUtils {
public:
	static BYTE* Load_Image(const char* filename, int& width, int& height);
	static FIBITMAP* Load_Image(const char* filename);
	static void Unload_Image(FIBITMAP* image);
};