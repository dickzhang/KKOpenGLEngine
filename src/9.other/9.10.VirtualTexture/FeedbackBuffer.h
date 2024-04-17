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

class FeedbackBuffer
{
public:
	FeedbackBuffer(VirtualTextureInfo* _info, int _width, int _height);
	~FeedbackBuffer();
	void clear();
	void copy(unsigned short  viewId);
	void download();

	// This function validates the pages and adds the page's parents
	// We do this so that we can fall back to them if we run out of memory
	void addRequestAndParents(Page request);

	const std::vector<int>& getRequests() const;
	unsigned short  getFrameBuffer();

	int getWidth() const;
	int getHeight() const;

private:
	VirtualTextureInfo* m_info;
	PageIndexer* m_indexer;

	int m_width = 0;
	int m_height = 0;

	StagingPool	m_stagingPool;
	unsigned short 	m_lastStagingTexture;
	unsigned short  m_feedbackFrameBuffer;

	// This stores the pages by index.  The int value is number of requests.
	std::vector<int>		m_requests;
	std::vector<uint8_t>	m_downloadBuffer;
};
