#pragma once
#include <learnopengl/shader_t.h>

class CloudsModel
{
public:
	CloudsModel();
	~CloudsModel();
	void generateWeatherMap();
	void generateModelTextures();
	void initShaders();
	void update();
	unsigned int generateTexture2D(int w, int h);
	unsigned int generateTexture3D(int w, int h, int d);
	void bindTexture2D(unsigned int tex, int unit);

public:
	float cloudSpeed = 450.0;
	float coverage = 0.45;
	float crispiness = 40.;
	float curliness = .1;
	float density = 0.02;
	float absorption = 0.35;

	float earthRadius = 600000.0;
	float sphereInnerRadius = 5000.0;
	float sphereOuterRadius = 17000.0;

	//‘Î…˘∆µ¬ 
	float perlinFrequency = 0.8;
	bool enableGodRays = false;
	bool postProcess = true;

	glm::vec3 seed = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 oldSeed = glm::vec3(0.0, 0.0, 0.0);

	glm::vec3 cloudColorTop = ( glm::vec3(169., 149., 149.) * ( 1.5f / 255.f ) );
	glm::vec3 cloudColorBottom = ( glm::vec3(65., 70., 80.) * ( 1.5f / 255.f ) );

	unsigned int weatherTex = 0;
	unsigned int perlinTex = 0;
	unsigned int worley32 = 0;

	Shader postProcessingShader;
	Shader volumetricCloudsShader;
	Shader weatherShader;
	Shader perlinworleyShader;
	Shader worleyShader;
};

