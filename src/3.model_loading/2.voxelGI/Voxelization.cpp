#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/shader_c.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void VoxelizeStaticScene();
void UpdateRadiance();
void DrawVoxels();
void InjectRadiance();
void GenerateMipmap();
void GenerateMipmapBase(const unsigned int& textureID);
void GenerateMipmapVolume();

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
// camera
Camera camera(glm::vec3(0.0f, 200.0f, 0.0f));
//Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float zero[4] = { 0, 0, 0, 0 };
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
unsigned int voxelAlbedo;
unsigned int voxelNormal;
unsigned int voxelRadiance;

std::array<unsigned int, 6> voxelTexMipmap;
std::vector<glm::mat4> viewProjectionMatrix;
std::vector<glm::mat4> viewProjectionMatrixI;
unsigned int volumeDimension = 256;
unsigned int voxelCount;
float volumeGridSize;
float voxelSize;
bool normalWeightedLambert = true;
bool injectFirstBounce = true;
float maxTracingDistance = 0.95;
unsigned int drawMipLevel = 0;

unsigned int VBO, GIVAO;
Shader ourShader;
Shader drawvoxlShader;
Model ourModel;
ComputeShader radianceShader;
ComputeShader propagationShader;
ComputeShader mipmapvolumeShader;
ComputeShader mipmapbaseShader;


void CreateTriangle()
{
	float vertices[] = {
		// positions  // texture coords
		 1.f,  1.f, 0.0f,1.0f, 1.0f, // top right
		 1.f, -1.f, 0.0f,1.0f, 0.0f, // bottom right
		-1.f, -1.f, 0.0f,0.0f, 0.0f, // bottom left
		-1.f,  1.f, 0.0f,0.0f, 1.0f  // top left 
	};
	glGenVertexArrays(1, &GIVAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(GIVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

//==========================进行场景体素化操作===============================
int main()
{
	glfwInit();
	//glClearTexImage只有OpenGL 4.5才支持
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	//stbi_set_flip_vertically_on_load(true);

	ourShader.LoadShader("Voxelization.vs", "Voxelization.fs", "Voxelization.gs");
	drawvoxlShader.LoadShader("Draw_Voxels.vs", "Draw_Voxels.fs", "Draw_Voxels.gs");
	//ourModel.loadModel(FileSystem::getPath("resources/objects/backpack/backpack.obj"));
	ourModel.loadModel(FileSystem::getPath("resources/objects/cryteksponza/sponza.obj"));

	radianceShader.LoadShader("inject_radiance.cs");
	propagationShader.LoadShader("inject_propagation.cs");
	mipmapvolumeShader.LoadShader("aniso_mipmapvolume.cs");
	mipmapbaseShader.LoadShader("aniso_mipmapbase.cs");

	voxelCount = volumeDimension * volumeDimension * volumeDimension;
	voxelSize = volumeGridSize / volumeDimension;

	// albedo volume
	glGenTextures(1, &voxelAlbedo);
	glBindTexture(GL_TEXTURE_3D, voxelAlbedo);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, volumeDimension, volumeDimension, volumeDimension, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// generate normal volume for radiance injection
	glGenTextures(1, &voxelNormal);
	glBindTexture(GL_TEXTURE_3D, voxelNormal);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, volumeDimension, volumeDimension, volumeDimension, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// generate normal volume for radiance injection
	glGenTextures(1, &voxelRadiance);
	glBindTexture(GL_TEXTURE_3D, voxelRadiance);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, volumeDimension, volumeDimension, volumeDimension, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	for (int i = 0; i < 6; i++)
	{
		glGenTextures(1, &voxelTexMipmap[i]);
		glBindTexture(GL_TEXTURE_3D, voxelTexMipmap[i]);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, volumeDimension / 2, volumeDimension / 2, volumeDimension / 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_3D);
	}

	CreateTriangle();
	glm::vec3 axisSize = ourModel.GetSize();
	glm::vec3 center = ourModel.GetCenter();
	volumeGridSize = glm::max(axisSize.x, glm::max(axisSize.y, axisSize.z));
	voxelSize = volumeGridSize / volumeDimension;
	auto halfSize = volumeGridSize / 2.0f;
	// projection matrices
	auto projection = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, 0.0f, volumeGridSize);
	// view matrices
	viewProjectionMatrix.push_back(glm::lookAt(center + glm::vec3(halfSize, 0.0f, 0.0f), center, glm::vec3(0.0f, 1.0f, 0.0f)));
	viewProjectionMatrix.push_back(glm::lookAt(center + glm::vec3(0.0f, halfSize, 0.0f), center, glm::vec3(0.0f, 0.0f, -1.0f)));
	viewProjectionMatrix.push_back(glm::lookAt(center + glm::vec3(0.0f, 0.0f, halfSize), center, glm::vec3(0.0f, 1.0f, 0.0f)));

	for (auto& matrix : viewProjectionMatrix)
	{
		matrix = projection * matrix;
		viewProjectionMatrixI.push_back(inverse(matrix));
	}
	bool isFirstFrame = true;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		//if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS|| isFirstFrame)
		{
			isFirstFrame = false;
			VoxelizeStaticScene();
		}
#if 1
		DrawVoxels();
#endif
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &GIVAO);
	glDeleteTextures(1, &voxelAlbedo);
	glDeleteTextures(1, &voxelNormal);
	glfwTerminate();
	return 0;
}

void VoxelizeStaticScene()
{
	glColorMask(false, false, false, false);
	glViewport(0, 0, volumeDimension, volumeDimension);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ourShader.use();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	int count = 0;
	for (auto matrix : viewProjectionMatrix)
	{
		ourShader.setMat4("viewProjections[" + std::to_string(count) + "]", matrix);
		count++;
	}
	count = 0;
	for (auto matrix : viewProjectionMatrixI)
	{
		ourShader.setMat4("viewProjectionsI[" + std::to_string(count) + "]", matrix);
		count++;
	}
	ourShader.setFloat("voxelScale", 1.0f / volumeGridSize);
	ourShader.setVec3("worldMinPoint", ourModel.GetMin());
	ourShader.setInt("volumeDimension", volumeDimension);

	glm::mat4 model = glm::mat4(1.0f);
	ourShader.setMat4("model", model);

	glClearTexImage(voxelAlbedo, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
	glClearTexImage(voxelNormal, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);

	glBindImageTexture(5, voxelAlbedo, 0, true, 0, GL_READ_WRITE, GL_R32UI);
	glBindImageTexture(6, voxelNormal, 0, true, 0, GL_READ_WRITE, GL_R32UI);
	ourModel.Draw(ourShader);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	UpdateRadiance();
}

void UpdateRadiance()
{
	InjectRadiance();
	GenerateMipmap();
}

void InjectRadiance()
{
	radianceShader.use();
	radianceShader.setFloat("voxelSize", voxelSize);
	radianceShader.setFloat("voxelScale", 1.0f / volumeGridSize);
	radianceShader.setVec3("worldMinPoint", ourModel.GetMin());
	radianceShader.setVec3("lightdiffuse", glm::vec3(1.0, 1.0, 1.0));
	radianceShader.setVec3("lightdirection", glm::vec3(-0.321393847, 0.766044438, -0.556670427));
	radianceShader.setInt("normalWeightedLambert", normalWeightedLambert ? 1 : 0);
	radianceShader.setInt("volumeDimension", volumeDimension);

	static float zero[] = { 0, 0, 0, 0 };
	glClearTexImage(voxelRadiance, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, voxelAlbedo);

	glBindImageTexture(1, voxelNormal, 0, true, 0, GL_READ_WRITE, GL_RGBA8);
	glBindImageTexture(2, voxelRadiance, 0, true, 0, GL_WRITE_ONLY, GL_RGBA8);

	auto workGroups = static_cast<unsigned>(glm::ceil(volumeDimension / 8.0f));
	glDispatchCompute(workGroups, workGroups, workGroups);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void GenerateMipmap()
{
	if (injectFirstBounce)
	{
		propagationShader.use();
		propagationShader.setFloat("maxTracingDistanceGlobal", maxTracingDistance);
		propagationShader.setInt("volumeDimension", volumeDimension);

		glBindImageTexture(0, voxelRadiance, 0, true, 0, GL_READ_WRITE, GL_RGBA8);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, voxelAlbedo);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, voxelNormal);

		for (int i = 0; i < voxelTexMipmap.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE3 + i);
			glBindTexture(GL_TEXTURE_3D, voxelTexMipmap[i]);
		}
		auto workGroups = static_cast<unsigned>(glm::ceil(volumeDimension / 8.0f));
		glDispatchCompute(workGroups, workGroups, workGroups);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		GenerateMipmapBase(voxelRadiance);
		GenerateMipmapVolume();
	}
	else
	{
		GenerateMipmapBase(voxelRadiance);
		GenerateMipmapVolume();
	}
}
void GenerateMipmapBase(const unsigned int& textureID)
{
	auto halfDimension = volumeDimension / 2;
	mipmapbaseShader.use();
	mipmapbaseShader.setInt("mipDimension", halfDimension);

	for (int i = 0; i < voxelTexMipmap.size(); ++i)
	{
		glBindImageTexture(i, voxelTexMipmap[i], 0, true, 0, GL_WRITE_ONLY, GL_RGBA8);
	}

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_3D, textureID);

	auto workGroups = static_cast<unsigned int>(ceil(halfDimension / 8));
	glDispatchCompute(workGroups, workGroups, workGroups);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void GenerateMipmapVolume()
{
	mipmapvolumeShader.use();
	auto mipDimension = volumeDimension / 4;
	auto mipLevel = 0;
	while (mipDimension >= 1)
	{
		auto volumeSize = glm::vec3(mipDimension, mipDimension, mipDimension);
		mipmapvolumeShader.setVec3("mipDimension", volumeSize);
		mipmapvolumeShader.setInt("mipLevel", mipLevel);

		for (auto i = 0; i < voxelTexMipmap.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE5 + i);
			glBindTexture(GL_TEXTURE_3D, voxelTexMipmap[i]);
			glBindImageTexture(i, voxelTexMipmap[i], mipLevel + 1, true, 0, GL_WRITE_ONLY, GL_RGBA8);
		}
		auto workGroups = static_cast<unsigned>(glm::ceil(mipDimension / 8.0f));
		glDispatchCompute(workGroups, workGroups, workGroups);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		mipLevel++;
		mipDimension /= 2;
	}
}

void DrawVoxels()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
	glColorMask(true, true, true, true);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClearDepth(1.0f);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	drawvoxlShader.use();
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.3f, 10000.0f);
	glm::mat4 view = camera.GetViewMatrix();
	view=glm::rotate(view, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	auto& viewProjection = projection * view;
	auto vDimension = static_cast<unsigned>(volumeDimension / pow(2.0f, drawMipLevel));
	auto vSize = volumeGridSize / vDimension;
	// pass voxel drawer uniforms
	//glBindImageTexture(0, voxelAlbedo, 0, true, 0, GL_READ_ONLY, GL_RGBA8);
	glBindImageTexture(0, voxelRadiance, 0, true, 0, GL_READ_ONLY, GL_RGBA8);
	//glBindImageTexture(0, voxelNormal, 0, true, 0, GL_READ_ONLY, GL_RGBA8);
	//glBindImageTexture(0, voxelTexMipmap[1], 0, true, 0, GL_READ_ONLY, GL_RGBA8);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, ourModel.GetMin());
	modelMatrix = glm::scale(modelMatrix, glm::vec3(vSize));

	drawvoxlShader.setInt("volumeDimension", vDimension);
	drawvoxlShader.setMat4("modelViewProjection", viewProjection * modelMatrix);
	camera.ExtractPlanes(viewProjection);
	auto& planes = camera.Planes();
	for (auto i = 0; i < 6; i++)
	{
		drawvoxlShader.setVec4("frustumPlanes[" + std::to_string(i) + "]", planes[i]);
	}
	drawvoxlShader.setFloat("voxelSize", voxelSize);
	drawvoxlShader.setVec3("worldMinPoint", ourModel.GetMin());
	glBindVertexArray(GIVAO);//需要绑定一个VAO 不然绘制不出来
	glDrawArrays(GL_POINTS, 0, voxelCount);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
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
	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
