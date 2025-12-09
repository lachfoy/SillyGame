#pragma once

#include "../engine/Renderer.h"
#include "../engine/Entity.h"
#include <memory>
#include <vector>

class World
{
  public:
	void init();
	void shutdown();

	void update(float dt);
	void render();

  private:
	std::vector<std::unique_ptr<Entity>> entities;
};
