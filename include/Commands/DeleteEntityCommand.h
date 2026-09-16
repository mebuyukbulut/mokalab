#pragma once 
#include "EntityCommand.h"

class DeleteEntityCommand : public EntityCommand{

public:
    DeleteEntityCommand(Entity* entity) : EntityCommand(entity) {}

    void execute() override{
        _entity->tombstone = true;
    }
    void undo() override{
        _entity->tombstone = false; 
    }

};