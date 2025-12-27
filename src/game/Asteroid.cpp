#include "Asteroid.h"

#include "../engine/Engine.h"

Asteroid::Asteroid() {}

Asteroid::~Asteroid() {}

void Asteroid::update(float dt)
{
	// TODO: move these into game params or world state?
	constexpr const float gravity = -30.f;
	constexpr const float groundHeight = 0.f;

	yVel += gravity * dt;
	position.y += yVel * dt;

	if (position.y <= groundHeight)
	{
		this->destroy();
	}
}

void Asteroid::render()
{
	drawShadow();

	Engine::instance->renderer->drawQuad(position, glm::vec3(0, 0, 0),
										 glm::vec3(0.5f, 0.5f, 0.5f),
										 glm::vec4(1, 1, 1, 1));
}
