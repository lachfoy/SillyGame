#include "engine/Engine.h"

#include <SDL3/SDL.h>
#include <iostream>

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

		frameTime = std::min(frameTime, 0.25);
		accumulator += frameTime;

		// --- Handle inputs ---------------------------------------------------
		engine.input->beginFrame();

		while (SDL_PollEvent(&e))
		{
#ifdef WITH_EDITOR
			ImGui_ImplSDL3_ProcessEvent(&e);
#endif

			if (e.type == SDL_EVENT_QUIT)
				running = false;

			engine.input->handleEvent(e);
		}

		// --- Handle updates --------------------------------------------------
		while (accumulator >= engine.fixedDelta)
		{
#ifdef WITH_EDITOR
			if (engine.input->pressed(SDLK_TAB))
				engine.editorMode = !engine.editorMode;
#endif

			engine.camera->update((float)engine.fixedDelta);

#ifdef WITH_EDITOR
			if (engine.editorMode)
			{
			}
			else
#endif
				engine.world->update((float)engine.fixedDelta);

			accumulator -= engine.fixedDelta;
		}

		// --- Rendering -------------------------------------------------------
#ifdef WITH_EDITOR
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
#endif

		engine.renderer->beginFrame();
		engine.renderer->clear(0.2f, 0.3f, 0.6f);

		engine.world->render();

		engine.renderer->endFrame();

		ImGui::Begin("Another Window");
		ImGui::Text("Hello from another window!");
		ImGui::End();

#ifdef WITH_EDITOR
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

		SDL_GL_SwapWindow(engine.window);
	}

#ifdef WITH_EDITOR
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
#endif

	engine.shutdown();

	return 0;
}
