#pragma once 
#include <memory>

class IAssetSaver{
public:
    virtual void save() = 0;
};