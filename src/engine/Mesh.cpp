#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
// #define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tiny_obj_loader.h>

#include "Engine.h"

Mesh::Mesh() {}

Mesh::~Mesh() {}

Mesh MeshUtils::createQuadMesh()
{
	auto *renderer = Engine::instance->renderer.get();

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	Vertex vertices[] = {
		{{-1, -1, 0}, {0, 0, 1}, {0, 0}},
		{{1, -1, 0}, {0, 0, 1}, {1, 0}},
		{{1, 1, 0}, {0, 0, 1}, {1, 1}},
		{{-1, 1, 0}, {0, 0, 1}, {0, 1}},
	};

	uint32_t indices[] = {0, 1, 2, 2, 3, 0};

	Mesh mesh;
	mesh.vertexBuffer =
		renderer->createVertexBuffer(vertices, sizeof(vertices));
	mesh.indexBuffer = renderer->createIndexBuffer(indices, 6);
	mesh.indexCount = 6;

	mesh.layout.stride = sizeof(Vertex);
	mesh.layout.attributes = {{0, 3, offsetof(Vertex, position)},
							  //{1, 3, offsetof(Vertex, normal)},
							  {2, 2, offsetof(Vertex, uv)}};

	mesh.vao = renderer->createVertexArray(mesh.vertexBuffer, mesh.indexBuffer,
										   mesh.layout);

	return mesh;
}
