#pragma once

#include <vector>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class CubedSphere
{
public:
    CubedSphere(glm::vec4 position, float radius);
    ~CubedSphere();

    std::vector<glm::vec4> GetVertices() const { return m_Vertices; }

    unsigned int GetNumOfVertices() const { return m_Vertices.size(); }

private:
    std::vector<glm::vec4> m_Vertices;

    void AddSideToCube(const std::vector<glm::vec4>& cubeSide, const glm::mat4& translation, const glm::mat4& rotation);
    void AddSideToCube(const std::vector<glm::vec4>& cubeSide, const glm::mat4& translation);
};