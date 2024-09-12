#include "ClusterForwardPlus.h"
#include "Common.h"

ModuleBase* CreateAppModule()
{
	return new ClusterForwardPlus();
}

ClusterForwardPlus::ClusterForwardPlus()
{

}

ClusterForwardPlus::~ClusterForwardPlus()
{
	if (m_LightGenerator)
	{
		delete m_LightGenerator;
		m_LightGenerator = nullptr;
	}
}

void ClusterForwardPlus::Init()
{
	m_LightGenerator = new LightGenerator();
	InitShader();
}

void ClusterForwardPlus::InitShader()
{
	m_ModelShader.loadShader("modelshader.vs", "modelshader.fs");
	m_GridBuildShader.loadShader("clusterShader.cs");
	m_LigthCullingShader.loadShader("clusterCullLightShader.cs");
	m_Model.loadModel(FileSystem::getPath("resources/objects/cryteksponza/sponza.obj"));
	InitSSBO();
}

void ClusterForwardPlus::InitSSBO()
{
	sizeX = (unsigned int)std::ceilf(WindowSize::SCR_WIDTH / (float)gridSizeX);

	//Buffer containing all the clusters
	{
		glGenBuffers(1, &AABBvolumeGridSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, AABBvolumeGridSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numClusters * sizeof(struct VolumeTileAABB), NULL, GL_STATIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, AABBvolumeGridSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	//Setting up lights buffer that contains all the lights in the scene
	{
		glGenBuffers(1, &lightSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, maxLights * sizeof(struct PointLight), NULL, GL_DYNAMIC_DRAW);
		GLint bufMask = GL_READ_WRITE;
		struct PointLight* lights = (struct PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, bufMask);

		auto lightsList = m_LightGenerator->GetLights();
		for (unsigned int i = 0; i < lightsList.size(); ++i)
		{
			lights[i].position = lightsList[i].position;
			lights[i].ambient = lightsList[i].ambient;
			lights[i].diffuse = lightsList[i].diffuse;
			lights[i].specular = lightsList[i].specular;
			lights[i].constant = lightsList[i].constant;
			lights[i].linear = lightsList[i].linear;
			lights[i].quadratic = lightsList[i].quadratic;
			lights[i].radius = lightsList[i].radius;
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	//A list of indices to the lights that are active and intersect with a cluster
	{
		unsigned int totalNumLights = numClusters * maxLightsPerTile * sizeof(unsigned int); //50 lights per tile max
		glGenBuffers(1, &lightIndexListSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexListSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, totalNumLights, NULL, GL_STATIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lightIndexListSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	//This implementation is straight up from Olsson paper
	{
		glGenBuffers(1, &lightGridSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightGridSSBO);

		auto len = numClusters * 2 * sizeof(unsigned int);
		glBufferData(GL_SHADER_STORAGE_BUFFER, len, NULL, GL_STATIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightGridSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	//Setting up simplest ssbo in the world
	{
		glGenBuffers(1, &lightIndexGlobalCountSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexGlobalCountSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int), NULL, GL_STATIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightIndexGlobalCountSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
}

void ClusterForwardPlus::PreRender(Camera* camera)
{
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glViewport(0, 0, WindowSize::SCR_WIDTH, WindowSize::SCR_HEIGHT);
	glClearColor(0.3, 0.3, 0.3, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if constexpr (Common::MSAA_ENABLED)
	{
		glEnable(GL_MULTISAMPLE);
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ClusterForwardPlus::Render(Camera* camera, glm::vec2 mouseuv)
{
	m_RenderCamera = camera;
	if (!m_RenderCamera)return;
	m_Projection = glm::perspective(glm::radians(60.0f), (float)WindowSize::SCR_WIDTH / (float)WindowSize::SCR_HEIGHT, m_CameraNear, m_CameraFar);
	m_View = m_RenderCamera->GetViewMatrix();
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, AABBvolumeGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexListSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightGridSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexGlobalCountSSBO);

	m_GridBuildShader.use();
	m_GridBuildShader.setInt("tileSizes", sizeX);
	m_GridBuildShader.setMat4("inverseProjection", glm::inverse(m_Projection));
	m_GridBuildShader.setFloat("zNear", m_CameraNear);
	m_GridBuildShader.setFloat("zFar", m_CameraFar);
	m_GridBuildShader.setVec2("screenDimensions", glm::vec2(WindowSize::SCR_WIDTH, WindowSize::SCR_HEIGHT));
	m_GridBuildShader.dispatch(gridSizeX, gridSizeY, gridSizeZ);

	m_LigthCullingShader.use();
	/*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lightIndexListSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightGridSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightIndexGlobalCountSSBO);*/
	m_LigthCullingShader.setMat4("viewMatrix", m_View);
	m_LigthCullingShader.dispatch(1, 1, 6);

	m_ModelShader.use();
	/*glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lightGridSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightIndexGlobalCountSSBO);*/
	m_ModelShader.setMat4("projection", m_Projection);
	m_ModelShader.setMat4("view", m_View);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
	m_ModelShader.setMat4("model", model);
	m_ModelShader.setFloat("zNear", m_CameraNear);
	m_ModelShader.setFloat("zFar", m_CameraFar);

	float scale = (float)gridSizeZ / std::log2f(m_CameraFar / m_CameraNear);
	float bias = -((float)gridSizeZ * std::log2f(m_CameraNear) / std::log2f(m_CameraFar / m_CameraNear));

	m_ModelShader.setFloat("scale", scale);
	m_ModelShader.setFloat("bias", bias);
	m_ModelShader.setVec4("tileSizes", glm::vec4(gridSizeX, gridSizeY, gridSizeZ, sizeX));
	m_ModelShader.setVec3("viewPos", m_RenderCamera->Position);
	m_Model.Draw(m_ModelShader);
}

//当窗口发生变化的时候,有些FBO就需要重新生成
void ClusterForwardPlus::WindowSizeChanged(int width, int height)
{
}

void ClusterForwardPlus::Exit()
{

}