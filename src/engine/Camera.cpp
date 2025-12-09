#include "Camera.h"

#include "Engine.h"

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

void Camera::update(float dt)
{
#ifdef WITH_EDITOR
	if (Engine::instance->mode == 0)
		return;

	// ----- Movement -----
	glm::vec3 forward;
	forward.x = sin(glm::radians(rotation.y));
	forward.y = 0;
	forward.z = -cos(glm::radians(rotation.y));

	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

	if (Engine::instance->input->isDown(SDLK_W))
		position += forward * moveSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_S))
		position -= forward * moveSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_A))
		position -= right * moveSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_D))
		position += right * moveSpeed * dt;

	// vertical movement
	if (Engine::instance->input->isDown(SDLK_SPACE))
		position.y += moveSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_LCTRL))
		position.y -= moveSpeed * dt;

	//// ----- Rotation -----
	if (Engine::instance->input->isDown(SDLK_UP))
		rotation.x -= rotSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_DOWN))
		rotation.x += rotSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_LEFT))
		rotation.y -= rotSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_RIGHT))
		rotation.y += rotSpeed * dt;

	// prevent pitch flipping
	if (rotation.x > 89.f)
		rotation.x = 89.f;
	if (rotation.x < -89.f)
		rotation.x = -89.f;
#endif
}