#include "Input.h"

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
}
