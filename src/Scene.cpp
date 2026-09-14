#include "Scene.h"
#include <yaml-cpp/yaml.h>

#include "FileUtils.h"
#include "Entity.h"
#include "Transform.h"

std::string Scene::getName(){ return _name; }
std::string Scene::getPath(){ return _path; }

void Scene::load(std::string path)
{
    LOG_TRACE("Loading scene...");
    if(path.empty()){
        LOG_ERROR("The path is empty!");
        return;
    }
    
    clear();

    _path = path; 
    YAML::Node root = YAML::LoadFile(path);
    deserialize(root);

    LOG_TRACE("Scene was loaded.");
}
void Scene::save(std::string path) {
    LOG_TRACE("Scene is saving");
    if(path.empty()){
        LOG_ERROR("The path is empty!");
        return;
    }
    std::filesystem::path filePath(path); 

    // abc     -> extension != .scn => Add .scn
    // abc.a1  -> extension != .scn => Add .scn
    // .a1     -> extension != .scn => Add .scn 
    // abc.scn -> extension == .scn & stem != extension   -> OK
    // .scn    -> extension == .scn & stem == extension => LOG_ERROR & return

    bool isExtensionValid = filePath.extension() == ".scn";
    bool isEmptyStem = filePath.extension() == filePath.stem();
    if(isExtensionValid) {
        if(!isEmptyStem){
            _name = filePath.stem();
        }
        else{
            LOG_ERROR("The name is empty!"); 
            return;
        }
    }
    else{
        _name = filePath.filename();
        filePath += ".scn";
        path = filePath.string();
    }

    _path = path;

    YAML::Emitter out;
    serialize(out);
    
    // std::ofstream fout(path);
    // fout << out.c_str();
    FileUtils::writeFile(path, out.c_str());
    LOG_INFO("Scene saved");
}

Entity *Scene::addEntity(std::unique_ptr<Entity> entity)
{   
    Entity* e = entity.get();
    _entities.push_back(std::move(entity));
    return e;
}

void Scene::removeEntity(Entity* entity)
{   
    _entities.erase(std::remove_if(_entities.begin(), _entities.end(),
    [entity](const std::unique_ptr<Entity>& t) {
        return t.get() == entity;
    }), 
    _entities.end());
}

void Scene::clear()
{
    _entities.clear();
    _name = "";
    _path = "";
}

std::string Scene::getUniqueName(std::string name)
{
    int i{};
    std::string uniq_name = name; 
    while (!isUniqueName(uniq_name))    
        uniq_name = name + std::to_string(i++);
    
    return uniq_name;
}

bool Scene::isUniqueName(std::string name)
{
    for (const auto& entity : _entities)
        if (entity->name == name) return false;

    return true;
}

void Scene::serialize(YAML::Emitter& out)
{
    out << YAML::BeginDoc;
	out << YAML::BeginMap;

    out << YAML::Key << "Scene" << YAML::Value << getName();
    out << YAML::Key << "Version" << YAML::Value << "1.0";
    out << YAML::Key << "Entities" << YAML::Value;

    out << YAML::BeginSeq;
    for (const auto& entity : _entities) {
        entity->serialize(out);
    }
    out << YAML::EndSeq;

	out << YAML::EndMap;
    out << YAML::EndDoc;
}

void Scene::deserialize(const YAML::Node& node)
{
    auto sceneNameNode = node["Scene"];
    _name = sceneNameNode.as<std::string>();
     


    auto versionNode = node["Version"]; // dosya versiyonu


    // UUID -> Entity* eşleşmesi için geçici bir "adres defteri"
    std::unordered_map<uint64_t, Transform*> entityMap;
    auto entitiesNode = node["Entities"];

    for (const auto& entityNode : entitiesNode) {
        auto entity = std::make_unique<Entity>();
        entity->deserialize(entityNode);
        LOG_TRACE("Load entity: {}", entity->name);

        // parent child relationship için transformun uuid sini mapliyoruz.
        uint64_t id = entity->transform->UUID;
        entityMap[id] = entity->transform.get();

        _entities.emplace_back(std::move(entity));
    }




    // --- PASS 2: Aile Bağlarını (Parent-Child) Kur ---
    for (const auto& entityNode : entitiesNode) {
        auto transformNode = entityNode["transform"];
        if (transformNode["parentUUID"]) { // YAML'da bu anahtarı kaydettiğini varsayıyoruz
            uint64_t childID = transformNode["UUID"].as<uint64_t>();
            uint64_t parentID = transformNode["parentUUID"].as<uint64_t>();

            if (parentID != 0) { // 0 genelde "root" (ebeveynsiz) demektir
                Transform* child = entityMap[childID];
                Transform* parent = entityMap[parentID];

                if (child && parent) {
                    child->setParent(parent);
                    LOG_TRACE("Pass 2: Linked {} -> Parent: {}", child->owner->name, parent->owner->name);
                }
            }
        }
    }


}

