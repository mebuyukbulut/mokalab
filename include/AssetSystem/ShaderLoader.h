#pragma once
#include "AssetSystem/IAssetLoader.h"
#include "Shader.h"
#include "AssetManager.h"


class ShaderLoader : public IAssetLoader{
    inline static const bool registered = []() {
        AssetManager::registerLoader<Shader>([](struct EngineContext* ece) -> std::shared_ptr<IAssetLoader> {
            return std::make_shared<ShaderLoader>();
        });
        return true;
    }();
    
public:
    std::shared_ptr<Asset> load(std::filesystem::path path, IAssetSettings* settings) override;
};