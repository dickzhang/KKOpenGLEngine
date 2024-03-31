#pragma once

#include <vector>

#include "DrawableObject.h"
#include "PointLight.h"
#include "ShaderProgram.h"

class DrawDebugLights : public DrawableObject
{
public:
    DrawDebugLights(const DrawDebugLights&);
    DrawDebugLights& operator=(const DrawDebugLights&);

    DrawDebugLights(const std::vector<PointLight>& pointLights);
    ~DrawDebugLights();

    void Draw(ShaderProgram& shader) override;

private:
    
    struct DebugLightVertex
    {
        DebugLightVertex(const glm::vec3& p) : position(p) {}
        glm::vec3 position;
    };
    
    void SetupDebugSpheres(
        const std::vector<PointLight>& pointLights,
        std::vector<DebugLightVertex>& vertices,
        std::vector<unsigned int>& indices);
    
    void SetupVertices(const PointLight& pointLights, std::vector<DebugLightVertex>& vertices);
};