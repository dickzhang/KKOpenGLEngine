#pragma once

#include <glm\glm.hpp>
#include <vector>
#include "Buffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

class TerrainMesh{
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;

	VertexArray* vArray;
	IndexBuffer* iBuffer;
public:
	TerrainMesh(std::vector<glm::vec4> vertices, std::vector<glm::vec2> uvs, std::vector<glm::vec3> normals, std::vector<unsigned int> indices);
	~TerrainMesh();

	VertexArray* GetVertexArray();
	IndexBuffer* GetIndexBuffer();

	static TerrainMesh * LoadFromObj(const char* filename);
};
