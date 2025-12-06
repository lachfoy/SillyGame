#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct CameraData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 cameraPos;
	float _pad0 = 0.0f; // std140 padding
};

class Renderer
{
  public:
	bool init();
	void shutdown();

	void beginFrame();
	void endFrame();

	void clear(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
				  glm::vec4 color) const;

  private:
	GLuint shaderProgram = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ubo = 0; // Camera ubo

	glm::mat4 projection;
	glm::mat4 view;
};
