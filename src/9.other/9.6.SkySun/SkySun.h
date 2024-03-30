#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <iostream>
#include "ModuleBase.h"

class SkySun :public ModuleBase
{
public:
	struct Vertex
	{
		glm::vec3 Pos;
		glm::vec2 TCoords;
	};
public:
	SkySun();

	virtual~SkySun();
	virtual void Init()override;
	virtual void PreRender(Camera* camera)override;
	virtual void Render(Camera* camera)override;
	virtual void Exit()override;

private:
	void GenerateMesh();
	void InitShader();
	unsigned int LoadTexture(char const* path);
	void CreateVAO();
private:

	int HorizontalResolution = 32;
	int VerticalResolution = 8;
	float TexturePercentage = 1.0;
	float SpherePercentage = 2.0;
	float Radius = 500.0;
	unsigned int VAO,VBO,EBO = 0;
	std::vector<Vertex> m_Vertexlist;
	std::vector<unsigned int> m_Indexlist;
	Shader m_Shader;

	//Ìì¿ÕµÄÁÁ¶È
	float m_skyIntensity = 0.5;

	bool m_useDirectionLight = true;
	glm::vec4 m_sunColor = { 1.0,1.0,0.0,1.0 };
	float m_sunIntensity = 0.8;
	glm::vec4 m_atmosphericColor = { 1.0,0.5,0.6,1.0};
	float m_atmosphericIntensity = 0.01;
	float m_sunSize = 800.0f;
	glm::vec4 m_glare1Color = {0.7,0.1,0.2,1.0 };
	float m_glare1Intensity = 0.3;
	glm::vec4 m_glare2Color = { 0.5,0.5,0.5,1.0 };
	float m_glare2Intensity = 0.15;
	glm::vec3 m_GroundColor = { 0.6,0.6,0.6};
	bool m_changed = true;
	glm::vec3 m_LightDirection = { -0.5,0.5,1.0 };
};
