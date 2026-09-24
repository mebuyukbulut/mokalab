#pragma once
#include "AssetSystem/IAssetLoader.h"
#include "Material.h"
#include "AssetManager.h"


class MaterialLoader : public IAssetLoader{
    inline static const bool registered = []() {
        AssetManager::registerLoader<Material>([](struct EngineContext* ece) -> std::shared_ptr<IAssetLoader> {
            return std::make_shared<MaterialLoader>();
        });
        return true;
    }();

    void loadDefault(std::string path, std::shared_ptr<Material> mat); 
public:

    std::shared_ptr<Asset> load(std::filesystem::path path, IAssetSettings* settings) override;


};