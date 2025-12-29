#pragma once

#include "EngineDefs.h"

#include <SDL3/SDL.h>
#include <memory>

#include "Camera.h"
#include "Editor.h"
#include "Input.h"
#include "Renderer.h"
#include "World.h"

struct Engine
{
	Engine() { instance = this; }
	~Engine() { instance = nullptr; }

	bool init();
	void shutdown();

	void setWorld(std::unique_ptr<World> world);

	SDL_Window *window = nullptr;
	SDL_GLContext glContext = nullptr;

	std::unique_ptr<Input> input;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<World> world;
#if WITH_EDITOR
	std::unique_ptr<Editor> editor;
#endif

	int mode = 0;

	double fixedDelta = 1.0 / 60.0;
	double accumulator = 0.0;

	static Engine *instance;
};
