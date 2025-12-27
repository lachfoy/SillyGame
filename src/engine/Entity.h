#pragma once

#include <glm/glm.hpp>

class Entity
{
  public:
	virtual ~Entity() = default;

	virtual void update(float dt) {}
	virtual void render() {}

	void destroy() { mPendingDestroy = true; }
	bool isPendingDestroy() const { return mPendingDestroy; }

	glm::vec3 position = glm::vec3(0, 0, 0);

  private:
	bool mPendingDestroy = false;
};
