#pragma once

#include "Entity.h"
#include <memory>
#include <vector>

class World
{
  public:
	virtual ~World() {}

	virtual void init() {}
	virtual void shutdown() { mEntities.clear(); }

	template <typename T, typename... Args> T *createEntity(Args &&...args)
	{
		static_assert(std::is_base_of_v<Entity, T>);
		auto e = std::make_unique<T>(std::forward<Args>(args)...);
		/*e->mWorld = this;*/
		T *ptr = e.get();
		mEntities.push_back(std::move(e));
		return ptr;
	}

	virtual void update(float dt)
	{
		for (auto &e : mEntities)
			e->update(dt);
	}

	virtual void render()
	{
		for (auto &e : mEntities)
			e->render();
	}

  private:
	std::vector<std::unique_ptr<Entity>> mEntities;
};
