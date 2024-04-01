#include "EngineUtils.h"
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

bool EngineUtils::UseLightCulling = true;
bool EngineUtils::DrawLightCullingDepthMap = false;
bool EngineUtils::DrawDebugLights = false;

bool EngineUtils::InitGLFW()
{
    return glfwInit() == GLFW_TRUE;
}

void EngineUtils::ReleaseGLFW()
{
    glfwTerminate();
}

void EngineUtils::SetupOpenGl()
{

    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    if constexpr (EngineUtils::MSAA_ENABLED)
    {
        glEnable(GL_MULTISAMPLE);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

std::string EngineUtils::ReadFile(std::string_view path) {
    constexpr auto read_size = std::size_t{4096};
    auto stream = std::ifstream{path.data()};
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string{};
    auto buf = std::string(read_size, '\0');
    while (stream.read(& buf[0], read_size)) {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

void EngineUtils::PrintFrameTime()
{
    static double lastTime = glfwGetTime();
    static unsigned int numOfFrames = 0;

    double currentTime = glfwGetTime();
    numOfFrames++;
    if (currentTime - lastTime > 1.f)
    {
        printf("%f ms/frame\n", 1000.0 / double(numOfFrames));
        numOfFrames = 0;
        lastTime += 1.0;
    }
}