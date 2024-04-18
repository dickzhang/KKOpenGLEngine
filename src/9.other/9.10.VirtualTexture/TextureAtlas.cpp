#include "TextureAtlas.h"
TextureAtlas::TextureAtlas(VirtualTextureInfo* _info, int _count, int _uploadsperframe)
	: m_info(_info)
	, m_stagingPool(_info->GetPageSize(), _info->GetPageSize(), _uploadsperframe, false)
{
	// Create atlas texture
	int pagesize = m_info->GetPageSize();
	int size = _count * pagesize;

	m_texture = bgfx::createTexture2D(
		(uint16_t)size
		, (uint16_t)size
		, false
		, 1
		, bgfx::TextureFormat::BGRA8
		, BGFX_SAMPLER_UVW_CLAMP | BGFX_TEXTURE_BLIT_DST
	);
}

TextureAtlas::~TextureAtlas()
{
	glDeleteTextures(1, &m_texture);
}

void TextureAtlas::setUploadsPerFrame(int count)
{
	m_stagingPool.grow(count);
}

void TextureAtlas::uploadPage(TPoint pt, uint8_t* data, unsigned short  blitViewId)
{
	// Get next staging texture to write to
	auto writer = m_stagingPool.getTexture();
	m_stagingPool.next();

	// Update texture with new atlas data
	auto   pagesize = uint16_t(m_info->GetPageSize());
	bgfx::updateTexture2D(
		writer
		, 0
		, 0
		, 0
		, 0
		, pagesize
		, pagesize
		, bgfx::copy(data, pagesize * pagesize * s_channelCount)
	);

	// Copy the texture part to the actual atlas texture
	auto xpos = uint16_t(pt.m_x * pagesize);
	auto ypos = uint16_t(pt.m_y * pagesize);
	bgfx::blit(blitViewId, m_texture, 0, xpos, ypos, 0, writer, 0, 0, 0, 0, pagesize, pagesize);
}

unsigned short TextureAtlas::getTexture()
{
	return m_texture;
}