#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform;

class Camera
{
protected:
	glm::vec2 _windowSize;
	bool isDirty{ true };
	bool isFrozen{ false }; // box select aktif ise frozen yoksa değil

	float _fov; // field of view
	float _aspectRatio; // width / height
	float _nearPlane; // near clipping plane
	float _farPlane; // far clipping plane

	glm::vec3 _position; // camera position
	glm::vec3 _target; // camera target

	glm::vec3 _front;
	glm::vec3 _right;
	glm::vec3 _up; 

	virtual void updateVectors() { isDirty = false; }

public:
	virtual void init(glm::vec2 windowSize);
	virtual void init(Transform* transform);

	
	glm::mat4 getProjectionMatrix();
	glm::mat4 getViewMatrix();

	void setWindowSize(int width, int height);
	glm::vec2 getWindowSize();
	glm::vec3 getPosition() { return _position; }

	virtual void move(glm::vec3 offset);
	virtual void rotate(glm::vec3 angle);
	virtual void zoom(float amount);

	virtual void resetFrame(Transform* transform = nullptr);
};

class OrbitCamera : public Camera{

	float _radius, _theta, _phi; // spherical coordinates for camera position

	virtual void updateVectors() {
		_position = _target;
		// Update camera position based on spherical coordinates
		_position.x += _radius * sin(_phi) * cos(_theta);
		_position.y += _radius * cos(_phi);
		_position.z += _radius * sin(_phi) * sin(_theta);

		_front = glm::normalize(_target - _position); // direction from camera to target
		_right = glm::normalize(glm::cross(_front, glm::vec3(0, 1, 0))); // right vector
		_up = glm::normalize(glm::cross(_right, _front)); // up vector
	}
public:
	void init(glm::vec2 windowSize) override {
		Camera::init(windowSize);
		_target = glm::vec3(0.0f, 0.0f, 0.0f); // _target is center point 

		// Spherical coordinates for camera position
		_radius = 5.0f; // distance from the target
		_theta = glm::radians(45.0f); // angle in the x-z plane
		_phi = glm::radians(45.0f); // angle from the y-axis
	}
	void move(glm::vec3 offset) override {
		_target += -_right * offset.x - _up * offset.y;
		isDirty = true; 
	}
	void rotate(glm::vec3 angle) override {		
		float dTheta = angle.x;
		float dPhi = angle.y;
		_theta += dTheta;
		_phi = glm::clamp(_phi + dPhi, 0.1f, glm::pi<float>() - 0.1f); // avoid flipping
	}
	void zoom(float amount) override {
		_radius = glm::max(0.1f, _radius + amount);
	}
	//virtual void resetFrame(Transform* transform = nullptr) override{
	//	init(_windowSize);
	//}
};