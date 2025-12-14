#include "GameWorld.h"

#include "../engine/Engine.h"
#include "Player.h"

struct params_Camera
{
	float baseDistance = 7.5f;
	float zoomFactor = 0.6f;
	float followSpeed = 3.0f;
	float cameraHeight = 2.5f;
	float playerHeight = 0.7f;
	float pitchSpeed = 0.5f;
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
		ImGui::SliderFloat("baseDistance", &params->baseDistance, 0.0f, 20.f);
		ImGui::SliderFloat("zoomFactor", &params->zoomFactor, 0.1f, 2.f);
		ImGui::SliderFloat("followSpeed", &params->followSpeed, 1.f, 10.f);
		ImGui::SliderFloat("cameraHeight", &params->cameraHeight, 0.f, 8.8f);
		ImGui::SliderFloat("playerHeight", &params->playerHeight, 0.f, 8.8f);
		ImGui::SliderFloat("pitchSpeed", &params->pitchSpeed, 0.5f, 3.f);
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

#if 1
	auto *cam = Engine::instance->camera.get();

	glm::vec3 playerTarget = mPlayer->position;
	playerTarget.y += cameraParams.playerHeight;
	std::vector<glm::vec3> targets = {playerTarget};

	glm::vec3 deadzone = glm::vec3(1.0f, 0.5f, 1.0f);

	glm::vec3 min = targets[0];
	glm::vec3 max = targets[0];

	for (const auto &p : targets)
	{
		min = glm::min(min, p);
		max = glm::max(max, p);
	}

	glm::vec3 center = (min + max) * 0.5f;
	glm::vec3 extents = max - min;

	float spread = glm::length(extents); // how big the action is
	float desiredDistance =
		cameraParams.baseDistance + spread * cameraParams.zoomFactor;

	glm::vec3 desiredPos = center;
	desiredPos.y += cameraParams.cameraHeight;
	desiredPos.z += desiredDistance;


	// Separate vertical speed to add a dampening
	cam->position.y =
		glm::mix(cam->position.y, desiredPos.y,
				 1.0f - exp(-(cameraParams.followSpeed * 0.5f) * dt));

	cam->position.x = glm::mix(cam->position.x, desiredPos.x,
							   1.0f - exp(-cameraParams.followSpeed * dt));
	cam->position.z = glm::mix(cam->position.z, desiredPos.z,
							   1.0f - exp(-cameraParams.followSpeed * dt));

	// Calculate look-at only affecting pitch
	glm::vec3 d = center - cam->position;
	float distXZ = glm::sqrt(d.x * d.x + d.z * d.z);
	float desiredPitch = glm::degrees(atan2(d.y, distXZ));
	cam->rotation.x = glm::mix(cam->rotation.x, desiredPitch,
							   1.0f - exp(-cameraParams.pitchSpeed * dt));
#endif
}

void GameWorld::render()
{
	Engine::instance->renderer->drawQuad(
		glm::vec3(0, 0, 0), glm::vec3(90, 0, 0), glm::vec3(10, 10, 10),
		glm::vec4(104 / 255.f, 218 / 255.f, 100 / 255.f, 1));

	World::render();
}
