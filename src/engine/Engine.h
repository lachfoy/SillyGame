#pragma once

#include <SDL3/SDL.h>
#include <memory>

#include "Input.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "Camera.h"

struct Engine
{
	SDL_Window *window = nullptr;
	SDL_GLContext glContext = nullptr;

	std::unique_ptr<Input> input;
	std::unique_ptr<AssetManager> assets;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<Renderer> renderer;

	bool debugCameraEnabled = true;

	double fixedDelta = 1.0 / 60.0;
	double accumulator = 0.0;

	static Engine *instance;

	Engine() { instance = this; }
	~Engine() { instance = nullptr; }

	bool init();
	void shutdown();
};
