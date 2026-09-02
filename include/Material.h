#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>
#include "Asset.h"

class Texture;
class Shader;
struct EngineContext;

class Material : public Asset
{
	// EngineContext* ece{};
	// struct DefaultTextures{
	// 	std::shared_ptr<Texture> white;
	// 	std::shared_ptr<Texture> black;
	// 	std::shared_ptr<Texture> normal;
	// 	DefaultTextures();
	// } defTex;
	std::shared_ptr<Texture> defaultWhite{};
	std::shared_ptr<Texture> defaultNormal{};
public:
	glm::vec4 baseColor;
	glm::vec4 emissive;
	float metallic;
	float roughness;
	float reflectance;
	float ao;

	std::shared_ptr<Texture> baseColorTexture{};
	std::shared_ptr<Texture> armTexture{}; // AO, Roughness, Metallic
	std::shared_ptr<Texture> normalTexture{};
	std::shared_ptr<Texture> emissiveTexture{};

	Material() :
		baseColor{ 1.0f, 1.0f, 1.0f, 1.0f },
		emissive{ 0.0f, 0.0f, 0.0f, 0.0f },
		metallic{ 0.0f },
		roughness{ 0.5f },
		reflectance{ 0.45f },
		ao{ 1.f }
	{}

	void use(Shader* shader, EngineContext* ece);

	void loadDefault(std::string path);

	// Inherited via Asset
	void load(std::filesystem::path path, IAssetSettings* settings) override;
	void unload() override;
	void uploadToGPU() override;

	virtual void onInspect();

	void setBaseColorTexture(std::shared_ptr<Texture> texture);
	void setArmTexture(std::shared_ptr<Texture> texture);
	void setNormalTexture(std::shared_ptr<Texture> texture);
	void setEmissiveTexture(std::shared_ptr<Texture> texture);

};



namespace EditorUI{
	// void MaterialEditor(std::shared_ptr<Material> mat);
	std::string materialSelector(EngineContext* ece);
	//std::vector<std::string> getMaterialList()
}