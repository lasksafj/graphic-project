#pragma once
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "ShaderProgram.h"
#include "Texture.h"

constexpr int MAX_BONE_PER_VERTEX = 4;


struct SkeletalVertex {
	// position
	glm::vec3 Position;
	// normal
	glm::vec3 Normal;
	// texCoords
	glm::vec2 TexCoords;
	// tangent
	glm::vec3 Tangent;

	// bone indexes which will influence this vertex
	int m_BoneIDs[MAX_BONE_PER_VERTEX];
	//weights from each bone
	float m_Weights[MAX_BONE_PER_VERTEX];

};

/**
 * @brief Represents a mesh whose vertices have positions, normal vectors, and texture coordinates;
 * as well as a list of Textures to bind when rendering the mesh.
 */
class SkeletalMesh {
private:
	uint32_t m_vao;
	std::vector<Texture> m_textures;
	size_t m_vertexCount;
	size_t m_faceCount;

public:
	SkeletalMesh() = delete;


	/**
	 * @brief Construcst a Mesh3D using existing vectors of vertices and faces.
	*/
	SkeletalMesh(std::vector<SkeletalVertex>&& vertices, std::vector<uint32_t>&& faces,
		Texture texture);

	SkeletalMesh(std::vector<SkeletalVertex>&& vertices, std::vector<uint32_t>&& faces,
		std::vector<Texture>&& textures);

	void addTexture(Texture texture);


	/**
	 * @brief Renders the mesh to the given context.
	 */
	void render(sf::RenderWindow& window, ShaderProgram& program) const;


	static SkeletalMesh square(const std::vector<Texture>& textures);
};
