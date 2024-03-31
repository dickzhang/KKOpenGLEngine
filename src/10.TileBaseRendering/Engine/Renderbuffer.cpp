#include "Renderbuffer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

Renderbuffer::Renderbuffer()
{
}

void Renderbuffer::Init(unsigned int width, unsigned int height, unsigned int storageType)
{
    glGenRenderbuffers(1, &id_);
    glBindRenderbuffer(GL_RENDERBUFFER, id_); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Renderbuffer::Bind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, id_); 
}

Renderbuffer::~Renderbuffer()
{
}
