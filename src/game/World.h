#pragma once

#include "Player.h"
#include "../engine/Renderer.h"
#include <memory>

class World
{
  public:
	void init();
	void shutdown();

	void update(float dt);
	void render();

  private:
	std::unique_ptr<Player> player;
	Texture groundTexture;
};
