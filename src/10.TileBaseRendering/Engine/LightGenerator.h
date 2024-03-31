#pragma once

#include <vector>
#include <string>
#include <random>

#include "PointLight.h"

class LightGenerator
{
public:
    LightGenerator();
    ~LightGenerator();

    std::vector<PointLight>& GetLights() { return lights_; }

private:
    glm::vec4 GenerateRandomPosition(std::uniform_real_distribution<>& dis, std::mt19937& gen);
    
    static const glm::vec4 LIGHT_MIN_BOUNDS;
    static const glm::vec4 LIGHT_MAX_BOUNDS;
    static const unsigned int NUM_OF_LIGHTS;

    std::vector<PointLight> lights_;
};