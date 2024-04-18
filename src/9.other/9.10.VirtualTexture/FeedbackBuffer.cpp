#include "FeedbackBuffer.h"

// FeedbackBuffer
FeedbackBuffer::FeedbackBuffer(VirtualTextureInfo* _info, int _width, int _height)
	:m_stagingPool(_width, _height, 1, true)
{
	m_info = _info;
	m_width = _width;
	m_height = _height;

	// Setup classes
	m_indexer = new PageIndexer(m_info);
	m_requests.resize(m_indexer->getCount());

	// Initialize and clear buffers
	m_downloadBuffer.resize(m_width * m_height * s_channelCount);
	std::memset(&m_downloadBuffer[0], 0, m_width * m_height * s_channelCount);
	clear();

	// Initialize feedback frame buffer
	unsigned short feedbackFrameBufferTextures[] =
	{
		bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT),
		bgfx::createTexture2D(uint16_t(m_width), uint16_t(m_height), false, 1, bgfx::TextureFormat::D32F,  BGFX_TEXTURE_RT),
	};

	m_feedbackFrameBuffer = bgfx::createFrameBuffer(BX_COUNTOF(feedbackFrameBufferTextures), feedbackFrameBufferTextures, true);
	m_lastStagingTexture = 0;
}

FeedbackBuffer::~FeedbackBuffer()
{
	if (m_indexer)
	{
		delete m_indexer;
		m_indexer = nullptr;
	}
	glDeleteFramebuffers(1, &m_feedbackFrameBuffer);
}

void FeedbackBuffer::clear()
{
	// Clear Table
	std::memset(&m_requests[0], 0, sizeof(int) * m_indexer->getCount());
}

void FeedbackBuffer::copy(unsigned short  viewId)
{
	m_lastStagingTexture = m_stagingPool.getTexture();
	// Copy feedback buffer render target to staging texture
	bgfx::blit(viewId, m_lastStagingTexture, 0, 0, bgfx::getTexture(m_feedbackFrameBuffer));
	m_stagingPool.next();
}

void FeedbackBuffer::download()
{
	// Check if there's an already rendered feedback buffer available
	if (m_lastStagingTexture == 0)
	{
		return;
	}

	// Read the texture
	bgfx::readTexture(m_lastStagingTexture, &m_downloadBuffer[0]);
	// Loop through pixels and check if anything was written
	auto data = &m_downloadBuffer[0];
	auto colors = (Color*)data;
	auto dataSize = m_width * m_height;

	for (int i = 0; i < dataSize; ++i)
	{
		auto& color = colors[i];
		if (color.m_a >= 0xff)
		{
			// Page found! Add it to the request queue
			Page request = { color.m_b, color.m_g, color.m_r };
			addRequestAndParents(request);
			// Clear the pixel, so that we don't have to do it in another pass
			color = { 0,0,0,0 };
		}
	}
}

// This function validates the pages and adds the page's parents
// We do this so that we can fall back to them if we run out of memory
void FeedbackBuffer::addRequestAndParents(Page request)
{
	auto PageTableSizeLog2 = m_indexer->getMipCount();
	auto count = PageTableSizeLog2 - request.m_mip;

	for (int i = 0; i < count; ++i)
	{
		int xpos = request.m_x >> i;
		int ypos = request.m_y >> i;

		Page page = { xpos, ypos, request.m_mip + i };

		// If it's not a valid page (position or mip out of range) just skip it
		if (!m_indexer->isValid(page))
		{
			return;
		}
		++m_requests[m_indexer->getIndexFromPage(page)];
	}
}

const std::vector<int>& FeedbackBuffer::getRequests() const
{
	return m_requests;
}

unsigned short  FeedbackBuffer::getFrameBuffer()
{
	return m_feedbackFrameBuffer;
}

int FeedbackBuffer::getWidth() const
{
	return m_width;
}

int FeedbackBuffer::getHeight() const
{
	return m_height;
}