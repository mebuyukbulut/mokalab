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
#include "Config.h"
#include "ParticleSystem.h"

#include "SceneManager.h"

class Engine
{
	GLFWwindow* _window{};
	Renderer _renderer{};
	SceneManager SM{};
	std::shared_ptr<Camera> _camera{new OrbitCamera()};
	UIManager _UI{};
	Mouse _mouse{};
	InputManager _IM{}; // Input Manager
	Time time{};
	//Config config;
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
	Engine() = default;
	void run();
};

