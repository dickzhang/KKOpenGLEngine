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

void framebuffer_size_callback(GLFWwindow* window,int width,int height);
void mouse_callback(GLFWwindow* window,double xpos,double ypos);
void scroll_callback(GLFWwindow* window,double xoffset,double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
int showMode = 0;
// camera
Camera camera(glm::vec3(0.0f,0.0f,0.0f));
float lastX = SCR_WIDTH/2.0;
float lastY = SCR_HEIGHT/2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float vertices[] =
{
	-1,-1,5,   0,0,
	-1,1,5,    0,1,
	1,1,5,     1,1,
	1,-1,5,    1,0,
};
unsigned int indices[] = {
	0,1,2, // first triangle
	2,3,0  // second triangle
};
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"LearnOpenGL",NULL,NULL);
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
	Shader shader("edgedetection.vs","edgedetection.fs");

	unsigned int VBO,VAO,EBO;
	glGenVertexArrays(1,&VAO);
	glGenBuffers(1,&VBO);
	glGenBuffers(1,&EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	stbi_set_flip_vertically_on_load(true);
	unsigned int texture = loadTexture(FileSystem::getPath("resources/textures/girl.jpg").c_str());
	shader.use();
	shader.setInt("mainTex",0);

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame-lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		glClearColor(0.3,0.3,0.3,1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		float cameraNear = 0.1;
		float cameraFar = 1000;
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(60.0f),(float)SCR_WIDTH/(float)SCR_HEIGHT,cameraNear,cameraFar);
		glm::mat4 vp = projection*view;

		shader.use();
		glm::mat4 model = glm::mat4(1.0f);
		shader.setMat4("mvp",projection*view*model);
		shader.setVec2("TexelSize",glm::vec2(1.0/512.0,1.0/512.0));
		shader.setVec4("edgecolor",glm::vec4(1.0,0.0,0.0,1.0));
		shader.setVec4("backgroundColor",glm::vec4(0,0,0,1));
		if(showMode==0)
		{
			shader.setFloat("edgeOnly",0);
		}
		else if(showMode==1)
		{
			shader.setFloat("edgeOnly",1.0);
		}
		else if(showMode==2)
		{
			shader.setFloat("edgeOnly",1.0);
		}
		shader.setInt("showMode",showMode);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,texture);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1,&VBO);
	glDeleteBuffers(1,&EBO);
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
	if(glfwGetKey(window,GLFW_KEY_J)==GLFW_PRESS)
	{
		showMode = 0;
	}
	if(glfwGetKey(window,GLFW_KEY_K)==GLFW_PRESS)
	{
		showMode = 1;
	}
	if(glfwGetKey(window,GLFW_KEY_L)==GLFW_PRESS)
	{
		showMode = 2;
	}
}

void framebuffer_size_callback(GLFWwindow* window,int width,int height)
{
	glViewport(0,0,width,height);
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
	}

	float xoffset = xpos-lastX;
	float yoffset = lastY-ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset,yoffset);
}

void scroll_callback(GLFWwindow* window,double xoffset,double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1,&textureID);

	int width,height,nrComponents;
	unsigned char* data = stbi_load(path,&width,&height,&nrComponents,0);
	if(data)
	{
		GLenum format;
		if(nrComponents==1)
			format = GL_RED;
		else if(nrComponents==3)
			format = GL_RGB;
		else if(nrComponents==4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D,textureID);
		glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout<<"Texture failed to load at path: "<<path<<std::endl;
		stbi_image_free(data);
	}

	return textureID;
}