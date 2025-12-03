#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		printf("SDL Init Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
						SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_Window *window =
		SDL_CreateWindow("SillyGame", 800, 600, SDL_WINDOW_OPENGL);
	if (!window)
	{
		printf("Window Error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_GLContext glctx = SDL_GL_CreateContext(window);
	if (!glctx)
	{
		printf("Context Error: %s\n", SDL_GetError());
		return 1;
	}

	// Load GL functions
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return 1;
	}

	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

	// Main loop
	bool running = true;
	SDL_Event e;

	while (running)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT)
				running = false;
		}

		glClearColor(0.2f, 0.3f, 0.6f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DestroyContext(glctx);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
