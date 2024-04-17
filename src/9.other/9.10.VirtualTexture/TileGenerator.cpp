#include "TileGenerator.h"

// TileGenerator
TileGenerator::TileGenerator(VirtualTextureInfo* _info)
	: m_info(_info)
	, m_indexer(nullptr)
	, m_tileDataFile(nullptr)
	, m_sourceImage(nullptr)
	, m_page1Image(nullptr)
	, m_page2Image(nullptr)
	, m_2xtileImage(nullptr)
	, m_4xtileImage(nullptr)
	, m_tileImage(nullptr)
{
	m_tilesize = m_info->m_tileSize;
	m_pagesize = m_info->GetPageSize();
}

TileGenerator::~TileGenerator()
{
	if (m_sourceImage != nullptr)
	{
		bimg::imageFree(m_sourceImage);
	}

	bx::deleteObject(VirtualTexture::getAllocator(), m_indexer);

	bx::deleteObject(VirtualTexture::getAllocator(), m_page1Image);
	bx::deleteObject(VirtualTexture::getAllocator(), m_page2Image);
	bx::deleteObject(VirtualTexture::getAllocator(), m_2xtileImage);
	bx::deleteObject(VirtualTexture::getAllocator(), m_4xtileImage);
	bx::deleteObject(VirtualTexture::getAllocator(), m_tileImage);
}

bool TileGenerator::generate(const bx::FilePath& _filePath)
{
	const bx::StringView baseName = _filePath.getBaseName();

	// Generate cache filename
	char tmp[256];
	bx::snprintf(tmp, sizeof(tmp), "%.*s.vt", baseName.getLength(), baseName.getPtr());

	bx::FilePath cacheFilePath("temp");
	cacheFilePath.join(tmp);

	// Check if tile file already exist
	{
		bx::Error err;
		bx::FileReader fileReader;

		if (bx::open(&fileReader, cacheFilePath, &err))
		{
			bx::close(&fileReader);

			bx::debugPrintf("Tile data file '%s' already exists. Skipping generation.\n", cacheFilePath.getCPtr());
			return true;
		}
	}

	// Read image
	{
		bx::debugPrintf("Reading image '%s'.\n", _filePath.getCPtr());

		bx::Error err;
		bx::FileReader fileReader;

		if (!bx::open(&fileReader, _filePath, &err))
		{
			bx::debugPrintf("Image open failed'%s'.\n", _filePath.getCPtr());
			return false;
		}

		int64_t size = bx::getSize(&fileReader);

		if (0 == size)
		{
			bx::debugPrintf("Image '%s' size is 0.\n", _filePath.getCPtr());
			return false;
		}

		uint8_t* rawImage = (uint8_t*)bx::alloc(VirtualTexture::getAllocator(), size_t(size));

		bx::read(&fileReader, rawImage, int32_t(size), &err);
		bx::close(&fileReader);

		if (!err.isOk())
		{
			bx::debugPrintf("Image read failed'%s'.\n", _filePath.getCPtr());
			bx::free(VirtualTexture::getAllocator(), rawImage);
			return false;
		}

		m_sourceImage = bimg::imageParse(VirtualTexture::getAllocator(), rawImage, uint32_t(size), bimg::TextureFormat::BGRA8, &err);
		bx::free(VirtualTexture::getAllocator(), rawImage);

		if (!err.isOk())
		{
			bx::debugPrintf("Image parse failed'%s'.\n", _filePath.getCPtr());
			return false;
		}
	}

	// Setup
	m_info->m_virtualTextureSize = int(m_sourceImage->m_width);
	m_indexer = BX_NEW(VirtualTexture::getAllocator(), PageIndexer)(m_info);

	// Open tile data file
	m_tileDataFile = BX_NEW(VirtualTexture::getAllocator(), TileDataFile)(cacheFilePath, m_info, true);
	m_page1Image = BX_NEW(VirtualTexture::getAllocator(), SimpleImage)(m_pagesize, m_pagesize, s_channelCount, 0xff);
	m_page2Image = BX_NEW(VirtualTexture::getAllocator(), SimpleImage)(m_pagesize, m_pagesize, s_channelCount, 0xff);
	m_tileImage = BX_NEW(VirtualTexture::getAllocator(), SimpleImage)(m_tilesize, m_tilesize, s_channelCount, 0xff);
	m_2xtileImage = BX_NEW(VirtualTexture::getAllocator(), SimpleImage)(m_tilesize * 2, m_tilesize * 2, s_channelCount, 0xff);
	m_4xtileImage = BX_NEW(VirtualTexture::getAllocator(), SimpleImage)(m_tilesize * 4, m_tilesize * 4, s_channelCount, 0xff);

	// Generate tiles
	bx::debugPrintf("Generating tiles\n");
	auto mipcount = m_indexer->getMipCount();
	for (int i = 0; i < mipcount; ++i)
	{
		int count = (m_info->m_virtualTextureSize / m_tilesize) >> i;
		bx::debugPrintf("Generating Mip:%d Count:%dx%d\n", i, count, count);
		for (int y = 0; y < count; ++y)
		{
			for (int x = 0; x < count; ++x)
			{
				Page page = { x, y, i };
				int index = m_indexer->getIndexFromPage(page);
				CopyTile(*m_page1Image, page);
				m_tileDataFile->writePage(index, &m_page1Image->m_data[0]);
			}
		}
	}

	bx::debugPrintf("Finising\n");
	// Write header
	m_tileDataFile->writeInfo();
	// Close tile file
	bx::deleteObject(VirtualTexture::getAllocator(), m_tileDataFile);
	m_tileDataFile = nullptr;
	bx::debugPrintf("Done!\n");
	return true;
}

void TileGenerator::CopyTile(SimpleImage& image, Page request)
{
	if (request.m_mip == 0)
	{
		int x = request.m_x * m_tilesize - m_info->m_borderSize;
		int y = request.m_y * m_tilesize - m_info->m_borderSize;
		// Copy sub-image with border
		auto srcPitch = m_sourceImage->m_width * s_channelCount;
		auto src = (uint8_t*)m_sourceImage->m_data;
		auto dstPitch = image.m_width * image.m_channelCount;
		auto dst = &image.m_data[0];
		for (int iy = 0; iy < m_pagesize; ++iy)
		{
			int ry = bx::clamp(y + iy, 0, (int)m_sourceImage->m_height - 1);
			for (int ix = 0; ix < m_pagesize; ++ix)
			{
				int rx = bx::clamp(x + ix, 0, (int)m_sourceImage->m_width - 1);
				bx::memCopy(&dst[iy * dstPitch + ix * image.m_channelCount], &src[ry * srcPitch + rx * s_channelCount], image.m_channelCount);
			}
		}
	}
	else
	{
		int xpos = request.m_x << 1;
		int ypos = request.m_y << 1;
		int mip = request.m_mip - 1;

		int size = m_info->GetPageTableSize() >> mip;

		m_4xtileImage->clear((uint8_t)request.m_mip);

		for (int y = 0; y < 4; ++y)
		{
			for (int x = 0; x < 4; ++x)
			{
				Page page = { xpos + x - 1, ypos + y - 1, mip };

				// Wrap so we get the border sections of other pages
				page.m_x = (int)bx::mod((float)page.m_x, (float)size);
				page.m_y = (int)bx::mod((float)page.m_y, (float)size);

				m_tileDataFile->readPage(m_indexer->getIndexFromPage(page), &m_page2Image->m_data[0]);

				Rect src_rect = { m_info->m_borderSize, m_info->m_borderSize, m_tilesize, m_tilesize };
				Point dst_offset = { x * m_tilesize, y * m_tilesize };

				m_4xtileImage->copy(dst_offset, *m_page2Image, src_rect);
			}
		}

		SimpleImage::mipmap(&m_4xtileImage->m_data[0], m_4xtileImage->m_width, s_channelCount, &m_2xtileImage->m_data[0]);

		Rect srect = { m_tilesize / 2 - m_info->m_borderSize, m_tilesize / 2 - m_info->m_borderSize, m_pagesize, m_pagesize };
		image.copy({ 0,0 }, *m_2xtileImage, srect);
	}
}