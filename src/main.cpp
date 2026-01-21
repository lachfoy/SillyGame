#include "engine/EngineDefs.h"

#include <SDL3/SDL.h>

#include "engine/Engine.h"
#include "game/GameWorld.h"

#include <iostream>

int main(int argc, char *argv[])
{
	try
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

			// --- Handle inputs
			// ---------------------------------------------------
			engine.input->beginFrame();

			while (SDL_PollEvent(&e))
			{
#if WITH_EDITOR
				engine.editor->processEvent(e);
#endif

				if (e.type == SDL_EVENT_QUIT)
					running = false;

				engine.input->handleEvent(e);
			}

			// --- Handle updates
			// --------------------------------------------------
			while (accumulator >= engine.fixedDelta)
			{
				if (engine.mode == 0) // Game mode
				{
					engine.world->update((float)engine.fixedDelta);
				}
#if WITH_EDITOR
				else
				{
					engine.editor->update((float)engine.fixedDelta);
				}
#endif

				accumulator -= engine.fixedDelta;
			}

			// --- Rendering
			// -------------------------------------------------------
			engine.renderer->beginFrame();
			engine.renderer->clear(0.2f, 0.3f, 0.6f);
			//engine.world->render();
			engine.renderer->endFrame();

			engine.renderer->begin2D(1280, 720);
			engine.testUI->Render();
			engine.renderer->end2D();

#if WITH_EDITOR
			engine.editor->beginFrame();
			engine.editor->draw();
			engine.editor->endFrame();
#endif

			SDL_GL_SwapWindow(engine.window);
		}

		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
