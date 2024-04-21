#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader_t.h>
#include <learnopengl/camera.h>
#include <vector>
#include "Common.h"
#include "VTCommon.h"
#include "TileDataFile.h"
#include "PageIndexer.h"
#include "PageTable.h"
#include "PageCache.h"
#include "TextureAtlas.h"
#include "PageLoader.h"
#include "OpenGLTexture.h"

class VirtualTexture
{
public:
	VirtualTexture(TileDataFile* _tileDataFile, VirtualTextureInfo _info, int _atlassize, int _uploadsperframe, int _mipBias = 4);
	~VirtualTexture();

	int  getMipBias() const;
	void setMipBias(int value);

	void setUploadsPerFrame(int count);
	int getUploadsPerFrame() const;

	void enableShowBoarders(bool enable);
	bool isShowBoardersEnabled() const;

	void enableColorMipLevels(bool enable);
	bool isColorMipLevelsEnabled() const;

	unsigned short getAtlastTexture();
	unsigned short getPageTableTexture();

	void clear();
	void update(const std::vector<int>& requests, unsigned short  blitViewId);
	void LoadShader();
	void setVTUniforms();
	void setMipUniforms();
private:
	TileDataFile* m_tileDataFile=nullptr;
	VirtualTextureInfo m_info;
	PageIndexer* m_indexer = nullptr;
	PageTable* m_pageTable = nullptr;
	TextureAtlas* m_atlas = nullptr;
	PageLoader* m_loader = nullptr;
	PageCache* m_cache = nullptr;
	int m_atlasCount;
	int m_uploadsPerFrame;
	std::vector<PageCount> m_pagesToLoad;
	int m_mipBias;
	unsigned short u_vt_settings_1;
	unsigned short u_vt_settings_2;
	unsigned short s_vt_page_table;
	unsigned short s_vt_texture_atlas;
	Shader m_vtShader;
	Shader m_mipShader;
};
