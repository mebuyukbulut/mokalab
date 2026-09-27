#include "AssetSystem/MaterialSaver.h"
#include <yaml-cpp/yaml.h>
#include "Logger.h"
#include <fstream>
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

void MaterialSaver::save(std::filesystem::path path, struct std::shared_ptr<Asset> asset){
    try {
        auto material = std::static_pointer_cast<Material>(asset);
        YAML::Node root;
        root = *material;
        
        std::ofstream fout(path);
        fout << root;
        fout.close();
    } catch (const std::exception& e) {
        LOG_ERROR("Material cannot be saved to the file: {}", path.string());		
    }
}

