#include "Engine.h"
#include <SDL3/SDL.h>
#include <iostream>

glm::vec3 position = {};
glm::vec3 velocity = {};

int main(int argc, char *argv[])
{
	Engine engine;

	if (!engine.init())
		return 1;

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

		if (frameTime > 0.25)
			frameTime = 0.25;

		accumulator += frameTime;

		// --- Handle inputs ---------------------------------------------------
		engine.input->beginFrame();

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT)
				running = false;

			engine.input->handleEvent(e);
		}

		while (accumulator >= engine.fixedDelta)
		{
			engine.camera->update(engine.fixedDelta);
			accumulator -= engine.fixedDelta;
		}

		/*velocity = glm::vec3(0.f);
		if (engine.input->isDown(SDLK_W))
			velocity.y = 1;
		if (engine.input->isDown(SDLK_A))
			velocity.x = -1;
		if (engine.input->isDown(SDLK_S))
			velocity.y = -1;
		if (engine.input->isDown(SDLK_D))
			velocity.x = 1;
		glm::normalize(velocity);
		position += velocity * 0.05f;*/

		engine.renderer->clear(0.2f, 0.3f, 0.6f);
		engine.renderer->drawQuad(position, glm::vec3(0, 0, 0),
								  glm::vec3(1, 1, 1), glm::vec4(1, 1, 1, 1));

		SDL_GL_SwapWindow(engine.window);
	}

	engine.shutdown();

	return 0;
}
