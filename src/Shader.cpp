#include "Shader.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include "FileUtils.h"
#include "Builtin.h"



// compile shader code
unsigned int Shader::compileShader(std::string shaderCode, unsigned int shaderType)
{
    //std::cout << "Compiling shader of type: " 
	//	<< ((shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT") << std::endl;
	//std::cout << "Shader code:\n" << shaderCode << std::endl << std::endl;


	const char* shaderSource = shaderCode.c_str();
    unsigned int newShader = glCreateShader(shaderType);
    glShaderSource(newShader, 1, &shaderSource, NULL);
    glCompileShader(newShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(newShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(newShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::"
            << ((shaderType == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT") <<
            "::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

	return newShader;
}

// link vertex and fragment shader into a shader program
unsigned int Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader)
{
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check for linking errors
    int success;
    char infoLog[2048];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 2048, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

int Shader::getUniformLocation(const std::string& name){
    if (_uniforms.contains(name))
        return _uniforms[name];
    
	int location = glGetUniformLocation(_shaderProgram, name.c_str());
    _uniforms[name] = location;
    return location;
}

void Shader::init(std::string vertexShaderSource, std::string fragmentShaderSource)
{
    unsigned int vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    _shaderProgram = linkProgram(vertexShader, fragmentShader);
}

void Shader::use(){
    glUseProgram(_shaderProgram);
}

void Shader::terminate(){
    glDeleteProgram(_shaderProgram);
}

void Shader::set(const std::string& name, const glm::mat4& mat) {
    int location = getUniformLocation(name);
    if(location < 0) return;
    glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}

void Shader::set(const std::string &name, const glm::vec2 &vec) {
    int location = getUniformLocation(name);
    if(location < 0) return;
    glUniform2fv(location, 1, &vec[0]);
}

void Shader::set(const std::string& name, const glm::vec3& vec) {
    int location = getUniformLocation(name);
    if(location < 0) return;
    glUniform3fv(location, 1, &vec[0]);
}

void Shader::set(const std::string& name, const glm::vec4& vec) {
    int location = getUniformLocation(name);
    if(location < 0) return;
    glUniform4fv(location, 1, &vec[0]);
}

void Shader::set(const std::string& name, float value){
    int location = getUniformLocation(name);
    if(location < 0) return;
    glUniform1f(location, value);
}

void Shader::set(const std::string& name, int value){
    int location = getUniformLocation(name);
    if(location < 0) return;
    glUniform1i(location, value);
}

void Shader::set(const std::string& name, uint32_t value){
    int location = getUniformLocation(name);
    if(location < 0) return;
    glUniform1ui(location, value);
}

void Shader::load(std::filesystem::path path, IAssetSettings* settings)
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
        vertexShaderName = a->vertexPath;
        fragmentShaderName = a->fragmentPath;
    }
    else if(isFX){
        ShaderSettings* a  = dynamic_cast<ShaderSettings*>(settings);
        vertexShaderName = a->vertexPath;
        fragmentShaderName = a->fragmentPath;
    }
    else{
        vertexShaderName = path.c_str();
        fragmentShaderName = path.c_str();
        vertexShaderName += ".vert";
        fragmentShaderName += ".frag";
    }

    std::string vertexShaderSource = FileUtils::readFile(vertexShaderName);
    std::string fragmentShaderSource = FileUtils::readFile(fragmentShaderName);

    init(vertexShaderSource, fragmentShaderSource);
    // newShader._type = shaderInfo.type;
    // shaders[shaderInfo.name] = newShader;
    _loadStatus = AssetLoadStatus::Complete;

}

void Shader::unload()
{
    terminate();
}

void Shader::uploadToGPU()
{
}
