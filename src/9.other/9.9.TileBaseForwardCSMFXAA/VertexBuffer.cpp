#include "VertexBuffer.h"
#include <cstring>

void VertexBuffer::AddVertexAtributeDescription(unsigned int index, unsigned int numElements,
 	                                  unsigned int elementType, bool normalized,
 	                                  int stride, const void * pointer)
{
    glVertexAttribPointer(index, numElements, elementType, normalized ? GL_TRUE : GL_FALSE, stride, pointer);
    glEnableVertexAttribArray(index);
}

VertexBuffer::VertexBuffer()
{
}

void VertexBuffer::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, id_);
}

VertexBuffer::~VertexBuffer()
{
}
