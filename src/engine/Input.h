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

	bool isDown(SDL_GamepadButton button) const
	{
		return gamepad.held.count(button);
	}
	bool pressed(SDL_GamepadButton button) const
	{
		return gamepad.framePressed.count(button);
	}

	float axis(SDL_GamepadAxis axis) const
	{
		return applyDeadzone(gamepad.axes[axis]);
	}

  private:
	static float applyDeadzone(float value);

	std::unordered_set<SDL_Keycode> held;
	std::unordered_set<SDL_Keycode> framePressed;

	struct
	{
		std::unordered_set<SDL_GamepadButton> held;
		std::unordered_set<SDL_GamepadButton> framePressed;
		float axes[SDL_GAMEPAD_AXIS_COUNT]{};
	} gamepad;
};
