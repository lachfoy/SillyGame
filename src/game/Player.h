#pragma once

#include <glm/glm.hpp>

#include "../engine/Renderer.h"

class Player
{
  public:
	Player();
	~Player();

	void update(float dt);

	void draw();

  private:
	glm::vec3 position = glm::vec3(0, 0, 0);
	float yVel = 0.f;
	bool spaceWasDown = false;
	bool isGrounded = true;

	Texture texture;
	Texture shadowTexture;

	friend class World;
};
