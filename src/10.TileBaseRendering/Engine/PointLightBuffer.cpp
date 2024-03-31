#include "PointLightBuffer.h"

int makeBy16(int number)
{
	int remainder = number % 16;
	int diff = (16 - remainder) % 16;
	int result = number + diff;
	return result;
}

void PointLightBuffer::Init(const std::vector<PointLight>& lights, int numTiles, int maxLigthsPerTile)
{
	lights_.Init(lights.data(), GL_SHADER_STORAGE_BUFFER, lights.size(), 1);
	//lights_.Bind();
	//PointLight* pointLights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	//for (int i = 0; i < lights.size(); i++)
	//{
	//	pointLights[i] = lights[i];
	//}
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	lights_.UnbindBuffer();
	int size = numTiles * maxLigthsPerTile;
	std::vector<int> initInd(size, -1);
	lightIndexes_.Init(initInd.data(), GL_SHADER_STORAGE_BUFFER, size, 2);
	lightIndexes_.UnbindBuffer();
}

void PointLightBuffer::Bind()
{
	lights_.Bind();
	lightIndexes_.Bind();
}
