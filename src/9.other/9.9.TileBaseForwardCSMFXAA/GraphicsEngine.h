#pragma once

#include "Camera.h"
#include "ObjectDrawPass.h"
#include "CSMDepthPrepass.h"
#include "LightCullingDepthPrepass.h"
#include "ComputeShader.h"
#include "DebugLightsPass.h"
#include "LightCullingDepthMapDrawPass.h"
#include "GraphicsEngineSetupUtil.h"

class GraphicsEngine
{
public:

    GraphicsEngine();
	void Run();

private:

    void PerformLightCulling();
    void Rebuild();

	Camera camera_;
    GraphicsEngineSetupUtil graphicsUtils_;
    ObjectDrawPass drawPass_;
    CSMDepthPrepass csmDepthPrepass_;
    LightCullingDepthPrepass depthPrepass_;
    ComputeShader computeShader_;
    DebugLightsPass debugLights_;
    LightCullingDepthMapDrawPass drawDepthMap_;

};

