#pragma once
#include <string>



struct AppConfig
{
    struct Window {
        int width;
        int height;
        std::string title;
        bool fullscreen;
    }window;

    struct Graphics {    
    };
// public:
//     Window window;
//     void load(std::string path = "../assets/config/config.yaml");
//     void save(std::string path = "../assets/config/config.yaml");
};

struct ProjectConfig{


    struct UI {
        bool isCreditsPanelOpen;
        bool isLightPanelOpen;
        bool isMaterialPanelOpen;
        bool isShaderPanelOpen;
    }ui;
// public:
//     UI ui;
//     void load(std::string path);
//     void save(std::string path);
};