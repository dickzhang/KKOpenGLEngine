#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"
#include "TileDataFile.h"
#include <functional>
#include "PageIndexer.h"

class PageLoader
{
public:
	struct ReadState
	{
		Page m_page;
		std::vector<uint8_t>	m_data;
	};
public:
	PageLoader(TileDataFile* _tileDataFile, PageIndexer* _indexer, VirtualTextureInfo* _info);
	void submit(Page request);
	void loadPage(ReadState& state);
	void onPageLoadComplete(ReadState& state);
	void copyBorder(uint8_t* image);
	void copyColor(uint8_t* image, Page request);

public:
	std::function<void(Page, uint8_t*)> loadComplete;
	bool m_colorMipLevels;
	bool m_showBorders;
private:
	TileDataFile* m_tileDataFile;
	PageIndexer* m_indexer;
	VirtualTextureInfo* m_info;
};
