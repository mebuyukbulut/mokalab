#pragma once
#include "ICommand.h"
#include "Entity.h"

class EntityCommand : public ICommand {
protected:
    Entity* _entity;

public:
    EntityCommand(Entity* entity) : _entity(entity) {
        if (_entity) {
            _entity->commandRefCount++;
        }
    }

    ~EntityCommand() override {
        if (_entity) {
            _entity->commandRefCount--;
            
            // Nesneyi tutan son komut da stack'ten düştüyse ve nesne silindiyse:
            if (_entity->commandRefCount == 0 && _entity->tombstone) {
                _entity->reap = true; // Artık FreeList / GC güvenle alabilir
            }
        }
    }
};