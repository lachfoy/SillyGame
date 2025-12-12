#pragma once

#include "../engine/World.h"
#include <memory>
#include <vector>

class Player;

class GameWorld : public World
{
  public:
	virtual void init() override;

	virtual void update(float dt) override;
	virtual void render() override;

  private:
	Player *mPlayer = nullptr;
};
