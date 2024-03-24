#include "VolumetricClouds.h"
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Common.h"

#define INT_CEIL(n,d) (int)ceil((float)n/d)

VolumetricClouds::VolumetricClouds(int SW, int SH, CloudsModel * model)
{
	this->model = model;
	cloudswidth = SW;
	cloudsheith = SH;
	cloudsFBO = new TextureSet(SW, SH, 4);
	cloudsPostProcessingFBO = new FrameBufferObject(SCR_WIDTH, SCR_HEIGHT, 2);
	initializeQuad();
}

void VolumetricClouds::initializeQuad() 
{
	if(!initialized) {
		float vertices[] = {
			-1.0f, -1.0f, 0.0, 0.0,
			1.0f, -1.0f, 1.0, 0.0,
			-1.0f,  1.0f, 0.0, 1.0,
			1.0f,  1.0f, 1.0, 1.0,
			-1.0f,  1.0f, 0.0, 1.0,
			1.0f, -1.0f, 1.0, 0.0
		};
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)( 2 * sizeof(float) ));
		glEnableVertexAttribArray(1);
	}
}

VolumetricClouds::~VolumetricClouds()
{

}

void VolumetricClouds::draw(Camera * cam, glm::mat4 projMatrix, unsigned int sceneDepthTex)
{
	lightDir = glm::normalize(lightDir);
	lightPos = lightDir * 1e6f + cam->Position;

	float t1, t2;
	for(int i = 0; i < cloudsFBO->getNTextures(); ++i)
	{
		model->bindTexture2D(cloudsFBO->getColorAttachmentTex(i), i);
	}
	Shader cloudsShader = model->volumetricCloudsShader;
	cloudsShader.use();
	cloudsShader.setVec2("iResolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
	cloudsShader.setFloat("iTime", glfwGetTime());
	cloudsShader.setMat4("inv_proj", glm::inverse(projMatrix));
	cloudsShader.setMat4("inv_view", glm::inverse(cam->GetViewMatrix()));
	cloudsShader.setVec3("cameraPosition", cam->Position);
	cloudsShader.setFloat("FOV", cam->Zoom);
	cloudsShader.setVec3("lightDirection", glm::normalize(lightPos - cam->Position));
	cloudsShader.setVec3("lightColor", lightColor);

	cloudsShader.setFloat("coverage_multiplier", model->coverage);
	cloudsShader.setFloat("cloudSpeed", model->cloudSpeed);
	cloudsShader.setFloat("crispiness", model->crispiness);
	cloudsShader.setFloat("curliness", model->curliness);
	cloudsShader.setFloat("absorption", model->absorption * 0.01);
	cloudsShader.setFloat("densityFactor", model->density);

	cloudsShader.setBool("enablePowder", enablePowder);

	cloudsShader.setFloat("earthRadius", model->earthRadius);
	cloudsShader.setFloat("sphereInnerRadius", model->sphereInnerRadius);
	cloudsShader.setFloat("sphereOuterRadius", model->sphereOuterRadius);

	cloudsShader.setVec3("cloudColorTop", model->cloudColorTop);
	cloudsShader.setVec3("cloudColorBottom", model->cloudColorBottom);

	glm::vec3 skyColorTop = glm::vec3(0.5, 0.7, 0.8) * 1.05f;
	glm::vec3 skyColorBottom = glm::vec3(0.9, 0.9, 0.95);
	cloudsShader.setVec3("skyColorTop", skyColorTop);
	cloudsShader.setVec3("skyColorBottom", skyColorBottom);

	//TODO和天空盒融合的时候有用
	/*cloudsShader.setVec3("skyColorTop", model->sky->skyColorTop);
	cloudsShader.setVec3("skyColorBottom", model->sky->skyColorBottom);*/

	glm::mat4 vp = projMatrix * cam->GetViewMatrix();
	cloudsShader.setMat4("invViewProj", glm::inverse(vp));
	cloudsShader.setMat4("gVP", vp);

	cloudsShader.setSampler3D("cloud", model->perlinTex, 0);
	cloudsShader.setSampler3D("worley32", model->worley32, 1);
	cloudsShader.setSampler2D("weatherTex", model->weatherTex, 2);
	cloudsShader.setSampler2D("depthMap", sceneDepthTex, 3);

	//TODO和天空盒融合的时候有用
	//cloudsShader.setSampler2D("sky", model->sky->getSkyTexture(), 4);

	//actual draw
	if(!WIRE_FRAME)
	{
		glDispatchCompute(INT_CEIL(SCR_WIDTH, 16), INT_CEIL(SCR_HEIGHT, 16), 1);
	}
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//copy to lastFrameFBO
	if(model->postProcess)
	{
		// cloud post processing filtering
		cloudsPostProcessingFBO->bind();
		auto cloudsPPShader = model->postProcessingShader;
		cloudsPPShader.use();
		cloudsPPShader.setSampler2D("clouds", cloudsFBO->getColorAttachmentTex(VolumetricClouds::fragColor), 0);
		cloudsPPShader.setSampler2D("emissions", cloudsFBO->getColorAttachmentTex(VolumetricClouds::bloom), 1);
		cloudsPPShader.setSampler2D("depthMap", sceneDepthTex, 2);

		cloudsPPShader.setVec2("cloudRenderResolution", glm::vec2(cloudswidth, cloudsheith));
		cloudsPPShader.setVec2("resolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));

		glm::mat4 lightModel;
		lightModel = glm::translate(lightModel,lightPos);
		glm::vec4 pos = vp * lightModel * glm::vec4(0.0, 60.0, 0.0, 1.0);
		pos = pos / pos.w;
		pos = pos * 0.5f + 0.5f;
		cloudsPPShader.setVec4("lightPos", pos);

		bool isLightInFront = false;
		float lightDotCameraFront = glm::dot(glm::normalize(lightPos -cam->Position), glm::normalize(cam->Front));
		if(lightDotCameraFront > 0.2)
		{
			isLightInFront = true;
		}
		cloudsPPShader.setBool("isLightInFront", isLightInFront);
		cloudsPPShader.setBool("enableGodRays", model->enableGodRays);
		cloudsPPShader.setFloat("lightDotCameraFront", lightDotCameraFront);
		cloudsPPShader.setFloat("time", glfwGetTime());
		if(!WIRE_FRAME)
		{
			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}
}