#pragma once
#include <string>
#include <vector>
#include "Asset.h"

class Shader;

class Texture : public Asset {
	unsigned char* _data{};
	int _nrChannels, _width, _height; 

	unsigned int _id; 
	unsigned int _type; // OpenGL Enum

	void loadInternal(std::string path);

public:

	void use();
	void bind(uint32_t bindingPoint);

	unsigned int getId() const { return _id; }

	// Inherited via Asset
	void load(std::filesystem::path path, IAssetSettings* settings) override;
	void unload() override;
	void uploadToGPU() override;

	void createColorTexture(uint32_t width, uint32_t height);
	void createColorTextureHDR(uint32_t width, uint32_t height);
	void createDepthTexture(uint32_t width, uint32_t height);
	void createIdTexture(uint32_t width, uint32_t height);
	void createShadowDepthTexture(uint32_t width, uint32_t height);
	void createSolidColorTextureRGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a);


	void destroy(); 
};



//class TextureFactory {
//public:
//	static Texture* loadCubeMap(std::vector<std::string> faces);
//	static Texture* loadHDR(std::string path, Shader* shader);
//};
//

