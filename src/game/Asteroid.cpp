#include "Asteroid.h"

#include "../engine/Engine.h"

Asteroid::Asteroid() {}

Asteroid::~Asteroid() {}

void Asteroid::update(float dt) {}

void Asteroid::render()
{
	drawShadow();

	Engine::instance->renderer->drawQuad(
		position + glm::vec3(0, 0.5f, 0), glm::vec3(0, 0, 0),
		glm::vec3(0.5f, 0.5f, 0.5f), glm::vec4(1, 1, 1, 1));
}
