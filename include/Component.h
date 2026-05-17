#pragma once
#include "Object.h"
#include <functional>
#include <memory>
enum class ComponentType {
    None,
    Transform,
    Model,
    PointLight,
	DirectionalLight,
	SpotLight,
    Camera,
    Custom
};

class ComponentUtils {
private:
    // String -> Enum (Hash lookup: O(1))
    static inline std::unordered_map<std::string, ComponentType> stringToType = {
        {"None"              ,  ComponentType::None                 },
        {"Transform"         ,  ComponentType::Transform            },
        {"Model"             ,  ComponentType::Model                },
        {"PointLight"        ,  ComponentType::PointLight           },
        {"DirectionalLight"  ,  ComponentType::DirectionalLight     },
        {"SpotLight"         ,  ComponentType::SpotLight            },
        {"Camera"            ,  ComponentType::Camera               },
        {"Custom"            ,  ComponentType::Custom               },
    };

    // Enum -> String (Hash lookup: O(1))
    // Not: Enum değerlerin ardışıksa (0, 1, 2...) std::vector veya array kullanmak daha da hızlıdır.
    static inline std::unordered_map<ComponentType, std::string> typeToString = {
        {ComponentType::None               ,  "None"                },
        {ComponentType::Transform          ,  "Transform"           },
        {ComponentType::Model              ,  "Model"               },
        {ComponentType::PointLight         ,  "PointLight"          },
        {ComponentType::DirectionalLight   ,  "DirectionalLight"    },
        {ComponentType::SpotLight          ,  "SpotLight"           },
        {ComponentType::Camera             ,  "Camera"              },
        {ComponentType::Custom             ,  "Custom"              },
    };

public:
    static std::string ToString(ComponentType type) {
        if (typeToString.contains(type)) return typeToString[type];
        return "Unknown Component";
    }

    static ComponentType FromString(const std::string& str) {
        if (stringToType.contains(str)) return stringToType[str];
        return ComponentType::None;
    }
};


class Entity;
class Component : public Object {
public:
    Entity* owner{};
	ComponentType type{ ComponentType::None };

    virtual void onUpdate(float dt) {}

    // Inherited via Object
    virtual void serialize(YAML::Emitter& out) override;
    virtual void deserialize(const YAML::Node& node) override;
};


// Gerekirse bu sınıfı farklı dosyaya taşıyabiliriz
class ComponentFactory {
    static inline std::unordered_map<ComponentType, std::function<std::unique_ptr<Component>()> > _registry;

public:
    static void registerType(const ComponentType& name, std::function<std::unique_ptr<Component>()> creator) {
        _registry[name] = creator;
    }

    template <typename T>
    static std::unique_ptr<T> create(const ComponentType& name) {
        if (_registry.contains(name)) {
            std::unique_ptr<Component> baseComponent = _registry[name]();

            if(!baseComponent) return nullptr;

            T* derived = dynamic_cast<T*>(baseComponent.get());

            if(derived){
                baseComponent.release();
                return std::unique_ptr<T>(derived);
            }
        }
        return nullptr;
    }
};

