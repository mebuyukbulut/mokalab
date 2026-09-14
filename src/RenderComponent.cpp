#include "RenderComponent.h"
#include "yaml-cpp/yaml.h"
#include "AssetManager.h"
#include "EngineContext.h"
#include "Builtin.h"

// TO DO Material Manager sıkıntısın hallet
const bool RenderComponent::registered = []() {
    ComponentFactory::registerType(ComponentType::Model, []() { return std::make_unique<RenderComponent>(); });
    return true;
    }();




void RenderComponent::onInspect()
{
    _model->onInspect();
}

void RenderComponent::resolveAssets(AssetManager & assets)
{
    std::cout << "My way : " << _path << std::endl;
    if (_path == "") {
        //g_Assets.get<Model>(Builtin::Model::Default);
        // _shape = static_cast<DefaultShapes>(node["shape"].as<int>());
        // loadDefault(_shape);
        // std::cout << " default loading..." << std::endl;
    }
    else
        _model = assets.get<Model>(_path);
        //_model = ece->assets.get<Model>(_path);
}

void RenderComponent::serialize(YAML::Emitter& out)
{

    out << YAML::BeginMap;
    Component::serialize(out);
    out << YAML::Key << "path" << YAML::Value << _model->getPath();
    //out << YAML::Key << "shape" << YAML::Value << static_cast<int>(_shape);
    out << YAML::EndMap;
}

void RenderComponent::deserialize(const YAML::Node& node)
{
    Component::deserialize(node);
    _path = node["path"].as<std::string>(); // bu gereksiz olabilir
    node["path"].IsNull() ? _path = "" : _path = node["path"].as<std::string>();
}
