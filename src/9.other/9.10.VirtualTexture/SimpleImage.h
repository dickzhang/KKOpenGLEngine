#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"

class SimpleImage
{
public:
	SimpleImage(int _width, int _height, int _channelCount, uint8_t _clearValue = 0);
	SimpleImage(int _width, int _height, int _channelCount, std::vector<uint8_t>& _data);

	void copy(TPoint dest_offset, SimpleImage& src, Rect src_rect);
	void clear(uint8_t clearValue = 0);
	void fill(Rect rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	static void mipmap(uint8_t* source, int size, int channels, uint8_t* dest);

public:
	std::vector<uint8_t> m_data;
	int	m_width = 0;
	int	m_height = 0;
	int	m_channelCount = 0;
};
