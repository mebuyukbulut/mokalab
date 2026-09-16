#pragma once

#include "EntityCommand.h"
#include "Entity.h"
#include "Transform.h"
#include <glm/glm.hpp>


struct TransformState{
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };
    glm::vec3 scale{ 1.0f };
};

class TransformCommand : public EntityCommand{

    TransformState _oldState;
    TransformState _newState;

public:
    TransformCommand(Entity* entity, const TransformState& oldState, const TransformState& newState)
        : EntityCommand(entity), _oldState(oldState), _newState(newState) {}

    void execute() override {
        _entity->transform->setPosition(_newState.position);
        _entity->transform->setRotation(_newState.rotation);
        _entity->transform->setScale   (_newState.scale);
    }

    void undo() override {
        _entity->transform->setPosition(_oldState.position);
        _entity->transform->setRotation(_oldState.rotation);
        _entity->transform->setScale   (_oldState.scale);
    }

};