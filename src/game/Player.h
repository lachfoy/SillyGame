#pragma once

#include <glm/glm.hpp>

#include "../engine/Entity.h"
#include "../engine/Renderer.h"

class Player : public Entity
{
  public:
	Player();
	~Player();

	virtual void update(float dt) override;
	virtual void render() override;

  private:
	glm::vec3 position = glm::vec3(0, 0, 0);
	float yVel = 0.f;
	bool spaceWasDown = false;
	bool isGrounded = true;

	Texture texture;
	Texture shadowTexture;
};
