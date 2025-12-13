#pragma once

#include <glm/glm.hpp>

class Entity
{
  public:
	virtual ~Entity() {}

	virtual void update(float dt) {}
	virtual void render() {}

	glm::vec3 position = glm::vec3(0, 0, 0);
};
