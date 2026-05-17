#include "Entity.h"
#include "Transform.h"
#include "RenderComponent.h"
#include "Logger.h"

#include "YAMLHelper.h"

Entity::Entity()
{
    transform = std::make_unique<Transform>();
    transform->owner = this;
}

Entity::~Entity() {
    //terminate();
}

void Entity::addComponent(std::unique_ptr<Component> component)
{
    if (component) {
		component->owner = this;
	    components.emplace_back(std::move(component));
    }
}

void Entity::onInspect()
{
    if (!transform) {
		LOG_ERROR("Entity has no transform!");
        return;
    }
    transform->onInspect();
    for(auto& component : components)
		component->onInspect();
}

void Entity::serialize(YAML::Emitter& out)
{
    out << YAML::BeginMap; 
	Object::serialize(out);

	out << YAML::Key << "transform" << YAML::Value;
	transform->serialize(out);

    out << YAML::Key << "components" << YAML::Value;
    out <<YAML::BeginSeq;
    for (const auto& component : components)
		component->serialize(out);
	out << YAML::EndSeq;

    out << YAML::EndMap;
}

void Entity::deserialize(const YAML::Node& node)
{

    Object::deserialize(node);
    auto transformNode = node["transform"];
    if (transformNode && transformNode.IsDefined()) {
        transform->deserialize(transformNode);
    }
    else {
        LOG_ERROR("Entity has no transform node!");
    }


    auto componentsNode = node["components"];

    if (!componentsNode || !componentsNode.IsDefined()) {
        LOG_ERROR("{} has no component!", name);
    }   
    
    for (const auto& componentNode : componentsNode) {
        std::string typeStr = componentNode["componentType"].as<std::string>();
        ComponentType type = ComponentUtils::FromString(typeStr);

        if (ComponentType::Model == type) {
            std::unique_ptr<RenderComponent> renderComponent(static_cast<RenderComponent*>(ComponentFactory::create(type).release()));
            // Component* a = ComponentFactory::create(type);
            // RenderComponent* renderComponent = dynamic_cast<RenderComponent*>(a);
            renderComponent->deserialize(componentNode);
            addComponent(std::move(renderComponent));
        }
        else {
            std::unique_ptr<Component> aComponent = ComponentFactory::create(type);
            //Component* a = ComponentFactory::create(type);
            aComponent->deserialize(componentNode);
            addComponent(std::move(aComponent));
        }

        //std::unique_ptr<Component> component;
        //switch (type) {
        //case ComponentType::Transform:
        //    component = std::make_unique<Transform>();
        //    break;
        //// Diğer bileşen türleri için benzer case blokları ekleyin
        //default:
        //    LOG_WARNING("Unknown component type: " + typeStr);
        //    continue; // Bilinmeyen bileşen türlerini atla
        //}
        //if (component) {
        //    component->deserialize(componentNode);
        //    addComponent(std::move(component));
        //}
    }
	

}

