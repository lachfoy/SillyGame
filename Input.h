#pragma once

#include <SDL3/SDL.h>
#include <unordered_set>

class Input
{
  public:
	void beginFrame() { framePressed.clear(); }

	void handleEvent(const SDL_Event &event);

	bool isDown(SDL_Keycode key) const { return held.find(key) != held.end(); }
	bool pressed(SDL_Keycode key) const
	{
		return framePressed.find(key) != framePressed.end();
	}

  private:
	std::unordered_set<SDL_Keycode> held;
	std::unordered_set<SDL_Keycode> framePressed;
};
