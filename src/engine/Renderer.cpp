#include "Renderer.h"
#include "Engine.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma warning(push)
#pragma warning(disable : 26451)
#pragma warning(disable : 26819)
#pragma warning(disable : 6262)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include <iostream>

struct GLTexture
{
	GLuint id;
	int width;
	int height;
};

struct RenderData
{
	GLuint ubo = 0; // Camera ubo
	GLuint shaderProgram = 0;
	GLuint vao = 0;
	GLuint vbo = 0;
};

Renderer::Renderer() { data = new RenderData(); }

Renderer::~Renderer() { delete data; }

bool Renderer::init()
{
	// --- Camera UBO -----------------------------------------------------
	glGenBuffers(1, &data->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, data->ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr,
				 GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the UBO to binding point 0
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, data->ubo);

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

	data->shaderProgram = glCreateProgram();
	glAttachShader(data->shaderProgram, vertex);
	glAttachShader(data->shaderProgram, fragment);
	glLinkProgram(data->shaderProgram);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// Set camera UBO
	GLuint index = glGetUniformBlockIndex(data->shaderProgram, "Camera");
	glUniformBlockBinding(data->shaderProgram, index, 0);

	// --- Quad Geometry ---------------------------------------------------
	float quadVertices[] = {
		-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f, 0.5f, 0.5f,	 0.0f, -0.5f, 0.5f, 0.0f,
	};

	glGenVertexArrays(1, &data->vao);
	glGenBuffers(1, &data->vbo);

	glBindVertexArray(data->vao);
	glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
						  (void *)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	// --- GL State --------------------------------------------------------
	glEnable(GL_DEPTH_TEST);

	return true;
}

void Renderer::shutdown()
{
	if (data->vbo != 0)
	{
		glDeleteBuffers(1, &data->vbo);
		data->vbo = 0;
	}

	if (data->vao != 0)
	{
		glDeleteVertexArrays(1, &data->vao);
		data->vao = 0;
	}

	if (data->shaderProgram != 0)
	{
		glDeleteProgram(data->shaderProgram);
		data->shaderProgram = 0;
	}

	if (data->ubo != 0)
	{
		glDeleteBuffers(1, &data->ubo);
		data->ubo = 0;
	}
}

Texture Renderer::loadTexture(const char *path)
{
	int w, h, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(path, &w, &h, &channels, 4);

	if (!data)
	{
		std::cerr << "Failed to load: " << path << std::endl;
		return {};
	}

	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	// Upload
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				 data);

	// Sampler parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);

	GLTexture *tex = new GLTexture();
	tex->id = id;
	tex->width = w;
	tex->height = h;

	return {reinterpret_cast<int64_t>(tex), w, h};
}

void Renderer::deleteTexture(Texture texture)
{
	GLTexture *tex = reinterpret_cast<GLTexture *>(texture.id);

	glDeleteTextures(1, &tex->id);

	delete tex;
}

void Renderer::beginFrame()
{
	// Update UBO
	CameraData data;
	data.view = Engine::instance->camera->getViewMatrix();
	data.proj = glm::perspective(
		glm::radians(50.0f), 800.0f / 600.0f, 0.1f,
		100.0f); // Right now the camera doesn't decide projection.
	data.cameraPos = Engine::instance->camera->position;

	glBindBuffer(GL_UNIFORM_BUFFER, this->data->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraData), &data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::endFrame()
{
	// stub
}

void Renderer::clear(float r, float g, float b)
{
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
						glm::vec4 color) const
{
	glUseProgram(data->shaderProgram);

	glm::mat4 model = glm::mat4(1.0f);

	// Translate
	model = glm::translate(model, position);

	// Rotate xyz
	model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
	model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
	model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});

	// Scale
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

	glUniformMatrix4fv(glGetUniformLocation(data->shaderProgram, "model"), 1,
					   GL_FALSE, glm::value_ptr(model));

	glUniform4fv(glGetUniformLocation(data->shaderProgram, "uColor"), 1,
				 glm::value_ptr(color));

	glBindVertexArray(data->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
