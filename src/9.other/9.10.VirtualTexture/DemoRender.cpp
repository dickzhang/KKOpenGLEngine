#include "DemoRender.h"
#include "Common.h"

ModuleBase* CreateAppModule()
{
	return new DemoRender();
}

DemoRender::DemoRender()
{

}

DemoRender::~DemoRender()
{

}

void DemoRender::Init()
{
	InitShader();
	GenerateMesh();
}

void DemoRender::InitShader()
{
}

void DemoRender::PreRender(Camera* camera)
{

}

void DemoRender::Render(Camera* camera,glm::vec2 mouseuv)
{
	float cameraNear = 0.1f;
	float cameraFar = 1000.0f;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f),(float)WindowSize::SCR_WIDTH/(float)WindowSize::SCR_HEIGHT,cameraNear,cameraFar);
	glm::mat4 view = camera->GetViewMatrix();
}

void DemoRender::Exit()
{
}

unsigned int DemoRender::LoadTexture(char const* path)
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

void DemoRender::GenerateMesh()
{
	CreateVAO();
}

void DemoRender::CreateVAO()
{
	//glGenVertexArrays(1,&VAO);
	//glGenBuffers(1,&VBO);
	//glGenBuffers(1,&EBO);
	//glBindVertexArray(VAO);
	//glBindBuffer(GL_ARRAY_BUFFER,VBO);
	//glBufferData(GL_ARRAY_BUFFER,m_Vertexlist.size()*sizeof(Vertex),m_Vertexlist.data(),GL_STATIC_DRAW);
	//
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_Indexlist.size()*sizeof(unsigned int),m_Indexlist.data(),GL_STATIC_DRAW);
	//
	//glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(1);
	//glBindVertexArray(0);
}


