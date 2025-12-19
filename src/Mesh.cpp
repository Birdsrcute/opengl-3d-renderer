#include <glad/glad.h>
#include "Mesh.h"
#include <cstdint>

Mesh Mesh::square(std::vector<Texture> textures) {
	Mesh m{
		{
			{ 0.5f,  0.5f, 0.0f,  1.f, 1.f,  0.f,0.f,1.f,  1.f,0.f,0.f }, // TR
			{ 0.5f, -0.5f, 0.0f,  1.f, 0.f,  0.f,0.f,1.f,  1.f,0.f,0.f }, // BR
			{-0.5f, -0.5f, 0.0f,  0.f, 0.f,  0.f,0.f,1.f,  1.f,0.f,0.f }, // BL
			{-0.5f,  0.5f, 0.0f,  0.f, 1.f,  0.f,0.f,1.f,  1.f,0.f,0.f }, // TL
		},
		{
			2, 1, 3,
			3, 1, 0,
		},
		std::move(textures)
	};
	return m;
}

Mesh::Mesh(const std::vector<Vertex3D> &vertices, const std::vector<uint32_t> &faces, 
	std::vector<Texture> meshTextures)
	: faceCount{ static_cast<uint32_t>(faces.size()) }, textures{std::move(meshTextures)}
{
	// Generate a vertex array object on the GPU.
	glGenVertexArrays(1, &vao);
	// "Bind" the newly-generated vao, which makes future functions operate on that specific object.
	glBindVertexArray(vao);

	// Generate a vertex buffer object on the GPU.
	uint32_t vbo;
	glGenBuffers(1, &vbo);

	// "Bind" the newly-generated vbo, which makes future functions operate on that specific object.
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// This vbo is now associated with m_vao.
	// Copy the contents of the vertices list to the buffer that lives on the GPU.
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex3D), &vertices[0], GL_STATIC_DRAW);

	// attribute 0: position (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex3D), 0); 
	glEnableVertexAttribArray(0);

	// attribute 1: texture coordinate (u, v)
	glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex3D), (void*)12);
	glEnableVertexAttribArray(1);

	// attribute 2: normal (nx, ny, nz)
	glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(Vertex3D), (void*)20);
	glEnableVertexAttribArray(2);

	// attribute 3: tangent (tx, ty, tz, tw)
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

void Mesh::drawMesh(ShaderProgram& program) const {
	for (uint32_t i{ 0 }; i < textures.size(); ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i].textureId);
		program.setUniform(textures[i].samplerName, (int)i);
	}
	
	glBindVertexArray(vao);
	// Draw the vertex array, using is "element buffer" to identify the faces, and whatever ShaderProgram
	// has been activated prior to this.
	glDrawElements(GL_TRIANGLES, faceCount, GL_UNSIGNED_INT, nullptr);
	// Deactivate the mesh's vertex array.
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

