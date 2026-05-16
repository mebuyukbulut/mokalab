#include "RenderComponent.h"
#include "yaml-cpp/yaml.h"
#include "AssetManager.h"
#include "Builtin.h"

// TO DO Material Manager sıkıntısın hallet
const bool RenderComponent::registered = []() {
    ComponentFactory::registerType(ComponentType::Model, []() { return new RenderComponent(); });
    return true;
    }();




void RenderComponent::onInspect()
{
    _model->onInspect();
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
    std::string path = node["path"].as<std::string>();
    node["path"].IsNull() ? path = "" : path = node["path"].as<std::string>();

    std::cout << "My way : " << path << std::endl;
    if (path == "") {
        //g_Assets.get<Model>(Builtin::Model::Default);
        // _shape = static_cast<DefaultShapes>(node["shape"].as<int>());
        // loadDefault(_shape);
        // std::cout << " default loading..." << std::endl;
    }
    else
        _model = g_Assets.get<Model>(path);

}
