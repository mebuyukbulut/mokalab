#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "Mesh.h"
#include "Material.h"

#include <glad/gl.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>
#include "Asset.h"
#include <future>
#include <memory>

class Texture;
struct EngineContext;

class Model : public Asset{
    EngineContext* ece{};
    //MaterialManager* _materialManager{};
    //TextureManager* _textureManager{};
    
    std::vector<std::shared_ptr<Material>> _materials{};
	
    std::vector<Mesh> meshes;

    void loadDefault(std::string pathStr);

    //unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = 1);
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::shared_ptr<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type);

public:
    Model(EngineContext* ece) { _type = AssetType::Model; this->ece = ece; }

    void draw(Shader* shader, bool bindMaterial = true);

    // Inherited via Asset
    void load(std::filesystem::path path, IAssetSettings* settings) override;
    void unload() override;
    void uploadToGPU() override;

    virtual void onInspect() override ;

};
