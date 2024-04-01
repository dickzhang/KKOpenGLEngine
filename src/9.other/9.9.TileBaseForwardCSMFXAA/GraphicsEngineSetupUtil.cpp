#include "GraphicsEngineSetupUtil.h"
#include "PCH.h"
#include <assert.h>
#include "EngineUtils.h"
#include "Window.h"

GraphicsEngineSetupUtil::GraphicsEngineSetupUtil(Camera& camera)
    : camera_(camera)
{
    light_.direction = glm::normalize(glm::vec3(20.0f, 500, -20.0f)); 
    light_.ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    light_.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    light_.specular = glm::vec3(0.5f, 0.5f, 0.5f);
    csmShadowMaps_.Init(camera_, light_, EngineUtils::CSM_SHADOW_RES, 2);

    sponzaModel_.Init(FileSystem::getPath("resources/objects/cryteksponza/"), "sponza.obj", true);
    
    drawFunc_ = [&](ShaderProgram& shader)
    {
        glm::mat4 model = glm::mat4(1.f);
        glm::mat4 view = camera_.GetViewMatrix();
        glm::mat4 proj = camera_.GetProjectionMatrix();

        sponzaModel_.SetView(view);
        sponzaModel_.SetProj(proj);
        sponzaModel_.SetModel(model);
        sponzaModel_.Draw(shader);
    };

    const auto& window = Window::Get();

    workGroupsX_ = window.GetWidth() / EngineUtils::TILE_SIZE;
    workGroupsY_ = window.GetHeight() / EngineUtils::TILE_SIZE;
    pointLightBuffer_.Init(lightGenerator_.GetLights(), workGroupsX_ * workGroupsY_, EngineUtils::LIGHTS_PER_TILE);
}

ObjectDrawPass GraphicsEngineSetupUtil::SetupMainPass()
{
    ShaderData vertexShaderData;
    vertexShaderData.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/objectDraw.vert"));
    ShaderData fragmentShaderData;
    fragmentShaderData.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/objectDraw.frag"));
    fragmentShaderData.defines = GetCSMDefines();
    fragmentShaderData.includes = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/shared.glsl"));
    
    if (EngineUtils::UseLightCulling)
    {
        auto lightCullingDefines = GetLightCullingDefines();
        fragmentShaderData.defines.insert(fragmentShaderData.defines.end(), lightCullingDefines.begin(), lightCullingDefines.end());
    }
    
    ShaderProgram shaderProgram;
    if (!shaderProgram.Init(&vertexShaderData, &fragmentShaderData))
    {
        EngineUtils::ReleaseGLFW();
        assert(false);
    }

    shaderProgram.UseProgram();
    shaderProgram.SetUniformValue("farPlane", Camera::FAR_PLANE);
    if (EngineUtils::UseLightCulling)
    {
        shaderProgram.SetUniformValue("lightsPerTile", EngineUtils::LIGHTS_PER_TILE);
        shaderProgram.SetUniformValue("numOfTilesX", workGroupsX_);
    }
    else
    {
        shaderProgram.SetUniformValue("numLights", (int)lightGenerator_.GetLights().size());
    }
    

    const auto& shadowCascadeLevels = csmShadowMaps_.GetShadowCascadeLevels();
    for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
    {
        shaderProgram.SetUniformValue(("cascadePlaneDistances[" + std::to_string(i) + "]").c_str(), shadowCascadeLevels[i]);
    }

    ObjectDrawPass drawPass(drawFunc_, shaderProgram, camera_, light_, lightGenerator_.GetLights(), csmShadowMaps_);
    if constexpr (!EngineUtils::MSAA_ENABLED)
    {
        drawPass.UseFXAA();
    }

	return drawPass;
}

CSMDepthPrepass GraphicsEngineSetupUtil::SetupCSMDepthPrepass()
{
    ShaderData depthMapVertexShader;
    depthMapVertexShader.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/layeredDepthMap.vert"));
    
    ShaderData depthMapFragmentShader;
    depthMapFragmentShader.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/layeredDepthMap.frag"));
    
    ShaderData depthMapGeometryShader;
    depthMapGeometryShader.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/layeredDepthMap.geom"));
    depthMapGeometryShader.defines = GetCSMDefines();
    
    ShaderProgram depthMapShader;
    if (!depthMapShader.Init(&depthMapVertexShader, &depthMapFragmentShader, &depthMapGeometryShader))
    {
        EngineUtils::ReleaseGLFW();
        assert(false);
    }

    return CSMDepthPrepass(drawFunc_, depthMapShader, csmShadowMaps_, EngineUtils::CSM_SHADOW_RES);
}

LightCullingDepthPrepass GraphicsEngineSetupUtil::SetupLightCullingDepthPrepass()
{
    ShaderData depthPrepassVertexShader;
    depthPrepassVertexShader.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/depthMap.vert"));

    ShaderData depthPrepassFragmentShader;
    depthPrepassFragmentShader.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/depthMap.frag"));

    ShaderProgram depthPrepassShader;
    depthPrepassShader.Init(&depthPrepassVertexShader, &depthPrepassFragmentShader);
    
    return LightCullingDepthPrepass(drawFunc_, depthPrepassShader);
}

ComputeShader GraphicsEngineSetupUtil::SetupLightCullingComputeShader(LightCullingDepthPrepass& depthPrepass)
{

    ShaderData computeShaderData;
    computeShaderData.sourceCode = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/lightCulling.comp"));
    computeShaderData.defines = GetLightCullingDefines();
    computeShaderData.includes = EngineUtils::ReadFile(FileSystem::getPath("resources/Shaders/shared.glsl"));

    const auto& window = Window::Get();

    ComputeShader computeShader(computeShaderData, workGroupsX_, workGroupsY_);
    computeShader.UseProgram();
    computeShader.SetUniformValue("invViewProj", glm::inverse(camera_.GetProjectionMatrix() * camera_.GetViewMatrix()));
    computeShader.SetUniformValue("numLights", (int)lightGenerator_.GetLights().size());
    computeShader.SetUniformValue("depthMap", (int)depthPrepass.GetDepthMap().GetTextureUnit());
    computeShader.SetUniformValue("screenWidth", window.GetWidth());
    computeShader.SetUniformValue("screenHeight", window.GetHeight());
    computeShader.SetUniformValue("lightsPerTile", EngineUtils::LIGHTS_PER_TILE);
    computeShader.SetStorageBuffer("LightBuffer", 1);
    computeShader.SetStorageBuffer("VisibleLightIndicesBuffer", 2);
    
    return computeShader;
}

DebugLightsPass GraphicsEngineSetupUtil::SetupDrawDebugLights()
{
    return DebugLightsPass(lightGenerator_.GetLights(), camera_);
}

LightCullingDepthMapDrawPass GraphicsEngineSetupUtil::SetupLightCullingDepthMapDrawPass(LightCullingDepthPrepass& depthPrepass)
{
    return LightCullingDepthMapDrawPass(depthPrepass.GetDepthMap());
}

std::vector<ShaderDefine> GraphicsEngineSetupUtil::GetCSMDefines() const
{
    std::vector<ShaderDefine> defines;
    defines.push_back(ShaderDefine("NUM_CSM_PLANES", std::to_string(csmShadowMaps_.GetNumCSMPlanes())));
    return defines;
}

std::vector<ShaderDefine> GraphicsEngineSetupUtil::GetLightCullingDefines() const
{
    std::vector<ShaderDefine> defines;
    defines.push_back(ShaderDefine("LIGHTS_PER_TILE", std::to_string(EngineUtils::LIGHTS_PER_TILE)));
    defines.push_back(ShaderDefine("TILE_SIZE", std::to_string(EngineUtils::TILE_SIZE)));
    defines.push_back(ShaderDefine("LIGHT_ID_END", std::to_string(EngineUtils::LIGHT_ID_END)));
    defines.push_back(ShaderDefine("USE_LIGHT_CULLING", ""));
    return defines;
}

