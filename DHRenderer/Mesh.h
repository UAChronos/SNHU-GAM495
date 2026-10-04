#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <vector>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture
{
	unsigned int id;
	std::string type;
	// Texture path for optimization
	std::string path;
};

class Mesh
{
public:
	// Mesh data
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	// Constructor
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	{
		// Set mesh data
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		// Initialize mesh
		setupMesh();
	}

	// Renders mesh using provided shader
	void Draw(Shader& shader)
	{
		// Holds current texture map index
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;

		for (unsigned int i = 0; i < textures.size(); i++)
		{
			// Activate respective texture unit
			glActiveTexture(GL_TEXTURE0 + i);

			// Retrieve texture sequential number
			std::string number;
			std::string name = textures[i].type;
			if (name == "texture_diffuse")
			{
				number = std::to_string(diffuseNr++);
			}
			else if (name == "texture_specular")
			{
				number = std::to_string(specularNr++);
			}

			// Pass texture unit id to shader
			shader.setInt(("material." + name + number).c_str(), i);

			// Bind texture
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		// Bind VAO and draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		// Unbind VAO
		glBindVertexArray(0);

		// Reset active texture unit
		glActiveTexture(GL_TEXTURE0);
	}

private:
	// Render data (vertex buffer object, vertex array object, element buffer object)
	unsigned int VAO, VBO, EBO;

	// Initializes data and prepares mesh for rendering
	void setupMesh()
	{
		// Create render buffers and store their IDs
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		// Bind VBO and copy vertices to buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		// Bind EBO and copy vertex indices to buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// Configure vertex attribute 0 (position) to read from VBO and enable it
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		// Configure vertex attribute 1 (Normal) to read from VBO and enable it
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		// Configure vertex attribute 2 (TexCoords) to read from VBO and enable it
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		// Unbind VBO
		glBindVertexArray(0);
	}
};