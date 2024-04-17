#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"
#include "SimpleImage.h"
#include "PageIndexer.h"
#include "TileDataFile.h"

class TileGenerator
{
public:
	TileGenerator(VirtualTextureInfo* _info);
	~TileGenerator();

	bool generate(const bx::FilePath& filename);

private:
	void CopyTile(SimpleImage& image, Page request);

private:
	VirtualTextureInfo* m_info;
	PageIndexer* m_indexer;
	TileDataFile* m_tileDataFile;

	int	m_tilesize;
	int	m_pagesize;

	bimg::ImageContainer* m_sourceImage;

	SimpleImage* m_page1Image;
	SimpleImage* m_page2Image;
	SimpleImage* m_2xtileImage;
	SimpleImage* m_4xtileImage;
	SimpleImage* m_tileImage;
};
