#include "Engine.h"

#include <glad/glad.h>
#include <iostream>

Engine *Engine::instance = nullptr;

bool Engine::init()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
						SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("SillyGame", 800, 600, SDL_WINDOW_OPENGL);
	if (!window)
	{
		std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
		return false;
	}

	glContext = SDL_GL_CreateContext(window);
	if (!glContext)
	{
		std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
		return false;
	}

	// Load GL functions
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD\n";
		return false;
	}

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	input = std::make_unique<Input>();

	camera = std::make_unique<Camera>();

	renderer = std::make_unique<Renderer>();
	renderer->init();

	return true;
}

void Engine::shutdown()
{
	renderer->shutdown();
	renderer.reset();

	camera.reset();
	input.reset();

	if (glContext)
	{
		SDL_GL_DestroyContext(glContext);
		glContext = nullptr;
	}

	if (window)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	SDL_Quit();
}
