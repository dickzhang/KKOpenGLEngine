#pragma once
class GLFWwindow;

class Window
{
public:
    static Window& Get();
    
    bool WindowShouldClose() const;
    void SwapBuffers();
    GLFWwindow* GetGLFWwindow() const { return window_; }
    float GetWidth() const { return width_; }
    float GetHeight() const { return height_; }
    void HandleResizeEvent();

    ~Window();
private:

    Window();
    bool Init(int windowWidth, int windowHeight);

    static Window* window;

    inline bool IsWindowResized() const;

    GLFWwindow* window_;

    float width_;
    float height_;

};