#pragma once

#include <glad/glad.h>

class Renderer
{
  public:
	void clear(float r, float g, float b)
	{
		glClearColor(r, g, b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void draw()
	{
		// stub
	}
};
