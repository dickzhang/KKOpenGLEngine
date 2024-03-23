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

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
void processInput(GLFWwindow * window);
unsigned int loadTexture(const char * path);
void initializePlaneVAO(const int res, const int width, GLuint * planeVAO, GLuint * planeVBO, GLuint * planeEBO);

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

glm::vec3 fogColor(0.5, 0.6, 0.7);

glm::vec3 lightColor(255 / 255.0, 255 / 255.0, 230 / 255.0);
glm::vec3 lightPos = glm::vec3(100, 50000.0, -50000.0);
glm::mat4 modelMatrix = glm::mat4(1.0);

unsigned int planeVBO, planeVAO, planeEBO;
int res = 4;
float frequency = 0.01f;
float fogFalloff = 1.5f;
float dispFactor = 20.0f;
float tessMultiplier = 1.0;
float grassCoverage = 0.65;
float power = 3.0;
int octaves = 13;
glm::vec3 rockColor = glm::vec4(120, 105, 75, 255) * 1.5f / 255.f;
bool drawFog = true;
unsigned int * textures, posBuffer = 0;
std::vector<glm::vec2> positionVec;
int length = 120;
int gridLength = length + ( length + 1 ) % 2;
Shader shad;
float waterHeight = 10.0f;
float up = 0.0;
glm::vec3 seed = glm::vec3(0.0);
const int tileW = 10. * 100.;

void setPos(int row, int col, glm::vec2 pos)
{
	positionVec[col + row * gridLength] = pos;
}

glm::vec2 getPos(int row, int col)
{
	return positionVec[col + row * gridLength];
}
void deleteBuffer(){
	glDeleteBuffers(1, &posBuffer);
	posBuffer = 0;
}
void generateTileGrid(glm::vec2 offset)
{
	float sc = tileW;
	glm::vec2 I = glm::vec2(1, 0) * sc;
	glm::vec2 J = glm::vec2(0, 1) * sc;

	for(int i = 0; i < gridLength; i++) {
		for(int j = 0; j < gridLength; j++) {
			glm::vec2 pos = (float)( j - gridLength / 2 ) * glm::vec2(I) + (float)( i - gridLength / 2 ) * glm::vec2(J);
			setPos(i, j, pos + offset);
		}
	}
}
void setPositionsArray(std::vector<glm::vec2> & pos) {
	if(posBuffer) {
		deleteBuffer();
	}
	glGenBuffers(1, &posBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec2), &pos[0], GL_STATIC_DRAW);

	glBindVertexArray(planeVAO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);

	glVertexAttribDivisor(3, 1);
	glBindVertexArray(0);
}

bool inTile(const Camera camera, glm::vec2 pos) {
	float camX = camera.Position.x;
	float camY = camera.Position.z;

	float x = pos.x;
	float y = pos.y;

	bool inX = false;
	if(( camX <= x + 1.0 * tileW / 2.0f ) && ( camX >= x - 1.0 * tileW / 2.0f )) {
		inX = true;
	}
	bool inY = false;
	if(( camY <= y + 1.0 * tileW / 2.0f ) && ( camY >= y - 1.0 * tileW / 2.0f )) {
		inY = true;
	}
	bool result = inX && inY;
	return result;
}

bool getWhichTileCameraIs(glm::vec2 & result) {

	for(glm::vec2 p : positionVec) {
		if(inTile(camera, p)) {
			//std::cout << "You're in Tile: " << p.x << ", " << p.y << std::endl;
			result = p;
			return true;
		}
	}
	return false;
}

void updateTilesPositions() {
	glm::vec2 camPosition(camera.Position.x, camera.Position.z);
	int whichTile = -1;
	int howManyTiles = 0;

	glm::vec2 currentTile;
	if(getWhichTileCameraIs(currentTile)) {
		glm::vec2 center = getPos(gridLength / 2, gridLength / 2);
		for(glm::vec2 & p : positionVec) {
			p += currentTile - center;
		}
		setPositionsArray(positionVec);

		/*if(waterPtr) {
			glm::vec2 center = getPos(gridLength / 2, gridLength / 2);
			waterPtr->setPosition(center, 1.0 * gridLength, waterPtr->getHeight());
		}*/
	}
}

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
	shad.loadShader("terrain.vs", "terrain.fs", nullptr, "terrain.tcs", "terrain.tes");
	initializePlaneVAO(res, tileW, &planeVAO, &planeVBO, &planeEBO);
	textures = new unsigned int[6];
	textures[0] = loadTexture(FileSystem::getPath("resources/textures/terrain/sand.jpg").c_str());
	textures[1] = loadTexture(FileSystem::getPath("resources/textures/terrain/grass.jpg").c_str());
	textures[2] = loadTexture(FileSystem::getPath("resources/textures/terrain/rdiffuse.jpg").c_str());
	textures[3] = loadTexture(FileSystem::getPath("resources/textures/terrain/snow2.jpg").c_str());
	textures[4] = loadTexture(FileSystem::getPath("resources/textures/terrain/rnormal.jpg").c_str());
	textures[5] = loadTexture(FileSystem::getPath("resources/textures/terrain/terrainTexture.jpg").c_str());
	positionVec.resize(gridLength * gridLength);
	generateTileGrid(glm::vec2(0.0, 0.0));
	setPositionsArray(positionVec);

	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>( glfwGetTime() );
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glClearColor(0.3, 0.3, 0.3, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float cameraNear = 5.f;
		float cameraFar = 1000000.0f;
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, cameraNear, cameraFar);
		glm::mat4 view = camera.GetViewMatrix();

		if(up != 0.0f) {
			glEnable(GL_CLIP_DISTANCE0);
		}

		shad.use();
		shad.setVec3("gEyeWorldPos", camera.Position);
		shad.setMat4("gWorld", modelMatrix);
		shad.setMat4("gVP", projection * view);
		shad.setFloat("gDispFactor", dispFactor);

		glm::vec4 clipPlane(0.0, 1.0, 0.0, -waterHeight);
		shad.setVec4("clipPlane", clipPlane * up);
		shad.setVec3("u_LightColor", lightColor);
		shad.setVec3("u_LightPosition", lightPos);
		shad.setVec3("u_ViewPosition", camera.Position);
		shad.setVec3("fogColor", fogColor);
		shad.setVec3("rockColor", rockColor);
		shad.setVec3("seed", seed);
		shad.setInt("octaves", octaves);
		shad.setFloat("freq", frequency);
		shad.setFloat("u_grassCoverage", grassCoverage);
		shad.setFloat("waterHeight", waterHeight);
		shad.setFloat("tessMultiplier", tessMultiplier);
		shad.setFloat("fogFalloff", fogFalloff * 1.e-6);
		shad.setFloat("power", power);
		shad.setBool("normals", true);
		shad.setBool("drawFog", drawFog);

		// set textures
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		shad.setInt("sand", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		shad.setInt("grass", 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		shad.setInt("rock", 3);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, textures[3]);
		shad.setInt("snow", 4);

		shad.setSampler2D("grass1", textures[5], 5);

		shad.setSampler2D("rockNormal", textures[4], 6);

		int nIstances = positionVec.size();
		glBindVertexArray(planeVAO);
		glDrawElementsInstanced(GL_PATCHES, ( res - 1 ) * ( res - 1 ) * 2 * 3, GL_UNSIGNED_INT, 0, nIstances);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow * window)
{
	camera.MovementSpeed =500.;
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
