#pragma once 
#include <memory>
#include <filesystem>

class IAssetSaver{
public:
    virtual void save(std::filesystem::path path, struct std::shared_ptr<class Asset> asset) = 0;
};