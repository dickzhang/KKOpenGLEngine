#include "SkySun.h"
#include "Common.h"

ModuleBase* CreateAppModule()
{
	return new SkySun();
}

SkySun::SkySun()
{

}

SkySun::~SkySun()
{

}

void SkySun::Init()
{
	InitShader();
	GenerateMesh();
}

void SkySun::InitShader()
{
	m_Shader.LoadShader("9.7.skysun.vs","9.7.skysun.fs");
}

void SkySun::PreRender(Camera* camera)
{

}

void SkySun::Render(Camera* camera)
{
	float cameraNear = 0.1f;
	float cameraFar = 1000.0f;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f),(float)WindowSize::SCR_WIDTH/(float)WindowSize::SCR_HEIGHT,cameraNear,cameraFar);
	glm::mat4 view = camera->GetViewMatrix();
	m_Shader.use();
	m_Shader.setMat4("uMvpMatrix",projection*view*m_ModelMatrix);
	m_Shader.setMat4("uWorldMatrix",m_ModelMatrix);
	m_Shader.setVec3("uLightDirection",m_LightDirection);
	m_Shader.setVec3("uCamPosition",camera->Position);
	m_Shader.setVec3("ugroundColor",m_GroundColor);

	m_Shader.setFloat("uSkyIntensity",m_skyIntensity);
	m_Shader.setFloat("uSunSize",m_sunSize);

	m_Shader.setVec4("uAtmospheric",glm::vec4(m_atmosphericColor.x,m_atmosphericColor.y,m_atmosphericColor.z,m_atmosphericIntensity));
	m_Shader.setVec4("uSun",glm::vec4(m_sunColor.x,m_sunColor.y,m_sunColor.z,m_sunIntensity));
	m_Shader.setVec4("uGlare1",glm::vec4(m_glare1Color.x,m_glare1Color.y,m_glare1Color.z,m_glare1Intensity));
	m_Shader.setVec4("uGlare2",glm::vec4(m_glare2Color.x,m_glare2Color.y,m_glare2Color.z,m_glare2Intensity));

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES,m_Indexlist.size(),GL_UNSIGNED_INT,0);
}

void SkySun::Exit()
{
	m_Vertexlist.clear();
	m_Indexlist.clear();
}

unsigned int SkySun::LoadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1,&textureID);
	int width,height,nrComponents;
	unsigned char* data = stbi_load(path,&width,&height,&nrComponents,0);
	if(data)
	{
		GLenum format;
		if(nrComponents==1)
		{
			format = GL_RED;
		}
		else if(nrComponents==3)
		{
			format = GL_RGB;
		}
		else if(nrComponents==4)
		{
			format = GL_RGBA;
		}
		glBindTexture(GL_TEXTURE_2D,textureID);
		glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		stbi_image_free(data);
	}
	else
	{
		std::cout<<"Texture failed to load at path: "<<path<<std::endl;
		stbi_image_free(data);
	}
	return textureID;
}

void SkySun::GenerateMesh()
{
	float azimuth;
	int k;
	float PI = glm::pi<float>();
	float HALF_PI = glm::pi<float>()*0.5f;
	const float azimuth_step = (PI*2.f)/HorizontalResolution;
	if(SpherePercentage<0.f)
	{
		SpherePercentage = -SpherePercentage;
	}
	if(SpherePercentage>2.f)
	{
		SpherePercentage = 2.f;
	}
	const float elevation_step = SpherePercentage*HALF_PI/(float)VerticalResolution;
	m_Vertexlist.clear();
	m_Indexlist.clear();
	const float tcV = TexturePercentage/VerticalResolution;
	for(k = 0,azimuth = 0; k<=HorizontalResolution; ++k)
	{
		float elevation = HALF_PI;
		const float tcU = (float)k/(float)HorizontalResolution;
		const float sinA = sinf(azimuth);
		const float cosA = cosf(azimuth);
		for(int j = 0; j<=VerticalResolution; ++j)
		{
			Vertex vtx;
			vtx.Color = glm::vec4(255,255,255,255);
			vtx.Normal = glm::vec3(0.0f,-1.f,0.0f);
			const float cosEr = Radius*cosf(elevation);
			vtx.Pos = glm::vec3(cosEr*sinA,Radius*sinf(elevation),cosEr*cosA);
			vtx.TCoords = glm::vec2(tcU,j*tcV);
			vtx.Normal = -vtx.Pos;
			glm::normalize(vtx.Normal);
			m_Vertexlist.push_back(vtx);
			elevation -= elevation_step;
		}
		azimuth += azimuth_step;
	}

	for(k = 0; k<HorizontalResolution; ++k)
	{
		m_Indexlist.push_back(VerticalResolution+2+(VerticalResolution+1)*k);
		m_Indexlist.push_back(1+(VerticalResolution+1)*k);
		m_Indexlist.push_back(0+(VerticalResolution+1)*k);

		for(int j = 1; j<VerticalResolution; ++j)
		{
			m_Indexlist.push_back(VerticalResolution+2+(VerticalResolution+1)*k+j);
			m_Indexlist.push_back(1+(VerticalResolution+1)*k+j);
			m_Indexlist.push_back(0+(VerticalResolution+1)*k+j);

			m_Indexlist.push_back(VerticalResolution+1+(VerticalResolution+1)*k+j);
			m_Indexlist.push_back(VerticalResolution+2+(VerticalResolution+1)*k+j);
			m_Indexlist.push_back(0+(VerticalResolution+1)*k+j);
		}
	}
	CreateVAO();
}


void SkySun::CreateVAO()
{
	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
	glGenBuffers(1,&EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,m_Vertexlist.size()*sizeof(Vertex),m_Vertexlist.data(),GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_Indexlist.size()*sizeof(unsigned int),m_Indexlist.data(),GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,12*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,12*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,12*sizeof(float),(void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE,12*sizeof(float),(void*)(10*sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}


