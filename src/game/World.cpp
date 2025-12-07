#include "World.h"

#include "../engine/Engine.h"

void World::update(float dt)
{
	// stub
}

void World::render()
{
	// Draw the 'player'
	Engine::instance->renderer->drawQuad(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0),
										 glm::vec3(1, 1, 1),
										 glm::vec4(1, 1, 1, 1));
}
