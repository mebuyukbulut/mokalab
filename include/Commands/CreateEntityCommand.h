#pragma once 
#include "EntityCommand.h"

class CreateEntityCommand : public EntityCommand{
    
public:
    CreateEntityCommand(Entity* entity) : EntityCommand(entity) {}

    void execute() override{
        _entity->tombstone = false;
    }
    void undo() override{
        _entity->tombstone = true; 
    }

};