#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"
#include "PageIndexer.h"
#include "SimpleImage.h"
#include "Quadtree.h"
#include "PageCache.h"

class PageTable
{
public:
	PageTable(PageCache* _cache, VirtualTextureInfo* _info, PageIndexer* _indexer);
	~PageTable();
	void update(unsigned short blitViewId);
	unsigned short getTexture();
private:
	VirtualTextureInfo* m_info = nullptr;
	unsigned short m_texture = 0;
	PageIndexer* m_indexer = nullptr;
	Quadtree* m_quadtree = nullptr;
	bool m_quadtreeDirty = true;
	std::vector<SimpleImage*> m_images;
	std::vector<unsigned short>	m_stagingTextures;
};
