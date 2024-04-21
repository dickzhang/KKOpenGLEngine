#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Point
struct TPoint
{
	int m_x, m_y;
};
// Rect
struct Rect
{
	int minX() const
	{
		return m_x;
	}

	int minY() const
	{
		return m_y;
	}

	int maxX() const
	{
		return m_x + m_width;
	}

	int maxY() const
	{
		return m_y + m_height;
	}

	bool contains(const TPoint& p) const
	{
		return p.m_x >= minX() && p.m_y >= minY() && p.m_x < maxX() && p.m_y < maxY();
	}

	int m_x, m_y, m_width, m_height;
};

// Color
struct Color
{
	uint8_t m_r, m_g, m_b, m_a;
};

// Page
struct Page
{
	int m_x;
	int m_y;
	int m_mip;

	operator size_t() const
	{
		return size_t((uint32_t(m_mip) << 16) | uint32_t((uint16_t(m_x) << 8) | uint16_t(m_y)));
	}
};

// PageCount
struct PageCount
{
	Page m_page;
	int  m_count;

	PageCount(Page _page, int _count)
	{
		m_page = _page;
		m_count = _count;
	}

	int compareTo(const PageCount& other) const
	{
		if (other.m_page.m_mip != m_page.m_mip)
		{
			return glm::clamp<int>(other.m_page.m_mip - m_page.m_mip, -1, 1);
		}
		return glm::clamp<int>(other.m_count - m_count, -1, 1);
	}
};

// VirtualTextureInfo
struct VirtualTextureInfo
{
	int m_virtualTextureSize = 0;
	int m_tileSize = 64;
	int m_borderSize = 1;

	int GetPageSize() const
	{
		return m_tileSize + 2 * m_borderSize;
	}
	int GetPageTableSize() const
	{
		return m_virtualTextureSize / m_tileSize;
	}
};

struct Memory
{
	//Memory() = delete;
	uint8_t* data; //!< Pointer to data.
	uint32_t size; //!< Data size.
};

static const int s_channelCount = 4;
static const int s_tileFileDataOffset = sizeof(VirtualTextureInfo);