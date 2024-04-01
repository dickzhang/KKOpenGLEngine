#pragma once

class Renderbuffer
{
public:
    Renderbuffer();

    void Init(unsigned int width, unsigned int height, unsigned int storageType);
    void Bind();
    unsigned int GetId() const { return id_; }

    ~Renderbuffer();

private:
    unsigned int id_;
};