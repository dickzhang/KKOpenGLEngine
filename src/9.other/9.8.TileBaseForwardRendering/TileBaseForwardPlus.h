#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader_t.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <iostream>
#include "ModuleBase.h"
#include "LightGenerator.h"
#include "PointLightBuffer.h"
#include "FBO.h"
#include "DepthTexture.h"

class TileBaseForwardPlus :public ModuleBase
{
public:
	struct Vertex
	{
		glm::vec3 Pos;
		glm::vec2 TCoords;
	};
public:
	TileBaseForwardPlus();

	virtual~TileBaseForwardPlus();
	virtual void Init()override;
	virtual void PreRender(Camera* camera)override;
	virtual void Render(Camera* camera,glm::vec2 mouseuv)override;
	virtual void Exit()override;
private:
	void InitShader();
	unsigned int LoadTexture(char const* path);
	void PreDepthPass();
	void LightCulling();
	void FinalShading();
private:

	Shader m_DepthShader;
	Shader m_LigthCullingShader;
	Shader m_ModelShader;
	Model m_Model;
	PointLightBuffer* m_PointLightBuffer;
	Camera* m_RenderCamera=nullptr;
	LightGenerator* m_LightGenerator=nullptr;
	glm::mat4 m_Projection = glm::mat4(1.0);
	glm::mat4 m_View = glm::mat4(1.0);
	float m_CameraNear = 0.1f;
	float m_CameraFar = 2000.0f;
	int TILE_SIZE = 16;
	int LIGHTS_PER_TILE =63;

	FBO framebuffer;
	DepthTexture depthTexture;
};
