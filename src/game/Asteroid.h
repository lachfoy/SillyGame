#pragma once

#include <glm/glm.hpp>

#include "../engine/Renderer.h"
#include "ShadowCaster.h"

class Asteroid : public ShadowCaster
{
  public:
	Asteroid();
	~Asteroid();

	virtual void update(float dt) override;
	virtual void render() override;

  private:
	float yVel = 0.f;
};
