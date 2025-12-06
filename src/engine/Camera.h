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

	glm::mat4 getViewMatrix() const
	{
		glm::mat4 view = glm::mat4(1.0f);

		// apply rotation (pitch->yaw->roll)
		view = glm::rotate(view, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		view = glm::rotate(view, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		view = glm::rotate(view, glm::radians(rotation.z), glm::vec3(0, 0, 1));

		// apply translation
		view = glm::translate(view, -position);

		return view;
	}

	void update(float dt);
};
