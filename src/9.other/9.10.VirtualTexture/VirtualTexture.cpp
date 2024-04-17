#include "VirtualTexture.h"

// VirtualTexture
VirtualTexture::VirtualTexture(TileDataFile* _tileDataFile, VirtualTextureInfo* _info, int _atlassize, int _uploadsperframe, int _mipBias)
	: m_tileDataFile(_tileDataFile)
	, m_info(_info)
	, m_uploadsPerFrame(_uploadsperframe)
	, m_mipBias(_mipBias)

{
	m_atlasCount = _atlassize / m_info->GetPageSize();

	// Setup indexer
	m_indexer = BX_NEW(VirtualTexture::getAllocator(), PageIndexer)(m_info);
	m_pagesToLoad.reserve(m_indexer->getCount());

	// Setup classes
	m_atlas = BX_NEW(VirtualTexture::getAllocator(), TextureAtlas)(m_info, m_atlasCount, m_uploadsPerFrame);
	m_loader = BX_NEW(VirtualTexture::getAllocator(), PageLoader)(m_tileDataFile, m_indexer, m_info);
	m_cache = BX_NEW(VirtualTexture::getAllocator(), PageCache)(m_atlas, m_loader, m_atlasCount);
	m_pageTable = BX_NEW(VirtualTexture::getAllocator(), PageTable)(m_cache, m_info, m_indexer);

	// Create uniforms
	u_vt_settings_1 = bgfx::createUniform("u_vt_settings_1", bgfx::UniformType::Vec4);
	u_vt_settings_2 = bgfx::createUniform("u_vt_settings_2", bgfx::UniformType::Vec4);
	s_vt_page_table = bgfx::createUniform("s_vt_page_table", bgfx::UniformType::Sampler);
	s_vt_texture_atlas = bgfx::createUniform("s_vt_texture_atlas", bgfx::UniformType::Sampler);
}

VirtualTexture::~VirtualTexture()
{
	// Destroy
	bx::deleteObject(VirtualTexture::getAllocator(), m_indexer);
	bx::deleteObject(VirtualTexture::getAllocator(), m_atlas);
	bx::deleteObject(VirtualTexture::getAllocator(), m_loader);
	bx::deleteObject(VirtualTexture::getAllocator(), m_cache);
	bx::deleteObject(VirtualTexture::getAllocator(), m_pageTable);
	// Destroy all uniforms and textures
	bgfx::destroy(u_vt_settings_1);
	bgfx::destroy(u_vt_settings_2);
	bgfx::destroy(s_vt_page_table);
	bgfx::destroy(s_vt_texture_atlas);
}

int VirtualTexture::getMipBias() const
{
	return m_mipBias;
}

void VirtualTexture::setMipBias(int value)
{
	m_mipBias = bx::max(0, value);
}

void VirtualTexture::setUniforms()
{
	struct
	{
		struct
		{
			float VirtualTextureSize;
			float ooAtlasScale;
			float BorderScale;
			float BorderOffset;
		} m_settings_1;

		struct
		{
			float MipBias;
			float PageTableSize;
			float unused1;
			float unused2;
		} m_settings_2;

	} uniforms;

	int pagesize = m_info->GetPageSize();
	uniforms.m_settings_1.VirtualTextureSize = (float)m_info->m_virtualTextureSize;
	uniforms.m_settings_1.ooAtlasScale = 1.0f / (float)m_atlasCount;
	uniforms.m_settings_1.BorderScale = (float)((pagesize - 2.0f * m_info->m_borderSize) / pagesize);
	uniforms.m_settings_1.BorderOffset = (float)m_info->m_borderSize / (float)pagesize;
	uniforms.m_settings_2.MipBias = (float)m_mipBias;
	uniforms.m_settings_2.PageTableSize = (float)m_info->GetPageTableSize();
	uniforms.m_settings_2.unused1 = uniforms.m_settings_2.unused2 = 0.0f;

	bgfx::setUniform(u_vt_settings_1, &uniforms.m_settings_1);
	bgfx::setUniform(u_vt_settings_2, &uniforms.m_settings_2);

	bgfx::setTexture(0, s_vt_page_table, m_pageTable->getTexture());
	bgfx::setTexture(1, s_vt_texture_atlas, m_atlas->getTexture());
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

bgfx::TextureHandle VirtualTexture::getAtlastTexture()
{
	return m_atlas->getTexture();
}

bgfx::TextureHandle VirtualTexture::getPageTableTexture()
{
	return m_pageTable->getTexture();
}

void VirtualTexture::clear()
{
	m_cache->clear();
}

void VirtualTexture::update(const tinystl::vector<int>& requests, bgfx::ViewId blitViewId)
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
		bx::quickSort(m_pagesToLoad.begin(), uint32_t(m_pagesToLoad.size()), sizeof(vt::PageCount)
			, [](const void* _a, const void* _b) -> int32_t
		{
			const vt::PageCount& lhs = *(const vt::PageCount*)(_a);
			const vt::PageCount& rhs = *(const vt::PageCount*)(_b);
			return lhs.compareTo(rhs);
		});

		// if more pages than will fit in memory or more than update per frame drop high res pages with lowest use count
		int loadcount = bx::min(bx::min((int)m_pagesToLoad.size(), m_uploadsPerFrame), m_atlasCount * m_atlasCount);
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

bx::AllocatorI* VirtualTexture::s_allocator = nullptr;

void VirtualTexture::setAllocator(bx::AllocatorI* allocator)
{
	s_allocator = allocator;
}

bx::AllocatorI* VirtualTexture::getAllocator()
{
	return s_allocator;
}