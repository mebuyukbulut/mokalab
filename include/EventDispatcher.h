#pragma once
#include <glm/glm.hpp>
#include <functional>
#include <string>



enum class EventType {
    onMove,
    onRotate,
    onZoom,
    ShaderSelected,
    EngineExit,
    ModelOpened,

    AddPointLight,
    AddDirectionalLight,
    AddSpotLight,

    AddCube,
    AddCone,
    AddCylinder,
    AddPlane,
    AddTorus,
    AddMonkey, 

    Delete,
    Select, 

    SaveScene,
    LoadScene,

    SetMainWindowTitle,

    ScenePopup,
    FocusToSelectedObject,
};

struct EventData {
    glm::vec3 vec{};
    std::string text{};
    bool check{};
};

struct Event {
	EventType type{};
	EventData data{};
};

class EventDispatcher {
public:
    using Callback = std::function<void(const Event&)>;

    void subscribe(EventType type, Callback cb) {
        _listeners[type].push_back(cb);
    }

    void dispatch(const Event& event) {
        for (auto& cb : _listeners[event.type]) {
            cb(event);
        }
    }

private:
    std::unordered_map<EventType, std::vector<Callback>> _listeners;
};


inline EventDispatcher dispatcher{}; // declared and defined in one place