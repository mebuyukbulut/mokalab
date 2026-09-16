#pragma once 

#include "ICommand.h"
#include "Entity.h"

class DeleteEntityCommand : public ICommand{

    Entity* _entity{};

public:
    DeleteEntityCommand(Entity* entity) : _entity{entity} {}
    ~DeleteEntityCommand(){ if(_entity->tombstone) _entity->reap = true; }

    void execute() override{
        _entity->tombstone = true;
    }
    void undo() override{
        _entity->tombstone = false; 
    }

};