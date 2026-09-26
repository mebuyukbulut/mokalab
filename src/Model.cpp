#include "Model.h"
#include "Texture.h"
#include "YAMLHelper.h"

#include "Logger.h"
#include <filesystem>
#include "AssetManager.h"
#include "Builtin.h"

#include <imgui.h>
#include "EngineContext.h"

Model::Model(EngineContext *ece) { 
    _type = AssetType::Model; 
    this->ece = ece; 
    fallbackMaterial = ece->assets->get<Material>(Builtin::Material::DefaultMaterial);
}
// draws the model, and thus all its meshes
void Model::draw(Shader* shader, bool bindMaterial) {
    if (_loadStatus != AssetLoadStatus::Complete) return; 

    //if (shader->_type == Shader::Type::Foreground)
    if(bindMaterial)
        if (_materials.size())
            _materials[0].resolve(ece)->use(shader);
        else
            fallbackMaterial.resolve(ece)->use(shader);  // her seferinde bunu sormasına gerek yok. initialization kısmında bunu default olarak alması lazım. 

    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].draw(shader);
}



void Model::purgeCPU()
{
    for (auto& mesh : meshes) {
        mesh.terminate();
    }
}

void Model::uploadGPU()
{
    // herhangi bir mesh fail olabilir. error durumunu handle et!
    for (auto& m : meshes)
        m.upload2GPU();
    _loadStatus = AssetLoadStatus::Complete;
}

#include <iostream>
void Model::onInspect(){
    if (!_materials.size()) return;
    
    // auto mats = g_Assets.getAll<Material>();
    // std::cout << mats.size() << std::endl;
    if(std::string path = EditorUI::materialSelector(ece); path != ""){
        _materials.clear();
        _materials.push_back(ece->assets->get<Material>(path));
    }

    Material* mat = _materials.front().resolve(ece).get();

    ImGui::Text(mat->name.c_str());
    mat->onInspect();
}
