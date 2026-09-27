#pragma once
#include "AssetSystem/IAssetSaver.h"
#include "Material.h"
#include "AssetManager.h"


class MaterialSaver : public IAssetSaver {
    inline static const bool registered = []() {
        AssetManager::registerSaver<Material>([]() -> std::shared_ptr<IAssetSaver> {
            return std::make_shared<MaterialSaver>();
        });
        return true;
    }();

public:
    void save(std::filesystem::path path, struct std::shared_ptr<class Asset> asset) override;
};