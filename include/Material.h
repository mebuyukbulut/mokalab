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
	std::shared_ptr<Texture> defaultWhite{};
	std::shared_ptr<Texture> defaultNormal{};
public:
	glm::vec4 baseColor;
	glm::vec4 emissive;
	float metallic;
	float roughness;
	float reflectance;
	float ao;

	inline static EngineContext* ece = nullptr;

	std::shared_ptr<Texture> baseColorTexture{};
	std::shared_ptr<Texture> armTexture{}; // AO, Roughness, Metallic
	std::shared_ptr<Texture> normalTexture{};
	std::shared_ptr<Texture> emissiveTexture{};

	Material();

	void use(Shader* shader);

	void loadDefault(std::string path);

	// Inherited via Asset
	void load(std::filesystem::path path, IAssetSettings* settings) override;
	void unload() override;
	void uploadToGPU() override;

	void save(std::filesystem::path path);

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