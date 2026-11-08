#pragma once
#include <GLFW/glfw3.h>

// This class must have only 1 unique instance because of we reset glfw time inside init function
// So we can use singleton pattern in future but for now it is unnecessary
class EngineTime
{
	double _deltaTime{};
	double _lastTime{};
	double _now{};

public:
	
	// initialize time 
	void init() {
		glfwSetTime(0);
		_lastTime = glfwGetTime();
	}

	// calculate delta time for current frame 
	inline void update(){
		_now = glfwGetTime();
		_deltaTime = _now - _lastTime;
		_lastTime = _now;
	}

	// returns delta time for current frame
	inline double deltaTime() const { return _deltaTime; }	

	// returns elapsed time after initialization
	inline double elapsedTime() const { return _now; }

};

