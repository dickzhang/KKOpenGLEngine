#include "BatchedMeshCreator.h"


BatchedMeshCreator::BatchedMeshCreator(/* args */)
{
}

BatchedMeshCreator::~BatchedMeshCreator()
{
}


void BatchedMeshCreator::AddMeshDataToBatch(
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices,
    MaterialData material)
{
    // we batch together meshes with the same material
    unsigned int batchId = material.diffuseMap.GetId();

    // this is the first mesh in the batch, so we need to assign a material
    if (meshes_[batchId].material.diffuseMap.GetId() == 0)
    {
        meshes_[batchId].material = material;
    }

    meshes_[batchId].indices.insert(meshes_[batchId].indices.end(), indices.begin(), indices.end());

    adjustIndicesForBatch(batchId);
    meshes_[batchId].vertices.insert(meshes_[batchId].vertices.end(), vertices.begin(), vertices.end());
    
    meshes_[batchId].m_indicieNumber += indices.size();
}

void BatchedMeshCreator::adjustIndicesForBatch(unsigned int batchId)
{
    // vertices won't have the same index in the batched mesh, so we need to adjust the index buffer values
    for (int i = meshes_[batchId].m_indicieNumber; i < meshes_[batchId].indices.size(); i++)
    {
        meshes_[batchId].indices[i] += meshes_[batchId].vertices.size();
    }
}

void BatchedMeshCreator::GenerateMeshes(std::vector<Mesh>& meshes)
{
    std::vector<VertexAttributeDescription> attribDescriptions;
    attribDescriptions.push_back(VertexAttributeDescription(3, false, VertexAttributeType::POSITION));
    attribDescriptions.push_back(VertexAttributeDescription(3, false, VertexAttributeType::NORMALS));
    attribDescriptions.push_back(VertexAttributeDescription(2, false, VertexAttributeType::TEX_COORDS));

    for (auto it = meshes_.begin(); it != meshes_.end(); it++)
    {
        BatchedMesh& batchedMesh = it->second;
        Mesh mesh;
        mesh.Init(batchedMesh.vertices, attribDescriptions, batchedMesh.indices, batchedMesh.material);
        meshes.push_back(mesh);
    }
}