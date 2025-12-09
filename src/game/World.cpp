#include "World.h"

#include "../engine/Engine.h"

void World::init()
{
	player = std::make_unique<Player>();

	unsigned char data[] = {0xff, 0xff, 0xff, 0xff};
	groundTexture = Engine::instance->renderer->createTexture(data, 1, 1);
}

void World::shutdown()
{
	player.reset();
	Engine::instance->renderer->deleteTexture(groundTexture);
}

void World::update(float dt) { player->update(dt); }

void World::render()
{
	// Ground
	Engine::instance->renderer->drawQuad(
		glm::vec3(0, 0, 0), glm::vec3(90, 0, 0), glm::vec3(10, 10, 10),
		glm::vec4(104 / 255.f, 218 / 255.f, 100 / 255.f, 1), groundTexture);

	// Player
	player->draw();
}
