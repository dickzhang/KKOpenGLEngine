#include "Camera.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.h"
#include "EngineUtils.h"

static float lastX = 0.f;
static float lastY = 0.f;
static float pitch = 0.f;
static float yaw = 0.f;
static bool firstMouse = true;
static bool frontChanged = true;
static float fov = 45.f;
static bool fovChanged = true;

static void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    frontChanged = true;
}

static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
    
    fovChanged = true;
}

Camera::Camera()
: position_(0.f)
, front_(0.f)
, up_(0.f)
, worldUp_(0.f)
, fov_(45.f)
, right_(0.f)
{
}

Camera::Camera(glm::vec3 position)
{
    Init(position);
}

void Camera::Init(glm::vec3 position)
{
    position_ = position;
    front_ = glm::vec3(0.0f, 0.0f, -1.0f);
    worldUp_ = glm::vec3(0.0f, 1.0f,  0.0f);

    const auto window = Window::Get();
    glfwSetCursorPosCallback(window.GetGLFWwindow(), mouseCallback);
    glfwSetScrollCallback(window.GetGLFWwindow(), scrollCallback);
    ProcessInputForMovement(0);
}

void Camera::ProcessInputForMovement(float deltaTime)
{
    float cameraSpeed = EngineUtils::MOVE_SPEED * deltaTime;

    if (frontChanged)
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front_ = glm::normalize(direction);
        right_ = glm::normalize(glm::cross(front_, worldUp_));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        up_ = glm::normalize(glm::cross(right_, front_));
        frontChanged = false;
    }

    if (fovChanged)
    {
        fov_ = fov;
        fovChanged = false;
    }

    const auto window = Window::Get();
    if(glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window.GetGLFWwindow(), true);
    }
    if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_W) == GLFW_PRESS)
    {
        position_ += cameraSpeed * front_;
    }
    if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_S) == GLFW_PRESS)
    {
        position_ -= cameraSpeed * front_;
    }
    if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_A) == GLFW_PRESS)
    {
        position_ -= right_ * cameraSpeed;
    }
    if (glfwGetKey(window.GetGLFWwindow(), GLFW_KEY_D) == GLFW_PRESS)
    {
        position_ += right_ * cameraSpeed;
    }
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    const auto window = Window::Get();
    return glm::perspective(glm::radians(fov_), window.GetWidth() / window.GetHeight(), NEAR_PLANE, FAR_PLANE);
}

glm::vec3 Camera::GetPosition() const
{
    return position_;
}
