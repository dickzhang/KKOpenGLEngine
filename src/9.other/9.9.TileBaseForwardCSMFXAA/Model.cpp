#include "Model.h"

#include <iostream>
#include <optional>

#include "ShaderProgram.h"
#include "Texture.h"

std::unordered_map<std::string, Texture> Model::loadedTextures_;

void Model::Init(std::string path, std::string fileName, bool batchMeshes)
{
    LoadModel(path, fileName, batchMeshes ? BatchedMeshCreator() : std::optional<BatchedMeshCreator>());
}

void Model::Draw(ShaderProgram &shader)
{
    for (Mesh& m : meshes_)
    {
        m.Draw(shader);
    }
}


void Model::SetModel(const glm::mat4 model)
{
    for (Mesh& mesh : meshes_)
    {
        mesh.SetModel(model);
    }
}

void Model::SetView(const glm::mat4 view)
{
    for (Mesh& mesh : meshes_)
    {
        mesh.SetView(view);
    }
}

void Model::SetProj(const glm::mat4 proj)
{
    for (Mesh& mesh : meshes_)
    {
        mesh.SetProj(proj);
    }
}

void Model::LoadModel(std::string path, std::string fileName, std::optional<BatchedMeshCreator> meshBatchCreator)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path + fileName, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << '\n';
        return;
    }
    directory_ = path;

    ProcessNode(scene->mRootNode, scene, meshBatchCreator);

    if (meshBatchCreator.has_value())
    {
        meshBatchCreator->GenerateMeshes(meshes_);
    }
}

void Model::ProcessNode(aiNode *node, const aiScene *scene, std::optional<BatchedMeshCreator>& meshBatchCreator)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, meshBatchCreator);
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, meshBatchCreator);
    }
}

void Model::processMesh(aiMesh *mesh, const aiScene *scene, std::optional<BatchedMeshCreator>& meshBatchCreator)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    MaterialData material;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;

        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // process material
    // TODO: only one diffuse and one specular map, implement more in the future
    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *assimpMaterial = scene->mMaterials[mesh->mMaterialIndex];
        material.diffuseMap = loadMaterialTexture(assimpMaterial,
                                                                aiTextureType_DIFFUSE, "texture_diffuse");
        
        material.specularMap = loadMaterialTexture(assimpMaterial,
                                                                 aiTextureType_SPECULAR, "texture_specular");
        material.shine = 32.0;
    }

    if (meshBatchCreator.has_value())
    {
        meshBatchCreator->AddMeshDataToBatch(vertices, indices, material);
    }
    else
    {
        Mesh m;
        std::vector<VertexAttributeDescription> attribDescriptions;
        attribDescriptions.push_back(VertexAttributeDescription(3, false, VertexAttributeType::POSITION));
        attribDescriptions.push_back(VertexAttributeDescription(3, false, VertexAttributeType::NORMALS));
        attribDescriptions.push_back(VertexAttributeDescription(2, false, VertexAttributeType::TEX_COORDS));

        m.Init(vertices, attribDescriptions, indices, material);
        meshes_.push_back(m);
    }
}

Texture Model::loadMaterialTexture(aiMaterial *mat, aiTextureType type,
                                                 std::string typeName)
{
    Texture texture;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string texPath = directory_ + str.C_Str();
        if (loadedTextures_.find(texPath) != loadedTextures_.end())
        {
            return loadedTextures_[texPath];
        }
        
        texture.Init(texPath.c_str(), loadedTextures_.size());
        loadedTextures_[texPath] = texture;

        break;
    }
    return texture;
}