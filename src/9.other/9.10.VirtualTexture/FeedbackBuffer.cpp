#include "FeedbackBuffer.h"
#include "OpenGLTexture.h"
#include "OpenGLMappings.h"

// FeedbackBuffer
FeedbackBuffer::FeedbackBuffer(VirtualTextureInfo* _info,int _width,int _height)
	:m_stagingPool(_width,_height,1,true)
{
	m_info = _info;
	m_width = _width;
	m_height = _height;

	m_indexer = new PageIndexer(m_info);
	m_requests.resize(m_indexer->getCount());

	// ��ʼ�������������
	m_downloadBuffer.resize(m_width*m_height*s_channelCount);
	std::memset(&m_downloadBuffer[0],0,m_width*m_height*s_channelCount);
	clear();

	//��ʼ������֡������
	m_RenderTexture = new RenderTexture(m_width,m_height,2);
	m_lastStagingTexture = 0;
}

FeedbackBuffer::~FeedbackBuffer()
{
	if(m_indexer)
	{
		delete m_indexer;
		m_indexer = nullptr;
	}
	if(m_RenderTexture)
	{
		delete m_RenderTexture;
		m_RenderTexture = nullptr;
	}
}

void FeedbackBuffer::clear()
{
	// Clear Table
	std::memset(&m_requests[0],0,sizeof(int)*m_indexer->getCount());
}

void FeedbackBuffer::copy(unsigned short viewId)
{
	m_lastStagingTexture = m_stagingPool.getTexture();
	//���Ʒ�����������ȾĿ�굽�ݴ�����
	OpenGLTexture::blit(m_lastStagingTexture,0,0,0,m_RenderTexture->getColorAttachmentTex(0),0,m_width,m_height);
	m_stagingPool.next();
}

RenderTexture* FeedbackBuffer::GetFrameBuffer()
{
	return m_RenderTexture;
}

void FeedbackBuffer::download()
{
	//����Ƿ���һ���Ѿ���Ⱦ�ķ�������������
	if(m_lastStagingTexture==0)
	{
		return;
	}

	GLenum format = OpenGLMappings::Get(EPixelFormat::PF_A8R8G8B8);
	GLenum pixelType = OpenGLMappings::GetPixelType(EPixelFormat::PF_A8R8G8B8);
	//��ȡ��������
	OpenGLTexture::readTexture2D(m_lastStagingTexture,0,format,pixelType,&m_downloadBuffer[0]);
	//ѭ���������ز�����Ƿ�д�����κ�����
	auto data = &m_downloadBuffer[0];
	auto colors = (Color*)data;
	auto dataSize = m_width*m_height;

	for(int i = 0; i<dataSize; ++i)
	{
		auto& color = colors[i];
		if(color.m_a>=0xff)
		{
			//ҳ���ҵ�!������ӵ����������
			Page request = { color.m_b,color.m_g,color.m_r };
			addRequestAndParents(request);
			//������أ��������ǾͲ���Ҫ����һ��ͨ��������
			color = { 0,0,0,0 };
		}
	}
}

void FeedbackBuffer::addRequestAndParents(Page request)
{
	auto PageTableSizeLog2 = m_indexer->getMipCount();
	auto count = PageTableSizeLog2-request.m_mip;

	for(int i = 0; i<count; ++i)
	{
		int xpos = request.m_x>>i;
		int ypos = request.m_y>>i;

		Page page = { xpos,ypos,request.m_mip+i };

		//���������һ����Ч��ҳ��(λ�û�mip������Χ)����������
		if(!m_indexer->isValid(page))
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

int FeedbackBuffer::getWidth() const
{
	return m_width;
}

int FeedbackBuffer::getHeight() const
{
	return m_height;
}