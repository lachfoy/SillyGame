#pragma once

#include <vector>

#include "../engine/World.h"

class Player;
class Asteroid;

#include "../engine/Renderer.h" // mesh

class GameWorld : public World
{
  public:
	virtual void init() override;

	virtual void update(float dt) override;
	virtual void render() override;

  private:
	Player *mPlayer = nullptr;
	Mesh mesh;
};
