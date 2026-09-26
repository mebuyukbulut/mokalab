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



	// glm::vec4 baseColor;
	// glm::vec4 emissive;
	// float metallic   ;
	// float roughness  ;
	// float reflectance;
	// float ao         ;





Material::Material() :
	baseColor{ 1.0f, 1.0f, 1.0f, 1.0f },
	emissive{ 0.0f, 0.0f, 0.0f, 0.0f },
	metallic{ 0.0f },
	roughness{ 0.5f },
	reflectance{ 0.45f },
	ao{ 1.f }
{
	if(!defaultWhite.isValid()) defaultWhite = ece->assets->get<Texture>(Builtin::Texture::SolidWhite);
	if(!defaultNormal.isValid()) defaultNormal = ece->assets->get<Texture>(Builtin::Texture::FlatNormal);
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
	if(baseColorTexture.isValid() && baseColorTexture.resolve(ece)->isLoaded())
		baseColorTexture.resolve(ece)->bind(Builtin::TextureSlot::BaseColor);
	else
		defaultWhite.resolve(ece)->bind(Builtin::TextureSlot::BaseColor);

	if(armTexture.isValid() && armTexture.resolve(ece)->isLoaded())
		armTexture.resolve(ece)->bind(Builtin::TextureSlot::ARM);
	else
		defaultWhite.resolve(ece)->bind(Builtin::TextureSlot::ARM);

	if(normalTexture.isValid() && normalTexture.resolve(ece)->isLoaded())
		normalTexture.resolve(ece)->bind(Builtin::TextureSlot::Normal);
	else
		defaultNormal.resolve(ece)->bind(Builtin::TextureSlot::Normal);

	if(emissiveTexture.isValid() && emissiveTexture.resolve(ece)->isLoaded())
		emissiveTexture.resolve(ece)->bind(Builtin::TextureSlot::Emissive);
	else
		defaultWhite.resolve(ece)->bind(Builtin::TextureSlot::Emissive);
		
	// (baseColorTexture.isValid() ? baseColorTexture : defaultWhite ).resolve(ece)->bind(Builtin::TextureSlot::BaseColor);
	// (armTexture.isValid()	  	? armTexture : 	     defaultWhite ).resolve(ece)->bind(Builtin::TextureSlot::ARM);
	// (normalTexture.isValid()	? normalTexture :    defaultNormal).resolve(ece)->bind(Builtin::TextureSlot::Normal);
	// (emissiveTexture.isValid() 	? emissiveTexture :  defaultWhite ).resolve(ece)->bind(Builtin::TextureSlot::Emissive);



	shader->set(Builtin::Material::BaseColor, 	 	baseColor);
	shader->set(Builtin::Material::Emissive, 	 	emissive);
	shader->set(Builtin::Material::Metallic, 	 	metallic);
	shader->set(Builtin::Material::Roughness, 	 	roughness);
	shader->set(Builtin::Material::Reflectance,  	reflectance);
	shader->set(Builtin::Material::AO, 			 	ao);
}

void Material::purgeCPU()
{ // Buna muhtemelen gerek yok. 
}

void Material::uploadGPU()
{ // Buna da muhtemelen gerek yok. 
}

// void Material::save(std::filesystem::path path, IAssetSettings* settings)
// {
//     try {
//         YAML::Node root;
//         root = *this; // YAML::convert<T>::encode çağrılır
//         std::ofstream fout(path);
//         fout << root;
//     } catch (const std::exception& e) {
//         LOG_ERROR("Material cannot be saved to the file: {}", path.string());		
//     }
// }

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

void Material::setBaseColorTexture(AssetHandle<Texture> texture){
	baseColorTexture = texture;
}
void Material::setArmTexture(AssetHandle<Texture> texture){
	armTexture = texture;
}
void Material::setNormalTexture(AssetHandle<Texture> texture){
	normalTexture = texture;
}
void Material::setEmissiveTexture(AssetHandle<Texture> texture){
	emissiveTexture = texture;
}

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
