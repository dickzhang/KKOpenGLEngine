#pragma once

#include <climits>
#include <vector>
#include <string>
#include <optional>
#include <glm/glm.hpp>

struct MaterialData;
struct LightProperties;
struct DirectionalLight;
struct PointLight;

struct ShaderDefine
{
    ShaderDefine(const std::string& name, const std::string& value) : variableName(name), variableValue(value) {}
    std::string variableName;
    std::string variableValue;
    std::string ToString() const { return "\n#define " + variableName + " " + variableValue + "\n"; }
};

struct ShaderData
{
    std::string includes;
    std::string sourceCode;
    std::vector<ShaderDefine> defines;
    std::string ToString() const;
};

class ShaderProgram
{
public:
    ShaderProgram();
    ~ShaderProgram();

    bool Init(
        const ShaderData* vertexData,
        const ShaderData* fragmentData,
        const ShaderData* geometryData = nullptr);
    void CheckForErrors();
    void UseProgram();
    void SetUniformValue(const char* uniformName, const glm::vec4& value);
    void SetUniformValue(const char* uniformName, const glm::vec3& value);
    void SetUniformValue(const char* uniformName, int value);
    void SetUniformValue(const char* uniformName, float value);
    void SetUniformValue(const char* uniformName, const glm::mat4& matrix);
    void SetUniformValue(const char* uniformName, MaterialData& material);
    void SetUniformValue(const char* uniformName, const DirectionalLight& light);
    void SetUniformValue(const char* uniformName, const PointLight& light);
    void SetUniformBuffer(const char* uniformBufferName, int binding);
    void SetStorageBuffer(const char* storageBufferName, int binding);

    template<typename T>
    void SetUniformValue(const char* uniformName, const std::vector<T>& lights);

protected:

    unsigned int CreateShader(const char* shaderSoruce, unsigned int shaderType);

    static constexpr unsigned int INVALID_SHADER_ID = UINT_MAX;
    unsigned int id_;
};

template<typename T>
void ShaderProgram::SetUniformValue(const char* uniformName, const std::vector<T>& values)
{
    for (int i = 0; i < values.size(); i++)
    {
        std::string name = uniformName;
        name.append("[").append(std::to_string(i)).append("]");
        SetUniformValue(name.c_str(), values[i]);
    }
}