#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader_t.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <iostream>

#include "HeightMapGenerator.h"
#include "Terrain.h"

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
void processInput(GLFWwindow * window);
unsigned int loadTexture(const char * path);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;
// camera
Camera camera(glm::vec3(0.0f, 800.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float intensity = 0.35f;
glm::vec3 lightColor(1, 1, 1);
glm::vec3 lightPos = glm::vec3(1024, 200, 3072);
glm::vec3 direction = glm::vec3(-1, -1, 0);

glm::mat4 modelMatrix = glm::mat4(1.0);
float waterHeight = 0.;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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
	int seed = rand() % 1000;
	HeightMapGenerator::Generate(FileSystem::getPath("resources/textures/HeightMaps/heightMap").c_str(), 256, 256, 7, seed);
	Shader  shader("Shader.vs", "Shader.fs");
	TerrainMesh * terrainMesh = Terrain::GenerateMesh(FileSystem::getPath("resources/textures/HeightMaps/heightMap.png").c_str());
	unsigned int grassTexture = loadTexture(FileSystem::getPath("resources/textures/HeightMaps/grass.jpg").c_str());

	glm::mat4 identity = glm::mat4(1.0);
	modelMatrix = glm::translate(identity, glm::vec3(0,500,0));

	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>( glfwGetTime() );
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		glDisable(GL_CLIP_DISTANCE0);
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClearColor(0.3, 0.3, 0.3, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float cameraNear = 1.f;
		float cameraFar = 10000.0f;
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNear, cameraFar);
		glm::mat4 view = camera.GetViewMatrix();

		shader.use();
		float up = 0.0;
		glm::vec4 clipPlane(0.0, 1.0, 0.0, -waterHeight);
		shader.setVec4("clipPlane", clipPlane * up);
		shader.setFloat("textureTiling",10);
		shader.setMat4("gProj", projection);
		shader.setMat4("gCamera", view);
		shader.setMat4("gWorld", modelMatrix);
		shader.setVec3("directionalLight.color",lightColor);
		shader.setVec3("directionalLight.direction",direction);
		shader.setFloat("directionalLight.intensity",intensity);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grassTexture);
		terrainMesh->GetVertexArray()->Bind();
		terrainMesh->GetIndexBuffer()->Bind();
		glDrawElements(GL_TRIANGLES, terrainMesh->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow * window)
{
	camera.MovementSpeed =10;
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
