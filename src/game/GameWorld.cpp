#include "GameWorld.h"

#include "../engine/Engine.h"
#include "Asteroid.h"
#include "Player.h"

inline float rand01()
{
	return static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) + 1.0f);
}

static glm::vec3 randomPointInCube(glm::vec3 center, float size)
{
	const float h = size * 0.5f;

	return glm::vec3(center.x + (rand01() * 2.0f - 1.0f) * h,
					 center.y + (rand01() * 2.0f - 1.0f) * h,
					 center.z + (rand01() * 2.0f - 1.0f) * h);
}

struct Camera_params
{
	float baseDistance = 7.5f;
	float zoomFactor = 0.6f;
	float followSpeed = 3.0f;
	float cameraHeight = 2.5f;
	float playerHeight = 0.7f;
	// float pitchSpeed = 0.5f;
	float verticalSpeedDamping = 0.5f;
};

struct Lighting_params
{
	glm::vec3 lightPos = {0, 10, -10};
	glm::vec3 lightColor = {1, 1, 1};
	glm::vec3 ambient = {0.1, 0.1, 0.1};
};

#if WITH_EDITOR
#include <imgui.h>
struct CameraParams : public EditorTool
{
	CameraParams(Camera_params *_params)
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
		ImGui::SliderFloat("verticalSpeedDamping",
						   &params->verticalSpeedDamping, 0.0f, 1.f);
	}

  private:
	Camera_params *params;
};

struct LightingParams : public EditorTool
{
	LightingParams(Lighting_params *_params)
		: EditorTool("LightingParams"), params(_params)
	{
	}

	void draw() override
	{
		ImGui::DragFloat3("lightPos", &params->lightPos[0], 0.1f);
		ImGui::ColorEdit3("lightColor", &params->lightColor[0]);
		ImGui::ColorEdit3("ambient", &params->ambient[0]);
	}

  private:
	Lighting_params *params;
};
#endif

Camera_params cameraParams;
Lighting_params lightingParams;

void GameWorld::init()
{
	mPlayer = createEntity<Player>();

	// mesh = Engine::instance->renderer->createQuadMesh();
	mesh = Engine::instance->renderer->loadMesh("gamedata/Suzanne.obj");

#if WITH_EDITOR
	Engine::instance->editor->registerTool<CameraParams>(&cameraParams);
	Engine::instance->editor->registerTool<LightingParams>(&lightingParams);
#endif
}

void GameWorld::update(float dt)
{
	World::update(dt);

	auto *cam = Engine::instance->camera.get();

	glm::vec3 playerTarget = mPlayer->position;
	playerTarget.y += cameraParams.playerHeight;
	std::vector<glm::vec3> targets = {playerTarget};

	auto asteroids = World::view<Asteroid>();
	for (const auto &asteroid : asteroids)
	{
		targets.push_back(asteroid->position);
	}

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
	cam->position.y = glm::mix(cam->position.y, desiredPos.y,
							   1.0f - exp(-(cameraParams.followSpeed *
											cameraParams.verticalSpeedDamping) *
										  dt));

	cam->position.x = glm::mix(cam->position.x, desiredPos.x,
							   1.0f - exp(-cameraParams.followSpeed * dt));
	cam->position.z = glm::mix(cam->position.z, desiredPos.z,
							   1.0f - exp(-cameraParams.followSpeed * dt));

	if (Engine::instance->input->pressed(SDLK_Z))
	{
		auto *asteroid = createEntity<Asteroid>();
		asteroid->position = randomPointInCube(glm::vec3(0, 4.5f, 0), 5);
	}

	Engine::instance->renderer->setLighting(lightingParams.lightPos,
											lightingParams.lightColor,
											lightingParams.ambient);
}

void GameWorld::render()
{
	 Engine::instance->renderer->drawQuad(
		glm::vec3(0, 0, 0), glm::vec3(90, 0, 0), glm::vec3(10, 10, 10),
		glm::vec4(104 / 255.f, 218 / 255.f, 100 / 255.f, 1));

	Engine::instance->renderer->drawMesh(mesh, glm::mat4(1.0f));

	World::render();
}
