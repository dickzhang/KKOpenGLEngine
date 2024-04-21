#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Common.h"
#include "VTCommon.h"
#include "StagingPool.h"
#include "PageIndexer.h"
#include "RenderTexture.h"

class FeedbackBuffer
{
public:
	FeedbackBuffer(VirtualTextureInfo* _info, int _width, int _height);
	~FeedbackBuffer();
	void clear();
	void copy(unsigned short  viewId);
	void download();

	//这个函数检测页面并添加页面的父级。这样做是为了在内存耗尽时可以返回到它们
	void addRequestAndParents(Page request);

	const std::vector<int>& getRequests() const;

	int getWidth() const;
	int getHeight() const;
private:
	VirtualTextureInfo* m_info = nullptr;
	PageIndexer* m_indexer = nullptr;
	int m_width = 0;
	int m_height = 0;
	StagingPool	m_stagingPool;
	unsigned short 	m_lastStagingTexture = 0;
	RenderTexture* m_RenderTexture=nullptr;
	//它按索引存储页面。int值是请求的数量。
	std::vector<int>		m_requests;
	std::vector<uint8_t>	m_downloadBuffer;
};
