#pragma once
#include "9.5.buffers.h"
#include "CloudsModel.h"
#include "TextureSet.h"
#include <learnopengl/camera.h>

class VolumetricClouds
{
public:
	enum cloudsTextureNames
	{
		fragColor, bloom, alphaness, cloudDistance
	};
	VolumetricClouds(int SW, int SH, CloudsModel * model);
	~VolumetricClouds();
	void draw(Camera * cam, glm::mat4 projMatrix, unsigned int sceneDepthTex);

	unsigned int getCloudsTexture() {
		return ( model->postProcess ? cloudsPostProcessingFBO->getColorAttachmentTex(0) : getCloudsRawTexture() );
	}

	unsigned int getCloudsTexture(int i) {
		return cloudsFBO->getColorAttachmentTex(i);
	}

	unsigned int getCloudsRawTexture(){
		return cloudsFBO->getColorAttachmentTex(0);
	}
	void initializeQuad();
private:
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	int cloudswidth, cloudsheith;
	glm::vec3 lightColor = glm::vec3(1.0, 1.0, 230 / 255.0);
	glm::vec3 lightPos = glm::vec3(0.0);
	glm::vec3 lightDir = glm::vec3(-.5, 0.5, 1.0);

	bool enablePowder = false;
	TextureSet * cloudsFBO;
	FrameBufferObject * cloudsPostProcessingFBO;
	CloudsModel * model;
	bool initialized = false;
};

