#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

struct Texture
{
	uintptr_t id = 0;
	int width = 0;
	int height = 0;
};

struct Buffer
{
	uintptr_t id = 0;
	size_t size = 0;
};

struct VertexLayout
{
	struct Attribute
	{
		uint32_t location;
		uint32_t size;
		uint32_t offset;
	};

	uint32_t stride;
	std::vector<Attribute> attributes;
};

class Mesh;

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

	Buffer createVertexBuffer(const void *data, size_t size);
	Buffer createIndexBuffer(const uint32_t *indices, size_t count);
	void deleteBuffer(Buffer buffer);

	uintptr_t createVertexArray(Buffer vertexBuffer, Buffer indexBuffer,
								const VertexLayout &layout);
	void deleteVertexArray(uintptr_t vao);

	void beginFrame();
	void endFrame();

	void clear(float r, float g, float b);

	void drawMesh(const Mesh &mesh);

	void drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
				  glm::vec4 color, Texture texture = sBlankTexture) const;

  private:
	struct RendererData *mRendererData = nullptr;
	static Texture sBlankTexture;
};
