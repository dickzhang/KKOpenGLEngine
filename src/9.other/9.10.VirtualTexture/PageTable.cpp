#include "PageTable.h"

// PageTable
PageTable::PageTable(PageCache* _cache, VirtualTextureInfo* _info, PageIndexer* _indexer)
{
	m_info = _info;
	m_indexer = _indexer;

	auto size = m_info->GetPageTableSize();
	m_quadtree = BX_NEW(VirtualTexture::getAllocator(), Quadtree)({ 0, 0, size, size }, (int)bx::log2((float)size));
	m_texture = bgfx::createTexture2D((uint16_t)size, (uint16_t)size, true, 1, bgfx::TextureFormat::BGRA8, BGFX_SAMPLER_UVW_CLAMP | BGFX_SAMPLER_POINT | BGFX_TEXTURE_BLIT_DST);

	_cache->added = [=](Page page, TPoint pt)
	{
		m_quadtreeDirty = true; m_quadtree->add(page, pt);
	};
	_cache->removed = [=](Page page, TPoint pt)
	{
		m_quadtreeDirty = true; m_quadtree->remove(page);
		BX_UNUSED(pt);
	};

	auto PageTableSizeLog2 = m_indexer->getMipCount();

	for (int i = 0; i < PageTableSizeLog2; ++i)
	{
		int  mipSize = m_info->GetPageTableSize() >> i;
		auto simpleImage = BX_NEW(VirtualTexture::getAllocator(), SimpleImage)(mipSize, mipSize, s_channelCount);
		auto stagingTexture = bgfx::createTexture2D((uint16_t)mipSize, (uint16_t)mipSize, false, 1, bgfx::TextureFormat::BGRA8, BGFX_SAMPLER_UVW_CLAMP | BGFX_SAMPLER_POINT);
		m_images.push_back(simpleImage);
		m_stagingTextures.push_back(stagingTexture);
	}
}

PageTable::~PageTable()
{
	bx::deleteObject(VirtualTexture::getAllocator(), m_quadtree);
	bgfx::destroy(m_texture);

	for (int i = 0; i < (int)m_images.size(); ++i)
	{
		bx::deleteObject(VirtualTexture::getAllocator(), m_images[i]);
	}

	for (int i = 0; i < (int)m_stagingTextures.size(); ++i)
	{
		glDeleteTextures(1, &m_stagingTextures[i]);
	}
}

void PageTable::update(unsigned short blitViewId)
{
	if (!m_quadtreeDirty)
	{
		return;
	}
	m_quadtreeDirty = false;
	auto PageTableSizeLog2 = m_indexer->getMipCount();
	for (int i = 0; i < PageTableSizeLog2; ++i)
	{
		m_quadtree->write(*m_images[i], i);
		auto stagingTexture = m_stagingTextures[i];
		auto size = uint16_t(m_info->GetPageTableSize() >> i);
		bgfx::updateTexture2D(stagingTexture, 0, 0, 0, 0, size, size, bgfx::copy(&m_images[i]->m_data[0], size * size * s_channelCount));
		bgfx::blit(blitViewId, m_texture, uint8_t(i), 0, 0, 0, stagingTexture, 0, 0, 0, 0, size, size);
	}
}

unsigned short PageTable::getTexture()
{
	return m_texture;
}
