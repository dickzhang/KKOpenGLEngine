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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

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

#define FIX_SCREEN_SIZE 1
#define USE_GLES    1
struct RenderWindow
{
    GLFWwindow* window{ nullptr };
    GLFWmonitor* monitor{ nullptr };
    unsigned int quadVAO{ 0 }, quadVBO{ 0 };
    int width{ SCR_WIDTH }, height{ SCR_HEIGHT };
    unsigned int framebuffer{ 0 };
    unsigned int textureColorbuffer{ 0 };
    unsigned int rbo{ 0 };

};

std::vector<RenderWindow> mRenderWindowList;

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

void CreateScreenGL(RenderWindow& window)
{
    // screen quad VAO
    //unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &window.quadVAO);
    glGenBuffers(1, &window.quadVBO);
    glBindVertexArray(window.quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, window.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void DestroyScreenGL(RenderWindow& window)
{
    glDeleteVertexArrays(1, &window.quadVAO);
    window.quadVAO = 0;
    glDeleteBuffers(1, &window.quadVBO);
    window.quadVBO = 0;
}

void DestroyRenderWindow(RenderWindow& win)
{
    glDeleteTextures(1, &win.textureColorbuffer);
    glDeleteRenderbuffers(1, &win.rbo);
    glDeleteFramebuffers(1, &win.framebuffer);
    DestroyScreenGL(win);
    glfwDestroyWindow(win.window);
}

void CreateRenderFrameBuffer(RenderWindow& win)
{
    //unsigned int framebuffer;
    glGenFramebuffers(1, &win.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, win.framebuffer);
    // create a color attachment texture
    //unsigned int textureColorbuffer;
    glGenTextures(1, &win.textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, win.textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win.width, win.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, win.textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
   
    glGenRenderbuffers(1, &win.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, win.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, win.width, win.height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, win.rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
int main()
{
#ifdef __linux
    setenv("DISPLAY", ":0", 0);
#endif
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
#if USE_GLES
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    //配置创建EGLcontext
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


    int count;
    int fps = 0 ;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    std::cout << "Moitors Count:" << count << std::endl;

#if 0
    int maxMonitorCount = count;
#else
    int maxMonitorCount = 2;
    //4: 108
    //3: 125
    //2: 185
    //1: 1170
#endif
    //GLFWmonitor* usingMonitor = nullptr;
    for (int i = 0; i < count && i < maxMonitorCount; i++)
    {
        auto mo = monitors[i];
        if (mo)
        {
            const char* name = glfwGetMonitorName(mo);
            std::cout << "Moitors i:" << i << " name: " << name << std::endl;
            //usingMonitor = mo;
            mRenderWindowList.push_back({nullptr, mo});
        }
    }

    GLFWwindow* mainWindow{ nullptr };
    int idx = 0;
    for (auto& info : mRenderWindowList)
    {
        // glfw window creation
        // --------------------
        char title[100]{ 0 };
        GLFWwindow* window = nullptr;
        if (idx == 0)
        {
            sprintf(title, "LearnOpenGL_%d", idx);
            window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title, NULL, mainWindow);
            info.width = SCR_WIDTH;
            info.height = SCR_HEIGHT;
            if (window == NULL)
            {
                std::cout << "Failed to create GLFW window" << std::endl;
                glfwTerminate();
                return -1;
            }
        }
        else
        {
#if !FIX_SCREEN_SIZE
            const GLFWvidmode* mode = glfwGetVideoMode(info.monitor);

            glfwWindowHint(GLFW_RED_BITS, mode->redBits);
            glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
            glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
            glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
            //主窗口获取焦点后不消失
            glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

            info.width = mode->width;
            info.height = mode->height;

            window = glfwCreateWindow(mode->width, mode->height, title, info.monitor, mainWindow);
#else
            info.width = SCR_WIDTH;
            info.height = SCR_HEIGHT;

            window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title, NULL, mainWindow);
#endif
        }

        if (!mainWindow)
        {
            mainWindow = window;
        }

        glfwMakeContextCurrent(window);

        if (idx == 0)
        {
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            {
                std::cout << "Failed to initialize GLAD" << std::endl;
                return -1;
            }
        }

        int m_Major, m_Minor;
        glGetIntegerv(GL_MAJOR_VERSION, &m_Major);
        glGetIntegerv(GL_MINOR_VERSION, &m_Minor);
        const GLubyte* version = glGetString(GL_VERSION); //返回当前OpenGL实现的版本号
        std::cout << "GL Version: " << version << ", Major: " << m_Major << ", Minor: " << m_Minor << std::endl;
        CreateScreenGL(info);

        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        //glfwSetCursorPosCallback(window, mouse_callback);
        //glfwSetScrollCallback(window, scroll_callback);

        // tell GLFW to capture our mouse
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        info.window = window;

        idx++;
    }

    glfwMakeContextCurrent(mRenderWindowList[0].window);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
#if USE_GLES
    Shader shader("5.1.framebuffers_es.vs", "5.1.framebuffers_es.fs");
    Shader screenShader("5.1.framebuffers_screen_es.vs", "5.1.framebuffers_screen_es.fs");
#else
    Shader shader("5.1.framebuffers.vs", "5.1.framebuffers.fs");
    Shader screenShader("5.1.framebuffers_screen.vs", "5.1.framebuffers_screen.fs");
#endif

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
    float planeVertices[] = {
        // positions          // texture Coords 
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };

    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    for (auto& win : mRenderWindowList)
    {
        CreateRenderFrameBuffer(win);
    }
    // load textures
    // -------------
    unsigned int cubeTexture = loadTexture(FileSystem::getPath("resources/textures/container.jpg").c_str());
    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/metal.png").c_str());

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture1", 0);

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    // framebuffer configuration
    // -------------------------


    // draw as wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    bool mainWindowClose = false;
    // render loop
    // -----------
    double lastFPSTime = glfwGetTime();
    while (!mainWindowClose && !mRenderWindowList.empty())
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetTime() - lastFPSTime > 1.0)
        {
            std::cout << "FPS: " << fps << std::endl;
            fps = 0;
            lastFPSTime = glfwGetTime();
        }

        fps++;
        

        for (auto it = mRenderWindowList.begin(); it != mRenderWindowList.end();)
        {
            if (glfwWindowShouldClose(it->window))
            {
                if (it->window == mainWindow)
                {
                    //主窗口关闭所有都关闭
                    mainWindowClose = true;
                    break;
                }
                DestroyRenderWindow(*it);
                it = mRenderWindowList.erase(it);
                continue;
            }
            // input
            // -----
            processInput(it->window);
            it++;
           
        }

        if (mainWindowClose)break;

        glfwMakeContextCurrent(mRenderWindowList[0].window);

        for (int i = 0; i < mRenderWindowList.size(); i++)
        {
            auto& win = mRenderWindowList[i];

            if (win.width == 0 || win.height == 0)continue;

            // render
            // ------
            // bind to framebuffer and draw scene as we normally would to color texture 
            glBindFramebuffer(GL_FRAMEBUFFER, win.framebuffer);
            glViewport(0, 0, win.width, win.height);
            glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

            // make sure we clear the framebuffer's content
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.use();
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom + i * 20), (float)win.width / (float)win.height, 0.1f, 100.0f);
            shader.setMat4("view", view);
            shader.setMat4("projection", projection);
            // cubes
            glBindVertexArray(cubeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubeTexture);
            model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // floor
            glBindVertexArray(planeVAO);
            glBindTexture(GL_TEXTURE_2D, floorTexture);
            shader.setMat4("model", glm::mat4(1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }

        for (int i = 0; i < mRenderWindowList.size(); i++)
        {
            auto& win = mRenderWindowList[i];
            if (win.width == 0 || win.height == 0)continue;

            glfwMakeContextCurrent(win.window);
            // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, win.width, win.height);
            glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
            // clear all relevant buffers
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
            glClear(GL_COLOR_BUFFER_BIT);

            screenShader.use();
            glBindVertexArray(win.quadVAO);
            glBindTexture(GL_TEXTURE_2D, win.textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glfwSwapInterval(0);
            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(win.window);
        }

        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &planeVAO);

    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);



    for (auto& win : mRenderWindowList)
    {
        DestroyRenderWindow(win);
    }
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
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
    //glViewport(0, 0, width, height);
#if !FIX_SCREEN_SIZE
    for (auto& info : mRenderWindowList)
    {
        if (info.window == window)
        {
            info.width = width;
            info.height = height;
        }
    }
#endif
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    //return;
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
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
