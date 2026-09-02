#pragma once
#include "EngineContext.h"
struct GLFWwindow; 

class InputManager{
    EngineContext* ece; 
public:
    void init(EngineContext& ece){this->ece = &ece;}
    void processInput(GLFWwindow* window);


};