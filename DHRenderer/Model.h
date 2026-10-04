#pragma once
// Renderer includes
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "Mesh.h"
#include "stb_image.h"

#include <string>
#include <vector>

// Function declarations
unsigned int TextureFromFile(const char* path, const std::string& directory);

class Model
{
public:
	// Model data
	std::vector<Mesh> meshes;
	std::string directory;
	// Vector of all the textures loaded so far for optimization purposes
	std::vector<Texture> textures_loaded;	

	Model(std::string const path)
	{
		loadModel(path);
	}

	// Draw model
	void Draw(Shader& shader)
	{
		// Draw each mesh comprising the model
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i].Draw(shader);
		}
	}

private:
	// Load model from file
	void loadModel(std::string const &path)
	{
		// Declare assimp importer
		Assimp::Importer importer;

		// TODO: test  | aiProcess_OptimizeMeshes
		// Import scene from file
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		// Check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			// Output error
			std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
			return;
		}

		// Get file parent directory path
		directory = path.substr(0, path.find_last_of('/'));

		// Process assimp's root node recursively
		processNode(scene->mRootNode, scene);
	}

	// Process scene/model nodes
	void processNode(aiNode* node, const aiScene* scene)
	{
		// Process node's meshes
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// Get node's mesh from scene meshes using mesh index
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			// Process mesh
			meshes.push_back(processMesh(mesh, scene));
		}

		// After mesh processing is complete recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			// Process child node
			processNode(node->mChildren[i], scene);
		}
	}

	// Process assimp mesh
	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		// Mesh data storage
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture> textures;

		// Process each vertex
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;

            // Placeholder vector used for convertion from assimp vector format
            glm::vec3 vector;

            // Get vertex coordinates
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Get texture coordinates if they are present
            if (mesh->mTextureCoords[0])
            {
				// Placeholder vector used for convertion from assimp vector format
                glm::vec2 vec;

                // Take first two coordinates since we don't need more
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			// Store vertex
            vertices.push_back(vertex);
        }

        // Iterate mesh's faces and retrieve the corresponding vertex indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
			// Get mesh face
            aiFace face = mesh->mFaces[i];

            // Get all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
        }

        // Parse materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Following convention is assumed for sampler names in the shaders
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. Diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // 2. Specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // Return extracted mesh object 
        return Mesh(vertices, indices, textures);
	}

	// Checks all material textures and loads them if they're not loaded yet
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
	{
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			// Check if texture was loaded before and if so, skip it
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++)
			{
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					// Texture with the same filepath has already been loaded, skip (optimization)
					skip = true; 
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				// Store in loaded texture vector to enable duplicate check
				textures_loaded.push_back(texture);  
			}
		}
		return textures;
	}
};

unsigned int TextureFromFile(const char* path, const std::string& directory)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
		{
			format = GL_RED;
		}
		else if (nrComponents == 3)
		{
			format = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}