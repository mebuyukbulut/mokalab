#pragma once
#include <vector>
#include "Material.h"
#include "LightManager.h"

class Scene : public Object{

    std::string _name{};
    std::string _path{};
	std::vector<std::unique_ptr<Entity>> _entities{};

public:
    std::string getName();
    std::string getPath();

    void setName(std::string newName);

    inline const std::vector<std::unique_ptr<Entity>>& entities(){return _entities;};
    // Sahnenin sahip olduğu kaynaklar
    //std::vector<MaterialHandle> materials;
    //std::vector<Light> lights;

    // TODO: entity list, camera list, vs.

    void load(std::string path);   // YAML/JSON’dan yükle
    void save(std::string path);   // dosyaya yaz

    Entity* addEntity(std::unique_ptr<Entity> entity);
    void removeEntity(Entity* entity);
    void clear();

    std::string getUniqueName(std::string name);
    bool isUniqueName(std::string name);


    // Inherited via Object
    void serialize(YAML::Emitter& out) override;
    void deserialize(const YAML::Node& node) override;
};