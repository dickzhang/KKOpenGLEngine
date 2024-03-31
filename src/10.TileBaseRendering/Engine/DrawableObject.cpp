#include "DrawableObject.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ShaderProgram.h"


DrawableObject::DrawableObject()
: model_(1.f)
, view_(1.f)
, proj_(1.f)
, viewProj_(1.f)
, modelViewProj_(1.f)
{

}

DrawableObject::DrawableObject(const DrawableObject& drawableObject)
{
    Copy(drawableObject);
}

DrawableObject& DrawableObject::operator=(const DrawableObject& drawableObject)
{
    Copy(drawableObject);
    return *this;
}

DrawableObject::~DrawableObject()
{
}

void DrawableObject::SetModel(const glm::mat4& model)
{
    model_ = model;
}

void DrawableObject::SetView(const glm::mat4& view)
{
    view_ = view;
}

void DrawableObject::SetProj(const glm::mat4& proj)
{
    proj_ = proj;
}

void DrawableObject::CalculateMVP()
{
    modelViewProj_ = viewProj_ * model_;
}

void DrawableObject::CalculateViewProj()
{
    viewProj_ = proj_ * view_;
}

void DrawableObject::Copy(const DrawableObject& drawableObject)
{
    this->vao_ = drawableObject.vao_;
    this->vbo_ = drawableObject.vbo_;
    this->ibo_ = drawableObject.ibo_;
    this->model_ = drawableObject.model_;
    this->view_ = drawableObject.view_;
    this->proj_ = drawableObject.proj_;
    this->viewProj_ = drawableObject.viewProj_;
    this->modelViewProj_ = drawableObject.modelViewProj_;
}

void DrawableObject::Translate(const glm::vec3& translation)
{
    model_ = glm::translate(model_, translation);
}

void DrawableObject::Scale(const glm::vec3& scale)
{
    model_ = glm::scale(model_, scale);
}

void DrawableObject::SetupMatrices(ShaderProgram& shader)
{
    CalculateViewProj();
    CalculateMVP();
    shader.SetUniformValue("modelViewProj", modelViewProj_);
    shader.SetUniformValue("viewProj", viewProj_);
    shader.SetUniformValue("model", model_);
}