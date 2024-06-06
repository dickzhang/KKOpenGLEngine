#include "TextureAtlas.h"
#include "OpenGLTexture.h"
#include "OpenGLMappings.h"

TextureAtlas::TextureAtlas(VirtualTextureInfo _info,int _count,int _uploadsperframe)
	: m_info(_info)
	,m_stagingPool(_info.GetPageSize(),_info.GetPageSize(),_uploadsperframe,false)
{
	// Create atlas texture
	int pagesize = m_info.GetPageSize();
	int size = _count*pagesize;

	TextureInfo info;
	info.width = size;
	info.height = size;
	info.hasmip = false;
	info.layernum = 1;
	info.format = EPixelFormat::PF_R8G8B8A8;
	info.addressingMode = ETextureAddressingMode::TAM_Clamp;
	m_texture = OpenGLTexture::generateTexture2D(info);
}

TextureAtlas::~TextureAtlas()
{
	glDeleteTextures(1,&m_texture);
}

void TextureAtlas::setUploadsPerFrame(int count)
{
	m_stagingPool.grow(count);
}

void TextureAtlas::uploadPage(TPoint pt,uint8_t* data,unsigned short blitViewId)
{
	// Get next staging texture to write to
	auto writer = m_stagingPool.getTexture();
	m_stagingPool.next();

	// Update texture with new atlas data
	auto pagesize = uint16_t(m_info.GetPageSize());

	OpenGLTexture::updateTexture2D(writer,0,0,0,pagesize,pagesize,data);

	// Copy the texture part to the actual atlas texture
	auto xpos = uint16_t(pt.m_x*pagesize);
	auto ypos = uint16_t(pt.m_y*pagesize);
	OpenGLTexture::blit(m_texture,0,xpos,ypos,writer,0,pagesize,pagesize);

}

unsigned short TextureAtlas::getTexture()
{
	return m_texture;
}