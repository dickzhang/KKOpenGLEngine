#include "Window.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include "EngineUtils.h"

static float newWidth = 0.f;
static float newHeight = 0.f;

static void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    newWidth = width;
    newHeight = height;
}

Window* Window::window = nullptr;

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

Window& Window::Get()
{
    if (window == nullptr)
    {
        window = new Window();
        bool success = window->Init(EngineUtils::WINDOW_WIDTH, EngineUtils::WINDOW_HEIGHT);
        assert(success);
    }
    return *window;
}

Window::Window(/* args */)
: window_(nullptr)
, width_(0.f)
, height_(0.f)
{
    glfwSetErrorCallback(error_callback);
}

bool Window::Init(int windowWidth, int windowHeight)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if constexpr (EngineUtils::MSAA_ENABLED)
    {
        glfwWindowHint(GLFW_SAMPLES, EngineUtils::MSAA_SAMPLES);
    }

    window_ = glfwCreateWindow(windowWidth, windowHeight, "Renderer", NULL, NULL);
    if (window_ == nullptr)
    {
        std::cout << "Failed to create GLFW window\n";
        return false;
    }
    glfwMakeContextCurrent(window_);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout<<"Failed to initialize GLAD"<<std::endl;
        return -1;
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);

    width_ = newWidth = windowWidth;
    height_ = newHeight = windowHeight;

    if constexpr (!EngineUtils::DEBUG_MODE_ENABLED)
    {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    }
    
    return true;
}

inline bool Window::IsWindowResized() const
{
    return width_ != newWidth || height_ != newHeight;
}

bool Window::WindowShouldClose() const
{
    return glfwWindowShouldClose(window_);
}

void Window::SwapBuffers()
{
    glfwSwapBuffers(window_);
}

void Window::HandleResizeEvent()
{
    if (IsWindowResized())
    {
        height_ = newHeight;
        width_ = newWidth;
    }
}

Window::~Window()
{
}

