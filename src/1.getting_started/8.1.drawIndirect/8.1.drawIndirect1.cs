
#version 450 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

struct Pixel
{
    ivec3 coord;

    vec3 centerPoint_middle;
    vec3 neighborPoint0_middle;
    vec3 neighborPoint1_middle;
    vec3 neighborPoint2_middle;
    vec3 neighborPoint3_middle;
    vec3 neighborPoint4_middle;
    vec3 neighborPoint5_middle;
    vec3 neighborPoint6_middle;
    vec3 neighborPoint7_middle;

    vec3 centerPoint_near;
    vec3 neighborPoint0_near;
    vec3 neighborPoint1_near;
    vec3 neighborPoint2_near;
    vec3 neighborPoint3_near;
    vec3 neighborPoint4_near;
    vec3 neighborPoint5_near;
    vec3 neighborPoint6_near;
    vec3 neighborPoint7_near;

    vec3 centerPoint_far;
    vec3 neighborPoint0_far;
    vec3 neighborPoint1_far;
    vec3 neighborPoint2_far;
    vec3 neighborPoint3_far;
    vec3 neighborPoint4_far;
    vec3 neighborPoint5_far;
    vec3 neighborPoint6_far;
    vec3 neighborPoint7_far;
};

layout(binding = 0, rgba8) uniform image2D _Texture2D;
layout(binding = 1, rgba8) uniform image3D _Texture3D;

layout(std430) buffer ColorBuffer
{
    vec4 _Colors [];
};

uniform int _Resolution;
uniform float _Frequency;
uniform bool _Is3D;
uniform bool _IsTilable;
uniform float _RandomSeed;
uniform vec3 _Evolution;
uniform int _FBMIteration;
uniform int _ReturnType;

float GetDistance(vec3 vec0, vec3 vec1)
{
    vec3 vec = vec0 - vec1;
    float dist;

    if (_ReturnType == 0 || _ReturnType == 2)
    {
        dist = length(vec);
    }
    else
    {
        vec = pow(abs(vec), vec3(3.0));
        dist = pow(vec.x + vec.y + vec.z, 1.0 / 3.0);
    }

    return dist;
}

ivec3 GetBlockMin(int blockSize, ivec3 blockCoord)
{
    ivec3 blockMin;
    blockMin.x = blockCoord.x * blockSize;
    blockMin.y = blockCoord.y * blockSize;
    blockMin.z = blockCoord.z * blockSize;
    return blockMin;
}
ivec3 GetBlockMax(int blockSize, ivec3 blockCoord)
{
    ivec3 blockMax;
    blockMax.x = blockCoord.x * blockSize + blockSize;
    blockMax.y = blockCoord.y * blockSize + blockSize;
    blockMax.z = blockCoord.z * blockSize + blockSize;
    return blockMax;
}

ivec3 PixelCoordToBlockCoord(int blockSize, ivec3 pixelCoord)
{
    ivec3 blockCoord;
    blockCoord.x = int(floor(pixelCoord.x / float(blockSize)));
    blockCoord.y = int(floor(pixelCoord.y / float(blockSize)));
    blockCoord.z = int(floor(pixelCoord.z / float(blockSize)));
    return blockCoord;
}

vec3 GetRandom3To3_Raw(vec3 param, float randomSeed)
{
    vec3 value;
    value.x = length(param) + 58.12 + 79.52 * randomSeed;
    value.y = length(param) + 96.53 + 36.95 * randomSeed;
    value.z = length(param) + 71.65 + 24.58 * randomSeed;
    value.x = mod(sin(value.x), 1);
    value.y = mod(sin(value.y), 1);
    value.z = mod(sin(value.z), 1);
    return value;
}

vec3 GetRandom3To3_Remapped(vec3 param, float randomSeed)
{
    vec3 value = GetRandom3To3_Raw(param, randomSeed);
    value.x = (value.x + 1) / 2;
    value.y = (value.y + 1) / 2;
    value.z = (value.z + 1) / 2;
    return value;
}

vec3 GetRandomPoint(int blockNumber, int blockSize, ivec3 blockCoord, float randomSeed)
{
    ivec3 blockMin = GetBlockMin(blockSize, blockCoord);

    if (_IsTilable)
    {
        if (blockCoord.x == blockNumber - 2)
            blockCoord.x = -2;
        if (blockCoord.y == blockNumber - 2)
            blockCoord.y = -2;
        if (blockCoord.z == blockNumber - 2)
            blockCoord.z = -2;

        if (blockCoord.x == blockNumber - 1)
            blockCoord.x = -1;
        if (blockCoord.y == blockNumber - 1)
            blockCoord.y = -1;
        if (blockCoord.z == blockNumber - 1)
            blockCoord.z = -1;

        if (blockCoord.x == blockNumber)
            blockCoord.x = 0;
        if (blockCoord.y == blockNumber)
            blockCoord.y = 0;
        if (blockCoord.z == blockNumber)
            blockCoord.z = 0;
    }

    vec3 randomNumber = GetRandom3To3_Remapped(blockCoord + _Evolution, length(blockCoord) * randomSeed);
    ivec3 pnt;
    pnt.x = int(round(mix(0, blockSize, randomNumber.x)));
    pnt.y = int(round(mix(0, blockSize, randomNumber.y)));
    pnt.z = int(round(mix(0, blockSize, randomNumber.z)));

    pnt += blockMin;

    return pnt;
}

float GetNoiseValue(uvec3 id, float tile, float randomSeed)
{
    int blockNumber = int(ceil(tile));

    int blockSize = int(ceil(float(_Resolution) / float(blockNumber)));

    Pixel pixel;// = Pixel(0);
    pixel.coord = ivec3(id.xyz);

    ivec3 blockCoord = PixelCoordToBlockCoord(blockSize, pixel.coord);

    pixel.centerPoint_middle = GetRandomPoint(blockNumber, blockSize, blockCoord, randomSeed);
    pixel.neighborPoint0_middle = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, 0, 0), randomSeed);
    pixel.neighborPoint1_middle = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, 1, 0), randomSeed);
    pixel.neighborPoint2_middle = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(0, 1, 0), randomSeed);
    pixel.neighborPoint3_middle = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, 1, 0), randomSeed);
    pixel.neighborPoint4_middle = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, 0, 0), randomSeed);
    pixel.neighborPoint5_middle = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, -1, 0), randomSeed);
    pixel.neighborPoint6_middle = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(0, -1, 0), randomSeed);
    pixel.neighborPoint7_middle = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, -1, 0), randomSeed);

    if (_Is3D)
    {
        pixel.centerPoint_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(0, 0, -1), randomSeed);
        pixel.neighborPoint0_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, 0, -1), randomSeed);
        pixel.neighborPoint1_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, 1, -1), randomSeed);
        pixel.neighborPoint2_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(0, 1, -1), randomSeed);
        pixel.neighborPoint3_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, 1, -1), randomSeed);
        pixel.neighborPoint4_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, 0, -1), randomSeed);
        pixel.neighborPoint5_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, -1, -1), randomSeed);
        pixel.neighborPoint6_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(0, -1, -1), randomSeed);
        pixel.neighborPoint7_near = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, -1, -1), randomSeed);

        pixel.centerPoint_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(0, 0, 1), randomSeed);
        pixel.neighborPoint0_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, 0, 1), randomSeed);
        pixel.neighborPoint1_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, 1, 1), randomSeed);
        pixel.neighborPoint2_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(0, 1, 1), randomSeed);
        pixel.neighborPoint3_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, 1, 1), randomSeed);
        pixel.neighborPoint4_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, 0, 1), randomSeed);
        pixel.neighborPoint5_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(1, -1, 1), randomSeed);
        pixel.neighborPoint6_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(0, -1, 1), randomSeed);
        pixel.neighborPoint7_far = GetRandomPoint(blockNumber, blockSize, blockCoord + ivec3(-1, -1, 1), randomSeed);
    }

    float distances[27];

    distances[0] = GetDistance(pixel.coord, pixel.centerPoint_middle);
    distances[1] = GetDistance(pixel.coord, pixel.neighborPoint0_middle);
    distances[2] = GetDistance(pixel.coord, pixel.neighborPoint1_middle);
    distances[3] = GetDistance(pixel.coord, pixel.neighborPoint2_middle);
    distances[4] = GetDistance(pixel.coord, pixel.neighborPoint3_middle);
    distances[5] = GetDistance(pixel.coord, pixel.neighborPoint4_middle);
    distances[6] = GetDistance(pixel.coord, pixel.neighborPoint5_middle);
    distances[7] = GetDistance(pixel.coord, pixel.neighborPoint6_middle);
    distances[8] = GetDistance(pixel.coord, pixel.neighborPoint7_middle);

    if (_Is3D)
    {
        distances[9] = GetDistance(pixel.coord, pixel.centerPoint_near);
        distances[10] = GetDistance(pixel.coord, pixel.neighborPoint0_near);
        distances[11] = GetDistance(pixel.coord, pixel.neighborPoint1_near);
        distances[12] = GetDistance(pixel.coord, pixel.neighborPoint2_near);
        distances[13] = GetDistance(pixel.coord, pixel.neighborPoint3_near);
        distances[14] = GetDistance(pixel.coord, pixel.neighborPoint4_near);
        distances[15] = GetDistance(pixel.coord, pixel.neighborPoint5_near);
        distances[16] = GetDistance(pixel.coord, pixel.neighborPoint6_near);
        distances[17] = GetDistance(pixel.coord, pixel.neighborPoint7_near);

        distances[18] = GetDistance(pixel.coord, pixel.centerPoint_far);
        distances[19] = GetDistance(pixel.coord, pixel.neighborPoint0_far);
        distances[20] = GetDistance(pixel.coord, pixel.neighborPoint1_far);
        distances[21] = GetDistance(pixel.coord, pixel.neighborPoint2_far);
        distances[22] = GetDistance(pixel.coord, pixel.neighborPoint3_far);
        distances[23] = GetDistance(pixel.coord, pixel.neighborPoint4_far);
        distances[24] = GetDistance(pixel.coord, pixel.neighborPoint5_far);
        distances[25] = GetDistance(pixel.coord, pixel.neighborPoint6_far);
        distances[26] = GetDistance(pixel.coord, pixel.neighborPoint7_far);
    }

    float closestDistance0 = 999999;
    float closestDistance1 = 999999;
    float closestDistance2 = 999999;

    if (_Is3D)
    {
        for (int iii = 0; iii < 27; iii++)
        {
            float tempDistance = distances[iii];

            if (tempDistance < closestDistance0)
            {
                closestDistance2 = closestDistance1;
                closestDistance1 = closestDistance0;
                closestDistance0 = tempDistance;
            }
            else if (tempDistance < closestDistance1)
            {
                closestDistance2 = closestDistance1;
                closestDistance1 = tempDistance;
            }
            else if (tempDistance < closestDistance2)
            {
                closestDistance2 = tempDistance;
            }
        }
    }
    else
    {
        for (int iii = 0; iii < 9; iii++)
        {
            float tempDistance = distances[iii];

            if (tempDistance < closestDistance0)
            {
                closestDistance2 = closestDistance1;
                closestDistance1 = closestDistance0;
                closestDistance0 = tempDistance;
            }
            else if (tempDistance < closestDistance1)
            {
                closestDistance2 = closestDistance1;
                closestDistance1 = tempDistance;
            }
            else if (tempDistance < closestDistance2)
            {
                closestDistance2 = tempDistance;
            }
        }
    }

    float noise;

    if (_ReturnType == 0 || _ReturnType == 1)
    {
        noise = closestDistance0;
        noise /= blockSize;
    }
    else
    {
        noise = closestDistance1 - closestDistance0;
        noise /= blockSize;
    }

    return noise;
}
void main()
{
    // 获取当前线程的全局坐标
    uvec3 id = gl_GlobalInvocationID.xyz;

    int colorIndex = int(id.x);
    colorIndex += _Resolution * int(id.y);
    colorIndex += _Resolution * _Resolution * int(id.z);

    if (_Is3D)
    {
        if (colorIndex >= _Resolution * _Resolution * _Resolution)
        {
            return;
        }
    }
    else
    {
        if (colorIndex >= _Resolution * _Resolution)
        {
            return;
        }
    }

    float noise = GetNoiseValue(id, _Frequency, _RandomSeed);

    float currentTile = _Frequency;
    float currentStrength = 1;
    for (int iii = 0; iii < _FBMIteration; iii++)
    {
        currentTile *= 2;
        currentStrength /= 2;
        if (currentTile >= _Resolution)
        {
            currentTile /= 2;
        }
        noise += GetNoiseValue(id, currentTile, _RandomSeed + currentTile) * currentStrength;
    }

    _Colors[colorIndex] = vec4(noise, noise, noise, 1);

    if (_Is3D)
    {
        imageStore(_Texture3D, ivec3(id), vec4(noise, noise, noise, 1));
    }
    else
    {
        imageStore(_Texture2D, ivec2(id.xy), vec4(noise, noise, noise, 1));
    }
}
