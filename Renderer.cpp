#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool Renderer::init()
{
	// --- Shader ---------------------------------------------------------
	const char *vs = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

	const char *fs = R"(
        #version 460 core
        out vec4 FragColor;

        void main() {
            FragColor = vec4(1, 1, 1, 1);
        }
    )";

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs, nullptr);
	glCompileShader(vertex);

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs, nullptr);
	glCompileShader(fragment);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertex);
	glAttachShader(shaderProgram, fragment);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// --- Quad Geometry ---------------------------------------------------
	float quadVertices[] = {
		-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.5f, 0.5f,	 0.0f, -0.5f, 0.5f, 0.0f,
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
						  (void *)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// --- Matrices --------------------------------------------------------
	projection =
		glm::perspective(glm::radians(70.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	view =
		glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	return true;
}

void Renderer::shutdown()
{
	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
	}

	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
	}

	if (shaderProgram != 0)
	{
		glDeleteProgram(shaderProgram);
	}
}

void Renderer::draw()
{
	glUseProgram(shaderProgram);

	glm::mat4 model = glm::mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1,
					   GL_FALSE, glm::value_ptr(model));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE,
					   glm::value_ptr(view));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1,
					   GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
