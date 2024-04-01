#include "FBO.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void FBO::BindDefault()
{
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

FBO::FBO()
{
}

void FBO::Init(bool isDepthOnly)
{
    glGenFramebuffers(1,&id_);
    Bind();

    if(isDepthOnly)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
}

void FBO::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER,id_);
}


void FBO::AttachTexture(unsigned int attachmentType,unsigned int textureId,unsigned int textureTarget)
{
    if(textureTarget==GL_TEXTURE_2D)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER,attachmentType,textureTarget,textureId,0);
    }
    else
    {
        glFramebufferTexture(GL_FRAMEBUFFER,attachmentType,textureId,0);
    }
}


void FBO::AttachRenderbuffer(unsigned int attachmentType,unsigned int renderbufferId)
{
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,attachmentType,GL_RENDERBUFFER,renderbufferId);
}

bool FBO::IsComplete() const
{
    return glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
}

FBO::~FBO()
{
}
