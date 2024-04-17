#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"

class TileDataFile
{
public:
	TileDataFile(const bx::FilePath& filename, VirtualTextureInfo* _info, bool _readWrite = false);
	~TileDataFile();

	void readInfo();
	void writeInfo();

	void readPage(int index, uint8_t* data);
	void writePage(int index, uint8_t* data);

private:
	VirtualTextureInfo* m_info;
	int					m_size;
	FILE* m_file;
};
