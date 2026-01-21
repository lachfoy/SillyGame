#include "Engine.h"

#include <glad/glad.h>
#include <iostream>
#include <SDL3/SDL_gamepad.h>

Engine *Engine::instance = nullptr;

Engine::~Engine()
{
	if (world)
	{
		world->shutdown();
		world.reset();
	}

	testUI.reset();

	renderer->shutdown();
	renderer.reset();

	camera.reset();
	input.reset();

#if WITH_EDITOR
	editor->shutdown();
	editor.reset();
#endif

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

	instance = nullptr;
}

bool Engine::init()
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
						SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("SillyGame", 1280, 720, SDL_WINDOW_OPENGL);
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

#if WITH_EDITOR
	editor = std::make_unique<Editor>();
	editor->init(window, glContext);
#endif

	input = std::make_unique<Input>();

	camera = std::make_unique<Camera>();

	renderer = std::make_unique<Renderer>();
	renderer->init();

	testUI = std::make_unique<TestUI>();
	testUI->Init();

	return true;
}

void Engine::setWorld(std::unique_ptr<World> _world)
{
	world = std::move(_world);
	world->init();
}
