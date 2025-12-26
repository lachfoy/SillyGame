#pragma once

#include <glm/glm.hpp>

#include "ShadowCaster.h"
#include "../engine/Renderer.h"

class Player : public ShadowCaster
{
  public:
	Player();
	~Player();

	virtual void update(float dt) override;
	virtual void render() override;

  private:
	float yVel = 0.f;
	bool spaceWasDown = false;
	bool isGrounded = true;

	Texture texture;
};
