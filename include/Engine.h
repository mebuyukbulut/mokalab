#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <memory>

#include "Renderer.h"
#include "Model.h"
#include "Camera.h"
#include "UIManager.h"
#include "LightManager.h"
#include "Time.h"
#include "Mouse.h"
#include "InputManager.h"
#include "ParticleSystem.h"

#include "Config.h"
#include "PathResolver.h"
#include "SceneManager.h"

#include "EventDispatcher.h"
#include "AssetManager.h"
#include "EngineContext.h"

class Engine
{	
    // ==========================
    // Engine Context
    // ==========================
	PathResolver    _paths{};
    Config          _config{};
    AssetManager    _assets{};
    EventDispatcher _dispatcher{};


    // ==========================
    // 
    // ==========================

	// burası çok dağınık düzenlenmeli sahipleri belirlenmeli. 
	GLFWwindow* _window{};
	Renderer _renderer{};
	SceneManager SM{};
	std::shared_ptr<Camera> _camera{new OrbitCamera()};
	UIManager _UI{};
	Mouse _mouse{};
	InputManager _IM{}; // Input Manager
	Time time{};
	ParticleSystem ps{};



	void initWindow();
	void initOpenGL();
	void initUI();
	void init();

	void mainLoop();
	void terminate();
	void processInput(GLFWwindow* window);

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

public:    

	EngineContext _ece;

	Engine():_ece {_paths, _config, _assets, _dispatcher } { _assets.setContext(&_ece); };
	void run();
};

