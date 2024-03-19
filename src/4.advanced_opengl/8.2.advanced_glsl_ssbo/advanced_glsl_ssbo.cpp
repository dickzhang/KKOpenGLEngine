#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

#define UseArray 1  //是否使用数组保存数据 

//经过测试发现,结构体里面使用列表的方式保存数据不可行,只能用数组和shader里面结构体对应

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct  SSBO_Data
{
	glm::mat4 projection;
	glm::mat4 view;
#if UseArray
	glm::vec4 colors[4];
#else
	std::vector<glm::vec4> colors;
#endif // UseArray
};

SSBO_Data ssbo_data;
int main()
{
	// glfw: initialize and configure
	// ------------------------------
	//SSBO OpenGLes支持版本3.1及以上,OpenGL 支持版本4.3
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
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

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader shader("8.2.advanced_glsl.vs", "8.2.advanced_glsl.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float cubeVertices[] = {
		// positions         
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};
	// cube VAO
	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	unsigned int SSBO;
	ssbo_data.projection = glm::perspective(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glGenBuffers(1, &SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo_data), &ssbo_data, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO); //与着色器中binding值绑定，这里设置是1

	//SSBO参数数据的获取
   // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);//隔断作用，为了让数据修改完成
   // p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY); //获取着色器buffer块内存地址
   // memcpy(&ssbo_data, p, sizeof(ssbo_data));//拷贝buffer块数据到结构体
   // glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

#if UseArray
	//todo 2.通过数组填充数据
	ssbo_data.colors[0] = glm::vec4(1.0,0,0,1.0);
	ssbo_data.colors[1] = glm::vec4(0,1.0,0,1.0);
	ssbo_data.colors[2] = glm::vec4(0,0,1.0,1.0);
	ssbo_data.colors[3] = glm::vec4(0.5,0.3,0.3,1);
#else
	//todo 1.通过集合进行动态填充数据
	ssbo_data.colors.push_back(glm::vec4(1.0, 0, 0, 1.0));
	ssbo_data.colors.push_back(glm::vec4(0, 1.0, 0, 1.0));
	ssbo_data.colors.push_back(glm::vec4(0, 0, 1.0, 1.0));
	ssbo_data.colors.push_back(glm::vec4(0.5, 0.3, 0.3, 1));
#endif // 

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// set the view and projection matrix in the uniform block - we only have to do this once per loop iteration.
		ssbo_data.view = camera.GetViewMatrix();
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);//先绑定到SSBO对象
		GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);//获取着色器中buffer块的内存地址
#if UseArray
		auto datasize=sizeof(ssbo_data);
		memcpy(p, &ssbo_data, datasize);//将结构体数据拷贝到着色器buffer块地址上
#else
		auto datasize = 2 * sizeof(glm::mat4) + ssbo_data.colors.size() * sizeof(glm::vec4);
		memcpy(p, &ssbo_data, datasize);//将结构体数据拷贝到着色器buffer块地址上
#endif
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);//释放内存地址
		// draw 4 cubes 
		// RED
		glBindVertexArray(cubeVAO);
		shader.use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f)); // move top-left
		shader.setMat4("model", model);
		shader.setInt("index",0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// GREEN
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f)); // move top-right
		shader.setMat4("model", model);
		shader.setInt("index", 1);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// YELLOW
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f)); // move bottom-left
		shader.setMat4("model", model);
		shader.setInt("index", 2);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// BLUE
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f)); // move bottom-right
		shader.setMat4("model", model);
		shader.setInt("index", 3);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
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

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
