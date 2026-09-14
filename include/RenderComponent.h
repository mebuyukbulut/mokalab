#pragma once
#include <memory>
#include "Component.h"
#include "Material.h"
#include "Model.h"

struct EngineContext; 

class RenderComponent : public Component
{
    static const bool registered;
    EngineContext* ece{};
    std::string _path;

public:
    std::shared_ptr<Model> _model; 

    RenderComponent() { type = ComponentType::Model; }

    void onInspect() override;
    void resolveAssets(class AssetManager&) override;
    void serialize(YAML::Emitter& out) override;
    void deserialize(const YAML::Node& node) override;
};

