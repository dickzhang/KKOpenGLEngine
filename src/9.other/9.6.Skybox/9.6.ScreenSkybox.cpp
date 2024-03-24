#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <iostream>
#include "9.6.buffers.h"

struct colorPreset
{
	glm::vec3 cloudColorBottom, skyColorTop, skyColorBottom, lightColor, fogColor;
};

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
void processInput(GLFWwindow * window);
unsigned int loadTexture(const char * path);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::vec3 skyColorTop = glm::vec3(0.5, 0.7, 0.8) * 1.05f;
glm::vec3 skyColorBottom = glm::vec3(0.9, 0.9, 0.95);

FrameBufferObject * skyboxFBO;
colorPreset presetSunset, highSunPreset;
glm::vec3 fogColor(0.5, 0.6, 0.7);
glm::vec3 lightColor(1.0, 1.0, 230.0 / 255.0);
glm::vec3 lightDir = glm::vec3(-.5, 0.5, 1.0);
glm::vec3 lightPos = glm::vec3(0.0);

colorPreset SunsetPreset() {
	colorPreset preset;
	preset.cloudColorBottom = glm::vec3(89, 96, 109) / 255.f;
	preset.skyColorTop = glm::vec3(177, 174, 119) / 255.f;
	preset.skyColorBottom = glm::vec3(234, 125, 125) / 255.f;
	preset.lightColor = glm::vec3(255, 171, 125) / 255.f;
	preset.fogColor = glm::vec3(85, 97, 120) / 255.f;
	presetSunset = preset;
	return preset;
}

colorPreset DefaultPreset() {
	colorPreset preset;
	preset.cloudColorBottom = ( glm::vec3(65., 70., 80.) * ( 1.5f / 255.f ) );
	preset.skyColorTop = glm::vec3(0.5, 0.7, 0.8) * 1.05f;
	preset.skyColorBottom = glm::vec3(0.9, 0.9, 0.95);
	preset.lightColor = glm::vec3(255, 255, 230) / 255.f;
	preset.fogColor = glm::vec3(0.5, 0.6, 0.7);
	highSunPreset = preset;
	return preset;
}

colorPreset SunsetPreset1() {
	colorPreset preset;
	preset.cloudColorBottom = glm::vec3(97, 98, 120) / 255.f;
	preset.skyColorTop = glm::vec3(133, 158, 214) / 255.f;
	preset.skyColorBottom = glm::vec3(241, 161, 161) / 255.f;
	preset.lightColor = glm::vec3(255, 201, 201) / 255.f;
	preset.fogColor = glm::vec3(128, 153, 179) / 255.f;
	presetSunset = preset;
	return preset;
}
void mixSkyColorPreset(float v, colorPreset p1, colorPreset p2) {
	float a = std::min(std::max(v, 0.0f), 1.0f);
	float b = 1.0 - a;
	//cloudColorBottom = p1.cloudColorBottom*a + p2.cloudColorBottom*b;
	skyColorTop = p1.skyColorTop * a + p2.skyColorTop * b;
	skyColorBottom = p1.skyColorBottom * a + p2.skyColorBottom * b;
	lightColor = p1.lightColor * a + p2.lightColor * b;
	fogColor = p1.fogColor * a + p2.fogColor * b;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	glfwMakeContextCurrent(window);
	if(window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	//skyboxFBO = new FrameBufferObject(SCR_WIDTH, SCR_HEIGHT);
	SunsetPreset1();
	DefaultPreset();

	Shader skyboxShader("9.6.skybox.vs", "9.6.skybox.fs");
	glEnableVertexAttribArray(0);
	unsigned int cubeTexture = loadTexture(FileSystem::getPath("resources/textures/marble.jpg").c_str());

	float vertices[] = {
			-1.0f, -1.0f, 0.0, 0.0,
			1.0f, -1.0f, 1.0, 0.0,
			-1.0f,  1.0f, 0.0, 1.0,
			1.0f,  1.0f, 1.0, 1.0,
			-1.0f,  1.0f, 0.0, 1.0,
			1.0f, -1.0f, 1.0, 0.0
	};
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)( 2 * sizeof(float) ));
	glEnableVertexAttribArray(1);

	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>( glfwGetTime() );
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		auto sigmoid = [] (float v)
		{
			return 1 / ( 1.0 + exp(8.0 - v * 40.0) );
		};
		mixSkyColorPreset(sigmoid(lightDir.y), highSunPreset, presetSunset);
		lightPos = lightDir * 1e6f + camera.Position;

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.3, 0.3, 0.3, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//skyboxFBO->bind();
		float cameraNear = 0.1;
		float cameraFar = 100;
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNear, cameraFar);
		glm::mat4 vp = projection * view;
		skyboxShader.use();
		skyboxShader.setVec2("resolution", glm::vec2(SCR_WIDTH, SCR_HEIGHT));
		skyboxShader.setMat4("inv_proj", glm::inverse(projection));
		skyboxShader.setMat4("inv_view", glm::inverse(view));
		skyboxShader.setVec3("lightDirection", glm::normalize(lightPos - camera.Position));
		skyboxShader.setVec3("skyColorTop", skyColorTop);
		skyboxShader.setVec3("skyColorBottom", skyColorBottom);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow * window)
{
	camera.MovementSpeed = 50.;
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow * window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>( xposIn );
	float ypos = static_cast<float>( yposIn );
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
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
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>( yoffset ));
}

unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char * data = stbi_load(path, &width, &height, &nrComponents, 0);
	if(data)
	{
		GLenum format;
		if(nrComponents == 1)
			format = GL_RED;
		else if(nrComponents == 3)
			format = GL_RGB;
		else if(nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}