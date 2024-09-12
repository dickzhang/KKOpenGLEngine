#version 430 core

uniform sampler2D texture_diffuse;
uniform vec3 viewPos;
uniform float zNear;
uniform float zFar;
uniform float scale;
uniform float bias;
uniform vec4 tileSizes;

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
struct LightGrid{
    uint offset;
    uint count;
};
in vec3 Normal;
in vec3 FragPosWorldSpace;
in vec2 TexCoords;
out vec4 FragColor;

layout(std430, binding = 2) buffer lightSSBO
{
    PointLight pointLight[];
};

layout(std430, binding = 3) buffer lightIndexSSBO
{
    uint globalLightIndexList [];
};

layout(std430, binding = 4) buffer lightGridSSBO
{
    LightGrid lightGrid [];
};

float linearDepth(float depthSample){
    float depthRange = 2.0 * depthSample - 1.0;
    // Near... Far... wherever you are...
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}

float attenuate(vec3 lightDirection, float radius)
{
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
    float specStrenght = pow(max(0, dot(normal, halfway)), 32);
    vec3 specular = vec3(light.specular) * specStrenght * objectColor;
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}

void main() 
{
    vec3 objectColor = vec3(texture(texture_diffuse, TexCoords));
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPosWorldSpace);

//Locating which cluster you are a part of
    uvec4 tilePage=uvec4(tileSizes);
    uint zTile     = uint(max(log2(linearDepth(gl_FragCoord.z)) * scale + bias, 0.0));
    uvec3 tiles    = uvec3( uvec2( gl_FragCoord.xy / tilePage[3] ), zTile);
    uint tileIndex = tiles.x +tilePage.x * tiles.y +
                (tilePage.x * tilePage.y) * tiles.z;  

// Point lights
    uint lightCount       = lightGrid[tileIndex].count;
    uint lightIndexOffset = lightGrid[tileIndex].offset;
    vec3 resultColor;
    for(uint i = 0; i < lightCount; i++){
        uint lightVectorIndex = globalLightIndexList[lightIndexOffset + i];
        resultColor+= CalcPointLight(pointLight[lightVectorIndex],normal, viewDir, objectColor);
    }
    resultColor+=vec3(0.3,0.3,0.3);
    resultColor*=objectColor;
    FragColor = vec4(resultColor, 1.0);
}