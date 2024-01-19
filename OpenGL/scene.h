#pragma once

#include <glm/glm.hpp>
#include "shader.h"
#include "model.h"

class Scene
{
private:
	vector<Model> objects;
	vector<glm::vec3> positions;

	glm::mat4 model;
public:
	Scene();
	void addObject(const Model& object, const glm::vec3& position);
	void render(Shader shader);
};