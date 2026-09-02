// EngineContext.h
#pragma once

class PathResolver;
class Config;
class AssetManager;
class EventDispatcher;

struct EngineContext {
    PathResolver&    paths;
    Config&          config;
    AssetManager&    assets;
    EventDispatcher& dispatcher;
};