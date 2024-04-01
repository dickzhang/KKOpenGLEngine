#include "VAO.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void VAO::Unbid()
{
    glBindVertexArray(0);
}

VAO::VAO()
{
}

void VAO::Init()
{
    glGenVertexArrays(1, &id_);
}

void VAO::Bind()
{
    glBindVertexArray(id_);
}

VAO::~VAO()
{
    glDeleteBuffers(1, &id_);
}
