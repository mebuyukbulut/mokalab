#include "EngineContext.h"
#include <filesystem>
#include "AssetManager.h"
#include "PathResolver.h"
#include "ConfigManager.h"
#include "Config.h"
#include "EventDispatcher.h"

void EngineContext::init(std::string projectPath)
{
    assets = std::make_unique<AssetManager>();
    assets->setContext(this);

    paths = std::make_unique<PathResolver>();
    paths->setProjectRoot(projectPath);

    appConfig = std::make_unique<AppConfig>();
    projectConfig = std::make_unique<ProjectConfig>();

    ConfigManager::load<AppConfig>(paths->appConfig, *appConfig);
    ConfigManager::load<ProjectConfig>(paths->projectConfig, *projectConfig);

    dispatcher = std::make_unique<EventDispatcher>();
}