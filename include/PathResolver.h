// PathResolver.h
#pragma once
#include <filesystem>

class PathResolver{
public:
    std::filesystem::path projectRoot{};
    std::filesystem::path mokaFile{};
    std::filesystem::path projectConfig{};
    std::filesystem::path contentFolder{};

    std::filesystem::path applicationRoot{}; 
    std::filesystem::path lastProjectsPath{}; 
    

    std::filesystem::path engineRoot{}; 
    std::filesystem::path assetRoot{}; 

    PathResolver(){
        engineRoot = std::filesystem::current_path().parent_path();
        assetRoot = engineRoot / "assets";

        applicationRoot = engineRoot / "appData"; 
        lastProjectsPath = applicationRoot / "last-projects-list.txt"; 
    }

    void setProjectRoot(std::filesystem::path newProjectRoot){ 
        projectRoot = newProjectRoot; 
        mokaFile = newProjectRoot / "project.moka";
        projectConfig = newProjectRoot / "config.yaml"; 
        contentFolder = newProjectRoot / "content"; 
    }
};