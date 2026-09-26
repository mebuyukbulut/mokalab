#include "AssetManager.h"

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
