#pragma once
#include "AssetHandle.h"
#include "Material.h"

class EngineContext;


class MaterialBrowser {
    EngineContext* ece;
    AssetHandle<Material> mat;

public:
    void draw();
    void init(EngineContext* ece);

};

// selectedMatPath gibi bir değişken lazım
// material sınıfı üzerinden oninspect fonksiyonunun aynısını yapabilriz
// ek olarak texture bölmesi için bir karesel slot a sürükle bırak davranışına ihtiyaç var