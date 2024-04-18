#include "VirtualTexture.h"

// VirtualTexture
VirtualTexture::VirtualTexture(TileDataFile* _tileDataFile, VirtualTextureInfo* _info, int _atlassize, int _uploadsperframe, int _mipBias)
{
	m_tileDataFile = _tileDataFile;
	m_info = _info;
	m_uploadsPerFrame = _uploadsperframe;
	m_mipBias = _mipBias;

	m_atlasCount = _atlassize / m_info->GetPageSize();
	// Setup indexer
	m_indexer = new PageIndexer(m_info);
	m_pagesToLoad.reserve(m_indexer->getCount());

	// Setup classes
	m_atlas = new TextureAtlas(m_info, m_atlasCount, m_uploadsPerFrame);
	m_loader = new PageLoader(m_tileDataFile, m_indexer, m_info);
	m_cache = new PageCache(m_atlas, m_loader, m_atlasCount);
	m_pageTable = new PageTable(m_cache, m_info, m_indexer);
}

VirtualTexture::~VirtualTexture()
{
	if (m_indexer)
	{
		delete m_indexer;
		m_indexer = nullptr;
	}
	if (m_atlas)
	{
		delete m_atlas;
		m_atlas = nullptr;
	}
	if (m_loader)
	{
		delete m_loader;
		m_loader = nullptr;
	}
	if (m_cache)
	{
		delete m_cache;
		m_cache = nullptr;
	}
	if (m_pageTable)
	{
		delete m_pageTable;
		m_pageTable = nullptr;
	}
}

void VirtualTexture::LoadShader()
{
	m_vtShader.loadShader("vs_vt_generic.vs","fs_vt_unlit.fs");
	m_mipShader.loadShader("vs_vt_generic.vs","fs_vt_mip.fs");
}

int VirtualTexture::getMipBias() const
{
	return m_mipBias;
}

void VirtualTexture::setMipBias(int value)
{
	m_mipBias = glm::max(0, value);
}

void VirtualTexture::setVTUniforms()
{
	//todo 这里要和shader里面的uniform名称对应上
	int pagesize = m_info->GetPageSize();
	m_vtShader.setFloat("VirtualTextureSize", (float)m_info->m_virtualTextureSize);
	m_vtShader.setFloat("ooAtlasScale", 1.0f / (float)m_atlasCount);
	m_vtShader.setFloat("BorderScale", (float)((pagesize - 2.0f * m_info->m_borderSize) / pagesize));
	m_vtShader.setFloat("BorderOffset", (float)m_info->m_borderSize / (float)pagesize);
	m_vtShader.setFloat("MipBias", (float)m_mipBias);
	m_vtShader.setFloat("PageTableSize", (float)m_info->GetPageTableSize());
	m_vtShader.setSampler2D("s_vt_page_table", m_pageTable->getTexture(),0);
	m_vtShader.setSampler2D("s_vt_texture_atlas", m_atlas->getTexture(),1);
}

void VirtualTexture::setMipUniforms()
{
	//todo 这里要和shader里面的uniform名称对应上
	int pagesize = m_info->GetPageSize();
	m_mipShader.setFloat("VirtualTextureSize", (float)m_info->m_virtualTextureSize);
	m_mipShader.setFloat("ooAtlasScale", 1.0f / (float)m_atlasCount);
	m_mipShader.setFloat("BorderScale", (float)((pagesize - 2.0f * m_info->m_borderSize) / pagesize));
	m_mipShader.setFloat("BorderOffset", (float)m_info->m_borderSize / (float)pagesize);
	m_mipShader.setFloat("MipBias", (float)m_mipBias);
	m_mipShader.setFloat("PageTableSize", (float)m_info->GetPageTableSize());
}

void VirtualTexture::setUploadsPerFrame(int count)
{
	m_uploadsPerFrame = count;
	m_atlas->setUploadsPerFrame(count);
}

int VirtualTexture::getUploadsPerFrame() const
{
	return m_uploadsPerFrame;
}


void VirtualTexture::enableShowBoarders(bool enable)
{
	if (m_loader->m_showBorders == enable)
	{
		return;
	}

	m_loader->m_showBorders = enable;
	clear();
}

bool VirtualTexture::isShowBoardersEnabled() const
{
	return m_loader->m_showBorders;
}

void VirtualTexture::enableColorMipLevels(bool enable)
{
	if (m_loader->m_colorMipLevels == enable)
	{
		return;
	}

	m_loader->m_colorMipLevels = enable;
	clear();
}

bool VirtualTexture::isColorMipLevelsEnabled() const
{
	return m_loader->m_colorMipLevels;
}

unsigned short VirtualTexture::getAtlastTexture()
{
	return m_atlas->getTexture();
}

unsigned short VirtualTexture::getPageTableTexture()
{
	return m_pageTable->getTexture();
}

void VirtualTexture::clear()
{
	m_cache->clear();
}

void VirtualTexture::update(const std::vector<int>& requests, unsigned short  blitViewId)
{
	m_pagesToLoad.clear();
	// Find out what is already in memory
	// If it is, update it's position in the LRU collection
	// Otherwise add it to the list of pages to load
	int touched = 0;
	for (int i = 0; i < (int)requests.size(); ++i)
	{
		if (requests[i] > 0)
		{
			PageCount pc(m_indexer->getPageFromIndex(i), requests[i]);
			if (!m_cache->touch(pc.m_page))
			{
				m_pagesToLoad.push_back(pc);
			}
			else
			{
				++touched;
			}
		}
	}

	// Check to make sure we don't thrash
	if (touched < m_atlasCount * m_atlasCount)
	{
		// sort by low res to high res and number of requests
		std::sort(m_pagesToLoad.begin(), m_pagesToLoad.end(),[](const void* _a, const void* _b) -> int32_t
		{
			const PageCount& lhs = *(const PageCount*)(_a);
			const PageCount& rhs = *(const PageCount*)(_b);
			return lhs.compareTo(rhs);
		});

		// if more pages than will fit in memory or more than update per frame drop high res pages with lowest use count
		int loadcount = glm::min(glm::min((int)m_pagesToLoad.size(), m_uploadsPerFrame), m_atlasCount * m_atlasCount);
		for (int i = 0; i < loadcount; ++i)
			m_cache->request(m_pagesToLoad[i].m_page, blitViewId);
	}
	else
	{
		// The problem here is that all pages in cache are requested and the new or high res ones don't get uploaded
		// We can adjust the mip bias to make it all fit. This solves the problem of page cache thrashing
		--m_mipBias;
	}
	// Update the page table
	m_pageTable->update(blitViewId);
}
