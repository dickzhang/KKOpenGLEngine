#pragma once
#include <glad/glad.h>
#include "OpenGLTexture.h"

class RenderTexture
{
public:
	RenderTexture(int W, int H);
	RenderTexture(int W, int H, bool isOnlyDepty);
	RenderTexture(int W, int H, int numColorAttachments);
	~RenderTexture();
	unsigned int getColorAttachmentTex(int i);
	unsigned int GetDepthTexture();
	void Bind();
	void Unbind();
private:

	void BindFrameBuffer(int frameBuffer, int width, int height);

	void UnbindCurrentFrameBuffer(int scrWidth, int scrHeight);

	unsigned int CreateFrameBuffer(bool isOnlyDepty=false);

	unsigned int CreateTextureAttachment(int width, int height);

	unsigned int* CreateColorAttachments(int width, int height, unsigned int nColorAttachments);

	unsigned int CreateDepthTextureAttachment(int width, int height);

	unsigned int CreateDepthBufferAttachment(int width, int height);

	unsigned int CreateRenderBufferAttachment(int width, int height);
private:
	unsigned int FBO, renderBuffer, depthTex;
	unsigned int tex;
	int W, H;
	int nColorAttachments;
	unsigned int* colorAttachments;
};
