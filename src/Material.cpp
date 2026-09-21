#include "Material.h"
#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <assimp/types.h>

#include <glad/gl.h>
#include "Texture.h"
#include "Logger.h"
#include "Builtin.h"

#include "AssetManager.h"
#include "EngineContext.h"

#include <fstream>
#include <yaml-cpp/yaml.h>


	// glm::vec4 baseColor;
	// glm::vec4 emissive;
	// float metallic   ;
	// float roughness  ;
	// float reflectance;
	// float ao         ;


namespace YAML {
// --- glm::vec4 ---
template<>
struct convert<glm::vec4> {
	// Deserialize (YAML Node -> glm::vec4)
	static bool decode(const Node& node, glm::vec4& rhs) {
		if (!node.IsSequence() || node.size() != 4) {
			return false;
		}

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		rhs.w = node[3].as<float>();
		return true;
	}

	// Serialize (glm::vec4 -> YAML Node)
	static Node encode(const glm::vec4& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		node.push_back(rhs.w);
		node.SetStyle(EmitterStyle::Flow); // [x, y, z, w] formatında yazar
		return node;
	}
};

template<>
struct convert<Material> {

    static Node encode(const Material& rhs) {
        Node node;
        node["name"		  ] = rhs.name		 ;
        node["baseColor"  ] = rhs.baseColor	 ;
        node["emissive"   ] = rhs.emissive	 ;
        node["metallic"   ] = rhs.metallic	 ;
        node["roughness"  ] = rhs.roughness  ;
        node["reflectance"] = rhs.reflectance;
        node["ao"         ] = rhs.ao		 ;
        return node;
    }

    static bool decode(const Node& node, Material& rhs) {
        if (!node.IsMap()) return false;
        
        if (node["name"		  ]) rhs.name		 = node["name"		 ].as<std::string>();
        if (node["baseColor"  ]) rhs.baseColor	 = node["baseColor"  ].as<glm::vec4>();
        if (node["emissive"   ]) rhs.emissive	 = node["emissive"   ].as<glm::vec4>();
        if (node["metallic"   ]) rhs.metallic	 = node["metallic"   ].as<float>();
        if (node["roughness"  ]) rhs.roughness 	 = node["roughness"  ].as<float>();
        if (node["reflectance"]) rhs.reflectance = node["reflectance"].as<float>();
        if (node["ao"         ]) rhs.ao			 = node["ao"         ].as<float>();
        return true;
    }
};


} // namespace YAML




Material::Material() :
	baseColor{ 1.0f, 1.0f, 1.0f, 1.0f },
	emissive{ 0.0f, 0.0f, 0.0f, 0.0f },
	metallic{ 0.0f },
	roughness{ 0.5f },
	reflectance{ 0.45f },
	ao{ 1.f }
{
	if(!defaultWhite) defaultWhite = ece->assets->get<Texture>(Builtin::Texture::SolidWhite);
	if(!defaultNormal) defaultNormal = ece->assets->get<Texture>(Builtin::Texture::FlatNormal);
}

void Material::use(Shader *shader)
{
    // Set Texture location uniforms 
	// We should do this for once. Because we always bind the same slot.  
	shader->set(Builtin::Material::BaseColorTexture, Builtin::TextureSlot::BaseColor); 
	shader->set(Builtin::Material::ARMTexture, 		 Builtin::TextureSlot::ARM); 
	shader->set(Builtin::Material::NormalTexture,	 Builtin::TextureSlot::Normal); 
	shader->set(Builtin::Material::EmissiveTexture,  Builtin::TextureSlot::Emissive); 


	// Bind Textures
	(baseColorTexture ? baseColorTexture : defaultWhite )->bind(Builtin::TextureSlot::BaseColor);
	(armTexture 	  ? armTexture : 	   defaultWhite )->bind(Builtin::TextureSlot::ARM);
	(normalTexture	  ? normalTexture :    defaultNormal)->bind(Builtin::TextureSlot::Normal);
	(emissiveTexture  ? emissiveTexture :  defaultWhite )->bind(Builtin::TextureSlot::Emissive);



	shader->set(Builtin::Material::BaseColor, 	 	baseColor);
	shader->set(Builtin::Material::Emissive, 	 	emissive);
	shader->set(Builtin::Material::Metallic, 	 	metallic);
	shader->set(Builtin::Material::Roughness, 	 	roughness);
	shader->set(Builtin::Material::Reflectance,  	reflectance);
	shader->set(Builtin::Material::AO, 			 	ao);
}

void Material::loadDefault(std::string path)
{
	if(path == Builtin::Material::DefaultMaterial){
		name = "Default Material";
	}
	else if(path == Builtin::Material::DefaultMetal){
		name = "Default Metal"; 
		metallic = 1.0f; 
		roughness = 0.3;
	}
	else if(path == Builtin::Material::PlasticRed){
		name = "Plastic Red";
		baseColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); 
		roughness = 0.25;
	}
	else if(path == Builtin::Material::PlasticBlue){
		name = "Plastic Blue";
		baseColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); 
		roughness = 0.25;
	}
	else if(path == Builtin::Material::BoxCrate){
		name = "Wood Crate";
		//baseColorTexture = ece->assets.get<Texture>("../assets/textures/box_crate.jpg");
		roughness = 0.75;
	}
	else{
		LOG_ERROR("The internal material path is wrong or unimplemented", path);
	}
	_loadStatus = AssetLoadStatus::Complete;
}

void Material::load(std::filesystem::path path, IAssetSettings* settings)
{

    _path = path; // bunu Model::Load da yapabiliriz belki. 
    std::string pathStr = path.string(); 

    // Sanal yolla model yükleme
    for(const char* key : Builtin::Material::All){
        if(key == pathStr){
            loadDefault(pathStr);
            return;
        }
    }


	// dosyadan okuyup material oluştur. 
	try {
        YAML::Node root = YAML::LoadFile(path.string());
        Material m = root.as<Material>();
		name = m.name;
		baseColor = m.baseColor;
		emissive = m.emissive;
		metallic = m.metallic;
		roughness = m.roughness;
		reflectance = m.reflectance;
		ao = m.ao; 
    } catch (const std::exception& e) {
        LOG_ERROR("Material cannot load from file: {}", path.string());
    }
}

void Material::unload()
{ // Buna muhtemelen gerek yok. 
}

void Material::uploadToGPU()
{ // Buna da muhtemelen gerek yok. 
}

void Material::save(std::filesystem::path path)
{
    try {
        YAML::Node root;
        root = this; // YAML::convert<T>::encode çağrılır
        std::ofstream fout(path);
        fout << root;
    } catch (const std::exception& e) {
        LOG_ERROR("Material cannot be saved to the file: {}", path.string());		
    }
}

void Material::onInspect()
{
	ImGui::SeparatorText("MATERIAL");
	ImGui::ColorEdit4("baseColor", glm::value_ptr(baseColor), ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel);

	ImGui::DragFloat("metallic", &metallic, 0.02f, 0.0f, 1.0f);
	ImGui::DragFloat("roughness", &roughness, 0.02f, 0.0f, 1.0f);
	ImGui::DragFloat("reflectance", &reflectance, 0.02f, 0.0f, 1.0f);
	ImGui::DragFloat("ao", &ao, 0.02f, 0.0f, 1.0f);
	ImGui::ColorEdit4("emissive", glm::value_ptr(emissive), ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel);
}

void Material::setBaseColorTexture(std::shared_ptr<Texture> texture){
	baseColorTexture = texture;
}
void Material::setArmTexture(std::shared_ptr<Texture> texture){
	armTexture = texture;
}
void Material::setNormalTexture(std::shared_ptr<Texture> texture){
	normalTexture = texture;
}
void Material::setEmissiveTexture(std::shared_ptr<Texture> texture){
	emissiveTexture = texture;
}

//
//// bu bindingler material sınıfında olmalı. 
//if (shader._type == Shader::Type::Foreground) {
//    // bind appropriate textures
//    unsigned int diffuseNr = 1;
//    unsigned int specularNr = 1;
//    unsigned int normalNr = 1;
//    unsigned int heightNr = 1;
//    for (unsigned int i = 0; i < _textures.size(); i++)
//    {
//        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
//        // retrieve texture number (the N in diffuse_textureN)
//        std::string number;
//        std::string name = _textures[i].get()->_type;
//        if (name == "texture_diffuse")
//            number = std::to_string(diffuseNr++);
//        else if (name == "texture_specular")
//            number = std::to_string(specularNr++); // transfer unsigned int to string
//        else if (name == "texture_normal")
//            number = std::to_string(normalNr++); // transfer unsigned int to string
//        else if (name == "texture_height")
//            number = std::to_string(heightNr++); // transfer unsigned int to string
//
//        // now set the sampler to the correct texture unit
//    //shader.setInt(name + number, i);
//        //std::cout << "texture name: " << (name+number) << std::endl;
//
//        // and finally bind the texture
//        glBindTexture(GL_TEXTURE_2D, _textures[i].id);
//    }
//}


// Material::DefaultTextures::DefaultTextures()
// {
// 	white  = ece->assets.get<Texture>(Builtin::Texture::SolidWhite);
// 	black  = ece->assets.get<Texture>(Builtin::Texture::SolidBlack);
// 	normal = ece->assets.get<Texture>(Builtin::Texture::FlatNormal);
// }

std::string EditorUI::materialSelector(EngineContext* ece)
{
    static int selectedMat = -1;
	auto mats = ece->assets->getAll<Material>();


    std::vector<std::string> matNames;
	matNames.reserve(mats.size()); 

	std::transform(mats.begin(), mats.end(), std::back_inserter(matNames),
		[](const std::shared_ptr<Material>& mat){
			return mat->name; // name
		});




	ImGui::SeparatorText("MATERIAL SELECTOR");
    // combo
    ImGui::SetNextItemWidth(200);

    if (ImGui::BeginCombo("##fxcombo", selectedMat == -1 ? "Select a Material" : matNames[selectedMat].c_str()))
    {
        for (int i = 0; i < matNames.size(); i++)
        {
            bool isSelected = (selectedMat == i);

            if (ImGui::Selectable(matNames[i].c_str(), isSelected))
                selectedMat = i;

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    // ImGui::SameLine();

    // add button
    if (ImGui::Button("Apply"))
    {
        if(selectedMat >= 0)
			return mats.at(selectedMat)->getPath();
    }


    // ImGui::Separator();
    return std::string();
}
