#pragma once 

#include "ICommand.h"
#include "Entity.h"

class CreateEntityCommand : public ICommand{

    Entity* _entity{};

public:
    CreateEntityCommand(Entity* entity) : _entity{entity} {}
    ~CreateEntityCommand(){ if(_entity->tombstone) _entity->reap = true; }

    void execute() override{
        _entity->tombstone = false;
    }
    void undo() override{
        _entity->tombstone = true; 
    }

};