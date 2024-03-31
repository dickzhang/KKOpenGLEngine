#version 450 core

// --------------------------------
// These defines come from the runtime, the comments are here to address the IDE errors
//!#define NUM_CSM_PLANES 5
//!#define PointLight int
//!#define TILE_SIZE 1
//!#define LIGHTS_PER_TILE 1
//!#define USE_LIGHT_CULLING
//!#define LIGHT_ID_END -2
// --------------------------------


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

in vec3 Normal;
in vec3 FragPosWorldSpace;
in vec2 TexCoords;
in vec4 FragPosViewSpace;

out vec4 FragColor;

uniform mat4 lightSpaceMatrices[NUM_CSM_PLANES];

layout(std430, binding = 1) readonly buffer LightBuffer {
    PointLight data[];
} lightBuffer;

#ifdef USE_LIGHT_CULLING
layout(std430, binding = 2) readonly buffer VisibleLightIndicesBuffer {
    int data[];
} visibleLightIndicesBuffer;
#endif

uniform float cascadePlaneDistances[NUM_CSM_PLANES - 1];
uniform MaterialData material;
uniform DirLight light;
uniform sampler2DArray shadowMap;
uniform vec3 viewPos;
uniform float farPlane;

#ifdef USE_LIGHT_CULLING
uniform int numOfTilesX;
#else
uniform int numLights;
#endif

int GetShadowMapLayer(int cascadeCount)
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
    int layer = GetShadowMapLayer(cascadeCount);
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

float attenuate(vec3 lightDirection, float radius) {
	float cutoff = 0.5;
	float attenuation = dot(lightDirection, lightDirection) / (100.0 * radius);
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);

	return clamp(attenuation, 0.0, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 objectColor)
{
    float distance = length(vec3(light.position) - FragPosWorldSpace);

    if (distance > light.radius)
    {
        return vec3(0.0);
    }

    vec3 ambient = vec3(light.ambient) * objectColor;

    vec3 lightDir = normalize(vec3(light.position) - FragPosWorldSpace);
    float diffuseStrength = max(dot(lightDir, normal), 0);
    vec3 diffuse = vec3(light.diffuse)* diffuseStrength * objectColor;
    
    vec3 halfway = normalize(lightDir + viewDir);
    float specStrenght = pow(max(0, dot(normal, halfway)), material.shine);
    vec3 specularHighlight = vec3(texture(material.specularMap, TexCoords));
    vec3 specular = vec3(light.specular) * specStrenght * objectColor * specularHighlight;
 

    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

#ifdef USE_LIGHT_CULLING
vec3 AddPointLights(vec3 normal, vec3 viewDir, vec3 objectColor)
{
   ivec2 location = ivec2(gl_FragCoord.xy);
   ivec2 tileId = location / ivec2(TILE_SIZE, TILE_SIZE);
   int offset = tileId.y * numOfTilesX + tileId.x;

   int indexStart = offset * LIGHTS_PER_TILE;
   int indexEnd = indexStart + LIGHTS_PER_TILE;
   int index = indexStart;
   int lightId = visibleLightIndicesBuffer.data[index];

   vec3 result = vec3(0.0);
   while (lightId != LIGHT_ID_END && index < indexEnd)
   {
       result += CalcPointLight(lightBuffer.data[lightId], normal, viewDir, objectColor);
       lightId = visibleLightIndicesBuffer.data[++index];
   }

   return result;
}
#else
vec3 AddPointLights(vec3 normal, vec3 viewDir, vec3 objectColor)
{
   vec3 result = vec3(0.0);

   for (int i = 0; i < numLights; i++)
   {
       result += CalcPointLight(lightBuffer.data[i], normal, viewDir, objectColor);
   }

   return result;
}
#endif

void main() 
{
    vec3 objectColor = vec3(texture(material.diffuseMap, TexCoords));
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPosWorldSpace);

    vec3 result = CalcDirectionalLight(normal, viewDir, objectColor);

    result += AddPointLights(normal, viewDir, objectColor);

    FragColor = vec4(result, 1.0);
}