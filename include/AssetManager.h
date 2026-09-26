#pragma once
#include <map>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <future>
#include <typeindex>
#include <algorithm>

#include "Object.h"
#include "Asset.h"

class IAssetLoader;
class IAssetSaver;
struct EngineContext;
template<class T> class AssetHandle;

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
	
    void setContext(EngineContext* context);

	// apply serialize and deserialize
};



