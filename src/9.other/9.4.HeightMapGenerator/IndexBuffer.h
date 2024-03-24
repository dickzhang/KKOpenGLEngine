#pragma once

//#include<GL/glew.h>
#include <vector>

class IndexBuffer {
private:
	unsigned int bufferID;
	int count;
public:
	IndexBuffer(void* data, unsigned int count);
	IndexBuffer(const std::vector<unsigned int>& data);
	~IndexBuffer();
	void Bind();
	void Unbind();
	unsigned int GetCount();
};