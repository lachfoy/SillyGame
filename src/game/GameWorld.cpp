#include "GameWorld.h"

#include "../engine/Engine.h"
#include "Player.h"

struct params_Camera
{
	float baseDistance = 8.0f;
	float zoomFactor = 0.6f;
	float followSpeed = 3.0f;
};

#ifdef WITH_EDITOR
#include <imgui.h>
struct CameraParams : public EditorTool
{
	CameraParams(params_Camera *_params)
		: EditorTool("CameraParams"), params(_params)
	{
	}

	void draw() override
	{
		ImGui::DragFloat("baseDistance", &params->baseDistance, 0.0f, 5.f,
						 20.f);
		ImGui::DragFloat("zoomFactor", &params->zoomFactor, 0.1f, 0.6f, 2.f);
	}

  private:
	params_Camera *params;
};
#endif

params_Camera cameraParams;

void GameWorld::init()
{
	mPlayer = createEntity<Player>();

#ifdef WITH_EDITOR
	Engine::instance->editor->registerTool<CameraParams>(&cameraParams);
#endif
}

void GameWorld::update(float dt)
{
	World::update(dt);

	auto *cam = Engine::instance->camera.get();

	std::vector<glm::vec3> targets = {mPlayer->position};

	glm::vec3 min = targets[0];
	glm::vec3 max = targets[0];

	for (const auto &p : targets)
	{
		min = glm::min(min, p);
		max = glm::max(max, p);
	}

	glm::vec3 center = (min + max) * 0.5f;
	glm::vec3 extents = max - min;

	constexpr const float baseDistance = 8.0f;
	constexpr const float zoomFactor = 0.6f;
	float spread = glm::length(extents); // how big the action is
	float desiredDistance = baseDistance + spread * zoomFactor;

	glm::vec3 cameraDir =
		glm::normalize(glm::vec3(0.0f, 0.0f, 1.f)); // angled down
	glm::vec3 desiredPos =
		center + glm::vec3(0, 1.f, 0) + cameraDir * desiredDistance;

	constexpr const float followSpeed = 3.0f;

	cam->position =
		glm::mix(cam->position, desiredPos, 1.0f - exp(-followSpeed * dt));
	cam->lookAt(center);
}

void GameWorld::render()
{
	Engine::instance->renderer->drawQuad(
		glm::vec3(0, 0, 0), glm::vec3(90, 0, 0), glm::vec3(10, 10, 10),
		glm::vec4(104 / 255.f, 218 / 255.f, 100 / 255.f, 1));
	World::render();
}
