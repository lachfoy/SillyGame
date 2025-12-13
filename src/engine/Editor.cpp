#include "Editor.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include "Engine.h"

void Editor::init(SDL_Window *window, SDL_GLContext glContext)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |=
		ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void Editor::shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}

void Editor::processEvent(const SDL_Event &e)
{
	ImGui_ImplSDL3_ProcessEvent(&e);
}

void Editor::update(float dt)
{
	// ----- Movement -----
	constexpr const float moveSpeed = 5.f;
	constexpr const float rotSpeed = 90.0f;

	auto *cam = Engine::instance->camera.get();

	glm::vec3 forward;
	forward.x = sin(glm::radians(cam->rotation.y));
	forward.y = 0;
	forward.z = -cos(glm::radians(cam->rotation.y));

	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

	if (Engine::instance->input->isDown(SDLK_W))
		cam->position += forward * moveSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_S))
		cam->position -= forward * moveSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_A))
		cam->position -= right * moveSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_D))
		cam->position += right * moveSpeed * dt;

	// vertical movement
	if (Engine::instance->input->isDown(SDLK_SPACE))
		cam->position.y += moveSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_LCTRL))
		cam->position.y -= moveSpeed * dt;

	//// ----- Rotation -----
	if (Engine::instance->input->isDown(SDLK_UP))
		cam->rotation.x -= rotSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_DOWN))
		cam->rotation.x += rotSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_LEFT))
		cam->rotation.y -= rotSpeed * dt;
	if (Engine::instance->input->isDown(SDLK_RIGHT))
		cam->rotation.y += rotSpeed * dt;

	// prevent pitch flipping
	if (cam->rotation.x > 89.f)
		cam->rotation.x = 89.f;
	if (cam->rotation.x < -89.f)
		cam->rotation.x = -89.f;
}

void Editor::beginFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void Editor::draw()
{
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGuiWindowFlags windowFlags = 0;
	windowFlags |= ImGuiWindowFlags_NoTitleBar;
	windowFlags |= ImGuiWindowFlags_NoScrollbar;
	windowFlags |= ImGuiWindowFlags_MenuBar;
	windowFlags |= ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoResize;
	windowFlags |= ImGuiWindowFlags_NoCollapse;
	windowFlags |= ImGuiWindowFlags_NoNav;
	windowFlags |= ImGuiWindowFlags_NoBackground;

	ImGui::Begin("Editor Window", NULL, windowFlags);

	if (ImGui::BeginMenuBar())
	{
		ImGui::Combo("Mode", &Engine::instance->mode, "Game\0Editor\0");

		for (auto &tool : mTools)
		{
			ImGui::MenuItem(tool->name.c_str(), nullptr, &tool->open);
		}

		ImGui::EndMenuBar();
	}

	for (auto &tool : mTools)
	{
		if (!tool->open)
			continue;
		if (ImGui::Begin(tool->name.c_str(), &tool->open))
		{
			tool->draw();
			ImGui::End();
		}
	}

	ImGui::End();
}

void Editor::endFrame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
