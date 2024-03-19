#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_tfb.h>
#include <learnopengl/camera.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset);
void processInput(GLFWwindow * window);

int frame_count = 0;
// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Member variables
GLuint vao[2];
GLuint vbo[2];

GLuint geometry_vbo;
GLuint render_vao;
GLuint geometry_tex;

GLuint cube_vbo;
GLuint cube_vao;

const int point_count = 5000;
static unsigned int seed = 0x13371337;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

char * varyings[] =
{
	"position_out", "velocity_out"
};
char * varyings2[] =
{
	"world_space_position"
};
struct buffer_t
{
	glm::vec4 position;
	glm::vec3 velocity;
};

float random_float()
{
	float res;
	unsigned int tmp;

	seed *= 16807;

	tmp = seed ^ ( seed >> 4 ) ^ ( seed << 15 );

	*( (unsigned int *)&res ) = ( tmp >> 9 ) | 0x3F800000;

	return ( res - 1.0f );
}

glm::vec3 random_vector(float minmag = 0.0f, float maxmag = 1.0f)
{
	glm::vec3 randomvec(random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f);
	randomvec = normalize(randomvec);
	randomvec *= ( random_float() * ( maxmag - minmag ) + minmag );

	return randomvec;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow * window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if(window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader blueShader("8.3.blue.vs", "8.3.blue.fs", varyings2, 1);
	Shader tfbShader("8.3.tfb.vs", "8.3.tfb.fs", varyings, 2);

	glGenVertexArrays(2, vao);
	glGenBuffers(2, vbo);

	for(int i = 0; i < 2; i++)
	{
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, vbo[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, point_count * ( sizeof(glm::vec4) + sizeof(glm::vec3) ), NULL, GL_DYNAMIC_COPY);
		if(i == 0)
		{
			buffer_t * buffer = (buffer_t *)glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_WRITE_ONLY);
			for(int j = 0; j < point_count; j++)
			{
				buffer[j].velocity = random_vector();
				buffer[j].position = glm::vec4(buffer[j].velocity + glm::vec3(-0.5f, 2.0f, 0.0f), 1.0f);
				buffer[j].velocity = glm::vec3(buffer[j].velocity[0], buffer[j].velocity[1] * 0.3f, buffer[j].velocity[2] * 0.3f);
				//std::cout << "position" << buffer[j].velocity.x << buffer[j].velocity.y << buffer[j].velocity.z << std::endl;
			}
			glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
		}
		glBindVertexArray(vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) + sizeof(glm::vec3), NULL);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4) + sizeof(glm::vec3), (GLvoid *)sizeof(glm::vec4));
		glEnableVertexAttribArray(1);
	}

	glGenBuffers(1, &geometry_vbo);
	glGenTextures(1, &geometry_tex);
	glBindBuffer(GL_TEXTURE_BUFFER, geometry_vbo);
	glBufferData(GL_TEXTURE_BUFFER, 1024 * 1024 * sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);
	glBindTexture(GL_TEXTURE_BUFFER, geometry_tex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, geometry_vbo);

	glGenVertexArrays(1, &render_vao);
	glBindVertexArray(render_vao);

	glBindBuffer(GL_ARRAY_BUFFER, geometry_vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);


	float vertices[] = {
   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	glGenVertexArrays(1, &cube_vao);
	glGenTextures(1, &cube_vbo);

	glBindVertexArray(cube_vao);
	glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)( 3 * sizeof(float) ));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	while(!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>( glfwGetTime() );
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		blueShader.use();
		blueShader.setMat4("view", view);
		blueShader.setMat4("projection_matrix", projection);
		glm::mat4 model_matrix = glm::mat4(1.0f);
		blueShader.setMat4("model_matrix", model_matrix);

		glBindVertexArray(render_vao);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, geometry_vbo);
		glBeginTransformFeedback(GL_TRIANGLES);

		glBindVertexArray(cube_vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glEndTransformFeedback();


		tfbShader.use();
		tfbShader.setMat4("model_matrix", model_matrix);
	    tfbShader.setMat4("view", view);
		tfbShader.setMat4("projection_matrix", projection);
		tfbShader.setInt("triangle_count", 12);
		tfbShader.setFloat("time_step", deltaTime);
		if(( frame_count & 1 ) != 0)
		{
			//std::cout << "point_count" << frame_count << std::endl;
			glBindVertexArray(vao[1]);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[0]);
		}
		else
		{
			glBindVertexArray(vao[0]);
			glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo[1]);
		}
		glPointSize(5);
		glBeginTransformFeedback(GL_POINTS);
		//std::cout << "point_count" << min(point_count, ( frame_count >> 3 )) << std::endl;
		glDrawArrays(GL_POINTS, 0, min(point_count, ( frame_count >> 3 )));
		glEndTransformFeedback();

		frame_count++;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(2, vao);
	glDeleteBuffers(2, vbo);
	glDeleteBuffers(1, &geometry_vbo);
	glDeleteVertexArrays(1, &render_vao);
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow * window)
{
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

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow * window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>( xposIn );
	float ypos = static_cast<float>( yposIn );

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
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>( yoffset ));
}