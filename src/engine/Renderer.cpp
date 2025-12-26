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

struct CameraData
{
	glm::mat4 view{};
	glm::mat4 proj{};
	glm::vec3 cameraPos{};
	float _pad0 = 0.0f; // std140 padding
};

Texture Renderer::sBlankTexture = {};

Renderer::Renderer() { mRenderData = new RenderData(); }

Renderer::~Renderer() { delete mRenderData; }

bool Renderer::init()
{
	// --- Camera UBO -----------------------------------------------------
	glGenBuffers(1, &mRenderData->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, mRenderData->ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr,
				 GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the UBO to binding point 0
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mRenderData->ubo);

	// --- Shader ---------------------------------------------------------
	const char *vs = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoords;

		layout(std140) uniform Camera
		{
			mat4 uView;
			mat4 uProj;
			vec3 uCameraPos;
			float _pad0;
		};

        uniform mat4 model;

        out vec2 texCoords;

        void main()
        {
			texCoords = aTexCoords;
            gl_Position = uProj * uView * model * vec4(aPos, 1.0);
        }
    )";

	const char *fs = R"(
        #version 460 core

        in vec2 texCoords;

		uniform sampler2D uTexture;
		uniform vec4 uColor;

        out vec4 FragColor;

        void main()
        {
			vec4 texColor = texture(uTexture, texCoords);

			if (texColor.a < 0.5)
				discard;

            FragColor = texColor * uColor;
        }
    )";

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs, nullptr);
	glCompileShader(vertex);

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs, nullptr);
	glCompileShader(fragment);

	mRenderData->shaderProgram = glCreateProgram();
	glAttachShader(mRenderData->shaderProgram, vertex);
	glAttachShader(mRenderData->shaderProgram, fragment);
	glLinkProgram(mRenderData->shaderProgram);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// Set camera UBO
	GLuint index = glGetUniformBlockIndex(mRenderData->shaderProgram, "Camera");
	glUniformBlockBinding(mRenderData->shaderProgram, index, 0);

	// --- Quad Geometry ---------------------------------------------------
	float quadVertices[] = {
		// pos        // uv
		-0.5f, -0.5f, 0.0f,	 0.0f, 0.0f, 0.5f, -0.5f, 0.0f,
		1.0f,  0.0f,  0.5f,	 0.5f, 0.0f, 1.0f, 1.0f,

		-0.5f, -0.5f, 0.0f,	 0.0f, 0.0f, 0.5f, 0.5f,  0.0f,
		1.0f,  1.0f,  -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	};

	glGenVertexArrays(1, &mRenderData->vao);
	glGenBuffers(1, &mRenderData->vbo);

	glBindVertexArray(mRenderData->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mRenderData->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
						  (void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
						  (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// --- GL State --------------------------------------------------------
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Blank texture
	unsigned char data[] = {0xff, 0xff, 0xff, 0xff};
	sBlankTexture = createTexture(data, 1, 1);

	return true;
}

void Renderer::shutdown()
{
	deleteTexture(sBlankTexture);

	if (mRenderData->vbo != 0)
	{
		glDeleteBuffers(1, &mRenderData->vbo);
		mRenderData->vbo = 0;
	}

	if (mRenderData->vao != 0)
	{
		glDeleteVertexArrays(1, &mRenderData->vao);
		mRenderData->vao = 0;
	}

	if (mRenderData->shaderProgram != 0)
	{
		glDeleteProgram(mRenderData->shaderProgram);
		mRenderData->shaderProgram = 0;
	}

	if (mRenderData->ubo != 0)
	{
		glDeleteBuffers(1, &mRenderData->ubo);
		mRenderData->ubo = 0;
	}
}

Texture Renderer::createTexture(unsigned char *data, int width, int height)
{
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	// Upload
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, data);

	// Sampler parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	GLTexture *tex = new GLTexture();
	tex->id = id;
	tex->width = width;
	tex->height = height;

	return {reinterpret_cast<int64_t>(tex), width, height};
}

Texture Renderer::loadTexture(const char *path)
{
	int w, h, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(path, &w, &h, &channels, 4);

	if (!data)
	{
		std::cerr << "Failed to load image: " << path << std::endl;
		return {};
	}

	Texture tex = createTexture(data, w, h);
	stbi_image_free(data);

	return tex;
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

	glBindBuffer(GL_UNIFORM_BUFFER, mRenderData->ubo);
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
						glm::vec4 color, Texture texture) const
{
	glUseProgram(mRenderData->shaderProgram);

	glm::mat4 model = glm::mat4(1.0f);

	// Translate
	model = glm::translate(model, position);

	// Rotate xyz
	model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
	model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
	model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});

	// Scale
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

	glUniformMatrix4fv(
		glGetUniformLocation(mRenderData->shaderProgram, "model"), 1, GL_FALSE,
		glm::value_ptr(model));

	glUniform1i(glGetUniformLocation(mRenderData->shaderProgram, "uTexture"),
				0);

	glUniform4fv(glGetUniformLocation(mRenderData->shaderProgram, "uColor"), 1,
				 glm::value_ptr(color));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLTexture *>(texture.id)->id);

	glBindVertexArray(mRenderData->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
