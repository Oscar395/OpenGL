#pragma once
#include <iostream>
#include "Objects/model.h"
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

	void GetMesh(std::string name);

private:
	std::map<std::string, Mesh> meshes;
	void loadCubeMesh();
	RsrcManager();
};

static RsrcManager& Resource = RsrcManager::ref();