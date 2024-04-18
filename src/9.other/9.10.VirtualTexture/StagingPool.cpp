#include "StagingPool.h"
#include "OpenGLTexture.h"

StagingPool::StagingPool(int _width, int _height, int _count, bool _readBack)
{
	m_width = _width;
	m_height = _height;
	m_TextureInfo.width = _width;
	m_TextureInfo.height = _height;
	m_TextureInfo.filterType = ETextureFilterType::Linear;
	m_TextureInfo.addressingMode = ETextureAddressingMode::TAM_Clamp;
	m_TextureInfo.colorSpace = EColorSpace::ECS_sRGB;
	m_TextureInfo.format = EPixelFormat::PF_BGRA;
	m_TextureInfo.readback = _readBack;

	m_flags = BGFX_TEXTURE_BLIT_DST | BGFX_SAMPLER_UVW_CLAMP;
	if (_readBack)
	{
		m_flags |= BGFX_TEXTURE_READ_BACK;
	}
	grow(_count);
}

StagingPool::~StagingPool()
{
	for (int i = 0; i < (int)m_stagingTextures.size(); ++i)
	{
		glDeleteTextures(1, &m_stagingTextures[i]);
	}
}

void StagingPool::grow(int count)
{
	while ((int)m_stagingTextures.size() < count)
	{
		//auto stagingTexture = OpenGLTexture::generateTexture2D(m_TextureInfo);

		auto stagingTexture = bgfx::createTexture2D((uint16_t)m_width, (uint16_t)m_height, false, 1, bgfx::TextureFormat::BGRA8, m_flags);
		m_stagingTextures.push_back(stagingTexture);
	}
}

unsigned int StagingPool::getTexture()
{
	return m_stagingTextures[m_stagingTextureIndex];
}

void StagingPool::next()
{
	m_stagingTextureIndex = (m_stagingTextureIndex + 1) % (int)m_stagingTextures.size();
}