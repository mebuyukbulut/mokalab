#include "AssetSystem/MaterialLoader.h"
#include "Builtin.h"

#include <yaml-cpp/yaml.h>
#include "Material.h"


namespace YAML {
// --- glm::vec4 ---
template<>
struct convert<glm::vec4> {
	// Deserialize (YAML Node -> glm::vec4)
	static bool decode(const Node& node, glm::vec4& rhs) {
		if (!node.IsSequence() || node.size() != 4) {
			return false;
		}

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		rhs.w = node[3].as<float>();
		return true;
	}

	// Serialize (glm::vec4 -> YAML Node)
	static Node encode(const glm::vec4& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		node.push_back(rhs.w);
		node.SetStyle(EmitterStyle::Flow); // [x, y, z, w] formatında yazar
		return node;
	}
};

template<>
struct convert<Material> {

    static Node encode(const Material& rhs) {
        Node node;
        node["name"		  ] = rhs.name		 ;
        node["baseColor"  ] = rhs.baseColor	 ;
        node["emissive"   ] = rhs.emissive	 ;
        node["metallic"   ] = rhs.metallic	 ;
        node["roughness"  ] = rhs.roughness  ;
        node["reflectance"] = rhs.reflectance;
        node["ao"         ] = rhs.ao		 ;
        return node;
    }

    static bool decode(const Node& node, Material& rhs) {
        if (!node.IsMap()) return false;
        
        if (node["name"		  ]) rhs.name		 = node["name"		 ].as<std::string>();
        if (node["baseColor"  ]) rhs.baseColor	 = node["baseColor"  ].as<glm::vec4>();
        if (node["emissive"   ]) rhs.emissive	 = node["emissive"   ].as<glm::vec4>();
        if (node["metallic"   ]) rhs.metallic	 = node["metallic"   ].as<float>();
        if (node["roughness"  ]) rhs.roughness 	 = node["roughness"  ].as<float>();
        if (node["reflectance"]) rhs.reflectance = node["reflectance"].as<float>();
        if (node["ao"         ]) rhs.ao			 = node["ao"         ].as<float>();
        return true;
    }
};


} // namespace YAML



std::shared_ptr<Asset> MaterialLoader::load(std::filesystem::path path, IAssetSettings *settings)
{
    std::shared_ptr<Material> mat = std::make_shared<Material>(); 

    mat->_path = path; // bunu Model::Load da yapabiliriz belki. 
    std::string pathStr = path.string(); 

    // Sanal yolla model yükleme
    for(const char* key : Builtin::Material::All){
        if(key == pathStr){
            loadDefault(pathStr, mat);
            return mat;
        }
    }


    // dosyadan okuyup material oluştur. 
    try {
        YAML::Node root = YAML::LoadFile(path.string());
        Material m = root.as<Material>();
        mat->name = m.name;
        mat->baseColor = m.baseColor;
        mat->emissive = m.emissive;
        mat->metallic = m.metallic;
        mat->roughness = m.roughness;
        mat->reflectance = m.reflectance;
        mat->ao = m.ao; 
    } catch (const std::exception& e) {
        LOG_ERROR("Material cannot load from file: {}", path.string());
    }

    return mat; 
}


void MaterialLoader::loadDefault(std::string path, std::shared_ptr<Material> mat)
{
	if(path == Builtin::Material::DefaultMaterial){
		mat->name = "Default Material";
	}
	else if(path == Builtin::Material::DefaultMetal){
		mat->name = "Default Metal"; 
		mat->metallic = 1.0f; 
		mat->roughness = 0.3;
	}
	else if(path == Builtin::Material::PlasticRed){
		mat->name = "Plastic Red";
		mat->baseColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); 
		mat->roughness = 0.25;
	}
	else if(path == Builtin::Material::PlasticBlue){
		mat->name = "Plastic Blue";
		mat->baseColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); 
		mat->roughness = 0.25;
	}
	else if(path == Builtin::Material::BoxCrate){
		mat->name = "Wood Crate";
		//baseColorTexture = ece->assets.get<Texture>("../assets/textures/box_crate.jpg");
		mat->roughness = 0.75;
	}
	else{
		LOG_ERROR("The internal material path is wrong or unimplemented", path);
	}
	mat->_loadStatus = AssetLoadStatus::Complete;
}


// {
//     return std::shared_ptr<Asset>();
// }
