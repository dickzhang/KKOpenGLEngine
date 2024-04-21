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

	// 初始化和清除缓冲区
	m_downloadBuffer.resize(m_width*m_height*s_channelCount);
	std::memset(&m_downloadBuffer[0],0,m_width*m_height*s_channelCount);
	clear();

	//初始化反馈帧缓冲区
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
	//复制反馈缓冲区渲染目标到暂存纹理
	OpenGLTexture::blit(m_lastStagingTexture,0,0,0,m_RenderTexture->getColorAttachmentTex(0),0,m_width,m_height);
	m_stagingPool.next();
}

RenderTexture* FeedbackBuffer::GetFrameBuffer()
{
	return m_RenderTexture;
}

void FeedbackBuffer::download()
{
	//检查是否有一个已经渲染的反馈缓冲区可用
	if(m_lastStagingTexture==0)
	{
		return;
	}

	GLenum format = OpenGLMappings::Get(EPixelFormat::PF_A8R8G8B8);
	GLenum pixelType = OpenGLMappings::GetPixelType(EPixelFormat::PF_A8R8G8B8);
	//读取纹理数据
	OpenGLTexture::readTexture2D(m_lastStagingTexture,0,format,pixelType,&m_downloadBuffer[0]);
	//循环遍历像素并检查是否写入了任何内容
	auto data = &m_downloadBuffer[0];
	auto colors = (Color*)data;
	auto dataSize = m_width*m_height;

	for(int i = 0; i<dataSize; ++i)
	{
		auto& color = colors[i];
		if(color.m_a>=0xff)
		{
			//页面找到!将其添加到请求队列中
			Page request = { color.m_b,color.m_g,color.m_r };
			addRequestAndParents(request);
			//清除像素，这样我们就不需要在另一个通道中做了
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

		//如果它不是一个有效的页面(位置或mip超出范围)，就跳过它
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