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
	void InitShader();
	unsigned int LoadTexture(char const* path);
	void CreateVAO();
private:
};
