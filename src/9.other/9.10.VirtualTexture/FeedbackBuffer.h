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

	//����������ҳ�沢���ҳ��ĸ�������������Ϊ�����ڴ�ľ�ʱ���Է��ص�����
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
	//���������洢ҳ�档intֵ�������������
	std::vector<int>		m_requests;
	std::vector<uint8_t>	m_downloadBuffer;
};
