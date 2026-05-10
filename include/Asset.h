#pragma once
#include "Object.h"
#include <filesystem>

enum class AssetType {
	Texture,
	Model,
	Mesh, 
	Material,
	Shader,  // ?
	Unknown, // ?
};
enum class AssetLoadStatus {
    NotLoaded, 
    LoadingToCPU, 
    ReadyToUpload,
    LoadingToGPU, // must done in main thread 
    Complete, 
    Error 
};

// Asset ile ilgili extra ayarları bu şekilde pass edeceğiz
// Belki metadata için de benzer bir şey yapılabilir 

struct IAssetSettings { virtual ~IAssetSettings() = default; };


struct TextureSettings : IAssetSettings {
    bool flipUVs = true;
    bool generateMipmaps = true;
    //int filterMode = GL_LINEAR; // OpenGL enum
    std::string realPath{};
};

struct ModelSettings : IAssetSettings {
    bool triangulate = true;
    bool calcTangentSpace = true;
    float scale = 1.0f;
};
struct ShaderSettings : IAssetSettings
{
    ShaderSettings() = default;
    ShaderSettings(std::string name, std::string vertexPath, std::string fragmentPath) 
        : name{name}, vertexPath{vertexPath}, fragmentPath{fragmentPath}{}

    std::string name{};
    std::string vertexPath{};
    std::string fragmentPath{};
};

class Asset : public Object{
protected:
    AssetType   _type{ AssetType::Unknown };
    AssetLoadStatus _loadStatus{ AssetLoadStatus::NotLoaded };
    std::filesystem::path _path; // Asseting bulunduğu fiziksel path
    uint32_t    _thumbnailID{ 0 }; // Content Browser için ikon/texture handle
    bool        _isDirty{ false }; // Değiştirildi mi? (Save gerekip gerekmediği için)

public:
    Asset() = default;
    virtual ~Asset() = default;

    // --- Zorunlu Fonksiyonlar ---
    virtual void load(std::filesystem::path path, IAssetSettings* settings) = 0;   // Diskten yükle
    virtual void unload() = 0; // Belleği boşalt
    virtual void uploadToGPU() = 0; // cpu -> gpu yükleme işlemi main threadden çağrılmalı

    // --- Getters & Setters ---
    AssetType getType() const { return _type; }
    AssetLoadStatus getLoadStatus() const { return _loadStatus; }
    std::string getPath() const { return _path.string(); }


    bool isLoaded() const { return _loadStatus == AssetLoadStatus::Complete; }
    bool isDirty() const { return _isDirty; }
    void setDirty(bool dirty) { _isDirty = dirty; }


    // OBject ten inherit edilen fonksiyonlar 
    virtual void onInspect() {};

    // --- Serialization (Object'ten override) ---
    //void serialize(YAML::Emitter& out) override {
    //    Object::serialize(out); // UUID ve Name buradan gelir
    //    out << YAML::Key << "Path" << YAML::Value << _path.string();
    //    out << YAML::Key << "Type" << YAML::Value << (int)_type;
    //}


};

// load fonksiyonu sekron veya asekron olarak çağrılabilir. 
// her iki durumda da load fonksiyonundan sonra eğer readyToUpload ise uploadToGPU fonksiyonu çalışmalı
// load fonksiyonu sadece asset manager tarafından kullanılmalı. Bunu önlemek için bir mekanizma yapılabilir mi bilmiyorum. 

// unload fonksiyonu için muhtemelen gpu dan opengl komutları ile silme işlemleri yapacağımz komutlar olacaktır. 
// daha buna karar verecek aşamaya gelemedim

// uploadToGPU fonksiyonu cpu tarafında hazır olan datanın gpu ya yüklenme işlemi. 
// Kimi assetler için bu tip bir fonksiyona gerek olmayabilir. Fakat şuanki assetlerimizde (mesh, texture, shader ...) bu gerekli. 
// Gerekli çünkü OpenGL main thread üzerinden çalışıyor ve biz yüklemeleri asekron yapmaya çalıştığımızda sadece cpu tarafını asekron yapabiliyoruz. 
// Bu sebepten dolayı asekron işlem bitince gpu tarafı için çağrılacak bir fonksiyona ihtiyacımız var. 