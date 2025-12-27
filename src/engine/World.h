#pragma once

#include "Entity.h"
#include <gsl/span>
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

	template <typename T> gsl::span<T *> view()
	{
		static_assert(std::is_base_of_v<Entity, T>);

		mViewScratch.clear();

		for (auto &e : mEntities)
		{
			if (auto *t = dynamic_cast<T *>(e.get()))
				mViewScratch.push_back(t);
		}

		return {reinterpret_cast<T **>(mViewScratch.data()),
				mViewScratch.size()};
	}

	virtual void update(float dt)
	{
		for (auto &e : mEntities)
			e->update(dt);

		mEntities.erase(std::remove_if(mEntities.begin(), mEntities.end(),
									   [](const auto &e)
									   { return e->isPendingDestroy(); }),
						mEntities.end());
	}

	virtual void render()
	{
		for (auto &e : mEntities)
			e->render();
	}

  private:
	std::vector<std::unique_ptr<Entity>> mEntities;
	std::vector<Entity *> mViewScratch; // scratch mem for allocation
};
