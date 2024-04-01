#include "Framebuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void Framebuffer::BindDefault()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::Framebuffer()
{
}

void Framebuffer::Init(bool isDepthOnly)
{
    glGenFramebuffers(1, &id_);
    Bind();
    
    if (isDepthOnly)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
}

void Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}


void Framebuffer::AttachTexture(unsigned int attachmentType, unsigned int textureId, unsigned int textureTarget)
{
    if (textureTarget == GL_TEXTURE_2D)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, textureTarget, textureId, 0);
    }
    else
    {
        glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, textureId, 0);
    }
}


void Framebuffer::AttachRenderbuffer(unsigned int attachmentType, unsigned int renderbufferId)
{
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, renderbufferId);
}

bool Framebuffer::IsComplete() const
{
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

Framebuffer::~Framebuffer()
{
}
