#include "World.h"

#include "../engine/Engine.h"
#include "Player.h"

void World::init() { entities.push_back(std::make_unique<Player>()); }

void World::shutdown() { entities.clear(); }

void World::update(float dt)
{
	for (auto &e : entities)
		e->update(dt);
}

void World::render()
{
	Engine::instance->renderer->drawQuad(
		glm::vec3(0, 0, 0), glm::vec3(90, 0, 0), glm::vec3(10, 10, 10),
		glm::vec4(104 / 255.f, 218 / 255.f, 100 / 255.f, 1));

	for (auto &e : entities)
		e->render();
}
