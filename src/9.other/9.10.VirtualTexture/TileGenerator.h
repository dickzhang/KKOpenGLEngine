#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include "Common.h"
#include "VTCommon.h"
#include "SimpleImage.h"
#include "PageIndexer.h"
#include "TileDataFile.h"

class TileGenerator
{
public:
	TileGenerator(VirtualTextureInfo _info);
	~TileGenerator();
	bool generate(const std::string& filename);
private:
	void CopyTile(SimpleImage& image, Page request);
private:
	VirtualTextureInfo m_info;
	PageIndexer* m_indexer = nullptr;
	TileDataFile* m_tileDataFile = nullptr;
	int	m_tilesize = 0;
	int	m_pagesize = 0;
	ImageContainer m_sourceImage;
	SimpleImage* m_page1Image = nullptr;
	SimpleImage* m_page2Image = nullptr;
	SimpleImage* m_2xtileImage = nullptr;
	SimpleImage* m_4xtileImage = nullptr;
	SimpleImage* m_tileImage = nullptr;
};
