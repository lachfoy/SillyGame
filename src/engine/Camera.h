#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera
{
	glm::mat4 getViewMatrix() const;

	void lookAt(glm::vec3 target);

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 rotation = glm::vec3(0.0f); // pitch, yaw, roll
};
