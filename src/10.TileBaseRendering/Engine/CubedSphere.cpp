#include "CubedSphere.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


CubedSphere::CubedSphere(glm::vec4 position, float radius)
{

    int divisions = 4;
    float length = 1.f / (float)divisions;

    std::vector<glm::vec4> cubeSide;

    for (int i = 0; i < divisions; i++)
    {
        for (int j = 0; j < divisions; j++)
        {
            float x1 = i * length - 0.5;
            float y1 = j * length - 0.5;

            float x2 = x1 + length;
            float y2 = y1 + length;

            cubeSide.push_back(glm::vec4(x1, y1, 0.0f, 1.0f));
            cubeSide.push_back(glm::vec4(x2, y1, 0.0f, 1.0f));
            cubeSide.push_back(glm::vec4(x2, y2, 0.0f, 1.0f));
            cubeSide.push_back(glm::vec4(x2, y2, 0.0f, 1.0f));
            cubeSide.push_back(glm::vec4(x1, y2, 0.0f, 1.0f));
            cubeSide.push_back(glm::vec4(x1, y1, 0.0f, 1.0f));
        }
    }

    glm::mat4 zTranslate = glm::mat4(1.0f);
    zTranslate = glm::translate(zTranslate, glm::vec3(0.0f, 0.0f, 0.5f));

    //back face
    AddSideToCube(cubeSide, zTranslate);

    // front face
    glm::mat4 yRotate180 = glm::rotate(glm::mat4(1.0f), (float)glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    AddSideToCube(cubeSide, zTranslate, yRotate180);

    // left face`
    glm::mat4 yRotate90 = glm::rotate(glm::mat4(1.0f), (float)glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    AddSideToCube(cubeSide, zTranslate, yRotate90);

    // right face
    glm::mat4 yRotate270 = glm::rotate(glm::mat4(1.0f), (float)glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    AddSideToCube(cubeSide, zTranslate, yRotate270);

    // top face
    glm::mat4 xRotate90 = glm::rotate(glm::mat4(1.0f), (float)glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    AddSideToCube(cubeSide, zTranslate, xRotate90);

    // bottom face
    glm::mat4 xRotate270 = glm::rotate(glm::mat4(1.0f), (float)glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    AddSideToCube(cubeSide, zTranslate, xRotate270);

    for (int i = 0; i < m_Vertices.size(); i++)
    {
        glm::vec3 norm = glm::normalize(glm::vec3(m_Vertices[i]));
        m_Vertices[i] = glm::vec4(norm * radius, 1.0f);
    }

    for (int i = 0; i < m_Vertices.size(); i++)
    {
        glm::vec3 norm = glm::normalize(glm::vec3(m_Vertices[i]));
        m_Vertices[i] = glm::vec4(norm * radius, 1.0f);
        m_Vertices[i] = m_Vertices[i] + position;
        glm::mat4 xRotate270 = glm::rotate(glm::mat4(1.0f), (float)glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    }

}

void CubedSphere::AddSideToCube(const std::vector<glm::vec4>& cubeSide, const glm::mat4& translation,
    const glm::mat4& rotation)
{
    for (int i = 0; i < cubeSide.size(); i++)
    {
        m_Vertices.push_back(rotation * translation * cubeSide[i]);
    }
}

void CubedSphere::AddSideToCube(const std::vector<glm::vec4>& cubeSide, const glm::mat4& translation)
{
    for (int i = 0; i < cubeSide.size(); i++)
    {
        m_Vertices.push_back(translation * cubeSide[i]);
    }
}

CubedSphere::~CubedSphere()
{
}