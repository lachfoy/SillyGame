#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <unordered_map>

struct Texture
{
	uint32_t id = 0;
	int width = 0;
	int height = 0;
};

class Renderer
{
  public:
	bool init();
	void shutdown();

	Texture loadTexture(const char *path);
	void destroyTexture(Texture texture);

	void beginFrame();
	void endFrame();

	void clear(float r, float g, float b);

	void drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
				  glm::vec4 color) const;

  private:
	uint32_t ubo = 0; // Camera ubo

	struct CameraData
	{
		glm::mat4 view{};
		glm::mat4 proj{};
		glm::vec3 cameraPos{};
		float _pad0 = 0.0f; // std140 padding
	};

	uint32_t shaderProgram = 0;
	uint32_t vao = 0;
	uint32_t vbo = 0;
};
