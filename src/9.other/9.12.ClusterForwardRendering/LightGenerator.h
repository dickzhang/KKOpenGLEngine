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
	std::vector<PointLight>& GetLights();
private:
	glm::vec4 GenerateRandomPosition(std::uniform_real_distribution<>& dis,std::mt19937& gen);
	glm::vec4 GenerateRandomColor(std::uniform_real_distribution<>& dis,std::mt19937& gen);
public:
	static unsigned int NUM_OF_LIGHTS;
private:
	static const glm::vec4 LIGHT_MIN_BOUNDS;
	static const glm::vec4 LIGHT_MAX_BOUNDS;
	static const glm::vec4 LIGHT_MIN_BOUNDS_COLOR;
	static const glm::vec4 LIGHT_MAX_BOUNDS_COLOR;
	std::vector<PointLight> lights_;
};