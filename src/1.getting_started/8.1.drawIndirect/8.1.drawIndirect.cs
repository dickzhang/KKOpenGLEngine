#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct VertexData {
    vec2 position;
    vec2 texCoord;
};

struct MeshData {
    uint vertexIndex;
    uint vertexCount;
    uint indicesIndex;
    uint indicesCount;
};

struct InstanceData
{
    uint instanceCount;
    uint meshIndex; 
    uint fillData1;
    uint fillData2;
    mat4 modelMat;
};

struct IndirectCommand
{
    uint vertexCount;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

layout(std430, binding = 0) buffer OriginVertexDataBuffer
{
    VertexData vertexDatas [];
};

layout(std430, binding = 1) buffer MeshDataBuffer
{
    MeshData meshDatas [];
};

layout(std430, binding = 2) buffer InstanceBuffer
{
    InstanceData instances[];
};

layout(std430, binding = 3) buffer IndirectCommandBuffer
{
    IndirectCommand indirectCommands[];
};

layout(std430, binding = 4) buffer ComputedVertexDataBuffer
{
    VertexData computedVertexDatas [];
};

void main(void)
{
    uint globalId = gl_GlobalInvocationID.x;
    uint index = 0;
    for (uint i = 0; i < globalId; i++)
    {
        index += meshDatas[instances[i].meshIndex].vertexCount;
    }
    InstanceData currentInstance = instances[globalId];
    MeshData currentMesh = meshDatas[currentInstance.meshIndex];
    for (uint i = 0; i < currentMesh.vertexCount; i++)
    {
        computedVertexDatas[index + i] = vertexDatas[currentMesh.vertexIndex + i];
        computedVertexDatas[index + i].position = (currentInstance.modelMat * vec4(computedVertexDatas[index + i].position, 0.0, 1.0)).xy;
    }
    indirectCommands[globalId].vertexCount = currentMesh.indicesCount;
    indirectCommands[globalId].instanceCount = 1;
    indirectCommands[globalId].firstIndex = currentMesh.indicesIndex;
    indirectCommands[globalId].baseVertex = index;
    indirectCommands[globalId].baseInstance = globalId;
}