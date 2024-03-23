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
#include "buffers.h"

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
void processInput(GLFWwindow * window);
unsigned int loadTexture(const char * path);
void renderSphere();
void initializePlaneVAO(const int res, const int width, GLuint * planeVAO, GLuint * planeVBO, GLuint * planeEBO);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

static const int FBOw = 1280;
static const int FBOh = 720;
// camera
Camera camera(glm::vec3(0.0f, 800.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 fogColor(0.5, 0.6, 0.7);
glm::vec3 lightColor(255 / 255.0, 255 / 255.0, 230 / 255.0);
glm::vec3 lightPos = glm::vec3(100, 50000.0, -50000.0);
int waterscale = 120;
float waterHeight = 10.;
glm::vec3 waterposition = glm::vec3(0.0, waterHeight, 0.0);
glm::mat4 modelMatrix = glm::mat4(1.0);
FrameBufferObject * reflectionFBO =nullptr;
FrameBufferObject * refractionFBO = nullptr;

const int tileW = 100;
unsigned int planeVAO, planeVBO, planeEBO;
unsigned int dudvMap, normalMap;
unsigned int reflectionMap, refractionMap;
glm::mat4 model = glm::mat4(1.0f);

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
	Shader shader("9.2.water.vs", "9.2.water.fs");

	glm::mat4 identity = glm::mat4(1.0);
	glm::mat4 scaleMatrix = glm::scale(identity, glm::vec3(waterscale));
	glm::mat4 transMatrix = glm::translate(identity, waterposition);
	modelMatrix = transMatrix * scaleMatrix;

	reflectionFBO = new FrameBufferObject(FBOw, FBOh);
	refractionFBO = new FrameBufferObject(FBOw, FBOh);
	initializePlaneVAO(2, tileW, &planeVAO, &planeVBO, &planeEBO);

	Shader ourShader("model_loading.vs", "model_loading.fs");
	Model ourModel(FileSystem::getPath("resources/objects/cryteksponza/sponza.obj"));

	dudvMap = loadTexture(FileSystem::getPath("resources/textures/water/waterDUDV.png").c_str());
	normalMap = loadTexture(FileSystem::getPath("resources/textures/water/normalMap.png").c_str());

	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>( glfwGetTime() );
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClearColor(0.3, 0.3, 0.3, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float cameraNear = 5.f;
		float cameraFar = 1000000.0f;
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNear, cameraFar);
		glm::mat4 view = camera.GetViewMatrix();
		glm::vec4 clipPlane(0.0, 1.0, 0.0, -waterHeight);

		camera.invertPitch();
		camera.Position.y -= 2 * ( camera.Position.y - waterHeight );
		view = camera.GetViewMatrix();
		//·´Éä
		float up = 1.0;
		reflectionFBO->bind();
		glViewport(0, 0, FBOw, FBOh);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ourShader.use();

		glEnable(GL_CLIP_DISTANCE0);
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		ourShader.setVec4("clipPlane", clipPlane * up);
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);

		camera.invertPitch();
		camera.Position.y += 2 * abs( camera.Position.y - waterHeight );
		view = camera.GetViewMatrix();

		//ÕÛÉä
		refractionFBO->bind();
		glViewport(0, 0, FBOw, FBOh);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		up = -1.0;
		ourShader.use();
		glEnable(GL_CLIP_DISTANCE0);
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		ourShader.setVec4("clipPlane", clipPlane * up);
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);

		up = 0.0;
		glDisable(GL_CLIP_DISTANCE0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(0.3, 0.3, 0.3, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		ourShader.use();
		ourShader.setMat4("projection", projection);
		ourShader.setVec4("clipPlane", clipPlane * up);
		ourShader.setMat4("view", view);
		ourShader.setMat4("model", model);
		ourModel.Draw(ourShader);

		//»æÖÆË®
		shader.use();
		shader.setMat4("modelMatrix", modelMatrix);
		shader.setMat4("gVP", projection * view);
		shader.setFloat("textureTiling", tileW/5.0);
		shader.setVec3("cameraPos", camera.Position);
		shader.setVec3("lightPos", lightPos);
		shader.setVec3("lightColor", lightColor);
		shader.setFloat("distorsionStrength", 0.05);
		shader.setFloat("specularPower", 20.0);
		shader.setFloat("near", cameraNear);
		shader.setFloat("far", cameraFar);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reflectionFBO->tex);
		shader.setInt("reflectionSampler", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, refractionFBO->tex);
		shader.setInt("refractionSampler", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, dudvMap);
		shader.setInt("dudvSampler", 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		shader.setInt("normalSampler", 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, refractionFBO->depthTex);
		shader.setInt("depthSampler", 4);

		float waveSpeed = 0.25;
		float time = glfwGetTime();
		float moveFactor = waveSpeed * time;
		shader.setFloat("moveFactor", moveFactor);
		glBindVertexArray(planeVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow * window)
{
	camera.MovementSpeed = 200.;
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

void initializePlaneVAO(const int res, const int width, GLuint * planeVAO, GLuint * planeVBO, GLuint * planeEBO)
{
	//const int res = 3;
	const int nPoints = res * res;
	const int size = nPoints * 3 + nPoints * 3 + nPoints * 2;
	float * vertices = new float[size];
	for(int i = 0; i < res; i++) {
		for(int j = 0; j < res; j++) {
			//add position
			float x = j * (float)width / ( res - 1 ) - width / 2.0;
			float y = 0.0;
			float z = -i * (float)width / ( res - 1 ) + width / 2.0;

			vertices[( i + j * res ) * 8] = x; //8 = 3 + 3 + 2, float per point
			vertices[( i + j * res ) * 8 + 1] = y;
			vertices[( i + j * res ) * 8 + 2] = z;

			//add normal
			float x_n = 0.0;
			float y_n = 1.0;
			float z_n = 0.0;

			vertices[( i + j * res ) * 8 + 3] = x_n;
			vertices[( i + j * res ) * 8 + 4] = y_n;
			vertices[( i + j * res ) * 8 + 5] = z_n;

			//add texcoords
			vertices[( i + j * res ) * 8 + 6] = (float)j / ( res - 1 );
			vertices[( i + j * res ) * 8 + 7] = (float)( res - i - 1 ) / ( res - 1 );
		}
	}

	const int nTris = ( res - 1 ) * ( res - 1 ) * 2;
	int * trisIndices = new int[nTris * 3];

	for(int i = 0; i < nTris; i++) {
		int trisPerRow = 2 * ( res - 1 );
		for(int j = 0; j < trisPerRow; j++) {
			if(!( i % 2 )) { //upper triangle
				int k = i * 3;
				int triIndex = i % trisPerRow;

				int row = i / trisPerRow;
				int col = triIndex / 2;
				trisIndices[k] = row * res + col;
				trisIndices[k + 1] = ++row * res + col;
				trisIndices[k + 2] = --row * res + ++col;
			}
			else {
				int k = i * 3;
				int triIndex = i % trisPerRow;

				int row = i / trisPerRow;
				int col = triIndex / 2;
				trisIndices[k] = row * res + ++col;
				trisIndices[k + 1] = ++row * res + --col;
				trisIndices[k + 2] = row * res + ++col;
			}
		}
	}

	glGenVertexArrays(1, planeVAO);
	glGenBuffers(1, planeVBO);
	glGenBuffers(1, planeEBO);
	glBindVertexArray(*planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, *planeVBO);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nTris * 3 * sizeof(unsigned int), trisIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)( 3 * sizeof(float) ));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)( 6 * sizeof(float) ));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	delete[] vertices;
}
