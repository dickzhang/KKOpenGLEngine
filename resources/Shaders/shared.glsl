
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