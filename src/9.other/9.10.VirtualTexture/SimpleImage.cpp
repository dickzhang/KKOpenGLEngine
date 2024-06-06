#include "SimpleImage.h"
#include "Quadtree.h"

SimpleImage::SimpleImage(int _width, int _height, int _channelCount, uint8_t _clearValue)
{
	m_width = _width;
	m_height = _height;
	m_channelCount = _channelCount;
	m_data.resize(m_width * m_height * m_channelCount);
	clear(_clearValue);
}

SimpleImage::SimpleImage(int _width, int _height, int _channelCount, std::vector<uint8_t>& _data)
{
	m_width = _width;
	m_height = _height;
	m_channelCount = _channelCount;
	m_data = _data;
}

void SimpleImage::copy(TPoint dest_offset, SimpleImage& src, Rect src_rect)
{
	int width = glm::min(m_width - dest_offset.m_x, src_rect.m_width);
	int height = glm::min(m_height - dest_offset.m_y, src_rect.m_height);
	int channels = glm::min(m_channelCount, src.m_channelCount);

	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			int i1 = ((j + dest_offset.m_y) * m_width + (i + dest_offset.m_x)) * m_channelCount;
			int i2 = ((j + src_rect.m_y) * src.m_width + (i + src_rect.m_x)) * src.m_channelCount;
			for (int c = 0; c < channels; ++c)
			{
				m_data[i1 + c] = src.m_data[i2 + c];
			}
		}
	}
}

void SimpleImage::clear(uint8_t clearValue)
{
	std::memset(&m_data[0], clearValue, m_width * m_height * m_channelCount);
}

void SimpleImage::fill(Rect rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	for (int y = rect.minY(); y < rect.maxY(); ++y)
	{
		for (int x = rect.minX(); x < rect.maxX(); ++x)
		{
			m_data[m_channelCount * (y * m_width + x) + 0] = b;
			m_data[m_channelCount * (y * m_width + x) + 1] = g;
			m_data[m_channelCount * (y * m_width + x) + 2] = r;
			m_data[m_channelCount * (y * m_width + x) + 3] = a;
		}
	}
}

void SimpleImage::mipmap(uint8_t* source, int size, int channels, uint8_t* dest)
{
	int mipsize = size / 2;

	for (int y = 0; y < mipsize; ++y)
	{
		for (int x = 0; x < mipsize; ++x)
		{
			for (int c = 0; c < channels; ++c)
			{
				int index = channels * ((y * 2) * size + (x * 2)) + c;
				int sum_value = 4 >> 1;
				sum_value += source[index + channels * (0 * size + 0)];
				sum_value += source[index + channels * (0 * size + 1)];
				sum_value += source[index + channels * (1 * size + 0)];
				sum_value += source[index + channels * (1 * size + 1)];
				dest[channels * (y * mipsize + x) + c] = (uint8_t)(sum_value / 4);
			}
		}
	}
}