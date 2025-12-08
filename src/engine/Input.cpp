#include "Input.h"

#include <iostream>

void Input::handleEvent(const SDL_Event &e)
{
	if (e.type == SDL_EVENT_KEY_DOWN)
	{
		held.insert(e.key.key);
		framePressed.insert(e.key.key);
	}
	else if (e.type == SDL_EVENT_KEY_UP)
	{
		held.erase(e.key.key);
	}

	// Gamepad
	switch (e.type)
	{
	case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		gamepad.held.insert((SDL_GamepadButton)e.gbutton.button);
		gamepad.framePressed.insert((SDL_GamepadButton)e.gbutton.button);
		break;
	case SDL_EVENT_GAMEPAD_BUTTON_UP:
		gamepad.held.erase((SDL_GamepadButton)e.gbutton.button);
		break;
	case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		gamepad.axes[e.gaxis.axis] = e.gaxis.value / 32767.f;
		break;
	}
}

float Input::applyDeadzone(float value)
{
	constexpr const float dz = 0.2f;

	if (std::fabs(value) < dz)
		return 0.0f;

	float sign = (value > 0.f) ? 1.f : -1.f;
	float scaled = (std::fabs(value) - dz) / (1.f - dz);
	return scaled * sign;
}
