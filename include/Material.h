#pragma once
#include <glm/glm.hpp>
#include "Asset.h"
#include "AssetHandle.h"

class Texture;
class Shader;
struct EngineContext;

class Material : public Asset
{
	friend class MaterialLoader;
	AssetHandle<Texture> defaultNormal{};
	AssetHandle<Texture> defaultWhite{};
public:
	glm::vec4 baseColor;
	glm::vec4 emissive;
	float metallic;
	float roughness;
	float reflectance;
	float ao;

	inline static EngineContext* ece = nullptr;

	AssetHandle<Texture> baseColorTexture{};
	AssetHandle<Texture> armTexture{}; // AO, Roughness, Metallic
	AssetHandle<Texture> normalTexture{};
	AssetHandle<Texture> emissiveTexture{};

	Material();

	void use(Shader* shader);

	// Inherited via Asset
	void purgeCPU() override;
	void purgeGPU() override {};
	void uploadGPU() override; 


	virtual void onInspect() override;

	void setBaseColorTexture(AssetHandle<Texture> texture);
	void setArmTexture(AssetHandle<Texture> texture);
	void setNormalTexture(AssetHandle<Texture> texture);
	void setEmissiveTexture(AssetHandle<Texture> texture);

};



namespace EditorUI{
	// void MaterialEditor(std::shared_ptr<Material> mat);
	std::string materialSelector(EngineContext* ece);
	//std::vector<std::string> getMaterialList()
}