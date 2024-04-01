#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "StorageBuffer.h"
#include "TextureArray.h"
#include "Camera.h"
#include "DirectionalLight.h"


//Abstraction for cascaded shadow mapping

class CSMShadowMaps
{
public:

    CSMShadowMaps() = default;

    void Init(
        const Camera& camera,
        const DirectionalLight& light,
        int shadowResolution,
        int depthMapTextureUnit
    );

    void GenerateShadows();
    void BindShadowMapTexture();
    unsigned int GetLightTransformationId() const { return lightTransformationMatricesGPU_.GetId(); }
    unsigned int GetShadowMapTextureId() const { return depthMapTextures_.GetId(); }
    const std::vector<float>& GetShadowCascadeLevels() const { return shadowCascadeLevels_; }
    int GetNumCSMPlanes() const { return shadowCascadeLevels_.size() + 1; }
    std::vector<glm::mat4> GetMatrices() const { return  GetLightSpaceMatrices(shadowCascadeLevels_, camera_, light_.direction); }
    ~CSMShadowMaps();

private:
    static std::vector<glm::mat4> GetLightSpaceMatrices(
        const std::vector<float>& shadowCascadeLevels,
        const Camera& camera,
        const glm::vec3& lightDir);

    static glm::mat4 GetLightSpaceMatrix(
        const float nearPlane,
        const float farPlane,
        const Camera& camera,
        const glm::vec3& lightDir);
    
    static std::vector<glm::vec4> GetFrustumCornersWorldSpace(
        const glm::mat4& proj,
        const glm::mat4& view);

    Camera camera_;
    DirectionalLight light_;

    std::vector<float> shadowCascadeLevels_;
    StorageBuffer<glm::mat4> lightTransformationMatricesGPU_;
    TextureArray depthMapTextures_;
};