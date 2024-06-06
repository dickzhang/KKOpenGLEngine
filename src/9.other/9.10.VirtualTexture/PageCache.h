#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <functional>
#include <unordered_set>
#include <scoped_allocator>
#include "Common.h"
#include "VTCommon.h"
#include "TextureAtlas.h"
#include "PageLoader.h"

class PageCache
{
public:
	struct LruPage
	{
		Page	m_page;
		TPoint	m_point;

		bool operator==(const LruPage& other) const
		{
			return m_page==other.m_page;
		}
	};

public:
	PageCache(TextureAtlas* _atlas,PageLoader* _loader,int _count);
	bool touch(Page page);
	bool request(Page request);
	void clear();
	void loadComplete(Page page,uint8_t* data);

	// These callbacks are used to notify the other systems
	std::function<void(Page,TPoint)> removed;
	std::function<void(Page,TPoint)> added;

private:
	TextureAtlas* m_atlas;
	PageLoader* m_loader;
	int m_count;
	int m_current; // This is used for generating the texture atlas indices before the lru is full
	//std::unordered_set<Page>    m_lru_used;
	std::vector<Page>    m_lru_used;
	std::vector<LruPage>		m_lru;
	//std::unordered_set<Page>	m_loading;
	std::vector<Page>	m_loading;
};
