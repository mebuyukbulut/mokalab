#include "AssetSystem/ShaderLoader.h"
#include "Builtin.h"
#include "FileUtils.h"
#include "Logger.h"

std::shared_ptr<Asset> ShaderLoader::load(std::filesystem::path path, IAssetSettings *settings)
{
    std::string vertexShaderName{};
    std::string fragmentShaderName{};

    std::string pathStr = path.string(); 

    bool isVirtual = false;
    bool isFX = false;
    // Sanal yolla model yükleme
    for(const char* key : Builtin::Shader::All){
        if(key == pathStr){        
            isVirtual = true;
            break;
        }
    }
    for(const char* key : Builtin::FX::All){
        if(key == pathStr){        
            isFX = true;
            break;
        }
    }

    if (isVirtual){
        ShaderSettings* a  = dynamic_cast<ShaderSettings*>(settings);
        vertexShaderName = a->vertexPath.string();
        fragmentShaderName = a->fragmentPath.string();
    }
    else if(isFX){
        ShaderSettings* a  = dynamic_cast<ShaderSettings*>(settings);
        vertexShaderName = a->vertexPath.string();
        fragmentShaderName = a->fragmentPath.string();
    }
    else{
        vertexShaderName = path.string();
        fragmentShaderName = path.string();
        vertexShaderName += ".vert";
        fragmentShaderName += ".frag";
    }
    LOG_TRACE("[ShaderLoader]\nvertex path: {}\nfragment path: {}", vertexShaderName, fragmentShaderName);
    std::string vertexShaderSource = FileUtils::readFile(vertexShaderName);
    std::string fragmentShaderSource = FileUtils::readFile(fragmentShaderName);

    
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(); 
    shader->init(vertexShaderSource, fragmentShaderSource);
    // newShader._type = shaderInfo.type;
    // shaders[shaderInfo.name] = newShader;
    shader->_loadStatus = AssetLoadStatus::Complete;



    return shader; 
}
