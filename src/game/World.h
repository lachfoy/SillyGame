#pragma once

#include "../engine/Entity.h"
#include "../engine/Renderer.h"
#include <memory>
#include <vector>

class World
{
  public:
	void init();
	void shutdown();

	template <typename T, typename... Args> T *createEntity(Args &&...args)
	{
		static_assert(std::is_base_of_v<Entity, T>);
		auto e = std::make_unique<T>(std::forward<Args>(args)...);
		/*e->mWorld = this;*/
		T *ptr = e.get();
		entities.push_back(std::move(e));
		return ptr;
	}

	void update(float dt);
	void render();

  private:
	std::vector<std::unique_ptr<Entity>> entities;
};
