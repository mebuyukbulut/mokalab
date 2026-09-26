#pragma once
#include "Object.h"
#include <unordered_map>
#include <map>
#include <filesystem>
#include <future>
#include <vector>
#include <typeindex>
#include <algorithm>
#include "Asset.h"
#include "Logger.h"
#include "EngineContext.h"

#include "AssetSystem/IAssetLoader.h"
#include "AssetSystem/IAssetSaver.h"
#include "Shader.h"
#include "Texture.h"
#include "AssetHandle.h"

class Shader;
class Texture;


class AssetManager : public Object
{

	class AssetRegistery {
	private:
		std::unordered_map<std::filesystem::path, uint64_t> path2uuid{};
		std::unordered_map<uint64_t, std::filesystem::path> uuid2path{};
	public:
		void addRecord(uint64_t uuid, std::filesystem::path path);
		void removeRecord(uint64_t uuid);
		void removeRecord(std::filesystem::path path);

		void importRegistery(const YAML::Node& node);
		void exportRegistery(YAML::Emitter& out);

		uint64_t operator()(std::filesystem::path path); // 0 if not exits 
		std::filesystem::path operator()(uint64_t uuid);

		void clear(); // clear all records. Maybe unneccessary 
	}AR;

	std::unordered_map<uint64_t, std::shared_ptr<Asset>> _assets; // Tüm assetler
	std::vector<std::future<std::shared_ptr<Asset>>> _activeLoads; // Async ile CPU da işlem görenler 
	//std::vector<std::shared_ptr<Asset>> _pendingUploads; // Async işlemi sonrası GPU ya yüklenmeyi bekleyenler 

	std::map<std::type_index, std::vector<std::weak_ptr<Asset>>> _typeLists ; 

	EngineContext* ece{};
	//void load(std::filesystem::path path, const IAssetSettings* settings = nullptr);
	//void loadAsync(std::filesystem::path path, const IAssetSettings* settings = nullptr);
	    
	static inline std::unordered_map<std::type_index, std::function<std::shared_ptr<IAssetLoader>(EngineContext*)> > _loaders;
	static inline std::unordered_map<std::type_index, std::function<std::shared_ptr<IAssetSaver>()> > _savers;

public:

	template <class T>
    static void registerLoader(std::function<std::shared_ptr<IAssetLoader>(EngineContext*)> creator) {
        _loaders[std::type_index(typeid(T))] = creator;
    }
	template <class T>
    static void registerSaver(std::function<std::shared_ptr<IAssetSaver>()> creator) {
        _savers[std::type_index(typeid(T))] = creator;
    }



public:

	template <class T>
	std::shared_ptr<T> get(uint64_t id);

	template <class T>
	AssetHandle<T> get(std::filesystem::path path, IAssetSettings* settings = nullptr, bool async = false);

	template <class T>
	std::vector<std::shared_ptr<T>> getAll();


	void update();
	
    void setContext(EngineContext* context) { 
        ece = context; 
    }
	// apply serialize and deserialize
};





template<class T>
inline std::shared_ptr<T> AssetManager::get(uint64_t id)
{
	// Acaba buradaki complete statüsün sorgulamamız yanlış mı? 
	//if (_assets.find(id) != _assets.end() && _assets[id]->getLoadStatus == AssetLoadStatus::Complete) 
	if (_assets.find(id) != _assets.end()) 
		return std::dynamic_pointer_cast<T>(_assets[id]);

	return std::shared_ptr<T>();

}

template<class T>
inline AssetHandle<T> AssetManager::get(std::filesystem::path path, IAssetSettings* settings, bool async)
{
	// Asset varsa olanı döndür
	if (uint64_t assetID = AR(path)) 
		return AssetHandle<T>(assetID);
		//return get<T>(assetID);


	auto func = _loaders[std::type_index(typeid(T))];

	std::shared_ptr<Asset> asset;

	auto loader = func(ece); // Loader örneği oluşturulur
	if (!loader) {
		LOG_CRITICAL("loader function is empty!");
		return AssetHandle<T>();
	}		
	
	if (async) {
		if constexpr (std::is_constructible_v<T, EngineContext*>) 
			asset = std::make_shared<T>(ece);		
		else if constexpr (std::is_constructible_v<T>) 
			asset = std::make_shared<T>();
			
		auto uuidOfTemp = asset->UUID;

		_activeLoads.push_back(
			std::async(std::launch::async, 
				[uuidOfTemp, loader, path, settings]() -> std::shared_ptr<Asset>{
					auto myAsset = loader->load(path, settings);
					myAsset->UUID = uuidOfTemp;
					return myAsset;
				}
		
		));

	}
	else{
		asset = loader->load(path, settings);	

		if (asset->getLoadStatus() == AssetLoadStatus::ReadyToUpload){
			asset->uploadGPU();
			LOG_SUCCESS("Asset was loaded:\t {}" , asset->getPath().c_str());
		}
		else{
			LOG_ERROR("Asset loading failed:\t {}" , asset->getPath().c_str());
		}
	}

	_assets[asset->UUID] = asset; 
	_typeLists[std::type_index(typeid(T))].push_back(asset);
	AR.addRecord(asset->UUID, path);

	
	return AssetHandle<T>(asset->UUID);
}

template <class T>
inline std::vector<std::shared_ptr<T>> AssetManager::getAll()
{
	auto typeindex = std::type_index(typeid(T));
	// if there is no index return empty vector
	if(! _typeLists.contains(typeindex)) return {};

	const std::vector<std::weak_ptr<Asset>>& src = _typeLists.at(typeindex);

	std::vector<std::shared_ptr<T>> result; 
	result.reserve(src.size());

    std::transform(src.begin(), src.end(), std::back_inserter(result),
        [](const std::weak_ptr<Asset>& weakAsset)
        {
			auto sharedAsset = weakAsset.lock();
			if(sharedAsset)
            	return std::static_pointer_cast<T>(sharedAsset);
			else
				return std::shared_ptr<T>(); 
        });

    std::erase_if(result,
        [](const std::shared_ptr<T>& asset)
        { // !asset could be verbose! 
            return !asset || asset->getLoadStatus() != AssetLoadStatus::Complete;
        });

    return result;
}
