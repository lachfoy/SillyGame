#pragma once

#include <SDL3/SDL.h>
#include <memory>

#include "../game/World.h"
#include "Camera.h"
#include "Input.h"
#include "Renderer.h"

#define WITH_EDITOR 1

#ifdef WITH_EDITOR
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#endif

struct Engine
{
	SDL_Window *window = nullptr;
	SDL_GLContext glContext = nullptr;

	std::unique_ptr<Input> input;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Renderer> renderer;
	std::unique_ptr<World> world;

#ifdef WITH_EDITOR
	bool editorMode = false;
#endif

	double fixedDelta = 1.0 / 60.0;
	double accumulator = 0.0;

	static Engine *instance;

	Engine() { instance = this; }
	~Engine() { instance = nullptr; }

	bool init();
	void shutdown();
};
