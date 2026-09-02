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
	std::vector<std::future<void>> _activeLoads; // Async ile CPU da işlem görenler 
	std::vector<std::shared_ptr<Asset>> _pendingUploads; // Async işlemi sonrası GPU ya yüklenmeyi bekleyenler 

	std::map<std::type_index, std::vector<std::shared_ptr<Asset>>> _typeLists ; 

	EngineContext* ece{};
	//void load(std::filesystem::path path, const IAssetSettings* settings = nullptr);
	//void loadAsync(std::filesystem::path path, const IAssetSettings* settings = nullptr);
public:

	template <class T>
	std::shared_ptr<T> get(uint64_t id);

	template <class T>
	std::shared_ptr<T> get(std::filesystem::path path, IAssetSettings* settings = nullptr, bool async = false);

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
inline std::shared_ptr<T> AssetManager::get(std::filesystem::path path, IAssetSettings* settings, bool async)
{
	// Asset varsa olanı döndür
	if (uint64_t assetID = AR(path)) 
		return get<T>(assetID);

	// Yoksa yükle
	std::shared_ptr<Asset> asset;

	// C++17 Metaprogramming:
	// Eğer T sınıfı "T(EngineContext*)" constructor'ına sahipse ona m_context verilir.
	if constexpr (std::is_constructible_v<T, EngineContext*>) {
		asset = std::make_shared<T>(ece);
	} 
	// Eğer varsayılan "T()" constructor'ına sahipse parametresiz çağrılır.
	else if constexpr (std::is_constructible_v<T>) {
		asset = std::make_shared<T>();
	}
	//std::shared_ptr<Asset> asset = std::make_shared<T>();
	_assets[asset->UUID] = asset; 
	_typeLists[std::type_index(typeid(T))].push_back(asset);

	AR.addRecord(asset->UUID, path);

	if (async) {
		_activeLoads.push_back(std::async(std::launch::async, &Asset::load, asset.get(), path, settings));
		_pendingUploads.push_back(asset);
	}
	else {
		asset->load(path, settings);
		if (asset->getLoadStatus() == AssetLoadStatus::ReadyToUpload){

			asset->uploadToGPU();
			//LOG_SUCCESS("[OK]\nAsset path: {}", asset->getPath().c_str());
		}
		else{
			//LOG_CRITICAL("SOMETHING GOES WRONG\nAsset path: {}", asset->getPath().c_str());
		}
	}

	return get<T>(asset->UUID); // yeni assetin id sini sorgula

}

template <class T>
inline std::vector<std::shared_ptr<T>> AssetManager::getAll()
{
	auto typeindex = std::type_index(typeid(T));
	// if there is no index return empty vector
	if(! _typeLists.contains(typeindex)) return {};

	const std::vector<std::shared_ptr<Asset>>& src = _typeLists.at(typeindex);

	std::vector<std::shared_ptr<T>> result; 
	result.reserve(src.size());

    std::transform(src.begin(), src.end(), std::back_inserter(result),
        [](const std::shared_ptr<Asset>& asset)
        {
            return std::static_pointer_cast<T>(asset);
        });

    std::erase_if(result,
        [](const std::shared_ptr<T>& asset)
        { // !asset could be verbose! 
            return !asset || asset->getLoadStatus() != AssetLoadStatus::Complete;
        });

    return result;
}
