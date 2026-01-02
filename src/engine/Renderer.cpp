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
#include <unordered_map>

struct GLTexture
{
	GLuint id;
	int width;
	int height;
};

struct RendererData
{
	GLuint ubo = 0; // Camera ubo
	GLuint shaderProgram = 0;
	GLuint vao = 0;
	GLuint vbo = 0;

	// mesh cache
	GLuint meshVAO; // All meshes share a vertex layout

	struct GLMesh
	{
		GLuint vbo;
		GLuint ibo;
		uint32_t indexCount;
	};

	std::unordered_map<int64_t, GLMesh> meshes;
	int64_t nextMeshId = 1;
};

struct CameraData
{
	glm::mat4 view{};
	glm::mat4 proj{};
	glm::vec3 cameraPos{};
	float _pad0 = 0.0f; // std140 padding
};

	struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

Texture Renderer::sBlankTexture = {};

Renderer::Renderer() { mRendererData = new RendererData(); }

Renderer::~Renderer() { delete mRendererData; }

bool Renderer::init()
{
	// --- Camera UBO -----------------------------------------------------
	glGenBuffers(1, &mRendererData->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, mRendererData->ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr,
				 GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind the UBO to binding point 0
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mRendererData->ubo);

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

	mRendererData->shaderProgram = glCreateProgram();
	glAttachShader(mRendererData->shaderProgram, vertex);
	glAttachShader(mRendererData->shaderProgram, fragment);
	glLinkProgram(mRendererData->shaderProgram);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// Set camera UBO
	GLuint index =
		glGetUniformBlockIndex(mRendererData->shaderProgram, "Camera");
	glUniformBlockBinding(mRendererData->shaderProgram, index, 0);

	// --- Quad Geometry ---------------------------------------------------
	float quadVertices[] = {
		// pos        // uv
		-0.5f, -0.5f, 0.0f,	 0.0f, 0.0f, 0.5f, -0.5f, 0.0f,
		1.0f,  0.0f,  0.5f,	 0.5f, 0.0f, 1.0f, 1.0f,

		-0.5f, -0.5f, 0.0f,	 0.0f, 0.0f, 0.5f, 0.5f,  0.0f,
		1.0f,  1.0f,  -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
	};

	glGenVertexArrays(1, &mRendererData->vao);
	glGenBuffers(1, &mRendererData->vbo);

	glBindVertexArray(mRendererData->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mRendererData->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
				 GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
						  (void *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
						  (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// --- Init mesh pipeline ---------------------------------------------------
	glGenVertexArrays(1, &mRendererData->meshVAO);
	glBindVertexArray(mRendererData->meshVAO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
						  (void *)offsetof(Vertex, uv));

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
	// Manual deleting?
	for (auto &[id, mesh] : mRendererData->meshes)
	{
		glDeleteBuffers(1, &mesh.vbo);
		glDeleteBuffers(1, &mesh.ibo);
	}

	glDeleteVertexArrays(1, &mRendererData->meshVAO);

	deleteTexture(sBlankTexture);

	if (mRendererData->vbo != 0)
	{
		glDeleteBuffers(1, &mRendererData->vbo);
		mRendererData->vbo = 0;
	}

	if (mRendererData->vao != 0)
	{
		glDeleteVertexArrays(1, &mRendererData->vao);
		mRendererData->vao = 0;
	}

	if (mRendererData->shaderProgram != 0)
	{
		glDeleteProgram(mRendererData->shaderProgram);
		mRendererData->shaderProgram = 0;
	}

	if (mRendererData->ubo != 0)
	{
		glDeleteBuffers(1, &mRendererData->ubo);
		mRendererData->ubo = 0;
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

	return {reinterpret_cast<uintptr_t>(tex), width, height};
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
	assert(texture.id != 0);

	GLTexture *tex = reinterpret_cast<GLTexture *>(texture.id);

	glDeleteTextures(1, &tex->id);
	delete tex;
}

Mesh Renderer::createQuadMesh()
{
	std::vector<Vertex> vertices = {
		{{-1, -1, 0}, {0, 0, 1}, {0, 0}},
		{{1, -1, 0}, {0, 0, 1}, {1, 0}},
		{{1, 1, 0}, {0, 0, 1}, {1, 1}},
		{{-1, 1, 0}, {0, 0, 1}, {0, 1}},
	};

	std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

	RendererData::GLMesh glMesh{};

	glGenBuffers(1, &glMesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, glMesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
				 vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &glMesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
				 indices.data(), GL_STATIC_DRAW);

	glMesh.indexCount = (uint32_t)indices.size();

	int64_t id = mRendererData->nextMeshId++;
	mRendererData->meshes[id] = glMesh;

	return {id};
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

	glBindBuffer(GL_UNIFORM_BUFFER, mRendererData->ubo);
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

void Renderer::drawMesh(Mesh mesh, glm::mat4 transform, Texture texture)
{
	auto it = mRendererData->meshes.find(mesh.id);
	if (it == mRendererData->meshes.end())
		return;

	const RendererData::GLMesh &glMesh = it->second;

	glUseProgram(mRendererData->shaderProgram);

	glUniformMatrix4fv(
		glGetUniformLocation(mRendererData->shaderProgram, "model"), 1,
		GL_FALSE, glm::value_ptr(transform));

	glUniform1i(glGetUniformLocation(mRendererData->shaderProgram, "uTexture"),
				0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLTexture *>(texture.id)->id);

	glBindVertexArray(mRendererData->meshVAO);

	glBindBuffer(GL_ARRAY_BUFFER, glMesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ibo);

	glDrawElements(GL_TRIANGLES, glMesh.indexCount, GL_UNSIGNED_INT, nullptr);

	glBindVertexArray(0);
}

void Renderer::drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
						glm::vec4 color, Texture texture) const
{
	glUseProgram(mRendererData->shaderProgram);

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
		glGetUniformLocation(mRendererData->shaderProgram, "model"), 1,
		GL_FALSE, glm::value_ptr(model));

	glUniform1i(glGetUniformLocation(mRendererData->shaderProgram, "uTexture"),
				0);

	glUniform4fv(glGetUniformLocation(mRendererData->shaderProgram, "uColor"),
				 1, glm::value_ptr(color));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLTexture *>(texture.id)->id);

	glBindVertexArray(mRendererData->vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}
