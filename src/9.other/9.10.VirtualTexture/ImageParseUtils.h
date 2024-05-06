#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "VTCommon.h"
#include "Common.h"

class ImageParseUtils
{
public:
	static ImageContainer* ImageParse(const ImageContainer& _data, EPixelFormat _dstFormat, EPixelFormat _srcFormat);
};
