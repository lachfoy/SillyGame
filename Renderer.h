#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class Renderer
{
  public:
	bool init();
	void shutdown();

	void clear(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void draw();

  private:
	GLuint shaderProgram = 0;
	GLuint vao = 0;
	GLuint vbo = 0;

	glm::mat4 projection;
	glm::mat4 view;
};
