#pragma once
#include "AssetSystem/IAssetLoader.h"
#include "AssetManager.h"
#include "Texture.h"


struct EngineContext;
class TextureLoader : public IAssetLoader{
    inline static const bool registered = []() {
        AssetManager::registerLoader<Texture>([](EngineContext* ece) -> std::shared_ptr<IAssetLoader> {
            return std::make_shared<TextureLoader>();
        });
        return true;
    }();

	void loadInternal(std::shared_ptr<Texture> texture, std::string path);
    void createSolidColorTextureRGBA8(std::shared_ptr<Texture> texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    
public:
    std::shared_ptr<Asset> load(std::filesystem::path path, IAssetSettings* settings) override;
};