//// Based on Ferran Soles: https://github.com/fsole/GLSamples/blob/master/src/MultidrawIndirect.cpp
//https://litasa.github.io/blog/2017/09/04/OpenGL-MultiDrawIndirect-with-Individual-Textures

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <learnopengl/shader_m.h>
#include <learnopengl/shader_c.h>

#define USE_COMPUTE_SHADER 1

void processInput(GLFWwindow* window);
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
namespace
{
	struct Vertex2D
	{
		float x, y;  //Position
		float u, v;  //Uv
	};

	struct DrawElementsCommand
	{
		GLuint vertexCount;
		GLuint instanceCount;
		GLuint firstIndex;
		GLuint baseVertex;
		GLuint baseInstance;
	};

	struct Matrix
	{
		float a0, a1, a2, a3;
		float b0, b1, b2, b3;
		float c0, c1, c2, c3;
		float d0, d1, d2, d3;
	};

	struct MeshData {
		GLuint vertexIndex;
		GLuint vertexCount;
		GLuint indicesIndex;
		GLuint indicesCount;
	};

	struct InstanceData {
		GLuint instanceCount;
		GLuint meshIndex;
		GLuint fillData1 = 0;
		GLuint fillData2 = 0;
		Matrix modelMat;
	};

	//����λ��
	void setMatrix(Matrix* matrix, const float x, const float y)
	{
		/*
		1 0 0 0
		0 1 0 0
		0 0 1 0
		x y 0 1
		*/
		matrix->a0 = 1;
		matrix->a1 = matrix->a2 = matrix->a3 = 0;

		matrix->b1 = 1;
		matrix->b0 = matrix->b2 = matrix->b3 = 0;

		matrix->c2 = 1;
		matrix->c0 = matrix->c1 = matrix->c3 = 0;

		matrix->d0 = x;
		matrix->d1 = y;
		matrix->d2 = 0;
		matrix->d3 = 1;
	}

	const std::vector<Vertex2D> gQuad = {
		//xy			//uv
		{ 0.0f,0.0f,	0.0f,0.0f },
		{ 0.1f,0.0f,	1.0f,0.0f },
		{ 0.05f, 0.05f, 0.5f, 0.5f},
		{ 0.0f,0.1f,	0.0f,1.0f },
		{ 0.1f,0.1f,	1.0f,1.0f }
	};

	const std::vector<Vertex2D> gTriangle =
	{
		{ 0.0f, 0.0f,	0.0f,0.0f},
		{ 0.05f, 0.1f,	0.5f,1.0f},
		{ 0.1f, 0.0f,	1.0f,0.0f}
	};

	const std::vector<unsigned int> gQuadIndex = {
		0,1,2,
		1,4,2,
		2,4,3,
		0,2,3
	};

	const std::vector<unsigned int> gTriangleIndex =
	{
		0,1,2
	};
	GLuint gVAO(0);
	GLuint gArrayTexture(0);
	GLuint gVertexBuffer(0);
	GLuint gElementBuffer(0);
	GLuint gIndirectBuffer(0);
	GLuint gMatrixBuffer(0);
	GLuint gProgram(0);
}//Unnamed namespace

DrawElementsCommand vDrawCommand[100];
void GenerateVertexsAndIndirectBuffer()
{
	const unsigned num_vertices = gQuad.size() + gTriangle.size();
	std::vector<Vertex2D> vVertex(num_vertices);
	unsigned vertexIndex(0);
	for (unsigned int k(0); k != gQuad.size(); ++k)
	{
		vVertex[vertexIndex++] = gQuad[k];
	}
	for (unsigned int k(0); k != gTriangle.size(); ++k)
	{
		vVertex[vertexIndex++] = gTriangle[k];
	}

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
	//Create a vertex buffer object
	glGenBuffers(1, &gVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * vVertex.size() * 50, NULL, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (GLvoid*)(offsetof(Vertex2D, x)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (GLvoid*)(offsetof(Vertex2D, u)));

	int triangle_bytes = sizeof(unsigned int) * gTriangleIndex.size();
	int quad_bytes = sizeof(unsigned int) * gQuadIndex.size();
	glGenBuffers(1, &gElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_bytes + quad_bytes, NULL, GL_STATIC_DRAW);

	//glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, quad_bytes, gQuadIndex.data());
	//glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, quad_bytes, triangle_bytes, gTriangleIndex.data());

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, quad_bytes, gQuadIndex.data());
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, quad_bytes, triangle_bytes, gTriangleIndex.data());

	GLuint originVertexDataBuffer;
	glGenBuffers(1, &originVertexDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, originVertexDataBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vertex2D) * vVertex.size(), vVertex.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, originVertexDataBuffer);

	std::vector<MeshData> meshDatas(2);
	meshDatas[0].vertexIndex = 0;
	meshDatas[0].vertexCount = 5;
	meshDatas[0].indicesIndex = 0;
	meshDatas[0].indicesCount = 12;
	meshDatas[1].vertexIndex = 5;
	meshDatas[1].vertexCount = 3;
	meshDatas[1].indicesIndex = 12;
	meshDatas[1].indicesCount = 3;

	GLuint meshDataBuffer;
	glGenBuffers(1, &meshDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshDataBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MeshData) * meshDatas.size(), meshDatas.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, meshDataBuffer);

	std::vector<InstanceData> instanceDatas(100);

	float xOffset(-0.95f);
	float yOffset(-0.95f);
	// populate geometry
	unsigned matrixIndex(0);
	for (unsigned int i(0); i != 10; ++i)
	{
		for (unsigned int j(0); j != 10; ++j)
		{
			if (j % 2 == 0)
			{
				instanceDatas[matrixIndex].meshIndex = 1;
			}
			else
			{
				instanceDatas[matrixIndex].meshIndex = 0;
			}
			instanceDatas[matrixIndex].instanceCount = 1;
			//set position in model matrix
			setMatrix(&instanceDatas[matrixIndex++].modelMat, xOffset, yOffset);
			xOffset += 0.2f;
		}
		yOffset += 0.2f;
		xOffset = -0.95f;
	}

	GLuint instanceDataBuffer;
	glGenBuffers(1, &instanceDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, instanceDataBuffer);
	InstanceData* dataPtr = instanceDatas.data();
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(InstanceData) * instanceDatas.size(), instanceDatas.data(), GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, instanceDataBuffer);

	GLuint drawElementsCommandBuffer;
	glGenBuffers(1, &drawElementsCommandBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawElementsCommandBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsCommand) * 100, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, drawElementsCommandBuffer);

	GLuint computedVertexDataBuffer;
	glGenBuffers(1, &computedVertexDataBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, computedVertexDataBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vertex2D) * 50 * vVertex.size(), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, computedVertexDataBuffer);

	ComputeShader computeShader("8.1.drawIndirect.cs");
	ComputeShader computeShader1("8.1.drawIndirect1.cs");
	computeShader.use();
	glDispatchCompute(instanceDatas.size(), 1, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	void* ptr = nullptr;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, drawElementsCommandBuffer);
	ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DrawElementsCommand) * 100, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	memcpy(vDrawCommand, ptr, sizeof(DrawElementsCommand) * 100);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glGenBuffers(1, &gIndirectBuffer);
	//feed the draw command data to the gpu
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gIndirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(vDrawCommand), vDrawCommand, GL_DYNAMIC_DRAW);

	const unsigned len = 50 * vVertex.size();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, computedVertexDataBuffer);
	ptr = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Vertex2D) * len, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex2D) * len, ptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glDeleteBuffers(1, &computedVertexDataBuffer);
	glDeleteBuffers(1, &drawElementsCommandBuffer);
	glDeleteBuffers(1, &instanceDataBuffer);
	glDeleteBuffers(1, &meshDataBuffer);
	glDeleteBuffers(1, &originVertexDataBuffer);
}

void GenerateGeometry()
{
	//Generate 50 quads, 50 triangles
	const unsigned num_vertices = gQuad.size() * 50 + gTriangle.size() * 50;
	std::vector<Vertex2D> vVertex(num_vertices);
	Matrix vMatrix[100];
	unsigned vertexIndex(0);
	unsigned matrixIndex(0);
	//Clipspace, lower left corner = (-1, -1)
	float xOffset(-0.95f);
	float yOffset(-0.95f);
	// populate geometry
	for (unsigned int i(0); i != 10; ++i)
	{
		for (unsigned int j(0); j != 10; ++j)
		{
			//quad
			if (j % 2 == 0)
			{
				for (unsigned int k(0); k != gQuad.size(); ++k)
				{
					vVertex[vertexIndex++] = gQuad[k];
				}
			}
			//triangle
			else
			{
				for (unsigned int k(0); k != gTriangle.size(); ++k)
				{
					vVertex[vertexIndex++] = gTriangle[k];
				}
			}
			//set position in model matrix
			setMatrix(&vMatrix[matrixIndex++], xOffset, yOffset);
			xOffset += 0.2f;
		}
		yOffset += 0.2f;
		xOffset = -0.95f;
	}

	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);
	//Create a vertex buffer object
	glGenBuffers(1, &gVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2D) * vVertex.size(), vVertex.data(), GL_STATIC_DRAW);

	//Specify vertex attributes for the shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (GLvoid*)(offsetof(Vertex2D, x)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (GLvoid*)(offsetof(Vertex2D, u)));

	//Create an element buffer and populate it
	int triangle_bytes = sizeof(unsigned int) * gTriangleIndex.size();
	int quad_bytes = sizeof(unsigned int) * gQuadIndex.size();
	glGenBuffers(1, &gElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_bytes + quad_bytes, NULL, GL_STATIC_DRAW);

	//glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, quad_bytes, gQuadIndex.data());
	//glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, quad_bytes, triangle_bytes, gTriangleIndex.data());

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, triangle_bytes, gTriangleIndex.data());
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, triangle_bytes, quad_bytes, gQuadIndex.data());

	//Setup per instance matrices
	//Method 1. Use Vertex attributes and the vertex attrib divisor
	glGenBuffers(1, &gMatrixBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gMatrixBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vMatrix), vMatrix, GL_STATIC_DRAW);
	//A matrix is 4 vec4s
	glEnableVertexAttribArray(3 + 0);
	glEnableVertexAttribArray(3 + 1);
	glEnableVertexAttribArray(3 + 2);
	glEnableVertexAttribArray(3 + 3);

	glVertexAttribPointer(3 + 0, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (GLvoid*)(offsetof(Matrix, a0)));
	glVertexAttribPointer(3 + 1, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (GLvoid*)(offsetof(Matrix, b0)));
	glVertexAttribPointer(3 + 2, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (GLvoid*)(offsetof(Matrix, c0)));
	glVertexAttribPointer(3 + 3, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (GLvoid*)(offsetof(Matrix, d0)));
	//Only apply one per instance
	glVertexAttribDivisor(3 + 0, 1);
	glVertexAttribDivisor(3 + 1, 1);
	glVertexAttribDivisor(3 + 2, 1);
	glVertexAttribDivisor(3 + 3, 1);
}

void GenerateArrayTexture()
{
	glGenTextures(1, &gArrayTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, gArrayTexture);
	// GL4.0��API
	//glTexStorage3D(GL_TEXTURE_2D_ARRAY,
	//    1,                    //No mipmaps as textures are 1x1
	//    GL_RGB8,              //Internal format
	//    1, 1,                 //width,height
	//    100                   //Number of layers
	//);
	glTexImage3D(GL_TEXTURE_2D_ARRAY,
		0,                    //No mipmaps as textures are 1x1
		GL_RGB8,              //Internal format
		1, 1, 100,            //width,height,depth
		0,                   //border must be 0
		GL_RGB,
		GL_UNSIGNED_BYTE,
		NULL
	);
	for (unsigned int i(0); i != 100; ++i)
	{
		//Choose a random color for the i-essim image
		GLubyte color[3] = { GLubyte(rand() % 255),GLubyte(rand() % 255),GLubyte(rand() % 255) };

		//Specify i-essim image
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
			0,                     //Mipmap number
			0, 0, i,               //xoffset, yoffset, zoffset
			1, 1, 1,               //width, height, depth
			GL_RGB,                //format
			GL_UNSIGNED_BYTE,      //type
			color);                //pointer to data
	}
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
void generateDrawCommands()
{
	//Generate draw commands

	GLuint baseVert = 0;
	for (unsigned int i(0); i < 100; ++i)
	{
		//quad
		if (i % 2 == 0)
		{
			vDrawCommand[i].vertexCount = 12;		//4 triangles = 12 vertices
			vDrawCommand[i].instanceCount = 1;		//Draw 1 instance
			vDrawCommand[i].firstIndex = 3;			//Draw from index 0 for this instance
			vDrawCommand[i].baseVertex = baseVert;	//Starting from baseVert
			vDrawCommand[i].baseInstance = i;		//gl_InstanceID
			baseVert += gQuad.size();
		}
		//triangle
		else
		{
			vDrawCommand[i].vertexCount = 3;		//1 triangle = 3 vertices
			vDrawCommand[i].instanceCount = 1;		//Draw 1 instance
			vDrawCommand[i].firstIndex = 0;			//Draw from index 0 for this instance
			vDrawCommand[i].baseVertex = baseVert;	//Starting from baseVert
			vDrawCommand[i].baseInstance = i;		//gl_InstanceID
			baseVert += gTriangle.size();
		}
	}
	glGenBuffers(1, &gIndirectBuffer);
	//feed the draw command data to the gpu
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gIndirectBuffer);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(vDrawCommand), vDrawCommand, GL_DYNAMIC_DRAW);
	//feed the instance id to the shader.
	//glBindBuffer(GL_ARRAY_BUFFER, gIndirectBuffer);
	
}
GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

int main()
{
#ifdef __linux
	setenv("DISPLAY", ":0", 0);
#endif

	glfwInit();
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	// enable OpenGL debug context if context allows for debug context
	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	Shader ourShader("8.1.drawIndirect.vs", "8.1.drawIndirect.fs");
	//GenerateGeometry();
	//generateDrawCommands();
	GenerateVertexsAndIndirectBuffer();
	GenerateArrayTexture();

	/*GLuint queries;
	GLuint world_time;
	glGenQueries(1, &queries);*/

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D_ARRAY, gArrayTexture);

		ourShader.use();
		glBindVertexArray(gVAO);
		//glBindBuffer(GL_ARRAY_BUFFER, gIndirectBuffer);
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gIndirectBuffer);
		//generateDrawCommands();

		//glBeginQuery(GL_TIME_ELAPSED, queries);
		//glDrawElementsIndirect GL4.0  GLES 3.0
		for (size_t i = 0; i < 100; i++)
		{
			GLuint offset = i * sizeof(DrawElementsCommand);
			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)(offset));
		}
		/*glEndQuery(GL_TIME_ELAPSED);
		glGetQueryObjectuiv(queries, GL_QUERY_RESULT, &world_time);
		std::cout << "world_time===" << world_time/1000000.0 << std::endl;*/

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	glDeleteProgram(gProgram);
	glDeleteVertexArrays(1, &gVAO);
	glDeleteBuffers(1, &gVertexBuffer);
	glDeleteBuffers(1, &gElementBuffer);
	glDeleteBuffers(1, &gMatrixBuffer);
	glDeleteBuffers(1, &gIndirectBuffer);
	glDeleteTextures(1, &gArrayTexture);
	//glDeleteQueries(1, &queries);
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}