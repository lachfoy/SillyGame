#include "Renderer.h"
#include "Engine.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma warning(push)
#pragma warning(disable : 26495)
#pragma warning(disable : 6262)
#pragma warning(disable : 6054)
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable : 26451)
#pragma warning(disable : 26819)
#pragma warning(disable : 6262)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include <exception>
#include <iostream>
#include <unordered_map>

struct GLTexture
{
	GLuint id;
	int width;
	int height;
};

struct GLMesh
{
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
	uint32_t indexCount;
};

struct RendererImpl
{
	GLuint cameraUbo = 0;
	GLuint lightingUbo = 0;
	GLuint shaderProgram = 0;

	GLuint whiteTexture = 0;

	GLuint quadVao = 0;
	GLuint quadVbo = 0;

	// mesh cache
	std::unordered_map<int64_t, GLMesh> meshes;
	int64_t nextMeshId = 1;

	// UI
	GLuint uiShader = 0;
	GLuint uiVao = 0;
	GLuint uiVbo = 0;

	glm::mat4 uiProj;
};

struct alignas(16) CameraData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 cameraPos;
	float _pad0;
};

struct alignas(16) LightingData
{
	glm::vec3 lightPos;
	float _pad0;
	glm::vec3 lightColor;
	float _pad1;
	glm::vec3 ambient;
	float _pad2;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

struct UIVertex
{
	glm::vec2 pos; // local quad position
	glm::vec2 uv;
};

Renderer::Renderer() { mRendererImpl = new RendererImpl(); }

Renderer::~Renderer() { delete mRendererImpl; }

bool Renderer::init()
{
	// --- Camera UBO -----------------------------------------------------
	glGenBuffers(1, &mRendererImpl->cameraUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, mRendererImpl->cameraUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraData), nullptr,
				 GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, mRendererImpl->cameraUbo);

	// Lighting ubo
	glGenBuffers(1, &mRendererImpl->lightingUbo);
	glBindBuffer(GL_UNIFORM_BUFFER, mRendererImpl->lightingUbo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightingData), nullptr,
				 GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, mRendererImpl->lightingUbo);

	// --- Shader ---------------------------------------------------------
	const char *vs = R"(
        #version 460 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;

		layout(std140, binding = 0) uniform Camera
		{
			mat4 uView;
			mat4 uProj;
			vec3 uCameraPos;
			float _pad0;
		};

		layout(std140, binding = 1) uniform Lighting
		{
			vec3 lightPos;
			float _pad1;
			vec3 lightColor;
			float _pad2;
			vec3 ambient;
			float _pad3;
		};

        uniform mat4 model;

        out vec2 texCoords;
		out vec4 vertexColor;

        void main()
        {
			vec3 worldPos = vec3(model * vec4(aPos, 1.0));

			mat3 normalMatrix = transpose(inverse(mat3(model)));
			vec3 norm = normalize(normalMatrix * aNormal);

			vec3 lightDir = normalize(lightPos - worldPos);
			float diff = max(dot(norm, lightDir), 0.0);

			vec3 diffuse = diff * lightColor;

            vertexColor = vec4(diffuse + ambient, 1.0);

			texCoords = aTexCoords;

            gl_Position = uProj * uView * model * vec4(aPos, 1.0);
        }
    )";

	const char *fs = R"(
        #version 460 core

        in vec2 texCoords;
        in vec4 vertexColor;

		uniform sampler2D uTexture;
		uniform vec4 uColor;

        out vec4 FragColor;

        void main()
        {
			vec4 texColor = texture(uTexture, texCoords);

			if (texColor.a < 0.5)
				discard;

            FragColor = texColor * uColor * vertexColor;
        }
    )";

	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs, nullptr);
	glCompileShader(vertex);

	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs, nullptr);
	glCompileShader(fragment);

	mRendererImpl->shaderProgram = glCreateProgram();
	glAttachShader(mRendererImpl->shaderProgram, vertex);
	glAttachShader(mRendererImpl->shaderProgram, fragment);
	glLinkProgram(mRendererImpl->shaderProgram);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// Set camera UBO
	GLuint cameraBlockIndex =
		glGetUniformBlockIndex(mRendererImpl->shaderProgram, "Camera");
	glUniformBlockBinding(mRendererImpl->shaderProgram, cameraBlockIndex, 0);
	GLuint lightingBlockIndex =
		glGetUniformBlockIndex(mRendererImpl->shaderProgram, "Lighting");
	glUniformBlockBinding(mRendererImpl->shaderProgram, lightingBlockIndex, 1);

	// --- Quad Geometry ---------------------------------------------------
	// clang-format off
	std::vector<Vertex> quadVertices = {
		// pos				      // normal		         // uv
		{{-0.5f, -0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},    {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},    {1.0f, 0.0f}},
		{{ 0.5f,  0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},    {1.0f, 1.0f}},
		{{-0.5f, -0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},    {0.0f, 0.0f}},
		{{ 0.5f,  0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},    {1.0f, 1.0f}},
		{{-0.5f,  0.5f, 0.0f},    {0.0f, 0.0f, 1.0f},    {0.0f, 1.0f}}
	};
	// clang-format on

	glGenBuffers(1, &mRendererImpl->quadVbo);

	glGenVertexArrays(1, &mRendererImpl->quadVao);
	glBindVertexArray(mRendererImpl->quadVao);

	glBindBuffer(GL_ARRAY_BUFFER, mRendererImpl->quadVbo);
	glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vertex),
				 quadVertices.data(), GL_STATIC_DRAW);

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

	// White texture to use for shaders if a texture is not specified
	glGenTextures(1, &mRendererImpl->whiteTexture);
	glBindTexture(GL_TEXTURE_2D, mRendererImpl->whiteTexture);

	unsigned char data[] = {0xff, 0xff, 0xff, 0xff};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				 data);

	glBindTexture(GL_TEXTURE_2D, 0);

	// --- UI --------------------------------------------------------
	const char *uiVertexSrc = R"(
        #version 460 core

        layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aUV;

		uniform mat4 uMVP;

		out vec2 vUV;

        void main()
        {
			vUV = aUV;
			gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
        }
    )";

	const char *uiFragSrc = R"(
        #version 460 core

		in vec2 vUV;

		uniform sampler2D uTexture;
		uniform vec4 uColor;

		out vec4 FragColor;

		void main()
		{
			vec4 tex = texture(uTexture, vUV);
			FragColor = tex * uColor;
		}
    )";

	GLuint uiVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(uiVertexShader, 1, &uiVertexSrc, nullptr);
	glCompileShader(uiVertexShader);

	GLuint uiFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(uiFragShader, 1, &uiFragSrc, nullptr);
	glCompileShader(uiFragShader);

	mRendererImpl->uiShader = glCreateProgram();
	glAttachShader(mRendererImpl->uiShader, uiVertexShader);
	glAttachShader(mRendererImpl->uiShader, uiFragShader);
	glLinkProgram(mRendererImpl->uiShader);

	glDeleteShader(uiVertexShader);
	glDeleteShader(uiFragShader);

	// clang-format off
	std::vector<UIVertex> uiQuad = {
		{{0, 0}, {0, 0}},
		{{1, 0}, {1, 0}},
		{{1, 1}, {1, 1}},

		{{0, 0}, {0, 0}},
		{{1, 1}, {1, 1}},
		{{0, 1}, {0, 1}},
	};
	// clang-format on

	glGenBuffers(1, &mRendererImpl->uiVbo);

	glGenVertexArrays(1, &mRendererImpl->uiVao);
	glBindVertexArray(mRendererImpl->uiVao);

	glBindBuffer(GL_ARRAY_BUFFER, mRendererImpl->uiVbo);
	glBufferData(GL_ARRAY_BUFFER, uiQuad.size() * sizeof(UIVertex),
				 uiQuad.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
						  (void *)offsetof(UIVertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex),
						  (void *)offsetof(UIVertex, uv));

	glBindVertexArray(0);

	return true;
}

void Renderer::shutdown() noexcept
{
	// Manual deleting?
	for (auto &[id, mesh] : mRendererImpl->meshes)
	{
		glDeleteVertexArrays(1, &mesh.vao);
		glDeleteBuffers(1, &mesh.vbo);
		glDeleteBuffers(1, &mesh.ibo);
	}

	glDeleteVertexArrays(1, &mRendererImpl->quadVao);

	if (mRendererImpl->whiteTexture != 0)
	{
		glDeleteTextures(1, &mRendererImpl->whiteTexture);
		mRendererImpl->whiteTexture = 0;
	}

	if (mRendererImpl->uiVbo != 0)
	{
		glDeleteBuffers(1, &mRendererImpl->uiVbo);
		mRendererImpl->uiVbo = 0;
	}

	if (mRendererImpl->uiVao != 0)
	{
		glDeleteVertexArrays(1, &mRendererImpl->uiVao);
		mRendererImpl->uiVao = 0;
	}

	if (mRendererImpl->uiShader != 0)
	{
		glDeleteProgram(mRendererImpl->uiShader);
		mRendererImpl->uiShader = 0;
	}

	if (mRendererImpl->quadVbo != 0)
	{
		glDeleteBuffers(1, &mRendererImpl->quadVbo);
		mRendererImpl->quadVbo = 0;
	}

	if (mRendererImpl->shaderProgram != 0)
	{
		glDeleteProgram(mRendererImpl->shaderProgram);
		mRendererImpl->shaderProgram = 0;
	}

	if (mRendererImpl->lightingUbo != 0)
	{
		glDeleteBuffers(1, &mRendererImpl->lightingUbo);
		mRendererImpl->lightingUbo = 0;
	}

	if (mRendererImpl->cameraUbo != 0)
	{
		glDeleteBuffers(1, &mRendererImpl->cameraUbo);
		mRendererImpl->cameraUbo = 0;
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
		std::stringstream ss;
		ss << "Failed to load " << path;
		throw std::runtime_error(ss.str());
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
	assert(false); // DO NOT TRY AND USE, this is broken

	std::vector<Vertex> vertices = {
		{{-1, -1, 0}, {0, 0, 1}, {0, 0}},
		{{1, -1, 0}, {0, 0, 1}, {1, 0}},
		{{1, 1, 0}, {0, 0, 1}, {1, 1}},
		{{-1, 1, 0}, {0, 0, 1}, {0, 1}},
	};

	std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

	GLMesh glMesh{};

	glGenBuffers(1, &glMesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, glMesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
				 vertices.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &glMesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
				 indices.data(), GL_STATIC_DRAW);

	glMesh.indexCount = (uint32_t)indices.size();

	int64_t id = mRendererImpl->nextMeshId++;
	mRendererImpl->meshes[id] = glMesh;

	return {id};
}

Mesh Renderer::loadMesh(const char *path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, nullptr, &err, path);

	if (!err.empty())
	{
		std::cerr << err << std::endl;
	}

	if (!ret)
	{
		std::stringstream ss;
		ss << "Failed to load " << path;
		throw std::runtime_error(ss.str());
	}

	if (shapes.size() > 1)
	{
		throw std::runtime_error("Right now only 1 shape is handled.");
	}

	struct IndexKey
	{
		int v;
		int n;
		int t;

		bool operator<(const IndexKey &other) const
		{
			if (v != other.v)
				return v < other.v;
			if (n != other.n)
				return n < other.n;
			return t < other.t;
		}
	};

	std::map<IndexKey, uint32_t> indexMap;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto &shape : shapes)
	{
		for (const auto &idx : shape.mesh.indices)
		{
			IndexKey key{idx.vertex_index, idx.normal_index,
						 idx.texcoord_index};

			auto it = indexMap.find(key);
			if (it == indexMap.end())
			{
				Vertex v{};

				v.position[0] = attrib.vertices[3 * idx.vertex_index + 0];
				v.position[1] = attrib.vertices[3 * idx.vertex_index + 1];
				v.position[2] = attrib.vertices[3 * idx.vertex_index + 2];

				if (idx.normal_index >= 0)
				{
					v.normal[0] = attrib.normals[3 * idx.normal_index + 0];
					v.normal[1] = attrib.normals[3 * idx.normal_index + 1];
					v.normal[2] = attrib.normals[3 * idx.normal_index + 2];
				}

				if (idx.texcoord_index >= 0)
				{
					v.uv[0] = attrib.texcoords[2 * idx.texcoord_index + 0];
					v.uv[1] =
						1.0f - attrib.texcoords[2 * idx.texcoord_index + 1];
				}

				uint32_t newIndex = static_cast<uint32_t>(vertices.size());
				vertices.push_back(v);
				indexMap[key] = newIndex;
				indices.push_back(newIndex);
			}
			else
			{
				indices.push_back(it->second);
			}
		}
	}

	GLMesh glMesh{};

	glGenBuffers(1, &glMesh.vbo);
	glGenBuffers(1, &glMesh.ibo);

	glGenVertexArrays(1, &glMesh.vao);
	glBindVertexArray(glMesh.vao);

	glBindBuffer(GL_ARRAY_BUFFER, glMesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
				 vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
				 indices.data(), GL_STATIC_DRAW);

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

	glMesh.indexCount = static_cast<uint32_t>(indices.size());

	int64_t id = mRendererImpl->nextMeshId++;
	mRendererImpl->meshes[id] = glMesh;

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

	glBindBuffer(GL_UNIFORM_BUFFER, mRendererImpl->cameraUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraData), &data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// GL state
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
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

void Renderer::setLighting(glm::vec3 lightPos, glm::vec3 lightColor,
						   glm::vec3 ambient)
{
	LightingData data;
	data.lightPos = lightPos;
	data.lightColor = lightColor;
	data.ambient = ambient;

	glBindBuffer(GL_UNIFORM_BUFFER, mRendererImpl->lightingUbo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingData), &data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::drawMesh(Mesh mesh, glm::mat4 transform, Texture texture)
{
	auto it = mRendererImpl->meshes.find(mesh.id);
	if (it == mRendererImpl->meshes.end())
		return;

	const GLMesh &glMesh = it->second;

	glUseProgram(mRendererImpl->shaderProgram);

	glUniformMatrix4fv(
		glGetUniformLocation(mRendererImpl->shaderProgram, "model"), 1,
		GL_FALSE, glm::value_ptr(transform));

	glUniform1i(glGetUniformLocation(mRendererImpl->shaderProgram, "uTexture"),
				0);

	glUniform4f(glGetUniformLocation(mRendererImpl->shaderProgram, "uColor"), 1,
				1, 1, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,
				  texture.id != 0
					  ? reinterpret_cast<GLTexture *>(texture.id)->id
					  : mRendererImpl->whiteTexture);

	glBindVertexArray(glMesh.vao);
	glDrawElements(GL_TRIANGLES, glMesh.indexCount, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void Renderer::drawQuad(glm::vec3 position, glm::vec3 rotation, glm::vec3 size,
						glm::vec4 color, Texture texture)
{
	glm::mat4 model = glm::mat4(1.0f);

	// Translate
	model = glm::translate(model, position);

	// Rotate xyz
	model = glm::rotate(model, glm::radians(rotation.x), {1, 0, 0});
	model = glm::rotate(model, glm::radians(rotation.y), {0, 1, 0});
	model = glm::rotate(model, glm::radians(rotation.z), {0, 0, 1});

	// Scale
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

	glUseProgram(mRendererImpl->shaderProgram);

	glUniformMatrix4fv(
		glGetUniformLocation(mRendererImpl->shaderProgram, "model"), 1,
		GL_FALSE, glm::value_ptr(model));

	glUniform1i(glGetUniformLocation(mRendererImpl->shaderProgram, "uTexture"),
				0);

	glUniform4fv(glGetUniformLocation(mRendererImpl->shaderProgram, "uColor"),
				 1, glm::value_ptr(color));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,
				  texture.id != 0
					  ? reinterpret_cast<GLTexture *>(texture.id)->id
					  : mRendererImpl->whiteTexture);

	glBindVertexArray(mRendererImpl->quadVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Renderer::begin2D(int screenWidth, int screenHeight)
{
	mRendererImpl->uiProj =
		glm::ortho(0.0f, static_cast<float>(screenWidth),
				   static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::end2D() {}

void Renderer::drawUIQuad(glm::vec2 position, glm::vec2 size, glm::vec4 color)
{
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));

	model = glm::scale(model, glm::vec3(size, 1.0f));

	glm::mat4 mvp = mRendererImpl->uiProj * model;

	glUseProgram(mRendererImpl->uiShader);

	glUniformMatrix4fv(glGetUniformLocation(mRendererImpl->uiShader, "uMVP"), 1,
					   GL_FALSE, glm::value_ptr(mvp));

	glUniform4fv(glGetUniformLocation(mRendererImpl->uiShader, "uColor"), 1,
				 glm::value_ptr(color));

	glUniform1i(glGetUniformLocation(mRendererImpl->uiShader, "uTexture"),
				0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mRendererImpl->whiteTexture);

	glBindVertexArray(mRendererImpl->uiVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
