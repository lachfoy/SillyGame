#include "Camera.h"

#include "Engine.h"

void Camera::update(float dt)
{
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
}