#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
  public:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 rotation = glm::vec3(0.0f); // pitch, yaw, roll

	float moveSpeed = 5.0f; // meters per second
	float rotSpeed = 90.0f; // degrees per second

	glm::mat4 getViewMatrix() const;

	void update(float dt);
};
