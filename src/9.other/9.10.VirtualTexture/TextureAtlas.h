#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"
#include "StagingPool.h"

class TextureAtlas
{
public:
	TextureAtlas(VirtualTextureInfo* _info, int count, int uploadsperframe);
	~TextureAtlas();

	void setUploadsPerFrame(int count);
	void uploadPage(TPoint pt, uint8_t* data, unsigned short  blitViewId);

	unsigned short getTexture();

private:
	VirtualTextureInfo* m_info;
	unsigned short m_texture;
	StagingPool m_stagingPool;
};
