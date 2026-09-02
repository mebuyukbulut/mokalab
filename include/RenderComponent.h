#pragma once
#include "Component.h"
#include "Material.h"
#include <memory>
#include "Model.h"

struct EngineContext; 

class RenderComponent : public Component
{
    static const bool registered;
    EngineContext* ece{};
public:
    std::shared_ptr<Model> _model; 

    RenderComponent() { type = ComponentType::Model; }

    void onInspect() override;
    void serialize(YAML::Emitter& out) override;
    void deserialize(const YAML::Node& node) override;
};

