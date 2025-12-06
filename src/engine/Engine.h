#pragma once

#include <SDL3/SDL.h>
#include <memory>

#include "Input.h"
#include "AssetManager.h"
#include "Renderer.h"

struct Engine
{
	SDL_Window *window = nullptr;
	SDL_GLContext glContext = nullptr;

	std::unique_ptr<Input> input;
	std::unique_ptr<AssetManager> assets;
	std::unique_ptr<Renderer> renderer;

	static Engine *instance;

	Engine() { instance = this; }
	~Engine() { instance = nullptr; }

	bool init();
	void shutdown();
};
