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
#include "RenderTexture.h"

class ClusterForwardPlus :public ModuleBase
{
public:
	struct Vertex
	{
		glm::vec3 Pos;
		glm::vec2 TCoords;
	};

	struct VolumeTileAABB
	{
		glm::vec4 minPoint;
		glm::vec4 maxPoint;
	};

	struct GPULight
	{
		glm::vec4 position;
		glm::vec4 ambient;
		glm::vec4 diffuse;
		glm::vec4 specular;
		float constant;
		float linear;
		float quadratic;
		float radius;
	};

public:
	ClusterForwardPlus();

	virtual~ClusterForwardPlus();
	virtual void Init()override;
	virtual void PreRender(Camera* camera)override;
	virtual void Render(Camera* camera, glm::vec2 mouseuv)override;
	virtual void Exit()override;
private:
	void InitShader();
	void InitSSBO();
	unsigned int LoadTexture(char const* path);
	void WindowSizeChanged(int width, int height);
private:

	Shader m_LigthCullingShader;
	Shader m_GridBuildShader;
	Shader m_ModelShader;
	Model m_Model;
	Camera* m_RenderCamera = nullptr;
	LightGenerator* m_LightGenerator = nullptr;
	glm::mat4 m_Projection = glm::mat4(1.0);
	glm::mat4 m_View = glm::mat4(1.0);
	float m_CameraNear = 0.01f;
	float m_CameraFar = 1000.0f;

	const unsigned int gridSizeX = 16;
	const unsigned int gridSizeY = 9;
	const unsigned int gridSizeZ = 24;
	const unsigned int numClusters = gridSizeX * gridSizeY * gridSizeZ;
	unsigned int sizeX, sizeY;
	const unsigned int maxLights = 500;
	const unsigned int maxLightsPerTile = 50;

	unsigned int AABBvolumeGridSSBO;
	unsigned int lightSSBO, lightIndexListSSBO, lightGridSSBO, lightIndexGlobalCountSSBO;
};
