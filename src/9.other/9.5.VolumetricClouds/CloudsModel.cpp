#include "CloudsModel.h"
#include "9.5.TextureUtil.h"
#include <glad/glad.h>
#define INT_CEIL(n,d) (int)ceil((float)n/d)

CloudsModel::CloudsModel()
{
	initShaders();
	generateModelTextures();
}

CloudsModel::~CloudsModel()
{
}

void CloudsModel::generateWeatherMap() {
	bindTexture2D(weatherTex, 0);
	weatherShader.use();
	weatherShader.setVec3("seed", seed);
	weatherShader.setFloat("perlinFrequency", perlinFrequency);
	glDispatchCompute(INT_CEIL(1024, 8), INT_CEIL(1024, 8), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void CloudsModel::generateModelTextures()
{
	/////////////////// TEXTURE GENERATION //////////////////
	if(!perlinTex)
	{
		perlinTex = generateTexture3D(128, 128, 128);
		perlinworleyShader.use();
		perlinworleyShader.setVec3("u_resolution", glm::vec3(128, 128, 128));
		glActiveTexture(GL_TEXTURE0);
		perlinworleyShader.setInt("outVolTex", 0);
		glBindTexture(GL_TEXTURE_3D, perlinTex);
		glBindImageTexture(0, perlinTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		glDispatchCompute(INT_CEIL(128, 4), INT_CEIL(128, 4), INT_CEIL(128, 4));
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glGenerateMipmap(GL_TEXTURE_3D);
	}
	if(!worley32)
	{
		worley32 = generateTexture3D(32, 32, 32);
		worleyShader.use();
		//worleyShader.setVec3("u_resolution", glm::vec3(32, 32, 32));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, worley32);
		glBindImageTexture(0, worley32, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		glDispatchCompute(INT_CEIL(32, 4), INT_CEIL(32, 4), INT_CEIL(32, 4));
		glGenerateMipmap(GL_TEXTURE_3D);
	}
	////////////////////////

	if(!weatherTex)
	{
		weatherTex = generateTexture2D(1024, 1024);
		generateWeatherMap();
		oldSeed = seed;
	}
}

void CloudsModel::initShaders()
{
	postProcessingShader.loadShader("9.5.screen.vs", "9.5.clouds_post.fs");
	volumetricCloudsShader.loadShader("9.5.volumetric_clouds.cs");
	weatherShader.loadShader("9.5.weather.cs");
	perlinworleyShader.loadShader("9.5.perlinworley.cs");
	worleyShader.loadShader("9.5.worley.cs");
	
}
void CloudsModel::update()
{
	if(seed != oldSeed) {
		generateWeatherMap();
		oldSeed = seed;
	}
}
unsigned int CloudsModel::generateTexture2D(int w, int h)
{
	unsigned int tex_output;
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	return tex_output;
}
unsigned int CloudsModel::generateTexture3D(int w, int h, int d)
{
	unsigned int tex_output;
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex_output);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexStorage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, w, h, d);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, w, h, d, 0, GL_RGBA, GL_FLOAT, NULL);
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	return tex_output;
}

void CloudsModel::bindTexture2D(unsigned int tex, int unit)
{
	glBindImageTexture(unit, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}