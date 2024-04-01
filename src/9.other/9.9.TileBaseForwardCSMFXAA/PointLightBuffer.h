#pragma once

#include <vector>

#include "PointLight.h"
#include "StorageBuffer.h"

class PointLightBuffer
{
public:
	void Init(const std::vector<PointLight>& lights, int numTiles, int maxLigthsPerTile);
	void Bind();
	
private:

	StorageBuffer<PointLight> lights_;
	StorageBuffer<int> lightIndexes_;
};

