#include "Camera.h"

glm::mat4 Camera::getViewMatrix() const
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
