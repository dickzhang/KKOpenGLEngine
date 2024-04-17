#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"
#include "SimpleImage.h"

class Quadtree
{
public:
	Quadtree(Rect _rect, int _level);
	~Quadtree();
	void             add(Page request, TPoint mapping);
	void             remove(Page request);
	void             write(SimpleImage& image, int miplevel);
	Rect			 getRectangle(int index);
	void		     write(Quadtree* node, SimpleImage& image, int miplevel);
	static Quadtree* findPage(Quadtree* node, Page request, int* index);
private:
	Rect      m_rectangle;
	int		  m_level;
	TPoint     m_mapping;
	Quadtree* m_children[4];
};
