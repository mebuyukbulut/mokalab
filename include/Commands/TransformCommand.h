#pragma once

#include "ICommand.h"
#include "Entity.h"
#include "Transform.h"
#include <glm/glm.hpp>


struct TransformState{
    glm::vec3 position{ 0.0f };
    glm::vec3 rotation{ 0.0f };
    glm::vec3 scale{ 1.0f };
};

class TransformCommand : public ICommand{

    Entity* _entity{};
    TransformState _oldState;
    TransformState _newState;

public:
    TransformCommand(Entity* entity, const TransformState& oldState, const TransformState& newState)
        : _entity(entity), _oldState(oldState), _newState(newState) {}

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