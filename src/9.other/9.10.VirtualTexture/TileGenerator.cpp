#include "TileGenerator.h"
#include "VirtualTexture.h"
#include <stdio.h>
#include <stb_image.h>
#include <string.h>
#include <filesystem>
#include <iostream>
#include <fstream>

// TileGenerator
TileGenerator::TileGenerator(VirtualTextureInfo _info)
{
	m_info = _info;
	m_tilesize = m_info.m_tileSize;
	m_pagesize = m_info.GetPageSize();
}

TileGenerator::~TileGenerator()
{
	m_sourceImage.release();

	if(m_indexer)
	{
		delete m_indexer;
		m_indexer = nullptr;
	}
	if(m_page1Image)
	{
		delete m_page1Image;
		m_page1Image = nullptr;
	}
	if(m_page2Image)
	{
		delete m_page2Image;
		m_page2Image = nullptr;
	}
	if(m_2xtileImage)
	{
		delete m_2xtileImage;
		m_2xtileImage = nullptr;
	}
	if(m_4xtileImage)
	{
		delete m_4xtileImage;
		m_4xtileImage = nullptr;
	}
	if(m_tileImage)
	{
		delete m_tileImage;
		m_tileImage = nullptr;
	}
}

bool TileGenerator::generate(const std::string& _filePath)
{
	const std::string baseName = _filePath;

	// Generate cache filename
	char tmp[256];
	std::snprintf(tmp,sizeof(tmp),"%.*s.vt",(unsigned int)baseName.length(),baseName.c_str());

	if(std::filesystem::exists(tmp))
	{
		std::cout<<"Tile data file already exists. Skipping generation"<<std::endl;
		return true;
	}

	// Read image
	{
		std::cout<<"Reading image:"<<_filePath.c_str()<<std::endl;
		std::ifstream file(_filePath,std::ios::binary);
		if(!file.is_open())
		{
			std::cerr<<"Unable to open file"<<std::endl;
			return false;
		}
		file.close();

		int width,height,nrChannels;
		unsigned char* data = stbi_load(_filePath.c_str(),&width,&height,&nrChannels,0);
		if(data)
		{
			m_sourceImage.m_data = data;
			m_sourceImage.m_width = width;
			m_sourceImage.m_height = height;
			m_sourceImage.m_format = EPixelFormat::PF_BGRA;
			//m_sourceImage.m_size = width*height*nrChannels;
			m_sourceImage.m_size = width*height*4;
			m_sourceImage.m_offset = 0;
			m_sourceImage.m_hasAlpha = nrChannels==4 ? true : false;
			m_sourceImage.m_hasAlpha = true;
		}
		else
		{
			std::cout<<"Image parse failed"<<_filePath.c_str()<<std::endl;
			return false;
		}
	}

	// Setup
	m_info.m_virtualTextureSize = m_sourceImage.m_width;
	m_indexer = new PageIndexer(m_info);

	// Open tile data file
	m_tileDataFile = new  TileDataFile(tmp,m_info,true);
	m_page1Image = new SimpleImage(m_pagesize,m_pagesize,s_channelCount,0xff);
	m_page2Image = new SimpleImage(m_pagesize,m_pagesize,s_channelCount,0xff);
	m_tileImage = new SimpleImage(m_tilesize,m_tilesize,s_channelCount,0xff);
	m_2xtileImage = new SimpleImage(m_tilesize*2,m_tilesize*2,s_channelCount,0xff);
	m_4xtileImage = new SimpleImage(m_tilesize*4,m_tilesize*4,s_channelCount,0xff);

	// Generate tiles
	std::cout<<"Generating tiles"<<std::endl;
	auto mipcount = m_indexer->getMipCount();
	for(int i = 0; i<mipcount; ++i)
	{
		int count = (m_info.m_virtualTextureSize/m_tilesize)>>i;
		std::cout<<"Generating Mip: Count"<<i<<count<<count<<std::endl;
		for(int y = 0; y<count; ++y)
		{
			for(int x = 0; x<count; ++x)
			{
				Page page = { x,y,i };
				int index = m_indexer->getIndexFromPage(page);
				CopyTile(*m_page1Image,page);
				m_tileDataFile->writePage(index,&m_page1Image->m_data[0]);
			}
		}
	}

	std::cout<<"Finising"<<std::endl;
	// Write header
	m_tileDataFile->writeInfo();
	// Close tile file
	if(m_tileDataFile)
	{
		delete m_tileDataFile;
		m_tileDataFile = nullptr;
	}
	m_tileDataFile = nullptr;
	std::cout<<"Done!"<<std::endl;
	return true;
}

void TileGenerator::CopyTile(SimpleImage& image,Page request)
{
	if(request.m_mip==0)
	{
		int x = request.m_x*m_tilesize-m_info.m_borderSize;
		int y = request.m_y*m_tilesize-m_info.m_borderSize;
		// Copy sub-image with border
		auto srcPitch = m_sourceImage.m_width*s_channelCount;
		auto src = (uint8_t*)m_sourceImage.m_data;
		auto dstPitch = image.m_width*image.m_channelCount;
		auto dst = &image.m_data[0];
		for(int iy = 0; iy<m_pagesize; ++iy)
		{
			int ry = glm::clamp(y+iy,0,(int)m_sourceImage.m_height-1);
			for(int ix = 0; ix<m_pagesize; ++ix)
			{
				int rx = glm::clamp(x+ix,0,(int)m_sourceImage.m_width-1);
				std::memcpy(&dst[iy*dstPitch+ix*image.m_channelCount],&src[ry*srcPitch+rx*s_channelCount],image.m_channelCount);
			}
		}
	}
	else
	{
		int xpos = request.m_x<<1;
		int ypos = request.m_y<<1;
		int mip = request.m_mip-1;
		int size = m_info.GetPageTableSize()>>mip;
		m_4xtileImage->clear((uint8_t)request.m_mip);

		for(int y = 0; y<4; ++y)
		{
			for(int x = 0; x<4; ++x)
			{
				Page page = { xpos+x-1,ypos+y-1,mip };

				// Wrap so we get the border sections of other pages
				page.m_x = (int)glm::mod((float)page.m_x,(float)size);
				page.m_y = (int)glm::mod((float)page.m_y,(float)size);
				m_tileDataFile->readPage(m_indexer->getIndexFromPage(page),&m_page2Image->m_data[0]);
				Rect src_rect = { m_info.m_borderSize,m_info.m_borderSize,m_tilesize,m_tilesize };
				TPoint dst_offset = { x*m_tilesize,y*m_tilesize };
				m_4xtileImage->copy(dst_offset,*m_page2Image,src_rect);
			}
		}

		SimpleImage::mipmap(&m_4xtileImage->m_data[0],m_4xtileImage->m_width,s_channelCount,&m_2xtileImage->m_data[0]);
		Rect srect = { m_tilesize/2-m_info.m_borderSize,m_tilesize/2-m_info.m_borderSize,m_pagesize,m_pagesize };
		image.copy({ 0,0 },*m_2xtileImage,srect);
	}
}