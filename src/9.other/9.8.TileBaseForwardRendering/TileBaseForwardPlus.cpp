#include "TileBaseForwardPlus.h"
#include "Common.h"

ModuleBase* CreateAppModule()
{
	return new TileBaseForwardPlus();
}

TileBaseForwardPlus::TileBaseForwardPlus()
{

}

TileBaseForwardPlus::~TileBaseForwardPlus()
{
	if(m_LightGenerator)
	{
		delete m_LightGenerator;
		m_LightGenerator = nullptr;
	}
	if(m_PointLightBuffer)
	{
		delete m_PointLightBuffer;
		m_PointLightBuffer = nullptr;
	}
	if (m_DepthRenderTexture)
	{
		delete m_DepthRenderTexture;
		m_DepthRenderTexture = nullptr;
	}
}

void TileBaseForwardPlus::Init()
{
	m_DepthRenderTexture = new RenderTexture(WindowSize::SCR_WIDTH, WindowSize::SCR_HEIGHT,true);

	m_LightGenerator = new LightGenerator();
	int workGroupsX = WindowSize::SCR_WIDTH/TILE_SIZE;
	int workGroupsY = WindowSize::SCR_HEIGHT/TILE_SIZE;
	m_PointLightBuffer = new PointLightBuffer();
	m_PointLightBuffer->Init(m_LightGenerator->GetLights(),workGroupsX*workGroupsY,LIGHTS_PER_TILE);
	InitShader();
}

void TileBaseForwardPlus::InitShader()
{
	m_DepthShader.loadShader("depthMap.vs","depthMap.fs");
	m_ModelShader.loadShader("modelshader.vs","modelshader.fs");
	m_LigthCullingShader.loadShader("lightCulling.cs");
	m_Model.loadModel(FileSystem::getPath("resources/objects/cryteksponza/sponza.obj"));
}

void TileBaseForwardPlus::PreRender(Camera* camera)
{

}

void TileBaseForwardPlus::Render(Camera* camera,glm::vec2 mouseuv)
{
	m_RenderCamera = camera;
	if(!m_RenderCamera)return;
	m_Projection = glm::perspective(glm::radians(60.0f),(float)WindowSize::SCR_WIDTH/(float)WindowSize::SCR_HEIGHT,m_CameraNear,m_CameraFar);
	m_View =m_RenderCamera->GetViewMatrix();

	//此配置很重要,不然会出现,采样错误的问题,将深度限制在[0,1]之间
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	if constexpr (Common::MSAA_ENABLED)
	{
		glEnable(GL_MULTISAMPLE);
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	PreDepthPass();
	LightCulling();
	FinalShading();
}

void TileBaseForwardPlus::PreDepthPass()
{
	m_DepthRenderTexture->Bind();
	glm::mat4 m_ModelMatrix = glm::mat4(1.0);
	m_DepthShader.use();
	m_DepthShader.setMat4("modelViewProj",m_Projection*m_View*m_ModelMatrix);
	m_Model.Draw(m_DepthShader);
	m_DepthRenderTexture->Unbind();
}

void TileBaseForwardPlus::LightCulling()
{
	m_LigthCullingShader.use();
	m_LigthCullingShader.setSampler2D("depthMap", m_DepthRenderTexture->GetDepthTexture(),0);
	m_LigthCullingShader.setMat4("invViewProj",glm::inverse(m_Projection*m_View));
	m_LigthCullingShader.setInt("numLights",LightGenerator::NUM_OF_LIGHTS);
	m_LigthCullingShader.setInt("screenWidth",(int)WindowSize::SCR_WIDTH);
	m_LigthCullingShader.setInt("screenHeight",(int)WindowSize::SCR_HEIGHT);
	m_PointLightBuffer->Bind();
	unsigned int workGroupsX = static_cast<unsigned int>(ceil(float(WindowSize::SCR_WIDTH)/float(TILE_SIZE)));
	unsigned int workGroupsY = static_cast<unsigned int>(ceil(float(WindowSize::SCR_HEIGHT)/float(TILE_SIZE)));
	glDispatchCompute(workGroupsX,workGroupsY,1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void TileBaseForwardPlus::FinalShading()
{
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glViewport(0,0,WindowSize::SCR_WIDTH,WindowSize::SCR_HEIGHT);
	glClearColor(0.3,0.3,0.3,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	m_PointLightBuffer->Bind();
	m_ModelShader.use();
	m_ModelShader.setMat4("projection",m_Projection);
	m_ModelShader.setMat4("view",m_View);
	glm::mat4 model = glm::mat4(1.0f);
	m_ModelShader.setMat4("model",model);
	m_ModelShader.setVec3("viewPos",m_RenderCamera->Position);
	int workGroupsX = static_cast<unsigned int>(ceil(float(WindowSize::SCR_WIDTH)/float(TILE_SIZE)));
	m_ModelShader.setInt("numOfTilesX",workGroupsX);
	m_Model.Draw(m_ModelShader);
}

void TileBaseForwardPlus::Exit()
{
}
