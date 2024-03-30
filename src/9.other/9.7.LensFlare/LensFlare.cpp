#include "LensFlare.h"
#include "Common.h"

ModuleBase* CreateAppModule()
{
	return new LensFlare();
}

LensFlare::LensFlare()
{

}

LensFlare::~LensFlare()
{

}

void LensFlare::Init()
{
	InitShader();
	GenerateMesh();
}

void LensFlare::InitShader()
{
	m_Shader.LoadShader("9.7.lensflare.vs","9.7.lensflare.fs");
}

void LensFlare::PreRender(Camera* camera)
{

}

void LensFlare::Render(Camera* camera,glm::vec2 mouseuv)
{
	float cameraNear = 0.1f;
	float cameraFar = 1000.0f;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f),(float)WindowSize::SCR_WIDTH/(float)WindowSize::SCR_HEIGHT,cameraNear,cameraFar);
	glm::mat4 view = camera->GetViewMatrix();
	glm::mat4 m_ModelMatrix = glm::mat4(1.0);
	m_Shader.use();
	m_Shader.setVec2("iMouse",mouseuv);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES,m_Indexlist.size(),GL_UNSIGNED_INT,0);
}

void LensFlare::Exit()
{
	m_Vertexlist.clear();
	m_Indexlist.clear();
}

unsigned int LensFlare::LoadTexture(char const* path)
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

void LensFlare::GenerateMesh()
{

	Vertex vtx0;
	vtx0.Pos = glm::vec3(-1,-1,0);
	vtx0.TCoords = glm::vec2(0,0);
	m_Vertexlist.push_back(vtx0);

	Vertex vtx1;
	vtx1.Pos = glm::vec3(-1,1,0);
	vtx1.TCoords = glm::vec2(0,1);
	m_Vertexlist.push_back(vtx1);

	Vertex vtx2;
	vtx2.Pos = glm::vec3(1,1,0);
	vtx2.TCoords = glm::vec2(1,1);
	m_Vertexlist.push_back(vtx2);

	Vertex vtx3;
	vtx3.Pos = glm::vec3(1,-1,0);
	vtx3.TCoords = glm::vec2(1,0);
	m_Vertexlist.push_back(vtx3);

	m_Indexlist.push_back(0);
	m_Indexlist.push_back(1);
	m_Indexlist.push_back(2);
	m_Indexlist.push_back(2);
	m_Indexlist.push_back(3);
	m_Indexlist.push_back(0);

	CreateVAO();
}


void LensFlare::CreateVAO()
{
	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
	glGenBuffers(1,&EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,m_Vertexlist.size()*sizeof(Vertex),m_Vertexlist.data(),GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_Indexlist.size()*sizeof(unsigned int),m_Indexlist.data(),GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}


