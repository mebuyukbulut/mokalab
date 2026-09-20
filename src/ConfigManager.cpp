#include "ConfigManager.h"
#include <yaml-cpp/yaml.h>
#include "Config.h"
#include <fstream>
namespace YAML {

template<>
struct convert<AppConfig> {
    static Node encode(const AppConfig& rhs) {
        Node node;
        node["Window.width"     ] = rhs.window.width;
        node["Window.height"    ] = rhs.window.height;
        node["Window.title"     ] = rhs.window.title;
        node["Window.fullscreen"] = rhs.window.fullscreen;
        return node;
    }

    static bool decode(const Node& node, AppConfig& rhs) {
        if (!node.IsMap()) return false;
        
        if (node["Window.width"     ]) rhs.window.width        = node["Window.width"     ].as<int>();
        if (node["Window.height"    ]) rhs.window.height       = node["Window.height"    ].as<int>();
        if (node["Window.title"     ]) rhs.window.title        = node["Window.title"     ].as<std::string>();
        if (node["Window.fullscreen"]) rhs.window.fullscreen   = node["Window.fullscreen"].as<bool>();
        return true;
    }
};

template<>
struct convert<ProjectConfig> {
    static Node encode(const ProjectConfig& rhs) {
        Node node;
        node["UI.isCreditsPanelOpen"  ] = rhs.ui.isCreditsPanelOpen; 
        node["UI.isLightPanelOpen"    ] = rhs.ui.isLightPanelOpen;   
        node["UI.isMaterialPanelOpen" ] = rhs.ui.isMaterialPanelOpen;
        node["UI.isShaderPanelOpen"   ] = rhs.ui.isShaderPanelOpen;  
        return node;
    }

    static bool decode(const Node& node, ProjectConfig& rhs) {
        if (!node.IsMap()) return false;
        
        if (node["UI.isCreditsPanelOpen" ]) rhs.ui.isCreditsPanelOpen  = node["UI.isCreditsPanelOpen" ].as<bool>();
        if (node["UI.isLightPanelOpen"   ]) rhs.ui.isLightPanelOpen    = node["UI.isLightPanelOpen"   ].as<bool>();
        if (node["UI.isMaterialPanelOpen"]) rhs.ui.isMaterialPanelOpen = node["UI.isMaterialPanelOpen"].as<bool>();
        if (node["UI.isShaderPanelOpen"  ]) rhs.ui.isShaderPanelOpen   = node["UI.isShaderPanelOpen"  ].as<bool>();
        return true;
    }
};

} // namespace YAML



template<typename T>
bool ConfigManager::load(const std::filesystem::path& path, T& outConfig){
    try {
        YAML::Node root = YAML::LoadFile(path.string());
        outConfig = root.as<T>();
        return true;
    } catch (const std::exception& e) {
        // Loglama mekanizması
        return false;
    }
}


template<typename T>
bool ConfigManager::save(const std::filesystem::path& path, const T& config){
    try {
        YAML::Node root;
        root = config; // YAML::convert<T>::encode çağrılır
        std::ofstream fout(path);
        fout << root;
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}



// --- EXPLICIT INSTANTIATION ---
// Kullanılacak tüm tipleri burada derleyiciye bildiriyoruz:
template bool ConfigManager::load<AppConfig>(const std::filesystem::path&, AppConfig&);
template bool ConfigManager::load<ProjectConfig>(const std::filesystem::path&, ProjectConfig&);

template bool ConfigManager::save<AppConfig>(const std::filesystem::path&, const AppConfig&);
template bool ConfigManager::save<ProjectConfig>(const std::filesystem::path&, const ProjectConfig&);