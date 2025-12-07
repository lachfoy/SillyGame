#pragma once

#include <SDL3/SDL.h>

class Editor
{
  public:
	void init(SDL_Window *window, SDL_GLContext glContext);
	void shutdown();

	void processEvent(const SDL_Event &e);

	void beginFrame();
	void draw();
	void endFrame();
};
