#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <memory>


enum class EventType {
    onMove,
    onRotate,
    onZoom,
    ShaderSelected,
    EngineExit,
    ModelOpened,

    AddLight,
    AddPrimitive,
    AddMonkey, 

    Delete,
    Select, 
    MouseDrag,

    SaveScene,
    LoadScene,

    SetMainWindowTitle,

    ScenePopup,
    FocusToSelectedObject,
};

struct EventData {
    // glm::vec3 vec{};
    // std::string text{};
    // bool check{};
};

struct EventData_Text : EventData{
    std::string text{};
    EventData_Text(){}
    EventData_Text(std::string text) : text{text} {}
};
struct EventData_Point : EventData{
    glm::vec3 vec{};
    EventData_Point(){vec = glm::vec3(0,0,0);}
    EventData_Point(glm::vec3 vec) : vec{vec} {}
    EventData_Point(float x, float y, float z){vec = glm::vec3(x,y,z);}
};

struct EventData_DoublePoint : EventData{
    glm::vec3 vecA{};
    glm::vec3 vecB{};

    EventData_DoublePoint() { 
        vecA = glm::vec3(0,0,0); 
        vecB = glm::vec3(0,0,0); 
    }
    EventData_DoublePoint(glm::vec3 vecA, glm::vec3 vecB) : vecA{vecA}, vecB{vecB}{}
};

struct EventData_Check : EventData{
    bool check{};
};


struct Event {
	EventType type{};
	std::unique_ptr<EventData> data{};
};

class EventDispatcher {
public:
    using Callback = std::function<void(std::unique_ptr<EventData> data)>;

    void subscribe(EventType type, Callback cb) {
        _listeners[type].push_back(cb);
    }

    void dispatch(Event& event) {
        for (auto& cb : _listeners[event.type]) {
            cb(std::move(event.data));
        }
    }

private:
    std::unordered_map<EventType, std::vector<Callback>> _listeners;
};