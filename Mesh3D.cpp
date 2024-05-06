#include <iostream>
#include "Mesh3D.h"
#include <glad/glad.h>
#include <GL/GL.h>

#include "Skeletal.h"

using std::vector;
using sf::Color;
using sf::Vector2u;
using glm::mat4;
using glm::vec4;

Mesh3D::Mesh3D(std::vector<Vertex3D>&& vertices, std::vector<uint32_t>&& faces,
	Texture texture) 
	: Mesh3D(std::move(vertices), std::move(faces), std::vector<Texture>{texture}) {
}

Mesh3D::Mesh3D(std::vector<Vertex3D>&& vertices, std::vector<uint32_t>&& faces, std::vector<Texture>&& textures)
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
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D), &vertices[0], GL_STATIC_DRAW);

	// Inform OpenGL how to interpret the buffer. Each vertex now has TWO attributes; a position and a color.
	// Atrribute 0 is position: 3 contiguous floats (x/y/z)...
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex3D), 0);
	glEnableVertexAttribArray(0);

	// Attribute 1 is normal (nx, ny, nz): 3 contiguous floats, starting 12 bytes after the beginning of the vertex.
	glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex3D), (void*)12);
	glEnableVertexAttribArray(1);

	// Attribute 2 is texture coordinates (u, v): 2 contiguous floats, starting 24 bytes after the beginning of the vertex.
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex3D), (void*)24);
	glEnableVertexAttribArray(2);

	// add: tangent vector
	glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(Vertex3D), (void*)32);
	glEnableVertexAttribArray(3);

	// Generate a second buffer, to store the indices of each triangle in the mesh.
	uint32_t ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(uint32_t), &faces[0], GL_STATIC_DRAW);

	// Unbind the vertex array, so no one else can accidentally mess with it.
	glBindVertexArray(0);
}

void Mesh3D::addTexture(Texture texture)
{
	m_textures.push_back(texture);
}

void Mesh3D::render(sf::RenderWindow& window, ShaderProgram& program) const {
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
	//std::cout << std::endl;

	// Draw the vertex array, using its "element buffer" to identify the faces.
	glDrawElements(GL_TRIANGLES, m_faceCount, GL_UNSIGNED_INT, nullptr);
	// Deactivate the mesh's vertex array and texture.
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Mesh3D Mesh3D::square(const std::vector<Texture> &textures) {

	return Mesh3D(
		{ 
		  { 0.5, 0.5, 0, 0, 0, 1, 1, 0, 1,0,0 },    // TR
		  { 0.5, -0.5, 0, 0, 0, 1, 1, 1, 1,0,0 },   // BR
		  { -0.5, -0.5, 0, 0, 0, 1, 0, 1, 1,0,0 },  // BL
		  { -0.5, 0.5, 0, 0, 0, 1, 0, 0, 1,0,0 },   // TL
		}, 
		{ 
			2, 1, 3,
			3, 1, 0,
		},
		std::vector<Texture>(textures)
	);

}

Mesh3D Mesh3D::triangle(Texture texture) {
	return Mesh3D(
		{ { -0.5, -0.5, 0., 0, 0, 1, 0., 1., 0,0,0 },
		  { -0.5, 0.5, 0, 0, 0, 1, 0., 0., 0,0,0 },
		  { 0.5, 0.5, 0, 0, 0, 1, 1, 0, 0,0,0 } },
		{ 2, 1, 0 },
		texture
	);
}

Mesh3D Mesh3D::cube(Texture texture) {
	//std::vector<Vertex3D> verts = {
	//	/*BUR*/{ 0.5, 0.5, -0.5,  0, 0,  0,0,0,0,0,0},
	//	/*BUL*/{ -0.5, 0.5, -0.5, 0, 0,  0,0,0,0,0,0},
	//	/*BLL*/{ -0.5, -0.5, -0.5, 1.0, 0,  0,0,0,0,0,0 },
	//	/*BLR*/{ 0.5, -0.5, -0.5, 0, 1.0,  0,0,0,0,0,0},
	//	/*FUR*/{ 0.5, 0.5, 0.5, 1.0, 0,  0,0,0,0,0,0},
	//	/*FUL*/{-0.5, 0.5, 0.5, 1.0, 1.0,  0,0,0,0,0,0},
	//	/*FLL*/{-0.5, -0.5, 0.5, 0, 1.0,  0,0,0,0,0,0},
	//	/*FLR*/{0.5, -0.5, 0.5, 1.0, 1.0,  0,0,0,0,0,0}
	//};
	//std::vector<uint32_t> tris = {
	//	0, 1, 2,
	//	0, 2, 3,
	//	4, 0, 3,
	//	4, 3, 7,
	//	5, 4, 7,
	//	5, 7, 6,
	//	1, 5, 6,
	//	1, 6, 2,
	//	4, 5, 1,
	//	4, 1, 0,
	//	2, 6, 7,
	//	2, 7, 3
	//};

	return Mesh3D(
		{
		/*BUR*/{ 0.5, 0.5, -0.5,			0, 0,  0,0,0,0,0,0},
		/*BUL*/{ -0.5, 0.5, -0.5,			0, 0,  0,0,0,0,0,0},
		/*BLL*/{ -0.5, -0.5, -0.5,			1.0, 0,  0,0,0,0,0,0 },
		/*BLR*/{ 0.5, -0.5, -0.5,			0, 1.0,  0,0,0,0,0,0},
		/*FUR*/{ 0.5, 0.5, 0.5,				1.0, 0,  0,0,0,0,0,0},
		/*FUL*/{-0.5, 0.5, 0.5,				1.0, 1.0,  0,0,0,0,0,0},
		/*FLL*/{-0.5, -0.5, 0.5,			0, 1.0,  0,0,0,0,0,0},
		/*FLR*/{0.5, -0.5, 0.5,				1.0, 1.0,  0,0,0,0,0,0}
		}, 
		//{
		//	{-1.0f, -1.0f,  1.0f,	0, 0,  0,0,0,0,0,0 },		//        7--------6
		//	{1.0f, -1.0f,  1.0f,	0, 0,  0,0,0,0,0,0	},		//       /|       /|
		//	{1.0f, -1.0f, -1.0f,	1.0, 0,  0,0,0,0,0,0 },		//      4--------5 |
		//	{-1.0f, -1.0f, -1.0f,	0, 1.0,  0,0,0,0,0,0 },		//      | |      | |
		//	{-1.0f,  1.0f,  1.0f,	1.0, 0,  0,0,0,0,0,0 },		//      | 3------|-2
		//	{ 1.0f,  1.0f,  1.0f,	1.0, 1.0,  0,0,0,0,0,0 },	//      |/       |/
		//	{ 1.0f,  1.0f, -1.0f,	0, 1.0,  0,0,0,0,0,0 },		//      0--------1
		//	{-1.0f,  1.0f, -1.0f,	1.0, 1.0,  0,0,0,0,0,0 },
		//},
		{
			// Right
			1, 2, 6,
			6, 5, 1,
			// Left
			0, 4, 7,
			7, 3, 0,
			// Top
			4, 5, 6,
			6, 7, 4,
			// Bottom
			0, 3, 2,
			2, 1, 0,
			// Back
			0, 1, 5,
			5, 4, 0,
			// Front
			3, 7, 6,
			6, 2, 3
		}, texture);
}
