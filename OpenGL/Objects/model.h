#pragma once
#include "shader.h"
#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <stb_image.h>

class Model {
public:
	Model(const char* path);
	void Draw(Shader& shader);
	Mesh GetFirstMesh();

private:
	// model data
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	string directory;

	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};