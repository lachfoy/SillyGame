#pragma once

#include <vector>

#include "../engine/World.h"

#include "../engine/Renderer.h"
#include "../engine/Mesh.h"

class Player;
class Asteroid;

class GameWorld : public World
{
  public:
	virtual void init() override;

	virtual void update(float dt) override;
	virtual void render() override;

  private:
	Player *mPlayer = nullptr;

	Mesh quadMesh;

};
