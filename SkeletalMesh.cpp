#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include "SkeletalMesh.h"
#include <glad/glad.h>
#include <GL/GL.h>

using std::vector;
using sf::Color;
using sf::Vector2u;
using glm::mat4;
using glm::vec4;

SkeletalMesh::SkeletalMesh(std::vector<SkeletalVertex>&& vertices, std::vector<uint32_t>&& faces,
	Texture texture)
	: SkeletalMesh(std::move(vertices), std::move(faces), std::vector<Texture>{texture}) {
}

SkeletalMesh::SkeletalMesh(std::vector<SkeletalVertex>&& vertices, std::vector<uint32_t>&& faces, std::vector<Texture>&& textures)
	: m_vertexCount(vertices.size()), m_faceCount(faces.size()), m_textures(textures) {

	// Generate a vertex array object on the GPU.
	glGenVertexArrays(1, &m_vao);
	// "Bind" the newly-generated vao, which makes future functions operate on that specific object.
	glBindVertexArray(m_vao);

	// Generate a vertex buffer object on the GPU.
	uint32_t vbo;
	glGenBuffers(1, &vbo);

	// "Bind" the newly-generated vbo, which makes future functions operate on that specific object.
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// This vbo is now associated with m_vao.
	// Copy the contents of the vertices list to the buffer that lives on the GPU.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SkeletalVertex), &vertices[0], GL_STATIC_DRAW);

	// Inform OpenGL how to interpret the buffer. Each vertex now has TWO attributes; a position and a color.
	// Atrribute 0 is position: 3 contiguous floats (x/y/z)...
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkeletalVertex), 0);
	glEnableVertexAttribArray(0);

	// Attribute 1 is normal (nx, ny, nz): 3 contiguous floats, starting 12 bytes after the beginning of the vertex.
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkeletalVertex), (void*)offsetof(SkeletalVertex, Normal));
	glEnableVertexAttribArray(1);

	// Attribute 2 is texture coordinates (u, v): 2 contiguous floats, starting 24 bytes after the beginning of the vertex.
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkeletalVertex), (void*)offsetof(SkeletalVertex, TexCoords));
	glEnableVertexAttribArray(2);

	// add: tangent vector
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(SkeletalVertex), (void*)offsetof(SkeletalVertex, Tangent));
	glEnableVertexAttribArray(3);

	// bones id
	glVertexAttribIPointer(4, 4, GL_INT, sizeof(SkeletalVertex), (void*)offsetof(SkeletalVertex, m_BoneIDs));
	glEnableVertexAttribArray(4);

	// weights
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(SkeletalVertex), (void*)offsetof(SkeletalVertex, m_Weights));
	glEnableVertexAttribArray(5);

	// Generate a second buffer, to store the indices of each triangle in the mesh.
	uint32_t ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(uint32_t), &faces[0], GL_STATIC_DRAW);

	// Unbind the vertex array, so no one else can accidentally mess with it.
	glBindVertexArray(0);
}

void SkeletalMesh::addTexture(Texture texture)
{
	m_textures.push_back(texture);
}

void SkeletalMesh::render(sf::RenderWindow& window, ShaderProgram& program) const {
	// Activate the mesh's vertex array.
	glBindVertexArray(m_vao);
	program.setUniform("hasNormalMap", false);
	program.setUniform("hasSpecularMap", false);

	for (auto i = 0; i < m_textures.size(); i++) {
		//std::cout << m_textures[i].samplerName << " ";
		if (m_textures[i].samplerName == "normalMap") {
			program.setUniform("hasNormalMap", true);
		}
		else if (m_textures[i].samplerName == "specularMap") {
			program.setUniform("hasSpecularMap", true);
		}

		program.setUniform(m_textures[i].samplerName, i);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[i].textureId);
	}
	//std::cout << m_faceCount;
	//std::cout << "\n";

	// Draw the vertex array, using its "element buffer" to identify the faces.
	glDrawElements(GL_TRIANGLES, m_faceCount, GL_UNSIGNED_INT, nullptr);
	// Deactivate the mesh's vertex array and texture.
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

SkeletalMesh SkeletalMesh::square(const std::vector<Texture>& textures) {

	std::vector<SkeletalVertex> vertices;
	SkeletalVertex a;
	a.Position = glm::vec3(-0.5f, 0.5f, 0.0f);
	a.TexCoords = glm::vec2(0.0f, 1.0f);
	a.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
	a.Tangent = glm::vec3(0);
	vertices.push_back(a);

	a.Position = glm::vec3(-0.5f, -0.5f, 0.0f);
	a.TexCoords = glm::vec2(0.0f, 0.0f);
	a.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
	a.Tangent = glm::vec3(0);
	vertices.push_back(a);

	a.Position = glm::vec3(0.5f, -0.5f, 0.0f);
	a.TexCoords = glm::vec2(1.0f, 0.0f);
	a.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
	a.Tangent = glm::vec3(0);
	vertices.push_back(a);

	a.Position = glm::vec3(0.5f, 0.5f, 0.0f);
	a.TexCoords = glm::vec2(1.0f, 1.0f);
	a.Normal = glm::vec3(0.0f, 0.0f, 1.0f);
	a.Tangent = glm::vec3(0);
	vertices.push_back(a);

	vector<uint32_t> faces = { 0,1,2,0,2,3 };


	for (unsigned int i = 0; i < faces.size(); i += 3) {
		auto& v0 = vertices[faces[i]];
		auto& v1 = vertices[faces[i + 1]];
		auto& v2 = vertices[faces[i + 2]];

		auto Edge1 = v1.Position - v0.Position;
		auto Edge2 = v2.Position - v0.Position;

		float DeltaU1 = v1.TexCoords.x - v0.TexCoords.x;
		float DeltaV1 = v1.TexCoords.y - v0.TexCoords.y;
		float DeltaU2 = v2.TexCoords.x - v0.TexCoords.x;
		float DeltaV2 = v2.TexCoords.y - v0.TexCoords.y;

		float f = 1.0 / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

		glm::vec3 Tangent, Bitangent;

		Tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
		Tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
		Tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

		Bitangent.x = f * (-DeltaU2 * Edge1.x + DeltaU1 * Edge2.x);
		Bitangent.y = f * (-DeltaU2 * Edge1.y + DeltaU1 * Edge2.y);
		Bitangent.z = f * (-DeltaU2 * Edge1.z + DeltaU1 * Edge2.z);

		v0.Tangent += Tangent;
		v1.Tangent += Tangent;
		v2.Tangent += Tangent;
	}

	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i].Tangent = glm::normalize(vertices[i].Tangent);

		for (int j = 0; j < MAX_BONE_PER_VERTEX; j++)
		{
			vertices[i].m_BoneIDs[j] = -1;
			vertices[i].m_Weights[j] = 0.0f;
		}

		//std::cout << vertices[i].Position << " " << vertices[i].Tangent << "\n";
	}

	return SkeletalMesh(std::vector<SkeletalVertex>(vertices), std::vector<uint32_t>(faces), std::vector<Texture>(textures));

}