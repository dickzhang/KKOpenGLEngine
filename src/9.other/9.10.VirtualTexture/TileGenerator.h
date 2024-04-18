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
#include <iostream>
class TileGenerator
{

public:
	struct ImageContainer
	{
		void* m_data = nullptr;
		EPixelFormat m_format;
		uint32_t m_size;
		uint32_t m_offset;
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_depth;
		uint16_t m_numLayers;
		uint8_t  m_numMips;
		bool     m_hasAlpha;
		bool     m_cubeMap;
		bool     m_ktx;
		bool     m_ktxLE;
		bool     m_pvr3;
		bool     m_srgb;

		void release()
		{
			if (m_data)
			{
				delete m_data;
				m_data = nullptr;
			}
		}
	};

public:
	TileGenerator(VirtualTextureInfo* _info);
	~TileGenerator();
	bool generate(const std::string& filename);

private:
	void CopyTile(SimpleImage& image, Page request);

private:
	VirtualTextureInfo* m_info = nullptr;
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
