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

	// ��ʼ�������������
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
	//��ʼ������֡������
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
	//���Ʒ�����������ȾĿ�굽�ݴ�����
	bgfx::blit(viewId, m_lastStagingTexture, 0, 0, bgfx::getTexture(m_feedbackFrameBuffer));
	m_stagingPool.next();
}

void FeedbackBuffer::download()
{
	//����Ƿ���һ���Ѿ���Ⱦ�ķ�������������
	if (m_lastStagingTexture == 0)
	{
		return;
	}
	//��ȡ��������
	bgfx::readTexture(m_lastStagingTexture, &m_downloadBuffer[0]);
	//ѭ���������ز�����Ƿ�д�����κ�����
	auto data = &m_downloadBuffer[0];
	auto colors = (Color*)data;
	auto dataSize = m_width * m_height;

	for (int i = 0; i < dataSize; ++i)
	{
		auto& color = colors[i];
		if (color.m_a >= 0xff)
		{
			//ҳ���ҵ�!������ӵ����������
			Page request = { color.m_b, color.m_g, color.m_r };
			addRequestAndParents(request);
			//������أ��������ǾͲ���Ҫ����һ��ͨ��������
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

		//���������һ����Ч��ҳ��(λ�û�mip������Χ)����������
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