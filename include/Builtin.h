#pragma once

namespace Builtin
{
    namespace Material{
        inline constexpr const char* DefaultMaterial = "builtin::materials::defaultMaterial";
        inline constexpr const char* DefaultMetal = "builtin::materials::defaultMetal";
        inline constexpr const char* PlasticRed = "builtin::materials::plasticRed";
        inline constexpr const char* PlasticBlue = "builtin::materials::plasticBlue";
        inline constexpr const char* BoxCrate = "builtin::materials::boxCrate";

        inline constexpr const char* All[] = {
            DefaultMaterial,       
            DefaultMetal,       
            PlasticRed,   
            PlasticBlue,   
            BoxCrate,   
        };


        inline constexpr const char* BaseColor   = "material.baseColor";
        inline constexpr const char* Metallic    = "material.metallic";
        inline constexpr const char* Roughness   = "material.roughness";
        inline constexpr const char* AO          = "material.ao";
        inline constexpr const char* Emissive    = "material.emissive";
        inline constexpr const char* Reflectance = "material.reflectance";

        inline constexpr const char* BaseColorTexture   = "baseColorMap";
        inline constexpr const char* ARMTexture         = "armMap";
        inline constexpr const char* NormalTexture      = "normalMap";
        inline constexpr const char* EmissiveTexture    = "emissiveMap";

    }
    namespace TextureSlot
    {
        inline constexpr int BaseColor = 0;
        inline constexpr int ARM       = 1;
        inline constexpr int Normal    = 2;
        inline constexpr int Emissive  = 3;
        inline constexpr int DirectionalShadowMap = 4;
    }
    namespace Texture
    {
        inline constexpr const char* SolidWhite = "builtin::texture::solidWhite";
        inline constexpr const char* SolidBlack = "builtin::texture::solidBlack";
        inline constexpr const char* FlatNormal = "builtin::texture::flatNormal";

        inline constexpr const char* All[] = {
            SolidWhite,       
            SolidBlack,       
            FlatNormal,   
        };
    }


    namespace Model
    {
        inline constexpr const char* Cube        = "builtin::models::cube";
        inline constexpr const char* Cone        = "builtin::models::cone";
        inline constexpr const char* Cylinder    = "builtin::models::cylinder";
        inline constexpr const char* Sphere      = "builtin::models::sphere";
        inline constexpr const char* Plane       = "builtin::models::plane";
        inline constexpr const char* Torus       = "builtin::models::torus";

        inline constexpr const char* BgPlane     = "builtin::models::bgPlane";
        inline constexpr const char* GridPlane   = "builtin::models::gridPlane";

        inline constexpr const char* LightArrow  = "builtin::models::lightArrow";
        inline constexpr const char* LightCone   = "builtin::models::lightCone";
        inline constexpr const char* LightSphere = "builtin::models::lightSphere";

        inline constexpr const char* All[] = {
            Cube,       
            Cone,       
            Cylinder,   
            Sphere,
            Plane,      
            Torus,      
            BgPlane,    
            GridPlane,  
            LightArrow, 
            LightCone,  
            LightSphere,
        };
    }

    namespace Shader
    {
        inline constexpr const char* PBR        = "builtin::shaders::pbr";
        inline constexpr const char* Matcap     = "builtin::shaders::matcap";
        inline constexpr const char* Wireframe  = "builtin::shaders::wireframe";
        inline constexpr const char* Grid       = "builtin::shaders::grid";
        inline constexpr const char* Background = "builtin::shaders::bg";
        inline constexpr const char* Selection  = "builtin::shaders::selection";
        inline constexpr const char* Shadow     = "builtin::shaders::shadow";
        inline constexpr const char* Light      = "builtin::shaders::light";

        inline constexpr const char* All[] = {
            PBR,
            Matcap,
            Wireframe,
            Grid,
            Background,
            Selection,
            Shadow,
            Light,
        };
    }

    namespace FX
    {
        inline constexpr const char* Grayscale       = "builtin::fx::grayscale";
        inline constexpr const char* PassThrough     = "builtin::fx::passThrough";
        inline constexpr const char* Invert          = "builtin::fx::invert";
        inline constexpr const char* Sepia           = "builtin::fx::sepia";
        inline constexpr const char* Vignette        = "builtin::fx::vignette";
        inline constexpr const char* GammaCorrection = "builtin::fx::gammaCorrection";
        inline constexpr const char* Posterize       = "builtin::fx::posterize";
        inline constexpr const char* Pixelate        = "builtin::fx::pixelate";

        inline constexpr const char* All[] = {
            Grayscale,
            PassThrough,
            Invert,
            Sepia,
            Vignette,
            GammaCorrection,
            Posterize,
            Pixelate
        };
        
    }

    namespace LightType
    {
        inline constexpr const char* Point       = "builtin::lightType::point";
        inline constexpr const char* Directional = "builtin::lightType::directional";
        inline constexpr const char* Spot        = "builtin::lightType::spot";
    }

    namespace Icon{
        namespace EditorTool{
            inline constexpr const char* Select    = "builtin::editorTool::select";
            inline constexpr const char* BoxSelect = "builtin::editorTool::boxSelect";
            inline constexpr const char* Translate = "builtin::editorTool::translate";
            inline constexpr const char* Rotate    = "builtin::editorTool::rotate";
            inline constexpr const char* Scale     = "builtin::editorTool::scale";

        }
        namespace ViewMode
        {
            inline constexpr const char* Lit        = "builtin::viewMode::Lit";
            inline constexpr const char* Matcap     = "builtin::viewMode::Matcap";
            inline constexpr const char* Wireframe  = "builtin::viewMode::Wireframe";
        }

        inline constexpr const char* All[] = {
            EditorTool::Select,
            EditorTool::BoxSelect,
            EditorTool::Translate,
            //EditorTool::Rotate,
            EditorTool::Scale,
            ViewMode::Lit,
            ViewMode::Matcap,
            ViewMode::Wireframe,
        };
    }
}