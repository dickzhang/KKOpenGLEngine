#pragma once

//#include <GL/glew.h>
#include <glm\glm.hpp>
#include <vector>

class Buffer {
private:
	unsigned int bufferID;
	int componentCount;
public:
	Buffer(void* data, int count, int componentCount);
	Buffer(const std::vector<glm::vec2>& data);
	Buffer(const std::vector<glm::vec3>& data);
	Buffer(const std::vector<glm::vec4>& data);
	~Buffer();

	void Bind();
	void Unbind();

	int GetComponentCount();
};