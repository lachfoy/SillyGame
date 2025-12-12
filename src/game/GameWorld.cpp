#include "GameWorld.h"

#include "../engine/Engine.h"
#include "Player.h"

void GameWorld::init() { mPlayer = createEntity<Player>(); }

void GameWorld::update(float dt) { World::update(dt); }

void GameWorld::render()
{
	Engine::instance->renderer->drawQuad(
		glm::vec3(0, 0, 0), glm::vec3(90, 0, 0), glm::vec3(10, 10, 10),
		glm::vec4(104 / 255.f, 218 / 255.f, 100 / 255.f, 1));
	World::render();
}
