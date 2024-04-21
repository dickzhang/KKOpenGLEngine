#include "PageIndexer.h"

PageIndexer::PageIndexer(VirtualTextureInfo* _info)
{
	m_info = _info;
	m_mipcount = int(glm::log2((float)m_info->GetPageTableSize()) + 1);
	m_sizes.resize(m_mipcount);
	for (int i = 0; i < m_mipcount; ++i)
	{
		m_sizes[i] = (m_info->m_virtualTextureSize / m_info->m_tileSize) >> i;
	}
	m_offsets.resize(m_mipcount);
	m_count = 0;

	for (int i = 0; i < m_mipcount; ++i)
	{
		m_offsets[i] = m_count;
		m_count += m_sizes[i] * m_sizes[i];
	}
	// ¼ÆËã·´ÏòÓ³Éä
	m_reverse.resize(m_count);

	for (int i = 0; i < m_mipcount; ++i)
	{
		int size = m_sizes[i];
		for (int y = 0; y < size; ++y)
		{
			for (int x = 0; x < size; ++x)
			{
				Page page = { x, y, i };
				m_reverse[getIndexFromPage(page)] = page;
			}
		}
	}
}
PageIndexer::~PageIndexer()
{

}

int PageIndexer::getIndexFromPage(Page page)
{
	int offset = m_offsets[page.m_mip];
	int stride = m_sizes[page.m_mip];
	return offset + page.m_y * stride + page.m_x;
}

Page PageIndexer::getPageFromIndex(int index)
{
	return m_reverse[index];
}

bool PageIndexer::isValid(Page page)
{
	if (page.m_mip < 0)
	{
		return false;
	}

	if (page.m_mip >= m_mipcount)
	{
		return false;
	}

	if (page.m_x < 0)
	{
		return false;
	}

	if (page.m_x >= m_sizes[page.m_mip])
	{
		return false;
	}

	if (page.m_y < 0)
	{
		return false;
	}

	if (page.m_y >= m_sizes[page.m_mip])
	{
		return false;
	}

	return true;
}

int PageIndexer::getCount() const
{
	return m_count;
}

int PageIndexer::getMipCount() const
{
	return m_mipcount;
}