//// Based on Ferran Soles: https://github.com/fsole/GLSamples/blob/master/src/MultidrawIndirect.cpp
//https://litasa.github.io/blog/2017/09/04/OpenGL-MultiDrawIndirect-with-Individual-Textures

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <learnopengl/shader_m.h>

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

	//设置位移
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
	GLuint gUBOBuffer(0);
	GLuint gMatrixBuffer(0);
	GLuint gProgram(0);
	GLuint gShaderID(0);
}//Unnamed namespace

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




	//EBO
	int triangle_bytes = sizeof(unsigned int) * gTriangleIndex.size();
	int quad_bytes = sizeof(unsigned int) * gQuadIndex.size();
	glGenBuffers(1, &gElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangle_bytes + quad_bytes, NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, quad_bytes, gQuadIndex.data());
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, quad_bytes, triangle_bytes, gTriangleIndex.data());
	
	GLuint ubo_index = glGetUniformBlockIndex(gShaderID, "UBOInfo");
	GLchar* maxName = new GLchar[ 7+ 1];//-------------------
	glGetActiveUniformBlockName(gShaderID, ubo_index, 7 + 1, nullptr, maxName);
	//maxName UBOInfo
	delete[]maxName;



	glUniformBlockBinding(gShaderID, ubo_index, 0);

	GLint binding;
	glGetActiveUniformBlockiv(gShaderID, ubo_index, GL_UNIFORM_BLOCK_BINDING, &binding);

	GLint nameLength;//最大名字多长
	glGetActiveUniformBlockiv(gShaderID, ubo_index, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);
	


	GLint activeNum;
	glGetActiveUniformBlockiv(gShaderID, ubo_index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeNum);

	GLint*indices=new GLint[activeNum];
	glGetActiveUniformBlockiv(gShaderID, ubo_index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices);
	for (int i = 0; i < activeNum; i++)
	{
		printf("%d \n", indices[i]);
	}
	delete []indices;
	GLint nBlockDataSize = 0;
	{

		glGetActiveUniformBlockiv(gShaderID, ubo_index, GL_UNIFORM_BLOCK_DATA_SIZE, &nBlockDataSize);
		printf("%d ", nBlockDataSize);
		const int NumberOfUniform = 4;//activeNum
		//建立变量名称数组
		const char* names[NumberOfUniform] =
		{
			"UBOInfo.testBind",
			"UBOInfo.testMatrix",
			"UBOInfo.testVec4",
			"UBOInfo.testVec3",
		};
		GLuint indices[NumberOfUniform];

		GLint size[NumberOfUniform];
		GLint offset[NumberOfUniform];
		GLint type[NumberOfUniform];
		glGetUniformIndices(gShaderID, NumberOfUniform, names, indices);

		glGetActiveUniformsiv(gShaderID, NumberOfUniform, indices, GL_UNIFORM_OFFSET, offset);
		glGetActiveUniformsiv(gShaderID, NumberOfUniform, indices, GL_UNIFORM_SIZE, size);
		glGetActiveUniformsiv(gShaderID, NumberOfUniform, indices, GL_UNIFORM_TYPE, type);

		for (GLuint i = 0; i < NumberOfUniform; i++) {
			printf("\r\n-------------------------\r\n");
			printf("uniform name  = %s \r\n", names[i]);
			printf("uniform indices = %d \r\n", indices[i]);
			printf("uniform offset = %d\r\n", offset[i]);
			printf("uniform size = %d\r\n", size[i]);
			printf("uniform type = 0x%x\r\n", type[i]);
		}


	}
	//UBO
	bool enable = true;
	glGenBuffers(1, &gUBOBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, gUBOBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, gUBOBuffer);
	glBufferData(GL_UNIFORM_BUFFER, nBlockDataSize, NULL, GL_STATIC_DRAW); // 分配150个字节的内存空间
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(bool), &enable);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
}

void GenerateArrayTexture()
{
	glGenTextures(1, &gArrayTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, gArrayTexture);
	// GL4.0的API
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
	GLubyte red[3] = {255,0,0};
	GLubyte green[3] = {0,255,0};
	GLubyte blue[3] = {0,0,255 };
	for (unsigned int i(0); i != 100; ++i)
	{

		//Choose a random color for the i-essim image
		int randi = i % 3;
		GLubyte color[3];
		
		switch (i)
		{
		case 0:
		{
			memcpy(color, red, sizeof(GLubyte) * 3);
			break;
		}
		case 1:
		{
			memcpy(color, green, sizeof(GLubyte) * 3);
			break;
		}
		{
			memcpy(color, blue, sizeof(GLubyte) * 3);
			break;
		}
		default:
			break;
		}
		
		

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
DrawElementsCommand vDrawCommand[100];
void generateDrawCommands()
{
	//Generate draw commands
	
	GLuint baseVert = 0;
	for (unsigned int i(0); i < 100; ++i)
	{
		//quad
		if (i % 2 == 0)
		{
			vDrawCommand[i].vertexCount = 12;		//4 triangles 12个index
			vDrawCommand[i].instanceCount = 1;		//Draw 1 instance  instanceID
			vDrawCommand[i].firstIndex = 0;			//
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
	//glEnableVertexAttribArray(2);
	//glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(DrawElementsCommand), (void*)(offsetof(DrawElementsCommand, baseInstance)));
	//glVertexAttribDivisor(2, 1); //only once per instance
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
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
	gShaderID = ourShader.ID;
	GenerateGeometry();
	generateDrawCommands();
	GenerateArrayTexture();

	/*GLuint queries;
	GLuint world_time;
	glGenQueries(1, &queries);*/
	{
		GLint blocks = 0;
		// 84
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &blocks);
		printf("GL_MAX_UNIFORM_BUFFER_BINDINGS = %i\n", blocks);
		
		
	}
	//glGetUniformIndices
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
		//glBindBuffer(GL_ARRAY_BUFFER, 
		// );
		//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
		//generateDrawCommands();
	
		//glBeginQuery(GL_TIME_ELAPSED, queries);
		//glDrawElementsIndirect GL4.0  GLES 3.0
		for (size_t i = 0; i < 100; i++)
		{
			GLuint offset = i* sizeof(DrawElementsCommand);
			//GLuint offset1 = vDrawCommand[i].baseInstance * sizeof(DrawElementsCommand);
			
			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)(offset));
			//glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (const void*)(vDrawCommand[i].baseInstance * sizeof(DrawElementsCommand)));
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

class MeshManager {



};



