#pragma once

#include "Player.h"
#include <memory>

#include "../engine/Renderer.h"

class World
{
  public:
	void init();
	void shutdown();

	void update(float dt);
	void render();

  private:
	std::unique_ptr<Player> player;
	Texture texture;
};
