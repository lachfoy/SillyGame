#pragma once

#include <vector>

#include "Renderer.h"

class Mesh
{
  public:
	Mesh();
	~Mesh();

	Buffer indexBuffer;
	Buffer vertexBuffer;
	uint32_t indexCount = 0;
	VertexLayout layout;

    uintptr_t vao = 0; // opaque VAO handle
};

namespace MeshUtils
{
Mesh createQuadMesh();
}
