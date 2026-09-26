#pragma once
#include "AssetSystem/IAssetLoader.h"
#include "Model.h"
#include "AssetManager.h"
#include "AssetHandle.h"
#include "Logger.h"

struct EngineContext;
class ModelLoader : public IAssetLoader{
    inline static const bool registered = []() {
        AssetManager::registerLoader<Model>([](EngineContext* ece) -> std::shared_ptr<IAssetLoader> {
            return std::make_shared<ModelLoader>(ece);
        });
        return true;
    }();

    EngineContext* _ece{}; 
    void loadDefault(std::shared_ptr<Model> model, std::string pathStr);
    void loadModel(std::shared_ptr<Model> model, const std::string& path);
    void processNode(std::shared_ptr<Model> model, aiNode* node, const aiScene* scene);
    Mesh processMesh(std::shared_ptr<Model> model, aiMesh* mesh, const aiScene* scene);
    AssetHandle<Texture> loadMaterialTextures(std::shared_ptr<Model> model, aiMaterial* mat, aiTextureType type);

public:
    ModelLoader(EngineContext* ece) : _ece{ece}{}
    std::shared_ptr<Asset> load(std::filesystem::path path, IAssetSettings* settings) override;
};