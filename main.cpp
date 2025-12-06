#include "Engine.h"
#include <SDL3/SDL.h>
#include <iostream>

int main(int argc, char *argv[])
{
	Engine engine;

	if (!engine.init())
		return 1;

	// Main loop
	bool running = true;
	SDL_Event e;
	while (running)
	{
		engine.input->beginFrame();

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT)
				running = false;

			engine.input->handleEvent(e);
		}

		engine.renderer->clear(0.2f, 0.3f, 0.6f);
		engine.renderer->drawQuad(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0),
								  glm::vec3(1, 1, 1), glm::vec4(1, 1, 1, 1));

		SDL_GL_SwapWindow(engine.window);
	}

	engine.shutdown();

	return 0;
}
