#pragma once

#include "ImageUtils.h"
#include "TerrainMesh.h"

class Terrain {
public:
	static float size;
	static float maxHeight;
private:
	static const float maxPixelColor;

	static int cells;
	static float** heights;

	static void CalculateHeights(FIBITMAP* data);
	static void SmoothHeights(float alpha);
	static glm::vec3 CalculateNormal(const int& x, const int& y);
	static float GetHeight(FIBITMAP* data, const int& x, const int& y);

public:
	static TerrainMesh * GenerateMesh(const char* heightMap);
};
