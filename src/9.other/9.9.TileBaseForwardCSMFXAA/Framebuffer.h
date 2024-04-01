#pragma once

class Framebuffer
{
public:
    Framebuffer();

    void Init(bool isDepthOnly = false);
    void Bind();
    void AttachTexture(unsigned int attachmentType, unsigned int textureId, unsigned int textureTarget = 0);
    void AttachRenderbuffer(unsigned int attachmentType, unsigned int renderbufferId);
    bool IsComplete() const;

    static void BindDefault();

    ~Framebuffer();

private:

    unsigned int id_;

};