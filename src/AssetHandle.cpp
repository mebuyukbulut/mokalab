#include "AssetHandle.h"
#include "EngineContext.h"
#include "AssetManager.h"
//#include "Mesh.h"
#include "Model.h"
#include "Material.h"
#include "Shader.h"
#include "Texture.h"

template<class T>
std::shared_ptr<T> AssetHandle<T>::resolve(EngineContext* ece) const
{
    if (auto sp = _cache.lock()) return sp;
    auto sp = ece->assets->get<T>(_uuid);
    _cache = sp;
    return sp;
}


// Explicit instantiation — kullanacağın her asset tipi için bir satır
//template class AssetHandle<Mesh>;
template class AssetHandle<Model>;
template class AssetHandle<Material>;
template class AssetHandle<Shader>;
template class AssetHandle<Texture>;