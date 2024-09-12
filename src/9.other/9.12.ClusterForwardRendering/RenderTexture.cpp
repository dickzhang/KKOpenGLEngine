#include "RenderTexture.h"

RenderTexture::RenderTexture(int W_, int H_)
{
	this->W = W_;
	this->H = H_;
	this->FBO = CreateFrameBuffer();
	this->tex = CreateTextureAttachment(W, H);
	this->depthTex = CreateDepthTextureAttachment(W, H);
	colorAttachments = NULL;
	nColorAttachments = 1;
}

RenderTexture::RenderTexture(int W, int H, bool isOnlyDepty)
{
	this->W = W;
	this->H = H;
	this->FBO = CreateFrameBuffer(isOnlyDepty);
	if (!isOnlyDepty)
	{
		this->tex = CreateTextureAttachment(W, H);
	}
	this->depthTex = CreateDepthTextureAttachment(W, H);
	colorAttachments = NULL;
	nColorAttachments = 1;
}

RenderTexture::RenderTexture(int W_, int H_, const int nColorAttachments)
{
	this->W = W_;
	this->H = H_;
	this->FBO = CreateFrameBuffer();

	this->tex = NULL;
	this->depthTex = CreateDepthTextureAttachment(W, H);
	this->colorAttachments = CreateColorAttachments(W, H, nColorAttachments);
	this->nColorAttachments = nColorAttachments;

	unsigned int* colorAttachmentsFlag = new unsigned int[nColorAttachments];
	for (unsigned int i = 0; i < nColorAttachments; i++)
	{
		colorAttachmentsFlag[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(nColorAttachments, colorAttachmentsFlag);
	delete colorAttachmentsFlag;
}

RenderTexture::~RenderTexture()
{
	std::cout << "~RenderTexture" << std::endl;
}

void RenderTexture::Bind()
{
	BindFrameBuffer(this->FBO, this->W, this->H);
}

void RenderTexture::Unbind()
{
	UnbindCurrentFrameBuffer(this->W, this->H);
}

unsigned int RenderTexture::getColorAttachmentTex(int i)
{
	if (i < 0 || i > nColorAttachments)
	{
		std::cout << "COLOR ATTACHMENT OUT OF RANGE" << std::endl;
		return 0;
	}
	return colorAttachments[i];
}

unsigned int RenderTexture::GetDepthTexture()
{
	return depthTex;
}

void RenderTexture::BindFrameBuffer(int frameBuffer, int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, width, height);
}

void RenderTexture::UnbindCurrentFrameBuffer(int scrWidth, int scrHeight)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, scrWidth, scrHeight);
}

unsigned int RenderTexture::CreateFrameBuffer(bool isOnlyDepty)
{
	unsigned int frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	if (isOnlyDepty)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else
	{
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}
	return frameBuffer;
}

unsigned int RenderTexture::CreateTextureAttachment(int width, int height)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	return texture;
}

unsigned int* RenderTexture::CreateColorAttachments(int width, int height, unsigned int nColorAttachments)
{
	unsigned int* colorAttachments = new unsigned int[nColorAttachments];
	glGenTextures(nColorAttachments, colorAttachments);
	for (unsigned int i = 0; i < nColorAttachments; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorAttachments[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorAttachments[i], 0);
	}
	return colorAttachments;
}

unsigned int RenderTexture::CreateDepthTextureAttachment(int width, int height)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
	return texture;
}

unsigned int RenderTexture::CreateDepthBufferAttachment(int width, int height)
{
	unsigned int depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	return depthBuffer;
}

unsigned int RenderTexture::CreateRenderBufferAttachment(int width, int height)
{
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	return rbo;
}

