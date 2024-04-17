#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/camera.h>
#include <iostream>
#include "ModuleBase.h"
#include "Common.h"

extern ModuleBase* CreateAppModule();

void framebuffer_size_callback(GLFWwindow* window,int width,int height);
void mouse_callback(GLFWwindow* window,double xpos,double ypos);
void scroll_callback(GLFWwindow* window,double xoffset,double yoffset);
void processInput(GLFWwindow* window);

unsigned int WindowSize::SCR_WIDTH = 1280;
unsigned int WindowSize::SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f,0.0f,0.0f));
float lastX = WindowSize::SCR_WIDTH/2.0;
float lastY = WindowSize::SCR_HEIGHT/2.0;
bool firstMouse = true;
float mouselastx = lastX;
float mouselasty = lastY;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec2 mouseUV = glm::vec2(0.5);
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_SAMPLES,4);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(WindowSize::SCR_WIDTH,WindowSize::SCR_HEIGHT,"LearnOpenGL",NULL,NULL);
	glfwMakeContextCurrent(window);
	if(window==NULL)
	{
		std::cout<<"Failed to create GLFW window"<<std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glfwSetCursorPosCallback(window,mouse_callback);
	glfwSetScrollCallback(window,scroll_callback);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout<<"Failed to initialize GLAD"<<std::endl;
		return -1;
	}
	auto appModule = CreateAppModule();
	appModule->Init();
	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame-lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		glViewport(0,0,WindowSize::SCR_WIDTH,WindowSize::SCR_HEIGHT);
		glClearColor(0.3,0.3,0.3,1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		appModule->PreRender(&camera);
		appModule->Render(&camera,mouseUV);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	appModule->Exit();
	delete appModule;
	appModule = nullptr;
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	camera.MovementSpeed = 50.;
	if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS)
		glfwSetWindowShouldClose(window,true);
	if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD,deltaTime);
	if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD,deltaTime);
	if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS)
		camera.ProcessKeyboard(LEFT,deltaTime);
	if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT,deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window,int width,int height)
{
	glViewport(0,0,width,height);
	WindowSize::SCR_WIDTH = width;
	WindowSize::SCR_HEIGHT = height;
}

void mouse_callback(GLFWwindow* window,double xposIn,double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if(glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)!=GLFW_PRESS)
	{
		lastX = xpos;
		lastY = ypos;
		return;
	}
	if(firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		mouselastx = lastX;
		mouselasty = lastY;
	}

	float xoffset = xpos-lastX;
	float yoffset = lastY-ypos; // reversed since y-coordinates go from bottom to top
	mouselastx += xoffset;
	mouselasty += yoffset;
	mouseUV = glm::vec2(mouselastx/WindowSize::SCR_WIDTH,mouselasty/WindowSize::SCR_WIDTH);

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset,yoffset);
}

void scroll_callback(GLFWwindow* window,double xoffset,double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

