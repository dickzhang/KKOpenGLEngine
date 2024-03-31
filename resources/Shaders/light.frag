#version 450 core

struct MaterialData
{
    sampler2D diffuseMap;
    sampler2D specularMap;
    float shine;
};

struct LightProperties {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

#define NR_POINT_LIGHTS 4  

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform MaterialData material;
uniform DirLight light;
uniform PointLight pointLights[NR_POINT_LIGHTS];

vec3 CalcDirectionalLight(DirLight light, vec3 normal, vec3 viewDir, vec3 objectColor);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 objectColor);

void main() 
{
    // vec3 objectColor = vec3(texture(material.diffuseMap, TexCoords));

    // vec3 ambient = objectColor * light.ambient;

    // vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(light.direction - FragPos);
    // float diffuseStrength = max(dot(lightDir, norm), 0);
    // vec3 diffuse = light.diffuse * (diffuseStrength * objectColor);

    // vec3 viewDir = normalize(-FragPos);
    // vec3 lightReflect = reflect(-lightDir, norm);
    // float specStrenght = pow(max(0, dot(viewDir, lightReflect)), material.shine);
    // vec3 specularHighlight = vec3(texture(material.specularMap, TexCoords));
    // vec3 specular = specularHighlight * specStrenght * light.specular;
    // FragColor = vec4(ambient + diffuse + specular, 1.0);

    vec3 objectColor = vec3(texture(material.diffuseMap, TexCoords));
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(-FragPos);

    vec3 result = CalcDirectionalLight(light, normal, viewDir, objectColor);

    for (int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        result += CalcPointLight(pointLights[i], normal, viewDir, objectColor);
    }

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirectionalLight(DirLight light, vec3 normal, vec3 viewDir, vec3 objectColor)
{
    vec3 ambient = objectColor * light.ambient;

    vec3 lightDir = normalize(-light.direction);
    float diffuseStrength = max(dot(lightDir, normal), 0);
    vec3 diffuse = light.diffuse * diffuseStrength * objectColor;

    vec3 lightReflect = reflect(-lightDir, normal);
    float specStrenght = pow(max(0, dot(viewDir, lightReflect)), material.shine);
    vec3 specularHighlight = vec3(texture(material.specularMap, TexCoords));
    vec3 specular = light.specular * specStrenght * specularHighlight;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 objectColor)
{
    vec3 ambient = objectColor * light.ambient;

    vec3 lightDir = normalize(light.position - FragPos);
    float diffuseStrength = max(dot(lightDir, normal), 0);
    vec3 diffuse = light.diffuse * diffuseStrength * objectColor;

    vec3 lightReflect = reflect(-lightDir, normal);
    float specStrenght = pow(max(0, dot(viewDir, lightReflect)), material.shine);
    vec3 specularHighlight = vec3(texture(material.specularMap, TexCoords));
    vec3 specular = light.specular * specStrenght * specularHighlight;
    
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
} 