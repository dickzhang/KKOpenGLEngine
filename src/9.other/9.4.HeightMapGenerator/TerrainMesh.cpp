#include "TerrainMesh.h"
#include "ObjLoader.h"

TerrainMesh::TerrainMesh(std::vector<glm::vec4> vertices, std::vector<glm::vec2> uvs, std::vector<glm::vec3> normals, std::vector<unsigned int> indices) {
	Buffer* vBuffer = new Buffer(vertices);
	Buffer* uvBuffer = new Buffer(uvs);
	Buffer* nBuffer = new Buffer(normals);

	vArray = new VertexArray();
	vArray->AddBuffer(vBuffer, 0);
	vArray->AddBuffer(uvBuffer, 1);
	vArray->AddBuffer(nBuffer, 2);

	iBuffer = new IndexBuffer(indices);
}
TerrainMesh::~TerrainMesh() {
	delete vArray;
	delete iBuffer;
}

VertexArray* TerrainMesh::GetVertexArray() {
	return vArray;
}
IndexBuffer* TerrainMesh::GetIndexBuffer() {
	return iBuffer;
}

TerrainMesh* TerrainMesh::LoadFromObj(const char* filename) {
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;
	ObjLoader::LoadObj(filename, vertices, uvs, normals, indices);

	return new TerrainMesh(vertices, uvs, normals, indices);
}