#pragma once

class FBO
{
public:
    FBO();
    void Init(bool isDepthOnly = false);
    void Bind();
    void AttachTexture(unsigned int attachmentType,unsigned int textureId,unsigned int textureTarget = 0);
    void AttachRenderbuffer(unsigned int attachmentType,unsigned int renderbufferId);
    bool IsComplete() const;
    static void BindDefault();
    ~FBO();
private:
    unsigned int id_;
};