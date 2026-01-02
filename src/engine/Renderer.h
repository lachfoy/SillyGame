#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct Texture
{
	uintptr_t id = 0;
	int width = 0;
	int height = 0;
};

struct Mesh
{
	int64_t id = 0;
};

class Renderer
{
  public:
	Renderer();
	~Renderer();

	bool init();
	void shutdown();

	Texture createTexture(unsigned char *data, int width, int height);
	Texture loadTexture(const char *path);
	void deleteTexture(Texture texture);

	Mesh createQuadMesh();

	void beginFrame();
	void endFrame();

	void clear(float r, float g, float b);

	void drawMesh(Mesh mesh, glm::mat4 transform,
				  Texture texture = sBlankTexture);

	void drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
				  glm::vec4 color, Texture texture = sBlankTexture) const;

  private:
	struct RendererData *mRendererData = nullptr;
	static Texture sBlankTexture;
};
