#include "PageLoader.h"

// PageLoader
PageLoader::PageLoader(TileDataFile* _tileDataFile, PageIndexer* _indexer, VirtualTextureInfo* _info)
	: m_colorMipLevels(false)
	, m_showBorders(false)
	, m_tileDataFile(_tileDataFile)
	, m_indexer(_indexer)
	, m_info(_info)
{
}

void PageLoader::submit(Page request)
{
	ReadState state;
	state.m_page = request;
	loadPage(state);
	onPageLoadComplete(state);
}

void PageLoader::loadPage(ReadState& state)
{
	int size = m_info->GetPageSize() * m_info->GetPageSize() * s_channelCount;
	state.m_data.resize(size);

	if (m_colorMipLevels)
	{
		copyColor(&state.m_data[0], state.m_page);
	}
	else if (m_tileDataFile != nullptr)
	{
		m_tileDataFile->readPage(m_indexer->getIndexFromPage(state.m_page), &state.m_data[0]);
	}

	if (m_showBorders)
	{
		copyBorder(&state.m_data[0]);
	}
}

void PageLoader::onPageLoadComplete(ReadState& state)
{
	loadComplete(state.m_page, &state.m_data[0]);
}

void PageLoader::copyBorder(uint8_t* image)
{
	int pagesize = m_info->GetPageSize();
	int bordersize = m_info->m_borderSize;

	for (int i = 0; i < pagesize; ++i)
	{
		int xindex = bordersize * pagesize + i;
		image[xindex * s_channelCount + 0] = 0;
		image[xindex * s_channelCount + 1] = 255;
		image[xindex * s_channelCount + 2] = 0;
		image[xindex * s_channelCount + 3] = 255;

		int yindex = i * pagesize + bordersize;
		image[yindex * s_channelCount + 0] = 0;
		image[yindex * s_channelCount + 1] = 255;
		image[yindex * s_channelCount + 2] = 0;
		image[yindex * s_channelCount + 3] = 255;
	}
}

void PageLoader::copyColor(uint8_t* image, Page request)
{
	static const Color colors[] =
	{
		{   0,   0, 255, 255 },
		{   0, 255, 255, 255 },
		{ 255,   0,   0, 255 },
		{ 255,   0, 255, 255 },
		{ 255, 255,   0, 255 },
		{  64,  64, 192, 255 },
		{  64, 192,  64, 255 },
		{  64, 192, 192, 255 },
		{ 192,  64,  64, 255 },
		{ 192,  64, 192, 255 },
		{ 192, 192,  64, 255 },
		{   0, 255,   0, 255 },
	};

	int pagesize = m_info->GetPageSize();

	for (int y = 0; y < pagesize; ++y)
	{
		for (int x = 0; x < pagesize; ++x)
		{
			image[(y * pagesize + x) * s_channelCount + 0] = colors[request.m_mip].m_b;
			image[(y * pagesize + x) * s_channelCount + 1] = colors[request.m_mip].m_g;
			image[(y * pagesize + x) * s_channelCount + 2] = colors[request.m_mip].m_r;
			image[(y * pagesize + x) * s_channelCount + 3] = colors[request.m_mip].m_a;
		}
	}
}