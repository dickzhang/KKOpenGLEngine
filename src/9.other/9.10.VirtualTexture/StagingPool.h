#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"

class StagingPool
{
public:
	StagingPool(int _width, int _height, int _count, bool _readBack);
	~StagingPool();

	void grow(int count);
	unsigned int getTexture();
	void next();

private:
	std::vector<unsigned int>  m_stagingTextures;
	int			m_stagingTextureIndex=0;
	int			m_width=0;
	int			m_height=0;
	uint64_t	m_flags;
	TextureInfo m_TextureInfo;
};
