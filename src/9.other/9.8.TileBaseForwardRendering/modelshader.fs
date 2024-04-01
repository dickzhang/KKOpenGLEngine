#version 450 core

#define TILE_SIZE 16
#define LIGHTS_PER_TILE 63
#define LIGHT_ID_END -2

uniform sampler2D texture_diffuse;
uniform vec3 viewPos;
uniform int numOfTilesX;

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
out vec4 FragColor;

layout(std430, binding = 1) readonly buffer LightBuffer 
{
    PointLight data[];
} lightBuffer;

layout(std430, binding = 2) readonly buffer VisibleLightIndicesBuffer 
{
    int data[];
} visibleLightIndicesBuffer;

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

void main() 
{
    vec3 objectColor = vec3(texture(texture_diffuse, TexCoords));
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPosWorldSpace);
    vec3 result= AddPointLights(normal, viewDir, objectColor);
    FragColor = vec4(result, 1.0);
}