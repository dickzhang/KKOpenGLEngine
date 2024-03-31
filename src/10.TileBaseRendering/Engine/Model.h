#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "BatchedMeshCreator.h"

class ShaderProgram;

class Model 
{
    public:
        void Init(std::string path, std::string fileName, bool batchMeshes);
        void Draw(ShaderProgram& shader);	

        void SetModel(const glm::mat4 model);
        void SetView(const glm::mat4 view);
        void SetProj(const glm::mat4 proj);

    private:

        //TODO: track only textures for models, track others as well?
        static std::unordered_map<std::string, Texture> loadedTextures_;

        // model data
        std::vector<Mesh> meshes_;
        std::string directory_;

        void LoadModel(std::string path, std::string fileName, std::optional<BatchedMeshCreator> meshBatchCreator);
        void ProcessNode(aiNode *node, const aiScene *scene, std::optional<BatchedMeshCreator>& meshBatchCreator);
        void processMesh(aiMesh *mesh, const aiScene *scene, std::optional<BatchedMeshCreator>& meshBatchCreator);
        Texture loadMaterialTexture(aiMaterial *mat, aiTextureType type, std::string typeName);
};
