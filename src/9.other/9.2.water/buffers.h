#pragma once
#include <glad/glad.h>
#include "TextureUtil.h"

void bindFrameBuffer(int frameBuffer, int width, int height);

void unbindCurrentFrameBuffer(int scrWidth, int scrHeight);

unsigned int createFrameBuffer();

unsigned int createTextureAttachment(int width, int height);

unsigned int * createColorAttachments(int width, int height, unsigned int nColorAttachments);

unsigned int createDepthTextureAttachment(int width, int height);

unsigned int createDepthBufferAttachment(int width, int height);

unsigned int createRenderBufferAttachment(int width, int height);

class FrameBufferObject {
public:
	FrameBufferObject(int W, int H);
	FrameBufferObject(int W, int H, int numColorAttachments);
	unsigned int FBO, renderBuffer, depthTex;
	unsigned int tex;
	unsigned int getColorAttachmentTex(int i);
	void bind();
private:
	int W, H;
	int nColorAttachments;
	unsigned int * colorAttachments;
};
