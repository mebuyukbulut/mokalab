#pragma once

#include <string>
#include <variant>
#include <vector>
#include <glm/glm.hpp>
#include "Builtin.h"
#include "IInspectable.h"

class Shader;
struct EngineContext; 

using FXValue = std::variant<
    float,
    int,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    bool
>;

enum class FXParamType
{
    Float,
    Int,
    Bool,
    Vec2,
    Vec3,
    Vec4,
};

struct FXParamDefinition{
    std::string uniformName{};
    std::string label{};
    std::string tooltip{};
    FXParamType type;

    FXValue min;
    FXValue max;
    FXValue defaultVal; 

    template<typename T>
    FXParamDefinition(std::string uniformName, std::string label, std::string tooltip, FXParamType type, T min, T max, T defaultVal)
    :uniformName{uniformName}, label{label}, tooltip{tooltip}, type{type}, min{min}, max{max}, defaultVal{defaultVal}{}
};

struct FXParam : public IInspectable{
    const FXParamDefinition* definition = nullptr;
    bool dirty{true};
    FXValue value; 

    FXParam() = default;
    FXParam(const FXParamDefinition* definition);

    void update(Shader* shader);

    template<typename T> 
    void setValue(T newValue){ value = newValue; dirty = true; }


    std::string getUniformName(){ return definition->uniformName; }
    std::string getLabel(){ return definition->label; }
    std::string getTooltip(){ return definition->tooltip; }

    FXParamType getType(){ return definition->type; }

    FXValue getValue(){ return value; }
    FXValue getMin(){ return definition->min; }
    FXValue getMax(){ return definition->max; }
    FXValue getDefaultValue(){ return definition->defaultVal; }

    void onInspect() override;

};

struct FXInstanceDefinition{
    std::string builtinID{};    // builtin::fx::grayscale 
    std::string label{}; 
    std::string tooltip{};
    std::string vertexPath{};   // ./assets/fx/...
    std::string fragmentPath{}; // ./assets/fx/...
    std::vector<const FXParamDefinition*> parameters{};

    FXInstanceDefinition() = default;
    FXInstanceDefinition(
        std::string builtinID, 
        std::string label,
        std::string tooltip,
        std::string vertexPath, std::string fragmentPath, 
        std::vector<const FXParamDefinition*> parameters)
        :
        builtinID{builtinID}, 
        label{label},
        tooltip{tooltip},
        vertexPath{vertexPath}, fragmentPath{fragmentPath}, 
        parameters{parameters}{}
};

class FXInstance : public IInspectable{
    bool enabled{true};
    float opacity{1.0f};
    std::string builtinID{};
    std::string label{}; 
    std::vector<FXParam> parameters{}; 
    EngineContext* ece{};
public:
    FXInstance(EngineContext* ece);
    FXInstance(const FXInstanceDefinition& definition, EngineContext* ece);

    Shader* getShader() const;
    std::vector<FXParam>& getParameters(){ return parameters; }

    bool isActive(){return enabled;}
    void update();
    void onInspect() override;
    
};

class FXRegistry : public IInspectable{
    static std::vector<FXInstanceDefinition> FXInstanceDefinitionStack;
    std::vector<FXInstance> FXStack{};
    EngineContext* ece{}; 

    void addInstance(int definitionIndex);
    void addInstance(std::string builtinID);

    std::vector<std::string> getDefinitionList();
public:
    void init(EngineContext& ece); // init all shaders 
    void onInspect() override;

    const std::vector<FXInstance> getActiveFXStack();
    
};

namespace Builtin
{
    namespace FX
    {
        namespace Params{
            // uniformName, labelName, tooltip, type, min, max, default
            //inline const FXParamDefinition MyFloat {"u_float", "value:", "This is a float", FXParamType::Float,  0.0f, 1.0f, 0.5f};
            inline const FXParamDefinition Pixelate_Values {
                "u_value2D", 
                "Pixel Count", 
                "Total pixel count in a row and column.", 
                FXParamType::Vec2,  
                glm::vec2(0, 0), glm::vec2(2048, 2048), glm::vec2(320,180)};

            inline const FXParamDefinition Posterize_Values {
                "u_value", 
                "Levels", 
                "Step count between absolute min value to absolute max value for each color channel.", 
                FXParamType::Float,  
                2.0f, 255.0f , 5.0f};

            inline const FXParamDefinition Vignette_Amount {
                "u_amount", 
                "Amount", 
                "Amount of darkness.", 
                FXParamType::Float,  
                0.0f, 1.0f , 0.5f};
            inline const FXParamDefinition Vignette_Distance {
                "u_distance", 
                "Distance", 
                "Where to start.", 
                FXParamType::Float,  
                0.0f, 1.0f , 0.5f};
            inline const FXParamDefinition Vignette_Feather {
                "u_feather", 
                "Feathering", 
                "Smoothness between light and dark areas.", 
                FXParamType::Float,  
                0.0f, 1.0f , 0.5f};

        }
    }
}


// {Builtin::FX::Grayscale,     "../assets/shaders/postfx/fullscreen_tris.vert", "../assets/shaders/postfx/grayscale.frag"},
// {Builtin::FX::PassThrough,   "../assets/shaders/postfx/fullscreen_tris.vert", "../assets/shaders/postfx/passthrough.frag"},
// {Builtin::FX::Invert,   "../assets/shaders/postfx/fullscreen_tris.vert", "../assets/shaders/postfx/invert.frag"},

// {Builtin::FX::Sepia,   "../assets/shaders/postfx/fullscreen_tris.vert", "../assets/shaders/postfx/sepia.frag"},
// {Builtin::FX::Vignette,   "../assets/shaders/postfx/fullscreen_tris.vert", "../assets/shaders/postfx/vignette.frag"},
// {Builtin::FX::GammaCorrection,   "../assets/shaders/postfx/fullscreen_tris.vert", "../assets/shaders/postfx/gamma_correction.frag"},

// {Builtin::FX::Posterize,   "../assets/shaders/postfx/fullscreen_tris.vert", "../assets/shaders/postfx/posterize.frag"},
// {Builtin::FX::Pixelate,   "../assets/shaders/postfx/fullscreen_tris.vert", "../assets/shaders/postfx/pixelate.frag"},