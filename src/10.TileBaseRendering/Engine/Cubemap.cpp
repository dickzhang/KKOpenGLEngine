#include "Cubemap.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

#include "stb_image.h"
#include "ShaderProgram.h"

Cubemap::Cubemap(/* args */)
{
}

void Cubemap::Bind()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
}

void Cubemap::Draw(ShaderProgram& shader)
{
    shader.UseProgram();
    SetupMatrices(shader);
    glDepthFunc(GL_LEQUAL);

    Bind();
    shader.SetUniformValue("skybox", 0);
    vao_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
}

void Cubemap::BindCubemapFaces(const std::string& cubemapFolder)
{
    std::vector<std::string> faces;
    faces.push_back(cubemapFolder + "/right.jpg");
    faces.push_back(cubemapFolder + "/left.jpg");
    faces.push_back(cubemapFolder + "/top.jpg");
    faces.push_back(cubemapFolder + "/bottom.jpg");
    faces.push_back(cubemapFolder + "/front.jpg");
    faces.push_back(cubemapFolder + "/back.jpg");


    stbi_set_flip_vertically_on_load(false);

    for(unsigned int i = 0; i < faces.size(); i++)
    {
        int width, height, nrChannels;
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data != nullptr)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load cubemape face: " << faces[i] << "\n";
        }
    }
}

void Cubemap::SpecifyWrappingAndFiltering()
{
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
}

Cubemap::~Cubemap()
{
}
