#include "PageCache.h"

PageCache::PageCache(TextureAtlas* _atlas, PageLoader* _loader, int _count)
	: m_atlas(_atlas)
	, m_loader(_loader)
	, m_count(_count)
{
	clear();
	m_loader->loadComplete = [&](Page page, uint8_t* data)
	{
		loadComplete(page, data);
	};
}

// Update the pages's position in the lru
bool PageCache::touch(Page page)
{
	if (m_loading.find(page) == m_loading.end())
	{
		if (m_lru_used.find(page) != m_lru_used.end())
		{
			// Find the page (slow!!) and add it to the back of the list
			for (auto it = m_lru.begin(); it != m_lru.end(); ++it)
			{
				if (it->m_page == page)
				{
					auto lruPage = *it;
					m_lru.erase(it);
					m_lru.push_back(lruPage);
					return true;
				}
			}
			return false;
		}
	}

	return false;
}

// Schedule a load if not already loaded or loading
bool PageCache::request(Page request, unsigned short  blitViewId)
{
	m_blitViewId = blitViewId;
	if (m_loading.find(request) == m_loading.end())
	{
		if (m_lru_used.find(request) == m_lru_used.end())
		{
			m_loading.insert(request);
			m_loader->submit(request);
			return true;
		}
	}

	return false;
}

void PageCache::clear()
{
	for (auto& lru_page : m_lru)
	{
		if (m_lru_used.find(lru_page.m_page) != m_lru_used.end())
		{
			removed(lru_page.m_page, lru_page.m_point);
		}
	}
	m_lru_used.clear();
	m_lru.clear();
	m_lru.reserve(m_count * m_count);
	m_current = 0;
}

void PageCache::loadComplete(Page page, uint8_t* data)
{
	m_loading.erase(page);

	// Find a place in the atlas for the data
	TPoint pt;

	if (m_current == m_count * m_count)
	{
		// Remove the oldest lru page and remember it's location so we can use it
		auto lru_page = m_lru[0];
		m_lru.erase(m_lru.begin());
		m_lru_used.erase(lru_page.m_page);
		pt = lru_page.m_point;
		// Notify that we removed a page
		removed(lru_page.m_page, lru_page.m_point);
	}
	else
	{
		pt = { m_current % m_count, m_current / m_count };
		++m_current;

		if (m_current == m_count * m_count)
		{
			std::cout << "Atlas is full!" << std::endl;
		}
	}

	// Notify atlas that he can upload the page and add the page to lru
	m_atlas->uploadPage(pt, data, m_blitViewId);
	m_lru.push_back({ page, pt });
	m_lru_used.insert(page);

	// Signal that we added a page
	added(page, pt);
}