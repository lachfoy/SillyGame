#include "World.h"

#include "../engine/Engine.h"

void World::update(float dt) { player.update(dt); }

void World::render()
{
	// Ground
	Engine::instance->renderer->drawQuad(
		glm::vec3(0, 0, 0), glm::vec3(90, 0, 0), glm::vec3(10, 10, 10),
		glm::vec4(0, 1, 1, 1));

	// Player
	Engine::instance->renderer->drawQuad(
		player.position + glm::vec3(0, 0.5f, 0), glm::vec3(0, 0, 0),
		glm::vec3(1, 1, 1), glm::vec4(1, 1, 1, 1));
}
