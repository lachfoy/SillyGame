#pragma once

class Entity
{
  public:
	virtual ~Entity() {}

	virtual void update(float dt) {}
	virtual void render() {}
};
