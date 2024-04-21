#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"

class PageIndexer
{
public:
	PageIndexer(VirtualTextureInfo _info);
	~PageIndexer();
	int  getIndexFromPage(Page page);
	Page getPageFromIndex(int index);
	bool isValid(Page page);
	int  getCount() const;
	int  getMipCount() const;

private:
	VirtualTextureInfo m_info;
	int                 m_mipcount;
	int					m_count;
	// This stores the offsets to the first page of the start of a mipmap level
	std::vector<int>    m_offsets;
	// This stores the sizes of various mip levels
	std::vector<int>    m_sizes;
	std::vector<Page>   m_reverse;
};
