#pragma once
#include <string>
#include <functional>
#include <glm/glm.hpp>
#include "Camera.h"
#include <memory>

#include "EngineContext.h"

struct GLFWwindow;
class LightManager;
class Camera;
class Material;

class UIManager
{
	GLFWwindow* _window;
	std::shared_ptr<Camera> _camera;
	EngineContext* ece;
	bool _IsHoveringSceneViewport = false;
	//glm::vec2 _windowSize;
public:
	void init(GLFWwindow* window,  std::shared_ptr <Camera> camera, EngineContext& ece);
	//void setWindowSize(int width, int height) { _windowSize = glm::vec2(width, height); }

	void terminate(); 

	void draw(class SceneManager* sm);

	bool isHoverOnUI();
	void setHoverOnUI(bool flag) { _IsHoveringSceneViewport = flag; }

	//void viewport_window();

	class ParticleSystem* ps;

private: 
	void beginFrame();
	void endFrame(); 

	bool isShaderPanelOpen = false;
	bool isCreditsPanelOpen = false;
	bool isLightPanelOpen = false; 

	void mainMenu();
	void shaderPanel();
	void creditsPanel();


};

