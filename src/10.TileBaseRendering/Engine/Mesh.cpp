#include "Mesh.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

#include "ShaderProgram.h"

Mesh::Mesh()
{
}

void Mesh::Draw(ShaderProgram& shader)
{
    shader.UseProgram();
    SetupMatrices(shader);
    if (material_.diffuseMap.GetId() != 0)
    {
        shader.SetUniformValue("material", material_);
        material_.diffuseMap.BindTexture();
        material_.specularMap.BindTexture();
    }
    
    // Draw mesh
    vao_.Bind();
    
    if (ibo_.GetNumIndicies() > 0)
    {
        glDrawElements(GL_TRIANGLES, ibo_.GetNumIndicies(), GL_UNSIGNED_INT, 0); 
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, vbo_.GetNumVertices());
    }
    VAO::Unbid();
    
}

Mesh::~Mesh()
{
}
