#include "DemoRender.h"
#include "Common.h"
#include "learnopengl/filesystem.h"

ModuleBase* CreateAppModule()
{
	return new DemoRender();
}

DemoRender::DemoRender()
{
	m_VirtualTextureInfo.m_virtualTextureSize = 8192;
	m_VirtualTextureInfo.m_tileSize = 128;
	m_VirtualTextureInfo.m_borderSize = 1;

	m_TileGenerator = new TileGenerator(m_VirtualTextureInfo);
	m_TileGenerator->generate(FileSystem::getPath("resources/textures/8k_mars.jpg"));

	m_TileDataFile = new TileDataFile(FileSystem::getPath("resources/textures/8k_mars.jpg.vt"),m_VirtualTextureInfo);
	m_TileDataFile->readInfo();

	m_VirtualTexture = new VirtualTexture(m_TileDataFile,m_VirtualTextureInfo,2048,1);
	m_FeedbackBuffer = new FeedbackBuffer(m_VirtualTextureInfo,64,64);
}

DemoRender::~DemoRender()
{
	if(m_TileGenerator)
	{
		delete m_TileGenerator;
		m_TileGenerator = nullptr;
	}
	if(m_TileDataFile)
	{
		delete m_TileDataFile;
		m_TileDataFile = nullptr;
	}
	if(m_VirtualTexture)
	{
		delete m_VirtualTexture;
		m_VirtualTexture = nullptr;
	}
	if(m_FeedbackBuffer)
	{
		delete m_FeedbackBuffer;
		m_FeedbackBuffer = nullptr;
	}
}

void DemoRender::Init()
{
	m_VirtualTexture->LoadShader();
	GenerateMesh();
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
	glm::mat4 vp = projection*view;
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glViewport(0,0,WindowSize::SCR_WIDTH,WindowSize::SCR_HEIGHT);
	glClearColor(0.3,0.3,0.3,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	auto feedBackFBO = m_FeedbackBuffer->GetFrameBuffer();
	feedBackFBO->Bind();
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.3,0.3,0.3,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glViewport(0,0,m_FeedbackBuffer->getWidth(),m_FeedbackBuffer->getHeight());
	m_VirtualTexture->setMipUniforms(vp);
	renderPlane();
	m_FeedbackBuffer->download();
	m_VirtualTexture->update(m_FeedbackBuffer->getRequests(),4);
	m_FeedbackBuffer->clear();
	m_FeedbackBuffer->copy(3);
	
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glViewport(0,0,WindowSize::SCR_WIDTH,WindowSize::SCR_HEIGHT);
	glClearColor(0.3,0.3,0.3,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	m_VirtualTexture->setVTUniforms(vp);
	renderPlane();
}

void DemoRender::renderPlane()
{
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES,0,6);
	glBindVertexArray(0);
}

void DemoRender::Exit()
{
}

void DemoRender::GenerateMesh()
{
	CreateVAO();
}

void DemoRender::CreateVAO()
{
	struct VertexAL
	{
		glm::vec3 position;
		glm::vec2 texcoord;
	};
	const GLfloat psize = 10.0f;
	VertexAL planeVertices[6] = {
		{ { -psize,0.0f,-psize },{ 0.0f,0.0f } },
		{ { -psize,0.0f,psize },{ 0.0f,1.0f } },
		{ { psize,0.0f,psize },{ 1.0f,1.0f } },
		{ { -psize,0.0f,-psize },{ 0.0f,0.0f } },
		{ { psize,0.0f,psize },{ 1.0f,1.0f } },
		{ { psize,0.0f,-psize },{ 1.0f,0.0f } }
	};
	// PLANE
	glGenVertexArrays(1,&planeVAO);
	glGenBuffers(1,&planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER,planeVBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(planeVertices),planeVertices,GL_STATIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(GLfloat),(GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}


