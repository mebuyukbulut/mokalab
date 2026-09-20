// EngineContext.h
#pragma once
#include <memory>
#include <string>

class PathResolver;
class AssetManager;
class EventDispatcher;
struct AppConfig;
struct ProjectConfig;

struct EngineContext {
    std::unique_ptr<AppConfig>       appConfig;
    std::unique_ptr<ProjectConfig>   projectConfig;
    std::unique_ptr<PathResolver>    paths;
    std::unique_ptr<AssetManager>    assets;
    std::unique_ptr<EventDispatcher> dispatcher;

    void init(std::string projectPath);
};