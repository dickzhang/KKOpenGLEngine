#include "PageTable.h"
#include "OpenGLTexture.h"

// PageTable
PageTable::PageTable(PageCache* _cache,VirtualTextureInfo* _info,PageIndexer* _indexer)
{
	m_info = _info;
	m_indexer = _indexer;

	auto size = m_info->GetPageTableSize();
	m_quadtree = new Quadtree({ 0,0,size,size },(int)glm::log2((float)size));

	TextureInfo info;
	info.width = size;
	info.height = size;
	info.hasmip = true;
	info.layernum = 1;
	info.format = EPixelFormat::PF_A8R8G8B8;
	info.addressingMode = ETextureAddressingMode::TAM_Clamp;
	info.filterType = ETextureFilterType::Point;
	m_texture = OpenGLTexture::generateTexture2D(info);

	_cache->added = [=](Page page,TPoint pt)
	{
		m_quadtreeDirty = true; m_quadtree->add(page,pt);
	};
	_cache->removed = [=](Page page,TPoint pt)
	{
		m_quadtreeDirty = true; m_quadtree->remove(page);
	};

	auto PageTableSizeLog2 = m_indexer->getMipCount();

	for(int i = 0; i<PageTableSizeLog2; ++i)
	{
		int  mipSize = m_info->GetPageTableSize()>>i;
		auto simpleImage = new SimpleImage(mipSize,mipSize,s_channelCount);

		TextureInfo info;
		info.width = mipSize;
		info.height = mipSize;
		info.hasmip = false;
		info.layernum = 1;
		info.format = EPixelFormat::PF_A8R8G8B8;
		info.addressingMode = ETextureAddressingMode::TAM_Clamp;
		info.filterType = ETextureFilterType::Point;
		auto stagingTexture = OpenGLTexture::generateTexture2D(info);
		m_images.push_back(simpleImage);
		m_stagingTextures.push_back(stagingTexture);
	}
}

PageTable::~PageTable()
{
	if(m_quadtree)
	{
		delete m_quadtree;
		m_quadtree = nullptr;
	}
	glDeleteTextures(1,&m_texture);

	for(int i = 0; i<(int)m_images.size(); ++i)
	{
		if(m_images[i])
		{
			delete m_images[i];
			m_images[i] = nullptr;
		}
	}

	for(int i = 0; i<(int)m_stagingTextures.size(); ++i)
	{
		glDeleteTextures(1,&m_stagingTextures[i]);
	}
}

void PageTable::update(unsigned short blitViewId)
{
	if(!m_quadtreeDirty)
	{
		return;
	}
	m_quadtreeDirty = false;
	auto PageTableSizeLog2 = m_indexer->getMipCount();
	for(int i = 0; i<PageTableSizeLog2; ++i)
	{
		m_quadtree->write(*m_images[i],i);
		auto stagingTexture = m_stagingTextures[i];
		auto size = uint16_t(m_info->GetPageTableSize()>>i);

		OpenGLTexture::updateTexture2D(stagingTexture,0,0,0,size,size,&m_images[i]->m_data[0]);
		OpenGLTexture::blit(m_texture,uint8_t(i),0,0,stagingTexture,0,size,size);
	}
}

unsigned short PageTable::getTexture()
{
	return m_texture;
}
