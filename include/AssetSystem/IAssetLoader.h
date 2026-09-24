#pragma once
#include <memory>
#include <filesystem>

class IAssetLoader{
public:
    virtual std::shared_ptr<class Asset> load(std::filesystem::path path, struct IAssetSettings* settings) = 0;
};