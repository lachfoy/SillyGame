#include "ShadowCaster.h"

#include "../engine/Engine.h"

ShadowCaster::ShadowCaster()
{
	shadowTexture =
		Engine::instance->renderer->loadTexture("gamedata/Shadow_0.png");
}

ShadowCaster::~ShadowCaster()
{
	Engine::instance->renderer->deleteTexture(shadowTexture);
}

void ShadowCaster::drawShadow()
{
	constexpr const float groundY = 0.f;
	constexpr const float lightHeight = 10.f;
	constexpr const float baseShadowScale = 1.f;
	constexpr const float minAlpha = 0.1f;

	float yTop = position.y + 1.f /* top of sprite */;

	// clamp yTop to avoid division by zero / crossing light plane
	const float eps = 0.001f;
	yTop = glm::clamp(yTop, groundY + eps, lightHeight - eps);

	// physically-based scale for a point light straight above
	float shadowScale = (lightHeight - groundY) / (lightHeight - yTop);
	shadowScale *= 0.5f;

	// alpha approximation: higher object => smaller (lighter) shadow
	float shadowAlpha = glm::clamp(
		(lightHeight - yTop) / (lightHeight - groundY), minAlpha, 1.0f);

	// shadow position on ground (tiny epsilon to avoid z-fighting)
	glm::vec3 shadowPos(position.x, groundY + 0.01f, position.z);

	Engine::instance->renderer->drawQuad(
		shadowPos, glm::vec3(-90, 0, 0), glm::vec3(shadowScale),
		glm::vec4(1, 1, 1, shadowAlpha), shadowTexture);
}
