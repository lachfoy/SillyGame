#pragma once

#include "Player.h"

class World
{
  public:
	void update(float dt);
	void render();

  private:
	Player player;
};
