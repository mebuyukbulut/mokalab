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
#include <future>

#include "Asset.h"
#include <memory>

class Texture;
struct EngineContext;

class Model : public Asset{
    friend class ModelLoader;
    EngineContext* ece{};
    //MaterialManager* _materialManager{};
    //TextureManager* _textureManager{};
    
    std::vector<std::shared_ptr<Material>> _materials{};
	
    std::vector<Mesh> meshes;


    //unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = 1);
    
public:
    Model(EngineContext* ece) { _type = AssetType::Model; this->ece = ece; }

    void draw(Shader* shader, bool bindMaterial = true);

    // Inherited via Asset
    void load(std::filesystem::path path, IAssetSettings* settings) override {};
    void save(std::filesystem::path path, IAssetSettings* settings) override {};
    void reload() override {};
    void purgeCPU() override;
    void purgeGPU() override {};
    void uploadGPU() override;

    virtual void onInspect() override ;

};
