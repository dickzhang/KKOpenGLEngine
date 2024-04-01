#include "CSMShadowMaps.h"

#include <glm/gtc/matrix_transform.hpp>

std::vector<glm::vec4> CSMShadowMaps::GetFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

glm::mat4 CSMShadowMaps::GetLightSpaceMatrix(
    const float nearPlane,
    const float farPlane,
    const Camera& camera,
    const glm::vec3& lightDir)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float width = viewport[2];
    float height = viewport[3];
    
    const auto proj = glm::perspective(
        glm::radians(camera.GetFOV()), width / height, nearPlane,
        farPlane);
    const auto corners = GetFrustumCornersWorldSpace(proj, camera.GetViewMatrix());

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = 0;
    float maxX = 0;
    float minY = 0;
    float maxY = 0;
    float minZ = 0;
    float maxZ = 0;
    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = min(minX, trf.x);
        maxX = max(maxX, trf.x);
        minY = min(minY, trf.y);
        maxY = max(maxY, trf.y);
        minZ = min(minZ, trf.z);
        maxZ = max(maxZ, trf.z);
    }
    // Tune this parameter according to the scene
    constexpr float zMult = 10.0f;
    if (minZ < 0)
    {
        minZ *= zMult;
    }
    else
    {
        minZ /= zMult;
    }
    if (maxZ < 0)
    {
        maxZ /= zMult;
    }
    else
    {
        maxZ *= zMult;
    }

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

    return lightProjection * lightView;
}

std::vector<glm::mat4> CSMShadowMaps::GetLightSpaceMatrices(
    const std::vector<float>& shadowCascadeLevels,
    const Camera& camera,
    const glm::vec3& lightDir)
{
    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(GetLightSpaceMatrix(Camera::NEAR_PLANE, shadowCascadeLevels[i], camera, lightDir));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(GetLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i], camera, lightDir));
        }
        else
        {
            ret.push_back(GetLightSpaceMatrix(shadowCascadeLevels[i - 1], Camera::FAR_PLANE, camera, lightDir));
        }
    }

    return ret;
}

void CSMShadowMaps::Init(
    const Camera& camera,
    const DirectionalLight& light,
    int shadowResolution,
    int depthMapTextureUnit
)
{
    camera_ = camera;
    light_ = light;
    shadowCascadeLevels_ = {
            Camera::FAR_PLANE / 50.0f,
            Camera::FAR_PLANE / 25.0f,
            Camera::FAR_PLANE / 10.0f,
            Camera::FAR_PLANE / 2.0f
    };
    depthMapTextures_.Init(shadowResolution, depthMapTextureUnit, shadowCascadeLevels_.size() + 1);
    lightTransformationMatricesGPU_.Init(nullptr, GL_UNIFORM_BUFFER, shadowCascadeLevels_.size() + 1, 0);
}

void CSMShadowMaps::GenerateShadows()
{
    const auto lightMatrices = GetLightSpaceMatrices(shadowCascadeLevels_, camera_, light_.direction);
    lightTransformationMatricesGPU_.Bind();
    
    for (size_t i = 0; i < lightMatrices.size(); ++i)
    {
        lightTransformationMatricesGPU_.InsertData((glm::mat4*)&lightMatrices[i], i * sizeof(glm::mat4));
    }

    lightTransformationMatricesGPU_.UnbindBuffer();
}

void CSMShadowMaps::BindShadowMapTexture()
{
    depthMapTextures_.Bind();
}

CSMShadowMaps::~CSMShadowMaps()
{
}
