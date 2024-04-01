#pragma once
#include <glm/glm.hpp>

struct PointLight
{
    glm::vec4 position;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float constant;
    float linear;
    float quadratic;  
    float radius;
};