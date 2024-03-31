#version 450 core

//! #define NUM_CSM_PLANES 5

struct MaterialData
{
    sampler2D diffuseMap;
    sampler2D specularMap;
    float shine;
};

struct DirLight
{
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 

struct PointLight
{    
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float constant;
    float linear;
    float quadratic;  
    float radius;
}; 

in vec3 Normal;
in vec3 FragPosWorldSpace;
in vec2 TexCoords;
in vec4 FragPosViewSpace;

out vec4 FragColor;

// NUM_CSM_PLANES is a static define that needs to be supplied to the shader
// via the ShaderData struct before building the shader itself
layout (std140, binding = 0) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[NUM_CSM_PLANES];
};

layout(std430, binding = 1) buffer LightBuffer {
	PointLight data[];
} lightBuffer;

uniform float cascadePlaneDistances[NUM_CSM_PLANES - 1];
uniform MaterialData material;
uniform DirLight light;
uniform sampler2DArray shadowMap;
uniform vec3 viewPos;
uniform float farPlane;
uniform int numLights;

int getShadowMapLayer(int cascadeCount)
{
    float depth = abs(FragPosViewSpace.z);
    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depth < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }

    if (layer == -1)
    {
        layer = cascadeCount;
    }

    return layer;
}

float ShadowCalculation(vec3 lightDir, vec3 normal)
{
    int cascadeCount = NUM_CSM_PLANES - 1; 
    int layer = getShadowMapLayer(cascadeCount);
    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(FragPosWorldSpace, 1.0);

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
    {
        return 0.0;
    }

    float currentDepth = projCoords.z; 
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    if (layer == cascadeCount)
    {
        bias *= 1 / (farPlane * 0.5f);
    }
    else
    {
        bias *= 1 / (cascadePlaneDistances[layer] * 0.5f);
    }

    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r; 
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    return shadow;
}

vec3 CalcDirectionalLight(vec3 normal, vec3 viewDir, vec3 objectColor)
{
    vec3 ambient = objectColor * light.ambient;

    vec3 lightDir = normalize(light.direction);
    float diffuseStrength = max(dot(lightDir, normal), 0);
    vec3 diffuse = light.diffuse * diffuseStrength * objectColor;

    vec3 halfway = normalize(lightDir + viewDir);
    float specStrenght = pow(max(0, dot(normal, halfway)), material.shine);
    vec3 specularHighlight = vec3(texture(material.specularMap, TexCoords));
    vec3 specular = light.specular * specStrenght * objectColor * specularHighlight;

    float shadow = ShadowCalculation(lightDir, normal);
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 objectColor)
{
    float distance = length(light.position.xyz - FragPosWorldSpace);

    if (distance > light.radius)
    {
        return vec3(0.0);
    }
    
    float attenuation =  1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    vec3 ambient = light.ambient.xyz * objectColor;

    vec3 lightDir = normalize(light.position.xyz - FragPosWorldSpace);
    float diffuseStrength = max(dot(lightDir, normal), 0);
    vec3 diffuse = light.diffuse.xyz * diffuseStrength * objectColor;

    vec3 halfway = normalize(lightDir + viewDir);
    float specStrenght = pow(max(0, dot(normal, halfway)), material.shine);
    vec3 specularHighlight = vec3(texture(material.specularMap, TexCoords));
    vec3 specular = light.specular.xyz * specStrenght * objectColor * specularHighlight;



    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

void main() 
{
    vec3 objectColor = vec3(texture(material.diffuseMap, TexCoords));
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPosWorldSpace);

    vec3 result = vec3(0); //CalcDirectionalLight(normal, viewDir, objectColor);

    for (int i = 0; i < numLights; i++)
    {
        result += CalcPointLight(lightBuffer.data[i], normal, viewDir, objectColor);
    }

    FragColor = vec4(result, 1.0);
}