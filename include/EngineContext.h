// EngineContext.h
#pragma once

class PathResolver;
class AppConfig;
class ProjectConfig;
class AssetManager;
class EventDispatcher;

struct EngineContext {
    PathResolver&    paths;
    AppConfig&       appConfig;
    ProjectConfig&   projectConfig;
    AssetManager&    assets;
    EventDispatcher& dispatcher;
};