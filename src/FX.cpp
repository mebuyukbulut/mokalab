#include "FX.h"
#include "Shader.h"
#include "AssetManager.h"
#include "Logger.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

FXParam::FXParam(const FXParamDefinition* definition)
{
    this->definition = definition;
    value = definition->defaultVal;
    dirty = true;
}

void FXParam::update(Shader *shader)
{
    if(!dirty) return;
    
    std::visit(
        [&](const auto& val)
        {
            shader->set(definition->uniformName, val);
        },
        value
    );
    dirty = false; 
}

void FXParam::onInspect()
{
    if(getType() == FXParamType::Float){
        float* val = std::get_if<float>(&value);
        float min = std::get<float>(getMin());
        float max = std::get<float>(getMax());
        //float defaultVal = std::get<float>(getDefaultValue());

        if(ImGui::DragFloat(getLabel().c_str(), std::get_if<float>(&value), 0.025f, min, max))
            dirty = true;
    }
    else if(getType() == FXParamType::Vec2){
        glm::vec2* val = std::get_if<glm::vec2>(&value);
        float min = std::get<glm::vec2>(getMin()).x;
        float max = std::get<glm::vec2>(getMax()).x;

        if (ImGui::DragFloat2(getLabel().c_str(), glm::value_ptr(*std::get_if<glm::vec2>(&value)) , 1.0f, min, max))
            dirty = true;
    }
    
}



FXInstance::FXInstance(const FXInstanceDefinition& definition)
{
    builtinID = definition.builtinID;
    label = definition.label;
    for(const FXParamDefinition* pDef : definition.parameters){
        FXParam parameter(pDef);

        parameters.push_back(parameter);
    }
}

Shader* FXInstance::getShader() const {
    return g_Assets.get<Shader>(builtinID).get();
}
void FXInstance::update()
{
    g_Assets.get<Shader>(builtinID).get()->use();
    for(FXParam& param : parameters)
        param.update(g_Assets.get<Shader>(builtinID).get());
}

void FXInstance::onInspect()
{
    ImGui::SameLine();

    ImGui::Checkbox("##enabled", &enabled);

    ImGui::SameLine();

    ImGui::Text("%s", label.c_str());

    ImGui::SameLine(280);//(200);

    // ImGui::SetNextItemWidth(80);
    // ImGui::DragFloat("##opacity", &opacity, 0.01f, 0.0f, 1.0f);

    // ImGui::SameLine();
}

/////////////////////////////////////////////////
//              FX-Registry                    //
/////////////////////////////////////////////////

// Add a new FXInstance to FXStack /// AddFX could be more meaningful name
void FXRegistry::addInstance(int definitionIndex){
    FXStack.push_back(FXInstance{FXInstanceDefinitionStack[definitionIndex]});
}
void FXRegistry::addInstance(std::string builtinID){
    for(const FXInstanceDefinition& def : FXInstanceDefinitionStack){
        if(def.builtinID == builtinID){
            FXStack.push_back(FXInstance{def});
            return;
        }
    }
    LOG_ERROR("void FXRegistry::addInstance(std::string builtinID) : Given built in path not found in the InstanceDefinitionStack");
}

// Give all FXInstanceDefinitions' name list
std::vector<std::string> FXRegistry::getDefinitionList(){
    std::vector<std::string> nameList{};
    for(auto& definition : FXInstanceDefinitionStack)
        nameList.push_back(definition.label);
    return nameList;
}


void FXRegistry::init()
{
    // load all shaders from files
    std::string path{"../assets/shaders/postfx/"};
    for(auto& def : FXInstanceDefinitionStack){
        ShaderSettings ss{def.builtinID, path + def.vertexPath, path + def.fragmentPath};
        g_Assets.get<Shader>(ss.name, &ss);
    }


}

void FXRegistry::onInspect()
{

    ImGui::Begin("FX Stack");

    static int selectedDef = -1;
    std::vector<std::string> fxNames = getDefinitionList();

    // combo
    ImGui::SetNextItemWidth(200);

    if (ImGui::BeginCombo("##fxcombo", selectedDef == -1 ? "Select an Effect" : fxNames[selectedDef].c_str()))
    {
        for (int i = 0; i < fxNames.size(); i++)
        {
            bool isSelected = (selectedDef == i);

            if (ImGui::Selectable(fxNames[i].c_str(), isSelected))
                selectedDef = i;

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // add button
    if (ImGui::Button("+ Add Effect"))
    {
        if(selectedDef >= 0)
            addInstance(selectedDef);
    }


    ImGui::Separator();

    static int selectedIndex = -1;

    // scrollable area
    ImGui::BeginChild("fx_list", ImVec2(0, 200), true);
    for (int i = 0; i < FXStack.size(); i++)
    {
        ImGui::PushID(i);

        FXInstance& fx = FXStack[i];
        // select row
        bool selected = (selectedIndex == i);

        if (ImGui::Selectable("##row", selected, ImGuiSelectableFlags_AllowOverlap, ImVec2(0, 32)))
        {
            selectedIndex = i;
        }

        fx.onInspect();

        if (ImGui::SmallButton("X"))
        {   
            if(selectedIndex == i) selectedIndex--;
            FXStack.erase(FXStack.begin() + i);
            ImGui::PopID();
            break;
        }

        // for(FXParam& param : parameters)
        //     param.onInspect();


        ImGui::PopID();
    }

    ImGui::EndChild();


    // selected effect parameters
    if (selectedIndex >= 0 && selectedIndex < FXStack.size())
    {
        ImGui::Separator();

        auto& fx = FXStack[selectedIndex];
        fx.update();

        auto& paramVec = fx.getParameters();
        ImGui::Text("Parameters");
        for(auto& param : paramVec)
            param.onInspect();
    }

    ImGui::End();

}

const std::vector<FXInstance> FXRegistry::getActiveFXStack()
{
    std::vector<FXInstance> activeStack{};
    
    // for (FXInstance& instance : FXStack)
    //     instance.update();

    for (FXInstance instance : FXStack){
        if(instance.isActive())
            activeStack.push_back(instance);
    }
    
    return activeStack;
}

/////////////////////////////////////////////////
//          Instance Definition Stack          //
/////////////////////////////////////////////////

std::vector<FXInstanceDefinition> FXRegistry::FXInstanceDefinitionStack{
    FXInstanceDefinition{
        Builtin::FX::Grayscale,  
        "Grayscale", 
        "This is grayscale", 
        "fullscreen_tris.vert", "grayscale.frag", 
        {}},

    FXInstanceDefinition{
        Builtin::FX::PassThrough, 
        "Pass Through", 
        "tooltip", 
        "fullscreen_tris.vert", "passthrough.frag", 
        {}},

    FXInstanceDefinition{
        Builtin::FX::Invert,      
        "Invert", 
        "tooltip",  
        "fullscreen_tris.vert", "invert.frag", 
        {}},



    FXInstanceDefinition{
        Builtin::FX::Sepia,      
        "Sepia", 
        "tooltip",   
        "fullscreen_tris.vert", "sepia.frag", 
        {}},

    FXInstanceDefinition{
        Builtin::FX::Vignette,    
        "Vignette", 
        "tooltip",  
        "fullscreen_tris.vert", "vignette.frag", 
        {
        {&Builtin::FX::Params::Vignette_Amount}, 
        {&Builtin::FX::Params::Vignette_Distance}, 
        {&Builtin::FX::Params::Vignette_Feather}, 
        }
    },

    FXInstanceDefinition{
        Builtin::FX::GammaCorrection,
        "GammaCorrection", 
        "tooltip", 
        "fullscreen_tris.vert", "gamma_correction.frag", 
        {}}, // bunu da paremetreye bağlayabiliriz



    FXInstanceDefinition{
        Builtin::FX::Posterize,   
        "Posterize", 
        "tooltip",  
        "fullscreen_tris.vert", "posterize.frag", 
        {{&Builtin::FX::Params::Posterize_Values}}},

    FXInstanceDefinition{
        Builtin::FX::Pixelate,     
        "Pixelate", 
        "tooltip", 
        "fullscreen_tris.vert", "pixelate.frag", 
        {{&Builtin::FX::Params::Pixelate_Values}}},

    FXInstanceDefinition{
        Builtin::FX::BlurHorizontal,
        "BlurHorizontal", 
        "tooltip", 
        "fullscreen_tris.vert", "blur_horizontal.frag", 
        {}},

    FXInstanceDefinition{
        Builtin::FX::BlurVertical,
        "BlurVertical", 
        "tooltip", 
        "fullscreen_tris.vert", "blur_vertical.frag", 
        {}},

    FXInstanceDefinition{
        Builtin::FX::PrePost,
        "PrePost", 
        "tooltip", 
        "fullscreen_tris.vert", "pre_post.frag", 
        {}},

};
