#include "AssetManager.h"
#include "EngineContext.h"

#include "Logger.h"

#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "AssetSystem/IAssetLoader.h"
#include "AssetSystem/IAssetSaver.h"
#include "AssetHandle.h"

void AssetManager::AssetRegistery::addRecord(uint64_t uuid, std::filesystem::path path)
{
	uuid2path[uuid] = path;
	path2uuid[path] = uuid; 
}

// remove methodlarının şuanlık aciliyeti yok
void AssetManager::AssetRegistery::removeRecord(uint64_t uuid)
{
}

void AssetManager::AssetRegistery::removeRecord(std::filesystem::path path)
{
}

// import ve export fonksiyonları bir sonraki aşamada yazılmalı
void AssetManager::AssetRegistery::importRegistery(const YAML::Node& node)
{
}

void AssetManager::AssetRegistery::exportRegistery(YAML::Emitter& out)
{
}

uint64_t AssetManager::AssetRegistery::operator()(std::filesystem::path path)
{
	return path2uuid.find(path) != path2uuid.end() ? path2uuid[path] : 0 ;
}

std::filesystem::path AssetManager::AssetRegistery::operator()(uint64_t uuid)
{
	return uuid2path.find(uuid) != uuid2path.end() ? uuid2path[uuid] : std::filesystem::path();
}

void AssetManager::AssetRegistery::clear()
{
	path2uuid.clear();
	uuid2path.clear();
}

void AssetManager::update()
{
	// thread bitmişse vector den sil
	std::erase_if(_activeLoads, [this](auto& f) {
		bool ready = f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;

		if(ready){
			auto newAsset = f.get();
			
			if (newAsset->getLoadStatus() == AssetLoadStatus::ReadyToUpload){
				newAsset->uploadGPU();
				LOG_SUCCESS( "ASYNC asset loading was complete:\t {}", newAsset->getPath().c_str());
			}
			else{
				LOG_ERROR( "ASYNC asset loading failed:\t {}", newAsset->getPath().c_str());
			}


			_assets[newAsset->UUID] = newAsset; 
		}

		return ready;
	});

}

void AssetManager::setContext(EngineContext *context) { 
	ece = context; 
}

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
			//LOG_SUCCESS("Asset was loaded: {}" , asset->getPath().c_str());
		}
		else{
			//LOG_ERROR("Asset loading failed: {}" , asset->getPath().c_str());
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

template <class T>
std::shared_ptr<T> AssetManager::get(){		
	if constexpr (std::is_constructible_v<T, EngineContext*>) 
		return std::make_shared<T>(ece);		
	else if constexpr (std::is_constructible_v<T>) 
		return std::make_shared<T>();
}

template <class T>
void AssetManager::save(std::filesystem::path path, std::shared_ptr<T> asset){

	auto func = _savers[std::type_index(typeid(T))];
	auto saver = func(); // Loader örneği oluşturulur
	if (!saver) {
		LOG_ERROR("Saver is not defined for this type!");
		return;
	}		

	saver->save(path, asset);
}

template std::shared_ptr<Material>    AssetManager::get<Material>();
template std::shared_ptr<Material>    AssetManager::get<Material>(uint64_t);
template AssetHandle<Material>        AssetManager::get<Material>(std::filesystem::path, IAssetSettings*, bool);
template std::vector<std::shared_ptr<Material>> AssetManager::getAll<Material>();

template std::shared_ptr<Model>    AssetManager::get<Model>();
template std::shared_ptr<Model>    AssetManager::get<Model>(uint64_t);
template AssetHandle<Model>        AssetManager::get<Model>(std::filesystem::path, IAssetSettings*, bool);
template std::vector<std::shared_ptr<Model>> AssetManager::getAll<Model>();

template std::shared_ptr<Shader>    AssetManager::get<Shader>();
template std::shared_ptr<Shader>    AssetManager::get<Shader>(uint64_t);
template AssetHandle<Shader>        AssetManager::get<Shader>(std::filesystem::path, IAssetSettings*, bool);
template std::vector<std::shared_ptr<Shader>> AssetManager::getAll<Shader>();

template std::shared_ptr<Texture>    AssetManager::get<Texture>();
template std::shared_ptr<Texture>    AssetManager::get<Texture>(uint64_t);
template AssetHandle<Texture>        AssetManager::get<Texture>(std::filesystem::path, IAssetSettings*, bool);
template std::vector<std::shared_ptr<Texture>> AssetManager::getAll<Texture>();

// template std::shared_ptr<Mesh>    AssetManager::get<Mesh>();
// template std::shared_ptr<Mesh>    AssetManager::get<Mesh>(uint64_t);
// template AssetHandle<Mesh>        AssetManager::get<Mesh>(std::filesystem::path, IAssetSettings*, bool);
// template std::vector<std::shared_ptr<Mesh>> AssetManager::getAll<Mesh>();



template void AssetManager::save<Material>(std::filesystem::path path, std::shared_ptr<Material> asset);