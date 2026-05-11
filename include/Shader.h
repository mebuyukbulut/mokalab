#pragma once
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Asset.h"

class Shader : public Asset
{

	unsigned int _shaderProgram;
	std::unordered_map<std::string, int> _uniforms;

	unsigned int compileShader(std::string shaderCode, unsigned int shaderType);
	unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
	int getUniformLocation(const std::string& name);

public:

	void init(std::string vertexShaderSource, std::string fragmentShaderSource);
	void use(); 
	void terminate(); 

	void set(const std::string& name, const glm::mat4& mat);
	void set(const std::string& name, const glm::vec2& vec);
	void set(const std::string& name, const glm::vec3& vec);
	void set(const std::string& name, const glm::vec4& vec);
	void set(const std::string& name, float value);
	void set(const std::string& name, int value);
	void set(const std::string& name, uint32_t value);


public:
	// Inherited via Asset
    void load(std::filesystem::path path, IAssetSettings* settings) override;
    void unload() override;
    void uploadToGPU() override;
};

