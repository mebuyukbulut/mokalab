#pragma once
#include <filesystem>

class PathResolver{
public:
    std::filesystem::path projectRoot{};
    std::filesystem::path engineRoot{}; 
    std::filesystem::path assetRoot{}; 

    PathResolver(){
        engineRoot = std::filesystem::current_path().parent_path();
        assetRoot = engineRoot / "assets";
    }
};