#pragma once
#include <iostream>
#include "Objects/model.h"
#include "Objects/shader.h"
#include <map>

class RsrcManager {
public:
	~RsrcManager();
	RsrcManager(const RsrcManager&) = delete;
	RsrcManager& operator=(const RsrcManager&) = delete;

	static RsrcManager& ref() {
		static RsrcManager reference;
		return reference;
	}

	std::shared_ptr<Mesh> GetMesh(std::string name);

	Shader ShaderProgram(std::string name);

private:
	std::map<std::string, Mesh> meshes;
	std::map<std::string, Shader> shaders;
	Mesh loadCubeMesh();
	void loadShaders();
	RsrcManager();
};

// static RsrcManager& Resource = RsrcManager::ref();