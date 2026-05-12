#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;
class UIManager;

class Mouse
{
	static Mouse* _this;
	static UIManager* _UI;

	float _mouseLastX = 0;
	float _mouseLastY = 0;
	bool _mouseLeftPress = false;
	bool _mouseMiddlePress = false; 
	bool _mouseRightPress = false;
	bool _firstMouse = true;

	float _timeThresholdPress = 0.2; // second 
	float _mouseLeftTime = 0; 

	glm::vec2 _dragPosBegin{};
	glm::vec2 _dragPosEnd{};
	bool _isDragActive{false};


	float _rotSens = .4f; // rotation sensitivity
	float _moveSens = 0.01f; // move sensitivity

public:
	void init(GLFWwindow* window, UIManager* UI);
	void update(float deltaTime);

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void mouse_cursor_callback(GLFWwindow* window, double xposIn, double yposIn);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

};

