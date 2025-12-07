#pragma once

#include <glm/glm.hpp>
#include <vector>

class Renderer
{
  public:
	bool init();
	void shutdown();

	unsigned int loadTexture(const char *path);

	void beginFrame();
	void endFrame();

	void clear(float r, float g, float b);

	void drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
				  glm::vec4 color) const;

  private:
	struct Texture
	{
		unsigned int id = 0;
		int width = 0;
		int height = 0;
	};

	struct CameraData
	{
		glm::mat4 view{};
		glm::mat4 proj{};
		glm::vec3 cameraPos{};
		float _pad0 = 0.0f; // std140 padding
	};

	unsigned int shaderProgram = 0;
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ubo = 0; // Camera ubo

	glm::mat4 projection{};
	glm::mat4 view{};

	std::vector<Texture> textures;
};
