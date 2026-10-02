#include "UI/MaterialBrowser.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>


#include "EngineContext.h"
#include "AssetManager.h"


void MaterialBrowser::draw(){
    if(!ImGui::Begin("Material BROWSER"))
    {
        ImGui::End(); 
        return;
    }

    //if(mat.isValid())



    auto material = mat.resolve(ece);

    ImGui::SeparatorText("MATERIAL");
	ImGui::ColorEdit4("baseColor", glm::value_ptr(material->baseColor), ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel);

	ImGui::DragFloat("metallic", &material->metallic, 0.02f, 0.0f, 1.0f);
	ImGui::DragFloat("roughness", &material->roughness, 0.02f, 0.0f, 1.0f);
	ImGui::DragFloat("reflectance", &material->reflectance, 0.02f, 0.0f, 1.0f);
	ImGui::DragFloat("ao", &material->ao, 0.02f, 0.0f, 1.0f);
	ImGui::ColorEdit4("emissive", glm::value_ptr(material->emissive), ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel);



    ImGui::End();
}
void MaterialBrowser::init(EngineContext* ece){
    this->ece = ece;
}
