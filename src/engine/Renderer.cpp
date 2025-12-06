#include "Renderer.h"
#include "Engine.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool Renderer::init()
{
	// --- Camera UBO -----------------------------------------------------
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr,
				 GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the UBO to binding point 0
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

	// --- Shader ---------------------------------------------------------
	const char *vs = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;

		layout(std140) uniform Camera
		{
			mat4 uView;
			mat4 uProj;
			vec3 uCameraPos;
			float _pad0;
		};

        uniform mat4 model;
        uniform vec4 uColor;

        out vec4 fragColor;

        void main()
        {
            fragColor = uColor;
            gl_Position = uProj * uView * model * vec4(aPos, 1.0);
        }
    )";

	const char *fs = R"(
        #version 460 core
        in vec4 fragColor;
        out vec4 FragColor;

        void main()
        {
            FragColor = fragColor;
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

	// Set camera UBO
	GLuint index = glGetUniformBlockIndex(shaderProgram, "Camera");
	glUniformBlockBinding(shaderProgram, index, 0);

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
		glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);

	view = Engine::instance->camera->getViewMatrix();

	return true;
}

void Renderer::shutdown()
{
	if (vbo != 0)
		glDeleteBuffers(1, &vbo);

	if (vao != 0)
		glDeleteVertexArrays(1, &vao);

	if (shaderProgram != 0)
		glDeleteProgram(shaderProgram);

	if (ubo != 0)
		glDeleteBuffers(1, &ubo);
}

void Renderer::beginFrame()
{
	// Update UBO
	CameraData data;
	data.view = Engine::instance->camera->getViewMatrix();
	data.proj = projection; // Right now the camera doesn't decide projection.
							// But this will probably change.
	data.cameraPos = Engine::instance->camera->position;

	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraData), &data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::endFrame()
{
	// stub
}

void Renderer::drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
						glm::vec4 color) const
{
	glUseProgram(shaderProgram);

	glm::mat4 model = glm::mat4(1.0f);

	// Translate
	model = glm::translate(model, position);

	// Rotate xyz
	model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
	model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
	model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});

	// Scale
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1,
					   GL_FALSE, glm::value_ptr(model));

	glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1,
				 glm::value_ptr(color));

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
