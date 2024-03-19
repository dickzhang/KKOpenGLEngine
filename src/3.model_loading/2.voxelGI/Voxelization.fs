#version 430 core

#extension GL_ARB_shader_image_load_store : require

in GeometryOut
{
    vec3 wsPosition;
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    flat vec4 triangleAABB;
} In;

layout (location = 0) out vec4 fragColor;
layout (pixel_center_integer) in vec4 gl_FragCoord;

layout(binding = 0)uniform sampler2D texture_diffuse;
layout(binding = 5, r32ui) uniform volatile coherent uimage3D voxelAlbedo;
layout(binding = 6, r32ui) uniform volatile coherent uimage3D voxelNormal;


vec4 convRGBA8ToVec4(uint val)
{
    return vec4(float((val & 0x000000FF)), 
    float((val & 0x0000FF00) >> 8U), 
    float((val & 0x00FF0000) >> 16U), 
    float((val & 0xFF000000) >> 24U));
}

uint convVec4ToRGBA8(vec4 val)
{
    return (uint(val.w) & 0x000000FF) << 24U | 
    (uint(val.z) & 0x000000FF) << 16U | 
    (uint(val.y) & 0x000000FF) << 8U | 
    (uint(val.x) & 0x000000FF);
}

vec3 EncodeNormal(vec3 normal)
{
    return normal * 0.5f + vec3(0.5f);
}

void imageAtomicRGBA8Avg(layout(r32ui) volatile coherent uimage3D grid, ivec3 coords, vec4 value)
{
    value.rgb *= 255.0;              
    uint newVal = convVec4ToRGBA8(value);
    uint prevStoredVal = 0;
    uint curStoredVal;
    uint numIterations = 0;

    while((curStoredVal = imageAtomicCompSwap(grid, coords, prevStoredVal, newVal)) != prevStoredVal && numIterations < 255)
    {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.rgb = (rval.rgb * rval.a); // Denormalize
        vec4 curValF = rval + value;    // Add
        curValF.rgb /= curValF.a;       // Renormalize
        newVal = convVec4ToRGBA8(curValF);
        ++numIterations;
    }
}

void main()
{
    if( In.position.x < In.triangleAABB.x || In.position.y < In.triangleAABB.y || 
        In.position.x > In.triangleAABB.z || In.position.y > In.triangleAABB.w )
    {
        discard;
    }
    vec4 albedo= texture(texture_diffuse, In.texCoord);
    albedo.rgb=albedo.rgb*0.588;
    vec4 normal = vec4(EncodeNormal(normalize(In.normal)), 1.0f);

    imageAtomicRGBA8Avg(voxelAlbedo, ivec3(In.wsPosition), albedo);
    imageAtomicRGBA8Avg(voxelNormal, ivec3(In.wsPosition),normal );
}
