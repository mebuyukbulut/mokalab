#pragma once
//#include <cstdint>
#include <memory>

struct EngineContext;

template<class T>
class AssetHandle {
    uint64_t _uuid{};
    mutable std::weak_ptr<T> _cache{};
public:
    AssetHandle() = default;
    AssetHandle(uint64_t uuid) : _uuid{uuid} {}

    uint64_t id() const { return _uuid; }
    bool isValid() const { return _uuid != 0; }

    std::shared_ptr<T> resolve(EngineContext* ece) const; // sadece deklarasyon
};