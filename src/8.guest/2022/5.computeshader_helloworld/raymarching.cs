#version 450
#define MAX_STEPS 512
#define MIN_DIST .000001
#define REFLECTIVE 1.0
#define MATTE      0.0

uniform uint workgroups;

const uint TILE_W = workgroups;
const uint TILE_H = workgroups;
const ivec2 TILE_SIZE = ivec2(TILE_W, TILE_H);
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

struct Camera
{
    vec4 pos;
    vec4 dir;
    vec4 yAxis;
    vec4 xAxis;
};

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct Ray
{
    vec3 origin;
    vec3 dir;
};

struct RayHit
{
    float hitpoint;
    vec3 color;
    int id;
    float material;
};

// SDFs and Soft shadows by Inigo quilez https://www.iquilezles.org/www/index.htm
float sdSphere(vec3 p, float r);
RayHit sdf(vec3 pos);
RayHit RayMarch(vec3 rayOrigin, vec3 rayDir);
RayHit reflectedRay(vec3 rayOrigin, vec3 rayDir);
vec3 render(vec3 rayOrigin, vec3 rayDir);
vec3 GetNormal(vec3 pos);
float sdPlane(vec3 p, vec4 n);
vec3 getPointLight(vec3 color, vec3 normal, vec3 pos); // https://learnopengl.com/Lighting/Light-casters
vec3 bounce(vec3 rayDir, vec3 pos, vec3 normal, vec3 color, RayHit primaryObject);
float softshadow(vec3 ro, vec3 rd, float k);
vec3 checkers(vec3 p);

uniform bool AA;
uniform int bounceVar; //number of bouncing rays
uniform float drand48; // testing
uniform float iTime; // Time since glfw was initialized
uniform vec3 mouse; // Calculated Euler Angles
uniform vec2 iMouse; // Carries the current pixel location of the mouse cursor
uniform Camera camera; // Camera rotation and movement values
uniform Light light;

Ray castRay(vec2 uv)
{
    float Perpective = 1;//cos()radians(45);
    vec4 dir = normalize(uv.x * camera.xAxis + uv.y * camera.yAxis + camera.dir * Perpective - camera.pos);

    return Ray(camera.pos.xyz, dir.xyz);
}

// https://www.shadertoy.com/view/3df3DH
vec3 checkers(vec3 p)
{
    return int(1000.0 + p.x) % 2 != int(1000.0 + p.z) % 2 ? vec3(1.0) : vec3(0.5);
}


float sdSphere(vec3 p, float r) { return length(p) - r; }

float sdPlane(vec3 p, vec4 n) { return dot(p, n.xyz) + n.w; }

float sdBox(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float sdTorus(vec3 p, vec2 t)
{
    return length(vec2(length(p.xz) - t.x, p.y)) - t.y;
}

float sdCapsule(vec3 p, vec3 a, vec3 b, float r)
{
    vec3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h) - r;
}

RayHit opU(RayHit d1, RayHit d2) { return (d1.hitpoint < d2.hitpoint) ? d1 : d2; }

RayHit sdf(vec3 pos)
{
    // The last vec3 refers to the color of each object
    RayHit t = RayHit(400, vec3(1, 0, 0), 0, MATTE);

    t = RayHit(sdSphere(pos - vec3(0.0, 0.0, -15), 1), vec3(0.1804, 0.6, 0.2157), 0, REFLECTIVE);
    t = opU(t, RayHit(sdSphere(pos - vec3(-10.0, 0.0, -40), 4), vec3(0.0, 0.851, 1.0), 1, REFLECTIVE));
    t = opU(t, RayHit(sdBox(pos - vec3(5.0, 0.0, -30.0), vec3(3, 2.5, 2.5)), vec3(1.0, 1.0, 1.0), 2, REFLECTIVE));

    t = opU(t, RayHit(sdPlane(pos, vec4(0, 1, 0, 5.5)), checkers(pos), 7, MATTE));
    return t;
}

RayHit RayMarch(vec3 rayOrigin, vec3 rayDir)
{
    float t = 0.0; // Stores current distance along ray
    float tmax = 400;
    RayHit dummy = { -1.0, vec3(0.0), -1, 1.0 };

    for (int i = 0; i < MAX_STEPS; i++)
    {
        RayHit res = sdf(rayOrigin + rayDir * t);
        if (res.hitpoint < (MIN_DIST * t))
        {
            return RayHit(t, res.color, res.id, res.material);
        }
        if (res.hitpoint > tmax)
            return dummy;
        t += res.hitpoint;
    }

    return dummy;
}

RayHit reflectedRay(vec3 rayOrigin, vec3 rayDir)
{
    float t = 0.0; // Stores current distance along ray
    float tmax = 200;
    RayHit dummy = { -1.0, vec3(0.0), -1, 1.0 };

    for (int i = 0; i < MAX_STEPS / 2; i++)
    {
        RayHit res = sdf(rayOrigin + rayDir * t);
        if (res.hitpoint < (MIN_DIST * t))
        {
            return RayHit(t, res.color, res.id, res.material);
        }
        if (res.hitpoint > tmax)
            return dummy;
        t += res.hitpoint;
    }

    return dummy;
}

vec3 bounce(vec3 rayDir, vec3 pos, vec3 normal, vec3 color, RayHit primaryObject)
{
    RayHit prevObject = primaryObject;
    float shadow = 1.0;
    vec3 prevColor = primaryObject.color;

    for (int i = 1; i <= bounceVar; i++)
    {
        rayDir = reflect(rayDir, normal);
        RayHit t = reflectedRay(pos + normal * 0.001, rayDir);
        pos = pos + rayDir * t.hitpoint;
        normal = GetNormal(pos);

        if (t.hitpoint == -1.0)
            t.color = vec3(0.36, 0.36, 0.60) - (rayDir.y * 0.2);
        else
            t.color = getPointLight(t.color, normal, pos);

        if (t.id == 7 && prevObject.material != MATTE && i < 3)
        {
            vec3 shadowRayOrigin = pos + normal * 0.02;
            vec3 shadowRayDir = light.position - pos;
            shadow = softshadow(shadowRayOrigin, shadowRayDir, 2.0);
            color *= shadow / i;
        }

        if (prevObject.material == MATTE)
            continue;
        else
            color += t.color * prevColor / i;

        prevColor = t.color;
        prevObject = t;
    }

    return color;
}

float softshadow(vec3 ro, vec3 rd, float k)
{
    float res = 1.0;
    float t = 0.0;
    for (int i = 0; i < 16; i++)
    {
        RayHit h = sdf(ro + rd * t);
        if (h.hitpoint < 0.001)
            return 0.05;

        res = min(res, k * h.hitpoint / t);
        t += h.hitpoint;
    }

    return res;
}


vec3 getPointLight(vec3 color, vec3 normal, vec3 pos)
{
    vec3 ambient = light.ambient;
    vec3 viewDir = normalize(pos - camera.pos.xyz);

    vec3 lightDir = normalize(light.position - pos);
    float NtoL_vector = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * NtoL_vector;

    vec3 reflectDir = reflect(lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 specular = light.specular * spec;

    float distance = length(light.position - pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diffuse *= attenuation;
    ambient *= attenuation;
    specular *= attenuation;

    vec3 final = color * (diffuse + ambient + specular);
    return final;
}

vec3 GetNormal(vec3 pos)
{
    vec3 n = vec3(0.0);
    for (int i = 0; i < 4; i++)
    {
        vec3 e = 0.5773 * (2.0 * vec3((((i + 3) >> 1) & 1), ((i >> 1) & 1), (i & 1)) - 1.0);
        n += e * sdf(pos + 0.0005 * e).hitpoint;
        //if( n.x+n.y+n.z>100.0 ) break;
    }
    return normalize(n);
}

vec3 render(vec3 rayOrigin, vec3 rayDir)
{
    // background
    vec3 color = vec3(0.30, 0.36, 0.60) - (max(rayDir.y, 0.0) * 0.2);

    RayHit t = RayMarch(rayOrigin, rayDir);

    float shadow = 1.0;

    if (t.hitpoint != -1.0)
    {
        vec3 pos = rayOrigin + rayDir * t.hitpoint;
        vec3 normal = GetNormal(pos);

        vec3 prevColor = t.color;
        color = t.color;//normal * vec3(0.5) + vec3(0.5);
        color = getPointLight(color, normal, pos);

        if (t.id == 7) // Everything bellow is applied only for the floor plane
        {
            vec3 shadowRayOrigin = pos + normal * 0.02;
            vec3 shadowRayDir = light.position - pos;
            shadow = softshadow(shadowRayOrigin, shadowRayDir, 2.0);
            color *= shadow;
            color = pow(color, vec3(0.4545));
            return color;
        }

        color = bounce(rayDir, pos, normal, color, t);

    }
    // Gamma Correction
    color = pow(color, vec3(0.4545));
    // Visualize depth
    // color = vec3(1.0-t.hitpoint*0.075);
    return color;
}

void main()
{
    vec3 finalColor = vec3(0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    // Compute global x, y coordinates utilizing local group ID
    float time = 32.0 + iTime * 1.5;
    ivec2 dims = imageSize(img_output);
    float x = (float(texelCoord.x) / float(gl_NumWorkGroups.x * gl_WorkGroupSize.x));
    float y = (float(texelCoord.y) / float(gl_NumWorkGroups.y * gl_WorkGroupSize.y));

    vec3 ro = camera.pos.xyz;

    vec2 fragCoord = vec2(x * dims.x, y * dims.y);

    for (int m = 0; m < 2; m++)
    {
        for (int n = 0; n < 2; n++)
        {
            vec2 o = vec2(float(m), float(n)) / 2.0 - 0.5;
            vec2 p = (2.0 * (fragCoord + o) - dims.xy) / dims.y;

            const float fl = 2.5;
            vec4 dir = p.x * camera.xAxis + p.y * camera.yAxis + fl * camera.dir;
            vec3 rd = normalize(dir.xyz);

            finalColor += render(ro, rd);
        }
    }
    finalColor /= 4.0;

    imageStore(img_output, texelCoord, vec4(finalColor, 1.0));
}
