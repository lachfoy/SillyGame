#pragma once

#include <glm/glm.hpp>

class Player
{
  public:
	void update(float dt);

  private:
	glm::vec3 position = glm::vec3(0, 0, 0);
	float yVel = 0.f;
	bool spaceWasDown = false;
	bool isGrounded = true;

	friend class World;
};
