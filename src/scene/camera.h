#pragma once
#include "../utility/includes.h"

class Camera {
public:
	Camera(float speed, float sensitivity);
	Camera();

	glm::vec3 getPosition() const;
	glm::mat4 getRotationMatrix() const;
	glm::vec3 getDirection() const;
	glm::vec3 getScreenDirection(GLFWwindow* window) const;

	void movePosition(glm::vec3 local_dir);
	void moveRotation(float dh, float dv);

public:
	glm::vec3 position;
	glm::vec2 rotation;

	float speed;
	float sensitivity;
	float fov;
	float focus_distance;
	float focus_blur;
	bool free_movement;
};