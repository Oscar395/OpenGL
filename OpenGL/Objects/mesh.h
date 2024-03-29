#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"

#include <string>
#include <vector>

using namespace std;

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};

struct Texture {
	unsigned int id;
	string type;
	string path;
};

class Mesh {
public:
	// mesh data
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	// unsigned int normalMap;
	bool usingNormalMap = false;
	bool usingHeightMap = false;

	Mesh() = default;
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices);
	void Draw(Shader &shader);
private:
	// render data
	unsigned int VAO, VBO, EBO;

	void setupMesh();
};