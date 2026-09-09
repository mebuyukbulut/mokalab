#pragma once
#include <string>



class AppConfig
{
    struct Window {
        int width;
        int height;
        std::string title;
        bool fullscreen;
    };

    struct Graphics {    
    };
public:
    Window window;
    void load(std::string path = "../assets/config/config.yaml");
    void save(std::string path = "../assets/config/config.yaml");
};

class ProjectConfig{


    struct UI {
        bool isCreditsPanelOpen;
        bool isLightPanelOpen;
        bool isMaterialPanelOpen;
        bool isShaderPanelOpen;
    };
public:
    UI ui;
    void load(std::string path);
    void save(std::string path);
};