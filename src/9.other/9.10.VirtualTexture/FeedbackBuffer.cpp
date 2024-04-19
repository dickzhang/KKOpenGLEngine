#include "FeedbackBuffer.h"
#include "OpenGLTexture.h"

// FeedbackBuffer
FeedbackBuffer::FeedbackBuffer(VirtualTextureInfo* _info, int _width, int _height)
	:m_stagingPool(_width, _height, 1, true)
{
	m_info = _info;
	m_width = _width;
	m_height = _height;

	m_indexer = new PageIndexer(m_info);
	m_requests.resize(m_indexer->getCount());

	// 初始化和清除缓冲区
	m_downloadBuffer.resize(m_width * m_height * s_channelCount);
	std::memset(&m_downloadBuffer[0], 0, m_width * m_height * s_channelCount);
	clear();

	TextureInfo info;
	info.width = m_width;
	info.height = m_height;
	info.hasmip = false;
	info.layernum = 1; 
	info.format = EPixelFormat::PF_A8R8G8B8;
	
	TextureInfo info1;
	info1.width = m_width;
	info1.height = m_height;
	info1.hasmip = false;
	info1.layernum = 1;
	info1.format = EPixelFormat::PF_R32G32B32A32;
	//初始化反馈帧缓冲区
	unsigned short feedbackFrameBufferTextures[] =
	{
		OpenGLTexture::generateTexture2D(info),
		OpenGLTexture::generateTexture2D(info1)
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
	glDeleteTransformFeedbacks(1, &m_feedbackFrameBuffer);
}

void FeedbackBuffer::clear()
{
	// Clear Table
	std::memset(&m_requests[0], 0, sizeof(int) * m_indexer->getCount());
}

void FeedbackBuffer::copy(unsigned short viewId)
{
	m_lastStagingTexture = m_stagingPool.getTexture();
	//复制反馈缓冲区渲染目标到暂存纹理
	bgfx::blit(viewId, m_lastStagingTexture, 0, 0, bgfx::getTexture(m_feedbackFrameBuffer));
	m_stagingPool.next();
}

void FeedbackBuffer::download()
{
	//检查是否有一个已经渲染的反馈缓冲区可用
	if (m_lastStagingTexture == 0)
	{
		return;
	}
	//读取纹理数据
	bgfx::readTexture(m_lastStagingTexture, &m_downloadBuffer[0]);
	//循环遍历像素并检查是否写入了任何内容
	auto data = &m_downloadBuffer[0];
	auto colors = (Color*)data;
	auto dataSize = m_width * m_height;

	for (int i = 0; i < dataSize; ++i)
	{
		auto& color = colors[i];
		if (color.m_a >= 0xff)
		{
			//页面找到!将其添加到请求队列中
			Page request = { color.m_b, color.m_g, color.m_r };
			addRequestAndParents(request);
			//清除像素，这样我们就不需要在另一个通道中做了
			color = { 0,0,0,0 };
		}
	}
}

void FeedbackBuffer::addRequestAndParents(Page request)
{
	auto PageTableSizeLog2 = m_indexer->getMipCount();
	auto count = PageTableSizeLog2 - request.m_mip;

	for (int i = 0; i < count; ++i)
	{
		int xpos = request.m_x >> i;
		int ypos = request.m_y >> i;

		Page page = { xpos, ypos, request.m_mip + i };

		//如果它不是一个有效的页面(位置或mip超出范围)，就跳过它
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