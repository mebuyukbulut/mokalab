#pragma once
#include <string>



class Config
{
    struct Window {
        int width;
        int height;
        std::string title;
        bool fullscreen;
    };

    struct Graphics {    
    };

    struct UI {
        bool isCreditsPanelOpen;
        bool isLightPanelOpen;
        bool isMaterialPanelOpen;
        bool isShaderPanelOpen;
    };
public:
    Window window;
    UI ui;
    void load(std::string path = "../assets/config/config.yaml");
    void save(std::string path = "../assets/config/config.yaml");
};