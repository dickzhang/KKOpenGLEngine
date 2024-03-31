#pragma once

#include <vector>
#include <unordered_map>

#include "MaterialData.h"
#include "Mesh.h"

class BatchedMeshCreator
{
public:
    BatchedMeshCreator(/* args */);
    ~BatchedMeshCreator();

    void AddMeshDataToBatch(
        const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices,
        MaterialData material);

    void GenerateMeshes(std::vector<Mesh>& meshes);

private:

    struct BatchedMesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        MaterialData material;
        unsigned int m_indicieNumber = 0;
    };

    void adjustIndicesForBatch(unsigned int batchId);

    std::unordered_map<unsigned int, BatchedMesh> meshes_;
};