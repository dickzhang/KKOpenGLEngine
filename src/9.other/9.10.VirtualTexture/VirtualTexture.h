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
#include "PageIndexer.h"
#include "PageTable.h"
#include "TextureAtlas.h"
#include "PageLoader.h"

class VirtualTexture
{
public:
	VirtualTexture(TileDataFile* _tileDataFile, VirtualTextureInfo* _info, int _atlassize, int _uploadsperframe, int _mipBias = 4);
	~VirtualTexture();

	int  getMipBias() const;
	void setMipBias(int value);

	void setUploadsPerFrame(int count);
	int getUploadsPerFrame() const;

	void enableShowBoarders(bool enable);
	bool isShowBoardersEnabled() const;

	void enableColorMipLevels(bool enable);
	bool isColorMipLevelsEnabled() const;

	bgfx::TextureHandle getAtlastTexture();
	bgfx::TextureHandle getPageTableTexture();

	void clear();
	void update(const std::vector<int>& requests, bgfx::ViewId blitViewId);

	void setUniforms();

	static void setAllocator(bx::AllocatorI* allocator);
	static bx::AllocatorI* getAllocator();

private:
	TileDataFile* m_tileDataFile;
	VirtualTextureInfo* m_info;
	PageIndexer* m_indexer;
	PageTable* m_pageTable;
	TextureAtlas* m_atlas;
	PageLoader* m_loader;
	PageCache* m_cache;

	int m_atlasCount;
	int m_uploadsPerFrame;

	std::vector<PageCount> m_pagesToLoad;

	int m_mipBias;

	bgfx::UniformHandle u_vt_settings_1;
	bgfx::UniformHandle u_vt_settings_2;
	bgfx::UniformHandle s_vt_page_table;
	bgfx::UniformHandle s_vt_texture_atlas;

	static bx::AllocatorI* s_allocator;
};