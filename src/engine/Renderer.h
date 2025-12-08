#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct Texture
{
	int64_t id = 0;
	int width = 0;
	int height = 0;
};

class Renderer
{
  public:
	Renderer();
	~Renderer();

	bool init();
	void shutdown();

	Texture loadTexture(const char *path);
	void deleteTexture(Texture texture);

	void beginFrame();
	void endFrame();

	void clear(float r, float g, float b);

	void drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
				  glm::vec4 color) const;

  private:
	struct RenderData *mRenderData = nullptr;

	struct CameraData
	{
		glm::mat4 view{};
		glm::mat4 proj{};
		glm::vec3 cameraPos{};
		float _pad0 = 0.0f; // std140 padding
	};
};
