#include "Player.h"

#include "../engine/Engine.h"

Player::Player()
{
	texture = Engine::instance->renderer->loadTexture("gamedata/Stick.png");
	shadowTexture =
		Engine::instance->renderer->loadTexture("gamedata/Shadow_0.png");
}

Player::~Player()
{
	Engine::instance->renderer->deleteTexture(texture);
	Engine::instance->renderer->deleteTexture(shadowTexture);
}

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

void Player::draw()
{
	constexpr const float groundY = 0.f;
	constexpr const float lightHeight = 10.f;
	constexpr const float baseShadowScale = 1.f;
	constexpr const float minAlpha = 0.1f;

	float yTop = position.y + 1.f /* top of sprite */;

	// clamp yTop to avoid division by zero / crossing light plane
	const float eps = 0.001f;
	yTop = glm::clamp(yTop, groundY + eps, lightHeight - eps);

	// physically-based scale for a point light straight above
	float shadowScale = (lightHeight - groundY) / (lightHeight - yTop);
	shadowScale *= 0.5f;

	// alpha approximation: higher object => smaller (lighter) shadow
	float shadowAlpha = glm::clamp(
		(lightHeight - yTop) / (lightHeight - groundY), minAlpha, 1.0f);

	// shadow position on ground (tiny epsilon to avoid z-fighting)
	glm::vec3 shadowPos(position.x, groundY + 0.01f, position.z);

	Engine::instance->renderer->drawQuad(
		shadowPos, glm::vec3(-90, 0, 0), glm::vec3(shadowScale),
		glm::vec4(1, 1, 1, shadowAlpha), shadowTexture);

	Engine::instance->renderer->drawQuad(position + glm::vec3(0, 0.5f, 0),
										 glm::vec3(0, 0, 0), glm::vec3(1, 1, 1),
										 glm::vec4(1, 1, 1, 1), texture);
}
