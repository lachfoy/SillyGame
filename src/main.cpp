#include "engine/Engine.h"

#include "game/GameWorld.h"

#include <SDL3/SDL.h>
#include <iostream>

int main(int argc, char *argv[])
{
	Engine engine;

	if (!engine.init())
		return 1;

	engine.setWorld(std::make_unique<GameWorld>());

	// Main loop
	double lastTime = SDL_GetTicks() * 0.001;
	double accumulator = 0.0;
	bool running = true;
	SDL_Event e;
	while (running)
	{
		double now = SDL_GetTicks() * 0.001;
		double frameTime = now - lastTime;
		lastTime = now;

		frameTime = std::min(frameTime, 0.25);
		accumulator += frameTime;

		// --- Handle inputs ---------------------------------------------------
		engine.input->beginFrame();

		while (SDL_PollEvent(&e))
		{
#ifdef WITH_EDITOR
			engine.editor->processEvent(e);
#endif

			if (e.type == SDL_EVENT_QUIT)
				running = false;

			engine.input->handleEvent(e);
		}

		// --- Handle updates --------------------------------------------------
		while (accumulator >= engine.fixedDelta)
		{
			if (engine.mode == 0) // Game mode
			{
				engine.world->update((float)engine.fixedDelta);
			}

			engine.camera->update((float)engine.fixedDelta);

			accumulator -= engine.fixedDelta;
		}

		// --- Rendering -------------------------------------------------------
		engine.renderer->beginFrame();
		engine.renderer->clear(0.2f, 0.3f, 0.6f);

		engine.world->render();

#ifdef WITH_EDITOR
		engine.editor->beginFrame();
		engine.editor->draw();
		engine.editor->endFrame();
#endif

		engine.renderer->endFrame();
		SDL_GL_SwapWindow(engine.window);
	}

	engine.shutdown();

	return 0;
}
