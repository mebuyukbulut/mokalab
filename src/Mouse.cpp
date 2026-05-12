#include "Mouse.h"
#include "UIManager.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <ImGuizmo.h>
#include "EventDispatcher.h"


Mouse* Mouse::_this = nullptr; 
UIManager* Mouse::_UI = nullptr;
    
void Mouse::init(GLFWwindow* window, UIManager* UI)
{
    _this = this;
    _UI = UI; 

    glfwSetMouseButtonCallback(window, Mouse::mouse_button_callback);
    glfwSetCursorPosCallback(window, Mouse::mouse_cursor_callback);
    glfwSetScrollCallback(window, Mouse::scroll_callback);
}


// Source - https://stackoverflow.com/a/37195173
// Posted by Drop, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-09, License - CC BY-SA 3.0
// state                  released               pressed                released
// timeline             -------------|------------------------------|---------------
//                                   ^                              ^
// mouse_callback calls          GLFW_PRESS                    GLFW_RELEASE

void Mouse::update(float deltaTime)
{
    static bool isFirstLeftPress = true;

    if (_mouseLeftPress) {
        _mouseLeftTime += deltaTime;

        ImVec2 mp = ImGui::GetMousePos();
        _dragPosEnd = glm::vec2(mp.x, mp.y);

        if(isFirstLeftPress){
            isFirstLeftPress = false; 
            _dragPosBegin = _dragPosEnd;    
        }
        
        if(glm::distance(_dragPosBegin, _dragPosEnd) > 10 && !_isDragActive)
            _isDragActive = true;            
        

        if(_isDragActive){
            glm::vec3 vecA = glm::vec3(_dragPosBegin, 0);
            glm::vec3 vecB = glm::vec3(_dragPosEnd, 0);
            Event e{ 
                EventType::MouseDrag,
                std::make_unique<EventData_DoublePoint>(vecA, vecB) };                
            dispatcher.dispatch(e);
        }
        
    }
    else { // mouse left released
        if (!_isDragActive && _mouseLeftTime && _mouseLeftTime <= _timeThresholdPress){
            ImVec2 mousePos = ImGui::GetMousePos();
            Event e{ 
                EventType::Select,
                std::make_unique<EventData_Point>(mousePos.x, mousePos.y, 0) };                
            dispatcher.dispatch(e);
            
        }

        _mouseLeftTime = 0.0f; 
        isFirstLeftPress = true;
        _isDragActive = false;   
    }

}

void Mouse::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // is Cursor On Viewport = !_UI->isHoverOnUI()
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !_UI->isHoverOnUI()) {
        _this->_mouseLeftPress = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        _this->_mouseLeftPress = false;
        _this->_firstMouse = true;
    }


    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS && !_UI->isHoverOnUI()) {
        _this->_mouseMiddlePress = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
        _this->_mouseMiddlePress = false;
        _this->_firstMouse = true;
    }


    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && !_UI->isHoverOnUI())
        _this->_mouseRightPress = true;
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        _this->_mouseRightPress = false;
        _this->_firstMouse = true;
    }

}
void Mouse::mouse_cursor_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!(_this->_mouseLeftPress || _this->_mouseMiddlePress ||_this->_mouseRightPress)) return;

    // Eğer Imguizmo kullanılıyorsa mouse hareketini yoksay 
    if (ImGuizmo::IsUsing()) {
        _this->_mouseLeftPress = false;
        _this->_mouseMiddlePress = false;
        _this->_mouseRightPress = false;
        _this->_firstMouse = true;
        return;
    }

    // İlk tıklanma noktasını al
    if (_this->_firstMouse) {
        _this->_mouseLastX = xposIn;
        _this->_mouseLastY = yposIn;
        _this->_firstMouse = false;
        return;
    }

    // ilk ve son noktalar arasındaki delta pozisyonu bul. 
    float xoffset = xposIn - _this->_mouseLastX;
    float yoffset = _this->_mouseLastY - yposIn; // reversed since y-coordinates go from bottom to top

    _this->_mouseLastX = xposIn;
    _this->_mouseLastY = yposIn;


    // gerekli komutları çalıştır 
    // box select 
    // rotate
    // move 
    // belki de bunların burada olması doğru değildir. 
    if (_this->_mouseLeftPress) {
        // update fonksiyonunda bu kısmı handle ediyoruz.
    }
    else if(_this->_mouseMiddlePress){
        xoffset = glm::radians(_this->_rotSens) * xoffset;
        yoffset = glm::radians(_this->_rotSens) * yoffset;
        glm::vec3 vec(xoffset, yoffset, 0);

        Event e{ 
            EventType::onRotate, 
            std::make_unique<EventData_Point>(glm::vec3(xoffset,yoffset,0))};
        dispatcher.dispatch(e);
    }
    else if(_this->_mouseRightPress){ 
        glm::vec3 vec(xoffset * _this->_moveSens, yoffset * _this->_moveSens, 0);
        Event e{ 
            EventType::onMove, 
            std::make_unique<EventData_Point>(vec) };
        dispatcher.dispatch(e);
    }
}
void Mouse::scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    if (!_UI->isHoverOnUI()) {
        glm::vec3 vec(0, yoffset, 0);
        Event e{ 
            EventType::onZoom, 
            std::make_unique<EventData_Point>(vec) };
        dispatcher.dispatch(e);
    }
}
