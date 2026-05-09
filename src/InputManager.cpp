#include "InputManager.h"
#include <GLFW/glfw3.h>
#include "EventDispatcher.h"

void InputManager::processInput(GLFWwindow *window)
{
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
        Event e{ EventType::FocusToSelectedObject, {} };
        dispatcher.dispatch(e);
    }

    if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        Event e{ EventType::Delete, {} };
        dispatcher.dispatch(e);
    }


    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        Event e{ EventType::ScenePopup, {} };
        dispatcher.dispatch(e);
    }
}