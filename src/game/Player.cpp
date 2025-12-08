#include "Player.h"

#include "../engine/Engine.h"

void Player::update(float dt)
{
	constexpr const float moveSpeed = 5.f;
	constexpr const float jumpForce = 10.f;
	constexpr const float gravity = -30.f;
	constexpr const float groundHeight = 0.f;

	float moveX = 0.f, moveZ = 0.f;
	if (Engine::instance->input->isDown(SDLK_W))
		moveZ -= 1.f;
	if (Engine::instance->input->isDown(SDLK_S))
		moveZ += 1.f;
	if (Engine::instance->input->isDown(SDLK_A))
		moveX -= 1.f;
	if (Engine::instance->input->isDown(SDLK_D))
		moveX += 1.f;

	moveZ += Engine::instance->input->axis(SDL_GAMEPAD_AXIS_LEFTY);
	moveX += Engine::instance->input->axis(SDL_GAMEPAD_AXIS_LEFTX);

	// Normalize 2D movement
	float len = std::sqrt(moveX * moveX + moveZ * moveZ);
	if (len > 0.f)
	{
		moveX /= len;
		moveZ /= len;
	}

	position.x += moveX * moveSpeed * dt;
	position.z += moveZ * moveSpeed * dt;

	// ==== Jumping ====
	bool spaceDown = Engine::instance->input->isDown(SDLK_SPACE) ||
					 Engine::instance->input->isDown(SDL_GAMEPAD_BUTTON_SOUTH);

	// Jump when space goes from up to down AND player is grounded
	if (spaceDown && !spaceWasDown && isGrounded)
	{
		yVel = jumpForce;
		isGrounded = false;
	}

	// Gravity
	if (!isGrounded)
	{
		yVel += gravity * dt;
		position.y += yVel * dt;

		if (position.y <= groundHeight)
		{
			position.y = groundHeight;
			yVel = 0;
			isGrounded = true;
		}
	}

	spaceWasDown = spaceDown;
}
