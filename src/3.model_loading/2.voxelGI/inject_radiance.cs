#version 430 core
#extension GL_ARB_shader_image_load_store : require
layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(binding = 0) uniform sampler3D voxelAlbedo;
layout(binding = 1, rgba8) uniform image3D voxelNormal;
layout(binding = 2, rgba8) uniform writeonly image3D voxelRadiance;

const float EPSILON = 1e-30;

uniform vec3 lightdiffuse;
uniform vec3 lightdirection;

uniform float voxelSize;
uniform float voxelScale;
uniform vec3 worldMinPoint;
uniform int volumeDimension;
uniform int normalWeightedLambert;

vec3 VoxelToWorld(ivec3 pos)
{
    vec3 result = vec3(pos);
    result *= voxelSize;
    return result + worldMinPoint;
}

vec3 BRDF(vec3 normal, vec3 albedo)
{
    float nDotL = 0.0f;
    //vec3 direction = normalize(lightdirection);
    if (normalWeightedLambert == 1)
    {
        vec3 weight = normal * normal;
        float rDotL = dot(vec3(1.0, 0.0, 0.0), lightdirection);
        float uDotL = dot(vec3(0.0, 1.0, 0.0), lightdirection);
        float fDotL = dot(vec3(0.0, 0.0, 1.0), lightdirection);

        //���÷��߼�Ȩ,�����һ���������ж�������ε�����
        //��ƽ�����߷�����ŵĹ��߷��������Ϊ0,ͬ�ŵĻ���ȡ���߷�������ľ���ֵ
        rDotL = normal.x > 0.0 ? max(rDotL, 0.0) : max(-rDotL, 0.0);
        uDotL = normal.y > 0.0 ? max(uDotL, 0.0) : max(-uDotL, 0.0);
        fDotL = normal.z > 0.0 ? max(fDotL, 0.0) : max(-fDotL, 0.0);

        nDotL = rDotL * weight.x + uDotL * weight.y + fDotL * weight.z;
    }
    else
    {
        nDotL = max(dot(normal, lightdirection), 0.0f);
    }
    //ʹ��������������ģ��,��ȡ��������ն�
    return lightdiffuse * albedo * nDotL;
}

vec4 CalculateDirectional( vec3 normal, vec3 position, vec3 albedo)
{
    return vec4(BRDF( normal, albedo) , 1.0);
}

vec4 CalculateDirectLighting(vec3 position, vec3 normal, vec3 albedo)
{
    normal = normalize(normal);
    float voxelWorldSize = 1.0 /  (voxelScale * volumeDimension);
    position = position + normal * voxelWorldSize * 0.5f;
    vec4 directLighting = CalculateDirectional(normal, position, albedo);
    return directLighting;
}

vec3 DecodeNormal(vec3 normal)
{
    return normal * 2.0f - vec3(1.0f);
}

void main()
{
	if(gl_GlobalInvocationID.x >= volumeDimension ||
		gl_GlobalInvocationID.y >= volumeDimension ||
		gl_GlobalInvocationID.z >= volumeDimension) return;

	ivec3 writePos = ivec3(gl_GlobalInvocationID);
	vec4 albedo = texelFetch(voxelAlbedo, writePos, 0);
    if(albedo.a < EPSILON) { return; }

    albedo.a = 0.0f;
    vec3 baseNormal = imageLoad(voxelNormal, writePos).xyz;
    //�ѷ���ת����-1~1��Χ��ֵ
    vec3 normal = DecodeNormal(baseNormal);
    //�ж�albedo.rgb��ֵ�Ƿ񶼴���0,any��ʾalbedo.rgbֻҪ��һ������0�򷵻�true
    if(any(greaterThan(albedo.rgb, vec3(0.0f))))
    {
        //voxel����ת�ص���������
        vec3 wsPosition = VoxelToWorld(writePos);
        albedo = CalculateDirectLighting(wsPosition, normal, albedo.rgb);
    }
    //����3D������д��,���յ�alphaֵ
    imageStore(voxelNormal, writePos, vec4(baseNormal, albedo.a));
    albedo.a = 1.0f;
    //����ն�3D������д��,�������յ���ɫֵ
	imageStore(voxelRadiance, writePos, albedo);
}