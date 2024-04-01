#include "LightGenerator.h"

#include <iostream>
#include <fstream>
#include <sstream>

const unsigned int LightGenerator::NUM_OF_LIGHTS = 1024;
const glm::vec4 LightGenerator::LIGHT_MIN_BOUNDS = glm::vec4(-2000.0f, 5.0f, -1000.0f, 0.0f);
const glm::vec4 LightGenerator::LIGHT_MAX_BOUNDS = glm::vec4(2000.0f, 1000.0f, 1000.0f, 0.0f);


LightGenerator::LightGenerator()
{
    std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1);

	for (int i = 0; i < NUM_OF_LIGHTS; i++)
    {
		PointLight light;
        light.position = GenerateRandomPosition(dis, gen);
        light.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 0.0f);
        light.diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 0.0f);
        light.specular = glm::vec4(1.f, 1.f, 1.f, 0.0f);
        light.constant = 1.0f;
        light.linear = 0.0045f;
        light.quadratic = 0.00075f;
        light.radius = 100;

        lights_.push_back(light);
	}
}

glm::vec4 LightGenerator::GenerateRandomPosition(std::uniform_real_distribution<>& dis, std::mt19937& gen)
{
	glm::vec4 position = glm::vec4(0.0);

	for (int i = 0; i < 3; i++)
    {
		float min = LIGHT_MIN_BOUNDS[i];
		float max = LIGHT_MAX_BOUNDS[i];
		position[i] = dis(gen) * (max - min) + min;
	}

	return position;
}

LightGenerator::~LightGenerator()
{
}
