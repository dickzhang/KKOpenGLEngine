#include "ShaderProgram.h"

#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "MaterialData.h"
#include "DirectionalLight.h"
#include "PointLight.h"

std::string ShaderData::ToString() const
{
    std::string allDefines;
    for (const ShaderDefine& define : defines)
    {
        allDefines.append(define.ToString());
    }
    allDefines.append(includes + "\n\n\n");
    std::string findStr = "#version 450 core";
    size_t mainPos = sourceCode.find(findStr) + findStr.size();
    std::string res = sourceCode;
    res.insert(mainPos, allDefines);
    return res;
}

ShaderProgram::ShaderProgram(/* args */)
{
}

unsigned int ShaderProgram::CreateShader(const char* shaderSoruce, unsigned int shaderType)
{
    unsigned int shaderId = glCreateShader(shaderType);
    glShaderSource(shaderId, 1, &shaderSoruce, NULL);
    glCompileShader(shaderId);

    int  success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    
    if(!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << '\n';
        return INVALID_SHADER_ID;
    }

    return shaderId;
}

bool ShaderProgram::Init(
    const ShaderData* vertexData,
    const ShaderData* fragmentData,
    const ShaderData* geometryData)
{
    unsigned int vertexShader = CreateShader(vertexData->ToString().c_str(), GL_VERTEX_SHADER);
    if (vertexShader == INVALID_SHADER_ID)
    {
        return false;
    }

    unsigned int fragmentShader = CreateShader(fragmentData->ToString().c_str(), GL_FRAGMENT_SHADER);
    if (fragmentShader == INVALID_SHADER_ID)
    {
        return false;
    }

    unsigned int geomertyShader = 0;
    if (geometryData != nullptr)
    {
        geomertyShader = CreateShader(geometryData->ToString().c_str(), GL_GEOMETRY_SHADER);
        if (geomertyShader == INVALID_SHADER_ID)
        {
            return false;
        }
    }

    id_ = glCreateProgram();
    glAttachShader(id_, vertexShader);
    glAttachShader(id_, fragmentShader);
    if (geometryData != nullptr)
    {
        glAttachShader(id_, geomertyShader);
    }
    glLinkProgram(id_);
    CheckForErrors();

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryData != nullptr)
    {
        glDeleteShader(geomertyShader);
    }

    return true;
}

void ShaderProgram::CheckForErrors()
{
    int  success;
    glGetProgramiv(id_, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(id_, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::CREATION_FAILED\n" << infoLog << std::endl;
    }
}

void ShaderProgram::UseProgram()
{
    glUseProgram(id_);
}

void ShaderProgram::SetUniformValue(const char* uniformName, const glm::vec4& value)
{
    int uniformLocation = glGetUniformLocation(id_, uniformName);
    glUniform4f(uniformLocation, value[0], value[1], value[2], value[3]);
}

void ShaderProgram::SetUniformValue(const char* uniformName, const glm::vec3& value)
{
    int uniformLocation = glGetUniformLocation(id_, uniformName);
    glUniform3f(uniformLocation, value[0], value[1], value[2]);
}

void ShaderProgram::SetUniformValue(const char* uniformName, int value)
{
    int uniformLocation = glGetUniformLocation(id_, uniformName);
    glUniform1i(uniformLocation, value);
}

void ShaderProgram::SetUniformValue(const char* uniformName, float value)
{
    int uniformLocation = glGetUniformLocation(id_, uniformName);
    glUniform1f(uniformLocation, value);
}

void ShaderProgram::SetUniformValue(const char* uniformName, const glm::mat4& matrix)
{
    int uniformLocation = glGetUniformLocation(id_, uniformName);
    glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::SetUniformValue(const char* uniformName, MaterialData& material)
{
    std::string baseName = uniformName;

    material.diffuseMap.BindTexture();    
    SetUniformValue((baseName + ".diffuseMap").c_str(), (int)material.diffuseMap.GetTextureUnit());
    SetUniformValue((baseName + ".specularMap").c_str(), (int)material.specularMap.GetTextureUnit());
    SetUniformValue((baseName + ".shine").c_str(), material.shine);
}

void ShaderProgram::SetUniformValue(const char* uniformName, const DirectionalLight& light)
{
    std::string baseName = uniformName;
    
    SetUniformValue((baseName + ".direction").c_str(), light.direction);
    SetUniformValue((baseName + ".ambient").c_str(), light.ambient);
    SetUniformValue((baseName + ".diffuse").c_str(), light.diffuse);
    SetUniformValue((baseName + ".specular").c_str(), light.specular);
}

void ShaderProgram::SetUniformValue(const char* uniformName, const PointLight& light)
{
    std::string baseName = uniformName;
    
    SetUniformValue((baseName + ".position").c_str(), light.position);

    SetUniformValue((baseName + ".constant").c_str(), light.constant);
    SetUniformValue((baseName + ".linear").c_str(), light.linear);
    SetUniformValue((baseName + ".quadratic").c_str(), light.quadratic);

    SetUniformValue((baseName + ".ambient").c_str(), light.ambient);
    SetUniformValue((baseName + ".diffuse").c_str(), light.diffuse);
    SetUniformValue((baseName + ".specular").c_str(), light.specular);

    SetUniformValue((baseName + ".radius").c_str(), light.radius);
}

void ShaderProgram::SetUniformBuffer(const char* uniformBufferName, int binding)
{
    unsigned int uboIndex = glGetUniformBlockIndex(id_, uniformBufferName);   
    glUniformBlockBinding(id_, uboIndex, binding);
}

void ShaderProgram::SetStorageBuffer(const char* storageBufferName, int binding)
{
    unsigned int uboIndex = glGetProgramResourceIndex(id_, GL_SHADER_STORAGE_BLOCK, storageBufferName);
    glShaderStorageBlockBinding(id_, uboIndex, binding);
}

ShaderProgram::~ShaderProgram()
{
}
