#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include "ModuleBase.h"
#include "VirtualTexture.h"
#include "FeedbackBuffer.h"
#include "TileGenerator.h"
#include "TileDataFile.h"

class DemoRender :public ModuleBase
{
public:
	struct Vertex
	{
		glm::vec3 Pos;
		glm::vec2 TCoords;
	};
public:
	DemoRender();

	virtual~DemoRender();
	virtual void Init()override;
	virtual void PreRender(Camera* camera)override;
	virtual void Render(Camera* camera,glm::vec2 mouseuv)override;
	virtual void Exit()override;

private:
	void GenerateMesh();
	void CreateVAO();
	void DrawObject();
private:
	VirtualTexture* m_VirtualTexture = nullptr;
	FeedbackBuffer* m_FeedbackBuffer = nullptr;
	VirtualTextureInfo m_VirtualTextureInfo;
	TileGenerator* m_TileGenerator = nullptr;
	TileDataFile* m_TileDataFile = nullptr;
};
