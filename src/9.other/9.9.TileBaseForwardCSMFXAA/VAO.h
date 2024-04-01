#pragma once

class VAO
{
public:
    VAO();
    
    void Init();
    void Bind();

    static void Unbid();
    bool IsInitialised() const { return id_ != 0; }

    ~VAO();
private:
    unsigned int id_ = 0;
};