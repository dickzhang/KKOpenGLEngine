#include "Quadtree.h"

Quadtree::Quadtree(Rect _rect, int _level)
{
	m_rectangle = _rect;
	m_level = _level;
	for (int i = 0; i < 4; ++i)
	{
		m_children[i] = nullptr;
	}
}

Quadtree::~Quadtree()
{
	for (int i = 0; i < 4; ++i)
	{
		if (m_children[i] != nullptr)
		{
			delete m_children[i];
			m_children[i] = nullptr;
		}
	}
}

void Quadtree::add(Page request, TPoint mapping)
{
	int scale = 1 << request.m_mip; // Same as pow( 2, mip )
	int x = request.m_x * scale;
	int y = request.m_y * scale;

	Quadtree* node = this;

	while (request.m_mip < node->m_level)
	{
		for (int i = 0; i < 4; ++i)
		{
			auto rect = node->getRectangle(i);
			if (rect.contains({ x, y }))
			{
				// Create a new one if needed
				if (node->m_children[i] == nullptr)
				{
					node->m_children[i] = new Quadtree(rect, node->m_level - 1);
					node = node->m_children[i];
					break;
				}
				// Otherwise traverse the tree
				else
				{
					node = node->m_children[i];
					break;
				}
			}
		}
	}
	// We have created the correct node, now set the mapping
	node->m_mapping = mapping;
}

void Quadtree::remove(Page request)
{
	int  index;
	auto node = findPage(this, request, &index);

	if (node != nullptr)
	{
		delete node->m_children[index];
		node->m_children[index] = nullptr;
	}
}

void Quadtree::write(SimpleImage& image, int miplevel)
{
	write(this, image, miplevel);
}

Rect Quadtree::getRectangle(int index)
{
	int x = m_rectangle.m_x;
	int y = m_rectangle.m_y;
	int w = m_rectangle.m_width / 2;
	int h = m_rectangle.m_width / 2;

	switch (index)
	{
		case 0: return { x    , y    , w, h };
		case 1: return { x + w, y    , w, h };
		case 2: return { x + w, y + h, w, h };
		case 3: return { x    , y + h, w, h };
		default: break;
	}

	return { 0, 0, 0, 0 };
}

void Quadtree::write(Quadtree* node, SimpleImage& image, int miplevel)
{
	if (node->m_level >= miplevel)
	{
		int rx = node->m_rectangle.m_x >> miplevel;
		int ry = node->m_rectangle.m_y >> miplevel;
		int rw = node->m_rectangle.m_width >> miplevel;
		int rh = node->m_rectangle.m_width >> miplevel;

		image.fill({ rx, ry, rw, rh }, (uint8_t)node->m_mapping.m_x, (uint8_t)node->m_mapping.m_y, (uint8_t)node->m_level, 255);

		for (int i = 0; i < 4; ++i)
		{
			auto child = node->m_children[i];
			if (child != nullptr)
			{
				Quadtree::write(child, image, miplevel);
			}
		}
	}
}

Quadtree* Quadtree::findPage(Quadtree* node, Page request, int* index)
{
	int scale = 1 << request.m_mip; // Same as pow( 2, mip )
	int x = request.m_x * scale;
	int y = request.m_y * scale;

	// Find the parent of the child we want to remove
	bool exitloop = false;
	while (!exitloop)
	{
		exitloop = true;
		for (int i = 0; i < 4; ++i)
		{
			if (node->m_children[i] != nullptr && node->m_children[i]->m_rectangle.contains({ x, y }))
			{
				// We found it
				if (request.m_mip == node->m_level - 1)
				{
					*index = i;
					return node;
				}
				// Check the children
				else
				{
					node = node->m_children[i];
					exitloop = false;
				}
			}
		}
	}
	// We couldn't find it so it must not exist anymore
	*index = -1;
	return nullptr;
}