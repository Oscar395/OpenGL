#include "Resource.h"

RsrcManager::RsrcManager() {
	meshes["CUBE"] = loadCubeMesh();
	loadShaders();
}
RsrcManager::~RsrcManager() {

}

void RsrcManager::loadShaders() {
	shaders["LIGHT"] = Shader("Resources/shaders/lightShader.vert", "Resources/shaders/lightShader.frag");
}

Shader RsrcManager::ShaderProgram(std::string name) {
	return shaders[name];
}

std::shared_ptr<Mesh> RsrcManager::GetMesh(std::string name) {
	// assert(meshes.find(name) != meshes.end() && "mesh does not exits!");
	return std::make_shared<Mesh>(meshes[name]);
}

Mesh RsrcManager::loadCubeMesh() {
	Model cube("Resources/models/cube/cube.obj");
	std::cout << "cube loaded" << std::endl;

	return cube.GetFirstMesh();
}