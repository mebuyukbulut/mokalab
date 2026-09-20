#pragma once
#include <filesystem>

class ConfigManager {
public:
    template<typename T>
    static bool load(const std::filesystem::path& path, T& outConfig);

    template<typename T>
    static bool save(const std::filesystem::path& path, const T& config);
};